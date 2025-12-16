#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out"
rm "${SCRIPT_NAME}.out"
exit
#endif

#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

struct LoudStrategy {
    void act() const {
        std::cout << "LoudStrategy::act\n";
    }
};

struct QuietStrategy {
    void act() const {
        std::cout << "QuietStrategy::act\n";
    }
};

template <typename Impl>
class Entity : private Impl {
    static_assert(std::is_default_constructible_v<Impl>, "Impl must be default-constructible");
public:
    void act() const {
        Impl::act();
    }
};

namespace tests {

struct Logger {
    std::size_t passed = 0;

    void pass(const std::string& name) {
        ++passed;
        std::cout << "PASS: " << name << '\n';
    }
};

void require(bool ok) {
    assert(ok);
}

template <typename F>
std::string capture_stdout(F&& fn) {
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    std::forward<F>(fn)();
    std::cout.rdbuf(old);
    return oss.str();
}

void test_loud(Logger& log) {
    const Entity<LoudStrategy> e;

    const std::string out = capture_stdout([&] { e.act(); });
    require(out == "LoudStrategy::act\n");

    log.pass("entity_uses_loud_strategy");
}

void test_quiet(Logger& log) {
    const Entity<QuietStrategy> e;

    const std::string out = capture_stdout([&] { e.act(); });
    require(out == "QuietStrategy::act\n");

    log.pass("entity_uses_quiet_strategy");
}

void test_two_entities_independent(Logger& log) {
    const Entity<LoudStrategy> e1;
    const Entity<QuietStrategy> e2;

    const std::string out = capture_stdout([&] {
        e1.act();
        e2.act();
        e1.act();
    });

    require(out == "LoudStrategy::act\nQuietStrategy::act\nLoudStrategy::act\n");
    log.pass("two_entities_with_different_strategies");
}

void run_all() {
    Logger log;
    test_loud(log);
    test_quiet(log);
    test_two_entities_independent(log);
    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
