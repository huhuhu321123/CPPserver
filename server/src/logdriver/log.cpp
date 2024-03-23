#include "log.h"
#include "../threads/threads.h"
void loggermanager::loggeradd(int retval,string op)
{
    cout<<op<<";"<<" result: "<<retval<<endl;
}