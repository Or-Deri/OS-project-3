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
Convex* shared_convex = nullptr;

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];

    // Main loop for client
    while (1) 
    {

        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes <= 0) {
            break;
        }
        
        std::string line(buffer);
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        std::string response;

        // Newgraph command: create a new set of points
        if (cmd == "Newgraph") {
    
            int n;
            ss >> n;
            
            if (n <= 0) {
            
                response = "Number of points must be positive \n";
                send(client_sock, response.c_str(), response.size(), 0);
            
                continue;
            }

            // Remove previous graph if exists
            delete shared_convex;
            shared_convex = new Convex(n);

            response = "Enter " + std::to_string(n) + " points x,y \n";
            send(client_sock, response.c_str(), response.size(), 0);

            // Read n points from the client x,y
            for (int i = 0; i < n; ++i) 
            {

                memset(buffer, 0, BUFFER_SIZE);
                int point_bytes = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
                
                // Client disconnected
                if (point_bytes <= 0){
                    break;
                }

                std::string ptline(buffer);
                size_t comma_pos = ptline.find(',');
                
                if (comma_pos == std::string::npos) {
                    
                    std::string msg = "Invalid input use format x,y\n";
                    send(client_sock, msg.c_str(), msg.size(), 0);
                    
                    --i;
                    continue;
                }
                
                float x;
                float y;

                x = std::stof(ptline.substr(0, comma_pos));
                y = std::stof(ptline.substr(comma_pos + 1));
                
                shared_convex->add_vx(x, y);
            }

            response = "Graph created\n";
        }

        // Add a new point to the current graph
        else if (cmd == "Newpoint") {
        
            std::string cord;
            ss >> cord;
            size_t comma_pos = cord.find(',');
        
            if (!shared_convex) {
                response = "No graph exists. Use Newgraph first\n";
            } 
            else if (comma_pos == std::string::npos) {
                response = "Invalid input, use format Newpoint x,y\n";
            } 
            else {

                float x;
                float y;

                x = std::stof(cord.substr(0, comma_pos));
                y = std::stof(cord.substr(comma_pos + 1));
                
                shared_convex->add_vx(x, y);
                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") added\n";
            }
        }

        // remove a point from the current graph
        else if (cmd == "Removepoint") {
        
            std::string cord;
            ss >> cord;

            size_t comma_pos = cord.find(',');
        
            if (!shared_convex) {
                response = "No graph exists. Use Newgraph first\n";
            } 
            else if (comma_pos == std::string::npos) {
                response = "Invalid input, use format Removepoint x,y\n";
            } 
            else {

                float x = std::stof(cord.substr(0, comma_pos));
                float y = std::stof(cord.substr(comma_pos + 1));


                shared_convex->remove_vx(x, y);

                std::cout << "Point (" << x << "," << y << ") removed" << std::endl;
                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") removed\n";
            }
        }

        // calculate and print the convex hull area
        else if (cmd == "CH") {
            
            if (!shared_convex) {
                response = "No graph exists, use Newgraph to create one \n";
            } 
            else {
                
                shared_convex->findConvexHull();
                auto hull = shared_convex->get_convex_vx();
                
                if (hull.size() < 3) {
                    response = "Convex hull cannot be formed with less than 3 points\n";
                    continue;
                } 

                try {
                    float area = shared_convex->calculate_area();
                    response = "Convex hull area: " + std::to_string(area) + "\n";

                } catch (const std::exception& ex) {
                    response = std::string("Error calculating area: ") + ex.what() + "\n";
                }
            }
        }
        // if command is unknown
        else {
            response = "Unknown command \n";
        }

        // send the response to the client
        send(client_sock, response.c_str(), response.size(), 0);
    }



    // close the connection for this client
    close(client_sock);
}

int main() {

    // create the server socket
    // tcp socket - sock stream
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {

        perror("socket failed");
        exit(1);
    }

    // create server address 
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    // bind the socket to port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {

        perror("bind failed");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {

        perror("listen failed");
        exit(1);
    }

    std::cout << "convex Hull Server started on port " << PORT << std::endl;

    // accept one client 
    while (1) {

        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_sock = accept(server_fd, (struct sockaddr*) &client_addr, &client_len);
        
        if (client_sock < 0) {
        
            perror("accept failed");
            continue;
        }
        
        std::cout << "Client connected." << std::endl;
    
        // process all commands for this client
        handle_client(client_sock);
        
        std::cout << "Client disconnected." << std::endl;
    }



    // clean up shared resources
    delete shared_convex;
    close(server_fd);

    return 0;
}