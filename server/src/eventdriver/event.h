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
    processmanager::spinprocess();

class eventlable
{};

class processor;  // 前向声明 processor 类，在event类中有引用
class event
{
    public:
        enum mode
        {
            ACCEPTOR=0,WAITER=1
        };
        static std::shared_ptr<event> create_event(int fd,mode mode);
        event(int fd);
        void process();
        virtual void create_processor()=0;
        void setnonblocking(int evfd);
        void addtomanager();
        int getfd() const;
        epoll_event* get_epoll_event_addr() const;
        mode getmode() const;
        void setevptr(std::shared_ptr<event> evptr);
    protected:
        int fd;
        mode eventmode;
        std::shared_ptr<epoll_event> epoll_eventptr;
        std::shared_ptr<event> evptr;
        std::shared_ptr<processor> processorptr;
};

class acceptor_event :public event
{
    public:
        acceptor_event(int fd);
        void create_processor();
};

class client_event :public event
{
    public:
        client_event(int fd);
        void create_processor();
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
        static std::shared_ptr<event> getevent(int fd);
        static std::vector<std::shared_ptr<event>> epollget();
};

class processmanager
{
    #define MAX_EVENTS 1024
    #define READ_BUF_SIZE 1024
    public:
        static void spinprocess();
        static void process();
        static void process(std::shared_ptr<event> eventptr);
    private:
        static std::vector<std::shared_ptr<event>> processing_events;
};

class processor
{
    protected:
        std::shared_ptr<char[]> bufferptr;
        std::shared_ptr<event> evptr;
    public:
        processor(std::shared_ptr<event> evptr);
        virtual void process_method()=0;
};

class accept_processor : public processor
{
    public:
        accept_processor(std::shared_ptr<event> evptr);
        void process_method();
};
class client_processor : public processor
{
    public:
        client_processor(std::shared_ptr<event> evptr);
        void process_method();
};