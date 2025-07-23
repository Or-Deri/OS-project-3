#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "../Convex/convex.hpp"


int main()
{

    int num_vx;
    // Ask the user for the number of vertices for the convex
    while (true)
    {
        std::cout << "Enter number of vertices for the convex:" << std::endl;
        std::cin >> num_vx;

        // Check that the input is valid 
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

    Convex my_conv(num_vx);
    int counter = 1;

    // Read points from the user, one by one
    while (1)
    {
        std::string cords;
        std::getline(std::cin, cords);

        // Replace comma with space to make input parsing easier
        for (auto &c : cords)
        {
            if (c == ',')
                c = ' ';
        }

        std::stringstream ss(cords);
        float x_cord, y_cord;

        // Try to read two floating point numbers x,y
        if (!(ss >> x_cord >> y_cord))
        {
            std::cout << "Invalid input, please enter in format x,y" << std::endl;
            continue;
        }

        // Add the point to the convex polygon
        my_conv.add_vx(x_cord, y_cord);

        // If we have enough points stop reading more
        if (counter == num_vx) 
        {
            break;
        }
        counter++;
    }

    // Find the convex hull of the given points
    my_conv.findConvexHull();

    // Get the points that make up the convex hull
    auto hull = my_conv.get_convex_vx();

    // If there are less than 3 points in the hull cannot form a polygon
    if (hull.size() < 3)
    {
        std::cout << "Convex hull cannot be formed with less than 3 points." << std::endl;
        return 1;
    }

    try
    {
        // Calculate the area of the convex hull
        float area = my_conv.calculate_area();
        std::cout << "The area of the Convex Hull is - " << area << std::endl;
    }
    catch (const std::exception &ex)
    {
        // If there was an error during area calculation
        std::cout << "Error calculating area: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
