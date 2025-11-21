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

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <utility>

class Vector
{
public:

    Vector() : m_array(nullptr), m_size(0), m_capacity(0)
    {
        std::cout << ("Vector::Vector (default)\n");
    }

    Vector(std::initializer_list<int> list)
    : m_size(std::size(list)),
      m_capacity(std::size(list))
    {
        std::cout << ("Vector::Vector (init-list)\n");
        m_array = m_size ? new int[m_size]{} : nullptr;
        std::ranges::copy(list, m_array);
    }

    Vector(Vector const & other)
    : m_array(nullptr),
      m_size(other.m_size),
      m_capacity(other.m_size)
    {
        std::cout << ("Vector::Vector (copy)\n");

        if (m_capacity != 0uz)
        {
            m_array = new int[m_capacity]{};
            std::copy_n(other.m_array, m_size, m_array);
        }
    }

    Vector(Vector && other) 
    : m_array(std::exchange(other.m_array, nullptr)),
      m_size(std::exchange(other.m_size, 0uz)),
      m_capacity(std::exchange(other.m_capacity, 0uz))
    {
        std::cout << ("Vector::Vector (move)\n");
    }

    ~Vector()
    {
        std::cout << ("Vector::~Vector\n");
        delete[] m_array;
    }

    Vector & operator=(Vector other)
    {
        std::cout << ("Vector::operator=\n");
        swap(other);
        return *this;
    }

    void swap(Vector & other) 
    {
        std::swap(m_array, other.m_array);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    std::size_t capacity() const 
    {
        return m_capacity;
    }

    std::size_t size() const 
    {
        return m_size;
    }

    bool empty() const 
    {
        return m_size == 0uz;
    }

    void clear() 
    {
        m_size = 0uz;
    }

    void push_back(int value)
    {
        grant_capacity_for_one_more();
        m_array[m_size++] = value;
    }

private:

    void grant_capacity_for_one_more()
    {
        if (m_size < m_capacity)
            return;

        std::size_t new_capacity = m_capacity ? m_capacity * 2 : 1;

        int * new_array = new int[new_capacity]{};

        if (m_size != 0uz)
            std::copy_n(m_array, m_size, new_array);

        delete[] m_array;
        m_array = new_array;
        m_capacity = new_capacity;
    }

private:

    int * m_array;
    std::size_t m_size;
    std::size_t m_capacity;
};

void swap(Vector & lhs, Vector & rhs)
{
    lhs.swap(rhs);
}

int main()
{
    Vector v0;
    assert(v0.size() == 0uz);
    assert(v0.capacity() == 0uz);
    assert(v0.empty());

    Vector v1 = {1, 2, 3, 4, 5};
    assert(v1.size() == 5uz);
    assert(v1.capacity() == 5uz);
    assert(!v1.empty());
    v1.clear();
    v1.push_back(10);

    Vector v2 = v1;
    assert(v2.size() == 1uz);
    assert(v2.capacity() == 1uz);

    Vector v3 = std::move(v2);
    assert(v3.size() == 1uz);
    assert(v3.capacity() == 1uz);
    assert(v2.size() == 0uz);
    assert(v2.capacity() == 0uz);

    Vector v4;
    std::size_t expected_capacity = 0uz;
    for (int i = 0; i < 10; ++i)
    {
        v4.push_back(i);
        if (expected_capacity == 0uz)
            expected_capacity = 1uz;
        while (expected_capacity <= static_cast<std::size_t>(i))
            expected_capacity *= 2;
        assert(v4.capacity() == expected_capacity);
        assert(v4.size() == static_cast<std::size_t>(i + 1));
    }
    assert(v4.capacity() == 16uz);

    auto cap_before_clear = v4.capacity();
    v4.clear();
    assert(v4.size() == 0uz);
    assert(v4.capacity() == cap_before_clear);
    assert(v4.empty());

    v4.push_back(42);
    assert(v4.size() == 1uz);
    assert(v4.capacity() == cap_before_clear);

    Vector v5;
    v5 = v1;
    assert(v5.size() == 1uz);
    assert(v5.capacity() == 1uz);

    Vector v6;
    v6 = Vector{7, 8, 9};
    assert(v6.size() == 3uz);
    assert(v6.capacity() == 3uz);

    swap(v5, v6);
    assert(v5.size() == 3uz);
    assert(v6.size() == 1uz);

    return 0;
}
