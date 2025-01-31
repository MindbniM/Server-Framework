#include "schedule.h"
namespace MindbniM
{
    static thread_local Schedule* t_schedule=nullptr;
    static thread_local std::coroutine_handle<> t_coroutine=nullptr;
    Schedule::Schedule(int threads,bool use_call,const std::string& name,bool auto_close):_name(name),_useCall(use_call),_autoClose(auto_close)
    {
        if(use_call)
        {
            threads--;
            Thread::setName(name);
        }
        else
        t_schedule=this;
        _threadCount=threads;
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
        auto idleTask=idle();
        TaskAndF t;
        while(1)
        {
            bool tick=false;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if(!_readyq.empty())
                {
                    t=_readyq.back();
                    _readyq.pop_back();
                    tick=!_readyq.empty();
                }
            }
            if(tick&&_idleCount)
            {
                tickle();
            }
            if(t._coroutine)
            {
                _activeCount++;
                if(!t._coroutine.done())
                {
                    t._coroutine.resume();
                }
                else
                {
                    if(t._coroutine.promise().managed_by_schedule()) 
                    {
                        t._coroutine.destroy();
                    }
                }
                _activeCount--;
            }
            else if(t._cb)
            {
                _activeCount++;
                if(t._cb)
                {
                    t._cb();
                }
                _activeCount--;
            }
            else 
            {
                if(!idleTask.get_coroutine().done())
                {
                    _idleCount++;
                    idleTask.resume();
                    _idleCount--;
                }
                else
                {
                    co_return;
                } 
            }
            t.clear();
        }
    }
    Task<void> Schedule::idle()
    {
        while(stopping())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            co_yield 0;
        }
    }
    bool Schedule::stopping()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _stop&&_activeCount==0&&_readyq.empty()&&_autoClose;
    }
    Schedule* Schedule::GetThis()
    {
        return t_schedule;
    }
    void Schedule::setScheduleCoroutine(Fiber coroutine)
    {
        t_coroutine=coroutine;
    }
    void Schedule::setThis()
    {
        t_schedule=this;
    }
    void Schedule::stop()
    {
        if(stopping())
        {
            return;
        }
        _stop=true;
        tickle();
        if(_useCall)
        {
            auto t=schedule();
            t.resume();
            for(auto& i:_threads)
            {
                i->join();
            }
        }
    }
    void Schedule::run()
    {
        auto task=schedule();
        setThis();
        setScheduleCoroutine(task.get_coroutine());
        task.resume();
    }
    Schedule::~Schedule()
    {
        if(GetThis()==this)
        {
            t_schedule=nullptr;
        }
    }
    void Schedule::tickle()
    {

    }
}