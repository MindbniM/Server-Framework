#pragma once
#include "schedule.h"
#include "timer.h"
#include "epoll.h"
namespace MindbniM
{
    enum Event
    {
        NONE = 0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT
    };
    struct EventContext
    {
        void clear()
        {
            _task.clear();
            _root = nullptr;
            epoll_event ev;
        }
        TaskAndF _task;  // 任务
        Schedule *_root; // 关联的调度器
    };
    struct FdContext
    {
        using ptr = std::shared_ptr<FdContext>;
        FdContext()=default;
        FdContext(int fd,Event event);
        ~FdContext();
        EventContext &getContext(Event ev);
        void triggerEvent(Event event);
        void clear();
        int _fd=-1;
        EventContext _read;
        EventContext _write;
        Event _event = Event::NONE;
    };
    class IoManager : public Schedule
    {
    public:
        IoManager(int threads=1,bool use_call=true,const std::string& name="Scheduler");
        ~IoManager();
        template<TaskType F>
        void addEvent(int fd,Event event,F cb);
        bool delEvent(int fd,Event event);
        bool cencelEvent(int fd,Event event);
        bool cencelAll(int fd);
        virtual void tickle() override;
        virtual bool stopping() override;
        virtual Task<void> idle() override;
        static IoManager* GetThis();
    private:
        Epoll _epoll;
        std::shared_mutex _mutex;
        std::atomic<int> _pendingEventCount={0};
        std::unordered_map<int, FdContext::ptr> _fdcontexts;
        TimerFd _tfd;
    };

    template<TaskType F>
    void IoManager::addEvent(int fd,Event event,F cb)
    {
        FdContext::ptr p=nullptr;
        std::shared_lock<std::shared_mutex> rlock(_mutex);
        if(_fdcontexts.count(fd))
        {
            p=_fdcontexts[fd];
            rlock.unlock();
        }
        else 
        {
            rlock.unlock();
            std::unique_lock<std::shared_mutex> wlock(_mutex);
            p=_fdcontexts[fd]=std::make_shared<FdContext>(fd,event);
        }
        if(p->_event&event)
        {
            return;
        }
        int op=p->_event? EPOLL_CTL_MOD:EPOLL_CTL_ADD;
        _epoll.ctlEvent(fd,EPOLLET|event|p->_event|EPOLLEXCLUSIVE,op);
        _pendingEventCount++;
        p->_event|=event;
        EventContext& ev=p->getContext(event);
        ev._root=Schedule::GetThis();
        ev._task=TaskAndF(cb);
    }
}