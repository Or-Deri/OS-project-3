#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include "../Convex/convex.hpp"

// Server settings
#define PORT 9034
#define BUFFER_SIZE 1024

// The convex object for all clients
// required by the question
Convex *shared_convex = nullptr;

void handle_clients(int srverfd_)
{
    char buffer[BUFFER_SIZE];

    fd_set mas_fd;
    fd_set read_fd;

    int fdmax_ = srverfd_;

    FD_ZERO(&mas_fd);
    FD_ZERO(&read_fd);

    FD_SET(srverfd_, &mas_fd);

    // loop for the clients - handling them based of the fds
    while (1)
    {
        read_fd = mas_fd;

        if (select(fdmax_ + 1, &read_fd, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= fdmax_; ++i)
        {
            if (FD_ISSET(i, &read_fd))
            {
                // new connection from a client to the server
                if (i == srverfd_)
                {
                    sockaddr_in client_address;
                    socklen_t address_length = sizeof(client_address);
                    int accept_fd = accept(srverfd_, (struct sockaddr *)&client_address, &address_length);
                    if (accept_fd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(accept_fd, &mas_fd);
                        if (accept_fd > fdmax_)
                        {
                            fdmax_ = accept_fd;
                        }
                        std::cout << "New client connected" << std::endl;
                    }
                }
                else
                {
                    memset(buffer, 0, BUFFER_SIZE);
                    int byte = recv(i, buffer, BUFFER_SIZE - 1, 0);

                    if (byte <= 0)
                    {
                        if (byte == 0)
                        {
                            std::cout << "client has disconnected" << std::endl;
                        }
                        else
                        {
                            // error connecting to the server
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &mas_fd);
                    }
                    else
                    {
                        std::string line(buffer);
                        std::stringstream ss(line);
                        std::string cmd;
                        ss >> cmd;

                        std::string r;

                        // new graph command to create a new graph
                        if (cmd == "Newgraph")
                        {
                            int n;
                            ss >> n;

                            if (n <= 0)
                            {
                                r = "Number of points must be positive \n";
                                send(i, r.c_str(), r.size(), 0);
                            }

                            // remove the previous graph if exists
                            delete shared_convex;
                            shared_convex = new Convex(n);

                            r = "Enter " + std::to_string(n) + " points x,y \n";
                            send(i, r.c_str(), r.size(), 0);

                            // read n points from the client x,y
                            for (int j = 0; j < n; ++j)
                            {
                                memset(buffer, 0, BUFFER_SIZE);
                                int point_bytes = recv(i, buffer, BUFFER_SIZE - 1, 0);

                                // client disconnected
                                if (point_bytes <= 0)
                                {
                                    break;
                                }

                                std::string ptline(buffer);
                                size_t comma_pos = ptline.find(',');

                                if (comma_pos == std::string::npos)
                                {
                                    std::string msg = "Invalid input use format x,y\n";
                                    send(i, msg.c_str(), msg.size(), 0);
                                    --j;
                                    continue;
                                }

                                float x;
                                float y;

                                x = std::stof(ptline.substr(0, comma_pos));
                                y = std::stof(ptline.substr(comma_pos + 1));

                                shared_convex->add_vx(x, y);
                            }

                            r = "Graph created\n";
                        }

                        // Add a new point to the current graph
                        else if (cmd == "Newpoint")
                        {
                            std::string cord;
                            ss >> cord;
                            size_t comma_pos = cord.find(',');

                            if (!shared_convex)
                            {
                                r = "No graph exists. Use Newgraph first\n";
                            }
                            else if (comma_pos == std::string::npos)
                            {
                                r = "Invalid input, use format Newpoint x,y\n";
                            }
                            else
                            {
                                float x;
                                float y;

                                x = std::stof(cord.substr(0, comma_pos));
                                y = std::stof(cord.substr(comma_pos + 1));

                                shared_convex->add_vx(x, y);
                                r = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") added\n";
                            }
                        }

                        // remove a point from the current graph
                        else if (cmd == "Removepoint")
                        {
                            std::string cord;
                            ss >> cord;
                            size_t comma_pos = cord.find(',');

                            if (!shared_convex)
                            {
                                r = "No graph exists. Use Newgraph first\n";
                            }
                            else if (comma_pos == std::string::npos)
                            {
                                r = "Invalid input, use format Removepoint x,y\n";
                            }
                            else
                            {
                                float x = std::stof(cord.substr(0, comma_pos));
                                float y = std::stof(cord.substr(comma_pos + 1));

                                shared_convex->remove_vx(x, y);

                                std::cout << "Point (" << x << "," << y << ") removed" << std::endl;
                                r = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") removed\n";
                            }
                        }

                        // calculate and print the convex hull area
                        else if (cmd == "CH")
                        {
                            if (!shared_convex)
                            {
                                r = "No graph exists, use Newgraph to create one \n";
                            }
                            else
                            {
                                shared_convex->findConvexHull_using_vector();
                                auto hull = shared_convex->get_convex_vx();

                                if (hull.size() < 3)
                                {
                                    r = "Convex hull cannot be formed with less than 3 points\n";
                                }
                                else
                                {
                                    try
                                    {
                                        float area = shared_convex->calculate_area();
                                        r = "Convex hull area: " + std::to_string(area) + "\n";
                                    }
                                    catch (const std::exception &ex)
                                    {
                                        r = std::string("Error calculating area: ") + ex.what() + "\n";
                                    }
                                }
                            }
                        }
                        // if command is unknown
                        else
                        {
                            r = "Unknown command \n";
                        }

                        // send the response to the client
                        send(i, r.c_str(), r.size(), 0);
                    }
                }
            }
        }
    }
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket failed");
        exit(1);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen failed");
        exit(1);
    }

    std::cout << "convex Hull Server started on port " << PORT << std::endl;

    handle_clients(server_fd);

    delete shared_convex;
    close(server_fd);

    return 0;
}
