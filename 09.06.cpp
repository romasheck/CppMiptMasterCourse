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

// Problem: 09.06
// Description: Fibonacci sequence with a manual forward iterator and a Boost.Iterator facade.

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace fibonacci_manual {

class Iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int*;
    using reference = const int&;

    Iterator() = default;

    explicit Iterator(int remaining) : remaining_(remaining) {
    }

    Iterator& operator++() {
        advance();
        return *this;
    }

    Iterator operator++(int) {
        Iterator copy = *this;
        advance();
        return copy;
    }

    int operator*() const {
        return current_;
    }

    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
        return lhs.remaining_ == rhs.remaining_ &&
               (lhs.remaining_ == 0 || (lhs.current_ == rhs.current_ && lhs.next_ == rhs.next_));
    }

private:
    void advance() {
        if (remaining_ == 0) {
            return;
        }

        const int new_next = current_ + next_;
        current_ = next_;
        next_ = new_next;
        --remaining_;
    }

private:
    int current_ = 0;
    int next_ = 1;
    int remaining_ = 0;
};

class Range {
public:
    explicit Range(int count) : count_(count) {
    }

    Iterator begin() const {
        return Iterator(count_);
    }

    Iterator end() const {
        return Iterator();
    }

private:
    int count_ = 0;
};

}  // namespace fibonacci_manual

namespace fibonacci_boost {

class Iterator : public boost::iterator_facade<Iterator, int, boost::forward_traversal_tag, int> {
public:
    Iterator() = default;

    explicit Iterator(int remaining) : remaining_(remaining) {
    }

private:
    friend class boost::iterator_core_access;

    void increment() {
        if (remaining_ == 0) {
            return;
        }

        const int new_next = current_ + next_;
        current_ = next_;
        next_ = new_next;
        --remaining_;
    }

    int dereference() const {
        return current_;
    }

    bool equal(const Iterator& other) const {
        return remaining_ == other.remaining_ &&
               (remaining_ == 0 || (current_ == other.current_ && next_ == other.next_));
    }

private:
    int current_ = 0;
    int next_ = 1;
    int remaining_ = 0;
};

class Range {
public:
    explicit Range(int count) : count_(count) {
    }

    Iterator begin() const {
        return Iterator(count_);
    }

    Iterator end() const {
        return Iterator();
    }

private:
    int count_ = 0;
};

}  // namespace fibonacci_boost

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

template <typename Range>
std::vector<int> collect(const Range& range) {
    std::vector<int> result;
    for (int value : range) {
        result.push_back(value);
    }
    return result;
}

void test_manual_sequence(Logger& log) {
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13};
    require(collect(fibonacci_manual::Range(8)) == expected, "manual iterator sequence mismatch");
    log.require(true, "manual iterator generates Fibonacci numbers");
}

void test_manual_postfix_increment(Logger& log) {
    fibonacci_manual::Iterator iterator(4);
    const int first = *iterator++;
    const int second = *iterator++;
    const int third = *iterator;

    require(first == 0, "expected first value");
    require(second == 1, "expected second value");
    require(third == 1, "expected third value after postfix increments");
    log.require(true, "manual postfix increment works");
}

void test_manual_default_iterator(Logger& log) {
    const fibonacci_manual::Iterator end_a;
    const fibonacci_manual::Iterator end_b;
    require(end_a == end_b, "default iterators should compare equal");
    log.require(true, "manual default constructor creates end iterator");
}

void test_boost_sequence(Logger& log) {
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13};
    require(collect(fibonacci_boost::Range(8)) == expected, "boost iterator sequence mismatch");
    log.require(true, "Boost.Iterator facade generates Fibonacci numbers");
}

void test_algorithms_match(Logger& log) {
    const auto manual = collect(fibonacci_manual::Range(12));
    const auto boost = collect(fibonacci_boost::Range(12));
    require(manual == boost, "manual and boost implementations should match");
    log.require(true, "manual and boost algorithms agree");
}

void run_all() {
    Logger log;

    try {
        test_manual_sequence(log);
        test_manual_postfix_increment(log);
        test_manual_default_iterator(log);
        test_boost_sequence(log);
        test_algorithms_match(log);
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

    std::cout << "\nmanual: ";
    for (int value : fibonacci_manual::Range(10)) {
        std::cout << value << ' ';
    }

    std::cout << "\nboost:  ";
    for (int value : fibonacci_boost::Range(10)) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    return 0;
}
