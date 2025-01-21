#include "timer.h"
namespace MindbniM
{
    static std::atomic<uint64_t> t_timer_id = {0};
    Timer::Timer(std::chrono::system_clock::duration ms, std::function<void()> cb, bool recurring, TimerManager *manager)
        : _ms(ms), _cb(cb), _recurring(recurring), _manager(manager)
    {
        _id = t_timer_id++;
    }
    bool Timer::cancel()
    {
    }
    bool Timer::refresh()
    {
    }
    bool TimerManager::addTimer(std::chrono::system_clock::duration ms, std::function<void()> cb, bool recurring = false)
    {
        bool need=false;
        std::unique_lock<std::shared_mutex> lock(_mutex);
        {
            const auto &[it, _] = _timers.insert(std::make_shared<Timer>(ms, cb, recurring, this));
            need=it==_timers.begin();
        }
        if(need)
        {
            onTimerInsertedAtFront();
        }
    }
    void TimerManager::onTimer(std::function<void()> cb,std::weak_ptr<void> weak_cond)
    {
        std::shared_ptr<void> p=weak_cond.lock();
        if(p)
        {
            cb();
        }
    }
    bool TimerManager::addConditionTimer(std::chrono::system_clock::duration ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false)
    {
        return addTimer(ms,std::bind(&TimerManager::onTimer,cb,weak_cond),recurring);
    }
    uint64_t TimerManager::getNextTimer()
    {
        std::shared_lock<std::shared_mutex> _lock(_mutex);
        if(_timers.empty())
        {
            return UINT64_MAX;
        }
        Timer::ptr p=*_timers.begin();
        std::chrono::duration dur=p->_timeOut-std::chrono::system_clock::now();
        std::chrono::milliseconds ret=std::chrono::duration_cast<std::chrono::milliseconds>(dur);
        return ret.count();
    }
}