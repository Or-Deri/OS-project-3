#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <exception>
#include <deque>

struct Point
{
    float x;
    float y;

    // operator == to 
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

    float calculate_distance(Point a, Point b);

    int orientation(Point a, Point b, Point c);

    void findConvexHull_using_vector();

    void findConvexHull_using_deque();

    float calculate_area();

    const std::vector<Point> &get_convex_vx() const;
};
