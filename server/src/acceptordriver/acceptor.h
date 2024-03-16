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
#define acceptor_block_init\
    acceptor::get_instance()->accept_new_client();

/**
 * @brief
 * lazy person singleton
*/
class acceptor
{
        static int acceptorfd;
        static std::shared_ptr<acceptor> instance;
        acceptor();
    public:
        static int accept_new_client();
        static void epollregister();
        static std::shared_ptr<acceptor> get_instance();
        static std::shared_ptr<acceptor> del_instance();
};