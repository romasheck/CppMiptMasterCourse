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

#include <type_traits>

template <typename T>
struct is_class {
private:
    template <typename U>
    static char test(int U::*);

    template <typename U>
    static int test(...);

public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(char);
};

template <typename T>
inline constexpr bool is_class_v = is_class<T>::value;

template <typename T>
struct add_const {
    using type = const T;
};

template <typename T>
using add_const_t = typename add_const<T>::type;

template <typename T>
inline constexpr bool add_const_v = std::is_same_v<add_const_t<T>, const T>;

template <typename T>
struct remove_const {
    using type = T;
};

template <typename T>
struct remove_const<const T> {
    using type = T;
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

template <typename T>
inline constexpr bool remove_const_v = std::is_same_v<remove_const_t<T>, std::remove_const_t<T>>;

template <bool B, typename T, typename F>
struct conditional {
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F> {
    using type = F;
};

template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

template <bool B, typename T, typename F>
inline constexpr bool conditional_is_t = std::is_same_v<conditional_t<B, T, F>, std::conditional_t<B, T, F>>;

template <typename T>
struct decay {
private:
    using U0 = std::remove_reference_t<T>;
    using U = remove_const_t<U0>;

public:
    using type = conditional_t<std::is_array_v<U>, std::add_pointer_t<std::remove_extent_t<U>>,
                               conditional_t<std::is_function_v<U>, std::add_pointer_t<U>, U>>;
};

template <typename T>
using decay_t = typename decay<T>::type;

template <typename T>
inline constexpr bool decay_matches_std_v = std::is_same_v<decay_t<T>, std::decay_t<T>>;

struct MyClass {};
union MyUnion {
    int a;
};
enum class MyEnum { A, B };

static_assert(is_class_v<MyClass>);
static_assert(is_class_v<MyUnion>);
static_assert(!is_class_v<int>);
static_assert(!is_class_v<int*>);
static_assert(!is_class_v<MyEnum>);

static_assert(std::is_same_v<add_const_t<int>, const int>);
static_assert(std::is_same_v<add_const_t<const int>, const int>);
static_assert(add_const_v<int>);

static_assert(std::is_same_v<remove_const_t<int>, int>);
static_assert(std::is_same_v<remove_const_t<const int>, int>);
static_assert(remove_const_v<const int>);

static_assert(std::is_same_v<conditional_t<true, int, double>, int>);
static_assert(std::is_same_v<conditional_t<false, int, double>, double>);
static_assert(conditional_is_t<true, int, double>);
static_assert(conditional_is_t<false, int, double>);

static_assert(std::is_same_v<decay_t<int&>, int>);
static_assert(std::is_same_v<decay_t<const int&>, int>);
static_assert(std::is_same_v<decay_t<int[]>, int*>);
static_assert(std::is_same_v<decay_t<void(int)>, void (*)(int)>);
static_assert(decay_matches_std_v<int&>);
static_assert(decay_matches_std_v<const int&>);
static_assert(decay_matches_std_v<int[]>);
static_assert(decay_matches_std_v<void(int)>);

int main() {
    return 0;
}
