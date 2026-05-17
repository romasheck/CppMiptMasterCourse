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

// Problem: 12.01
// Description: RUB to USD conversion with localized money input and output.

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

constexpr long double kRubPerUsd = 90.0L;

std::string trim(std::string_view text) {
    const std::string_view whitespace = " \t\n\r\f\v";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string_view::npos) {
        return {};
    }

    const std::size_t last = text.find_last_not_of(whitespace);
    return std::string{text.substr(first, last - first + 1)};
}

bool starts_with_rub(std::string_view text) {
    return text.size() >= 3 && text.substr(0, 3) == "RUB";
}

std::string normalize_rub_input(std::string_view text) {
    std::string normalized = trim(text);
    if (starts_with_rub(normalized)) {
        normalized = trim(std::string_view{normalized}.substr(3)) + " RUB";
    }
    return normalized;
}

long double read_rubles_in_kopecks(std::string_view text) {
    std::stringstream input(normalize_rub_input(text));
    input.imbue(std::locale("ru_RU.utf8"));

    long double kopecks = 0.0L;
    input >> std::get_money(kopecks, true);
    if (!input) {
        throw std::invalid_argument("invalid RUB money value");
    }

    return kopecks;
}

long double convert_rub_kopecks_to_usd_cents(long double rub_kopecks) {
    return std::round(rub_kopecks / kRubPerUsd);
}

std::string write_usd(long double usd_cents) {
    std::stringstream output;
    output.imbue(std::locale("en_US.utf8"));
    output << std::showbase << std::put_money(usd_cents, true);
    return output.str();
}

std::string convert_rub_to_usd(std::string_view rub_text) {
    return write_usd(convert_rub_kopecks_to_usd_cents(read_rubles_in_kopecks(rub_text)));
}

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

void test_rub_after_amount(Logger& log) {
    log.require(convert_rub_to_usd("900,00 RUB") == "USD  10.00",
                "reads RUB after the amount");
}

void test_rub_before_amount(Logger& log) {
    log.require(convert_rub_to_usd("RUB 180,00") == "USD  2.00",
                "reads RUB before the amount");
}

void test_fractional_rounding(Logger& log) {
    log.require(convert_rub_to_usd("123,45 RUB") == "USD  1.37",
                "rounds converted cents");
}

void test_invalid_input(Logger& log) {
    bool thrown = false;
    try {
        (void)convert_rub_to_usd("not money");
    } catch (const std::invalid_argument&) {
        thrown = true;
    }

    log.require(thrown, "rejects invalid money text");
}

void run_all() {
    Logger log;

    test_rub_after_amount(log);
    test_rub_before_amount(log);
    test_fractional_rounding(log);
    test_invalid_input(log);

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
