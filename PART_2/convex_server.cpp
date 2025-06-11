#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "../Convex/convex.hpp"

// Runs convex hull algorithm using vector-based implementation
void run_convexHull_vector(Convex &cvx)
{
    cvx.findConvexHull_using_vector();
}

// Runs convex hull algorithm using deque-based implementation
void run_convexHull_deque(Convex &cvx)
{
    cvx.findConvexHull_using_deque();
}

int main()
{
    int num_vx;

    // Ask the user for the number of vertices
    while (true)
    {
        std::cout << "Enter number of vertices for the convex:" << std::endl;
        std::cin >> num_vx;

        // Validate input is positive
        if (num_vx <= 0)
        {
            std::cout << "Wrong input for number of vertices. Must be higher than zero." << std::endl;
            // Clear bad input state and discard rest of the line
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        break;
    }

    // Clear leftover newline character after reading num_vx
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Create Convex objects for vector and deque implementations
    Convex my_conv_vector(num_vx);
    Convex myconv_deque(num_vx);

    int counter = 0;
    // Read points from user line by line until reaching num_vx
    while (counter < num_vx)
    {
        std::string cords;
        std::getline(std::cin, cords);

        if (cords.empty()) continue; // skip empty lines

        // Replace commas with spaces to ease parsing
        for (auto &c : cords)
        {
            if (c == ',')
                c = ' ';
        }

        std::stringstream ss(cords);
        float x_cord, y_cord;

        // Attempt to read two floats (x, y)
        if (!(ss >> x_cord >> y_cord))
        {
            std::cout << "Invalid input, please enter in format x,y" << std::endl;
            continue;
        }

        // Add point to both Convex instances
        my_conv_vector.add_vx(x_cord, y_cord);
        myconv_deque.add_vx(x_cord, y_cord);

        counter++;
    }

    // Run convex hull computations
    run_convexHull_vector(my_conv_vector);
    run_convexHull_deque(myconv_deque);

    auto hull_vector = my_conv_vector.get_convex_vx();
    auto hull_deque = myconv_deque.get_convex_vx();

    // Check if convex hull was formed properly (need at least 3 points)
    if (hull_vector.size() < 3 || hull_deque.size() < 3)
    {
        std::cout << "Convex hull cannot be formed with less than 3 points." << std::endl;
        return 1;
    }

    try
    {
        // Calculate and print areas of the convex hulls
        float area_vec = my_conv_vector.calculate_area();
        float area_deque = myconv_deque.calculate_area();

        std::cout << "The area of the Convex Hull using two different data structures is - " << std::endl;
        std::cout << "Using Vector - " << area_vec << std::endl;
        std::cout << "Using Deque - " << area_deque << std::endl;
    }
    catch (const std::exception &ex)
    {
        // Handle possible errors during area calculation
        std::cout << "Error calculating area: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
