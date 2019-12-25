#include <iostream>
#include <assert.h>
#include <memory>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/sendfile.h>
#include "epoll.h"

#include "http_req.h"

#include "Timer_list.h"
#include "threadpool.h"

#define MAX_FD 63525
using namespace std;

timer_list ti_l; //因为要线程里边加定时器，所以将定时器设置成了全局变量。

//static const char* szret[] = {"I get a correct result\n", "Something wrong\n"};
int main()
{
    int listenfd = _create_socket();
    int epollfd = _create_epollfd(listenfd);
    //std::vector<epoll_event> ep_events(MAX_FD);
    epoll_event ep_events[MAX_FD];

    HTTP_R::set_epfd(epollfd);
    HTTP_R* clients = new HTTP_R[MAX_FD];

    ThreadPool thr_p(4,1000);//线程池中有4个线程，最多任务队列中能有1000个任务
    while(1){
        int number = epoll_wait(epollfd,ep_events,MAX_EVENT_NUMBER,-1);

        if (number<0 && errno!=EINTR){
            cout<<"eopll error"<<endl;
            break;
        }

        for (int i=0;i<number;i++){
            int sockfd = ep_events[i].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addressLength = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addressLength);

                if (connfd<0){continue;}
                clients[connfd].init(connfd,client_address);
            }
            else if (ep_events[i].events&EPOLLIN){
                if (clients[sockfd].http_read()){
                    while(!thr_p.addTask(clients+sockfd));//加入线程池的任务队列
                }
                else{
                    clients[sockfd].close_t();
                }
            }
        }

        ti_l.tick();//清理超时的连接
    }
    return 0;
}
