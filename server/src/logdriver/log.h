#pragma once
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <string>
#include <iostream>
using namespace std;
class loggermanager
{
    public:
        static void loggeradd(int retval,string op);
};
class logger
{};
class loggerinfo
{};