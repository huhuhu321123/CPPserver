#include "utils.h"
using namespace std;
shared_ptr<sockaddr> setsockaddr()
{
    sockaddr_in* serv_addr=new sockaddr_in();
    memset(serv_addr,0,sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr->sin_port = htons(8888);
    return shared_ptr<sockaddr>((sockaddr*)serv_addr);
}