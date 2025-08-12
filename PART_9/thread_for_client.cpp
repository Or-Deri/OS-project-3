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

// beej's port
// and a buffer size 1024
const int PORT = 9034;
const int BUFFER_SIZE = 1024;

// the convex object for all clients
Convex* shared_convex = nullptr;

// mutex to synchronize the access to the clients shared convex
pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_client_commands(int client_fd) {
    char buffer[BUFFER_SIZE];

    // loop for the client commands (multiple clients and not just one)
    while (1) {

        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes <= 0) {
            // if bytes ==0 then client has disconnected
            // else, there was an error connecting
            break;
        }
        
        // we need to parse the commands given by the clients
        std::string line(buffer);
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        std::string response;

        // newgraph - create a newgraph with a number of vertices specified
        if (cmd == "Newgraph") {
            
            int n;
            ss >> n;
            
            if (n <= 0) {
            
                response = "Number of points must be positive \n";
                send(client_fd, response.c_str(), response.size(), 0);
            
                continue;
            }

            // remove previous graph if exists
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
                if (point_bytes <= 0)

                {
                    // if bytes ==0 then client has disconnected
                    // else, there was an error connecting
                    break;
                }


                std::string ptline(buffer);
                size_t find_com = ptline.find(',');
                
                if (find_com == std::string::npos) {
                    
                    std::string msg = "Invalid input use format x,y\n";
                    send(client_fd, msg.c_str(), msg.size(), 0);
                    
                    --i;
                    continue;
                }
                


                float x;
                float y;

                x = std::stof(ptline.substr(0, find_com));
                y = std::stof(ptline.substr(find_com + 1));
                
                shared_convex->add_vx(x, y);
            }
            pthread_mutex_unlock(&graph_mutex);
            response = "Graph created\n";
        }

        // Add a new point to the current graph
        else if (cmd == "Newpoint") {
            

            std::string p;
            ss >> p;
            size_t find_com = p.find(',');
        
            // lock before adding a new point
            pthread_mutex_lock(&graph_mutex);

            // check if graph exists
            if (!shared_convex) {
                response = "No graph exists. Use Newgraph first\n";
            } 
            
            else if (find_com == std::string::npos) {
                response = "Invalid input, use format Newpoint x,y\n";
            } 
            else {

                float x;
                float y;

                x = std::stof(p.substr(0, find_com));
                y = std::stof(p.substr(find_com + 1));
                
                shared_convex->add_vx(x, y);
                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") added\n";
            }
            pthread_mutex_unlock(&graph_mutex);
        }

        // Remove a point from the current graph
        else if (cmd == "Removepoint") {

            std::string p;
            ss >> p;
            size_t find_com = p.find(',');

            // lock before removing point to avoid conflicts
            pthread_mutex_lock(&graph_mutex);

            // check if graph exists
            if (!shared_convex) {
                response = "No graph exists. Use Newgraph first\n";
            } 
            else if (find_com == std::string::npos) {
                response = "Invalid input, use format Removepoint x,y\n";
            } 
            else {
                // extract the point 
                float x = std::stof(p.substr(0, find_com));

                float y = std::stof(p.substr(find_com + 1));

                shared_convex->remove_vx(x, y);

                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") removed\n";
            }

            /// unlock mutex
            pthread_mutex_unlock(&graph_mutex);
        }

        // calculate and print the convex hull area
        else if (cmd == "CH") {

            pthread_mutex_lock(&graph_mutex);
            if (!shared_convex) {
                response = "No graph exists, use Newgraph to create one \n";
            } 
            else 
            {
                
                shared_convex->findConvexHull_using_vector();
                auto hull = shared_convex->get_convex_vx();
                
                if (hull.size() < 3) {
                    response = "Convex hull cannot be formed with less than 3 points\n";
                } 

                try {
                    float area = shared_convex->calculate_area();

                    response = "Convex hull area: " + std::to_string(area) + "\n";
                } catch (const std::exception& ex) {
                    response = std::string("Error calculating area: ") + ex.what() + "\n";
                }
            }
            pthread_mutex_unlock(&graph_mutex);
        }
        // if command is unknown
        else {
            response = "Unknown command \n";
        }

        // Send the response to the client
        send(client_fd, response.c_str(), response.size(), 0);
    }
    // close the connection for this client
    close(client_fd);
}


/// @brief  handle the client commands using 
/// @param client_fd client file descriptor
/// @return nullptr
void* client_handler(int client_fd) {
    handle_client_commands(client_fd);
    return nullptr;
}

int main() {
    int server_fd;
    struct sockaddr_in serv_addr;

    // create the server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

     // reuse the socket immediately after shut down
    // mainly for testing
    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    // set the server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    // ipv4
    serv_addr.sin_family = AF_INET;

    /// accept client connections locally
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // set the port (beej port)
    serv_addr.sin_port = htons(PORT);

     // bind socket the ip and port
    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }


    // start listening to connections
    int opt = listen(server_fd, 10);
    if (opt < 0)
    {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("server listening on port %d\n", PORT);

    /// thread that handles the client connections
    pthread_t proactor_thread = start_proactor(server_fd, client_handler);

    // wait for pro_actor to finish
    pthread_join(proactor_thread, nullptr);

    ///destroy all the mutex and pthread conditions vars
    // before quitting
    pthread_mutex_destroy(&graph_mutex);

    //delelte the shared convex amongst all clients
    // we have to lock the process to avoid conflics
    pthread_mutex_lock(&graph_mutex);

    delete shared_convex;
    shared_convex = nullptr;

    // unlock mutex
    pthread_mutex_unlock(&graph_mutex);

    close(server_fd);
    return 0;
}