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
#include <vector>
#include <cassert>

class Entity_v1
{
public :
    virtual ~Entity_v1()
    {
        std::cout << "Entity_v1::~Entity_v1\n";
    }

    virtual void test(int & m_value) const = 0;
};

class Entity_v2
{
public :
    virtual ~Entity_v2()
    {
        std::cout << "Entity_v2::~Entity_v2\n";
    }

    virtual void test(int & m_value) const = 0;
};

class Adapter_v1 : public Entity_v1
{
public :
    virtual ~Adapter_v1()
    {
        std::cout << "Adapter_v1::~Adapter_v1\n";
    }

    void test(int & m_value) const override
    {
        this->test_v1(m_value);
    }

protected :
    virtual void test_v1(int & m_value) const
    {
        m_value += 1;
        std::cout << "Adapter_v1::test_v1 (default) -> +1\n";
    }
};

class Adapter_v2 : public Entity_v2
{
public :
    virtual
    ~Adapter_v2()
    {
        std::cout << "Adapter_v2::~Adapter_v2\n";
    }

    void test(int & m_value) const override
    {
        this->test_v2(m_value);
    }

protected :
    virtual void test_v2(int & m_value) const
    {
        m_value *= 2;
        std::cout << "Adapter_v2::test_v2 (default) -> *2\n";
    }
};

class Client : public Adapter_v1, public Adapter_v2
{
public :
    Client()
    {
    }

    ~Client() override
    {
        std::cout << "Client::~Client\n";
    }

protected :
    void test_v1(int & m_value) const override
    {
        m_value += 1;
        std::cout << "Client::test_v1 -> +1 (entity_1 behavior)\n";
    }

    void test_v2(int & m_value) const override
    {
        m_value *= 2;
        std::cout << "Client::test_v2 -> *2 (entity_2 behavior)\n";
    }
};

int main()
{
    Client * client = new Client;

    Entity_v1 * e1 = client; 
    Entity_v2 * e2 = client; 

    int value1 = 5;
    e1->test(value1);
    assert(value1 == 6);

    int value2 = 7;
    e2->test(value2);
    assert(value2 == 14);

    delete client;

    return 0;
}
