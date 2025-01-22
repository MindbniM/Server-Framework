#pragma once
#include"schedule.h"
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/epoll.h>
namespace MindbniM
{
    enum Event
    {
        NONE=0,
        READ=EPOLLIN,
        WRItE=EPOLLOUT
    };
    struct EventContext
    {
        void clear()
        {
            _task.clear();
            _root=nullptr;
            epoll_event ev;
        }
        TaskAndF _task;     //任务
        Schedule* _root;    //关联的调度器
    };
    struct FdContext
    {
        EventContext& getContext(Event ev)
        {
            switch(ev)
            {
                case Event::READ:
                    return _read;
                case Event::WRItE:
                    return _write;
                default:
                {
                    LOG_WARNING(LOG_NAME("system"))<<"unknow event";
                    throw std::logic_error("unknow event");
                }
            }
        }
        int _fd;
        EventContext _read;
        EventContext _write;
        Event _event=Event::NONE;
    };
    class IoManager
    {
    public:
    private:
        int _epollfd;           //epoll-fd
        int _fd;                //定时器触发fd
    };
}