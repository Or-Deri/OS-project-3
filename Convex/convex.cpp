#include "convex.hpp"

void Convex::add_vx(float x, float y)
{
    vx_pairs.push_back({x, y});
}

float Convex::calculate_distance(Point a, Point b)
{
    return ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y));
}

int Convex::orientation(Point a, Point b, Point c)
{
    double ans = a.x * (b.y - c.y) +b.x * (c.y - a.y) + c.x * (a.y - b.y);

    if (ans < 0) 
    {
        return -1; // clockwise
    }
    if (ans > 0) 
    {
        return 1; // counter clockwise
    }
    return 0; // collinear
}

void Convex::findConvexHull_using_vector()
{
    int n = vx_pairs.size();

    if (n < 3)
    {
        convex_vx.clear();
        return; // cannot form hull
    }

    // Find point with smallest y (and leftmost if tie)
    Point p0 = *std::min_element(vx_pairs.begin(), vx_pairs.end(), [](Point a, Point b)
                                 {
                                     return std::make_pair(a.y, a.x) < std::make_pair(b.y, b.x);
                                 });

    // Sort points by polar angle with p0
    std::sort(vx_pairs.begin(), vx_pairs.end(), [&](const Point &a, const Point &b)
              {
                  int o = orientation(p0, a, b);
                  if (o == 0)
                      return calculate_distance(p0, a) < calculate_distance(p0, b);
                  return o == 1;
              });

    convex_vx.clear();

    for (int i = 0; i < n; ++i)
    {
        while (convex_vx.size() > 1 &&
               orientation(convex_vx[convex_vx.size() - 2], convex_vx.back(), vx_pairs[i]) != 1)
            convex_vx.pop_back();

        convex_vx.push_back(vx_pairs[i]);
    }
}


#include <deque>

void Convex::findConvexHull_using_deque()
{
    int n = vx_pairs.size();

    if (n < 3)
    {
        convex_vx.clear();
        return; // cannot form hull
    }

    // Find point with smallest y (and leftmost if tie)
    Point p0 = *std::min_element(vx_pairs.begin(), vx_pairs.end(), [](Point a, Point b) { return std::make_pair(a.y, a.x) < std::make_pair(b.y, b.x);});

    // Sort points by polar angle with p0
    std::sort(vx_pairs.begin(), vx_pairs.end(), [&](const Point &a, const Point &b)
              {
                  int o = orientation(p0, a, b);
                  if (o == 0)
                      return calculate_distance(p0, a) < calculate_distance(p0, b);
                  return o == 1;
              });

    std::deque<Point> dq;

    for (int i = 0; i < n; ++i)
    {
        while (dq.size() > 1 &&
               orientation(*(dq.rbegin() + 1), dq.back(), vx_pairs[i]) != 1)
        {
            dq.pop_back();
        }
        dq.push_back(vx_pairs[i]);
    }

    convex_vx.assign(dq.begin(), dq.end());
}

float Convex::calculate_area()
{
    if (convex_vx.size() < 3)
        throw std::invalid_argument("Need at least 3 points to calculate area");

    float area = 0;
    int n = convex_vx.size();

    for (int i = 0; i < n; ++i)
    {
        Point p1 = convex_vx[i];
        Point p2 = convex_vx[(i + 1) % n];
        area += p1.x * p2.y - p2.x * p1.y;
    }

    return std::abs(area) / 2.0f;
}

const std::vector<Point> &Convex::get_convex_vx() const
{
    return convex_vx;
}
