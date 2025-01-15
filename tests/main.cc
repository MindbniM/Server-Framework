#include <coroutine>
#include <iostream>
#include "../code/coroutine.hpp"
using namespace MindbniM;
Task<double> word()
{
    std::cout<<"word"<<std::endl;
    co_return 3.14;
}
Task<int> hello()
{
    std::cout<<"hello"<<std::endl;
    double temp=co_await word();
    std::cout<<"word co_await -> "<<temp<<std::endl;
    co_yield 1;
    co_yield 2;
    co_yield 3;
    co_return 10;
}
    //1.1当调用 word() 时，编译器会为其生成协程。
    //1.2调用 Promise<double>::get_return_object() 返回一个 std::coroutine_handle<Promise<double>>，这是协程的句柄。
    //1.3协程开始执行，遇到 std::suspend_always()（由 initial_suspend 提供），协程挂起。
    //1.4生成的 std::coroutine_handle 会被 Task<double> 包装成对象 _coroutine，作为其内部协程的管理句柄。

    //co_await
    //2.1调用 co_await word() 时，会触发 Task<double> 的 operator co_await()，返回 Awaiter 对象。
    //2.2调用 Awaiter::await_ready() 此函数返回 false，表明协程未准备好，控制权将切换。
    //2.3调用 Awaiter::await_suspend()将调用方的句柄保存到 Promise<double>::_coroutine，用于在当前协程完成后恢复调用方。
    //2.4返回 _coroutine（word 的协程句柄），表明应切换执行到 word。执行协程 word

    //word 的协程开始执行主体代码。
    //3.1调用 Promise<double>::return_value(3.14) 保存返回值 _value。
    //3.2协程结束调用 Promise<double>::final_suspend() 返回 PreviousAwaiter，切换控制权回到调用方协程句柄 _coroutine。

    //回到co_await的最后一个函数
    //4.调用 Awaiter::await_resume()


int main() 
{
    std::cout << "main即将调用hello" << std::endl;
    Task t = hello();
    std::cout << "main调用完了hello" << std::endl;

    while (!t._coroutine.done()) 
    {
        t._coroutine.resume();
        std::cout << "main得到hello结果为 " << t._coroutine.promise()._value << std::endl;
    }

    return 0;
}
