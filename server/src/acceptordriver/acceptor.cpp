#include "acceptor.h"
#include "../eventdriver/event.h"
#include "../logdriver/log.h"
#include "../utils/utils.h"
#include "../threads/threads.h"
/**
 * @brief
 * 1. acceptor static member initialize
 * 2. acceptor member function define
*/
int acceptor::acceptorfd=-1;
std::shared_ptr<acceptor> acceptor::instance=nullptr;
acceptor::acceptor()
{
    /**
     * @brief
     * definition of the LOG statement
    */
    #define socketing "server_acceptor_creating_socket"
    #define binding "server_acceptor_binding(sockfd,addr)"
    #define listening "server_acceptor_set_to_listen"
    #define log(ret,op)\
        loggermanager::loggeradd(ret,op);
    
    shared_ptr<sockaddr> serv_addr=setsockaddr();
    log(acceptorfd = socket(AF_INET, SOCK_STREAM, 0),socketing)
    log(bind(acceptorfd, serv_addr.get(), sizeof(*serv_addr)),binding)
    log(listen(acceptorfd,SOMAXCONN),listening)
}
int acceptor::accept_new_client()
{
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));
    int clnt_sockfd = accept(acceptorfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
    loggermanager::loggeradd(clnt_sockfd,"accept new client");
    printf("new client fd %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
    eventmanager::eventloopaddfd(clnt_sockfd);
    return clnt_sockfd;
}

void acceptor::epollregister()
{
    event::create_event(acceptorfd,event::ACCEPTOR);
}

std::shared_ptr<acceptor> acceptor::get_instance()
{
    if (instance==nullptr) instance=std::shared_ptr<acceptor>(new acceptor());
    return instance;
}