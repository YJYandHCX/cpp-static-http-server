#include "Timer_list.h"

bool timer::set_http(HTTP_R*& req){
    if (req==NULL) return false;
    request = req;
    return true;
}
timer::timer(HTTP_R* req,int delay){
    request = req;
    expire = delay+time(NULL);
}
void timer_list::tick(){
    time_t cur = time(NULL);
    cout<<"IIIII am working"<<endl;
    while(1){
        cout<<"I am worksing"<<endl;
        std::unique_lock<std::mutex> l(mu,try_to_lock);

        if (l){
            while(1){
                if (timer_delist.empty()) {
                    l.unlock();
                    return ;
                }
                if ((timer_delist.front()->get_http())==NULL) {
                    auto p = timer_delist.front();
                    timer_delist.pop_front();
                    delete p;
                    cout<<"Timer is OVER"<<endl;
                    continue;
                }
                if ((timer_delist.front()->get_expire())>cur)
                {
                    l.unlock();
                    break;
                }
                if ((timer_delist.front()->get_expire())<cur){
                    (timer_delist.front()->get_http())->close_t();
                    delete timer_delist.front();
                    timer_delist.pop_front();
                    cout<<"Timer is OVER"<<endl;
                }
            }
            break;
        }
    }
}

void timer_list::push(timer* t){
     while(1){

        std::unique_lock<std::mutex> l(mu,try_to_lock);
        if (l){
            timer_delist.push_back(t);
            l.unlock();
            return;
        }
    }
}
