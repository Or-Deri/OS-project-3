#include "convex.hpp"

/// @brief add vetrex with x y cords to the convex
/// @param x x cord
/// @param y y cord
void Convex::add_vx(float x, float y)
{
    // adding a pair of x,y cords
    vx_pairs.push_back({x, y});
}

/// @brief remove a vertex from the convex
/// @param x cord
/// @param y cord
void Convex::remove_vx(float x, float y)
{
    // we use iterator to traverse the convex vxs
    // if we find the correct vx with the correct x,y cords
    // we remove it.
    for (auto it = vx_pairs.begin(); it != vx_pairs.end(); ++it)
    {
        if (it->x == x && it->y == y)
        {
            vx_pairs.erase(it);
            break;
        }
    }
}

/// @brief calculate the distance between the points of vxs
/// @param a point
/// @param b point
/// @return return the distance as a float val
float Convex::calculate_distance(Point a, Point b)
{
    //
    return ((a.x - b.x) * (a.x - b.x)) + ((a.y - b.y) * (a.y - b.y));
}

/// @brief calc the orientation of the triplet points (a - b - c)
/// @param a first point
/// @param b second point
/// @param c third point
/// @return 1 if  oreintation is counter clockwise, -1 if clockwise, 0 if collinear
int Convex::orientation(Point a, Point b, Point c)
{
    double ans = a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);

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

/// @brief find convex hull using vector
void Convex::findConvexHull_using_vector()
{
    int n = vx_pairs.size();

    if (n < 3)
    {
        convex_vx.clear();
        return; // cannot form hull
    }

    // find point with smallest y (and leftmost if tie)
    Point p0 = *std::min_element(vx_pairs.begin(), vx_pairs.end(), [](Point a, Point b) { // using iterator and lambda func to find the smallest y cord point
        return std::make_pair(a.y, a.x) < std::make_pair(b.y, b.x);
    });

    // sort points by polar angle with p0
    std::sort(
                vx_pairs.begin(), vx_pairs.end(), [&](const Point &a, const Point &b)
              {
                int o = orientation(p0, a, b);
                  if (o == 0)
                  {
                    return calculate_distance(p0, a) < calculate_distance(p0, b);
                  }
                  return o == 1;
                }  
            );

    convex_vx.clear();

    // we use the graham scan using vector
    for (int i = 0; i < n; ++i)
    {
        while (convex_vx.size() > 1 &&
               orientation(convex_vx[convex_vx.size() - 2], convex_vx.back(), vx_pairs[i]) != 1)
            convex_vx.pop_back();

        convex_vx.push_back(vx_pairs[i]);
    }
}

/// @brief find convex hull using deque
void Convex::findConvexHull_using_deque()
{
    int n = vx_pairs.size();

    if (n < 3)
    {
        convex_vx.clear();
        return; // cannot form hull
    }

    // find point with smallest y (and leftmost if tie)
    Point p0 = *std::min_element
    (
                vx_pairs.begin(), vx_pairs.end(), [](Point a, Point b)
                { 
                return std::make_pair(a.y, a.x) < std::make_pair(b.y, b.x); 
                }
    );

    // sort points by polar angle with p0
    std::sort(
                vx_pairs.begin(), vx_pairs.end(), [&](const Point &a, const Point &b)
                {
                  int o = orientation(p0, a, b);
                  if (o == 0) 
                  {
                    return calculate_distance(p0, a) < calculate_distance(p0, b);
                  }
                  return o == 1; 
                }
            );

    std::deque<Point> dq;
        
    // we use the Graham scan using deque
    for (int i = 0; i < n; ++i)
    {
        while (dq.size() > 1 && orientation(*(dq.rbegin() + 1), dq.back(), vx_pairs[i]) != 1)
        {
            dq.pop_back();
        }
        dq.push_back(vx_pairs[i]);
    }
    // copy deque into convex_vx vector
    convex_vx.assign(dq.begin(), dq.end());
}

/// @brief calculate the area of the convex
/// @return area as a float val
float Convex::calculate_area()
{
    // convex area calc requires at least 3 vxs
    if (convex_vx.size() < 3)
        throw std::invalid_argument("needs at least 3 points to calculate area");

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

/// @brief return all the conex points (vxs)
/// @return retunts them as a vector of points(vxs)
const std::vector<Point> &Convex::get_convex_vx() const
{
    return convex_vx;
}