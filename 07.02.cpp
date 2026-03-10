#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 07.02 - exceptions for Rational and standard library failures
// Description: extends Rational with a custom exception and demonstrates
//              handling of required standard exceptions in main.

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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
    friend T& operator++(T& value) {
        value += T{1};
        return value;
    }

    friend T operator++(T& value, int) {
        T old = value;
        ++value;
        return old;
    }
};

template <typename T>
class decrementable {
    friend T& operator--(T& value) {
        value -= T{1};
        return value;
    }

    friend T operator--(T& value, int) {
        T old = value;
        --value;
        return old;
    }
};

class Exception : public std::exception {
public:
    explicit Exception(std::string message) : message_(std::move(message)) {
    }

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

template <typename T>
class Rational : public addable<Rational<T>>,
                 public subtractable<Rational<T>>,
                 public multipliable<Rational<T>>,
                 public dividable<Rational<T>>,
                 public incrementable<Rational<T>>,
                 public decrementable<Rational<T>> {
public:
    Rational(T numerator = 0, T denominator = 1)
        : numerator_(numerator), denominator_(denominator) {
        if (denominator_ == 0) {
            throw Exception("Rational: zero denominator");
        }
        normalize();
    }

    Rational& operator+=(const Rational& other) {
        numerator_ = numerator_ * other.denominator_ + other.numerator_ * denominator_;
        denominator_ *= other.denominator_;
        normalize();
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        numerator_ = numerator_ * other.denominator_ - other.numerator_ * denominator_;
        denominator_ *= other.denominator_;
        normalize();
        return *this;
    }

    Rational& operator*=(const Rational& other) {
        numerator_ *= other.numerator_;
        denominator_ *= other.denominator_;
        normalize();
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        if (other.numerator_ == 0) {
            throw Exception("Rational: division by zero");
        }

        numerator_ *= other.denominator_;
        denominator_ *= other.numerator_;
        normalize();
        return *this;
    }

    bool operator==(const Rational& other) const {
        return numerator_ == other.numerator_ && denominator_ == other.denominator_;
    }

    friend std::ostream& operator<<(std::ostream& out, const Rational& value) {
        out << value.numerator_ << '/' << value.denominator_;
        return out;
    }

private:
    void normalize() {
        const T gcd = std::gcd(numerator_, denominator_);
        numerator_ /= gcd;
        denominator_ /= gcd;

        if (denominator_ < 0) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }
    }

private:
    T numerator_;
    T denominator_;
};

namespace tests {

struct Logger {
    int passed = 0;
    int failed = 0;

    void require(bool condition, const char* message) {
        if (condition) {
            ++passed;
            std::cout << "[OK]   " << message << '\n';
        } else {
            ++failed;
            std::cout << "[FAIL] " << message << '\n';
        }
    }
};

void run_all() {
    Logger log;

    {
        const Rational<int> value(2, 4);
        log.require(value == Rational<int>(1, 2), "constructor reduces fraction");
    }

    {
        const Rational<int> sum = Rational<int>(1, 2) + Rational<int>(1, 3);
        log.require(sum == Rational<int>(5, 6), "addition works");
    }

    {
        const Rational<int> difference = Rational<int>(5, 6) - Rational<int>(1, 3);
        log.require(difference == Rational<int>(1, 2), "subtraction works");
    }

    {
        const Rational<int> product = Rational<int>(2, 3) * Rational<int>(3, 5);
        log.require(product == Rational<int>(2, 5), "multiplication works");
    }

    {
        const Rational<int> quotient = Rational<int>(3, 4) / Rational<int>(5, 6);
        log.require(quotient == Rational<int>(9, 10), "division works");
    }

    {
        Rational<int> value(1, 2);
        const Rational<int> old = value++;
        log.require(old == Rational<int>(1, 2), "post-increment returns old value");
        log.require(value == Rational<int>(3, 2), "post-increment updates object");
    }

    {
        Rational<int> value(1, 2);
        const Rational<int> old = value--;
        log.require(old == Rational<int>(1, 2), "post-decrement returns old value");
        log.require(value == Rational<int>(-1, 2), "post-decrement updates object");
    }

    {
        bool caught = false;
        try {
            const Rational<int> invalid(1, 0);
            (void)invalid;
        } catch (const Exception& error) {
            caught = std::string(error.what()) == "Rational: zero denominator";
        }
        log.require(caught, "custom exception is thrown for zero denominator");
    }

    {
        bool caught = false;
        try {
            const Rational<int> value = Rational<int>(1, 2) / Rational<int>(0, 3);
            (void)value;
        } catch (const Exception& error) {
            caught = std::string(error.what()) == "Rational: division by zero";
        }
        log.require(caught, "custom exception is thrown for division by zero");
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

[[noreturn]] void throw_unknown_exception() {
    throw 42;
}

[[noreturn]] void provoke_bad_alloc() {
    // Request an impossibly large block to force allocation failure.
    (void)::operator new(std::numeric_limits<std::size_t>::max());
    std::abort();
}

int main() {
    try {
        tests::run_all();
        std::cout.flush();

        try {
            const Rational<int> invalid(7, 0);
            (void)invalid;
        } catch (const std::exception& error) {
            std::cerr << "Caught Exception via std::exception: " << error.what()
                      << ". Cause: Rational constructor received denominator 0.\n";
        } catch (...) {
            std::cerr << "Caught unknown exception while creating Rational.\n";
        }

        try {
            provoke_bad_alloc();
        } catch (const std::exception& error) {
            std::cerr << "Caught std::bad_alloc: " << error.what()
                      << ". Cause: operator new could not allocate the requested memory block.\n";
        } catch (...) {
            std::cerr << "Caught unknown exception instead of std::bad_alloc.\n";
        }

        try {
            std::variant<int, std::string> value = 17;
            (void)std::get<std::string>(value);
        } catch (const std::exception& error) {
            std::cerr << "Caught std::bad_variant_access: " << error.what()
                      << ". Cause: requested inactive alternative from std::variant.\n";
        } catch (...) {
            std::cerr << "Caught unknown exception instead of std::bad_variant_access.\n";
        }

        try {
            const std::optional<int> value;
            (void)value.value();
        } catch (const std::exception& error) {
            std::cerr << "Caught std::bad_optional_access: " << error.what()
                      << ". Cause: attempted to extract value from empty std::optional.\n";
        } catch (...) {
            std::cerr << "Caught unknown exception instead of std::bad_optional_access.\n";
        }

        try {
            std::vector<int> values;
            values.reserve(values.max_size() + 1);
        } catch (const std::exception& error) {
            std::cerr << "Caught std::length_error: " << error.what()
                      << ". Cause: std::vector::reserve requested size beyond max_size().\n";
        } catch (...) {
            std::cerr << "Caught unknown exception instead of std::length_error.\n";
        }

        try {
            const std::vector<int> values{1, 2, 3};
            (void)values.at(10);
        } catch (const std::exception& error) {
            std::cerr << "Caught std::out_of_range: " << error.what()
                      << ". Cause: std::vector::at checked an invalid index.\n";
        } catch (...) {
            std::cerr << "Caught unknown exception instead of std::out_of_range.\n";
        }

        try {
            throw_unknown_exception();
        } catch (const std::exception& error) {
            std::cerr << "Caught std::exception unexpectedly: " << error.what() << '\n';
        } catch (...) {
            std::cerr << "Caught non-standard exception in catch(...).\n";
        }
    } catch (const std::exception& error) {
        std::cerr << "Unhandled std::exception in main: " << error.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unhandled unknown exception in main.\n";
        return 1;
    }

    return 0;
}
