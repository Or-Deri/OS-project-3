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

#define BACKLOG 10

// Server settings
constexpr int PORT = 9034;
constexpr int BUFFER_SIZE = 1024;

// The convex object for all clients
Convex* shared_convex = nullptr;

pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;

  
void handle_client_commands(int client_fd) {
    char buffer[BUFFER_SIZE];

    // Send instructions to the client
    std::string welcome = "Convex Server ready. Use commands: Newgraph, Newpoint, Removepoint, CH\n";
    send(client_fd, welcome.c_str(), welcome.size(), 0);

    // Main loop for client
    while (true) {

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

        // Calculate and print the convex hull area
        else if (cmd == "CH") {

            if (!shared_convex) {
                response = "No graph exists, use Newgraph to create one \n";
            } 
            else {
                
                shared_convex->findConvexHull();
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
    // Close the connection for this client
    close(client_fd);
}

void *client_thread(void *arg) {
    int client_fd = *((int*)arg);
    free(arg);

    handle_client_commands(client_fd);

    close(client_fd);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in serv_addr;

    // socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // bind
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // clients
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);


    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int *client_fd = (int*)malloc(sizeof(int));
        if (!client_fd) {
            perror("malloc");
            continue;
        }

        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }
        pthread_detach(tid);
    }

    
    pthread_mutex_destroy(&graph_mutex);
    close(server_fd);
    return 0;
}
