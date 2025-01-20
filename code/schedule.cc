#include "schedule.h"
namespace MindbniM
{
    static thread_local Schedule* t_schedule=nullptr;
    static thread_local std::coroutine_handle<> t_coroutine=nullptr;
    Schedule::Schedule(int threads,bool use_call,const std::string& name):_name(name),_useCall(use_call)
    {
        if(use_call)
        {
            threads--;
        }
        t_schedule=this;
        Thread::setName(name);
        _threadCount=threads;
    }
    void Schedule::push(std::coroutine_handle<> coroutine)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _readyq.push_front(coroutine);
    }
    void Schedule::start()
    {
        LOG_DEBUG(LOG_ROOT())<<"start";
        if(_stop)
        {
            LOG_INFO(LOG_ROOT())<<"Schedule is stoping";
            return;
        }
        _threads.resize(_threadCount);
        for(int i=0;i<_threadCount;i++)
        {
            _threads[i].reset(new Thread(_name+"-"+std::to_string(i),&Schedule::run,this));
        }
        
    }
    Task<void> Schedule::schedule()
    {
    }
    Schedule* Schedule::GetThis()
    {
        return t_schedule;
    }
    void Schedule::setScheduleCoroutine(std::coroutine_handle<> coroutine)
    {
        t_coroutine=coroutine;
    }
    void Schedule::setThis()
    {
        t_schedule=this;
    }
    void Schedule::run()
    {
        auto task=schedule();
        setThis();
        setScheduleCoroutine(task.get_coroutine());
        task.resume();
    }
}