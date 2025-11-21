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

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////

void order(std::vector<int> & vector, std::size_t left, std::size_t right)
{
	for (auto i = left + 1; i < right; ++i) 
	{
		for (auto j = i; j > left; --j)
		{
			if (vector[j - 1] > vector[j]) 
			{
				std::swap(vector[j], vector[j - 1]);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////

std::size_t separate(std::vector<int>& vector, std::size_t left, std::size_t right)
{
    // pivot selection

	std::size_t middle = std::midpoint(left, right - 1);

    if (vector[middle] < vector[left])
        std::swap(vector[middle], vector[left]);
    if (vector[right - 1] < vector[left])
        std::swap(vector[right - 1], vector[left]);
    if (vector[right - 1] < vector[middle])
        std::swap(vector[right - 1], vector[middle]);

    int pivot = vector[middle];

    // Hoar     
	auto i = left;
    auto j = right - 1;
    
    while (true)
    {
        while (vector[i] < pivot)
            ++i;
        while (vector[j] > pivot)
            --j;
            
        if (i >= j)
            return j + 1;
            
        std::swap(vector[i], vector[j]);
        ++i;
        --j;
    }
}

////////////////////////////////////////////////////////////////////////////////////

void quick_sort(std::vector<int> & vector, std::size_t left, std::size_t right)
{
	if (right - left > 16)
	{
		std::size_t separator_pos =	separate(vector, left, right);
		quick_sort(vector, left, separator_pos);
		quick_sort(vector, separator_pos, right);
	}
	else
	{
		order(vector, left,   right);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void sort(std::vector < int > & vector)
{
	quick_sort(vector, 0, std::size(vector));
}

////////////////////////////////////////////////////////////////////////////////////

int main()
{
	{
		auto size = 1'000uz;
		std::vector < int > vector(size, 0);
		for (auto i = 0uz; i < size; ++i)
		{
			vector[i] = size - i;
		}
		sort(vector);
		assert(std::ranges::is_sorted(vector));
	}
	{
		auto size = 1'000uz;
		std::vector < int > vector(size, 10);
		sort(vector);
		assert(std::ranges::is_sorted(vector));
	}
}

////////////////////////////////////////////////////////////////////////////////////
