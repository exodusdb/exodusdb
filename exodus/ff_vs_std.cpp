#include <charconv>
#include <string_view>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <cstdint>

#include "fast_float/fast_float.h"  // adjust path if needed

//fast_float show x2 speed over std::from_chars on clang-20/libc++ from llvm.sh
// see ff_vs_std.cpp
//
// Compile
// c++ -std=c++26 -O3 -march=native -flto     -I/path/to/fast_float/include     -Wall -Wextra     ff_vs_std.cpp -o ff_vs_std -stdlib=libc++
/*
root@u2404bc-2:~/exodus/fast_float/include# ./ff_vs_std 
Generating 5000000 random float strings...
Total input size ≈ 75.2 MB

=== std::from_chars ===
std::from_chars:
  run 1: 487 ms
  run 2: 427 ms
  run 3: 496 ms
  run 4: 481 ms
  run 5: 398 ms
  Average: 457.8 ms  →  164.4 MB/s   10.9 Mfloat/s

=== fast_float::from_chars ===
fast_float:
  run 1: 258 ms
  run 2: 254 ms
  run 3: 231 ms
  run 4: 233 ms
  run 5: 294 ms
  Average: 254.0 ms  →  296.2 MB/s   19.7 Mfloat/s

root@u2404bc-2:~/exodus/fast_float/include# lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 24.04.3 LTS
Release:        24.04
Codename:       noble
root@u2404bc-2:~/exodus/fast_float/include# c++ --version
Ubuntu clang version 20.1.8 (++20250804090239+87f0227cb601-1~exp1~20250804210352.139)
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/lib/llvm-20/bin
root@u2404bc-2:~/exodus/fast_float/include# 
*/

using namespace std::chrono_literals;

// Utility: generate realistic-looking float strings (scientific + normal notation)
std::vector<std::string> generate_test_strings(size_t count, size_t avg_len = 12) {
    std::vector<std::string> result;
    result.reserve(count);

    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(-1e12, 1e12);
    std::uniform_int_distribution<int> exp_dist(-300, 300);
    std::uniform_int_distribution<int> prec_dist(4, 16);

    for (size_t i = 0; i < count; ++i) {
        double val = dist(rng);
        // Occasionally force scientific notation
        if (rng() % 7 == 0) {
            int exp = exp_dist(rng);
            val *= std::pow(10.0, exp);
        }
        int prec = prec_dist(rng);
        char buf[64];
        int len = std::snprintf(buf, sizeof(buf), "%.*g", prec, val);
        result.emplace_back(buf, len);
    }
    return result;
}

template<typename F>
uint64_t measure_parse_time_ms(const std::vector<std::string>& inputs, F&& parser) {
    volatile double sink = 0.0; // prevent optimization away
    auto start = std::chrono::steady_clock::now();

    for (const auto& s : inputs) {
        double value{};
        std::string_view sv{s};
        auto [p, ec] = parser(sv.data(), sv.data() + sv.size(), value);
        if (ec == std::errc()) {
            sink += value;
        }
    }

    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main() {
    constexpr size_t N = 5'000'000;       // adjust higher if you want longer runs
    constexpr int warmup_rounds = 3;
    constexpr int measure_rounds = 5;

    std::cout << "Generating " << N << " random float strings...\n";
    auto strings = generate_test_strings(N);

    // Compute total input size (for MB/s)
    size_t total_bytes = 0;
    for (const auto& s : strings) total_bytes += s.size() + 1; // +1 for realistic separator
    double total_mb = total_bytes / 1'000'000.0;

    std::cout << "Total input size ≈ " << std::fixed << std::setprecision(1)
              << total_mb << " MB\n\n";

    auto run_benchmark = [&](const char* name, auto parser_func) {
        std::cout << name << ":\n";

        // Warmup
        for (int i = 0; i < warmup_rounds; ++i) {
            measure_parse_time_ms(strings, parser_func);
        }

        std::vector<uint64_t> times_ms;
        for (int i = 0; i < measure_rounds; ++i) {
            uint64_t ms = measure_parse_time_ms(strings, parser_func);
            times_ms.push_back(ms);
            std::cout << "  run " << (i+1) << ": " << ms << " ms\n";
        }

        double avg_ms = 0;
        for (auto t : times_ms) avg_ms += t;
        avg_ms /= times_ms.size();

        double avg_sec = avg_ms / 1000.0;
        double mfloat_s = N / avg_sec / 1'000'000.0;
        double mb_s = total_mb / avg_sec;

        std::cout << "  Average: " << std::fixed << std::setprecision(1)
                  << avg_ms << " ms  →  "
                  << mb_s << " MB/s   "
                  << mfloat_s << " Mfloat/s\n\n";
    };

    std::cout << "=== std::from_chars ===\n";
    run_benchmark("std::from_chars", [](const char* b, const char* e, double& v) {
        return std::from_chars(b, e, v);
    });

    std::cout << "=== fast_float::from_chars ===\n";
    run_benchmark("fast_float", [](const char* b, const char* e, double& v) {
        return fast_float::from_chars(b, e, v);
    });

    return 0;
}
