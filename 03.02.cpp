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

#include <numbers>
#include <cmath>
#include <cassert>

class Triangle
{
private:
    double m_side1;
    double m_side2;
    double m_side3;

public:
    Triangle(double s1, double s2, double s3) : m_side1(s1), m_side2(s2), m_side3(s3)
    {
        assert(m_side1 > 0 && m_side2 > 0 && m_side3 > 0);
        assert(m_side1 + m_side2 > m_side3);
        assert(m_side1 + m_side3 > m_side2);
        assert(m_side2 + m_side3 > m_side1);
    }

    double perimeter() const
    {
        return m_side1 + m_side2 + m_side3;
    }

    double area() const
    {
        double p = perimeter() / 2.0;
        return std::sqrt(p * (p - m_side1) * (p - m_side2) * (p - m_side3));
    }
};

class Square
{
private:
    double m_side;

public:
    explicit Square(double side) : m_side(side)
    {
        assert(m_side > 0);
    }

    double perimeter() const
    {
        return 4.0 * m_side;
    }

    double area() const
    {
        return m_side * m_side;
    }
};

class Circle
{
private:
    double m_radius;

public:
    explicit Circle(double radius) : m_radius(radius)
    {
        assert(m_radius > 0);
    }

    double perimeter() const
    {
        return 2.0 * std::numbers::pi * m_radius;
    }

    double area() const
    {
        return std::numbers::pi * m_radius * m_radius;
    }
};

int main()
{
    Triangle triangle(3.0, 4.0, 5.0);
    assert(std::abs(triangle.perimeter() - 12.0) < 1e-3);
    assert(std::abs(triangle.area() - 6.0) < 1e-3);

    Square square(5.0);
    assert(std::abs(square.perimeter() - 20.0) < 1e-3);
    assert(std::abs(square.area() - 25.0) < 1e-3);

    Circle circle(2.0);
    assert(std::abs(circle.perimeter() - 4.0 * std::numbers::pi) < 1e-3);
    assert(std::abs(circle.area() - 4.0 * std::numbers::pi) < 1e-3);

    return 0;
}