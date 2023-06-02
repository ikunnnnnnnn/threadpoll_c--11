#pragma once
#include<thread>
#include<functional>
#include<iostream>
using namespace std;

class Thread{
public:
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc func) : m_func(func)
                            ,m_threadId(m_generateId++) 
    {

    };
    ~Thread(){

    };
    void start(){
        std::thread t(m_func,m_threadId);
        t.detach();//与主线程分离  后台执行 守护线程
    }
    int getId()const{
        return m_threadId;
    }
private:
    ThreadFunc m_func;
    int m_threadId;
    static int m_generateId;
};