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
#include <numeric>
#include <stdexcept>
#include <string>

// Mixin classes for operators
template <typename T>
class addable {
    friend T operator+(T lhs, const T& rhs) {
        lhs += rhs;
        return lhs;
    }
};

template <typename T>
class subtractable {
    friend T operator-(T lhs, const T& rhs) {
        lhs -= rhs;
        return lhs;
    }
};

template <typename T>
class multipliable {
    friend T operator*(T lhs, const T& rhs) {
        lhs *= rhs;
        return lhs;
    }
};

template <typename T>
class dividable {
    friend T operator/(T lhs, const T& rhs) {
        lhs /= rhs;
        return lhs;
    }
};

template <typename T>
class incrementable {
    friend T& operator++(T& x) {
        x += T(1);
        return x;
    }
    friend T operator++(T& x, int) {
        T temp = x;
        x += T(1);
        return temp;
    }
};

template <typename T>
class decrementable {
    friend T& operator--(T& x) {
        x -= T(1);
        return x;
    }
    friend T operator--(T& x, int) {
        T temp = x;
        x -= T(1);
        return temp;
    }
};

template <typename T>
class Rational : public addable<Rational<T>>,
                 public subtractable<Rational<T>>,
                 public multipliable<Rational<T>>,
                 public dividable<Rational<T>>,
                 public incrementable<Rational<T>>,
                 public decrementable<Rational<T>> {
private:
    T num, den;

    void reduce() {
        T g = std::gcd(num, den);
        num /= g;
        den /= g;
        if (den < 0) {
            num = -num;
            den = -den;
        }
    }

public:
    Rational(T n = 0, T d = 1) : num(n), den(d) {
        if (den == 0) throw std::invalid_argument("Denominator cannot be zero");
        reduce();
    }

    Rational& operator+=(const Rational& other) {
        num = num * other.den + other.num * den;
        den *= other.den;
        reduce();
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        num = num * other.den - other.num * den;
        den *= other.den;
        reduce();
        return *this;
    }

    Rational& operator*=(const Rational& other) {
        num *= other.num;
        den *= other.den;
        reduce();
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        if (other.num == 0) throw std::invalid_argument("Division by zero");
        num *= other.den;
        den *= other.num;
        reduce();
        return *this;
    }

    bool operator==(const Rational& rhs) const {
        return num == rhs.num && den == rhs.den;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rational& r) {
        os << r.num << "/" << r.den;
        return os;
    }
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
    if (!ok) throw std::runtime_error("Test failed");
}

template <typename T>
void test_construction(Logger& log, const std::string& name) {
    Rational<T> r(1, 2);
    require(r == Rational<T>(1, 2));
    log.pass(name);
}

template <typename T>
void test_addition(Logger& log, const std::string& name) {
    Rational<T> a(1, 2);
    Rational<T> b(1, 3);
    Rational<T> c = a + b;
    require(c == Rational<T>(5, 6));
    log.pass(name);
}

template <typename T>
void test_subtraction(Logger& log, const std::string& name) {
    Rational<T> a(1, 2);
    Rational<T> b(1, 3);
    Rational<T> c = a - b;
    require(c == Rational<T>(1, 6));
    log.pass(name);
}

template <typename T>
void test_multiplication(Logger& log, const std::string& name) {
    Rational<T> a(1, 2);
    Rational<T> b(1, 3);
    Rational<T> c = a * b;
    require(c == Rational<T>(1, 6));
    log.pass(name);
}

template <typename T>
void test_division(Logger& log, const std::string& name) {
    Rational<T> a(1, 2);
    Rational<T> b(1, 3);
    Rational<T> c = a / b;
    require(c == Rational<T>(3, 2));
    log.pass(name);
}

template <typename T>
void test_increment(Logger& log, const std::string& name) {
    Rational<T> a(1, 2);
    ++a;
    require(a == Rational<T>(3, 2));
    log.pass(name);
}

template <typename T>
void test_decrement(Logger& log, const std::string& name) {
    Rational<T> a(1, 2);
    --a;
    require(a == Rational<T>(-1, 2));
    log.pass(name);
}

void run_all() {
    Logger log;

    test_construction<int>(log, "Rational<int> construction");
    test_addition<int>(log, "Rational<int> addition");
    test_subtraction<int>(log, "Rational<int> subtraction");
    test_multiplication<int>(log, "Rational<int> multiplication");
    test_division<int>(log, "Rational<int> division");
    test_increment<int>(log, "Rational<int> increment");
    test_decrement<int>(log, "Rational<int> decrement");

    test_construction<long long>(log, "Rational<long long> construction");
    test_addition<long long>(log, "Rational<long long> addition");
    test_subtraction<long long>(log, "Rational<long long> subtraction");
    test_multiplication<long long>(log, "Rational<long long> multiplication");
    test_division<long long>(log, "Rational<long long> division");
    test_increment<long long>(log, "Rational<long long> increment");
    test_decrement<long long>(log, "Rational<long long> decrement");

    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}