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

// Problem: 13.01
// Description: Convert vector<uint8_t> to lowercase hex string and back.

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

std::string to_hex_string(const std::vector<std::uint8_t>& data) {
    std::ostringstream oss;
    oss << std::hex << std::nouppercase;
    for (auto b : data) {
        oss << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

std::vector<std::uint8_t> from_hex_string(std::string_view s) {
    std::vector<std::uint8_t> out;
    if (s.size() % 2 != 0) throw std::runtime_error("Hex string length must be even");
    auto hexval = [](char c)->int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        throw std::runtime_error("Invalid hex char");
    };
    for (size_t i = 0; i < s.size(); i += 2) {
        int hi = hexval(s[i]);
        int lo = hexval(s[i+1]);
        out.push_back(static_cast<std::uint8_t>((hi << 4) | lo));
    }
    return out;
}

namespace tests {
    void run_all() {
        std::vector<std::uint8_t> v{0x00, 0x0f, 0x10, 0xab, 0xff};
        std::string s = to_hex_string(v);
        if (s != "000f10abff") { std::cout<<"[FAIL] to_hex_string\n"; std::exit(1); }
        auto v2 = from_hex_string(s);
        if (v2 != v) { std::cout<<"[FAIL] from_hex_string\n"; std::exit(1); }
        std::cout<<"[OK] 13.01 tests passed\n";
    }
}

int main(){ tests::run_all(); return 0; }
