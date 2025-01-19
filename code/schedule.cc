#include "schedule.h"
namespace MindbniM
{
    static thread_local Schedule* t_schedule=nullptr;
    Schedule::Schedule(int threads,const std::string& name=""):_name(name)
    {
        threads--;
        t_schedule=this;
        Thread::setName(name);
        _threadCount=threads;
    }
    void Schedule::push(std::coroutine_handle<> coroutine,uint64_t thread_id=-1)
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
    void Schedule::setThis()
    {
        t_schedule=this;
    }
    void Schedule::run()
    {
        auto task=schedule();
        setThis();
        task.resume();
    }
}