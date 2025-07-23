#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include "../Convex/convex.hpp"
#include "../PART_5/reactor.hpp"

#define PORT 9034
#define BUFFER_SIZE 1024

// a global reactor and client state

Reactor* global_reactor = nullptr;

std::string input_buffer[FD_SETSIZE];

Convex* my_conv[FD_SETSIZE] = {nullptr};




// the client handler -
void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];

    int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read <= 0)
    {
        std::cout << "Client disconnected: " << client_fd << std::endl;

        close(client_fd);
        remove_fd_from_reactor(global_reactor, client_fd);

        input_buffer[client_fd].clear();

        if (my_conv[client_fd])
        {
            delete my_conv[client_fd];
            my_conv[client_fd] = nullptr;
        }

        return;
    }

    buffer[bytes_read] = '\0';
    input_buffer[client_fd] += std::string(buffer);

    // handle all complete lines
    size_t pos;
    while ((pos = input_buffer[client_fd].find('\n')) != std::string::npos)
    {
        std::string line = input_buffer[client_fd].substr(0, pos);
        input_buffer[client_fd].erase(0, pos + 1);

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        std::ostringstream response;

        if (cmd == "Newgraph")
        {
            int n;
            ss >> n;

            if (n <= 0)
            {
                response << "Number of points must be positive\n";
            }
            else
            {
                if (my_conv[client_fd])
                    delete my_conv[client_fd];

                my_conv[client_fd] = new Convex(n);

                response << "Send " << n << " points (x,y):\n";
            }
        }
        else if (cmd == "Newpoint")
        {
            std::string rest;
            ss >> rest;
            size_t comma = rest.find(',');

            if (!my_conv[client_fd])
            {
                response << "No graph exists. Use Newgraph first.\n";
            }
            else if (comma == std::string::npos)
            {
                response << "Invalid input, use format Newpoint x,y\n";
            }
            else
            {
                float x = std::stof(rest.substr(0, comma));
                float y = std::stof(rest.substr(comma + 1));

                my_conv[client_fd]->add_vx(x, y);

                response << "Point (" << x << "," << y << ") added\n";
            }
        }
        else if (cmd == "Removepoint")
        {
            std::string rest;
            ss >> rest;
            size_t comma = rest.find(',');

            if (!my_conv[client_fd])
            {
                response << "No graph exists. Use Newgraph first.\n";
            }
            else if (comma == std::string::npos)
            {
                response << "Invalid input, use format Removepoint x,y\n";
            }
            else
            {
                float x = std::stof(rest.substr(0, comma));
                float y = std::stof(rest.substr(comma + 1));

                my_conv[client_fd]->remove_vx(x, y);

                response << "Point (" << x << "," << y << ") removed\n";
            }
        }
        else if (cmd == "CH")
        {
            if (!my_conv[client_fd])
            {
                response << "No graph exists.\n";
            }
            else
            {
                my_conv[client_fd]->findConvexHull_using_vector();
                auto hull = my_conv[client_fd]->get_convex_vx();

                if (hull.size() < 3)
                {
                    response << "Convex hull cannot be formed with less than 3 points\n";
                }
                else
                {
                    float area = my_conv[client_fd]->calculate_area();
                    response << "Convex hull area: " << area << "\n";
                }
            }
        }
        else
        {
            response << "Unknown command\n";
        }

        // always send a response for every line/command
        send(client_fd, response.str().c_str(), response.str().size(), 0);
    }
}

//accept client function
void accept_client(int server_fd)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);

    if (client_fd == -1)
    {
        std::cerr << "Failed to accept client\n";
        return;
    }

    std::cout << "Client connected: " << client_fd << std::endl;

    input_buffer[client_fd].clear();

    if (my_conv[client_fd])
        delete my_conv[client_fd];

    my_conv[client_fd] = nullptr;

    // register this client in the reactor
    add_fd_to_reactor(global_reactor, client_fd, handle_client);

    std::string welcome = "Convex Server ready. Use commands: Newgraph, Newpoint, Removepoint, CH\n";
    send(client_fd, welcome.c_str(), welcome.size(), 0);
}

// Main func
int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        std::cerr << "bind failed\n";
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "listen failed\n";
        return 1;
    }

    std::cout << "Convex Hull Server started on port " << PORT << std::endl;

    // create and run the reactor!
    global_reactor = start_reactor();
    add_fd_to_reactor(global_reactor, server_fd, accept_client);
    run_reactor(global_reactor);

    // cleanup on exit
    close(server_fd);

    for (int i = 0; i < FD_SETSIZE; ++i)
        if (my_conv[i])
            delete my_conv[i];

    delete global_reactor;

    return 0;
}
