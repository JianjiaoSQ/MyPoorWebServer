// #include "time_wheel_timer.h"
#include "../http/http_conn.h"


time_wheel::time_wheel() : cur_slot(0) {
    for(int i = 0; i < N; ++i) {
        slots[i] = NULL; // 初始化每个槽的头节点
    }
}
time_wheel::~time_wheel() {
    // 遍历每一个槽，并销毁其中的定时器
    for(int i = 0; i < N; ++i) {
        tw_timer* tmp = slots[i];
        while(tmp) {
            slots[i] = tmp->next;
            delete tmp;
            tmp = slots[i];
        }
    }
}

    // 根据定时值 timeout 创建一个定时器，并把它插入合适的槽中
void time_wheel::add_timer(tw_timer* timer, int timesolt) {
    if (timesolt < 0) {
        return;
    }

    int ticks = 0;
    /* 根据插入定时器的超时值计算它将在时间轮转动多少个滴答后被触发，并将
    该滴答数存于变量 ticks 中 */
    if (timesolt < SI) {
        ticks = 1;
    } else {
        ticks = timesolt / SI;
    }

    int rotation  = ticks / N; // 计算圈数
    int ts = (cur_slot + (ticks % N)) % N; // 计算待插入定时器的槽位

    timer->ratation = rotation;
    timer->time_slot = ts;

    // 插入合适的位置
    // 圈数大的放在最前面，即链表的前端
    if(!slots[ts]) {    // 头节点
        slots[ts] = timer;
    } else {    // 非头节点   
        timer->next = slots[ts];
        slots[ts]->prev = timer;
        slots[ts] = timer;
    }
}
    
    // 删除目标定时器
void time_wheel::del_timer(tw_timer* timer) {
    if(!timer) {
        return;
    } 
    int ts = timer->time_slot;
    // 如果是头节点
    if(slots[ts] == timer) {
        slots[ts] = slots[ts]->next;
        if(slots[ts]) {
            slots[ts]->prev = NULL;
        }
        delete timer;
    } else {
        timer->prev->next = timer->next;
        if(timer->next) {
            timer->next->prev = timer->prev;
        }
        delete timer;
    }
}

    // 调整定时器
void time_wheel::adjust_timer(tw_timer* timer,int timeslot) {
    if(!timer) {
        return;
    }
    // 先断开原位置的联系
    // 头节点
    /*
    if(timer == slots[timer->time_slot]) {
        slots[timer->time_slot] = slots[timer->time_slot]->next;
        slots[timer->time_slot]->prev = NULL;
        timer->next = NULL;
        //add_timer(timer,slots[timer->time_slot]);
    } else if(!timer->next) {  // 尾节点
        timer->prev->next = NULL;
        timer->prev = NULL;
    } else { // 中间节点
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        //add_timer(timer,slots[timer->time_slot]->next);
    } */
    int ts = timer->time_slot;
    // 如果是头节点
    if(slots[ts] == timer) {
        slots[ts] = slots[ts]->next;
        if(slots[ts]) {
            slots[ts]->prev = NULL;
        }
        timer->prev = NULL;
        timer->next = NULL;
    } else {
        timer->prev->next = timer->next;
        if(timer->next) {
            timer->next->prev = timer->prev;
        }
        timer->prev = NULL;
        timer->next = NULL;
    }
    // 调整新的圈数和槽位
    //timer->ratation += timeslot / SI / N;
    //timer->time_slot = (cur_slot + (timeslot / SI % N)) % N;
    
    // 添加到新的位置
    add_timer(timer,timeslot); 
}
/*
void time_wheel::add_timer(tw_timer* timer) {
    // 新槽位的首节点
    tw_timer* tmp = slots[timer->time_slot];
    // 如果首节点为空，直接添加
    if(!tmp) {
        slots[timer->time_slot] = timer;
    }
   
    while(tmp) {
        if(timer->ratation > tmp->ratation) {
            if(timer == tmp) {
                timer->next = tmp;
                tmp->prev = timer;
                slots[timer->time_slot] = timer;

            } else {
                timer->prev = tmp->prev;
                tmp->prev->next = timer;
                
                tmp->prev = timer;
                timer->next = tmp;
            }
        }
        tmp = tmp->next;
    }
    // 比此时链表上的所有节点的圈数都小
    if(!tmp) {
        tmp->prev->next = timer;
        timer->prev = tmp->prev;
        timer->next = NULL;
    }
} */

    // SI 时间到后，调用该函数，时间轮向前滚动一个槽的时间
void time_wheel::tick() {
    tw_timer* tmp = slots[cur_slot]; // 取得时间轮上当前槽的头节点
    while(tmp) {
        // 将当前槽位上的定时器链表遍历一遍，对它们的圈数标记-1，表示新的一轮计时开始；
        if(tmp->ratation > 0) {
            tmp->ratation--;
            tmp = tmp->next;
        } else {
        // 说明定时器到期，执行定时任务，然后删除定时器
            tmp->cb_func(tmp->user_data);
            if(tmp == slots[cur_slot]) {
                slots[cur_slot] = tmp->next;
                delete tmp;
                if(slots[cur_slot]) {
                    slots[cur_slot]->prev = NULL;
                }
                tmp = slots[cur_slot];
            } else {
                tmp->prev->next = tmp->next;
                if(tmp->next) {
                    tmp->next->prev = tmp->prev;
                }
                tw_timer* tmp2 = tmp->next;
                delete tmp;
                tmp = tmp2;
            }
        }
    }
    cur_slot = ++cur_slot % N; // 更新时间轮的当前槽，以反映时间轮的转动
}

void Utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//信号处理函数
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

//设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler()
{
    m_time_wheel.tick();
    alarm(m_TIMESLOT);
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;
void cb_func(client_data *user_data)
{
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
}

