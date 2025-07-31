#include "Reactor.hpp"
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <vector>

Pro_actor global_pro_actor;

/// @brief crete a new pointer reactor obeject
/// and we have to make sure we activate it
/// we can do this easily by a bool var that
/// is valued 0 1 and check every time if the reactor
/// is active or not.
/// @return the reactor pointer
Reactor *start_reactor()
{
    Reactor *rec = new Reactor();
    rec->is_active = 1;
    return rec;
}

/// @brief add file descriptors with its handler
/// @param reactor object
/// @param fd file descriptor
/// @param func a funtion callback in order to invoke an activity
/// @return 0 for success, otherwise -1.
int add_fd_to_reactor(Reactor *reactor, int fd, reactorFunc func)
{
    if (!reactor || fd < 0 || !func) 
    {
        return -1;
    }
    
    // lock using mutex before modifiying the reactor
    std::lock_guard<std::mutex> lock(reactor->mtx);

    // store handler
    reactor->file_des[fd] = func;
    return 0;
}


/// @brief remove file descriptor from reactor
/// @param reactor object
/// @param fd file descriptor to remove
/// @return 0 for success, otherwise -1.
int remove_fd_from_reactor(Reactor *reactor, int fd)
{
    if (!reactor || fd < 0) 
    {
        return -1;
    }

    // lock fd to avoid conflics
    std::lock_guard<std::mutex> lock(reactor->mtx);

    // remove the file descriPTOR from the map
    reactor->file_des.erase(fd);
    return 0;
}

/// @brief stop the reactor
/// @param reactor object
/// @return 1 success -1 otherwise
int stop_reactor(Reactor *reactor)
{
    if (reactor)
    {
        // we make sure its inactive
        reactor->is_active = 0;
        return 1;
    }
    return -1;
}

/// @brief run the reactor with its main loop
/// @param reac reactor object
void run_reactor(Reactor *reac)
{
    // if reactor isnt active
    if (!reac)
        return;

    // otherwise, enter loop
    while (reac->is_active)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        int max_fd = -1;

        {
            // lock map before checking 
            std::lock_guard<std::mutex> lock(reac->mtx);
            if (reac->file_des.empty()) 
            {
                break;
            }

            // set all the file descriptors in the 
            for (const auto &p : reac->file_des)
            {
                FD_SET(p.first, &readfds);
                if (p.first > max_fd)
                    max_fd = p.first;
            }
        }

        // if no valid fds,
        // we sleep and then continue
        if (max_fd < 0)
        {

            usleep(10000);
            continue;
        }

        /// timeout for select
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // 
        int ret = select(max_fd + 1, &readfds, nullptr, nullptr, &tv);
        if (ret < 0)
        {
            perror("select");
            break;
        }

        // reached timeout
        if (ret == 0)
            continue; 

        
        // vector that consists of all the fds the are ready
        std::vector<std::pair<int, reactorFunc>> ready_fds;
        {
            // lock map
            std::lock_guard<std::mutex> lock(reac->mtx);
            for (const auto &p : reac->file_des)
            {
                // check if the fd is ready
                if (FD_ISSET(p.first, &readfds))
                {
                    // store it in the vector (ready_fds) 
                    ready_fds.push_back(p);
                }
            }
        }

        // call the function callbacks for each ready
        // fds
        for (const auto &p : ready_fds)
        {
            if (p.second)
            // the function callbacks are stored within 
            // the map, we just call them with the df
                p.second(p.first);
        }
    }    
}

void *run_threads(void *arg)
{
    handle_thread *thread_arguments = (handle_thread *)arg;
    if (thread_arguments->pro_func)
    {
        thread_arguments->pro_func(thread_arguments->client_socket);
    }
    close(thread_arguments->client_socket);
    delete thread_arguments;
    return nullptr;
}

// new thread for the client that has been added using accept
// thread_id + client_socket
void *accept_thread_arguments(void *arg)
{
    accept_args *args = (accept_args *)arg;
    int sockfd = args->sockfd;
    proactorFunc func = args->func;
    delete args;

    while (1)
    {
        int clint_sock = accept(sockfd, nullptr, nullptr);
        if (clint_sock == -1)
        {
            perror("failed to accept");

            // dont stop for other threads
            continue;
        }

        handle_thread *thread_data = new handle_thread{clint_sock, func};
        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, run_threads, thread_data);

        add_to_global_pro_actor(thread_id, clint_sock);
    }
    return nullptr;
}

// note - those are different threads with different sockets
// and they have to be saved seperately.

// start the proactor
// thread is listening to the accept (main loop)
// thread_id + sockfd
pthread_t start_proactor(int sockfd, proactorFunc threadFunc)
{
    accept_args *args = new accept_args{sockfd, threadFunc};
    pthread_t tid;
    int ret = pthread_create(&tid, nullptr, accept_thread_arguments, args);
    if (ret != 0) 
    {
        perror("failed creating a thread");
        delete args;
        return 0;  
    }
    add_to_global_pro_actor(tid, sockfd);

    return tid;
}

// stop the proactor
int stop_proactor(pthread_t thread_id)
{
    std::lock_guard<std::mutex> lock(global_pro_actor.mtx);

    auto it = global_pro_actor.thrds.find(thread_id);
    if (it == global_pro_actor.thrds.end())
    {
        perror("thread id not found");
        return -1;
    }

    pthread_cancel(thread_id);
    pthread_join(thread_id, nullptr);

    close(it->second);

    global_pro_actor.thrds.erase(it);

    if (global_pro_actor.thrds.empty())
    {
        global_pro_actor.is_active = false;
    }

    return 1;
}

void add_to_global_pro_actor(pthread_t thread_id, int client_sk)
{
    std::lock_guard<std::mutex> lock(global_pro_actor.mtx);
    global_pro_actor.thrds[thread_id] = client_sk;
    global_pro_actor.is_active = 1;
}
