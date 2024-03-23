#include "event.h"
#include "../acceptordriver/acceptor.h"
#include "../logdriver/log.h"
#include "../utils/utils.h"
#include "../threads/threads.h"
/**
 * @brief
 *  event manager
*/
std::shared_ptr<eventmanager> eventmanager::instance=nullptr;
std::unordered_set<int> eventmanager::fds;
std::unordered_map<int,std::shared_ptr<event>> eventmanager::fdmap;
int eventmanager::epfd;

eventmanager::eventmanager()
{
    epfd=epoll_create1(0);
    loggermanager::loggeradd(epfd,"create epoll tree");
}
shared_ptr<eventmanager> eventmanager::get_instance()
{
    if (instance==nullptr) {
        instance=shared_ptr<eventmanager>(new eventmanager());
    }
    return instance;
}
int eventmanager::eventloopaddfd(int fd)
{
    fds.emplace(fd);
    return 0;
}
int eventmanager::eventloopdelfd(int fd)
{
    fds.erase(fd);
    return 0;
}
std::vector<std::shared_ptr<event>> eventmanager::epollget()
{
    vector<std::shared_ptr<event>> ret;
    epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for (int i=0;i<nfds;i++) {
        ret.push_back(fdmap[events[i].data.fd]);
    }
    return ret;
}
void eventmanager::epolladd(shared_ptr<event> evptr)
{
    fdmap.emplace(evptr->getfd(),evptr);
    epoll_ctl(epfd, EPOLL_CTL_ADD, evptr->getfd(), evptr->get_epoll_event_addr());
}

void eventmanager::epolldel(shared_ptr<event> evptr)
{
    close(evptr->get_epoll_event_addr()->data.fd);
    //actually you can swap the "close" and "erase"
    //because this function is still a valid scope for evptr
    //but once exited this function, the evptr will be freed automatically
    fdmap.erase(evptr->getfd());
}
std::shared_ptr<event> eventmanager::getevent(int fd)
{
    return fdmap[fd];
}

void eventmanager::eventepoll() 
{
    #define MAX_EVENTS 1024
    #define READ_BUF_SIZE 1024
    epoll_event events[MAX_EVENTS];
    loggermanager::loggeradd(fdmap.size(),"fdmap size");
    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        // loggermanager::loggeradd(nfds,"epoll wait");
        for (int i=0;i<nfds;i++) {
            event::mode thismode=fdmap[events[i].data.fd]->getmode();
            switch (thismode) {
                case event::ACCEPTOR:
                    {
                        loggermanager::loggeradd(1,"ACCEPTOR EVENT");
                        int clntfd=acceptor::accept_new_client();
                        event::create_event(clntfd,event::WAITER)->addtomanager();
                        break;
                    }
                case event::WAITER:
                    {
                        loggermanager::loggeradd(1,"CLIENT EVENT");
                        char buf[READ_BUF_SIZE];
                        while(true){    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
                            bzero(&buf, sizeof(buf));
                            ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
                            if(bytes_read > 0){
                                printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                                write(events[i].data.fd, buf, sizeof(buf));
                            } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
                                printf("continue reading");
                                continue;
                            } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
                                printf("finish reading once, errno: %d\n", errno);
                                break;
                            } else if(bytes_read == 0){  //EOF，客户端断开连接
                                printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                                eventmanager::epolldel(fdmap[events[i].data.fd]);//关闭socket会自动将文件描述符从epoll树上移除
                                break;
                            }
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    }
}

/**
 * @brief
 * event class
*/
event::event(int fd):fd(fd)
{
    epoll_eventptr=shared_ptr<epoll_event>(new epoll_event);
    epoll_eventptr->data.fd = fd;
    epoll_eventptr->events = EPOLLIN | EPOLLET;
    setnonblocking(fd);
}
std::shared_ptr<event> event::create_event(int fd,mode mode)
{
    shared_ptr<event> newevptr;
    if (mode==ACCEPTOR) newevptr=std::make_shared<acceptor_event>(fd);
    else if (mode==WAITER) newevptr=std::make_shared<client_event>(fd);
    newevptr->setevptr(newevptr);
    newevptr->addtomanager();
    newevptr->create_processor();
    return newevptr;
}
void event::setevptr(std::shared_ptr<event> evptr)
{
    this->evptr=evptr;
}
void event::process()
{
    this->processorptr->process_method();
}
void event::setnonblocking(int fd){
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}
int event::getfd() const{
    return fd;
}
epoll_event* event::get_epoll_event_addr() const{
    return epoll_eventptr.get();
}
event::mode event::getmode() const
{
    return this->eventmode;
}
void event::addtomanager()
{
    eventmanager::epolladd(this->evptr);
}

/**
 * @brief
 * acceptor event class
*/
acceptor_event::acceptor_event(int fd):event(fd)
{}
void acceptor_event::create_processor()
{
    this->processorptr=std::shared_ptr<accept_processor>(new accept_processor(this->evptr));
}
/**
 * @brief
 * client event class
*/
client_event::client_event(int fd):event(fd)
{}
void client_event::create_processor()
{
    this->processorptr=std::shared_ptr<client_processor>(new client_processor(this->evptr));
}
/**
 * @brief
 * processor
*/
processor::processor(std::shared_ptr<event> evptr)
{
    this->evptr=evptr;
    bufferptr=shared_ptr<char[]>(new char[READ_BUF_SIZE]);
}
/**
 * @brief
 * acceptor processor
*/
accept_processor::accept_processor(std::shared_ptr<event> evptr):processor(evptr)
{}
void accept_processor::process_method()
{
    event::create_event(acceptor::accept_new_client(),event::WAITER);
}
/**
 * @brief
 * client processor
*/
client_processor::client_processor(std::shared_ptr<event> evptr):processor(evptr)
{}
void client_processor::process_method()
{
    memset(this->bufferptr.get(),0,sizeof(*bufferptr.get()));
    ssize_t bytes_read = read(evptr->getfd(), bufferptr.get(), sizeof(char)*READ_BUF_SIZE);
    if(bytes_read > 0){
        printf("message from client fd %d: %s\n", evptr->getfd(), bufferptr.get());
        write(evptr->getfd(), bufferptr.get(), sizeof(char)*READ_BUF_SIZE);
    } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
        printf("continue reading");
    } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
        printf("finish reading once, errno: %d\n", errno);
    } else if(bytes_read == 0){  //EOF，客户端断开连接
        printf("EOF, client fd %d disconnected\n", evptr->getfd());
        eventmanager::epolldel(evptr);//关闭socket会自动将文件描述符从epoll树上移除
    }
}
/**
 * @brief
 * processmanager
*/
std::vector<std::shared_ptr<event>> processmanager::processing_events;
void processmanager::spinprocess()
{
    while (true) {
        process();
    }
}
void processmanager::process()
{
    processing_events.clear();
    processing_events=eventmanager::epollget();
    for (auto curevent:processing_events) {
        threadsmanager::addtask(std::bind(taskprocess,curevent));
    }
}
void processmanager::taskprocess(std::shared_ptr<event> curevent)
{
    // std::cout<<"there is a new task! "<<curevent->getfd()<<endl;
    curevent->process();
}