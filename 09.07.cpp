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

// Problem: 09.07
// Description: Pimpl with in-place storage and placement new instead of heap allocation.

#include <array>
#include <bit>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

class Entity {
public:
    struct Implementation {
        int id = next_id++;
        int value = 0;

        static inline int next_id = 1;
    };

    Entity() {
        static_assert(sizeof(Implementation) <= kStorageSize,
                      "Implementation must fit into inline storage");
        static_assert(alignof(Implementation) <= alignof(std::max_align_t),
                      "Implementation alignment must fit inline storage alignment");

        ::new (storage_.data()) Implementation();
    }

    Entity(Entity&& other) noexcept {
        ::new (storage_.data()) Implementation(std::move(*other.get()));
    }

    ~Entity() {
        std::destroy_at(get());
    }

    Entity& operator=(Entity&& other) noexcept {
        if (this != &other) {
            *get() = std::move(*other.get());
        }
        return *this;
    }

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    void test() const {
        std::cout << "Entity{id=" << get()->id << ", value=" << get()->value << "}\n";
    }

    void set_value(int value) {
        get()->value = value;
    }

    int value() const {
        return get()->value;
    }

    Implementation* get() {
        auto raw = std::bit_cast<Implementation*>(storage_.data());
        return std::launder(raw);
    }

    const Implementation* get() const {
        auto raw = std::bit_cast<const Implementation*>(storage_.data());
        return std::launder(raw);
    }

private:
    static constexpr std::size_t kStorageSize = 16;

    alignas(std::max_align_t) std::array<std::byte, kStorageSize> storage_{};
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

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_storage_contract(Logger& log) {
    require(sizeof(Entity::Implementation) <= 16, "implementation should fit 16 bytes");
    require(alignof(Entity::Implementation) <= alignof(std::max_align_t),
            "implementation alignment should fit max_align_t");
    log.require(true, "inline storage contract holds");
}

void test_get_returns_live_object(Logger& log) {
    Entity entity;
    entity.set_value(42);

    require(entity.get() != nullptr, "non-const get should return pointer");
    require(entity.get()->value == 42, "non-const get should reach implementation");

    const Entity& cref = entity;
    require(cref.get() != nullptr, "const get should return pointer");
    require(cref.get()->value == 42, "const get should read implementation");
    log.require(true, "get returns laundered implementation pointers");
}

void test_move_constructor(Logger& log) {
    Entity source;
    source.set_value(77);

    const int source_id = source.get()->id;
    Entity moved(std::move(source));

    require(moved.value() == 77, "moved object should preserve value");
    require(moved.get()->id == source_id, "move construction should preserve implementation state");
    require(moved.get() != source.get(), "different entities should have different inline addresses");
    log.require(true, "move constructor rebuilds implementation in destination storage");
}

void test_move_assignment(Logger& log) {
    Entity source;
    source.set_value(15);

    Entity destination;
    destination.set_value(99);
    destination = std::move(source);

    require(destination.value() == 15, "move assignment should transfer value");
    log.require(true, "move assignment updates inline implementation");
}

void test_pointer_lives_inside_entity(Logger& log) {
    Entity entity;

    const auto begin = reinterpret_cast<std::uintptr_t>(&entity);
    const auto end = begin + sizeof(Entity);
    const auto impl = reinterpret_cast<std::uintptr_t>(entity.get());

    require(begin <= impl && impl < end, "implementation pointer should point into entity object");
    log.require(true, "implementation is stored inline");
}

void run_all() {
    Logger log;

    try {
        test_storage_contract(log);
        test_get_returns_live_object(log);
        test_move_constructor(log);
        test_move_assignment(log);
        test_pointer_lives_inside_entity(log);
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

    Entity entity;
    entity.set_value(123);
    entity.test();

    std::cout << "Implementation address: " << entity.get() << '\n';
    std::cout << "Entity address range: [" << static_cast<const void*>(&entity) << ", "
              << static_cast<const void*>(reinterpret_cast<const std::byte*>(&entity) + sizeof(Entity))
              << ")\n";

    return 0;
}
