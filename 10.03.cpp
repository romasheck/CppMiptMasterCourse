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

// Problem: 10.03
// Description: Two stack implementations that track the current minimum instead of the maximum.

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <utility>

namespace pair_stack {

template <typename T>
class Stack {
public:
    void push(T value) {
        const T current_min = data_.empty() ? value : std::min(value, data_.top().second);
        data_.emplace(value, current_min);
    }

    T top() const {
        ensure_not_empty();
        return data_.top().first;
    }

    void pop() {
        ensure_not_empty();
        data_.pop();
    }

    T min() const {
        ensure_not_empty();
        return data_.top().second;
    }

    bool empty() const {
        return data_.empty();
    }

private:
    void ensure_not_empty() const {
        if (data_.empty()) {
            throw std::runtime_error("stack is empty");
        }
    }

    std::stack<std::pair<T, T>> data_;
};

}  // namespace pair_stack

namespace encoded_stack {

template <typename T>
class Stack {
public:
    void push(T value) {
        if (data_.empty()) {
            data_.push(value);
            min_ = value;
            return;
        }

        if (value < min_) {
            data_.push(2 * value - min_);
            min_ = value;
        } else {
            data_.push(value);
        }
    }

    T top() const {
        ensure_not_empty();
        return data_.top() < min_ ? min_ : data_.top();
    }

    void pop() {
        ensure_not_empty();

        const T encoded = data_.top();
        data_.pop();

        if (data_.empty()) {
            return;
        }

        if (encoded < min_) {
            min_ = 2 * min_ - encoded;
        }
    }

    T min() const {
        ensure_not_empty();
        return min_;
    }

    bool empty() const {
        return data_.empty();
    }

private:
    void ensure_not_empty() const {
        if (data_.empty()) {
            throw std::runtime_error("stack is empty");
        }
    }

    std::stack<T> data_;
    T min_{};
};

}  // namespace encoded_stack

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

template <typename Stack>
void verify_basic_sequence(Stack& stack) {
    stack.push(3);
    require(stack.top() == 3 && stack.min() == 3, "after push 3");

    stack.push(1);
    require(stack.top() == 1 && stack.min() == 1, "after push 1");

    stack.push(2);
    require(stack.top() == 2 && stack.min() == 1, "after push 2");

    stack.push(0);
    require(stack.top() == 0 && stack.min() == 0, "after push 0");

    stack.pop();
    require(stack.top() == 2 && stack.min() == 1, "after pop 0");

    stack.pop();
    require(stack.top() == 1 && stack.min() == 1, "after pop 2");

    stack.pop();
    require(stack.top() == 3 && stack.min() == 3, "after pop 1");

    stack.pop();
    require(stack.empty(), "stack should be empty");
}

void test_pair_stack(Logger& log) {
    pair_stack::Stack<int> stack;
    verify_basic_sequence(stack);
    log.require(true, "pair-based stack tracks minimum");
}

void test_encoded_stack(Logger& log) {
    encoded_stack::Stack<int> stack;
    verify_basic_sequence(stack);
    log.require(true, "encoded stack tracks minimum");
}

template <typename Stack>
void verify_negative_values(Stack& stack) {
    stack.push(-2);
    stack.push(4);
    stack.push(-5);
    stack.push(3);

    require(stack.min() == -5, "minimum with negative values");
    stack.pop();
    require(stack.min() == -5, "minimum should stay after popping larger value");
    stack.pop();
    require(stack.min() == -2, "minimum should restore after popping smallest value");
}

void test_negative_values(Logger& log) {
    {
        pair_stack::Stack<int> stack;
        verify_negative_values(stack);
    }
    {
        encoded_stack::Stack<int> stack;
        verify_negative_values(stack);
    }

    log.require(true, "both stacks handle negative values");
}

template <typename Stack>
void verify_empty_guard() {
    Stack stack;
    bool caught = false;

    try {
        static_cast<void>(stack.min());
    } catch (const std::runtime_error&) {
        caught = true;
    }

    require(caught, "min() should reject empty stack");
}

void test_empty_stack(Logger& log) {
    verify_empty_guard<pair_stack::Stack<int>>();
    verify_empty_guard<encoded_stack::Stack<int>>();
    log.require(true, "empty stack access is guarded");
}

void run_all() {
    Logger log;

    try {
        test_pair_stack(log);
        test_encoded_stack(log);
        test_negative_values(log);
        test_empty_stack(log);
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
