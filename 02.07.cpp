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
#include <cmath>
#include <format>

int main()
{
    std::size_t n;
    std::cin >> n;
    if (n <= 0) {
        std::cout << "Non-valid size" << std::endl;
        return 0;
    }
    double *arr = new double[n];
    for (auto i = 0uz; i < n; ++i) 
        std::cin >> arr[i];
    
    double max{arr[0]}, min{arr[0]}, avr{0.0};
    for (auto i = 0uz; i < n; ++i)
    {
        max = std::max(max, arr[i]);
        min = std::min(min, arr[i]);
        avr += arr[i];
    }
    avr = avr / static_cast<double>(n);

    double div{0.0};
    if (n > 1)
    {
        for (auto i = 0uz; i < n; ++i)
            div += (arr[i] - avr) * (arr[i] - avr);
        div = div / static_cast<double>(n - 1);
        div = std::sqrt(div);
    }

    delete[] arr;

    std::cout << std::format("min: {:.3f}, max: {:.3f}, avr: {:.3f}, div: {:.3f}", min, max, avr, div) << std::endl;

    return 0;
}
