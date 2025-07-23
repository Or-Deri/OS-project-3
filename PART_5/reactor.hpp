#pragma once
#include <vector>
#include <mutex>
#include <map>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <vector>

typedef void (*reactorFunc)(int fd);

struct Reactor {
    std::map<int, reactorFunc> file_des;
    std::mutex mtx;

    int is_active = 0; // disabled by default;
};

Reactor* start_reactor();


int add_fd_to_reactor(Reactor* reactor, int fd, reactorFunc func);


int remove_fd_from_reactor(Reactor* reactor, int fd);


int stop_reactor(Reactor* reactor);

void run_reactor(Reactor* reac);
