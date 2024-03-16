#pragma once
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <memory>
using namespace std;
shared_ptr<sockaddr> setsockaddr();