#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include "../Convex/convex.hpp"

int main()
{
    Convex *my_conv = nullptr;

    std::string line;

    while (1)
    {

        std::getline(std::cin, line);

        std::stringstream ss(line);

        std::string cmd;

        ss >> cmd;

        // Create a new graph and read n points
        if (cmd == "Newgraph")
        {

            try
            {

                int n;
                ss >> n;
                if (n <= 0)
                {
                    std::cout << "Number of points must be positive" << std::endl;
                    continue;
                }

                delete my_conv;
                my_conv = new Convex(n);

                std::cout << "Enter " << n << " points x,y" << std::endl;

                // Read n points from the client x,y

                for (int i = 0; i < n; ++i)
                {

                    std::getline(std::cin, line);
                    size_t comma_pos = line.find(',');

                    if (comma_pos == std::string::npos)
                    {
                        std::cout << "Invalid input use format x,y" << std::endl;

                        --i; // re-ask for this point
                        continue;
                    }

                    float x;
                    float y;

                    try
                    {
                        x = std::stof(line.substr(0, comma_pos));
                        y = std::stof(line.substr(comma_pos + 1));
                    }
                    catch (const std::exception &ex)
                    {
                        std::cout << "Invalid number format: " << ex.what() << std::endl;
                        --i;
                        continue;
                    }

                    my_conv->add_vx(x, y);
                }
                std::cout << "Graph created " << std::endl;
            }
            catch (const std::exception &ex)
            {
                // error with graph creation
                std::cout << "error creating graph: " << ex.what() << std::endl;
                continue;
            }
        }

        // add a new point action
        else if (cmd == "Newpoint")
        {

            std::string cord;
            ss >> cord;

            size_t comma_pos = cord.find(',');

            if (comma_pos == std::string::npos)
            {
                std::cout << "Invalid input, use format Newpoint x,y" << std::endl;
                continue;
            }

            if (!my_conv)
            {
                std::cout << "No graph exists. Use Newgraph first" << std::endl;
                continue;
            }

            float x;
            float y;

            try
            {
                x = std::stof(cord.substr(0, comma_pos));
                y = std::stof(cord.substr(comma_pos + 1));
            }
            catch (const std::exception &ex)
            {
                std::cout << "Invalid number format: " << ex.what() << std::endl;
                continue;
            }

            my_conv->add_vx(x, y);

            std::cout << "Point (" << x << "," << y << ") added" << std::endl;
        }

        // remove point option
        else if (cmd == "Removepoint")
        {

            std::string rest;
            ss >> rest;
            size_t comma_pos = rest.find(',');

            if (!my_conv)
            {
                std::cout << "No graph exists. Use Newgraph first" << std::endl;
                continue;
            }
            if (comma_pos == std::string::npos)
            {
                std::cout << "Invalid input, use format Removepoint x,y" << std::endl;
                continue;
            }

            float x;
            float y;

            try
            {
                x = std::stof(rest.substr(0, comma_pos));
                y = std::stof(rest.substr(comma_pos + 1));
            }
            catch (const std::exception &ex)
            {
                std::cout << "Invalid number format: " << ex.what() << std::endl;
                continue;
            }

            my_conv->remove_vx(x, y);

            std::cout << "Point (" << x << "," << y << ") removed" << std::endl;
        }

        // calculate the convx area
        else if (cmd == "CH")
        {

            if (!my_conv)
            {
                std::cout << "No graph exists, use Newgraph to create one" << std::endl;
                continue;
            }

            my_conv->findConvexHull_using_vector();
            auto hull = my_conv->get_convex_vx();

            if (hull.size() < 3)
            {
                std::cout << "Convex hull cannot be formed with less than 3 points" << std::endl;
                continue;
            }

            try
            {
                float area = my_conv->calculate_area();
                std::cout << "Convex hull area: " << area << std::endl;
            }
            catch (const std::exception &ex)
            {
                std::cout << "Error calculating area: " << ex.what() << std::endl;
            }
        }

        else
        {
            std::cout << "Unknown command " << std::endl;
        }
        std::cout << "Enter next command:" << std::endl;
    }

    // delete memory of convex
    delete my_conv;
    return 0;
}
