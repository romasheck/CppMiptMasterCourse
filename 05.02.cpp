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
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

class Entity {
public:
    virtual ~Entity() = default;
    virtual void test() const = 0;
};

class Client : virtual public Entity {
public:
    void test() const override {
        std::cout << "Client::test\n";
    }
};

class Server : virtual public Entity {
public:
    void test() const override {
        std::cout << "Server::test\n";
    }
};

template <typename T>
class Decorator : public virtual Entity, private T {
    static_assert(std::is_base_of_v<Entity, T>, "T must implement Entity");
public:
    void test() const override {
        std::cout << "Decorator::test: ";
        T::test();
    }
};

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

template <typename F>
std::string capture_stdout(F&& fn) {
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    std::forward<F>(fn)();
    std::cout.rdbuf(old);
    return oss.str();
}

void test_decorates_server(Logger& log) {
    Decorator<Server> d;
    Entity& e = d;

    const std::string out = capture_stdout([&] { e.test(); });
    require(out == "Decorator::test: Server::test\n");

    log.pass("decorator_wraps_server_output");
}

void test_decorates_client(Logger& log) {
    Decorator<Client> d;
    Entity& e = d;

    const std::string out = capture_stdout([&] { e.test(); });
    require(out == "Decorator::test: Client::test\n");

    log.pass("decorator_wraps_client_output");
}

void test_polymorphic_use(Logger& log) {
    const std::string out = capture_stdout([] {
        Entity* e1 = new Decorator<Server>();
        e1->test();
        delete e1;

        Entity* e2 = new Decorator<Client>();
        e2->test();
        delete e2;
    });

    require(out == "Decorator::test: Server::test\nDecorator::test: Client::test\n");
    log.pass("polymorphic_use_via_entity_pointer");
}

void run_all() {
    Logger log;
    test_decorates_server(log);
    test_decorates_client(log);
    test_polymorphic_use(log);
    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
