#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "../Convex/convex.hpp"

/*

for this part we chose to implement the convex area algorithm using the following data structures -

1. vector - 


1. deque - 


*/


// retun the algo using vector
void run_convexHull_vector(Convex &cvx)
{
    cvx.findConvexHull_using_vector();
}

// run the algrothim using deque
void run_convexHull_deque(Convex &cvx)
{
    cvx.findConvexHull_using_deque();
}

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

    // Clear leftover newline character after reading num_vx
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 2 convex instances for profiling
    Convex my_conv_vector(num_vx);
    Convex myconv_deque(num_vx);

    int counter = 1;
    

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

        // add the vxs to both the vector and the deque implementations
        // since profilingf is required for this part
        my_conv_vector.add_vx(x, y);
        myconv_deque.add_vx(x, y);

        if (counter == num_vx) 
        {
            break;
        }
        counter++;
    }

    // run convex hull ch calculations (using algo)
    run_convexHull_vector(my_conv_vector);
    run_convexHull_deque(myconv_deque);

    auto hull_vector = my_conv_vector.get_convex_vx();
    auto hull_deque = myconv_deque.get_convex_vx();

    try
    {
        // check the size of the convex.
        // it needs to be of size at least 3.
        if (hull_vector.size() < 3 || hull_deque.size() < 3)
        {
            std::cout << "convex cannot be formed with less than 3 points." << std::endl;
            return 1;

        }

        // calculate and print areas of the convex hulls
        float area_vec = my_conv_vector.calculate_area();
        float area_deque = myconv_deque.calculate_area();

        // print area calculations
        std::cout << "The area of the Convex Hull using two different data structures is - " << std::endl;
        std::cout << "Using Vector - " << area_vec << std::endl;
        std::cout << "Using Deque - " << area_deque << std::endl;
    }
    catch (const std::exception &ex)
    {
        std::cout << "Error calculating area: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
