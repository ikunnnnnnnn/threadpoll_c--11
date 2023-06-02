#pragma once
#include<iostream>
#include<memory>
#include<mutex>
#include<atomic>
#include<condition_variable>

using namespace std;

class Any{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    template<typename T>
    Any(T data) : m_base(std::make_unique<Derive<T>>(data)){
    }

    template<typename T>
    T cast(){
        Derive<T>* dp = dynamic_cast<Derive<T>*>(m_base.get());

        if(dp == nullptr){
            perror("error");
        }

        return dp->m_data;
    }

private:
    class Base{
        public:
            virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base{
        public:
            Derive(T data) : m_data(data){
            }
            T m_data;
    };

private:
    std::unique_ptr<Base> m_base;
};


class Semaphore{
    public:
        Semaphore(int limit = 0) : m_resLimit(limit),m_isExit(false){

        }

        ~Semaphore(){
            m_isExit = true;
        }

        void wait(){
            if(m_isExit)
                return;
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[&]()->bool{
                return m_resLimit > 0;
            });
            m_resLimit--;
        }

        void post(){
            if(m_isExit)
                return;
            std::unique_lock<std::mutex> lock(m_mutex);
            m_resLimit++;
            m_cond.notify_all();
        }
        
    private:
        std::mutex m_mutex;
        std::atomic_bool m_isExit;
        std::condition_variable m_cond;
        int m_resLimit; 
};
