// 基于 RAII 的思想对对象进行封装 

#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// RAII信号量
class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    // 对信号量尝试减一操作 无法操作时会阻塞 原子
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    // 对信号量加一操作 原子
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

// RAII 互斥锁
class locker
{
public:
    locker()
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
    // 加锁 原子
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    // 解锁 原子
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    // 获取互斥锁
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};

// RAII 条件变量
class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            throw std::exception();  
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    // 使用前当前线程应加锁 防止修改共享资源导致唤醒丢失 阻塞后 wait 会自动释放锁 唤醒时再尝试加锁
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        ret = pthread_cond_wait(&m_cond, m_mutex); 
        return ret == 0;
    }
    // 在 wait 基础上多了超时唤醒机制 
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        return ret == 0;
    }
    // 唤醒单个线程
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    // 唤醒所有线程
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
};
#endif
