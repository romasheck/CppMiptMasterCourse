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

// Problem: 08.02

#include <cstdlib>
#include <cctype>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Integer {
public:
    using digit_t = long long;

    Integer() : is_negative_(false), digits_(kCapacity, 0), size_(1) {
    }

    Integer(digit_t value) : Integer() {
        parse(std::to_string(value));
    }

    Integer(const std::string& value) : Integer() {
        parse(value);
    }

    void swap(Integer& other) {
        std::swap(is_negative_, other.is_negative_);
        std::swap(digits_, other.digits_);
        std::swap(size_, other.size_);
    }

    Integer& operator+=(Integer other) {
        if (is_negative_ == other.is_negative_) {
            add(other);
        } else if (!is_negative_ && other.is_negative_) {
            if (less(other)) {
                *this = std::move(other.subtract(*this));
                is_negative_ = true;
            } else {
                subtract(other);
            }
        } else if (is_negative_ && !other.is_negative_) {
            if (less(other)) {
                *this = std::move(other.subtract(*this));
            } else {
                subtract(other);
                is_negative_ = true;
            }
        }

        normalize_zero_sign();
        return *this;
    }

    Integer& operator-=(Integer other) {
        other.is_negative_ = !other.is_negative_;
        return *this += other;
    }

    Integer& operator*=(Integer other) {
        Integer result;
        result.is_negative_ = is_negative_ ^ other.is_negative_;

        for (std::size_t i = 0; i < size_; ++i) {
            digit_t carry = 0;

            for (std::size_t j = 0; (j < other.size_) || carry; ++j) {
                result.digits_[i + j] += digits_[i] * other.digits_[j] + carry;
                carry = result.digits_[i + j] / kBase;
                result.digits_[i + j] -= carry * kBase;
            }
        }

        result.size_ = size_ + other.size_;
        swap(result);
        reduce();
        normalize_zero_sign();
        return *this;
    }

    Integer& operator/=(Integer other) {
        if (other == 0) {
            throw std::runtime_error("division by zero");
        }

        Integer result;
        result.size_ = size_;
        result.is_negative_ = is_negative_ ^ other.is_negative_;

        other.is_negative_ = false;
        Integer current;

        for (int i = static_cast<int>(size_) - 1; i >= 0; --i) {
            current *= kBase;
            current.digits_.front() = digits_[static_cast<std::size_t>(i)];

            digit_t left = 0;
            digit_t right = kBase;
            digit_t digit = 0;

            while (left <= right) {
                const digit_t middle = std::midpoint(left, right);
                if (other * middle <= current) {
                    left = middle + 1;
                    digit = middle;
                } else {
                    right = middle - 1;
                }
            }

            result.digits_[static_cast<std::size_t>(i)] = digit;
            current -= other * digit;
        }

        swap(result);
        reduce();
        normalize_zero_sign();
        return *this;
    }

    Integer& operator%=(const Integer& other) {
        if (other == 0) {
            throw std::runtime_error("modulo by zero");
        }

        *this -= (*this / other) * other;
        normalize_zero_sign();
        return *this;
    }

    Integer operator++(int) {
        Integer copy = *this;
        *this += 1;
        return copy;
    }

    Integer operator--(int) {
        Integer copy = *this;
        *this -= 1;
        return copy;
    }

    Integer& operator++() {
        *this += 1;
        return *this;
    }

    Integer& operator--() {
        *this -= 1;
        return *this;
    }

    int sign() const {
        if (is_zero()) {
            return 0;
        }
        return is_negative_ ? -1 : 1;
    }

    Integer abs() const {
        Integer copy = *this;
        copy.is_negative_ = false;
        return copy;
    }

    friend Integer operator+(Integer lhs, const Integer& rhs) {
        return lhs += rhs;
    }

    friend Integer operator-(Integer lhs, const Integer& rhs) {
        return lhs -= rhs;
    }

    friend Integer operator*(Integer lhs, const Integer& rhs) {
        return lhs *= rhs;
    }

    friend Integer operator/(Integer lhs, const Integer& rhs) {
        return lhs /= rhs;
    }

    friend Integer operator%(Integer lhs, const Integer& rhs) {
        return lhs %= rhs;
    }

    friend Integer pow(Integer base, unsigned int exponent) {
        Integer result(1);

        while (exponent > 0) {
            if ((exponent & 1u) != 0u) {
                result *= base;
            }
            exponent >>= 1u;
            if (exponent != 0u) {
                base *= base;
            }
        }

        return result;
    }

    friend bool operator<(const Integer& lhs, const Integer& rhs) {
        if (lhs.is_negative_ != rhs.is_negative_) {
            return lhs.is_negative_;
        }

        if (lhs.is_negative_ && rhs.is_negative_) {
            return rhs.less(lhs);
        }

        return lhs.less(rhs);
    }

    friend bool operator>(const Integer& lhs, const Integer& rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const Integer& lhs, const Integer& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Integer& lhs, const Integer& rhs) {
        return !(lhs < rhs);
    }

    friend bool operator==(const Integer& lhs, const Integer& rhs) {
        if (lhs.is_negative_ != rhs.is_negative_ || lhs.size_ != rhs.size_) {
            return false;
        }

        for (std::size_t i = 0; i < lhs.size_; ++i) {
            if (lhs.digits_[i] != rhs.digits_[i]) {
                return false;
            }
        }

        return true;
    }

    friend std::istream& operator>>(std::istream& in, Integer& value) {
        std::string text;
        in >> text;
        value = Integer(text);
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Integer& value) {
        if (value.is_negative_) {
            out << '-';
        } else {
            out << '+';
        }

        out << value.digits_[value.size_ - 1];
        for (int i = static_cast<int>(value.size_) - 2; i >= 0; --i) {
            out << std::setw(static_cast<int>(kStep)) << std::setfill('0')
                << value.digits_[static_cast<std::size_t>(i)];
        }
        out << std::setfill(' ');
        return out;
    }

private:
    void parse(const std::string& text) {
        if (text.empty()) {
            throw std::runtime_error("empty integer string");
        }

        is_negative_ = text.front() == '-';
        size_ = 0;

        for (long long i = static_cast<long long>(text.size()) - 1; i >= 0; i -= static_cast<long long>(kStep)) {
            long long begin = std::max(i - static_cast<long long>(kStep) + 1, 0LL);

            if (begin == 0 && !std::isdigit(static_cast<unsigned char>(text.front()))) {
                ++begin;
            }

            const std::string digit = text.substr(static_cast<std::size_t>(begin),
                                                  static_cast<std::size_t>(i - begin + 1));

            if (!digit.empty()) {
                digits_[size_++] = std::stoll(digit);
            }
        }

        reduce();
        normalize_zero_sign();
    }

    void reduce() {
        while (size_ > 1 && digits_[size_ - 1] == 0) {
            --size_;
        }
    }

    void normalize_zero_sign() {
        if (is_zero()) {
            is_negative_ = false;
        }
    }

    bool is_zero() const {
        return size_ == 1 && digits_[0] == 0;
    }

    Integer& add(const Integer& other) {
        size_ = std::max(size_, other.size_);

        for (std::size_t i = 0; i < size_; ++i) {
            digits_[i] += other.digits_[i];

            if (digits_[i] >= kBase) {
                digits_[i] -= kBase;
                ++digits_[i + 1];
            }
        }

        size_ += digits_[size_];
        return *this;
    }

    Integer& subtract(const Integer& other) {
        for (std::size_t i = 0; i < size_; ++i) {
            digits_[i] -= other.digits_[i];

            if (digits_[i] < 0) {
                digits_[i] += kBase;
                --digits_[i + 1];
            }
        }

        reduce();
        return *this;
    }

    bool less(const Integer& other) const {
        if (size_ != other.size_) {
            return size_ < other.size_;
        }

        for (int i = static_cast<int>(size_) - 1; i >= 0; --i) {
            const std::size_t index = static_cast<std::size_t>(i);
            if (digits_[index] != other.digits_[index]) {
                return digits_[index] < other.digits_[index];
            }
        }

        return false;
    }

private:
    bool is_negative_ = false;
    std::vector<digit_t> digits_;
    std::size_t size_ = 0;

    static inline constexpr std::size_t kCapacity = 1000;
    static inline constexpr std::size_t kStep =
        static_cast<std::size_t>(std::numeric_limits<digit_t>::digits10 / 2);
    static inline constexpr digit_t kBase = 1000000000LL;
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

std::string stringify(const Integer& value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

void run_all() {
    Logger log;

    {
        Integer x("11111111111111111111111111111111");
        Integer y("22222222222222222222222222222222");
        log.require(stringify(x + y) == "+33333333333333333333333333333333",
                    "addition from previous example still works");
    }

    {
        Integer x("100");
        x %= Integer("7");
        log.require(stringify(x) == "+2", "operator%= computes remainder");
    }

    {
        const Integer x("-100");
        const Integer y("7");
        log.require(stringify(x % y) == "-2", "operator% keeps dividend sign");
    }

    {
        log.require(stringify(pow(Integer("12"), 0)) == "+1", "pow handles zero exponent");
        log.require(stringify(pow(Integer("12"), 3)) == "+1728", "pow raises to positive exponent");
    }

    {
        log.require(Integer("-456").sign() == -1, "sign returns -1 for negative");
        log.require(Integer("0").sign() == 0, "sign returns 0 for zero");
        log.require(Integer("789").sign() == 1, "sign returns 1 for positive");
    }

    {
        log.require(stringify(Integer("-123456").abs()) == "+123456",
                    "abs returns positive value");
        log.require(stringify(Integer("123456").abs()) == "+123456",
                    "abs leaves positive value unchanged");
    }

    {
        bool caught = false;
        try {
            Integer x("10");
            x %= Integer("0");
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "modulo by zero is rejected");
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
