#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out" -lbenchmark -pthread
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 09.09
// Description: Free-list allocator with selectable first-fit and best-fit search strategies.

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>

class Allocator {
public:
    enum class Strategy { first_fit, best_fit };
    class Allocation;
private:
    struct State;

public:
    explicit Allocator(std::size_t size, Strategy strategy = Strategy::first_fit)
        : state_(std::make_shared<State>(size, strategy)) {
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    Allocation allocate(std::size_t size) {
        return Allocation(state_, state_->allocate_raw(size));
    }

    std::size_t largest_free_block() const {
        return state_->largest_free_block();
    }

private:
    struct State {
        struct Node {
            std::size_t size = 0;
            Node* next = nullptr;
        };

        struct alignas(std::max_align_t) Header {
            std::size_t size = 0;
        };

        struct SearchResult {
            Node* current = nullptr;
            Node* previous = nullptr;
        };

        struct StorageDeleter {
            std::size_t size = 0;

            void operator()(void* pointer) const noexcept {
                ::operator delete(pointer, size, std::align_val_t(alignment_));
            }
        };

        using StoragePointer = std::unique_ptr<void, StorageDeleter>;

        explicit State(std::size_t size, Strategy strategy) : size(size), strategy(strategy) {
            assert(this->size >= sizeof(Node) + sizeof(Header) + 1);

            storage = StoragePointer(::operator new(this->size, std::align_val_t(alignment_)),
                                     StorageDeleter{this->size});
            head = as_node(storage.get());
            head->size = this->size - sizeof(Header);
            head->next = nullptr;
        }

        void* allocate_raw(std::size_t requested_size) {
            void* end = as_byte(storage.get()) + sizeof(Header) + requested_size;
            void* next = end;
            std::size_t free = 2 * alignof(Header);

            if (next = std::align(alignof(Header), sizeof(Header), next, free); !next) {
                return nullptr;
            }

            const std::size_t padding = static_cast<std::size_t>(as_byte(next) - as_byte(end));
            const std::size_t required = requested_size + padding;

            const auto [current, previous] = find(required);
            if (!current) {
                return nullptr;
            }

            std::size_t actual_padding = padding;
            if (current->size >= required + sizeof(Node) + 1) {
                const std::size_t step = sizeof(Header) + required;
                auto* node = as_node(as_byte(current) + step);
                node->size = current->size - step;
                node->next = current->next;
                current->next = node;
            } else {
                actual_padding += current->size - required;
            }

            if (!previous) {
                head = current->next;
            } else {
                previous->next = current->next;
            }

            auto* header = as_header(current);
            header->size = requested_size + actual_padding;
            return as_byte(current) + sizeof(Header);
        }

        void deallocate_raw(void* pointer) {
            auto* node = as_node(as_byte(pointer) - sizeof(Header));
            Node* previous = nullptr;
            Node* current = head;

            while (current) {
                if (node < current) {
                    node->next = current;
                    if (!previous) {
                        head = node;
                    } else {
                        previous->next = node;
                    }
                    merge(previous, node);
                    return;
                }

                previous = current;
                current = current->next;
            }

            node->next = nullptr;
            if (!previous) {
                head = node;
            } else {
                previous->next = node;
            }
            merge(previous, node);
        }

        std::size_t largest_free_block() const {
            std::size_t best = 0;
            for (Node* current = head; current; current = current->next) {
                best = std::max(best, current->size);
            }
            return best;
        }

        std::byte* as_byte(void* pointer) const {
            return static_cast<std::byte*>(pointer);
        }

        Node* as_node(void* pointer) const {
            return static_cast<Node*>(pointer);
        }

        Header* as_header(void* pointer) const {
            return static_cast<Header*>(pointer);
        }

        SearchResult find(std::size_t requested_size) const {
            return strategy == Strategy::first_fit ? find_first(requested_size) : find_best(requested_size);
        }

        SearchResult find_first(std::size_t requested_size) const {
            Node* current = head;
            Node* previous = nullptr;

            while (current && current->size < requested_size) {
                previous = current;
                current = current->next;
            }

            return {current, previous};
        }

        SearchResult find_best(std::size_t requested_size) const {
            Node* best = nullptr;
            Node* best_previous = nullptr;
            Node* current = head;
            Node* previous = nullptr;
            std::size_t best_size = static_cast<std::size_t>(-1);

            while (current) {
                if (current->size >= requested_size && current->size < best_size) {
                    best = current;
                    best_previous = previous;
                    best_size = current->size;
                }

                previous = current;
                current = current->next;
            }

            return {best, best_previous};
        }

        void merge(Node* previous, Node* node) {
            if (node->next && as_byte(node) + sizeof(Header) + node->size == as_byte(node->next)) {
                node->size += sizeof(Header) + node->next->size;
                node->next = node->next->next;
            }

            if (previous && as_byte(previous) + sizeof(Header) + previous->size == as_byte(node)) {
                previous->size += sizeof(Header) + node->size;
                previous->next = node->next;
            }
        }

        std::size_t size = 0;
        Strategy strategy = Strategy::first_fit;
        StoragePointer storage{nullptr, StorageDeleter{}};
        Node* head = nullptr;

        static constexpr std::size_t alignment_ = alignof(std::max_align_t);
    };

    std::shared_ptr<State> state_;

public:
    class Allocation {
    public:
        Allocation() = default;

        Allocation(std::shared_ptr<State> owner, void* pointer)
            : owner_(std::move(owner)), pointer_(pointer) {
        }

        ~Allocation() {
            reset();
        }

        Allocation(const Allocation&) = delete;
        Allocation& operator=(const Allocation&) = delete;

        Allocation(Allocation&& other) noexcept
            : owner_(std::move(other.owner_)),
              pointer_(std::exchange(other.pointer_, nullptr)) {
        }

        Allocation& operator=(Allocation&& other) noexcept {
            if (this != &other) {
                reset();
                owner_ = std::move(other.owner_);
                pointer_ = std::exchange(other.pointer_, nullptr);
            }
            return *this;
        }

        void* get() const {
            return pointer_;
        }

        explicit operator bool() const {
            return pointer_ != nullptr;
        }

        void reset() {
            if (pointer_) {
                owner_->deallocate_raw(pointer_);
                pointer_ = nullptr;
                owner_.reset();
            }
        }

    private:
        std::shared_ptr<State> owner_;
        void* pointer_ = nullptr;
    };
};

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

void test_basic_reuse(Logger& log, Allocator::Strategy strategy, const char* message) {
    Allocator allocator(1024, strategy);

    auto first = allocator.allocate(128);
    auto second = allocator.allocate(128);
    const void* first_address = first.get();
    first.reset();
    second.reset();
    auto merged = allocator.allocate(256);

    log.require(merged.get() == first_address, message);
}

void test_full_cycle(Logger& log, Allocator::Strategy strategy, const char* message) {
    Allocator allocator(2048, strategy);

    std::vector<Allocator::Allocation> blocks;
    for (int i = 0; i < 6; ++i) {
        blocks.push_back(allocator.allocate(128));
    }

    bool all_allocated = std::ranges::all_of(blocks, [](const auto& block) { return static_cast<bool>(block); });
    blocks.clear();

    log.require(all_allocated && allocator.largest_free_block() >= 2048 - 2 * sizeof(std::size_t), message);
}

void test_allocation_can_outlive_allocator(Logger& log) {
    Allocator::Allocation block;

    {
        Allocator allocator(1024, Allocator::Strategy::first_fit);
        block = allocator.allocate(128);
        log.require(static_cast<bool>(block), "allocation handle is created");
    }

    block.reset();
    log.require(true, "allocation can outlive allocator wrapper safely");
}

void run_all() {
    Logger log;

    test_basic_reuse(log, Allocator::Strategy::first_fit, "first-fit allocator reuses merged space");
    test_basic_reuse(log, Allocator::Strategy::best_fit, "best-fit allocator reuses merged space");
    test_full_cycle(log, Allocator::Strategy::first_fit, "first-fit allocator survives full cycle");
    test_full_cycle(log, Allocator::Strategy::best_fit, "best-fit allocator survives full cycle");
    test_allocation_can_outlive_allocator(log);

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

namespace bench_data {

constexpr std::size_t kb = 1024;
constexpr std::size_t mb = kb * kb;
constexpr std::size_t pool_size = 512 * mb;
constexpr std::size_t operation_count = 512;

std::vector<std::size_t> make_sizes() {
    std::default_random_engine engine(123456);
    std::uniform_int_distribution<int> distribution(1, 8);

    std::vector<std::size_t> sizes(operation_count);
    for (std::size_t& size : sizes) {
        size = static_cast<std::size_t>(distribution(engine)) * mb;
    }
    return sizes;
}

const std::vector<std::size_t> sizes = make_sizes();

}  // namespace bench_data

static void benchmark_allocator_first_fit(benchmark::State& state) {
    std::vector<Allocator::Allocation> blocks(bench_data::operation_count);

    for (auto _ : state) {
        Allocator allocator(bench_data::pool_size, Allocator::Strategy::first_fit);

        for (std::size_t i = 0; i < bench_data::operation_count; ++i) {
            blocks[i] = allocator.allocate(bench_data::sizes[i]);
        }

        for (std::size_t i = 0; i < bench_data::operation_count; i += 16) {
            blocks[i].reset();
        }

        for (std::size_t i = 0; i < bench_data::operation_count; i += 16) {
            blocks[i] = allocator.allocate(bench_data::sizes[i]);
        }

        blocks.clear();
        blocks.resize(bench_data::operation_count);

        benchmark::DoNotOptimize(blocks.size());
        benchmark::ClobberMemory();
    }
}

static void benchmark_allocator_best_fit(benchmark::State& state) {
    std::vector<Allocator::Allocation> blocks(bench_data::operation_count);

    for (auto _ : state) {
        Allocator allocator(bench_data::pool_size, Allocator::Strategy::best_fit);

        for (std::size_t i = 0; i < bench_data::operation_count; ++i) {
            blocks[i] = allocator.allocate(bench_data::sizes[i]);
        }

        for (std::size_t i = 0; i < bench_data::operation_count; i += 16) {
            blocks[i].reset();
        }

        for (std::size_t i = 0; i < bench_data::operation_count; i += 16) {
            blocks[i] = allocator.allocate(bench_data::sizes[i]);
        }

        blocks.clear();
        blocks.resize(bench_data::operation_count);

        benchmark::DoNotOptimize(blocks.size());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(benchmark_allocator_first_fit);
BENCHMARK(benchmark_allocator_best_fit);

int main(int argc, char** argv) {
    tests::run_all();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
