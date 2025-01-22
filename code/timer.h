#pragma once
#include"log.h"
#include<chrono>
#include<shared_mutex>
namespace MindbniM
{
    class TimerManager;
    struct Timer : public std::enable_shared_from_this<Timer>
    {
        using ptr=std::shared_ptr<Timer>;

        Timer(std::chrono::milliseconds ms,std::function<void()> cb,bool recurring,TimerManager* manager);
        bool cancel();
        bool refresh();
        void clear();
        bool reset(std::chrono::milliseconds ms,bool from_now);
        bool _recurring;
        uint64_t _id;
        std::chrono::milliseconds _ms;
        std::chrono::steady_clock::time_point _timeOut;
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
        Timer::ptr addTimer(std::chrono::milliseconds ms,std::function<void()> cb,bool recurring=false);
        void addTimer(Timer::ptr timer);
        bool erase(Timer::ptr timer);
        static void onTimer(std::function<void()> cb,std::weak_ptr<void> weak_cond);
        uint64_t getNextTimer();
        bool count(Timer::ptr);
        void listcb(std::vector<std::function<void()>>& cbs);
        Timer::ptr addConditionTimer(std::chrono::milliseconds ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring=false);
        virtual void onTimerInsertedAtFront() = 0;

    protected:
        std::shared_mutex _mutex;
        std::set<Timer::ptr,__TimeComp__> _timers;
    };
}