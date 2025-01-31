#pragma once
#include"iomanager.h"
#include<unistd.h>
#include<dlfcn.h>
namespace MindbniM
{
    /**
     * @brief 是否使用自定义await
     */
    bool hook_enable();

    /**
     * @brief 设置使用await
     * 比如 SleepAwaiter, 如果不启用该选项, 那么就相当于调用原始sleep, 如果启用, 那么就让休眠的协程加入定时器 
     */
    void set_hook_enable(bool flag);


    /**
     * @brief 此等待体是为了当协程休眠时自动加入定时器
     */
    struct SleepAwaiter
    {
        SleepAwaiter(std::chrono::milliseconds time);
        SleepAwaiter(std::chrono::seconds time);
        bool await_ready() const;
        void await_suspend(std::coroutine_handle<> coroutine) const;
        void await_resume()const;

        std::chrono::milliseconds _dur;
    };


}