#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <sstream>
#include "../Convex/convex.hpp"
#include "../PART_8/reactor.hpp"

// how many pending connections the queue holds
#define BACKLOG 10

// Server settings
constexpr int PORT = 9034;
constexpr int BUFFER_SIZE = 1024;

// The convex object for all clients
Convex* shared_convex = nullptr;

pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;

  
void handle_client_commands(int client_fd) {
    char buffer[BUFFER_SIZE];

    // Main loop for client
    while (1) {

        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
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
                send(client_fd, response.c_str(), response.size(), 0);
            
                continue;
            }

            // Remove previous graph if exists
            pthread_mutex_lock(&graph_mutex);
            delete shared_convex;
            shared_convex = new Convex(n);

            response = "Enter " + std::to_string(n) + " points x,y \n";
            send(client_fd, response.c_str(), response.size(), 0);

            // Read n points from the client x,y
            for (int i = 0; i < n; ++i) {

                memset(buffer, 0, BUFFER_SIZE);
                int point_bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
                
                // Client disconnected
                if (point_bytes <= 0){
                    break;
                }

                std::string ptline(buffer);
                size_t comma_pos = ptline.find(',');
                
                if (comma_pos == std::string::npos) {
                    
                    std::string msg = "Invalid input use format x,y\n";
                    send(client_fd, msg.c_str(), msg.size(), 0);
                    
                    --i;
                    continue;
                }
                
                float x;
                float y;

                x = std::stof(ptline.substr(0, comma_pos));
                y = std::stof(ptline.substr(comma_pos + 1));
                
                shared_convex->add_vx(x, y);
            }
            pthread_mutex_unlock(&graph_mutex);
            response = "Graph created\n";
        }

        // Add a new point to the current graph
        else if (cmd == "Newpoint") {
        
            std::string rest;
            ss >> rest;
            size_t comma_pos = rest.find(',');
        
            pthread_mutex_lock(&graph_mutex);
            if (!shared_convex) {
                response = "No graph exists. Use Newgraph first\n";
            } 
            
            else if (comma_pos == std::string::npos) {
                response = "Invalid input, use format Newpoint x,y\n";
            } 
            else {

                float x;
                float y;

                x = std::stof(rest.substr(0, comma_pos));
                y = std::stof(rest.substr(comma_pos + 1));
                
                shared_convex->add_vx(x, y);
                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") added\n";
            }
            pthread_mutex_unlock(&graph_mutex);
        }

        // Remove a point from the current graph
        else if (cmd == "Removepoint") {

            std::string rest;
            ss >> rest;
            size_t comma_pos = rest.find(',');

            pthread_mutex_lock(&graph_mutex);

            if (!shared_convex) {
                response = "No graph exists. Use Newgraph first\n";
            } 
            else if (comma_pos == std::string::npos) {
                response = "Invalid input, use format Removepoint x,y\n";
            } 
            else {
                float x = std::stof(rest.substr(0, comma_pos));
                float y = std::stof(rest.substr(comma_pos + 1));
                shared_convex->remove_vx(x, y);
                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") removed\n";
            }
            pthread_mutex_unlock(&graph_mutex);
        }

        // calculate and print the convex hull area
        else if (cmd == "CH") {

            if (!shared_convex) {
                response = "No graph exists, use Newgraph to create one \n";
            } 
            else 
            {
                
                shared_convex->findConvexHull_using_vector();
                auto hull = shared_convex->get_convex_vx();
                
                if (hull.size() < 3) {
                    response = "Convex hull cannot be formed with less than 3 points\n";
                    send(client_fd, response.c_str(), response.size(), 0);
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
        // If command is unknown
        else {
            response = "Unknown command \n";
        }

        // Send the response to the client
        send(client_fd, response.c_str(), response.size(), 0);
    }
    // close the connection for this client
    close(client_fd);
}

void* client_handler(int client_fd) {
    handle_client_commands(client_fd);
    return nullptr;
}

int main() {
    int server_fd;
    struct sockaddr_in serv_addr;

    // socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // bind
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    int opt = listen(server_fd, BACKLOG);

    if (opt < 0)
    {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    pthread_t proactor_thread = start_proactor(server_fd, client_handler);

    pthread_join(proactor_thread, nullptr);

    pthread_mutex_destroy(&graph_mutex);

    pthread_mutex_lock(&graph_mutex);
    delete shared_convex;
    shared_convex = nullptr;
    pthread_mutex_unlock(&graph_mutex);

    close(server_fd);
    return 0;
}