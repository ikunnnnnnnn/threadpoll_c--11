#include"ThreadPool.h"
#include<iostream>
#include<thread>
#include"Thread.h"
#include<chrono>
#include<pthread.h>
using namespace std;

const int TASK_MAX_THRESHHOLD = INT32_MAX;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60;
int Thread::m_generateId = 0;

ThreadPool::ThreadPool() : 
    m_idleThreadSize(0),
    m_initThreadSize(0),
    m_isPoolRunning(false),
    m_taskqueMaxThreshHold(TASK_MAX_THRESHHOLD),
    m_threadSizeThreshHold(THREAD_MAX_THRESHHOLD),
    m_taskSize(0),
    m_curThreadSize(0),
    m_poolMode(PoolMode::MODE_FIED)
{
}

ThreadPool::~ThreadPool(){
    m_isPoolRunning = false;

    std::unique_lock<std::mutex> lock(m_taskmutex);
    m_notEmpty.notify_all();
    m_exitCond.wait(lock,[&]()->bool{
        return m_threads.size() == 0;
    });
}


 void ThreadPool::setMode(PoolMode mode){
    if(checkRunningState())
        return; 
    m_poolMode = mode;
 }

 void ThreadPool::setTaskQueMaxThreshHold(int threshhold){
    m_taskqueMaxThreshHold = threshhold;
 }

 void ThreadPool::setThreadSizeThreshHold(int threadHold){
    if(checkRunningState())
        return; 
    if(m_poolMode == PoolMode::MODE_CACHED)
        m_threadSizeThreshHold = threadHold;
 }

 std::shared_ptr<Result> ThreadPool::submitTask(std::shared_ptr<Task> sp){
    std::unique_lock<std::mutex>lock(m_taskmutex);
    //最长阻塞不能超过1s 否则提交失败
    if(!m_notFull.wait_for(lock,std::chrono::seconds(1),[&](){
        return m_taskSize < TASK_MAX_THRESHHOLD;
    }))
    {
        std::perror("task queue is full ,submit is failed");
        return make_shared<Result>(sp,false);
    }

    m_taskque.emplace(sp);
    m_taskSize++;
    m_notFull.notify_all();

    if(m_poolMode == PoolMode::MODE_CACHED && m_taskSize > m_idleThreadSize && m_curThreadSize < m_threadSizeThreshHold){
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this,std::placeholders::_1));
        int threadId = ptr->getId();
        m_threads.emplace(threadId,std::move(ptr));
        m_threads[threadId]->start();

        m_curThreadSize++;
        m_idleThreadSize++;
    }

    return make_shared<Result>(sp);
 }

 void ThreadPool::start(int initThreadSize)
 {
    m_isPoolRunning = true;
    m_initThreadSize = initThreadSize;
    m_curThreadSize = initThreadSize;

    for(int i = 0; i < m_initThreadSize; ++i){
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this,std::placeholders::_1));
        int threadId = ptr->getId();
        m_threads.emplace(threadId,std::move(ptr));
    }   

    for(int i = 0; i < m_initThreadSize; ++i){

        m_threads[i]->start();
        m_idleThreadSize++;
    }
 }

 void ThreadPool::threadFunc(int threadId){
    auto lasttime = std::chrono::high_resolution_clock().now();

    while (true)
    {   
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(m_taskmutex);

            cout<<"当前线程tid :" << this_thread::get_id() << "获取任务"<< endl;

            while(m_taskque.size() == 0){
                if(!m_isPoolRunning){
                    m_threads.erase(threadId);
                    cout<<"当前线程tid:"<<this_thread::get_id()<<"exit.."<<endl;
                    m_exitCond.notify_all();//通知此线程退出
                    return;
                }

                if(PoolMode::MODE_CACHED == m_poolMode){
                    if(std::cv_status::timeout == m_notEmpty.wait_for(lock,std::chrono::seconds(1))){//一秒后检测队列是否为空 为空在检测线程距离上次队列为空时所用时间 超过1min回收线程
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lasttime);
                        if(dur.count() >= THREAD_MAX_IDLE_TIME && m_curThreadSize > m_initThreadSize){
                            m_threads.erase(threadId);
                            m_curThreadSize--;
                            m_idleThreadSize--;
                            std::cout<<"线程 tid:"<<std::this_thread::get_id()<<"exit"<<endl;
                            return;
                        }
                    }
                }else{
                    m_notEmpty.wait(lock);
                }
            }
            //取任务
            m_idleThreadSize--;
            cout<<"tid:"<<this_thread::get_id()<<"获取任务成功"<<endl;

            task = m_taskque.front();//获取任务
            m_taskque.pop();
            m_taskSize--;

            if(m_taskSize > 0)
                m_notEmpty.notify_all();//任务队列不为空 

            m_notFull.notify_all();//任务队列不满
        }

        if(task != nullptr)
            task->exec();

        m_idleThreadSize++;
        lasttime = std::chrono::high_resolution_clock().now();
    }
    
 }

 bool ThreadPool::checkRunningState()const{
    return m_isPoolRunning;
 }

 void Task::exec(){
        
        if(m_result != nullptr){
             m_result->SetVal(run());
        }   
    }   