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
#include <vector>
#include <format>

std::size_t collatz_len(const unsigned long long int &start, std::vector<std::size_t> &cache) 
{
    std::size_t len = 1;
    unsigned long long int x = start; 
    while (x != 1) 
    {
        if (x < cache.size() && cache[x] != 0)
        {
            len += cache[x];
            x = 1;
        } 
        else 
        {
            x = (x % 2 == 0 ? x / 2 : x * 3 + 1);
            ++len;
        }
    }
    if (start < cache.size())
        cache[start] = len;
    return len;
}

int main()
{
    const unsigned long long int MAX_START{100};
    const unsigned long long int MAX_CACHE{1000}; 
    std::vector<std::size_t> cache(MAX_CACHE + 1, 0);

    unsigned long long int max{0};
    std::size_t max_value{0};

    for (unsigned long long int start{1}; start <= MAX_START; ++start)
    {
        auto len = collatz_len(start, cache);
        if (len > max_value)
        {
            max = start;
            max_value = len;
        }
    }

    std::cout << max << " " << max_value << std::endl;
    
    return 0;
}
