#ifndef _TIME_WHEEL_TIMER_H_
#define _TIME_WHEEL_TIMER_H_

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"


class tw_timer;

struct client_data {
    sockaddr_in address;
    int sockfd;
    // char buf[BUFFER_SIZE];
    tw_timer* timer;
};

class tw_timer { // 定时器类
public:
    tw_timer (int rot, int ts) : next(NULL), prev(NULL), ratation(rot), time_slot(ts) {}
public:
    int ratation; // 记录定时器在时间轮转多少圈之后生效
    int time_slot; // 记录定时器属于时间轮上哪个槽
    void (*cb_func) (client_data*); // 定时器回调函数
    client_data* user_data; // 客户数据

    tw_timer* next; // 指向下一个定时器
    tw_timer* prev; // 指向前一个定时器
};

class time_wheel { // 时间轮定时器
public:
    time_wheel();
    ~time_wheel();

    // 添加定时器
    void add_timer(tw_timer* timer, int timeslot);
    // 调整定时器
    void adjust_timer(tw_timer* timer, int timeslot);
    // 删除目标定时器
    void del_timer(tw_timer* timer);
    // SI 时间到后，调用该函数，时间轮向前滚动一个槽的时间
    void tick();

public:
    static const int N = 60; // 时间轮上的槽数
    static const int SI = 1; // 槽之间时间间隔为 1s
    int cur_slot; // 时间轮上的当前槽 (设置成 public 存在危险)

private:
    tw_timer* slots[N]; // 时间轮的槽集合，其中每个元素指向一个定时器链表，链表无序 ？？(不是倍数吗？？？)
    //void add_timer(tw_timer* timer);
};

class Utils // 处理长时间非活动连接类
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    static int u_epollfd;
    time_wheel m_time_wheel;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif
