#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include "../Convex/convex.hpp"
#include "../PART_5/reactor.hpp"

// beej's port
// and a buffer size 1024
const int PORT = 9034;
const int BUFFER_SIZE = 1024;

// a global reactor and client state

Reactor* global_reactor = nullptr;

std::string input_buffer[FD_SETSIZE];

// shared graph for all clients
Convex* shared_conv = nullptr;


// the client handler -
void handle_clients(int client_fd)
{
    char buffer[BUFFER_SIZE];

    int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read <= 0)
    {
        std::cout << "Client disconnected: " << client_fd << std::endl;

        close(client_fd);
        remove_fd_from_reactor(global_reactor, client_fd);

        input_buffer[client_fd].clear();

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
                if (shared_conv)
                    delete shared_conv;

                shared_conv = new Convex(n);

                response << "Send " << n << " points (x,y):\n";
            }
        }
        else if (cmd == "Newpoint")
        {
            std::string rest;
            ss >> rest;
            size_t comma = rest.find(',');

            if (!shared_conv)
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

                shared_conv->add_vx(x, y);

                response << "Point (" << x << "," << y << ") added\n";
            }
        }
        else if (cmd == "Removepoint")
        {
            std::string rest;
            ss >> rest;
            size_t comma = rest.find(',');

            if (!shared_conv)
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

                shared_conv->remove_vx(x, y);

                response << "Point (" << x << "," << y << ") removed\n";
            }
        }
        else if (cmd == "CH")
        {
            if (!shared_conv)
            {
                response << "No graph exists.\n";
            }
            else
            {
                shared_conv->findConvexHull_using_vector();
                auto hull = shared_conv->get_convex_vx();

                if (hull.size() < 3)
                {
                    response << "Convex hull cannot be formed with less than 3 points\n";
                }
                else
                {
                    float area = shared_conv->calculate_area();
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

    // register this client in the reactor
    add_fd_to_reactor(global_reactor, client_fd, handle_clients);

}

// main function
int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        std::cerr << "bind failed\n";
        return 1;
    }

    // listen for client connections
    int opt = listen(server_fd, 10);

    if (opt < 0)
    {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    std::cout << "convex Hull Server started on port " << PORT << std::endl;

    // create and run the reactor!
    global_reactor = start_reactor();

    add_fd_to_reactor(global_reactor, server_fd, accept_client);


    run_reactor(global_reactor);

    // cleanup on exit
    close(server_fd);

    // check if not nullptr
    if (shared_conv != nullptr) 
    {
        delete shared_conv;
    }

    delete global_reactor;

    return 0;
}
