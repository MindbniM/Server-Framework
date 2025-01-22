#include "timer.h"
namespace MindbniM
{
    static std::atomic<uint64_t> t_timer_id = {0};
    Timer::Timer(std::chrono::milliseconds ms, std::function<void()> cb, bool recurring, TimerManager *manager)
        : _ms(ms), _cb(cb), _recurring(recurring), _manager(manager)
    {
        _id = t_timer_id++;
        _timeOut=std::chrono::steady_clock::now()+ms;
    }
    bool Timer::cancel()
    {
        if(_manager==nullptr)
        {
            LOG_WARNING(LOG_NAME("system"))<<"this Timer is not in manager";
            return ;
        }
        _manager->erase(shared_from_this());
    }
    bool Timer::refresh()
    {
        if(_manager==nullptr)
        {
            LOG_WARNING(LOG_NAME("system"))<<"this Timer is not in manager";
            return false;
        }
        if(!_manager->count(shared_from_this())) 
        {
            return false;
        }
        _manager->erase(shared_from_this());
        _timeOut=std::chrono::steady_clock::now()+_ms;
        _manager->addTimer(shared_from_this());
    }
    bool Timer::reset(std::chrono::milliseconds ms,bool from_now)
    {
        if(_manager==nullptr)
        {
            LOG_WARNING(LOG_NAME("system"))<<"this Timer is not in manager";
            return false;
        }
        if(from_now)
        {
            _ms=ms;
            return refresh();
        }
        if(!_manager->count(shared_from_this())) 
        {
            return false;
        }
        std::chrono::milliseconds dur=ms-_ms;
        _ms=ms;
        _manager->erase(shared_from_this());
        _timeOut+=dur;
        _manager->addTimer(shared_from_this());
    }
    void Timer::clear()
    {
        _manager=nullptr;
        _cb=nullptr;
    }
    Timer::ptr TimerManager::addTimer(std::chrono::system_clock::duration ms, std::function<void()> cb, bool recurring = false)
    {
        bool need=false;
        Timer::ptr p=std::make_shared<Timer>(ms,cb,recurring,this);
        {
            std::unique_lock<std::shared_mutex> lock(_mutex);
            const auto &[it, _] = _timers.insert(p);
            need=it==_timers.begin();
        }
        if(need)
        {
            onTimerInsertedAtFront();
        }
        return p;
    }
    void TimerManager::addTimer(Timer::ptr timer)
    {
        bool need=false;
        {
            std::unique_lock<std::shared_mutex> lock(_mutex);
            const auto &[it, _] = _timers.insert(timer);
            need=it==_timers.begin();
        }
        if(need)
        {
            onTimerInsertedAtFront();
        }
    }
    bool TimerManager::count(Timer::ptr timer)
    {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _timers.count(timer);
    }
    bool TimerManager::erase(Timer::ptr timer)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it=_timers.find(timer);
        if(it!=_timers.end())
        {
            _timers.erase(it);
            return true;
        }
        return false;
    }
    void TimerManager::onTimer(std::function<void()> cb,std::weak_ptr<void> weak_cond)
    {
        std::shared_ptr<void> p=weak_cond.lock();
        if(p)
        {
            cb();
        }
    }
    Timer::ptr TimerManager::addConditionTimer(std::chrono::system_clock::duration ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false)
    {
        return addTimer(ms,std::bind(&TimerManager::onTimer,cb,weak_cond),recurring);
    }
    uint64_t TimerManager::getNextTimer()
    {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        if(_timers.empty())
        {
            return UINT64_MAX;
        }
        Timer::ptr p=*_timers.begin();
        std::chrono::duration dur=p->_timeOut-std::chrono::steady_clock::now();
        std::chrono::milliseconds ret=std::chrono::duration_cast<std::chrono::milliseconds>(dur);
        return ret.count();
    }
    void TimerManager::listcb(std::vector<std::function<void()>>& cbs)
    {
        std::chrono::steady_clock::time_point now=std::chrono::steady_clock::now();
        cbs.clear();
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it=_timers.begin();
        while(it!=_timers.end())
        {
            if((*it)->_timeOut>now)
            {
                break;
            }
            Timer::ptr p=*it;
            _timers.erase(it);
            cbs.push_back(p->_cb);
            if(p->_recurring)
            {
                p->_timeOut=now+p->_ms;
                _timers.insert(p);
            }
            else
            {
                p->clear();
            }
        }
    }
}