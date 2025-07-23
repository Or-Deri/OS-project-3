#pragma once
#include <vector>
#include <mutex>
#include <map>
#include <thread>
#include <pthread.h>

typedef void (*reactorFunc)(int fd);
typedef void (*proactorFunc)(int sockfd);

struct Reactor {
    std::map<int, reactorFunc> file_des;
    std::mutex mtx;
    int is_active = 0;
};

struct Pro_actor {
    std::mutex mtx;
    std::map<pthread_t, int> thrds;
    bool is_active = 0;
};

struct handle_thread {
    int client_socket;
    proactorFunc pro_func;
};

struct accept_args {
    int sockfd;
    proactorFunc func;
};

Reactor* start_reactor();
int add_fd_to_reactor(Reactor* reactor, int fd, reactorFunc func);
int remove_fd_from_reactor(Reactor* reactor, int fd);
int stop_reactor(Reactor* reactor);
void run_reactor(Reactor* reac);
pthread_t start_proactor(int sockfd, proactorFunc threadFunc);
int stop_proactor(pthread_t thread_id);
void add_to_global_pro_actor(pthread_t thread_id, int client_sk);
