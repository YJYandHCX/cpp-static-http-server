#include "epoll.h"


int setnonblocking( int fd ){
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epfd, int fd, bool one_shot){
    epoll_event event;
    event.data.fd = fd;
    //event.events = EPOLLIN|EPOLLRDHUP;
    event.events = EPOLLIN|EPOLLET|EPOLLRDHUP|EPOLLOUT;

    if (one_shot){event.events |= EPOLLONESHOT;}

    epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

int _create_socket(){
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd>=0);

    struct linger tmp = {1,0};
    setsockopt( listenfd, SOL_SOCKET,SO_LINGER,&tmp, sizeof(tmp));

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret >= 0);

    ret = listen(listenfd, MAX_CONNECT);
    assert(ret >= 0);

    return listenfd;
}
int _create_epollfd(int listenfd){
    //epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(MAX_EVENT_NUMBER);
    assert(epollfd!=-1);
    addfd(epollfd,listenfd,false);
    return epollfd;
}
void removefd(int& epfd,int& fd){
    epoll_ctl(epfd, EPOLL_CTL_DEL,fd,0);
    close(fd);
}
void modfd(int epfd, int fd){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&event);
}
