#pragma once
#include<iostream>
#include<unordered_map>
#include<queue>
#include"Thread.h"
#include<atomic>
#include<condition_variable>
#include<tool.h>
using namespace std;

class Result;

class Task{
public:
    Task(): m_result(nullptr){}
    ~Task() = default;
    
    void setResult(Result* res){
        m_result = res;
    }

    virtual Any run() = 0;

    void exec();
    
private:
    Result* m_result;
};

    
class Result{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true) : m_task(task),m_isValid(isValid)
    {
        m_task->setResult(this);
    }

    void SetVal(Any any){
        this->m_any = std::move(any);
        m_sem.post();
    }

    Any get(){

        if(!m_isValid){
            return "";
        }

        m_sem.wait();
        return std::move(m_any);
    }

    ~Result() = default;

private:
    std::shared_ptr<Task> m_task;
    Any m_any;
    Semaphore m_sem;
    std::atomic_bool m_isValid;
};






enum class PoolMode{
    MODE_FIED, //固定数量的线程
    MODE_CACHED,//线程数量可以动态增长
};

class ThreadPool{
public:
    ThreadPool();

    ~ThreadPool();

    void setMode(PoolMode mode);//设置线程池的工作模式

    void setTaskQueMaxThreshHold(int threshhold);//设置任务队列阈值

    void setThreadSizeThreshHold(int threadHold);//设置线程池Cache模式下的线程阈值

    shared_ptr<Result> submitTask(std::shared_ptr<Task> sp);//提交任务

    void start(int initThreadSize = std::thread::hardware_concurrency());//thread::hardware_concurrency获取硬件并发数

    ThreadPool(const ThreadPool&) = delete;
    
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void threadFunc(int threadId);//线程函数

    bool checkRunningState()const;//检测线程状态
private:
    std::unordered_map<int,std::unique_ptr<Thread>> m_threads;//线程列表

    int m_initThreadSize;//线程初始化数量
    
    std::atomic_int m_curThreadSize;//线程总数量
    
    int m_threadSizeThreshHold;//线程数量上限阈值
    
    std::atomic_int m_idleThreadSize;//纪录空闲线程的数量

    std::queue<std::shared_ptr<Task>> m_taskque;//任务队列
    
    std::atomic_int m_taskSize;//任务的数量

    int m_taskqueMaxThreshHold;//任务队列的上限阈值

    std::mutex m_taskmutex;//保证任务队列线程安全

    std::condition_variable m_notFull;//任务队列不满

    std::condition_variable m_notEmpty;//任务队列不为空

    std::condition_variable m_exitCond;//等待所以线程资源全部回收

    PoolMode m_poolMode;//当前线程的工作模式

    std::atomic_bool m_isPoolRunning;
};


