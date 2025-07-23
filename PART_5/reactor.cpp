#include "Reactor.hpp"

Reactor *start_reactor()
{
    Reactor *rec = new Reactor();
    rec->is_active = 1;
    return rec;
}

int add_fd_to_reactor(Reactor *reactor, int fd, reactorFunc func)
{
    if (!reactor || fd < 0 || !func) 
    {
        return -1;
    }
    std::lock_guard<std::mutex> lock(reactor->mtx);

    reactor->file_des[fd] = func;
    return 0;
}

int remove_fd_from_reactor(Reactor *reactor, int fd)
{
    if (!reactor || fd < 0)
        return -1;
    std::lock_guard<std::mutex> lock(reactor->mtx);
    reactor->file_des.erase(fd);
    return 0;
}

int stop_reactor(Reactor *reactor)
{
    if (reactor)
    {
        reactor->is_active = 0;
        return 0;
    }
    return -1;
}

void run_reactor(Reactor *reac)
{
    if (!reac)
        return;

    while (reac->is_active)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        int max_fd = -1;

        {
            std::lock_guard<std::mutex> lock(reac->mtx);
            if (reac->file_des.empty())
                break;

            for (const auto &p : reac->file_des)
            {
                FD_SET(p.first, &readfds);
                if (p.first > max_fd)
                    max_fd = p.first;
            }
        }

        if (max_fd < 0)
        {
            usleep(10000);
            continue;
        }

        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int ret = select(max_fd + 1, &readfds, nullptr, nullptr, &tv);
        if (ret < 0)
        {
            perror("select");
            break;
        }
        if (ret == 0)
            continue; // timeout

        std::vector<std::pair<int, reactorFunc>> ready;
        {
            std::lock_guard<std::mutex> lock(reac->mtx);
            for (const auto &p : reac->file_des)
            {
                if (FD_ISSET(p.first, &readfds))
                {
                    ready.push_back(p);
                }
            }
        }

        for (const auto &p : ready)
        {
            if (p.second)
                p.second(p.first);
        }
    }    
}
