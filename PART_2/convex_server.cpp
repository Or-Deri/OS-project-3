#include <iostream>
#include <string>
#include <sstream>
#include "../Convex/convex.hpp"

int main()
{
    int num_vx;
    while (true)
    {
        std::cout << "Enter number of vertices for the convex:" << std::endl;
        std::cin >> num_vx;

        if (num_vx <= 0)
        {
            std::cout << "Wrong input for number of vertices. Must be higher than zero." << std::endl;
            continue;
        }
        break;
    }

    
    // Clear newline left in input buffer after reading num_vx
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Convex my_conv(num_vx);
    int counter = 1;
    while (1)
    {
        std::string cords;
        std::getline(std::cin, cords);

        for (auto &c : cords)
        {
            if (c == ',')
                c = ' ';
        }

        std::stringstream ss(cords);
        float x_cord, y_cord;
        if (!(ss >> x_cord >> y_cord))
        {
            std::cout << "Invalid input, please enter in format x,y" << std::endl;
            continue;
        }

        my_conv.add_vx(x_cord, y_cord);

        if (counter == num_vx) 
        {
            break;
        }
        counter++;
    }

    my_conv.findConvexHull();

    auto hull = my_conv.get_convex_vx();

    if (hull.size() < 3)
    {
        std::cout << "Convex hull cannot be formed with less than 3 points." << std::endl;
        return 1;
    }

    try
    {
        float area = my_conv.calculate_area();
        std::cout << "The area of the Convex Hull is - " << area << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cout << "Error calculating area: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
