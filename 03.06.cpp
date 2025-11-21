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
#include <cmath>
#include <numbers>
#include <memory>
#include <vector>

class Shape
{
public:
    virtual ~Shape() = default;

    virtual double perimeter() const = 0;
    virtual double area() const = 0;
};

class Triangle : public Shape
{
private:
    double m_side1;
    double m_side2;
    double m_side3;

public:
    Triangle(double s1, double s2, double s3)
        : m_side1(s1), m_side2(s2), m_side3(s3)
    {        
        assert(m_side1 > 0 && m_side2 > 0 && m_side3 > 0);
        assert(m_side1 + m_side2 > m_side3);
        assert(m_side1 + m_side3 > m_side2);
        assert(m_side2 + m_side3 > m_side1);
    }

    double perimeter() const override final
    {
        return m_side1 + m_side2 + m_side3;
    }

    double area() const override final
    {
        const double p = perimeter() / 2.0;
        return std::sqrt(p * (p - m_side1) * (p - m_side2) * (p - m_side3));
    }
};

class Square final : public Shape
{
private:
    double m_side;

public:
    explicit Square(double side)
        : m_side(side)
    {
        assert(m_side > 0.0);
    }

    double perimeter() const override
    {
        return 4.0 * m_side;
    }

    double area() const override
    {
        return m_side * m_side;
    }
};

class Circle final : public Shape
{
private:
    double m_radius;

public:
    explicit Circle(double r)
        : m_radius(r)
    {
        assert(m_radius > 0.0);
    }

    double perimeter() const override
    {
        return 2.0 * std::numbers::pi * m_radius;
    }

    double area() const override
    {
        return std::numbers::pi * m_radius * m_radius;
    }
};

int main()
{
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.emplace_back(std::make_unique<Triangle>(3.0, 4.0, 5.0));
    shapes.emplace_back(std::make_unique<Square>(2.0));
    shapes.emplace_back(std::make_unique<Circle>(1.0));

    {
        const double p = shapes[0]->perimeter();
        const double a = shapes[0]->area();
        assert(std::abs(p - 12.0) < 1e-3);
        assert(std::abs(a - 6.0) < 1e-3);
    }

    {
        const double p = shapes[1]->perimeter();
        const double a = shapes[1]->area();
        assert(std::abs(p - 8.0) < 1e-3);
        assert(std::abs(a - 4.0) < 1e-3);
    }

    {
        const double p = shapes[2]->perimeter();
        const double a = shapes[2]->area();
        assert(std::abs(p - 2.0 * std::numbers::pi) < 1e-3);
        assert(std::abs(a - 1.0 * std::numbers::pi) < 1e-3);
    }

    return 0;
}
