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
#include <format>

int main()
{
    const double epsilon = 1e-10;
    double e = 0;
    double cur_e = 1;

    for (int i = 1; cur_e > epsilon; ++i) {
        e += cur_e;
        cur_e /= i;
    }

    std::cout << std::format("{:.20f}", e) << std::endl;
    
    return 0;
}
