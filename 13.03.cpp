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

// Problem: 13.03
// Description: Remove blank and whitespace-only lines; handle raw string literals.

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

std::string remove_blank_lines(std::string_view input) {
    std::string source(input);
    std::istringstream iss(source);
    std::ostringstream oss;
    std::string line;
    bool first = true;

    while (std::getline(iss, line)) {
        bool all_ws = std::all_of(line.begin(), line.end(), [](unsigned char c) {
            return std::isspace(c);
        });
        if (all_ws) continue;
        if (!first) oss << '\n';
        oss << line;
        first = false;
    }

    return oss.str();
}

namespace tests {
    void run_all() {
        const auto raw = R"RAW(line1


line2
    
line3
)RAW";

        auto out = remove_blank_lines(raw);
        if (out.find("line1") == std::string::npos) { std::cout << "[FAIL] missing line1\n"; std::exit(1); }
        if (out.find("line2") == std::string::npos) { std::cout << "[FAIL] missing line2\n"; std::exit(1); }
        if (out.find("line3") == std::string::npos) { std::cout << "[FAIL] missing line3\n"; std::exit(1); }
        std::cout << "[OK] 13.03 tests passed\n";
    }
}

int main() { tests::run_all(); return 0; }
