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

template <typename... Ts>
struct Deque {};

template <typename D>
struct Size;

template <typename... Ts>
struct Size<Deque<Ts...>> {
    static constexpr std::size_t value = sizeof...(Ts);
};

template <typename D>
inline constexpr std::size_t size_v = Size<D>::value;

template <typename D>
inline constexpr bool is_empty_v = (size_v<D> == 0u);

template <typename D>
struct Front;

template <typename T, typename... Ts>
struct Front<Deque<T, Ts...>> {
    using type = T;
};

template <typename D>
using front = typename Front<D>::type;

template <typename T, typename D>
struct Push_Front;

template <typename T, typename... Ts>
struct Push_Front<T, Deque<Ts...>> {
    using type = Deque<T, Ts...>;
};

template <typename T, typename D>
using push_front = typename Push_Front<T, D>::type;

template <typename D>
struct Pop_Front;

template <typename T, typename... Ts>
struct Pop_Front<Deque<T, Ts...>> {
    using type = Deque<Ts...>;
};

template <typename D>
using pop_front = typename Pop_Front<D>::type;

template <typename D>
struct Back;

template <typename T>
struct Back<Deque<T>> {
    using type = T;
};

template <typename T, typename... Ts>
struct Back<Deque<T, Ts...>> {
    using type = typename Back<Deque<Ts...>>::type;
};

template <typename D>
using back = typename Back<D>::type;

template <typename T, typename D, bool C = is_empty_v<D>>
struct Push_Back;

template <typename T, typename D>
struct Push_Back<T, D, false> {
    using type = push_front<front<D>, typename Push_Back<T, pop_front<D>>::type>;
};

template <typename T, typename D>
struct Push_Back<T, D, true> {
    using type = push_front<T, D>;
};

template <typename T, typename D>
using push_back = typename Push_Back<T, D>::type;

template <typename D>
struct Pop_Back;

template <typename T>
struct Pop_Back<Deque<T>> {
    using type = Deque<>;
};

template <typename T, typename... Ts>
struct Pop_Back<Deque<T, Ts...>> {
    using type = push_front<T, typename Pop_Back<Deque<Ts...>>::type>;
};

template <typename D>
using pop_back = typename Pop_Back<D>::type;

template <typename D, std::size_t I>
class Nth : public Nth<pop_front<D>, I - 1> {};

template <typename D>
class Nth<D, 0> : public Front<D> {};

template <typename D, std::size_t I>
using nth = typename Nth<D, I>::type;

template <typename D, bool C = is_empty_v<D>>
class Max_Type;

template <typename D>
class Max_Type<D, false> {
private:
    using current_t = front<D>;
    using max_t = typename Max_Type<pop_front<D>>::type;

public:
    using type = std::conditional_t<(sizeof(current_t) >= sizeof(max_t)), current_t, max_t>;
};

template <typename D>
class Max_Type<D, true> {
public:
    using type = std::byte;
};

template <typename D>
using max_type = typename Max_Type<D>::type;

template <typename T, typename D, bool C = is_empty_v<D>>
struct Has;

template <typename T, typename D>
struct Has<T, D, true> {
    static constexpr bool value = false;
};

template <typename T, typename D>
struct Has<T, D, false> {
private:
    using first_t = front<D>;
    using rest_t = pop_front<D>;

public:
    static constexpr bool value = std::is_same_v<T, first_t> || Has<T, rest_t>::value;
};

template <typename T, typename D>
inline constexpr bool has_v = Has<T, D>::value;

struct A {};
struct B {};
struct C {};

static_assert(size_v<Deque<>> == 0u);
static_assert(is_empty_v<Deque<>>);

static_assert(size_v<Deque<int>> == 1u);
static_assert(!is_empty_v<Deque<int>>);

static_assert(std::is_same_v<front<Deque<int, double>>, int>);
static_assert(std::is_same_v<back<Deque<int, double, char>>, char>);

static_assert(std::is_same_v<push_front<int, Deque<>>, Deque<int>>);
static_assert(std::is_same_v<push_back<int, Deque<>>, Deque<int>>);

static_assert(std::is_same_v<pop_front<Deque<int>>, Deque<>>);
static_assert(std::is_same_v<pop_back<Deque<int>>, Deque<>>);

static_assert(std::is_same_v<nth<Deque<int, double, char>, 0>, int>);
static_assert(std::is_same_v<nth<Deque<int, double, char>, 1>, double>);
static_assert(std::is_same_v<nth<Deque<int, double, char>, 2>, char>);

static_assert(std::is_same_v<max_type<Deque<int, double>>, double>);

static_assert(has_v<int, Deque<>> == false);
static_assert(has_v<int, Deque<int>> == true);
static_assert(has_v<double, Deque<int>> == false);
static_assert(has_v<int, Deque<int, double, char>> == true);
static_assert(has_v<double, Deque<int, double, char>> == true);
static_assert(has_v<char, Deque<int, double, char>> == true);
static_assert(has_v<float, Deque<int, double, char>> == false);

static_assert(has_v<A, Deque<A, B, C>> == true);
static_assert(has_v<B, Deque<A, B, C>> == true);
static_assert(has_v<C, Deque<A, B, C>> == true);
static_assert(has_v<int, Deque<A, B, C>> == false);

int main() {
    return 0;
}
