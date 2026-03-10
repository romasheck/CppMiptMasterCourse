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

// Problem: 08.01

#include <cstdlib>
#include <iostream>
#include <type_traits>

class Entity_v1 {
public:
    explicit Entity_v1(int value = 0) : value_(value) {
    }

    int get() const {
        return value_;
    }

private:
    int value_ = 0;
};

class Entity_v2 {
public:
    int value = 0;
};

static_assert(std::is_standard_layout_v<Entity_v1>);
static_assert(std::is_standard_layout_v<Entity_v2>);
static_assert(sizeof(Entity_v1) == sizeof(Entity_v2));
static_assert(alignof(Entity_v1) == alignof(Entity_v2));

void hack_with_reinterpret(Entity_v1& entity, int value) {
    auto& fake = reinterpret_cast<Entity_v2&>(entity);
    fake.value = value;
}

void hack_with_raw_pointer(Entity_v1& entity, int value) {
    auto* raw = reinterpret_cast<int*>(&entity);
    *raw = value;
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

void run_all() {
    Logger log;

    {
        const Entity_v1 entity(10);
        log.require(entity.get() == 10, "Entity_v1 stores private int");
    }

    {
        Entity_v1 entity(10);
        hack_with_reinterpret(entity, 42);
        log.require(entity.get() == 42, "reinterpret_cast changes private field");
    }

    {
        Entity_v1 entity(-7);
        hack_with_raw_pointer(entity, 99);
        log.require(entity.get() == 99, "raw pointer cast changes private field");
    }

    {
        Entity_v1 entity(5);
        hack_with_reinterpret(entity, -123);
        hack_with_raw_pointer(entity, 777);
        log.require(entity.get() == 777, "multiple hacks keep modifying the same field");
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
