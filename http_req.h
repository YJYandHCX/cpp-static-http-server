#pragma once

#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<errno.h>
#include<map>
#include<mutex>
#include <iostream>
#include <string>
#include<fcntl.h>
#include<sys/sendfile.h>

#include "epoll.h"
#include "Timer_list.h"
using namespace std;

class timer;
class timer_list;
extern timer_list ti_l;

class HTTP_R{
public:

    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    static const int FILENAME_LEN = 1024;

    int m_contenxt_len;


    enum METHOD{GET=0,POST,HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT,PATCH};
    enum CHECK_STATE {CHECK_STATE_REQUESTLINE = 0,
                      CHECK_STATE_HEADER,
                      CHECK_STATE_CONTENT
    };
    enum HTTP_CODE {NO_REQUEST,GET_REQUEST,BAD_REQUEST,NO_RESOURCE,FORBIDDEN_REQUEST,FILE_REQUEST,INTERNAL_ERROR,CLOSED_CONNECTION};

    enum LINE_STATUS {LINE_OK=0, LINE_BAD,LINE_OPEN};



    //static int m_epollfd;
    //static int m_users_count;




    HTTP_CODE process_read();

    LINE_STATUS parse_line();



    //static set_epollfd(const int& epollfd){m_epollfd = epollfd;}


    HTTP_R();
    bool http_read();
    //bool http_read(int sockfd);
    HTTP_CODE parse_headers(char* text);

    HTTP_CODE parse_request_line(char* text);

    HTTP_CODE parse_content(char* text);
   // HTTP_CODE process_read();

    int m_start_line;


    char* get_line(){return m_read_buf+m_start_line;}
    HTTP_CODE do_process();

    bool is_long_connect(){
        return keep_alive;
    }

    bool close_t();
    //static const map<int,std::string> sta2n{
    //  {200,"OK"},
    //  {400,"Bad Request"},
    //  {403,"Forbidden"},
    //  {404,"NO Found"},
    //  {500,"Internal Error"}
    //};
    int get_connfd(){return connfd;};
    bool is_long_live() {return keep_alive;};

    int connfd;

    void init(int socked, sockaddr_in client_addr);
    //void setTimer();


    static void set_epfd(int ep){HTTP_R::epfd = ep;}

    bool reset_timer();
    bool set_timer();
    timer* get_timer() {return my_t;};
    void show_read_buf(){
        cout<<m_read_buf<<endl;
    };
    int get_sockfd(){return m_sockfd;};
private:
    static int epfd;
    int m_sockfd;
    sockaddr_in m_address;


    std::mutex mu;

    timer* my_t;

    bool keep_alive;
    CHECK_STATE m_check_state;
    char* m_url;
    METHOD m_method;

    //char* m_version;

    char m_write_buf[WRITE_BUFFER_SIZE];
    char m_real_file[FILENAME_LEN];
    int m_checked_idx;
    int m_read_idx;
    char m_read_buf[READ_BUFFER_SIZE];
    char* m_host;
};
