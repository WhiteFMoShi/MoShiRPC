#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <unistd.h> // alarm, _exit

#include "common/timer.hpp"

using moshi::Timer;
using moshi::Alarm;
using namespace std::chrono_literals;

static_assert(std::is_same_v<decltype(std::declval<const Alarm&>().GetID()), uint64_t>,
              "Alarm::GetID() must return uint64_t to avoid truncation under load.");
static_assert(std::is_constructible_v<Alarm,
                                     std::chrono::time_point<std::chrono::steady_clock>,
                                     std::function<void()>,
                                     std::chrono::milliseconds,
                                     uint64_t>,
              "Alarm must be constructible with a uint64_t id.");

TEST(TimerTest, OneShotTaskRuns) {
    Timer timer;
    timer.Start();

    std::atomic<int> hits{0};
    std::mutex mtx;
    std::condition_variable cv;

    timer.AddMsTask(30, [&] {
        hits.fetch_add(1, std::memory_order_relaxed);
        cv.notify_all();
    });

    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 500ms, [&] { return hits.load(std::memory_order_relaxed) >= 1; }));
    }
    timer.Stop();
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1);
}

TEST(TimerTest, LoopTaskRunsMultipleTimes) {
    Timer timer;
    timer.Start();

    std::atomic<int> hits{0};
    std::mutex mtx;
    std::condition_variable cv;

    timer.AddMsTask(10, [&] {
        hits.fetch_add(1, std::memory_order_relaxed);
        cv.notify_all();
    }, true);

    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 400ms, [&] { return hits.load(std::memory_order_relaxed) >= 3; }));
    }
    timer.Stop();
    EXPECT_GE(hits.load(std::memory_order_relaxed), 3);
}

TEST(TimerTest, ClearCancelsPendingCallbacks) {
    Timer timer;
    timer.Start();

    std::atomic<int> hits{0};
    timer.AddMsTask(200, [&] { hits.fetch_add(1, std::memory_order_relaxed); });
    timer.AddMsTask(200, [&] { hits.fetch_add(1, std::memory_order_relaxed); });

    timer.Clear();
    std::this_thread::sleep_for(350ms);
    timer.Stop();
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 0);
}

TEST(TimerTest, StopDoesNotHang) {
    // Run in a subprocess with an alarm so a deadlock turns into a failing test,
    // instead of hanging the entire test runner.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    ASSERT_EXIT(
        {
            alarm(2);
            Timer timer;
            timer.Start();
            timer.AddMsTask(1, [] {}, true);
            std::this_thread::sleep_for(30ms);
            timer.Stop();
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, TasksAddedBeforeStartRunAfterStart) {
    Timer timer;

    std::atomic<int> hits{0};
    std::mutex mtx;
    std::condition_variable cv;

    timer.AddMsTask(10, [&] {
        hits.fetch_add(1, std::memory_order_relaxed);
        cv.notify_all();
    });

    timer.Start();
    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 600ms, [&] { return hits.load(std::memory_order_relaxed) >= 1; }));
    }
    timer.Stop();
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1);
}

TEST(TimerTest, ConcurrentAddMsTaskAllRun) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    // This is a behavioral contract test we want from the public API:
    // calling AddMsTask concurrently should not deadlock and tasks should execute.
    ASSERT_EXIT(
        {
            alarm(4);

            Timer timer;
            timer.Start();

            constexpr int kThreads = 8;
            constexpr int kTasksPerThread = 50;
            constexpr int kExpected = kThreads * kTasksPerThread;

            std::atomic<int> hits{0};
            std::mutex mtx;
            std::condition_variable cv;

            std::vector<std::thread> producers;
            producers.reserve(kThreads);
            for (int t = 0; t < kThreads; ++t) {
                producers.emplace_back([&, t] {
                    for (int i = 0; i < kTasksPerThread; ++i) {
                        // Spread deadlines a bit to avoid a thundering herd on a single timepoint.
                        const uint32_t delay_ms = static_cast<uint32_t>(1 + (t + i) % 17);
                        timer.AddMsTask(delay_ms, [&] {
                            hits.fetch_add(1, std::memory_order_relaxed);
                            cv.notify_all();
                        });
                    }
                });
            }
            for (auto& th : producers) th.join();

            const auto deadline = std::chrono::steady_clock::now() + 2s;
            {
                std::unique_lock<std::mutex> lk(mtx);
                while (hits.load(std::memory_order_relaxed) < kExpected) {
                    if (std::chrono::steady_clock::now() >= deadline) break;
                    cv.wait_for(lk, 50ms);
                }
            }

            timer.Stop();

            const int got = hits.load(std::memory_order_relaxed);
            if (got != kExpected) _exit(1);
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, StopPreventsFurtherCallbacksAfterReturn) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    ASSERT_EXIT(
        {
            alarm(4);

            Timer timer;
            timer.Start();

            std::atomic<int> hits{0};
            timer.AddMsTask(1, [&] { hits.fetch_add(1, std::memory_order_relaxed); }, true);

            // Ensure the loop callback has run at least a few times before stopping.
            const auto warmup_deadline = std::chrono::steady_clock::now() + 600ms;
            while (hits.load(std::memory_order_relaxed) < 3 &&
                   std::chrono::steady_clock::now() < warmup_deadline) {
                std::this_thread::sleep_for(5ms);
            }

            timer.Stop();
            const int after_stop = hits.load(std::memory_order_relaxed);

            std::this_thread::sleep_for(100ms);
            const int later = hits.load(std::memory_order_relaxed);

            if (later != after_stop) _exit(1);
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, StopCancelsPendingCallbacks) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    // If Stop() returns, no callbacks are allowed to run afterwards, even if they were scheduled.
    ASSERT_EXIT(
        {
            alarm(4);

            Timer timer;
            timer.Start();

            std::atomic<int> hits{0};
            timer.AddMsTask(1500, [&] { hits.fetch_add(1, std::memory_order_relaxed); });

            timer.Stop();
            std::this_thread::sleep_for(200ms);

            if (hits.load(std::memory_order_relaxed) != 0) _exit(1);
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, ClearIsThreadSafeWithConcurrentAdd) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    // Clear() is allowed concurrently with Start() and AddMsTask(). It should not deadlock and
    // should not cause cancelled tasks to execute.
    ASSERT_EXIT(
        {
            alarm(4);

            Timer timer;
            timer.Start();

            constexpr int kThreads = 6;
            constexpr int kTasksPerThread = 60;
            constexpr int kTotal = kThreads * kTasksPerThread;

            std::atomic<int> hits{0};
            std::atomic<int> duplicates{0};

            auto executed = std::make_unique<std::atomic<int>[]>(kTotal);
            for (int i = 0; i < kTotal; ++i) executed[i].store(0, std::memory_order_relaxed);

            std::atomic<bool> producers_done{false};
            std::thread clearer([&] {
                while (!producers_done.load(std::memory_order_relaxed)) {
                    timer.Clear();
                    std::this_thread::sleep_for(2ms);
                }
            });

            std::vector<std::thread> producers;
            producers.reserve(kThreads);
            for (int t = 0; t < kThreads; ++t) {
                producers.emplace_back([&, t] {
                    for (int i = 0; i < kTasksPerThread; ++i) {
                        const int id = t * kTasksPerThread + i;
                        timer.AddMsTask(
                            10'000,
                            [&, id] {
                                if (executed[id].fetch_add(1, std::memory_order_relaxed) != 0) {
                                    duplicates.fetch_add(1, std::memory_order_relaxed);
                                }
                                hits.fetch_add(1, std::memory_order_relaxed);
                            });
                    }
                });
            }
            for (auto& th : producers) th.join();
            producers_done.store(true, std::memory_order_relaxed);
            clearer.join();

            // Final drain for anything added after the last Clear() iteration.
            timer.Clear();
            timer.Stop();

            if (hits.load(std::memory_order_relaxed) != 0) _exit(1);
            if (duplicates.load(std::memory_order_relaxed) != 0) _exit(1);
            for (int i = 0; i < kTotal; ++i) {
                if (executed[i].load(std::memory_order_relaxed) != 0) _exit(1);
            }
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, StopReturnsPromptlyEvenWithFarFutureTask) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    // Contract: Stop() should return promptly even if the next scheduled task is far in the future.
    // If Stop() doesn't wake the timer thread out of wait_until(), join() may block for a long time.
    ASSERT_EXIT(
        {
            alarm(2);

            Timer timer;
            timer.AddMsTask(30'000, [] {});
            timer.Start();

            std::this_thread::sleep_for(20ms);
            timer.Stop();
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, StartStopStartWorks) {
    Timer timer;

    std::atomic<int> hits{0};
    std::mutex mtx;
    std::condition_variable cv;

    timer.Start();
    timer.AddMsTask(20, [&] {
        hits.fetch_add(1, std::memory_order_relaxed);
        cv.notify_all();
    });

    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 600ms, [&] { return hits.load(std::memory_order_relaxed) >= 1; }));
    }
    timer.Stop();

    timer.Start();
    timer.AddMsTask(20, [&] {
        hits.fetch_add(1, std::memory_order_relaxed);
        cv.notify_all();
    });
    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 600ms, [&] { return hits.load(std::memory_order_relaxed) >= 2; }));
    }
    timer.Stop();
}

TEST(TimerTest, ClearCancelsLoopTaskEvenIfCallbackInFlight) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    // Strict contract: Clear() cancels pending callbacks, including future repeats of loop tasks.
    // This test makes the loop callback "in flight" while Clear() is called, which is a common
    // race in real systems.
    ASSERT_EXIT(
        {
            alarm(3);

            Timer timer;
            timer.Start();

            std::atomic<int> hits{0};
            std::atomic<bool> entered{false};
            std::atomic<bool> release{false};

            timer.AddMsTask(
                1,
                [&] {
                    hits.fetch_add(1, std::memory_order_relaxed);
                    entered.store(true, std::memory_order_relaxed);
                    while (!release.load(std::memory_order_relaxed)) {
                        std::this_thread::sleep_for(1ms);
                    }
                },
                true);

            // Wait until the first callback is definitely running (blocked on `release`).
            const auto deadline = std::chrono::steady_clock::now() + 400ms;
            while (!entered.load(std::memory_order_relaxed) &&
                   std::chrono::steady_clock::now() < deadline) {
                std::this_thread::sleep_for(1ms);
            }
            if (!entered.load(std::memory_order_relaxed)) _exit(1);

            timer.Clear(); // should cancel future loop repeats
            const int after_clear = hits.load(std::memory_order_relaxed);

            // Let the in-flight callback return; if the implementation re-enqueues loop tasks
            // unconditionally, we will see more hits after Clear().
            release.store(true, std::memory_order_relaxed);
            std::this_thread::sleep_for(80ms);

            timer.Stop();
            const int later = hits.load(std::memory_order_relaxed);
            if (later != after_clear) _exit(1);
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}

TEST(TimerTest, StopIsIdempotent) {
    Timer timer;
    timer.Start();
    timer.AddMsTask(10, [] {});
    std::this_thread::sleep_for(10ms);
    timer.Stop();
    timer.Stop();
}

TEST(TimerTest, AddAfterStopRunsAfterRestart) {
    Timer timer;
    timer.Start();
    timer.Stop();

    std::atomic<int> hits{0};
    std::mutex mtx;
    std::condition_variable cv;

    timer.AddMsTask(10, [&] {
        hits.fetch_add(1, std::memory_order_relaxed);
        cv.notify_all();
    });
    timer.Start();

    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 600ms, [&] { return hits.load(std::memory_order_relaxed) >= 1; }));
    }
    timer.Stop();
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1);
}

TEST(TimerTest, MultipleTimersClearIsolation) {
    Timer a;
    Timer b;
    a.Start();
    b.Start();

    std::atomic<int> hits_a{0};
    std::atomic<int> hits_b{0};
    std::mutex mtx;
    std::condition_variable cv;

    a.AddMsTask(5, [&] { hits_a.fetch_add(1, std::memory_order_relaxed); cv.notify_all(); }, true);
    b.AddMsTask(5, [&] { hits_b.fetch_add(1, std::memory_order_relaxed); cv.notify_all(); }, true);

    {
        std::unique_lock<std::mutex> lk(mtx);
        ASSERT_TRUE(cv.wait_for(lk, 600ms, [&] {
            return hits_a.load(std::memory_order_relaxed) >= 3 &&
                   hits_b.load(std::memory_order_relaxed) >= 3;
        }));
    }

    a.Clear();
    const int a_after_clear = hits_a.load(std::memory_order_relaxed);
    const int b_after_clear = hits_b.load(std::memory_order_relaxed);

    std::this_thread::sleep_for(80ms);

    // a should stop producing new callbacks, b should keep running.
    EXPECT_EQ(hits_a.load(std::memory_order_relaxed), a_after_clear);
    EXPECT_GT(hits_b.load(std::memory_order_relaxed), b_after_clear);

    a.Stop();
    b.Stop();
}

TEST(TimerTest, ConcurrentAddClearStopStressNoHang) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    ASSERT_EXIT(
        {
            alarm(5);

            Timer timer;
            timer.Start();

            std::atomic<bool> done{false};
            std::vector<std::thread> threads;

            // Producers: mix loop and one-shot tasks.
            for (int t = 0; t < 4; ++t) {
                threads.emplace_back([&] {
                    while (!done.load(std::memory_order_relaxed)) {
                        timer.AddMsTask(1, [] {});
                        timer.AddMsTask(2, [] {}, true);
                    }
                });
            }

            // Concurrent clearer.
            threads.emplace_back([&] {
                for (int i = 0; i < 200; ++i) {
                    timer.Clear();
                    std::this_thread::sleep_for(1ms);
                }
            });

            std::this_thread::sleep_for(80ms);
            done.store(true, std::memory_order_relaxed);
            for (auto& th : threads) th.join();

            timer.Stop();
            _exit(0);
        },
        ::testing::ExitedWithCode(0),
        "");
}
