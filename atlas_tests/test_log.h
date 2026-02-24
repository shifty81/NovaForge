#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <iomanip>

namespace atlas::test {

struct TestResult {
    std::string name;
    std::string section;
    bool passed = false;
    double durationMs = 0.0;
    std::string failureMessage;
};

class TestLog {
public:
    static TestLog& Instance() {
        static TestLog inst;
        return inst;
    }

    void BeginSection(const std::string& name) {
        m_currentSection = name;
        std::cout << "\n--- " << name << " ---" << std::endl;
    }

    void RecordPass(const std::string& testName, double durationMs) {
        m_results.push_back({testName, m_currentSection, true, durationMs, ""});
        ++m_passed;
        std::cout << "[PASS] " << testName;
        if (durationMs >= 1.0) {
            std::cout << " (" << std::fixed << std::setprecision(1) << durationMs << " ms)";
        }
        std::cout << std::endl;
    }

    void RecordFail(const std::string& testName, const std::string& message, double durationMs) {
        m_results.push_back({testName, m_currentSection, false, durationMs, message});
        ++m_failed;
        std::cout << "[FAIL] " << testName << " — " << message << std::endl;
    }

    void RecordSkip(const std::string& testName, const std::string& reason) {
        m_results.push_back({testName, m_currentSection, true, 0.0, ""});
        ++m_skipped;
        std::cout << "[SKIP] " << testName << " — " << reason << std::endl;
    }

    int PrintSummary() const {
        double totalMs = 0.0;
        for (auto& r : m_results) totalMs += r.durationMs;

        std::cout << "\n======================================" << std::endl;
        std::cout << "  Test Summary" << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << "  Passed:  " << m_passed << std::endl;
        std::cout << "  Failed:  " << m_failed << std::endl;
        std::cout << "  Skipped: " << m_skipped << std::endl;
        std::cout << "  Total:   " << (m_passed + m_failed + m_skipped) << std::endl;
        std::cout << "  Time:    " << std::fixed << std::setprecision(1) << totalMs << " ms" << std::endl;
        std::cout << "======================================" << std::endl;

        if (m_failed > 0) {
            std::cout << "\nFailed tests:" << std::endl;
            for (auto& r : m_results) {
                if (!r.passed) {
                    std::cout << "  [FAIL] " << r.name << " — " << r.failureMessage << std::endl;
                }
            }
        }

        if (m_failed == 0) {
            std::cout << "\n=== All tests passed! ===" << std::endl;
        } else {
            std::cout << "\n=== " << m_failed << " test(s) FAILED ===" << std::endl;
        }

        return m_failed > 0 ? 1 : 0;
    }

    void WriteLogFile(const std::string& path) const {
        std::ofstream ofs(path);
        if (!ofs) return;

        double totalMs = 0.0;
        for (auto& r : m_results) totalMs += r.durationMs;

        ofs << "Atlas Engine Test Results" << std::endl;
        ofs << "========================" << std::endl;
        ofs << "Passed: " << m_passed << "  Failed: " << m_failed
            << "  Skipped: " << m_skipped
            << "  Total: " << (m_passed + m_failed + m_skipped) << std::endl;
        ofs << "Time: " << std::fixed << std::setprecision(1) << totalMs << " ms" << std::endl;
        ofs << std::endl;

        std::string lastSection;
        for (auto& r : m_results) {
            if (r.section != lastSection) {
                ofs << "\n--- " << r.section << " ---" << std::endl;
                lastSection = r.section;
            }
            if (r.passed) {
                ofs << "[PASS] " << r.name << std::endl;
            } else {
                ofs << "[FAIL] " << r.name << " — " << r.failureMessage << std::endl;
            }
        }
    }

    static void WaitOnWindows() {
#ifdef _WIN32
        std::cout << "\nPress Enter to close..." << std::endl;
        std::cin.get();
#endif
    }

    int Passed() const { return m_passed; }
    int Failed() const { return m_failed; }
    int Skipped() const { return m_skipped; }
    const std::vector<TestResult>& Results() const { return m_results; }

private:
    TestLog() = default;
    std::vector<TestResult> m_results;
    std::string m_currentSection;
    int m_passed = 0;
    int m_failed = 0;
    int m_skipped = 0;
};

class TestTimer {
public:
    TestTimer() : m_start(std::chrono::high_resolution_clock::now()) {}
    double ElapsedMs() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - m_start).count();
    }
private:
    std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace atlas::test

#define ATLAS_TEST_BEGIN(name) \
    { atlas::test::TestTimer _timer; \
      const char* _testName = name; \
      try {

#define ATLAS_TEST_END() \
        atlas::test::TestLog::Instance().RecordPass(_testName, _timer.ElapsedMs()); \
      } catch (const std::exception& e) { \
        atlas::test::TestLog::Instance().RecordFail(_testName, e.what(), _timer.ElapsedMs()); \
      } catch (...) { \
        atlas::test::TestLog::Instance().RecordFail(_testName, "Unknown exception", _timer.ElapsedMs()); \
      } \
    }

#define ATLAS_ASSERT(expr) \
    do { if (!(expr)) throw std::runtime_error("Assertion failed: " #expr); } while(0)

#define ATLAS_ASSERT_MSG(expr, msg) \
    do { if (!(expr)) throw std::runtime_error(std::string("Assertion failed: ") + msg); } while(0)

// Wrap an existing test function call: records pass/fail and timing
// The wrapped function may still use raw assert() — if it aborts, the process
// terminates before RecordFail can fire.  For graceful failure handling,
// prefer ATLAS_ASSERT inside ATLAS_TEST_BEGIN/END blocks.
#define RUN_TEST(fn) \
    { atlas::test::TestTimer _t; \
      try { fn(); \
        atlas::test::TestLog::Instance().RecordPass(#fn, _t.ElapsedMs()); \
      } catch (const std::exception& e) { \
        atlas::test::TestLog::Instance().RecordFail(#fn, e.what(), _t.ElapsedMs()); \
      } catch (...) { \
        atlas::test::TestLog::Instance().RecordFail(#fn, "Unknown exception", _t.ElapsedMs()); \
      } \
    }
