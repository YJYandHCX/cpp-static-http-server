#pragma once

#include <iostream>
#include <assert.h>
#include <errno.h>
#include <memory>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <time.h>
#include <list>
#define ip "127.0.0.1"
#define port 12345
#define MAX_EVENT_NUMBER 1024
#define MAX_FD 63525
#define MAX_CONNECT 2500

#define DELAY_TIME 10
#include "http_req.h"
using namespace std;

class HTTP_R;
/*这里定时器，想法是每个定时器有一个指向任务的指针，每个任务有一个指向定时器的指针。
在任务要重新设置定时器时候，就把原来定时器的任务指针指空，把原来的定时器指针指向
另外一个新的timer。在清理定时器的时候，没有任务指针的定时器直接弄吊就好了*/

class timer{
public:

    HTTP_R* get_http() {return request;};
    bool set_http(HTTP_R*& req);
    bool set_http_n() {request=NULL;return true;};
    timer(HTTP_R* req,int delay);

    time_t get_expire() {return expire;};
private:
    HTTP_R* request;

    time_t expire;

};
class timer_list{
public:
    void push(timer* t);
    void tick(); //心搏函数清理超时连接
private:
    std::mutex mu;
    std::list<timer*> timer_delist;
};
