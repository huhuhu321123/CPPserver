#include "threads.h"
#include "../acceptordriver/acceptor.h"
#include "../eventdriver/event.h"
#include "../logdriver/log.h"
#include "../utils/utils.h"
/**
 * @brief
 * class threadsmanager:
*/
std::mutex threadsmanager::threadmtx;
bool threadsmanager::working;
std::condition_variable threadsmanager::cv;
std::vector<std::thread> threadsmanager::threadpool;
std::queue<std::function<void()>> threadsmanager::tasksqueue;
std::shared_ptr<threadsmanager> threadsmanager::myptr;
std::shared_ptr<threadsmanager> threadsmanager::getinstance()
{
    if (threadsmanager::myptr==nullptr) {
        myptr=std::shared_ptr<threadsmanager>(new threadsmanager);
    }
    return myptr;
}
threadsmanager::threadsmanager()
{
    working=true;
    for (int i=0;i<threadspoolsize;i++) {
        threadpool.emplace_back(std::thread(threadmethod));
    }
}
void threadsmanager::threadmethod()
{
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lck(threadmtx);
            cv.wait(lck,taskavailable);
            if ((!working)&&(tasksqueue.empty())) return;
            task=tasksqueue.front();
            tasksqueue.pop();
        }
        task();
    }
}
void threadsmanager::init()
{
    std::cout<<"thread manager initialized!"<<std::endl;
}
bool threadsmanager::taskavailable()
{
    return (!tasksqueue.empty())||(!working);
}
void threadsmanager::addtask(std::function<void()> task)
{
    tasksqueue.push(task);
    cv.notify_one();
    //here!!! if dont cout, there would be a segmentation fault???
    // std::cout<<"addtask done!"<<std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(1));
}
threadsmanager::~threadsmanager()
{
    working=false;
    cv.notify_all();
    for (int i=0;i<threadspoolsize;i++) {
        threadpool[i].join();
    }
}