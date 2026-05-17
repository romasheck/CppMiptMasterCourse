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

// Problem: 13.04
// Description: List directory entries that match a user-specified regex.

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> list_matching(const std::filesystem::path& dir, const std::string& pattern) {
    std::vector<std::string> out;
    std::regex re(pattern);
    for (auto const & entry : std::filesystem::directory_iterator(dir)) {
        auto name = entry.path().filename().string();
        if (std::regex_search(name, re)) out.push_back(name);
    }
    return out;
}

namespace tests {
    void run_all() {
        namespace fs = std::filesystem;
        fs::path d = "13_04_test_dir";
        fs::remove_all(d);
        fs::create_directory(d);
        std::vector<std::string> files = {"foo.txt","bar.log","baz.txt","README.md","test123.txt"};
        for (auto &f: files) { std::ofstream(d / f); }

        auto v = list_matching(d, R"(.*\.txt$)");
        if (v.size() != 3) { std::cout<<"[FAIL] expected 3 .txt files\n"; std::exit(1); }

        auto v2 = list_matching(d, R"(^test)");
        if (v2.size() != 1) { std::cout<<"[FAIL] expected 1 match for ^test\n"; std::exit(1); }

        std::cout<<"[OK] 13.04 tests passed\n";
        fs::remove_all(d);
    }
}

int main(){ tests::run_all(); return 0; }
