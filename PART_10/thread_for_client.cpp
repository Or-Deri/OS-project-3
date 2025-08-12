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

// we need to track whether the are of the convex reaches >= 100
bool area_above_100 = false;

// shared convex to all clients connected
Convex *shared_convex = nullptr;

// mutex to synchronize the access to the clients shared convex
pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;

// mutex to protect the access when the area_above_100 bool turns true.
pthread_mutex_t ch_mutex = PTHREAD_MUTEX_INITIALIZER;

// a condition thread to signal when area_above_100 turns true/false
pthread_cond_t ch_cond = PTHREAD_COND_INITIALIZER;

void handle_client_commands(int client_fd)
{
    char buffer[BUFFER_SIZE];

    while (1)
    {
        // clear buffer
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0)
        {
            // if bytes ==0 then client has disconnected
            // else, there was an error connecting
            break;
        }

        // we need to parse the commands given by the clients
        std::string line(buffer);
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        // build the response to the client
        std::string response;

        // create the convex graph
        if (cmd == "Newgraph")
        {
            int n;
            ss >> n;

            if (n <= 0)
            {
                response = "Number of points must be positive \n";
                send(client_fd, response.c_str(), response.size(), 0);
                continue;
            }

            // lock the graph before modifying new graph
            pthread_mutex_lock(&graph_mutex);

            // delete the previous graph - free the memory
            delete shared_convex;

            // create a new graph
            shared_convex = new Convex(n);

            // send the clients a message to enter "n" points (from Newgraph (num of vxs))
            response = "Enter " + std::to_string(n) + " points x,y \n";
            send(client_fd, response.c_str(), response.size(), 0);

            // read the points
            for (int i = 0; i < n; ++i)
            {
                // clear buffer
                memset(buffer, 0, BUFFER_SIZE);
                int point_bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

                if (point_bytes <= 0)
                {
                    // if bytes ==0 then client has disconnected
                    // else, there was an error
                    break;
                }

                std::string ptline(buffer);
                size_t comma_pos = ptline.find(',');

                if (comma_pos == std::string::npos)
                {
                    std::string msg = "Invalid input use format x,y\n";
                    send(client_fd, msg.c_str(), msg.size(), 0);
                    --i;
                    continue;
                }

                float x = std::stof(ptline.substr(0, comma_pos));
                float y = std::stof(ptline.substr(comma_pos + 1));

                shared_convex->add_vx(x, y);
            }
            // unlock mutex after graph creation
            pthread_mutex_unlock(&graph_mutex);

            response = "Graph created\n";
        }
        //
        else if (cmd == "Newpoint")
        {
            std::string p;
            ss >> p;
            size_t comma_pos = p.find(',');

            // lock before adding a new point
            pthread_mutex_lock(&graph_mutex);

            // check if graph exists
            if (!shared_convex)
            {
                response = "No graph exists. Use Newgraph first\n";
            }
            else if (comma_pos == std::string::npos)
            {
                response = "Invalid input, use format Newpoint x,y\n";
            }
            else
            {
                float x = std::stof(p.substr(0, comma_pos));
                float y = std::stof(p.substr(comma_pos + 1));

                shared_convex->add_vx(x, y);
                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") added\n";
            }
            pthread_mutex_unlock(&graph_mutex);
        }
        else if (cmd == "Removepoint")
        {
            std::string p;

            ss >> p;

            size_t find_com = p.find(',');

            // lock before removing point to avoid conflicts
            pthread_mutex_lock(&graph_mutex);

            // check if graph exists
            if (!shared_convex)
            {
                response = "No graph exists. Use Newgraph first\n";
            }
            else if (find_com == std::string::npos) 
            {
                response = "Invalid input, use format Removepoint x,y\n";
            }
            else
            {
                // extract the point
                float x = std::stof(p.substr(0, find_com));
                float y = std::stof(p.substr(find_com + 1));

                // remove point (function in the convex class)
                shared_convex->remove_vx(x, y);

                response = "Point (" + std::to_string(x) + "," + std::to_string(y) + ") removed\n";
            }

            /// unlock mutex
            pthread_mutex_unlock(&graph_mutex);
        }
        else if (cmd == "CH") // calculate convex area
        {
            if (!shared_convex)
            {
                response = "No graph exists, use Newgraph to create one \n";
            }
            else
            {
                // use algo to calculate
                shared_convex->findConvexHull_using_vector();
                auto hull = shared_convex->get_convex_vx();

                // convex area cant be calculated using less than 3 vxs
                if (hull.size() < 3)
                {
                    response = "Convex hull cannot be formed with less than 3 points\n";
                    send(client_fd, response.c_str(), response.size(), 0);
                    continue;
                }

                try
                {
                    /// area calc
                    float area = shared_convex->calculate_area();

                    // lock mutex
                    pthread_mutex_lock(&ch_mutex);

                    // check if area is above 100 as well as the
                    // global bool variable to see if its changed since then
                    if (area >= 100 && !area_above_100)
                    {
                        // if the bool var was false we now set it to true
                        area_above_100 = true;

                        // we signal the thread that is waitng using
                        // pthread_cond_signal()
                        pthread_cond_signal(&ch_cond);
                    }
                    else if (area < 100 && area_above_100) // else, the bool var is on
                    // but the area is below a hundered
                    {
                        area_above_100 = false;
                        // we signal the thread that is waitng using
                        // pthread_cond_signal()
                        pthread_cond_signal(&ch_cond);
                    }
                    // unlock the mutex
                    pthread_mutex_unlock(&ch_mutex);

                    response = "Convex hull area: " + std::to_string(area) + "\n";
                }
                catch (const std::exception &ex)
                {
                    response = std::string("Error calculating area: ") + ex.what() + "\n";
                }
            }
        }
        else
        {
            response = "Unknown command \n";
        }

        /// send the response to the client
        send(client_fd, response.c_str(), response.size(), 0);
    }
    // close sock
    close(client_fd);
}

/// @brief  handle the client commands using
/// @param client_fd client file descriptor
/// @return nullptr
void *client_handler(int client_fd)
{
    handle_client_commands(client_fd);
    return nullptr;
}

/// @brief this function accounts for the waiting thread (area >= 100)
/// @param args
/// @return
void *waiting_ch_thread(void *args)
{
    // lock mutex
    pthread_mutex_lock(&ch_mutex);

    // save the current state of the global var using a new one
    bool saved_state = area_above_100;

    // while loop
    while (1)
    {
        // while
        while (area_above_100 == saved_state)
        {
            // we wait until area_above_100 changes value
            pthread_cond_wait(&ch_cond, &ch_mutex);
        }

        // save the state of the gloabl bool var
        saved_state = area_above_100;

        if (area_above_100)
        {
            std::cout << "At Least 100 units belongs to CH" << std::endl;
        }
        else
        {
            std::cout << "At Least 100 units no longer belongs to CH" << std::endl;
        }
    }

    // unlocked mutex
    pthread_mutex_unlock(&ch_mutex);
    return nullptr;
}

int main()
{
    // server sock file descriptor
    int server_fd;
    struct sockaddr_in serv_addr;

    // create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    // reuse the socket immediately after shut down
    // mainly for testing
    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    // set server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    // ipv4
    serv_addr.sin_family = AF_INET;

    // accept client connections locally
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // set port
    serv_addr.sin_port = htons(PORT);

    // bind socket the ip and port
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    // thread to check for changes in the calculations of the convex area
    // checking if the are is greater-eqaul-to or lower than 100
    pthread_t ch_waiting_thread;
    // create thread using pthread_create()
    pthread_create(&ch_waiting_thread, nullptr, waiting_ch_thread, nullptr);

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

    // wait for pro_actor thread to finish
    pthread_join(proactor_thread, nullptr);

    // wait for the convex hull (that checks for 100 >= or <= 100)
    // to finish
    pthread_join(ch_waiting_thread, nullptr);

    // delelte the shared convex amongst all clients
    //  we have to lock the process to avoid conflics
    pthread_mutex_lock(&graph_mutex);
    delete shared_convex;

    shared_convex = nullptr;

    // unlock mutex
    pthread_mutex_unlock(&graph_mutex);

    /// destroy all the mutex and pthread conditions vars
    // before quitting
    pthread_mutex_destroy(&graph_mutex);
    pthread_mutex_destroy(&ch_mutex);
    pthread_cond_destroy(&ch_cond);

    // close server sock
    close(server_fd);

    return 0;
}
