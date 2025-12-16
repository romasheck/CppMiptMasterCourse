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

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

// ------------------------------------------------------------------------------------------------

class IPv4
{
public:
    static constexpr std::size_t octetCount = 4;
    static constexpr std::uint8_t octetMax = 255u;
    static constexpr char octetSep = '.';

    IPv4(std::uint8_t a = 0, std::uint8_t b = 0, std::uint8_t c = 0, std::uint8_t d = 0)
        : m_octet{a, b, c, d}
    {
    }

    IPv4 & operator++() 
    {
        for (int i = octetCount - 1; i >= 0; --i)            
        {
            if (m_octet[i] < octetMax)
            {
                ++m_octet[i];
                break;
            }
            else
                m_octet[i] = 0u;
        }
        return *this;
    }

    IPv4 & operator--() 
    {
        for (int i = octetCount - 1; i >= 0; --i)
        {
            if (m_octet[i] > 0u)
            {
                --m_octet[i];
                break;
            }
            else
                m_octet[i] = static_cast<std::uint8_t>(octetMax);
        }
        return *this;
    }

    IPv4 operator++(int) 
    {
        IPv4 tmp = *this;
        ++(*this);
        return tmp;
    }

    IPv4 operator--(int) 
    {
        IPv4 tmp = *this;
        --(*this);
        return tmp;
    }

    friend bool operator<(IPv4 const & lhs, IPv4 const & rhs)
    {
        for (std::size_t i = 0; i < octetCount; ++i)
            if (lhs.m_octet[i] != rhs.m_octet[i])
                return lhs.m_octet[i] < rhs.m_octet[i];
        return false;
    }

    friend bool operator>(IPv4 const & lhs, IPv4 const & rhs) 
    {
        return rhs < lhs;
    }

    friend bool operator<=(IPv4 const & lhs, IPv4 const & rhs) 
    {
        return !(rhs < lhs);
    }

    friend bool operator>=(IPv4 const & lhs, IPv4 const & rhs) 
    {
        return !(lhs < rhs);
    }

    friend bool operator==(IPv4 const & lhs, IPv4 const & rhs) 
    {
        for (std::size_t i = 0; i < octetCount; ++i)
            if (lhs.m_octet[i] != rhs.m_octet[i])
                return false;
        return true;
    }

    friend bool operator!=(IPv4 const & lhs, IPv4 const & rhs) 
    {
        return !(lhs == rhs);
    }

    friend std::ostream & operator<<(std::ostream & os, IPv4 const & ip)
    {
        os << static_cast<unsigned>(ip.m_octet[0]) << octetSep
           << static_cast<unsigned>(ip.m_octet[1]) << octetSep
           << static_cast<unsigned>(ip.m_octet[2]) << octetSep
           << static_cast<unsigned>(ip.m_octet[3]);
        return os;
    }

    friend std::istream & operator>>(std::istream & is, IPv4 & ip)
    {
        unsigned a = 0u, b = 0u, c = 0u, d = 0u;
        char dot1 = '\0', dot2 = '\0', dot3 = '\0';

        bool success = ((is >> a) && (is >> dot1) &&
                    (is >> b) && (is >> dot2) &&
                    (is >> c) && (is >> dot3) &&
                    (is >> d) &&
                    dot1 == octetSep && dot2 == octetSep && dot3 == octetSep &&
                    a <= octetMax && b <= octetMax && c <= octetMax && d <= octetMax);
        assert(success);
        ip = IPv4(static_cast<std::uint8_t>(a),
                 static_cast<std::uint8_t>(b),
                 static_cast<std::uint8_t>(c),
                 static_cast<std::uint8_t>(d));
        return is;
    }

private:

    std::uint8_t m_octet[octetCount];
};


int main()
{
    {
        IPv4 a;
        std::ostringstream os;
        os << a;
        assert(os.str() == std::string("0.0.0.0"));
    }

    {
        IPv4 b(static_cast<std::uint8_t>(192),
               static_cast<std::uint8_t>(168),
               static_cast<std::uint8_t>(0),
               static_cast<std::uint8_t>(1));
        std::ostringstream os;
        os << b;
        assert(os.str() == std::string("192.168.0.1"));
    }

    {
        IPv4 x(static_cast<std::uint8_t>(0),
               static_cast<std::uint8_t>(0),
               static_cast<std::uint8_t>(0),
               static_cast<std::uint8_t>(255));
        ++x; 
        std::ostringstream os;
        os << x;
        assert(os.str() == std::string("0.0.1.0"));

        IPv4 y(static_cast<std::uint8_t>(255),
               static_cast<std::uint8_t>(255),
               static_cast<std::uint8_t>(255),
               static_cast<std::uint8_t>(255));
        ++y; 
        std::ostringstream os2;
        os2 << y;
        assert(os2.str() == std::string("0.0.0.0"));

        IPv4 z;
        --z; 
        std::ostringstream os3;
        os3 << z;
        assert(os3.str() == std::string("255.255.255.255"));

        IPv4 p(static_cast<std::uint8_t>(10),
               static_cast<std::uint8_t>(0),
               static_cast<std::uint8_t>(0),
               static_cast<std::uint8_t>(0));
        IPv4 p_post = p++;
        std::ostringstream osp, ospp;
        osp << p_post;   
        ospp << p;     
        assert(osp.str()  == std::string("10.0.0.0"));
        assert(ospp.str() == std::string("10.0.0.1"));
    }

    {
        IPv4 a(static_cast<std::uint8_t>(1), static_cast<std::uint8_t>(2),
               static_cast<std::uint8_t>(3), static_cast<std::uint8_t>(4));
        IPv4 b(static_cast<std::uint8_t>(1), static_cast<std::uint8_t>(2),
               static_cast<std::uint8_t>(3), static_cast<std::uint8_t>(5));
        assert(a < b);
        assert(!(a > b));
        assert(a <= b);
        assert(!(a >= b));
        assert(a != b);

        IPv4 c = a;
        assert(c == a);
        assert(!(c != a));
        assert(!(c < a));
        assert(!(c > a));
        assert(c <= a);
        assert(c >= a);
    }

    {
        std::stringstream is("255.0.128.1");
        IPv4 ip;
        is >> ip;
        std::ostringstream os;
        os << ip;
        assert(os.str() == std::string("255.0.128.1"));
    }

    {
        IPv4 src(static_cast<std::uint8_t>(8), static_cast<std::uint8_t>(8),
                 static_cast<std::uint8_t>(8), static_cast<std::uint8_t>(8));
        std::ostringstream os;
        os << src;

        std::stringstream mid(os.str());
        IPv4 dst;
        mid >> dst;

        assert(src == dst);
    }

    return 0;
}
