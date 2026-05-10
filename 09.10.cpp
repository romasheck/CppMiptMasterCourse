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

// Problem: 09.10
// Description: Four polymorphic allocators sharing one abstract interface.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Allocator {
public:
    virtual ~Allocator() = default;

    virtual void* allocate(std::size_t size) = 0;
    virtual void deallocate(void* pointer) = 0;

protected:
    struct alignas(std::max_align_t) Header {
        std::size_t size = 0;
        std::size_t previous_offset = 0;
    };

    struct Node {
        std::size_t size = 0;
        Node* next = nullptr;
    };

    template <typename T>
    T* get(void* pointer) const {
        return static_cast<T*>(pointer);
    }

    template <typename T>
    const T* get(const void* pointer) const {
        return static_cast<const T*>(pointer);
    }
};

class RawStorage {
public:
    explicit RawStorage(std::size_t size)
        : size_(size),
          data_(::operator new(size_, std::align_val_t(kAlignment)), Deleter{size_}) {
    }

    std::byte* data() const {
        return static_cast<std::byte*>(data_.get());
    }

    std::size_t size() const {
        return size_;
    }

    static constexpr std::size_t alignment() {
        return kAlignment;
    }

private:
    struct Deleter {
        std::size_t size = 0;

        void operator()(void* pointer) const noexcept {
            ::operator delete(pointer, size, std::align_val_t(kAlignment));
        }
    };

    static constexpr std::size_t kAlignment = alignof(std::max_align_t);

    std::size_t size_ = 0;
    std::unique_ptr<void, Deleter> data_;
};

class LinearAllocator final : public Allocator {
public:
    explicit LinearAllocator(std::size_t size) : storage_(size) {
    }

    void* allocate(std::size_t size) override {
        void* begin = storage_.data() + offset_;
        std::size_t free = storage_.size() - offset_;

        if (std::align(RawStorage::alignment(), size, begin, free) == nullptr) {
            return nullptr;
        }

        offset_ = static_cast<std::size_t>(static_cast<std::byte*>(begin) - storage_.data()) + size;
        return begin;
    }

    void deallocate(void*) override {
    }

private:
    RawStorage storage_;
    std::size_t offset_ = 0;
};

class StackAllocator final : public Allocator {
public:
    explicit StackAllocator(std::size_t size) : storage_(size) {
    }

    void* allocate(std::size_t size) override {
        void* begin = storage_.data() + offset_;
        std::size_t free = storage_.size() - offset_;
        const std::size_t requested = sizeof(Header) + size;

        if (std::align(RawStorage::alignment(), requested, begin, free) == nullptr) {
            return nullptr;
        }

        auto* header = get<Header>(begin);
        header->size = size;
        header->previous_offset = offset_;

        auto* user = reinterpret_cast<std::byte*>(header + 1);
        offset_ = static_cast<std::size_t>(user - storage_.data()) + size;
        return user;
    }

    void deallocate(void* pointer) override {
        if (pointer == nullptr) {
            return;
        }

        auto* header = get<Header>(static_cast<std::byte*>(pointer) - sizeof(Header));
        offset_ = header->previous_offset;
    }

private:
    RawStorage storage_;
    std::size_t offset_ = 0;
};

class PoolAllocator final : public Allocator {
public:
    PoolAllocator(std::size_t block_size, std::size_t block_count)
        : block_size_(std::max(block_size, sizeof(Node))),
          storage_(block_size_ * block_count) {
        for (std::size_t i = 0; i < block_count; ++i) {
            auto* node = get<Node>(storage_.data() + i * block_size_);
            node->next = head_;
            head_ = node;
        }
    }

    void* allocate(std::size_t size) override {
        if (size > block_size_ || head_ == nullptr) {
            return nullptr;
        }

        Node* node = head_;
        head_ = head_->next;
        return node;
    }

    void deallocate(void* pointer) override {
        if (pointer == nullptr) {
            return;
        }

        auto* node = get<Node>(pointer);
        node->next = head_;
        head_ = node;
    }

private:
    std::size_t block_size_ = 0;
    RawStorage storage_;
    Node* head_ = nullptr;
};

class FreeListAllocator final : public Allocator {
public:
    explicit FreeListAllocator(std::size_t size) : storage_(size) {
        assert(size >= sizeof(Header) + sizeof(Node) + 1);
        head_ = get<Node>(storage_.data());
        head_->size = storage_.size() - sizeof(Header);
        head_->next = nullptr;
    }

    void* allocate(std::size_t size) override {
        void* end = storage_.data() + sizeof(Header) + size;
        void* next = end;
        std::size_t free = 2 * alignof(Header);

        if (std::align(alignof(Header), sizeof(Header), next, free) == nullptr) {
            return nullptr;
        }

        const std::size_t padding = static_cast<std::size_t>(static_cast<std::byte*>(next) -
                                                             static_cast<std::byte*>(end));
        const std::size_t requested = size + padding;
        auto [current, previous] = find_first(requested);
        if (current == nullptr) {
            return nullptr;
        }

        std::size_t actual_padding = padding;
        if (current->size >= requested + sizeof(Header) + sizeof(Node) + 1) {
            const std::size_t step = sizeof(Header) + requested;
            auto* node = get<Node>(reinterpret_cast<std::byte*>(current) + step);
            node->size = current->size - step;
            node->next = current->next;
            current->next = node;
        } else {
            actual_padding += current->size - requested;
        }

        if (previous == nullptr) {
            head_ = current->next;
        } else {
            previous->next = current->next;
        }

        auto* header = get<Header>(current);
        header->size = size + actual_padding;
        return reinterpret_cast<std::byte*>(current) + sizeof(Header);
    }

    void deallocate(void* pointer) override {
        if (pointer == nullptr) {
            return;
        }

        auto* node = get<Node>(static_cast<std::byte*>(pointer) - sizeof(Header));
        Node* previous = nullptr;
        Node* current = head_;

        while (current != nullptr && current < node) {
            previous = current;
            current = current->next;
        }

        node->next = current;
        if (previous == nullptr) {
            head_ = node;
        } else {
            previous->next = node;
        }

        merge(previous, node);
    }

private:
    struct SearchResult {
        Node* current = nullptr;
        Node* previous = nullptr;
    };

    SearchResult find_first(std::size_t size) const {
        Node* current = head_;
        Node* previous = nullptr;

        while (current != nullptr && current->size < size) {
            previous = current;
            current = current->next;
        }

        return {current, previous};
    }

    void merge(Node* previous, Node* node) {
        if (node->next != nullptr &&
            reinterpret_cast<std::byte*>(node) + sizeof(Header) + node->size ==
                reinterpret_cast<std::byte*>(node->next)) {
            node->size += sizeof(Header) + node->next->size;
            node->next = node->next->next;
        }

        if (previous != nullptr &&
            reinterpret_cast<std::byte*>(previous) + sizeof(Header) + previous->size ==
                reinterpret_cast<std::byte*>(node)) {
            previous->size += sizeof(Header) + node->size;
            previous->next = node->next;
        }
    }

    RawStorage storage_;
    Node* head_ = nullptr;
};

template <typename T, typename... Args>
T* allocate_object(Allocator& allocator, Args&&... args) {
    void* memory = allocator.allocate(sizeof(T));
    if (memory == nullptr) {
        throw std::bad_alloc();
    }
    return std::construct_at(static_cast<T*>(memory), std::forward<Args>(args)...);
}

template <typename T>
void deallocate_object(Allocator& allocator, T* pointer) {
    std::destroy_at(pointer);
    allocator.deallocate(pointer);
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

void test_polymorphic_allocators(Logger& log) {
    std::vector<std::unique_ptr<Allocator>> allocators;
    allocators.push_back(std::make_unique<LinearAllocator>(1024));
    allocators.push_back(std::make_unique<StackAllocator>(1024));
    allocators.push_back(std::make_unique<PoolAllocator>(64, 8));
    allocators.push_back(std::make_unique<FreeListAllocator>(1024));

    for (auto& allocator : allocators) {
        int* value = allocate_object<int>(*allocator, 42);
        require(*value == 42, "allocated object keeps its value");
        deallocate_object(*allocator, value);
    }

    log.require(true, "all allocators work through the abstract interface");
}

void test_pool_reuses_blocks(Logger& log) {
    PoolAllocator allocator(64, 1);
    void* first = allocator.allocate(sizeof(int));
    require(first != nullptr, "first pool allocation");
    require(allocator.allocate(sizeof(int)) == nullptr, "single block pool should be exhausted");
    allocator.deallocate(first);
    require(allocator.allocate(sizeof(int)) == first, "pool should reuse released block");
    log.require(true, "pool allocator reuses memory");
}

void test_free_list_merges(Logger& log) {
    FreeListAllocator allocator(1024);
    void* first = allocator.allocate(64);
    void* second = allocator.allocate(64);
    require(first != nullptr && second != nullptr, "free-list allocations");

    allocator.deallocate(second);
    allocator.deallocate(first);
    require(allocator.allocate(128) == first, "neighboring free blocks should merge");
    log.require(true, "free-list allocator merges adjacent blocks");
}

void run_all() {
    Logger log;

    try {
        test_polymorphic_allocators(log);
        test_pool_reuses_blocks(log);
        test_free_list_merges(log);
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
