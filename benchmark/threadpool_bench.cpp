#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <mutex>
#include <numeric>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/threadpool.hpp"

namespace {

using clock_type = std::chrono::steady_clock;

uint64_t parse_u64(std::string_view s, uint64_t def) {
    if (s.empty()) return def;
    char* end = nullptr;
    const std::string tmp(s);
    const unsigned long long v = std::strtoull(tmp.c_str(), &end, 10);
    if (!end || *end != '\0') return def;
    return static_cast<uint64_t>(v);
}

double secs_since(clock_type::time_point start, clock_type::time_point end) {
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

void print_rate(const char* name, uint64_t ops, double seconds) {
    const double ops_s = seconds > 0 ? (static_cast<double>(ops) / seconds) : 0.0;
    std::printf("%s: ops=%llu, time=%.6fs, rate=%.3f ops/s\n",
                name,
                static_cast<unsigned long long>(ops),
                seconds,
                ops_s);
}

struct Stats {
    int64_t min = 0;
    int64_t p50 = 0;
    int64_t p95 = 0;
    int64_t p99 = 0;
    int64_t max = 0;
    double mean = 0.0;
};

Stats compute_stats(std::vector<int64_t> data) {
    Stats s{};
    if (data.empty()) return s;
    std::sort(data.begin(), data.end());

    const auto idx = [&](double q) -> size_t {
        const double pos = q * (static_cast<double>(data.size() - 1));
        return static_cast<size_t>(pos + 0.5);
    };

    s.min = data.front();
    s.max = data.back();
    s.p50 = data[idx(0.50)];
    s.p95 = data[idx(0.95)];
    s.p99 = data[idx(0.99)];

    long double sum = 0;
    for (auto v : data) sum += static_cast<long double>(v);
    s.mean = static_cast<double>(sum / static_cast<long double>(data.size()));
    return s;
}

void print_us_stats(const char* name, const Stats& s) {
    std::printf("%s (us): min=%lld p50=%lld p95=%lld p99=%lld max=%lld mean=%.3f\n",
                name,
                static_cast<long long>(s.min),
                static_cast<long long>(s.p50),
                static_cast<long long>(s.p95),
                static_cast<long long>(s.p99),
                static_cast<long long>(s.max),
                s.mean);
}

// Silence noisy ThreadWorker startup prints so the benchmark output is readable.
// We keep this scoped so it won't hide benchmark prints.
struct ScopedCoutSilence {
    struct NullBuf final : std::streambuf {
        int overflow(int c) override { return c; }
    };

    std::ostream* os = &std::cout;
    std::streambuf* old = nullptr;
    NullBuf nb;

    ScopedCoutSilence() {
        old = os->rdbuf(&nb);
    }
    ~ScopedCoutSilence() {
        os->rdbuf(old);
    }
};

inline void spin_iters(uint64_t iters) {
    // Prevent being optimized away.
    volatile uint64_t x = 0;
    for (uint64_t i = 0; i < iters; ++i) x += i;
    (void)x;
}

void bench_submit_and_wait_cv(uint64_t n, uint64_t worker_threads, uint64_t work_iters) {
    ScopedCoutSilence silence;
    ThreadPool pool(static_cast<int>(worker_threads));
    (void)silence;

    std::atomic<uint64_t> done{0};
    std::mutex mtx;
    std::condition_variable cv;

    const auto t0 = clock_type::now();
    for (uint64_t i = 0; i < n; ++i) {
        (void)pool.Submit([&] {
            if (work_iters) spin_iters(work_iters);
            const uint64_t v = done.fetch_add(1, std::memory_order_relaxed) + 1;
            if (v == n) cv.notify_one();
        });
    }

    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return done.load(std::memory_order_relaxed) >= n; });
    }
    const auto t1 = clock_type::now();

    print_rate("submit_and_wait_cv", n, secs_since(t0, t1));
}

void bench_submit_and_wait_futures(uint64_t n, uint64_t worker_threads, uint64_t work_iters) {
    ScopedCoutSilence silence;
    ThreadPool pool(static_cast<int>(worker_threads));
    (void)silence;

    std::vector<std::future<void>> futs;
    futs.reserve(static_cast<size_t>(n));

    const auto t0 = clock_type::now();
    for (uint64_t i = 0; i < n; ++i) {
        futs.emplace_back(pool.Submit([=] {
            if (work_iters) spin_iters(work_iters);
        }));
    }
    for (auto& f : futs) f.get();
    const auto t1 = clock_type::now();

    print_rate("submit_and_wait_futures", n, secs_since(t0, t1));
}

void bench_concurrent_submit(uint64_t producers,
                             uint64_t per_producer,
                             uint64_t worker_threads,
                             uint64_t work_iters) {
    ScopedCoutSilence silence;
    ThreadPool pool(static_cast<int>(worker_threads));
    (void)silence;

    const uint64_t total = producers * per_producer;
    std::atomic<uint64_t> done{0};
    std::mutex mtx;
    std::condition_variable cv;

    std::vector<std::thread> ths;
    ths.reserve(static_cast<size_t>(producers));

    const auto t0 = clock_type::now();
    for (uint64_t p = 0; p < producers; ++p) {
        ths.emplace_back([&] {
            for (uint64_t i = 0; i < per_producer; ++i) {
                (void)pool.Submit([&] {
                    if (work_iters) spin_iters(work_iters);
                    const uint64_t v = done.fetch_add(1, std::memory_order_relaxed) + 1;
                    if (v == total) cv.notify_one();
                });
            }
        });
    }
    for (auto& t : ths) t.join();

    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return done.load(std::memory_order_relaxed) >= total; });
    }
    const auto t1 = clock_type::now();

    print_rate("concurrent_submit_total", total, secs_since(t0, t1));
}

void bench_queue_delay(uint64_t samples, uint64_t worker_threads, uint64_t work_iters) {
    // Measures queue delay from "just before Submit" to "task begins executing".
    ScopedCoutSilence silence;
    ThreadPool pool(static_cast<int>(worker_threads));
    (void)silence;

    std::vector<int64_t> delay_us;
    delay_us.resize(static_cast<size_t>(samples), 0);

    std::atomic<uint64_t> idx{0};
    std::atomic<uint64_t> done{0};
    std::mutex mtx;
    std::condition_variable cv;

    const auto t0 = clock_type::now();
    for (uint64_t i = 0; i < samples; ++i) {
        const auto submitted_at = clock_type::now();
        (void)pool.Submit([&, submitted_at] {
            const auto started = clock_type::now();
            const int64_t d =
                std::chrono::duration_cast<std::chrono::microseconds>(started - submitted_at).count();

            const uint64_t slot = idx.fetch_add(1, std::memory_order_relaxed);
            if (slot < samples) {
                delay_us[static_cast<size_t>(slot)] = d;
            }

            if (work_iters) spin_iters(work_iters);

            const uint64_t v = done.fetch_add(1, std::memory_order_relaxed) + 1;
            if (v == samples) cv.notify_one();
        });
    }

    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return done.load(std::memory_order_relaxed) >= samples; });
    }
    const auto t1 = clock_type::now();

    const auto s = compute_stats(std::move(delay_us));
    std::printf("queue_delay: samples=%llu workers=%llu total_time=%.6fs\n",
                static_cast<unsigned long long>(samples),
                static_cast<unsigned long long>(worker_threads),
                secs_since(t0, t1));
    print_us_stats("queue_delay", s);
}

void bench_parallelism_sanity(uint64_t worker_threads) {
    // A sanity check: are we actually using multiple worker threads?
    ScopedCoutSilence silence;
    ThreadPool pool(static_cast<int>(worker_threads));
    (void)silence;

    std::mutex ids_mtx;
    std::unordered_set<std::thread::id> ids;

    std::promise<void> go;
    const auto go_fut = go.get_future().share();

    constexpr int kTasks = 32;
    std::vector<std::future<void>> futs;
    futs.reserve(kTasks);

    for (int i = 0; i < kTasks; ++i) {
        futs.emplace_back(pool.Submit([&, go_fut] {
            go_fut.wait();
            std::lock_guard<std::mutex> lk(ids_mtx);
            ids.insert(std::this_thread::get_id());
        }));
    }
    go.set_value();
    for (auto& f : futs) f.get();

    std::printf("parallelism_threads_observed: %llu (workers=%llu)\n",
                static_cast<unsigned long long>(ids.size()),
                static_cast<unsigned long long>(worker_threads));
}

} // namespace

int main(int argc, char** argv) {
    // Defaults chosen to finish in a few seconds.
    int argi = 1;
    if (argc > 1 && std::string_view(argv[1]) == "--") {
        // xmake may forward this separator into argv.
        argi = 2;
    }

    const uint64_t workers = (argc > argi) ? parse_u64(argv[argi], 4) : 4;
    const uint64_t n = (argc > argi + 1) ? parse_u64(argv[argi + 1], 200'000) : 200'000;
    const uint64_t producers = (argc > argi + 2) ? parse_u64(argv[argi + 2], 4) : 4;
    const uint64_t work_iters = (argc > argi + 3) ? parse_u64(argv[argi + 3], 0) : 0;

    const uint64_t per_producer = (producers > 0) ? (n / producers) : n;
    const uint64_t total = (producers > 0) ? (producers * per_producer) : n;

    std::printf("threadpool_bench config\n");
    std::printf("- workers=%llu\n", static_cast<unsigned long long>(workers));
    std::printf("- tasks_n=%llu\n", static_cast<unsigned long long>(n));
    std::printf("- producers=%llu\n", static_cast<unsigned long long>(producers));
    std::printf("- per_producer=%llu\n", static_cast<unsigned long long>(per_producer));
    std::printf("- total(submit)=%llu\n", static_cast<unsigned long long>(total));
    std::printf("- work_iters=%llu\n", static_cast<unsigned long long>(work_iters));
    std::printf("\n");

    // Fast path: minimal work; end-to-end throughput with CV completion.
    bench_submit_and_wait_cv(n, workers, work_iters);
    // Same but with futures to measure the overhead of keeping futures around.
    bench_submit_and_wait_futures(std::min<uint64_t>(n, 50'000), workers, work_iters);
    std::printf("\n");

    // Contention: concurrent Submit from multiple producer threads.
    bench_concurrent_submit(producers, per_producer, workers, work_iters);
    std::printf("\n");

    // Queueing delay distribution (microseconds).
    bench_queue_delay(/*samples=*/1000, workers, work_iters);
    std::printf("\n");

    bench_parallelism_sanity(workers);
    return 0;
}
