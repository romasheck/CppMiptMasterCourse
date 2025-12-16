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

#include <limits>

template <int N>
struct Fibonacci {
    static_assert(N >= 1, "index must be positive");

    static_assert(Fibonacci<N - 2>::value <=
                      std::numeric_limits<int>::max() - Fibonacci<N - 1>::value,
                  "overflow");

    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template <>
struct Fibonacci<1> {
    static constexpr int value = 1;
};

template <>
struct Fibonacci<2> {
    static constexpr int value = 1;
};

template <int N>
inline constexpr int fibonacci_v = Fibonacci<N>::value;

static_assert(fibonacci_v<1> == 1);
static_assert(fibonacci_v<2> == 1);
static_assert(fibonacci_v<3> == 2);
static_assert(fibonacci_v<4> == 3);
static_assert(fibonacci_v<5> == 5);
static_assert(fibonacci_v<6> == 8);
static_assert(fibonacci_v<7> == 13);
static_assert(fibonacci_v<8> == 21);
static_assert(fibonacci_v<9> == 34);
static_assert(fibonacci_v<10> == 55);

int main() {
    return 0;
}
