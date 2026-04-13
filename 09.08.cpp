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

// Problem: 09.08
// Description: Overloaded allocation and deallocation for objects and built-in dynamic arrays.

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>

namespace memory_log {

struct Counters {
    int new_calls = 0;
    int delete_calls = 0;
    int new_array_calls = 0;
    int delete_array_calls = 0;
    int nothrow_new_calls = 0;
    int nothrow_delete_calls = 0;
    int nothrow_new_array_calls = 0;
    int nothrow_delete_array_calls = 0;
};

inline Counters counters{};

void reset() {
    counters = Counters{};
}

}  // namespace memory_log

template <typename Derived>
class Entity {
public:
    static void* operator new(std::size_t size) {
        ++memory_log::counters.new_calls;
        std::cout << "Entity::operator new (" << size << ")\n";
        return ::operator new(size);
    }

    static void operator delete(void* pointer) noexcept {
        ++memory_log::counters.delete_calls;
        std::cout << "Entity::operator delete\n";
        ::operator delete(pointer);
    }

    static void* operator new[](std::size_t size) {
        ++memory_log::counters.new_array_calls;
        std::cout << "Entity::operator new[] (" << size << ")\n";
        return ::operator new[](size);
    }

    static void operator delete[](void* pointer) noexcept {
        ++memory_log::counters.delete_array_calls;
        std::cout << "Entity::operator delete[]\n";
        ::operator delete[](pointer);
    }

    static void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept {
        ++memory_log::counters.nothrow_new_calls;
        std::cout << "Entity::operator new nothrow (" << size << ")\n";
        return ::operator new(size, tag);
    }

    static void operator delete(void* pointer, const std::nothrow_t&) noexcept {
        ++memory_log::counters.nothrow_delete_calls;
        std::cout << "Entity::operator delete nothrow\n";
        ::operator delete(pointer);
    }

    static void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept {
        ++memory_log::counters.nothrow_new_array_calls;
        std::cout << "Entity::operator new[] nothrow (" << size << ")\n";
        return ::operator new[](size, tag);
    }

    static void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
        ++memory_log::counters.nothrow_delete_array_calls;
        std::cout << "Entity::operator delete[] nothrow\n";
        ::operator delete[](pointer);
    }

protected:
    Entity() = default;
    ~Entity() = default;
};

class Client : private Entity<Client> {
private:
    using base_t = Entity<Client>;

public:
    Client() {
        std::cout << "Client::Client\n";
    }

    ~Client() {
        std::cout << "Client::~Client\n";
    }

    using base_t::operator new;
    using base_t::operator delete;
    using base_t::operator new[];
    using base_t::operator delete[];
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

void test_single_object_allocation(Logger& log) {
    memory_log::reset();

    Client* client = new Client;
    delete client;

    require(memory_log::counters.new_calls == 1, "expected one scalar new");
    require(memory_log::counters.delete_calls == 1, "expected one scalar delete");
    require(memory_log::counters.new_array_calls == 0, "unexpected array new");
    log.require(true, "scalar allocation uses overloaded new/delete");
}

void test_array_allocation(Logger& log) {
    memory_log::reset();

    Client* clients = new Client[3];
    delete[] clients;

    require(memory_log::counters.new_array_calls == 1, "expected one array new");
    require(memory_log::counters.delete_array_calls == 1, "expected one array delete");
    require(memory_log::counters.new_calls == 0, "unexpected scalar new");
    log.require(true, "array allocation uses overloaded new[]/delete[]");
}

void test_nothrow_scalar_allocation(Logger& log) {
    memory_log::reset();

    Client* client = new (std::nothrow) Client;
    require(client != nullptr, "nothrow scalar allocation should succeed in test");
    delete client;

    require(memory_log::counters.nothrow_new_calls == 1, "expected one nothrow scalar new");
    require(memory_log::counters.delete_calls == 1, "ordinary delete should release successful nothrow new");
    log.require(true, "nothrow scalar allocation overload is available");
}

void test_nothrow_array_allocation(Logger& log) {
    memory_log::reset();

    Client* clients = new (std::nothrow) Client[2];
    require(clients != nullptr, "nothrow array allocation should succeed in test");
    delete[] clients;

    require(memory_log::counters.nothrow_new_array_calls == 1, "expected one nothrow array new");
    require(memory_log::counters.delete_array_calls == 1,
            "ordinary delete[] should release successful nothrow new[]");
    log.require(true, "nothrow array allocation overload is available");
}

void test_explicit_nothrow_delete_overloads(Logger& log) {
    memory_log::reset();

    void* raw_object = ::operator new(sizeof(Client));
    Client::operator delete(raw_object, std::nothrow);

    void* raw_array = ::operator new[](2 * sizeof(Client));
    Client::operator delete[](raw_array, std::nothrow);

    require(memory_log::counters.nothrow_delete_calls == 1, "expected nothrow scalar delete overload");
    require(memory_log::counters.nothrow_delete_array_calls == 1,
            "expected nothrow array delete overload");
    log.require(true, "nothrow delete overloads are callable");
}

void run_all() {
    Logger log;

    try {
        test_single_object_allocation(log);
        test_array_allocation(log);
        test_nothrow_scalar_allocation(log);
        test_nothrow_array_allocation(log);
        test_explicit_nothrow_delete_overloads(log);
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
