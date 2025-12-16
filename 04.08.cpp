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

#include <cstddef>
#include <type_traits>
#include <utility>

template <typename... Ts>
class Tuple {
public:
    constexpr Tuple() = default;

    constexpr std::size_t size() const noexcept {
        return sizeof...(Ts);
    }
};

template <typename T, typename... Ts>
class Tuple<T, Ts...> {
public:
    constexpr Tuple(T&& x, Ts&&... xs)
        : m_head(std::forward<T>(x)), m_tail(std::forward<Ts>(xs)...) {
    }

    constexpr std::size_t size() const noexcept {
        return 1u + sizeof...(Ts);
    }

    template <std::size_t I>
    constexpr decltype(auto) get() const {
        if constexpr (I > 0) {
            return m_tail.template get<I - 1>();
        } else {
            return (m_head);
        }
    }

private:
    T m_head;
    Tuple<Ts...> m_tail;
};

static_assert(Tuple<>{}.size() == 0u);

constexpr Tuple<int> single_tuple(42);
static_assert(single_tuple.size() == 1u);
static_assert(single_tuple.get<0>() == 42);

constexpr Tuple<int, double, char> multi_tuple(10, 3.14, 'A');
static_assert(multi_tuple.size() == 3u);
static_assert(multi_tuple.get<0>() == 10);
static_assert(multi_tuple.get<1>() == 3.14);
static_assert(multi_tuple.get<2>() == 'A');

constexpr Tuple<int, Tuple<double, char>> nested_tuple(5, Tuple<double, char>(3.14, 'X'));
static_assert(nested_tuple.size() == 2u);
static_assert(nested_tuple.get<0>() == 5);
static_assert(nested_tuple.get<1>().size() == 2u);
static_assert(nested_tuple.get<1>().get<0>() == 3.14);
static_assert(nested_tuple.get<1>().get<1>() == 'X');

int main() {
    return 0;
}
