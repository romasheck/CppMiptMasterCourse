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

// Problem: 11.01
// Description: Return a wrapper that can be dereferenced into the function itself.

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

class Wrapper {
public:
    using function_t = Wrapper (*)();

    explicit Wrapper(function_t function = nullptr) : function_(function) {
    }

    operator function_t() const {
        return function_;
    }

    function_t operator*() const {
        return function_;
    }

private:
    function_t function_ = nullptr;
};

Wrapper test() {
    std::cout << "test\n";
    return Wrapper(test);
}

namespace tests {

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

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_wrapper_types(Logger& log) {
    static_assert(std::is_same_v<decltype(test), Wrapper()>);
    static_assert(std::is_same_v<decltype(*test()), Wrapper::function_t>);
    log.require(true, "wrapper exposes function pointer type");
}

void test_required_syntax(Logger& log) {
    Wrapper function = test();
    Wrapper next = (*function)();
    Wrapper again = (*next)();

    require(static_cast<Wrapper::function_t>(function) == test, "wrapper should keep pointer to test");
    require(static_cast<Wrapper::function_t>(next) == test, "recursive call should still point to test");
    require(static_cast<Wrapper::function_t>(again) == test, "repeated dereference should still work");
    log.require(true, "required syntax works");
}

void run_all() {
    Logger log;

    try {
        test_wrapper_types(log);
        test_required_syntax(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
