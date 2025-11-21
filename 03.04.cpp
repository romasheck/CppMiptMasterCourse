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

class Entity
{
private:
    static int test_v1(int value)
    {
        return value + value;
    }

    static int test_v2(int value)
    {
        return value;
    }

    friend class EntityAttorney_v1;
    friend class EntityAttorney_v2;
};

class EntityAttorney_v1
{
private:
    friend class Tester_v1;

    static int call_test_v1(int value)
    {
        return Entity::test_v1(value);
    }
};


class EntityAttorney_v2
{
private:
    friend class Tester_v2;

    static int call_test_v2(int value)
    {
        return Entity::test_v2(value);
    }
};


class Tester_v1
{
public:
    static void run_tests()
    {
        {
            const int input = 5;
            const int expected = 10;
            const int result = EntityAttorney_v1::call_test_v1(input);
            assert(result == expected);
        }
        
        {
            const int input = 0;
            const int expected = 0;
            const int result = EntityAttorney_v1::call_test_v1(input);
            assert(result == expected);
        }
    }
};

class Tester_v2
{
public:
    static void run_tests()
    {
       {
            const int input = 5;
            const int expected = 6;
            const int result = EntityAttorney_v2::call_test_v2(input);
            assert(result == expected);
        }
        
        {
            const int input = 0;
            const int expected = 1;
            const int result = EntityAttorney_v2::call_test_v2(input);
            assert(result == expected);
        }
        
        {
            const int input = -3;
            const int expected = -2;
            const int result = EntityAttorney_v2::call_test_v2(input);
            assert(result == expected);
        }
        
        {
            const int input = 999;
            const int expected = 1000;
            const int result = EntityAttorney_v2::call_test_v2(input);
            assert(result == expected);
        }
    }
};

int main()
{
    Tester_v1::run_tests();    
    Tester_v2::run_tests();
    
    return 0;
}