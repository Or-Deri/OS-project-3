#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "../Convex/convex.hpp"

int main()
{
    int num_vx;


    // ask the user for the number of vertices for the convex
    while (1)
    {
        std::cout << "Enter number of vertices for the convex:" << std::endl;
        std::cin >> num_vx;

        // check that the input is valid
        if (num_vx <= 0)
        {
            std::cout << "Wrong input for number of vertices. Must be higher than zero." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        break;
    }

    // Clear newline left in input buffer after reading num_vx
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // create an empty convex 
    // for the user to build
    // note - convex object is created from the convex class 
    Convex my_conv(num_vx);

    // counter for the vertices
    int counter = 1;

    // Read points from the user, one by one
    while (1)
    {
        std::string cords;
        std::getline(std::cin, cords);

        // remove the "," symbol and replace it with a blank symbol
        for (auto &c : cords)
        {
            if (c == ',')
                c = ' ';
        }

        std::stringstream ss(cords);

        float x; 
        float y;

        // read the x y cords
        if (!(ss >> x >> y))
        {
            std::cout << "Invalid input, please enter in format x,y" << std::endl;
            continue;
        }

        // add the point to the convex polygon
        my_conv.add_vx(x, y);

        // if we have enough points stop reading more
        if (counter == num_vx)
        {
            break;
        }
        counter++;
    }

    // Find the convex hull of the given points
    my_conv.findConvexHull_using_vector();

    // Get the points that make up the convex hull
    auto hull = my_conv.get_convex_vx();

    

    try
    {
        // check the size of the convex.
        // it needs to be of size at least 3.
        if (hull.size() < 3)
        {
            std::cout << "convex cannot be formed with less than 3 points." << std::endl;
            return 1;

        }

        // calculate the area of the convex hull
        float area = my_conv.calculate_area();
        std::cout << "the area of the convex is - " << area << std::endl;
    }
    catch (const std::exception &ex)
    {
        // If there was an error during area calculation
        std::cout << "error when calculating area: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
