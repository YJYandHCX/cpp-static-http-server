#include "http_req.h"


HTTP_R::HTTP_R(){
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_checked_idx = 0;
    m_read_idx = 0;

    m_start_line = 0;
    connfd = -1;

    keep_alive = false;
    //my_t = NULL;
    memset(m_read_buf,'\0',READ_BUFFER_SIZE);
    memset(m_write_buf,'\0',WRITE_BUFFER_SIZE);
}
void HTTP_R::init(int sockfd, sockaddr_in client_addr){
    m_address = client_addr;
    my_t = NULL;

    m_sockfd = sockfd;

    m_check_state = CHECK_STATE_REQUESTLINE;
    m_checked_idx = 0;
    m_read_idx = 0;

    m_start_line = 0;
    connfd = sockfd;

    keep_alive = false;
    //my_t = NULL;
    memset(m_read_buf,'\0',READ_BUFFER_SIZE);
    memset(m_write_buf,'\0',WRITE_BUFFER_SIZE);
    addfd(epfd,sockfd,false);

}
HTTP_R::LINE_STATUS HTTP_R::parse_line(){
    char tmp;
    for (;m_checked_idx<m_read_idx;m_checked_idx++){

        tmp = m_read_buf[m_checked_idx];

        if (tmp == '/r'){
            if (m_checked_idx == m_read_idx-1){
                return LINE_OPEN;
            }

            else if(m_read_buf[m_checked_idx +1] == '\n'){
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';

                return LINE_OK;
            }
        }

        else if (tmp == '\n'){
            if ((m_checked_idx>1) && (m_read_buf[m_checked_idx-1] == '\r')){
                m_read_buf[m_checked_idx-1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';

                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}
bool HTTP_R::http_read(){

    if (!reset_timer()) return false;
    if(m_read_idx>READ_BUFFER_SIZE) return false;

    int bytes_read = 0;
    while(true){
        bytes_read = recv(m_sockfd,m_read_buf+m_read_idx,READ_BUFFER_SIZE-m_read_idx,0);

        //cout<<"wefewf"<<bytes_read<<endl;

        if (bytes_read == 0){
            if (errno == EAGAIN||errno == EWOULDBLOCK){
                break;
            }
            return false;
        }
        if (bytes_read == -1) return false;
        m_read_idx += bytes_read;

        //cout<<m_read_buf<<endl;
        if (bytes_read>0)
        {break;;}
    }
    return true;
}


HTTP_R::HTTP_CODE HTTP_R::parse_request_line(char* text){
    //cout<<text<<endl;
    m_url = strpbrk(text," \t");
    //cout<<m_url<<endl;
    if (!m_url){
        //cout<<"url"<<endl;
        return BAD_REQUEST;
    }
    *(m_url++) = '\0';
    char* method = text;
 
    if (strcasecmp(method, "GET") == 0){
        m_method = GET;
    }
    else {
        //cout<<"NO_GET"<<endl;
        return BAD_REQUEST;
    }
    m_url += strspn(m_url,"\t");

    if(strncasecmp(m_url,"http://",7) == 0){
        m_url+=7;
        m_url = strchr(m_url,'/');
    }

    if (!m_url || m_url[0]!='/') return BAD_REQUEST;

    m_check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTP_R::HTTP_CODE HTTP_R::parse_headers(char* text){
    if(text[0] == '\0'){
        if(m_contenxt_len !=0){
            m_check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }

    else if (strncasecmp(text,"Connection:",11) == 0){
        text += 11;
        text += strspn(text,"\t");
        if (strcasecmp(text,"keep-alive")==0){
            keep_alive = true;
			//设置为长连接
        }
    }
    else if(strncasecmp(text,"Content-Length",15)==0){
        text += 15;
        text += strspn(text,"\t");
        m_contenxt_len = atol(text);
    }

    else if(strncasecmp(text,"Host:",5)==0){
        text+=5;
        text+=strspn(text,"\t");
        m_host = text;
    }
    else{
        cout<<"OK"<<endl;
    }

    return NO_REQUEST;
}
HTTP_R::HTTP_CODE HTTP_R::parse_content(char* text){
    if (m_read_idx>= (m_contenxt_len+m_checked_idx)){
        text[m_contenxt_len] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}
HTTP_R::HTTP_CODE HTTP_R::process_read(){
    if (strstr(m_read_buf,"keep-alive")){
        keep_alive = true;

    }
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char* text = 0;
    //cout<<"ttt"<<endl;
    while(((m_check_state==CHECK_STATE_CONTENT) && (line_status==LINE_OK)) || ((line_status = parse_line())==LINE_OK))
    {
        text = get_line();
        //cout<<text<<endl;
        m_start_line = m_checked_idx;

        switch(m_check_state){
            case CHECK_STATE_REQUESTLINE:
            {
                ret = parse_request_line(text);
                if (ret == BAD_REQUEST){
                    return BAD_REQUEST;

                }
                break;
            }
            case CHECK_STATE_HEADER:
            {

                 ret = parse_headers(text);;
                 if (ret  == BAD_REQUEST){
                    return BAD_REQUEST;

                 }
                 else if(ret == GET_REQUEST){
                    return do_process();

                 }
                 break;
            }
            case CHECK_STATE_CONTENT:
            {
                ret = parse_content(text);
                if (ret == GET_REQUEST)
                {
                    return do_process();

                }
                line_status = LINE_OPEN;
                break;
            }
            default:{
                return INTERNAL_ERROR;
            }
        }
    }
    return NO_REQUEST;
}
HTTP_R::HTTP_CODE HTTP_R::do_process(){
    if (strstr(m_read_buf,"keep-alive")){
        //cout<<"ALIVE ALIVE"<<endl;
        keep_alive = true;

    }
    if (!keep_alive){
        char m_write_buf[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n";
        int s = send(m_sockfd,m_write_buf,strlen(m_write_buf),0);

    }
    else {
        char m_write_buf[WRITE_BUFFER_SIZE] = "HTTP/1.1 200 ok\r\nconnection:keep-alive\r\n\r\n";
        int s = send(m_sockfd,m_write_buf,strlen(m_write_buf),0);
    }
    int fd = open("1.html",O_RDONLY);
    sendfile(m_sockfd,fd,NULL,WRITE_BUFFER_SIZE);
    close(fd);
    usleep(100);
    if (!keep_alive)
        {
        close_t();}
    else{
        set_timer();
    }

    return NO_REQUEST;
}
bool HTTP_R::close_t(){
    if (my_t!=NULL){my_t=NULL;}
    my_t = NULL;

    removefd(this->epfd,m_sockfd);


    return true;
}
int HTTP_R::epfd = 0;
bool HTTP_R::reset_timer(){
    if (my_t==NULL) return true;
    if (my_t!=NULL){
        my_t->set_http_n();
        my_t = NULL;
        return true;
    }
    return false;
};
bool HTTP_R::set_timer(){
    if (my_t != NULL){
        my_t->set_http_n();
        my_t = NULL;
    }
    my_t = new timer(this,DELAY_TIME);
    ti_l.push(my_t);
    return true;
}
