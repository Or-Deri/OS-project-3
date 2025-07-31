#include "Reactor.hpp"


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
