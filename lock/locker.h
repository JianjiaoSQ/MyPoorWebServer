#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem
{
public:
    /*创建并初始化信号量*/
    sem() 
    {
        if (sem_init(&m_sem, 0, 0) != 0) // 初始化成功返回0，错误时为-1
        {
            throw std::exception();
        }
    }
    sem(int num) 
    {
        if (sem_init(&m_sem, 0, num) != 0) //num为信号量初始值
        {
            throw std::exception();
        }
    }
    ~sem() /*销毁信号量*/
    {
        sem_destroy(&m_sem);
    }
    bool wait()
    {
        return sem_wait(&m_sem) == 0; // 信号量值-1
    }
    bool post()
    {
        return sem_post(&m_sem) == 0; // 信号量值+1
    }

private:
    sem_t m_sem;
};
class locker /* 互斥锁类，用于同步线程对共享数据的访问 */
{
public:
    locker() /*创建并初始化*/
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock() // 获取互斥锁
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()  // 释放互斥锁
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get() // 得到指向操作目标的指针
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};
/* 条件变量，线程之间同步共享数据的值 */
class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal() /* 唤醒等待条件变量的线程 */
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast() /* 以广播的形式唤醒所有等待条件变量的线程 */
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif
