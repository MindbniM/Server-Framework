#include "awaiter.h"
namespace MindbniM
{
    static thread_local bool t_hook_enable;

    bool hook_enable()
    {
        return t_hook_enable;
    }
    void set_hook_enable(bool flag)
    {
        t_hook_enable = flag;
    }

    SleepAwaiter::SleepAwaiter(std::chrono::milliseconds time):_dur(time)
    {
    }
    SleepAwaiter::SleepAwaiter(std::chrono::seconds time):_dur(time)
    {
    }
    bool SleepAwaiter::await_ready() const
    {
        if(!hook_enable())
        {
            ::sleep(_dur.count());
            return true;
        }
        return false;
    }
    void SleepAwaiter::await_suspend(std::coroutine_handle<> coroutine) const
    {
        //std::cout<<"添加定时任务"<<_dur.count()<<std::endl;
        IoManager::GetThis()->getTimerManager().addTimer(_dur,[coroutine]
        {
            coroutine.resume();
            if(coroutine.done())
            {
                coroutine.destroy();
            }
        },false);
    }
    void SleepAwaiter::await_resume()const {}

}