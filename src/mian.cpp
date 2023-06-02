#include"ThreadPool.h"
#include<iostream>
using namespace std;

class mytask : public Task{
public:
    mytask(int s,int e) : start(s),end(e){
        
    }

    Any run(){
        cout<<"线程tid: "<<std::this_thread::get_id()<<"开始任务"<<endl;
        unsigned long long sum = 0;
        for(int i = start; i < end; ++i){
            sum += i;
        }
        return sum;
    }

private:    
    int start;
    int end;    
};


int main(){
    {
        ThreadPool tp;
        tp.setMode(PoolMode::MODE_CACHED);

        tp.start(2);
        std::shared_ptr<Result> result1 = tp.submitTask(std::make_shared<mytask>(1,100));
        std::shared_ptr<Result> result2 = tp.submitTask(std::make_shared<mytask>(1,200));
        std::shared_ptr<Result> result3 = tp.submitTask(std::make_shared<mytask>(100,200));
        tp.submitTask(std::make_shared<mytask>(300,400));
        tp.submitTask(std::make_shared<mytask>(400,1000));
        tp.submitTask(std::make_shared<mytask>(500,1000));

        unsigned long long res = result1->get().cast<unsigned long long>();
        cout<<"main res = "<<res<<endl;
    }

    cout<<"main end"<<endl;
    return 0;
};