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

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>

template <typename T>
class Vector {
public:
    Vector() : m_array(nullptr), m_size(0), m_capacity(0) {
    }

    Vector(std::initializer_list<T> list) : m_size(std::size(list)), m_capacity(std::size(list)) {
        m_array = m_size ? new T[m_size]{} : nullptr;
        std::ranges::copy(list, m_array);
    }

    Vector(Vector const& other)
        : m_array(nullptr), m_size(other.m_size), m_capacity(other.m_size) {
        if (m_capacity != 0u) {
            m_array = new T[m_capacity]{};
            std::copy_n(other.m_array, m_size, m_array);
        }
    }

    Vector(Vector&& other)
        : m_array(std::exchange(other.m_array, nullptr)),
          m_size(std::exchange(other.m_size, 0u)),
          m_capacity(std::exchange(other.m_capacity, 0u)) {
    }

    ~Vector() {
        delete[] m_array;
    }

    Vector& operator=(Vector other) {
        swap(other);
        return *this;
    }

    void swap(Vector& other) {
        std::swap(m_array, other.m_array);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    std::size_t capacity() const {
        return m_capacity;
    }

    std::size_t size() const {
        return m_size;
    }

    bool empty() const {
        return m_size == 0u;
    }

    void clear() {
        m_size = 0u;
    }

    void push_back(T value) {
        grant_capacity_for_one_more();
        m_array[m_size++] = std::move(value);
    }

private:
    void grant_capacity_for_one_more() {
        if (m_size < m_capacity) {
            return;
        }

        const std::size_t new_capacity = m_capacity ? m_capacity * 2u : 1u;
        T* new_array = new T[new_capacity]{};

        if (m_size != 0u) {
            std::copy_n(m_array, m_size, new_array);
        }

        delete[] m_array;
        m_array = new_array;
        m_capacity = new_capacity;
    }

private:
    T* m_array;
    std::size_t m_size;
    std::size_t m_capacity;
};

template <typename T>
void swap(Vector<T>& lhs, Vector<T>& rhs) {
    lhs.swap(rhs);
}

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

template <typename T>
void test_default_ctor(Logger& log, const std::string& name) {
    Vector<T> v;
    require(v.size() == 0u);
    require(v.capacity() == 0u);
    require(v.empty());
    log.pass(name);
}

template <typename T>
void test_init_list_and_clear(Logger& log, const std::string& name, std::initializer_list<T> init,
                              T after_clear_value) {
    Vector<T> v = init;
    require(v.size() == init.size());
    require(v.capacity() == init.size());
    require(!v.empty());

    v.clear();
    require(v.size() == 0u);
    require(v.capacity() == init.size());
    require(v.empty());

    v.push_back(after_clear_value);
    require(v.size() == 1u);
    require(v.capacity() == init.size());
    log.pass(name);
}

template <typename T>
void test_copy_ctor(Logger& log, const std::string& name, std::initializer_list<T> init) {
    Vector<T> a = init;
    Vector<T> b = a;
    require(b.size() == a.size());
    require(b.capacity() == a.size());
    log.pass(name);
}

template <typename T>
void test_move_ctor(Logger& log, const std::string& name, std::initializer_list<T> init) {
    Vector<T> a = init;
    Vector<T> b = a;
    Vector<T> c = std::move(b);

    require(c.size() == a.size());
    require(c.capacity() == a.size());
    require(b.size() == 0u);
    require(b.capacity() == 0u);
    log.pass(name);
}

template <typename T>
void test_growth_and_capacity(Logger& log, const std::string& name, T base_value) {
    Vector<T> v;

    std::size_t expected_capacity = 0u;
    for (int i = 0; i < 10; ++i) {
        v.push_back(static_cast<T>(base_value + static_cast<T>(i)));
        if (expected_capacity == 0u) {
            expected_capacity = 1u;
        }
        while (expected_capacity <= static_cast<std::size_t>(i)) {
            expected_capacity *= 2u;
        }
        require(v.capacity() == expected_capacity);
        require(v.size() == static_cast<std::size_t>(i + 1));
    }
    require(v.capacity() == 16u);

    const std::size_t cap_before_clear = v.capacity();
    v.clear();
    require(v.size() == 0u);
    require(v.capacity() == cap_before_clear);
    require(v.empty());

    v.push_back(static_cast<T>(base_value));
    require(v.size() == 1u);
    require(v.capacity() == cap_before_clear);

    log.pass(name);
}

template <typename T>
void test_copy_assignment(Logger& log, const std::string& name, std::initializer_list<T> init) {
    Vector<T> a = init;
    Vector<T> b;
    b = a;
    require(b.size() == a.size());
    require(b.capacity() == a.size());
    log.pass(name);
}

template <typename T>
void test_move_assignment(Logger& log, const std::string& name, std::initializer_list<T> init) {
    Vector<T> v;
    v = Vector<T>{init};
    require(v.size() == init.size());
    require(v.capacity() == init.size());
    log.pass(name);
}

template <typename T>
void test_adl_swap(Logger& log, const std::string& name, std::initializer_list<T> a,
                   std::initializer_list<T> b) {
    Vector<T> v1 = a;
    Vector<T> v2 = b;
    swap(v1, v2);
    require(v1.size() == b.size());
    require(v2.size() == a.size());
    log.pass(name);
}

void run_all() {
    Logger log;

    test_default_ctor<int>(log, "Vector<int> default");
    test_init_list_and_clear<int>(log, "Vector<int> init_list+clear", {1, 2, 3, 4, 5}, 10);
    test_copy_ctor<int>(log, "Vector<int> copy_ctor", {10});
    test_move_ctor<int>(log, "Vector<int> move_ctor", {10});
    test_growth_and_capacity<int>(log, "Vector<int> growth", 0);
    test_copy_assignment<int>(log, "Vector<int> copy_assign", {10});
    test_move_assignment<int>(log, "Vector<int> move_assign", {7, 8, 9});
    test_adl_swap<int>(log, "Vector<int> swap", {10}, {7, 8, 9});

    test_default_ctor<double>(log, "Vector<double> default");
    test_init_list_and_clear<double>(log, "Vector<double> init_list+clear", {1.1, 2.2, 3.3, 4.4, 5.5},
                                     10.5);
    test_copy_ctor<double>(log, "Vector<double> copy_ctor", {10.5});
    test_move_ctor<double>(log, "Vector<double> move_ctor", {10.5});
    test_growth_and_capacity<double>(log, "Vector<double> growth", 0.1);
    test_copy_assignment<double>(log, "Vector<double> copy_assign", {10.5});
    test_move_assignment<double>(log, "Vector<double> move_assign", {7.1, 8.2, 9.3});
    test_adl_swap<double>(log, "Vector<double> swap", {10.5}, {7.1, 8.2, 9.3});

    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
