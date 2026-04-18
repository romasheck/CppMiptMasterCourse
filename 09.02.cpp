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

// Problem: 09.02
// Description: Binary tree with shared ownership of children and weak parent links.

#include <cstdlib>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>

template <typename T>
class Tree {
public:
    struct Node {
        const T& value() const {
            return value_;
        }

        std::shared_ptr<Node> left() const {
            return left_;
        }

        std::shared_ptr<Node> right() const {
            return right_;
        }

        std::shared_ptr<Node> parent() const {
            return parent_.lock();
        }

        ~Node() {
            std::cout << "Node(" << value_ << ") destroyed\n";
        }

    private:
        friend class Tree;

        explicit Node(T value) : value_(std::move(value)) {
            std::cout << "Node(" << value_ << ") constructed\n";
        }

        T value_{};
        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
    };

    std::shared_ptr<Node> root() const {
        return root_;
    }

    std::shared_ptr<Node> set_root(T value) {
        root_ = std::shared_ptr<Node>(new Node(std::move(value)));
        return root_;
    }

    std::shared_ptr<Node> attach_left(const std::shared_ptr<Node>& parent, T value) {
        return attach_child(parent, std::move(value), Side::left);
    }

    std::shared_ptr<Node> attach_right(const std::shared_ptr<Node>& parent, T value) {
        return attach_child(parent, std::move(value), Side::right);
    }

    void traverse_v1() const {
        if (!root_) {
            std::cout << '\n';
            return;
        }

        std::queue<std::shared_ptr<Node>> pending;
        pending.push(root_);
        bool first = true;

        while (!pending.empty()) {
            const std::shared_ptr<Node> node = pending.front();
            pending.pop();

            if (!first) {
                std::cout << ' ';
            }
            std::cout << node->value();
            first = false;

            if (node->left()) {
                pending.push(node->left());
            }
            if (node->right()) {
                pending.push(node->right());
            }
        }

        std::cout << '\n';
    }

    void traverse_v2() const {
        traverse_depth_first(root_, true);
        std::cout << '\n';
    }

private:
    enum class Side { left, right };

    std::shared_ptr<Node> attach_child(const std::shared_ptr<Node>& parent, T value, Side side) {
        if (!parent) {
            throw std::logic_error("parent node must exist");
        }

        std::shared_ptr<Node> child(new Node(std::move(value)));
        child->parent_ = parent;

        std::shared_ptr<Node>& slot = side == Side::left ? parent->left_ : parent->right_;
        if (slot) {
            slot->parent_.reset();
        }
        slot = child;
        return child;
    }

    static void traverse_depth_first(const std::shared_ptr<Node>& node, bool first) {
        if (!node) {
            return;
        }

        if (!first) {
            std::cout << ' ';
        }
        std::cout << node->value();

        const bool has_left = static_cast<bool>(node->left());
        if (has_left) {
            traverse_depth_first(node->left(), false);
        }
        if (node->right()) {
            if (!has_left) {
                std::cout << ' ';
            }
            traverse_depth_first(node->right(), false);
        }
    }

private:
    std::shared_ptr<Node> root_;
};

Tree<int> make_demo_tree() {
    Tree<int> tree;

    const auto root = tree.set_root(1);
    const auto left = tree.attach_left(root, 2);
    const auto right = tree.attach_right(root, 3);
    tree.attach_left(left, 4);
    tree.attach_right(left, 5);
    tree.attach_left(right, 6);
    tree.attach_right(right, 7);

    return tree;
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

    void reset() {
        buffer_.str("");
        buffer_.clear();
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

void test_traverse_breadth_first(Logger& log) {
    CoutCapture capture;
    Tree<int> tree = make_demo_tree();

    capture.reset();
    tree.traverse_v1();

    const std::string output = capture.str();
    require(output.find("1 2 3 4 5 6 7\n") != std::string::npos, "expected BFS order");
    log.require(true, "traverse_v1 prints breadth-first order");
}

void test_traverse_depth_first(Logger& log) {
    CoutCapture capture;
    Tree<int> tree = make_demo_tree();

    capture.reset();
    tree.traverse_v2();

    const std::string output = capture.str();
    require(output.find("1 2 4 5 3 6 7\n") != std::string::npos, "expected DFS order");
    log.require(true, "traverse_v2 prints depth-first order");
}

void test_parent_links(Logger& log) {
    Tree<int> tree = make_demo_tree();
    const auto root = tree.root();

    require(root != nullptr, "expected root node");
    require(!root->parent(), "root should not have parent");
    require(root->left()->parent() == root, "left child should know parent");
    require(root->right()->parent() == root, "right child should know parent");
    require(root->left()->left()->parent() == root->left(), "grandchild should know parent");

    log.require(true, "weak parent links are wired correctly");
}

void test_attach_requires_parent(Logger& log) {
    Tree<int> tree;
    bool caught = false;

    try {
        tree.attach_left(nullptr, 42);
    } catch (const std::logic_error&) {
        caught = true;
    }

    require(caught, "expected null-parent rejection");
    log.require(true, "tree rejects attaching child without parent");
}

void test_replacing_child_updates_parent(Logger& log) {
    Tree<int> tree = make_demo_tree();
    const auto root = tree.root();
    const auto left = root->left();
    const auto replacement = tree.attach_left(root, 8);

    require(root->left() == replacement, "expected replacement in left slot");
    require(replacement->parent() == root, "replacement should know parent");
    require(left->parent() == nullptr, "detached child should forget parent");
    log.require(true, "replacing child preserves parent invariant");
}

void test_no_strong_cycle(Logger& log) {
    std::weak_ptr<Tree<int>::Node> weak_root;
    std::weak_ptr<Tree<int>::Node> weak_leaf;
    std::string output;

    {
        CoutCapture capture;
        {
            Tree<int> tree = make_demo_tree();
            weak_root = tree.root();
            weak_leaf = tree.root()->right()->right();
        }
        output = capture.str();
    }

    require(weak_root.expired(), "root should expire after tree destruction");
    require(weak_leaf.expired(), "leaf should expire after tree destruction");
    require(output.find("Node(1) destroyed") != std::string::npos, "expected root destructor output");
    require(output.find("Node(7) destroyed") != std::string::npos, "expected leaf destructor output");

    log.require(true, "weak_ptr prevents ownership cycles");
}

void run_all() {
    Logger log;

    try {
        test_traverse_breadth_first(log);
        test_traverse_depth_first(log);
        test_parent_links(log);
        test_attach_requires_parent(log);
        test_replacing_child_updates_parent(log);
        test_no_strong_cycle(log);
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

    std::cout << "\ndemo tree construction:\n";
    {
        Tree<int> tree = make_demo_tree();

        std::cout << "BFS: ";
        tree.traverse_v1();

        std::cout << "DFS: ";
        tree.traverse_v2();
    }
    std::cout << "tree scope finished\n";

    return 0;
}

// Score is 9/10
