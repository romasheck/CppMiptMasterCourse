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

int main()
{
    for (auto i = 1; i <= 9; ++i) {
        auto x1 = i * i * i;
        for (auto j = 0; j <= 9; ++j) {
            auto x2 = j * j * j;
            for (auto k = 0; k <= 9; ++k) {
                auto x3 = k * k * k;
                int sum = x1 + x2 + x3;
                int num = i * 100 + j * 10 + k;
                if (sum == num)
                    std::cout << num << std::endl; 
            }
        }
    }
    return 0;
}
