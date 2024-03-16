#include "event.h"
#include "../acceptordriver/acceptor.h"
#include "../logdriver/log.h"
#include "../utils/utils.h"
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
void eventmanager::eventloop()
{
    char buf[1024];
    while (!fds.empty()){
        memset(buf,0,sizeof(buf));
        for (auto fd:fds) {
            ssize_t read_bytes = read(fd, buf, sizeof(buf));
            if (read_bytes>0) {
                printf("message from fd %d: %s\n", fd, buf);
                write(fd,buf,sizeof(buf));
            } else if (read_bytes==0) {
                printf("fd %d disconnected\n",fd);
                eventloopdelfd(fd);
                close(fd);
                break; //you have to break here because the container fds has changed
            } else {
                close(fd);
                eventloopdelfd(fd);
                printf("fd %d readerror\n",fd);
                break;
            }
        }
    }
    return;
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
event::event(int fd,mode mode):fd(fd),eventmode(mode) {
    epoll_eventptr=shared_ptr<epoll_event>(new epoll_event);
    epoll_eventptr->data.fd = fd;
    epoll_eventptr->events = EPOLLIN | EPOLLET;
    setnonblocking(fd);
}
std::shared_ptr<event> event::create_event(int fd,mode mode)
{
    shared_ptr<event> newevptr=shared_ptr<event>(new event(fd,mode));
    return newevptr->evptr=newevptr;
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