#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <unistd.h> // alarm, _exit

#include "common/threadpool.hpp"

using namespace std::chrono_literals;

namespace {
struct ReturnsInt {
    int operator()() const { return 1; }
};
} // namespace

static_assert(std::is_same_v<decltype(std::declval<ThreadPool&>().Submit(ReturnsInt{})),
                             std::future<int>>,
              "ThreadPool::Submit must return std::future<R> in C++17.");

namespace {

template <class Fut>
void assert_future_ready(Fut& fut, std::chrono::milliseconds timeout) {
    ASSERT_EQ(fut.wait_for(timeout), std::future_status::ready);
}

} // namespace

TEST(ThreadPoolTest, SubmitRunsAndReturnsValue) {
    ThreadPool pool(/*thread_num=*/2);
    auto f = pool.Submit([] { return 7; });
    assert_future_ready(f, 800ms);
    EXPECT_EQ(f.get(), 7);
}

TEST(ThreadPoolTest, SubmitBindsArgumentsCorrectly) {
    ThreadPool pool(/*thread_num=*/2);
    auto f = pool.Submit([](int a, int b) { return a + b; }, 40, 2);
    assert_future_ready(f, 800ms);
    EXPECT_EQ(f.get(), 42);
}

TEST(ThreadPoolTest, SubmitVoidTaskCompletes) {
    ThreadPool pool(/*thread_num=*/2);
    std::atomic<int> hits{0};
    auto f = pool.Submit([&] { hits.fetch_add(1, std::memory_order_relaxed); });
    assert_future_ready(f, 800ms);
    f.get();
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1);
}

TEST(ThreadPoolTest, ManyTasksAllComplete) {
    ThreadPool pool(/*thread_num=*/4);
    constexpr int kN = 5000;

    std::atomic<int> hits{0};
    std::vector<std::future<void>> futs;
    futs.reserve(kN);

    for (int i = 0; i < kN; ++i) {
        futs.emplace_back(pool.Submit([&] { hits.fetch_add(1, std::memory_order_relaxed); }));
    }
    for (auto& f : futs) {
        assert_future_ready(f, 2000ms);
        f.get();
    }
    EXPECT_EQ(hits.load(std::memory_order_relaxed), kN);
}

TEST(ThreadPoolTest, ExceptionsPropagateToFuture) {
    ThreadPool pool(/*thread_num=*/2);
    auto f = pool.Submit([]() -> int { throw std::runtime_error("boom"); });
    assert_future_ready(f, 800ms);
    EXPECT_THROW((void)f.get(), std::runtime_error);
}

TEST(ThreadPoolTest, NestedSubmitWorksWithEnoughThreads) {
    // A realistic usage pattern: a task enqueues another task and waits for it.
    // With >=2 workers this should complete (it is allowed to deadlock with 1 worker).
    ThreadPool pool(/*thread_num=*/2);
    auto outer = pool.Submit([&] {
        auto inner = pool.Submit([] { return 123; });
        return inner.get();
    });
    assert_future_ready(outer, 1200ms);
    EXPECT_EQ(outer.get(), 123);
}

TEST(ThreadPoolTest, ConcurrentSubmitFromManyThreadsNoLoss) {
    ThreadPool pool(/*thread_num=*/4);

    constexpr int kProducers = 8;
    constexpr int kPerProducer = 2000;

    std::atomic<int> hits{0};
    std::vector<std::thread> producers;
    producers.reserve(kProducers);

    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&] {
            for (int i = 0; i < kPerProducer; ++i) {
                (void)pool.Submit([&] { hits.fetch_add(1, std::memory_order_relaxed); });
            }
        });
    }
    for (auto& t : producers) t.join();

    // Wait (bounded) for all tasks to drain.
    const auto deadline = std::chrono::steady_clock::now() + 3s;
    while (hits.load(std::memory_order_relaxed) < kProducers * kPerProducer &&
           std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(2ms);
    }
    EXPECT_EQ(hits.load(std::memory_order_relaxed), kProducers * kPerProducer);
}

TEST(ThreadPoolTest, UsesMoreThanOneWorkerThreadWhenConfigured) {
    ThreadPool pool(/*thread_num=*/4);

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> ready{0};
    std::atomic<int> done{0};
    std::promise<void> go;
    auto go_fut = go.get_future().share();

    constexpr int kTasks = 16;
    std::vector<std::future<void>> futs;
    futs.reserve(kTasks);

    std::mutex ids_mtx;
    std::unordered_set<std::thread::id> ids;

    for (int i = 0; i < kTasks; ++i) {
        futs.emplace_back(pool.Submit([&, go_fut] {
            ready.fetch_add(1, std::memory_order_relaxed);
            cv.notify_all();
            go_fut.wait();

            {
                std::lock_guard<std::mutex> lk(ids_mtx);
                ids.insert(std::this_thread::get_id());
            }
            done.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    // Ensure tasks have started and are waiting, otherwise we might not observe parallelism.
    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 1000ms, [&] { return ready.load(std::memory_order_relaxed) >= 4; }));
    }

    go.set_value();
    for (auto& f : futs) {
        assert_future_ready(f, 1500ms);
        f.get();
    }

    EXPECT_GE(done.load(std::memory_order_relaxed), kTasks);
    EXPECT_GE(ids.size(), 2u) << "Expected the pool to execute tasks on >=2 distinct worker threads.";
}

TEST(ThreadPoolTest, DestructorDoesNotDeadlockWithBlockedTasks) {
    // Use a subprocess: if ThreadPool deadlocks in destructor, we fail fast instead of hanging the whole suite.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    ASSERT_EXIT(
        {
            alarm(3);

            std::mutex mtx;
            std::condition_variable cv;
            bool go = false;

            {
                ThreadPool pool(/*thread_num=*/2);
                // Two blocked tasks: if Stop_ tries to join while holding locks incorrectly,
                // this kind of pattern often exposes it.
                auto f1 = pool.Submit([&] {
                    std::unique_lock<std::mutex> lk(mtx);
                    cv.wait(lk, [&] { return go; });
                });
                auto f2 = pool.Submit([&] {
                    std::unique_lock<std::mutex> lk(mtx);
                    cv.wait(lk, [&] { return go; });
                });

                // Let them enter waiting state.
                std::this_thread::sleep_for(30ms);

                {
                    std::lock_guard<std::mutex> lk(mtx);
                    go = true;
                }
                cv.notify_all();

                // Make sure tasks can finish before pool destructor runs.
                (void)f1.get();
                (void)f2.get();
            }

            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}
