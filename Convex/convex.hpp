#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <exception>

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <exception>

struct Point
{
    float x;
    float y;

    bool operator==(const Point &t) const
    {
        return x == t.x && y == t.y;
    }
    
};

class Convex
{

private:
    int num_vx;
    std::vector<Point> vx_pairs;
    std::vector<Point> convex_vx;

public:
    Convex(int num) : num_vx(num) {}

    void add_vx(float x, float y);

    void remove_vx(float x, float y);

    int calculate_distance(Point a, Point b);

    int orientation(Point a, Point b, Point c);

    void findConvexHull();

    float calculate_area();

    const std::vector<Point> &get_convex_vx() const;
};