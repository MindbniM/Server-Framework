#include <coroutine>
#include <iostream>
#include <unordered_map>
#include "schedule.h"

using namespace MindbniM;
//Task<double> word()
//{
//    std::cout<<"word"<<std::endl;
//    std::cout<<"word 开始挂起"<<std::endl;
//    co_await SleepAwaiter(std::chrono::seconds(10));
//    std::cout<<"word 挂起结束"<<std::endl;
//    co_return 3.14;
//}
Task<void> hello()
{
    std::cout<<"hello"<<std::endl;
    co_yield 0;
    std::cout<<"1"<<std::endl;
    co_yield 0;
    std::cout<<"2"<<std::endl;
    co_yield 0;
    std::cout<<"3"<<std::endl;
    co_yield 0;
    std::cout<<"4"<<std::endl;
    co_return ;
}
Task<void> word()
{
    std::cout<<"word"<<std::endl;
    co_yield 0;
    std::cout<<"5"<<std::endl;
    co_yield 0;
    std::cout<<"6"<<std::endl;
    co_yield 0;
    std::cout<<"7"<<std::endl;
    co_yield 0;
    std::cout<<"8"<<std::endl;
    co_return ;
}
int main() 
{
    auto t1=hello();
    auto t2=hello();
    //t1.get_coroutine().promise().set_managed_by_schedule();
    std::cout<<"调度器构造"<<std::endl;
    Schedule s(3);
    std::cout<<"调度器构造完成"<<std::endl;
    s.push(t1.get_coroutine());
    s.push(t2.get_coroutine());
    std::cout<<"调度器启动"<<std::endl;
    s.start();
    std::cout<<"调度器准备停止"<<std::endl;
    s.stop();
    return 0;
}
