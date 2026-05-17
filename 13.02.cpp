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

// Problem: 13.02
// Description: Read expressions from a file using std::fstream and evaluate using Parser (from 12.05).

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cmath>
#include <cctype>
#include <stdexcept>

// Minimal Parser (copied/adapted from 12.05) -------------------------------------------------
class Parser {
public:
    explicit Parser(std::string_view s) : str(s), pos(0) {}
    double parse() { skip_ws(); double val = expression(); skip_ws(); if (pos != str.size()) throw std::runtime_error("Unexpected input"); return val; }
private:
    std::string_view str; size_t pos;
    void skip_ws(){ while (pos<str.size() && std::isspace((unsigned char)str[pos])) ++pos; }
    bool peek(char c) const { size_t p=pos; while(p<str.size() && std::isspace((unsigned char)str[p])) ++p; return p<str.size() && str[p]==c; }
    bool accept(char c){ skip_ws(); if (pos<str.size() && str[pos]==c){++pos; return true;} return false; }
    double expression(){ double l=term(); for(;;){ skip_ws(); if(pos>=str.size()) break; char op=str[pos]; if(op=='+'||op=='-'){++pos; double r=term(); l=(op=='+')?l+r:l-r;} else break;} return l; }
    double term(){ double l=unary(); for(;;){ skip_ws(); if(pos>=str.size()) break; char op=str[pos]; if(op=='*'||op=='/'||op=='%'){++pos; double r=unary(); if(op=='*') l*=r; else if(op=='/') l/=r; else l=std::fmod(l,r);} else break;} return l; }
    double power(){ double l=primary(); skip_ws(); if(peek('^')){ ++pos; double r=power(); l=std::pow(l,r);} return l; }
    double unary(){ skip_ws(); if(pos<str.size() && str[pos]=='+'){++pos; return unary();} if(pos<str.size() && str[pos]=='-'){++pos; return -unary();} return power(); }
    double primary(){ skip_ws(); if(pos>=str.size()) throw std::runtime_error("Unexpected"); double val=0.0; if(str[pos]=='('||str[pos]=='['||str[pos]=='{'){ char open=str[pos++]; char close=(open=='(')?')':(open=='['?']':'}'); val=expression(); if(!accept(close)) throw std::runtime_error("Mismatched"); } else if(std::isdigit((unsigned char)str[pos])||str[pos]=='.'){ size_t start=pos; while(pos<str.size() && (std::isdigit((unsigned char)str[pos])||str[pos]=='.'||str[pos]=='e'||str[pos]=='E'||str[pos]=='+'||str[pos]=='-')){ if((str[pos]=='+'||str[pos]=='-') && pos!=start) break; ++pos;} val=std::stod(std::string(str.substr(start,pos-start))); } else throw std::runtime_error("Bad"); skip_ws(); while(pos<str.size() && str[pos]=='!'){ ++pos; val = factorial_of(val); skip_ws(); } return val; }
    static double factorial_of(double v){ long long n=(long long)v; if(n<0) throw std::runtime_error("neg"); double r=1; for(long long i=1;i<=n;++i) r*= (double)i; return r; }
};
// --------------------------------------------------------------------------------------------

int main(){
    // prepare a small test input file
    const char* fname = "13.02_input.txt";
    {
        std::ofstream ofs(fname);
        ofs << "1+2*3\n";
        ofs << "(1+2)*3\n";
        ofs << "3!\n";
    }

    std::ifstream ifs(fname);
    if (!ifs) { std::cerr<<"Failed open file"; return 1; }
    std::string line;
    std::vector<double> results;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        Parser p(line);
        results.push_back(p.parse());
    }

    if (results.size() != 3) { std::cerr<<"Wrong count"; return 1; }
    if (results[0] != 7.0 || results[1] != 9.0 || results[2] != 6.0) { std::cerr<<"Bad results"; return 1; }

    std::cout<<"[OK] 13.02 tests passed\n";
    return 0;
}
