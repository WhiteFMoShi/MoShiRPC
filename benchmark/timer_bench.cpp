#include <chrono>
#include <condition_variable>
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
#include <vector>
#include <atomic>
#include <algorithm>

#include "common/timer.hpp"

using moshi::Timer;

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
    std::cout << name << ": ops=" << ops << ", time=" << seconds << "s, rate=" << ops_s << " ops/s\n";
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

void print_jitter_us(const char* name, const Stats& s) {
    std::cout << name << " (us)"
              << ": min=" << s.min
              << " p50=" << s.p50
              << " p95=" << s.p95
              << " p99=" << s.p99
              << " max=" << s.max
              << " mean=" << s.mean
              << "\n";
}

void bench_add_only(uint64_t n, bool loop) {
    Timer timer;
    const auto t0 = clock_type::now();
    for (uint64_t i = 0; i < n; ++i) {
        timer.AddMsTask(60'000, [] {}, loop);
    }
    const auto t1 = clock_type::now();
    print_rate(loop ? "add_only(loop=true)" : "add_only(loop=false)", n, secs_since(t0, t1));
}

void bench_concurrent_add(uint64_t threads, uint64_t per_thread, bool loop) {
    Timer timer;
    std::vector<std::thread> producers;
    producers.reserve(static_cast<size_t>(threads));

    const auto t0 = clock_type::now();
    for (uint64_t t = 0; t < threads; ++t) {
        producers.emplace_back([&timer, per_thread, loop] {
            for (uint64_t i = 0; i < per_thread; ++i) {
                timer.AddMsTask(60'000, [] {}, loop);
            }
        });
    }
    for (auto& th : producers) th.join();
    const auto t1 = clock_type::now();

    const uint64_t total = threads * per_thread;
    print_rate(loop ? "concurrent_add(loop=true)" : "concurrent_add(loop=false)", total, secs_since(t0, t1));
}

void bench_execute_oneshot(uint64_t n, uint32_t delay_ms) {
    Timer timer;
    timer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    std::atomic<uint64_t> hits{0};
    std::mutex mtx;
    std::condition_variable cv;

    const auto t_add0 = clock_type::now();
    for (uint64_t i = 0; i < n; ++i) {
        timer.AddMsTask(delay_ms, [&] {
            const uint64_t v = hits.fetch_add(1, std::memory_order_relaxed) + 1;
            if (v == n) cv.notify_one();
        });
    }
    const auto t_add1 = clock_type::now();

    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return hits.load(std::memory_order_relaxed) >= n; });
    }
    const auto t_done = clock_type::now();

    timer.Stop();

    print_rate("execute_oneshot(add phase)", n, secs_since(t_add0, t_add1));
    print_rate("execute_oneshot(total)", n, secs_since(t_add0, t_done));
}

void bench_loop_hits(uint64_t target_hits, uint32_t delay_ms) {
    Timer timer;
    timer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    std::atomic<uint64_t> hits{0};
    timer.AddMsTask(delay_ms, [&] { hits.fetch_add(1, std::memory_order_relaxed); }, true);

    const auto t0 = clock_type::now();
    while (hits.load(std::memory_order_relaxed) < target_hits) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const auto t1 = clock_type::now();
    timer.Clear();
    timer.Stop();

    print_rate("loop_hits", target_hits, secs_since(t0, t1));
}

void bench_clear_cost(uint64_t n) {
    Timer timer;
    for (uint64_t i = 0; i < n; ++i) {
        timer.AddMsTask(60'000, [] {});
    }
    const auto t0 = clock_type::now();
    timer.Clear();
    const auto t1 = clock_type::now();
    print_rate("clear_cost(pop n tasks)", n, secs_since(t0, t1));
}

void bench_stop_latency_far_future() {
    Timer timer;
    timer.AddMsTask(30'000, [] {});
    timer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    const auto t0 = clock_type::now();
    timer.Stop();
    const auto t1 = clock_type::now();
    print_rate("stop_latency_far_future", 1, secs_since(t0, t1));
}

void bench_jitter_oneshot_spread(uint64_t n, uint32_t base_delay_ms, uint32_t spacing_ms) {
    // Schedules n one-shot tasks with increasing deadlines to avoid backlog effects.
    // Measures (actual_fire_time - expected_fire_time) in microseconds.
    Timer timer;
    timer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    std::vector<int64_t> lat_us;
    lat_us.resize(static_cast<size_t>(n), 0);

    std::atomic<uint64_t> done{0};
    std::mutex mtx;
    std::condition_variable cv;

    const auto t0 = clock_type::now();
    for (uint64_t i = 0; i < n; ++i) {
        const uint32_t delay_ms = base_delay_ms + static_cast<uint32_t>(i * spacing_ms);
        const auto scheduled_at = clock_type::now();
        const auto expected = scheduled_at + std::chrono::milliseconds(delay_ms);
        timer.AddMsTask(delay_ms, [&, i, expected] {
            const auto now = clock_type::now();
            const auto d = std::chrono::duration_cast<std::chrono::microseconds>(now - expected).count();
            lat_us[static_cast<size_t>(i)] = static_cast<int64_t>(d);
            const uint64_t v = done.fetch_add(1, std::memory_order_relaxed) + 1;
            if (v == n) cv.notify_one();
        });
    }

    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return done.load(std::memory_order_relaxed) >= n; });
    }
    const auto t1 = clock_type::now();

    timer.Stop();

    const auto s = compute_stats(std::move(lat_us));
    std::cout << "oneshot_jitter_spread: n=" << n
              << " base_delay_ms=" << base_delay_ms
              << " spacing_ms=" << spacing_ms
              << " total_time=" << secs_since(t0, t1) << "s\n";
    print_jitter_us("oneshot_jitter_spread", s);
}

void bench_jitter_loop_intervals(uint64_t samples, uint32_t delay_ms) {
    // Measures the interval between consecutive loop callbacks.
    Timer timer;
    timer.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    std::vector<int64_t> interval_us;
    interval_us.resize(static_cast<size_t>(samples), 0);

    std::atomic<uint64_t> idx{0};
    std::atomic<bool> has_prev{false};
    std::atomic<int64_t> prev_us{0};

    std::mutex mtx;
    std::condition_variable cv;

    const auto start = clock_type::now();
    timer.AddMsTask(delay_ms, [&] {
        const auto now = clock_type::now();
        const int64_t now_us =
            std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        if (!has_prev.exchange(true, std::memory_order_relaxed)) {
            prev_us.store(now_us, std::memory_order_relaxed);
            return;
        }

        const int64_t last = prev_us.exchange(now_us, std::memory_order_relaxed);
        const int64_t delta = now_us - last;

        const uint64_t i = idx.fetch_add(1, std::memory_order_relaxed);
        if (i < samples) {
            interval_us[static_cast<size_t>(i)] = delta;
            if (i + 1 == samples) cv.notify_one();
        }
    }, true);

    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [&] { return idx.load(std::memory_order_relaxed) >= samples; });
    }
    const auto end = clock_type::now();

    timer.Clear();
    timer.Stop();

    // Convert interval jitter into (observed_interval - expected_interval).
    const int64_t expected_us = static_cast<int64_t>(delay_ms) * 1000;
    for (auto& v : interval_us) v -= expected_us;

    const auto s = compute_stats(std::move(interval_us));
    std::cout << "loop_interval_jitter: samples=" << samples
              << " delay_ms=" << delay_ms
              << " total_time=" << secs_since(start, end) << "s\n";
    print_jitter_us("loop_interval_jitter(observed-expected)", s);
}

} // namespace

int main(int argc, char** argv) {
    // Defaults chosen to finish in a few seconds.
    int argi = 1;
    if (argc > 1 && std::string_view(argv[1]) == "--") {
        // xmake may forward this separator into argv.
        argi = 2;
    }

    uint64_t add_n = (argc > argi) ? parse_u64(argv[argi], 200'000) : 200'000;
    uint64_t exec_n = (argc > argi + 1) ? parse_u64(argv[argi + 1], 50'000) : 50'000;
    uint64_t threads = (argc > argi + 2) ? parse_u64(argv[argi + 2], 4) : 4;

    if (threads == 0) threads = 1;
    if (threads > 64) {
        std::cout << "warning: threads capped from " << threads << " to 64\n";
        threads = 64;
    }

    const uint64_t per_thread = add_n / threads;

    std::cout << "timer_bench config\n";
    std::cout << "- add_n=" << add_n << "\n";
    std::cout << "- exec_n=" << exec_n << "\n";
    std::cout << "- threads=" << threads << "\n";
    std::cout << "\n";

    bench_add_only(add_n, false);
    bench_add_only(add_n, true);
    std::cout << "\n";

    bench_concurrent_add(threads, per_thread, false);
    bench_concurrent_add(threads, per_thread, true);
    std::cout << "\n";

    bench_execute_oneshot(exec_n, /*delay_ms=*/0);
    std::cout << "\n";

    bench_loop_hits(/*target_hits=*/1'000, /*delay_ms=*/1);
    std::cout << "\n";

    // Jitter measurements (latency distribution).
    bench_jitter_oneshot_spread(/*n=*/1000, /*base_delay_ms=*/10, /*spacing_ms=*/1);
    bench_jitter_loop_intervals(/*samples=*/1000, /*delay_ms=*/2);
    std::cout << "\n";

    bench_clear_cost(add_n);
    bench_stop_latency_far_future();

    return 0;
}
