#pragma once
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <unordered_set>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unordered_map>

#define eventloop_init\
    eventmanager::getinstance()->eventloop();

#define epoll_mode_init\
    acceptor::get_instance();\
    eventmanager::get_instance();\
    acceptor::epollregister();\
    eventmanager::eventepoll();

class eventlable
{};

class event
{
    public:
        enum mode
        {
            ACCEPTOR=0,WAITER=1
        };
        static std::shared_ptr<event> create_event(int fd,mode mode);
        void setnonblocking(int evfd);
        void addtomanager();
        int getfd() const;
        epoll_event* get_epoll_event_addr() const;
        mode getmode() const;
    private:
        int fd;
        mode eventmode;
        std::shared_ptr<epoll_event> epoll_eventptr;
        event(int fd,mode mode);
        std::shared_ptr<event> evptr;
};

class eventmanager
{
    static std::shared_ptr<eventmanager> instance;
    static std::unordered_set<int> fds;
    static std::unordered_map<int,std::shared_ptr<event>> fdmap;
    eventmanager();
    static int epfd;
    public:
        static int eventloopaddfd(int fd);
        static int eventloopdelfd(int fd);
        static void epolladd(std::shared_ptr<event> evptr);
        static void epolldel(std::shared_ptr<event> evptr);
        static std::shared_ptr<eventmanager> get_instance();
        static void eventloop();
        static void eventepoll();
};

class eventprocess
{};