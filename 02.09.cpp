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
#include <numeric> 
#include <cmath> 
#include <cassert> 

int gcd_recursive(int a, int b) 
{
    if (b == 0)
        return std::abs(a);
    return gcd_recursive(b, a % b);
}

int gcd_iterative(int a, int b) 
{
    a = std::abs(a);
    b = std::abs(b);
    
    int temp;
    while (b != 0) 
    {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int lcm_through_gcd(int a, int b) 
{
    if (a == 0 || b == 0)
        return 0;
    return std::abs(a * b) / gcd_iterative(a, b);
}

void test(int a, int b) 
{   
    int gcd_rec = gcd_recursive(a, b);
    int gcd_iter = gcd_iterative(a, b);
    int gcd_std = std::gcd(a, b);
    
    int lcm = lcm_through_gcd(a, b);
    int lcm_std = std::lcm(a, b);
    
    bool gcd_correct = (gcd_rec == gcd_std) && (gcd_iter == gcd_std);
    bool lcm_correct = (lcm == lcm_std);
    assert(gcd_correct && lcm_correct);

    std::cout << "Test passed : a = " << a << ", b = " << b << ", answer is " << lcm << std::endl; 
}

int main() {
    
    test(48, 18);
    test(1, 13);
    test(3, 5);
    test(0, 5);
    test(2, 0);
    test(0, 0);
    test(-48, 18);

    return 0;
}