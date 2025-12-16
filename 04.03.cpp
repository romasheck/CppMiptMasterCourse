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
#include <string>
#include <vector>

template <typename Container>
void handle(Container& container, int value) {
    container.push_back(value);
}

template <typename Container, typename T>
void handle([[maybe_unused]] Container& container, [[maybe_unused]] T&& value) {
}

template <typename Container, typename... Args>
void insert_pack(Container& container, Args&&... args) {
    (handle(container, std::forward<Args>(args)), ...);
}

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

void test_only_ints(Logger& log) {
    std::vector<int> v;
    insert_pack(v, 1, 2, 3, 4, 5);

    require(v.size() == 5);
    require(v[0] == 1);
    require(v[1] == 2);
    require(v[2] == 3);
    require(v[3] == 4);
    require(v[4] == 5);

    log.pass("only_ints_preserve_order");
}

void test_empty_pack(Logger& log) {
    std::vector<int> v;
    insert_pack(v);

    require(v.empty());
    log.pass("empty_pack_no_change");
}

void test_mixed_types(Logger& log) {
    std::vector<int> v;

    const double pi = 3.14;
    const char ch = 'a';
    const bool flag = true;

    insert_pack(v, 42, pi, "hello", ch, 100, flag);

    require(v.size() == 2);
    require(v[0] == 42);
    require(v[1] == 100);

    log.pass("mixed_types_only_int_inserted");
}

void test_negative_and_zero(Logger& log) {
    std::vector<int> v;
    insert_pack(v, 0, -1, 5, -100, 7);

    require(v.size() == 5);
    require(v[0] == 0);
    require(v[1] == -1);
    require(v[2] == 5);
    require(v[3] == -100);
    require(v[4] == 7);

    log.pass("negative_and_zero_ints");
}

void test_many_non_int(Logger& log) {
    std::vector<int> v;

    const std::string s = "abc";
    insert_pack(v, s, 1.5, 'x', false, 123, 2.5F, 456);

    require(v.size() == 2);
    require(v[0] == 123);
    require(v[1] == 456);

    log.pass("many_non_int_ignored");
}

void run_all() {
    Logger log;
    test_only_ints(log);
    test_empty_pack(log);
    test_mixed_types(log);
    test_negative_and_zero(log);
    test_many_non_int(log);
    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
