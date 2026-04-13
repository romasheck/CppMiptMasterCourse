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

// Problem: 09.05
// Description: Doubly linked list with bidirectional iterator based on smart pointers.

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template <typename T>
class List {
public:
    struct Node {
        explicit Node(T value) : value(std::move(value)) {
        }

        T value;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;
    };

public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator& operator++() {
            if (node_) {
                node_ = node_->next;
            }
            return *this;
        }

        Iterator operator++(int) {
            Iterator copy = *this;
            ++(*this);
            return copy;
        }

        Iterator& operator--() {
            if (node_) {
                node_ = node_->prev.lock();
            } else {
                node_ = tail_;
            }
            return *this;
        }

        Iterator operator--(int) {
            Iterator copy = *this;
            --(*this);
            return copy;
        }

        reference operator*() const {
            return node_->value;
        }

        pointer operator->() const {
            return &node_->value;
        }

        friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
            return lhs.node_ == rhs.node_;
        }

    private:
        friend class List;

        Iterator(std::shared_ptr<Node> node, std::shared_ptr<Node> tail)
            : node_(std::move(node)), tail_(std::move(tail)) {
        }

        std::shared_ptr<Node> node_;
        std::shared_ptr<Node> tail_;
    };

    Iterator begin() const {
        return make_iterator(head_);
    }

    Iterator end() const {
        return make_iterator(nullptr);
    }

    void push_back(T value) {
        auto node = std::make_shared<Node>(std::move(value));

        if (!head_) {
            head_ = node;
            tail_ = node;
            return;
        }

        node->prev = tail_;
        tail_->next = node;
        tail_ = std::move(node);
    }

    std::size_t size() const {
        std::size_t count = 0;
        for (auto current = head_; current; current = current->next) {
            ++count;
        }
        return count;
    }

private:
    Iterator make_iterator(std::shared_ptr<Node> node) const {
        return Iterator(std::move(node), tail_);
    }

    std::shared_ptr<Node> head_;
    std::shared_ptr<Node> tail_;
};

namespace tests {

struct Item {
    std::string name;
    int value = 0;
};

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

template <typename T>
std::vector<T> collect_forward(const List<T>& list) {
    std::vector<T> result;
    for (auto iterator = list.begin(); iterator != list.end(); ++iterator) {
        result.push_back(*iterator);
    }
    return result;
}

template <typename T>
std::vector<T> collect_backward(const List<T>& list) {
    std::vector<T> result;
    for (auto iterator = list.end(); iterator != list.begin();) {
        --iterator;
        result.push_back(*iterator);
    }
    return result;
}

void test_push_back_and_forward_iteration(Logger& log) {
    List<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    require(list.size() == 3, "expected size 3");
    require(collect_forward(list) == std::vector<int>({1, 2, 3}), "expected forward order");
    log.require(true, "push_back preserves forward order");
}

void test_prefix_decrement_from_end(Logger& log) {
    List<int> list;
    for (int value = 1; value <= 4; ++value) {
        list.push_back(value);
    }

    auto iterator = list.end();
    --iterator;
    require(*iterator == 4, "expected tail after decrement from end");
    --iterator;
    require(*iterator == 3, "expected previous node after second decrement");
    log.require(true, "prefix decrement works from end");
}

void test_postfix_decrement(Logger& log) {
    List<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_back(30);

    auto iterator = list.end();
    auto copy = iterator--;

    require(copy == list.end(), "postfix should return old iterator");
    require(*iterator == 30, "iterator should move to tail");

    auto second = iterator--;
    require(*second == 30, "postfix should preserve old value");
    require(*iterator == 20, "iterator should move backward");
    log.require(true, "postfix decrement returns previous state");
}

void test_reverse_iteration(Logger& log) {
    List<int> list;
    for (int value = 1; value <= 5; ++value) {
        list.push_back(value);
    }

    require(collect_backward(list) == std::vector<int>({5, 4, 3, 2, 1}), "expected reverse order");
    log.require(true, "reverse traversal works");
}

void test_arrow_operator(Logger& log) {
    List<Item> list;
    list.push_back({"alpha", 1});
    list.push_back({"beta", 2});

    auto iterator = list.begin();
    require(iterator->name == "alpha", "operator-> should access first element");
    ++iterator;
    require(iterator->value == 2, "operator-> should access second element");
    log.require(true, "operator-> works");
}

void test_range_based_for(Logger& log) {
    List<int> list;
    list.push_back(4);
    list.push_back(5);
    list.push_back(6);

    std::ostringstream output;
    for (const int value : list) {
        output << value << ' ';
    }

    require(output.str() == "4 5 6 ", "range-based for should use iterator");
    log.require(true, "range-based for works");
}

void run_all() {
    Logger log;

    try {
        test_push_back_and_forward_iteration(log);
        test_prefix_decrement_from_end(log);
        test_postfix_decrement(log);
        test_reverse_iteration(log);
        test_arrow_operator(log);
        test_range_based_for(log);
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
