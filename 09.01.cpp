#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 09.01
// Description: RAII tracer for function entry and exit with std::source_location.

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <source_location>

class Tracer {
public:
    explicit Tracer(const std::source_location location = std::source_location::current())
        : location_(location) {
        std::cout << "enter " << location_.function_name() << " (" << location_.file_name() << ':'
                  << location_.line() << ':' << location_.column() << ")\n";
    }

    ~Tracer() {
        std::cout << "leave " << location_.function_name() << " (" << location_.file_name() << ':'
                  << location_.line() << ':' << location_.column() << ")\n";
    }

private:
    std::source_location location_;
};

#ifndef NDEBUG
#define trace() Tracer tracer_instance_##__LINE__ {}
#else
#define trace() ((void)0)
#endif

namespace tests {

bool contains(const std::string& text, const std::string& pattern) {
    return text.find(pattern) != std::string::npos;
}

struct Logger {
    int passed = 0;
    int failed = 0;

    void require(bool condition, const char* message) {
        if (condition) {
            ++passed;
            std::cout << "[OK]   " << message << '\n';
        } else {
            ++failed;
            std::cout << "[FAIL] " << message << '\n';
        }
    }
};

class CoutCapture {
public:
    CoutCapture() : old_buffer_(std::cout.rdbuf(buffer_.rdbuf())) {
    }

    ~CoutCapture() {
        std::cout.rdbuf(old_buffer_);
    }

    [[nodiscard]] std::string str() const {
        return buffer_.str();
    }

private:
    std::ostringstream buffer_;
    std::streambuf* old_buffer_;
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void direct_trace_scope() {
    trace();
}

void nested_inner_trace_scope() {
    trace();
}

void nested_outer_trace_scope() {
    trace();
    nested_inner_trace_scope();
}

void test_direct_construction(Logger& log) {
    CoutCapture capture;

    { Tracer tracer; }

    const std::string output = capture.str();
    require(output.find("enter ") != std::string::npos, "expected enter message");
    require(output.find("leave ") != std::string::npos, "expected leave message");
    require(output.find("test_direct_construction") != std::string::npos ||
                output.find("tests::test_direct_construction") != std::string::npos,
            "expected current function name");

    log.require(true, "constructor and destructor print paired messages");
}

void test_trace_macro(Logger& log) {
    CoutCapture capture;

    direct_trace_scope();

    const std::string output = capture.str();
#ifndef NDEBUG
    require(output.find("direct_trace_scope") != std::string::npos, "expected traced function name");
    require(output.find("enter ") < output.find("leave "), "expected enter before leave");
#else
    require(output.empty(), "expected no output when NDEBUG is defined");
#endif

    log.require(true, "trace macro follows current build mode");
}

void test_nested_scopes(Logger& log) {
    CoutCapture capture;

    nested_outer_trace_scope();

    const std::string output = capture.str();
#ifndef NDEBUG
    const std::size_t enter_outer = output.find("enter ");
    const std::size_t enter_inner = output.find("enter ", enter_outer + 1);
    const std::size_t leave_inner = output.find("leave ");
    const std::size_t leave_outer = output.find("leave ", leave_inner + 1);

    require(contains(output, "nested_outer_trace_scope"), "expected outer function name");
    require(contains(output, "nested_inner_trace_scope"), "expected inner function name");
    require(enter_outer != std::string::npos, "expected first enter");
    require(enter_inner != std::string::npos, "expected second enter");
    require(leave_inner != std::string::npos, "expected first leave");
    require(leave_outer != std::string::npos, "expected second leave");
    require(enter_outer < enter_inner, "expected outer enter before inner enter");
    require(enter_inner < leave_inner, "expected inner enter before inner leave");
    require(leave_inner < leave_outer, "expected inner leave before outer leave");
#else
    require(output.empty(), "expected no tracing output when disabled");
#endif

    log.require(true, "RAII preserves nested entry and exit order");
}

void run_all() {
    Logger log;

    try {
        test_direct_construction(log);
        test_trace_macro(log);
        test_nested_scopes(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

void demo_leaf() {
    trace();
}

void demo_middle() {
    trace();
    demo_leaf();
}

void demo_root() {
    trace();
    demo_middle();
}

int main() {
    tests::run_all();

#ifndef NDEBUG
    std::cout << "\ntrace demo:\n";
    demo_root();
#endif

    return 0;
}
