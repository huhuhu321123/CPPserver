#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <bits/stdc++.h>
#include <chrono>
#define threadspoolsize 10
class threadsmanager
{
    private:
        static std::mutex threadmtx;
        static std::condition_variable cv;
        static std::shared_ptr<threadsmanager> myptr;
        threadsmanager();
        static std::vector<std::thread> threadpool;
        static void threadmethod();
        static std::queue<std::function<void()>> tasksqueue;
        static bool taskavailable();
        static bool working;
    public:
        ~threadsmanager();
        static std::shared_ptr<threadsmanager> getinstance();
        static void init();
        static void addtask(std::function<void()> task);
};