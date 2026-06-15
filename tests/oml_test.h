// oml_test.h - a tiny, dependency-free test foundation for OpenMachineLab.
// oml_test.h - OpenMachineLab 的轻量无依赖测试基座。
//
// One base class (TestBase) shared by functional, performance, and safety tests
// so they grow together. It provides:
//   - Check / CheckEq / Invariant  : assertions (written to stderr)
//   - Benchmark / ReportPerf       : time a body over N iterations
//   - SilentLog                    : suppress the model's trace so perf numbers
//                                    reflect CPU, not console I/O
//   - RunConcurrently              : spawn N threads — scaffolding for the
//                                    concurrency-safety tests (RoadMap Phase 3)
// 一个基类（TestBase）被功能/性能/安全测试共用，使其共同成长。提供：
//   - Check / CheckEq / Invariant  : 断言（输出到 stderr）
//   - Benchmark / ReportPerf       : 对 body 计时 N 次迭代
//   - SilentLog                    : 屏蔽模型 trace，使性能数字只反映 CPU 而非 I/O
//   - RunConcurrently              : 起 N 个线程——并发安全测试的脚手架
//
// Test output goes to stderr; the model's trace goes to stdout. That keeps
// programmatic checks readable even when a test drives the model.
// 测试输出到 stderr；模型 trace 到 stdout。使程序化断言在驱动模型时仍清晰可读。
#pragma once

#include "log/Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace oml::test {

// Result of a Benchmark run.
// Benchmark 运行结果。
struct PerfResult {
    int    iterations = 0;
    double avg_ns     = 0;
    double min_ns     = 0;
    double max_ns     = 0;
    double per_second = 0; // throughput 吞吐量
};

// Base class for every test. Subclasses implement Name() and Run().
// 每个测试的基类。子类实现 Name() 和 Run()。
class TestBase {
public:
    virtual ~TestBase() = default;

    virtual std::string Name() const = 0;
    virtual void        Run()        = 0;

    // --- assertions (stderr) ----------------------------------------------
    // --- 断言（stderr）-----------------------------------------------------
    void Check(bool cond, const std::string& msg) {
        ++checks_;
        if (cond) {
            std::cerr << "    ok:   " << msg << "\n";
        } else {
            std::cerr << "    FAIL: " << msg << "\n";
            ++failures_;
        }
    }

    template <class A, class B>
    void CheckEq(const A& a, const B& b, const std::string& msg) { Check(a == b, msg); }

    // A condition that must always hold. Same mechanic as Check, labelled so
    // safety regressions stand out.
    // 必须始终成立的条件。与 Check 机制相同，标注 [invariant] 使安全回归更醒目。
    void Invariant(bool cond, const std::string& msg) {
        Check(cond, std::string("[invariant] ") + msg);
    }

    // --- performance ------------------------------------------------------
    // Time `body` over `iterations` runs (one untimed warmup first). Does not
    // print — call ReportPerf with the result.
    // 对 `body` 计时 `iterations` 次（先一次不计时的预热）。不打印——
    // 用结果调用 ReportPerf。
    template <class Body>
    PerfResult Benchmark(int iterations, Body body) {
        body(); // warmup 预热
        double min = 1e30, max = 0, sum = 0;
        for (int i = 0; i < iterations; ++i) {
            const auto t0 = std::chrono::steady_clock::now();
            body();
            const auto t1 = std::chrono::steady_clock::now();
            const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
            if (ns < min) min = ns;
            if (ns > max) max = ns;
            sum += ns;
        }
        const double avg = sum / iterations;
        return { iterations, avg, min, max, avg > 0 ? 1e9 / avg : 0.0 };
    }

    void ReportPerf(const std::string& label, const PerfResult& r) {
        std::ostringstream os;
        os << std::fixed << std::setprecision(1);
        os << "    perf: " << label << "  avg " << r.avg_ns << " ns"
           << "  min " << r.min_ns << " ns  max " << r.max_ns << " ns"
           << "  " << r.per_second << " /s  (x" << r.iterations << ")";
        std::cerr << os.str() << "\n";
    }

    // --- concurrency scaffolding (Phase 3 safety tests) -------------------
    // Spawn `n` threads all running `fn`, then join. Returns when all finish.
    // 起 `n` 个线程都运行 `fn`，然后 join。全部结束后返回。
    template <class Fn>
    void RunConcurrently(int n, Fn fn) {
        std::vector<std::thread> ts;
        ts.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) ts.emplace_back(fn);
        for (auto& t : ts) t.join();
    }

    int Failures() const { return failures_; }

private:
    int checks_   = 0;
    int failures_ = 0;
};

// Silence the model's trace for the lifetime of this object. The model writes
// via oml::Log(); point its sink at nothing so perf measurements are not
// distorted by I/O and so test assertions (on stderr) stay readable.
// 在此对象的生命周期内屏蔽模型 trace。模型通过 oml::Log() 输出；
// 将 sink 设为 null 使性能测量不被 I/O 扭曲，并使测试断言（stderr）清晰可读。
class SilentLog {
public:
    SilentLog() : saved_(oml::Log().Sink()) { oml::Log().SetSink(nullptr); }
    ~SilentLog() { oml::Log().SetSink(saved_); }
    SilentLog(const SilentLog&)            = delete;
    SilentLog& operator=(const SilentLog&) = delete;

private:
    std::ostream* saved_;
};

// Run a suite. Accepts unique_ptr<TestBase> arguments directly (a variadic
// template, not initializer_list, so the move-only unique_ptrs are not copied).
// 运行测试套件。直接接受 unique_ptr<TestBase> 参数（变参模板，非 initializer_list，
// 使 move-only 的 unique_ptr 不被拷贝）。
template <class... Tests>
int RunAll(Tests... tests) {
    std::vector<std::unique_ptr<TestBase>> suite;
    (suite.emplace_back(std::move(tests)), ...);

    int total = 0;
    for (auto& t : suite) {
        std::cerr << "\n[" << t->Name() << "]\n";
        t->Run();
        total += t->Failures();
    }
    if (total == 0) {
        std::cerr << "\nALL TESTS PASSED\n";
        return 0;
    }
    std::cerr << "\n" << total << " CHECK(S) FAILED\n";
    return 1;
}

} // namespace oml::test
