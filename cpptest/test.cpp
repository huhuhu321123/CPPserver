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
using namespace std;
void change(int* x) {
    *x=100;
}
int main()
{
    unordered_set<shared_ptr<int>> m;
    shared_ptr<int>* tracker;
    {
        shared_ptr<int>b=shared_ptr<int>(new int);
        *b=2;
        tracker=&b;
        m.emplace(b);
        m.erase(b);
    }
    cout<<**tracker<<endl;
    cout<<**tracker<<endl;
    cout<<endl;
    shared_ptr<int> aaa=shared_ptr<int>(new int(1));
    change(aaa.get());
    cout<<*aaa<<endl;
    return 0;
}