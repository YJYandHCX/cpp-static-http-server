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

#define ip "127.0.0.1"
#define port 12345
#define MAX_EVENT_NUMBER 1024
#define MAX_FD 63525
#define MAX_CONNECT 2500

using namespace std;


int setnonblocking( int fd );
void addfd(int epfd, int fd, bool one_shot);
void removefd(int& epfd,int& fd);
void modfd(int epfd, int fd);


int _create_socket();
int _create_epollfd(int listenfd);
