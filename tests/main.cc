#include <coroutine>
#include <iostream>
#include "../code/coroutine.hpp"
using namespace MindbniM;
Task<double> word()
{
    std::cout<<"word"<<std::endl;
    std::cout<<"word 开始挂起"<<std::endl;
    co_await SleepAwaiter(std::chrono::seconds(10));
    std::cout<<"word 挂起结束"<<std::endl;
    co_return 3.14;
}
Task<int> hello()
{
    std::cout<<"hello"<<std::endl;
    std::cout<<"hello 开始挂起"<<std::endl;
    co_await SleepAwaiter(std::chrono::seconds(3));
    std::cout<<"hello 挂起结束"<<std::endl;
    co_return 10;
}
int main() 
{
    auto t1=word();
    auto t2=hello();
    auto& loop=Schedule::GetInstance();
    loop.push(t1._coroutine);
    loop.push(t2._coroutine);
    loop.runAll();
    return 0;
}
