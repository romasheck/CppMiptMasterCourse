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

// Problem: 09.03
// Description: Smart-pointer versions of examples 05.01, 05.03, 05.04, 05.09 and 05.13.

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace builder_example {

struct Entity {
    int x = 0;
    int y = 0;
};

class Builder {
public:
    virtual ~Builder() = default;

    std::unique_ptr<Entity> make_entity() {
        entity_ = std::make_unique<Entity>();
        set_x();
        set_y();
        return std::move(entity_);
    }

    virtual void set_x() = 0;
    virtual void set_y() = 0;

protected:
    Entity& entity() {
        return *entity_;
    }

private:
    std::unique_ptr<Entity> entity_;
};

class ClientBuilder final : public Builder {
public:
    void set_x() override {
        entity().x = 1;
    }

    void set_y() override {
        entity().y = 2;
    }
};

class ServerBuilder final : public Builder {
public:
    void set_x() override {
        entity().x = 10;
    }

    void set_y() override {
        entity().y = 20;
    }
};

}  // namespace builder_example

namespace abstract_factory_example {

class Entity {
public:
    virtual ~Entity() = default;
    virtual const char* name() const = 0;
};

class Client final : public Entity {
public:
    const char* name() const override {
        return "Client";
    }
};

class Server final : public Entity {
public:
    const char* name() const override {
        return "Server";
    }
};

class Factory {
public:
    virtual ~Factory() = default;
    virtual std::unique_ptr<Entity> make_entity() const = 0;
};

class ClientFactory final : public Factory {
public:
    std::unique_ptr<Entity> make_entity() const override {
        return std::make_unique<Client>();
    }
};

class ServerFactory final : public Factory {
public:
    std::unique_ptr<Entity> make_entity() const override {
        return std::make_unique<Server>();
    }
};

}  // namespace abstract_factory_example

namespace prototype_example {

class Entity {
public:
    virtual ~Entity() = default;
    virtual std::unique_ptr<Entity> copy() const = 0;
    virtual const char* name() const = 0;
};

class Client final : public Entity {
public:
    std::unique_ptr<Entity> copy() const override {
        return std::make_unique<Client>(*this);
    }

    const char* name() const override {
        return "Client";
    }
};

class Server final : public Entity {
public:
    std::unique_ptr<Entity> copy() const override {
        return std::make_unique<Server>(*this);
    }

    const char* name() const override {
        return "Server";
    }
};

class Prototype {
public:
    Prototype() {
        entities_.push_back(std::make_unique<Client>());
        entities_.push_back(std::make_unique<Server>());
    }

    std::unique_ptr<Entity> make_client() const {
        return entities_.at(0)->copy();
    }

    std::unique_ptr<Entity> make_server() const {
        return entities_.at(1)->copy();
    }

private:
    std::vector<std::unique_ptr<Entity>> entities_;
};

}  // namespace prototype_example

namespace composite_example {

class Entity {
public:
    virtual ~Entity() = default;
    virtual int test() const = 0;
};

class Client final : public Entity {
public:
    int test() const override {
        return 1;
    }
};

class Server final : public Entity {
public:
    int test() const override {
        return 2;
    }
};

class Composite final : public Entity {
public:
    void add(std::unique_ptr<Entity> entity) {
        entities_.push_back(std::move(entity));
    }

    int test() const override {
        int sum = 0;
        for (const auto& entity : entities_) {
            sum += entity->test();
        }
        return sum;
    }

private:
    std::vector<std::unique_ptr<Entity>> entities_;
};

std::unique_ptr<Entity> make_composite(std::size_t clients, std::size_t servers) {
    auto composite = std::make_unique<Composite>();

    for (std::size_t i = 0; i < clients; ++i) {
        composite->add(std::make_unique<Client>());
    }

    for (std::size_t i = 0; i < servers; ++i) {
        composite->add(std::make_unique<Server>());
    }

    return composite;
}

}  // namespace composite_example

namespace observer_example {

class Observer {
public:
    virtual ~Observer() = default;
    virtual void test(int x) = 0;
};

class Entity {
public:
    void add(std::shared_ptr<Observer> observer) {
        observers_.push_back(std::move(observer));
    }

    void set(int x) {
        x_ = x;
        notify_all();
    }

private:
    void notify_all() {
        for (const auto& observer : observers_) {
            observer->test(x_);
        }
    }

private:
    int x_ = 0;
    std::vector<std::shared_ptr<Observer>> observers_;
};

class RecordingObserver final : public Observer {
public:
    explicit RecordingObserver(std::string name) : name_(std::move(name)) {
    }

    void test(int x) override {
        events_.push_back(name_ + ":" + std::to_string(x));
    }

    const std::vector<std::string>& events() const {
        return events_;
    }

private:
    std::string name_;
    std::vector<std::string> events_;
};

}  // namespace observer_example

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

void test_builder_uses_unique_ptr(Logger& log) {
    std::unique_ptr<builder_example::Builder> builder = std::make_unique<builder_example::ClientBuilder>();
    std::unique_ptr<builder_example::Entity> entity = builder->make_entity();

    require(entity != nullptr, "builder must return entity");
    require(entity->x == 1, "client builder should set x");
    require(entity->y == 2, "client builder should set y");
    log.require(true, "05.01 builder uses unique_ptr");
}

void test_abstract_factory_uses_unique_ptr(Logger& log) {
    std::unique_ptr<abstract_factory_example::Factory> factory =
        std::make_unique<abstract_factory_example::ServerFactory>();
    std::unique_ptr<abstract_factory_example::Entity> entity = factory->make_entity();

    require(entity != nullptr, "factory must return entity");
    require(std::string(entity->name()) == "Server", "server factory should create server");
    log.require(true, "05.03 abstract factory uses unique_ptr");
}

void test_prototype_clones_with_unique_ptr(Logger& log) {
    const prototype_example::Prototype prototype;
    std::unique_ptr<prototype_example::Entity> client = prototype.make_client();
    std::unique_ptr<prototype_example::Entity> server = prototype.make_server();

    require(client != nullptr, "prototype should clone client");
    require(server != nullptr, "prototype should clone server");
    require(std::string(client->name()) == "Client", "cloned client should preserve type");
    require(std::string(server->name()) == "Server", "cloned server should preserve type");
    require(client.get() != server.get(), "clones must be distinct objects");
    log.require(true, "05.04 prototype uses unique_ptr");
}

void test_composite_owns_children_with_unique_ptr(Logger& log) {
    auto composite = std::make_unique<composite_example::Composite>();
    composite->add(composite_example::make_composite(1, 1));
    composite->add(composite_example::make_composite(1, 1));
    composite->add(composite_example::make_composite(1, 1));
    composite->add(composite_example::make_composite(1, 1));
    composite->add(composite_example::make_composite(1, 1));

    std::unique_ptr<composite_example::Entity> entity = std::move(composite);
    require(entity->test() == 15, "composite should sum nested children");
    log.require(true, "05.09 composite uses unique_ptr");
}

void test_observer_shares_subscribers_with_shared_ptr(Logger& log) {
    observer_example::Entity first;
    observer_example::Entity second;

    std::shared_ptr<observer_example::RecordingObserver> shared =
        std::make_shared<observer_example::RecordingObserver>("shared");
    std::shared_ptr<observer_example::RecordingObserver> local =
        std::make_shared<observer_example::RecordingObserver>("local");

    first.add(shared);
    first.add(local);
    second.add(shared);

    first.set(1);
    second.set(2);

    require(shared.use_count() == 3, "shared observer should be owned by caller and two subjects");
    require(local.use_count() == 2, "local observer should be owned by caller and one subject");
    require(shared->events().size() == 2, "shared observer should receive notifications from both subjects");
    require(local->events().size() == 1, "local observer should receive notifications from one subject");
    require(shared->events().at(0) == "shared:1", "first shared event should match");
    require(shared->events().at(1) == "shared:2", "second shared event should match");
    require(local->events().at(0) == "local:1", "local event should match");
    log.require(true, "05.13 observer uses shared_ptr");
}

void run_all() {
    Logger log;

    try {
        test_builder_uses_unique_ptr(log);
        test_abstract_factory_uses_unique_ptr(log);
        test_prototype_clones_with_unique_ptr(log);
        test_composite_owns_children_with_unique_ptr(log);
        test_observer_shares_subscribers_with_shared_ptr(log);
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
