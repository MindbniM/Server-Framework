#pragma once
#include<set>
#include<functional>
#include<memory>
#include<chrono>
#include<atomic>
#include<mutex>
#include<shared_mutex>
namespace MindbniM
{
    class TimerManager;
    struct Timer : public std::enable_shared_from_this<Timer>
    {
        using ptr=std::shared_ptr<Timer>;

        Timer(std::chrono::system_clock::duration ms,std::function<void()> cb,bool recurring,TimerManager* manager);
        bool cancel();
        bool refresh();

        bool _recurring;
        uint64_t _id;
        std::chrono::system_clock::duration _ms;
        std::chrono::system_clock::time_point _timeOut;
        std::function<void()> _cb;
        TimerManager* _manager=nullptr;
    };
    struct __TimeComp__
    {
        bool operator()(const Timer::ptr& p1,const Timer::ptr& p2) const
        {
            if(p1->_timeOut==p2->_timeOut)
            {
                return p1->_id<p2->_id;
            }
            return p1->_timeOut<p2->_timeOut;
        }
    };
    class TimerManager
    {
    public:
        TimerManager();
        bool addTimer(std::chrono::system_clock::duration ms,std::function<void()> cb,bool recurring=false);
        static void onTimer(std::function<void()> cb,std::weak_ptr<void> weak_cond);
        uint64_t getNextTimer();
        bool addConditionTimer(std::chrono::system_clock::duration ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring=false);
        virtual void onTimerInsertedAtFront() = 0;

    protected:
        std::shared_mutex _mutex;
        std::set<Timer::ptr,__TimeComp__> _timers;
    };
}