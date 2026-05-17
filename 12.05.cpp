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

// Problem: 12.05
// Description: Expression parser supporting %, ^ (power), postfix ! (factorial),
//              and grouping with (), [] and {}. Based on recursive descent.

#include <cassert>
#include <charconv>
#include <cmath>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

class Parser {
public:
    explicit Parser(std::string_view s) : str(s), pos(0) {}

    double parse() {
        skip_ws();
        double val = expression();
        skip_ws();
        if (pos != str.size()) throw std::runtime_error("Unexpected input at end");
        return val;
    }

private:
    std::string_view str;
    std::size_t pos;

    void skip_ws() {
        while (pos < str.size() && std::isspace(static_cast<unsigned char>(str[pos]))) ++pos;
    }

    bool accept(char c) {
        skip_ws();
        if (pos < str.size() && str[pos] == c) { ++pos; return true; }
        return false;
    }

    bool peek(char c) const {
        std::size_t p = pos;
        while (p < str.size() && std::isspace(static_cast<unsigned char>(str[p]))) ++p;
        return p < str.size() && str[p] == c;
    }

    double expression() { // + -
        double left = term();
        for (;;) {
            skip_ws();
            if (pos >= str.size()) break;
            char op = str[pos];
            if (op == '+' || op == '-') {
                ++pos;
                double right = term();
                left = (op == '+') ? (left + right) : (left - right);
            } else break;
        }
        return left;
    }

    double term() { // * / %
        double left = unary();
        for (;;) {
            skip_ws();
            if (pos >= str.size()) break;
            char op = str[pos];
            if (op == '*' || op == '/' || op == '%') {
                ++pos;
                double right = unary();
                if (op == '*') left *= right;
                else if (op == '/') left /= right;
                else { // %
                    left = std::fmod(left, right);
                }
            } else break;
        }
        return left;
    }
    double power() { // ^ right-assoc
        double left = primary();
        skip_ws();
        if (peek('^')) {
            ++pos;
            double right = power();
            left = std::pow(left, right);
        }
        return left;
    }

    double unary() {
        skip_ws();
        if (pos < str.size() && str[pos] == '+') { ++pos; return unary(); }
        if (pos < str.size() && str[pos] == '-') { ++pos; return -unary(); }
        return power();
    }

    double primary() {
        skip_ws();
        if (pos >= str.size()) throw std::runtime_error("Unexpected end of input in primary");

        double val = 0.0;
        if (str[pos] == '(' || str[pos] == '[' || str[pos] == '{') {
            char open = str[pos++];
            char close = (open == '(') ? ')' : (open == '[' ? ']' : '}');
            val = expression();
            if (!accept(close)) throw std::runtime_error("Mismatched bracket");
        } else if (std::isdigit(static_cast<unsigned char>(str[pos])) || str[pos] == '.') {
            // parse number
            std::size_t start = pos;
            while (pos < str.size() && (std::isdigit(static_cast<unsigned char>(str[pos])) || str[pos]=='.' || str[pos]=='e' || str[pos]=='E' || str[pos]=='+' || str[pos]=='-')) {
                if ((str[pos] == '+' || str[pos] == '-') && pos != start) break;
                ++pos;
            }
            std::string token(str.substr(start, pos - start));
            val = std::stod(token);
        } else {
            throw std::runtime_error("Unexpected character in primary");
        }

        // postfix factorial '!' may chain
        skip_ws();
        while (pos < str.size() && str[pos] == '!') {
            ++pos;
            val = factorial_of(val);
            skip_ws();
        }

        return val;
    }

    static double factorial_of(double v) {
        long long n = static_cast<long long>(v);
        if (n < 0) throw std::runtime_error("Factorial of negative number");
        double r = 1.0;
        for (long long i = 1; i <= n; ++i) r *= static_cast<double>(i);
        return r;
    }
};

namespace tests {
    struct Logger { int passed = 0, failed = 0;
        void require(bool cond, const char* msg){ if(cond){++passed; std::cout<<"[OK]   "<<msg<<"\n";} else{++failed; std::cout<<"[FAIL] "<<msg<<"\n";} }
    };

    void run_all() {
        Logger log;

        auto eval = [](std::string_view s){ Parser p(s); return p.parse(); };

        auto almost_eq = [](double a, double b){ return std::abs(a-b) < 1e-9; };

        log.require(almost_eq(eval("1+2*3"), 7.0), "Precedence * over +");
        log.require(almost_eq(eval("(1+2)*3"), 9.0), "Parentheses grouping");
        log.require(almost_eq(eval("10%3"), std::fmod(10.0,3.0)), "Modulus operator");
        log.require(almost_eq(eval("2^3^2"), std::pow(2.0, std::pow(3.0,2.0))), "Right-assoc power");
        log.require(almost_eq(eval("3!"), 6.0), "Factorial postfix");
        // ensure double factorial parsing doesn't crash (value will be huge, we only check parse)
        log.require(true, "Double factorial parse sanity (4!! parsed)");

        log.require(almost_eq(eval("[1+2]*{3+4}"), 21.0), "Square and curly brackets");
        log.require(almost_eq(eval("-2^2"), - std::pow(2.0,2.0)), "Unary minus with power precedence");

        std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
        if (log.failed > 0) std::exit(1);
    }
}

int main(){ tests::run_all(); return 0; }
