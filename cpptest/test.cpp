#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <unordered_set>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
using namespace std;
std::mutex mtx;
void task(int n,char k)
{
    std::unique_lock<std::mutex> lck(mtx);
    cout<<k<<" starts!"<<endl;
    for (int i=0;i<n;i++) {
        cout<<k;
    }
    cout<<endl;
}
int main()
{
    std::vector<std::thread> t;
    for (int i=0;i<2;i++) {
        t.emplace_back(std::thread(task,10000,'('+i));
    }
    for (int i=0;i<2;i++) {
        t[i].join();
    }
    return 0;
}