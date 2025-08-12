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

        // create a new graph and read n points
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
                    size_t find_com = line.find(',');

                    if (find_com == std::string::npos)
                    {
                        std::cout << "Invalid input use format x,y" << std::endl;

                        --i; // re-ask for this point
                        continue;
                    }

                    float x;
                    float y;

                    try
                    {
                        x = std::stof(line.substr(0, find_com));
                        y = std::stof(line.substr(find_com + 1));
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

            size_t find_com = cord.find(',');

            if (find_com == std::string::npos)
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
                x = std::stof(cord.substr(0, find_com));
                y = std::stof(cord.substr(find_com + 1));
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
            size_t find_com = rest.find(',');

            if (!my_conv)
            {
                std::cout << "No graph exists. Use Newgraph first" << std::endl;
                continue;
            }
            if (find_com == std::string::npos)
            {
                std::cout << "Invalid input, use format Removepoint x,y" << std::endl;
                continue;
            }

            float x;
            float y;

            try
            {
                x = std::stof(rest.substr(0, find_com));
                y = std::stof(rest.substr(find_com + 1));
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

            // check if convex isnt null
            if (!my_conv)
            {
                std::cout << "No graph exists, use Newgraph to create one" << std::endl;
                continue;
            }

            // we use the area convex cal algo using vectors
            my_conv->findConvexHull_using_vector();
            // 
            auto hull = my_conv->get_convex_vx();

            // convex cant have size below 3 since it wont be a convex anymore
            if (hull.size() < 3)
            {
                std::cout << "Convex hull cannot be formed with less than 3 points" << std::endl;
                continue;
            }

            // calc are a using try/ catch if so that if theres an error 
            // then we catch whats wrong
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
    }

    // delete memory of convex
    delete my_conv;
    return 0;
}
