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
#include <string>
#include <utility>

class Person {
public:
    Person() = default;

    const std::string& get_name() const noexcept {
        return m_name;
    }

    int get_age() const noexcept {
        return m_age;
    }

    int get_grade() const noexcept {
        return m_grade;
    }

private:
    friend class Builder;

    std::string m_name{};
    int m_age = 0;
    int m_grade = 0;
};

class Builder {
public:
    Builder() = default;

    Builder& name(std::string value) {
        m_person.m_name = std::move(value);
        return *this;
    }

    Builder& age(int value) {
        m_person.m_age = value;
        return *this;
    }

    Builder& grade(int value) {
        m_person.m_grade = value;
        return *this;
    }

    [[nodiscard]] Person get() const {
        return m_person;
    }

private:
    Person m_person{};
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

void test_full_chain(Logger& log) {
    Builder builder;
    const Person p = builder.name("Ivan").age(25).grade(10).get();

    require(p.get_name() == "Ivan");
    require(p.get_age() == 25);
    require(p.get_grade() == 10);

    log.pass("full_chain_sets_all_fields");
}

void test_default_person(Logger& log) {
    const Person p = Builder().get();

    require(p.get_name().empty());
    require(p.get_age() == 0);
    require(p.get_grade() == 0);

    log.pass("default_builder_produces_default_person");
}

void test_partial_build(Logger& log) {
    Builder builder;
    const Person p = builder.name("Mila").get();

    require(p.get_name() == "Mila");
    require(p.get_age() == 0);
    require(p.get_grade() == 0);

    log.pass("partial_build_name_only");
}

void test_overwrite_fields(Logger& log) {
    Builder builder;
    const Person p = builder.name("A").name("B").age(18).age(19).grade(7).grade(8).get();

    require(p.get_name() == "B");
    require(p.get_age() == 19);
    require(p.get_grade() == 8);

    log.pass("overwrite_fields_last_value_wins");
}

void test_person_is_snapshot(Logger& log) {
    Builder builder;
    const Person p1 = builder.name("Alex").age(20).grade(9).get();

    builder.name("Chris").age(30).grade(11);
    const Person p2 = builder.get();

    require(p1.get_name() == "Alex");
    require(p1.get_age() == 20);
    require(p1.get_grade() == 9);

    require(p2.get_name() == "Chris");
    require(p2.get_age() == 30);
    require(p2.get_grade() == 11);

    log.pass("get_returns_snapshot_independent_of_future_changes");
}

void test_temporary_builder_chain(Logger& log) {
    const Person p = Builder().name("Zoe").grade(12).get();

    require(p.get_name() == "Zoe");
    require(p.get_age() == 0);
    require(p.get_grade() == 12);

    log.pass("temporary_builder_chain_works");
}

void run_all() {
    Logger log;
    test_full_chain(log);
    test_default_person(log);
    test_partial_build(log);
    test_overwrite_fields(log);
    test_person_is_snapshot(log);
    test_temporary_builder_chain(log);
    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
