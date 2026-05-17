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

// Problem: 12.02
// Description: Print this program's own source code with std::printf.

#include <cstdio>

static const char* source =
"#if 0\n"
"set -e\n"
"CXX=g++\n"
"CXXFLAGS=\"-std=c++23 -Wall -Wextra -Wpedantic -O2\"\n"
"SCRIPT_NAME=$(basename \"$0\" .cpp)\n"
"$CXX $CXXFLAGS \"$0\" -o \"${SCRIPT_NAME}.out\"\n"
"./\"${SCRIPT_NAME}.out\" \"$@\"\n"
"rm \"${SCRIPT_NAME}.out\"\n"
"exit\n"
"#endif\n"
"\n"
"// Problem: 12.02\n"
"// Description: Print this program's own source code with std::printf.\n"
"\n"
"#include <cstdio>\n"
"\n"
"static const char* source =\n"
"\"%c%s%c\\n\"\n"
"\n"
"int main() {\n"
"    std::printf(source, 34, source, 34);\n"
"    return 0;\n"
"}\n";

int main() {
    std::printf(source, 34, source, 34);
    return 0;
}
