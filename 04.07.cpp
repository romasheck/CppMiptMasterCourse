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
#include <numeric>
#include <type_traits>
#include <utility>

namespace ct_int {

consteval bool fits_int(long long v) {
    return v >= static_cast<long long>(std::numeric_limits<int>::min()) &&
           v <= static_cast<long long>(std::numeric_limits<int>::max());
}

consteval int checked_add(int a, int b) {
    const long long s = static_cast<long long>(a) + static_cast<long long>(b);
    if (!fits_int(s)) {
        throw "int overflow in add";
    }
    return static_cast<int>(s);
}

consteval int checked_mul(int a, int b) {
    const long long p = static_cast<long long>(a) * static_cast<long long>(b);
    if (!fits_int(p)) {
        throw "int overflow in mul";
    }
    return static_cast<int>(p);
}

consteval int checked_neg(int a) {
    if (a == std::numeric_limits<int>::min()) {
        throw "int overflow in neg";
    }
    return -a;
}

}  // namespace ct_int

template <int N = 0, int D = 1>
struct Ratio {
    static_assert(D != 0, "zero denominator");
    static constexpr int num = N;
    static constexpr int den = D;
};

template <typename R>
struct Simplify {
private:
    static constexpr int g = std::gcd(R::num, R::den);
    static constexpr int n0 = R::num / g;
    static constexpr int d0 = R::den / g;

    static constexpr int n1 = (d0 < 0) ? ct_int::checked_neg(n0) : n0;
    static constexpr int d1 = (d0 < 0) ? -d0 : d0;

public:
    static constexpr int num = n1;
    static constexpr int den = d1;
    using type = Ratio<num, den>;
};

template <typename R>
using simplify = typename Simplify<R>::type;

template <typename R1, typename R2>
struct Sum {
private:
    static constexpr int n1 = ct_int::checked_mul(R1::num, R2::den);
    static constexpr int n2 = ct_int::checked_mul(R2::num, R1::den);
    static constexpr int n = ct_int::checked_add(n1, n2);
    static constexpr int d = ct_int::checked_mul(R1::den, R2::den);

public:
    using type = simplify<Ratio<n, d>>;
};

template <typename R1, typename R2>
using sum = typename Sum<R1, R2>::type;

template <typename R1, typename R2>
struct Sub {
    using type = sum<R1, Ratio<ct_int::checked_neg(R2::num), R2::den>>;
};

template <typename R1, typename R2>
using sub = typename Sub<R1, R2>::type;

template <typename R1, typename R2>
struct Mul {
private:
    static constexpr int n = ct_int::checked_mul(R1::num, R2::num);
    static constexpr int d = ct_int::checked_mul(R1::den, R2::den);

public:
    using type = simplify<Ratio<n, d>>;
};

template <typename R1, typename R2>
using mul = typename Mul<R1, R2>::type;

template <typename R1, typename R2>
struct Div {
    static_assert(R2::num != 0, "div by zero");
    using type = mul<R1, Ratio<R2::den, R2::num>>;
};

template <typename R1, typename R2>
using div = typename Div<R1, R2>::type;

template <typename T, typename R = Ratio<1>>
struct Duration {
    T x{};

    constexpr Duration() = default;
    constexpr explicit Duration(T v) : x(v) {
    }
};

template <typename T1, typename R1, typename T2, typename R2>
constexpr auto operator+(Duration<T1, R1> const& lhs, Duration<T2, R2> const& rhs) {
    using common_t = std::common_type_t<T1, T2>;
    using ratio_t = Ratio<1, sum<R1, R2>::den>;

    const common_t left_scaled =
        static_cast<common_t>(lhs.x) * static_cast<common_t>(ratio_t::den / R1::den) *
        static_cast<common_t>(R1::num);

    const common_t right_scaled =
        static_cast<common_t>(rhs.x) * static_cast<common_t>(ratio_t::den / R2::den) *
        static_cast<common_t>(R2::num);

    const auto x = left_scaled + right_scaled;
    return Duration<decltype(x), ratio_t>{x};
}

template <typename T1, typename R1, typename T2, typename R2>
constexpr auto operator-(Duration<T1, R1> const& lhs, Duration<T2, R2> const& rhs) {
    return lhs + Duration<T2, Ratio<ct_int::checked_neg(R2::num), R2::den>>{rhs.x};
}

using R1 = Ratio<1, 2>;
using R2 = Ratio<1, 3>;

static_assert(sum<R1, R2>::num == 5);
static_assert(sum<R1, R2>::den == 6);

static_assert(sub<R1, R2>::num == 1);
static_assert(sub<R1, R2>::den == 6);

static_assert(mul<R1, R2>::num == 1);
static_assert(mul<R1, R2>::den == 6);

static_assert(div<R1, R2>::num == 3);
static_assert(div<R1, R2>::den == 2);

using R3 = Ratio<2, 4>;
static_assert(simplify<R3>::num == 1);
static_assert(simplify<R3>::den == 2);

using R4 = Ratio<4, 8>;
static_assert(mul<R3, R4>::num == 1);
static_assert(mul<R3, R4>::den == 4);

static_assert(simplify<Ratio<1, -2>>::num == -1);
static_assert(simplify<Ratio<1, -2>>::den == 2);

constexpr Duration<int, Ratio<1, 2>> d1{1};
constexpr Duration<int, Ratio<1, 3>> d2{2};

constexpr auto dsum = d1 + d2;
static_assert(dsum.x == 7);

constexpr auto dsub = d1 - d2;
static_assert(dsub.x == -1);

int main() {
    return 0;
}
