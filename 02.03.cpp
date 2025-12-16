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
    char c;
    std::cin >> c;
    
    switch (c) 
    {
        case '0' ... '9' : 
        {
            std::cout << "цифра" << std::endl;
            break;
        }
        case 'A' ... 'Z' : 
        {
            std::cout << "заглавная буква" << std::endl;
            break;
        }
        case 'a' ... 'z' : 
        {
            std::cout << "строчная буква" << std::endl;
            break;
        }
        case '.' :
        case ',' :
        case ':' :
        case ';' :
        case '!' :
        case '?' :
        {
            std::cout << "строчная буква" << std::endl;
            break;
        }
        default :
        {
            std::cout << "прочий символ" << std::endl;
            break;
        }   
    }
    return 0;
}
