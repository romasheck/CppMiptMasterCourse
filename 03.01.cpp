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

//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

//////////////////////////////////////////////////////////////////////

struct Rectangle
{ 
	int left_x = 0, top_y = 0, right_x = 0, bottom_y = 0;
    
    Rectangle (int leftx = 0, int topy = 0, int rightx = 0, int bottomy = 0) : 
               left_x(leftx), top_y(topy), right_x(rightx), bottom_y(bottomy) {}

    bool isEmpty() const 
    {
        return left_x > right_x || bottom_y < top_y;
    }

    bool isDegenerate() const 
    {
        return left_x == right_x || bottom_y == top_y;
    }

    int width() const 
    {
        return right_x - left_x;
    }
    
    int height() const 
    {
        return bottom_y - top_y;
    }

    int area() const 
    {
        if (isEmpty()) 
            return 0;
        return width() * height();
    }

};

Rectangle intersection(const std::vector<Rectangle>& rectangles) 
{
    if (rectangles.empty()) 
        return 0;
    
    int left_x = std::ranges::max_element(rectangles, {}, &Rectangle::left_x)->left_x;
    int top_y = std::ranges::max_element(rectangles, {}, &Rectangle::top_y)->top_y;
    int right_x = std::ranges::min_element(rectangles, {}, &Rectangle::right_x)->right_x;
    int bottom_y = std::ranges::min_element(rectangles, {}, &Rectangle::bottom_y)->bottom_y;
    
    auto res = Rectangle(left_x, top_y, right_x, bottom_y);
    if (res.isEmpty()) 
        return Rectangle();
    return res;
}

int intersectionArea(const std::vector<Rectangle>& rectangles) 
{
    return intersection(rectangles).area();
}

Rectangle boundingBox(const std::vector<Rectangle>& rectangles)
{
    if (rectangles.empty()) {
        return Rectangle();
    }
    
    int min_left_x = rectangles[0].left_x;
    int min_top_y = rectangles[0].top_y;
    int max_right_x = rectangles[0].right_x;
    int max_bottom_y = rectangles[0].bottom_y;
    
    for (const auto& rect : rectangles) 
    {    
        min_left_x = std::min(min_left_x, rect.left_x);
        min_top_y = std::min(min_top_y, rect.top_y);
        max_right_x = std::max(max_right_x, rect.right_x);
        max_bottom_y = std::max(max_bottom_y, rect.bottom_y);
    }
    
    return Rectangle(min_left_x, min_top_y, max_right_x, max_bottom_y);
}

//////////////////////////////////////////////////////////////////////

int main()
{
    std::vector<Rectangle> rects = {
        Rectangle(0, 0, 10, 10),   
        Rectangle(5, 5, 15, 15),   
        Rectangle(8, 8, 12, 12)    
    };
    
    
    assert(intersectionArea(rects) == 4);
    
    Rectangle bbox = boundingBox(rects);
    assert(bbox.left_x == 0 && bbox.top_y == 0 && bbox.right_x == 15 && bbox.bottom_y == 15);
    
    std::vector<Rectangle> noIntersection = {
        Rectangle(0, 0, 5, 5),
        Rectangle(10, 10, 15, 15)
    };
    
    assert(intersectionArea(noIntersection) == 0);

    return 0;
}

//////////////////////////////////////////////////////////////////////