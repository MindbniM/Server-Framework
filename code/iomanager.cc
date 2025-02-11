#include "iomanager.h"
namespace MindbniM
{

    EventContext &FdContext::getContext(Event ev)
    {
        switch (ev)
        {
        case Event::READ:
            return _read;
        case Event::WRITE:
            return _write;
        default:
        {
            LOG_WARNING(LOG_ROOT()) << "unknow event";
            throw std::invalid_argument("unknow event");
        }
        }
    }
    void FdContext::clear()
    {
        _read.clear();
        _write.clear();
        _event = Event::NONE;
    }
    void FdContext::triggerEvent(Event event)
    {
        EventContext &ev = getContext(event);
        if (ev._task._cb)
        {
            ev._root->push(ev._task._cb);
            if(ev._recurring)
            {
                IoManager::GetThis()->addEvent(_fd,event,ev._task._cb,ev._recurring);
            }
        }
        else if (ev._task._coroutine)
        {
            ev._root->push(ev._task._coroutine);
            if(ev._recurring)
            {
                IoManager::GetThis()->addEvent(_fd,event,ev._task._coroutine,ev._recurring);
            }
        }
    }
    FdContext::FdContext(int fd, Event event) : _fd(fd), _event(event)
    {
    }
    FdContext::~FdContext()
    {
        clear();
    }
    IoManager::IoManager(int threads, bool use_call, const std::string &name, bool auto_close)
        : Schedule(threads, use_call, name, auto_close)
    {
        Util::setFdNoBlock(_tfd.fd());
        _epoll.addEvent(_tfd.fd(), EPOLLIN | EPOLLET);
        start();
    }
    IoManager::~IoManager()
    {
        stop();
    }
    IoManager *IoManager::GetThis()
    {
        return dynamic_cast<IoManager *>(Schedule::GetThis());
    }
    void IoManager::contextResize(size_t size)
    {
        _fdcontexts.resize(size);

        for (size_t i = 0; i < _fdcontexts.size(); ++i)
        {
            if (!_fdcontexts[i])
            {
                _fdcontexts[i] = new FdContext(i, Event::NONE);
                _fdcontexts[i]->_fd = i;
            }
        }
    }
    // bool IoManager::delEvent(int fd, Event event)
    //{
    //     FdContext::ptr p = nullptr;
    //     std::shared_lock<std::shared_mutex> rlock(_mutex);
    //     if ((int)_fdcontexts.size()<fd)
    //         return false;
    //     p = _fdcontexts[fd];
    //     rlock.unlock();
    //     if (!(p->_event & event))
    //         return false;
    //     Event nevent = (Event)((p->_event) & (~event));
    //     int op = nevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    //     _epoll.ctlEvent(fd, EPOLLET | (int)nevent, op);
    //     LOG_INFO(LOG_ROOT())<<"del... fd: "<<fd<<" "<<p->_event<<" "<<event<<" "<<op;
    //     --_pendingEventCount;
    //     p->_event = nevent;
    //     if(event&READ)  p->getContext(READ).clear();
    //     if(event&WRITE)  p->getContext(WRITE).clear();
    //     return true;
    // }
    bool IoManager::delEvent(int fd, Event event)
    {
        FdContext *fd_ctx = nullptr;
        std::shared_lock<std::shared_mutex> read_lock(_mutex); // 读锁
        if ((int)_fdcontexts.size() > fd)
        {
            fd_ctx = _fdcontexts[fd];
            read_lock.unlock();
        }
        else
        {
            read_lock.unlock();
            return false;
        }
        std::lock_guard<std::mutex> lock(fd_ctx->_mutex);

        if (!(fd_ctx->_event & event))
        {
            return false;
        }

        Event new_events = (Event)(fd_ctx->_event & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        _epoll.ctlEvent(fd, &epevent, op);

        --_pendingEventCount;
        fd_ctx->_event = new_events;

        EventContext &event_ctx = fd_ctx->getContext(event);
        event_ctx.clear();
        return true;
    }
    bool IoManager::cancelEvent(int fd, Event event)
    {
        FdContext *fd_ctx = nullptr;
        std::shared_lock<std::shared_mutex> read_lock(_mutex);
        if ((int)_fdcontexts.size() > fd)
        {
            fd_ctx = _fdcontexts[fd];
            read_lock.unlock();
        }
        else
        {
            read_lock.unlock();
            return false;
        }

        std::lock_guard<std::mutex> lock(fd_ctx->_mutex);

        if (!(fd_ctx->_event & event))
        {
            return false;
        }

        Event new_events = (Event)(fd_ctx->_event & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        _epoll.ctlEvent(fd, &epevent, op);

        --_pendingEventCount;

        fd_ctx->triggerEvent(event); // 和delete最后的处理不同一个是重置，一个是调用事件的回调函数
        return true;
    }
    // bool IoManager::cencelEvent(int fd, Event event)
    //{
    //     FdContext::ptr p = nullptr;
    //     std::shared_lock<std::shared_mutex> rlock(_mutex);
    //     if ((int)_fdcontexts.size()<fd)
    //         return false;
    //     p = _fdcontexts[fd];
    //     rlock.unlock();
    //     if (!(p->_event & event))
    //         return false;
    //     Event nevent = (Event)((p->_event) & (~event));
    //     int op = nevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    //     LOG_INFO(LOG_ROOT())<<"cencel fd: "<<fd;
    //     _epoll.ctlEvent(fd, EPOLLET | (int)event, op);
    //     --_pendingEventCount;
    //     p->triggerEvent(event);
    //     return true;
    // }
    bool IoManager::cancelAll(int fd)
    {
        FdContext *fd_ctx = nullptr;

        std::shared_lock<std::shared_mutex> read_lock(_mutex);
        if ((int)_fdcontexts.size() > fd)
        {
            fd_ctx = _fdcontexts[fd];
            read_lock.unlock();
        }
        else
        {
            read_lock.unlock();
            return false;
        }

        std::lock_guard<std::mutex> lock(fd_ctx->_mutex);

        if (!fd_ctx->_event)
        {
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = 0;
        epevent.data.ptr = fd_ctx;

        _epoll.ctlEvent(fd, &epevent, op);
        if (fd_ctx->_event & READ)
        {
            fd_ctx->triggerEvent(READ);
            --_pendingEventCount;
        }
        if (fd_ctx->_event & WRITE)
        {
            fd_ctx->triggerEvent(WRITE);
            --_pendingEventCount;
        }

        return true;
    }
    // bool IoManager::cencelAll(int fd)
    //{
    //     FdContext::ptr p = nullptr;
    //     std::shared_lock<std::shared_mutex> rlock(_mutex);
    //     if ((int)_fdcontexts.size()<fd)
    //         return false;
    //     p = _fdcontexts[fd];
    //     rlock.unlock();
    //     if (!p->_event)
    //         return false;
    //     _epoll.delEvent(fd);
    //     if (p->_event & READ)
    //     {
    //         p->triggerEvent(READ);
    //         --_pendingEventCount;
    //     }
    //     if (p->_event & WRITE)
    //     {
    //         p->triggerEvent(WRITE);
    //         --_pendingEventCount;
    //     }
    //     return true;
    // }
    void IoManager::tickle()
    {
        _tfd.tickle();
    }
    bool IoManager::stopping()
    {
        uint64_t timeout = _tfd.getNextTimer();
        return timeout == UINT64_MAX && _pendingEventCount == 0 && Schedule::stopping();
    }
    Task<void> IoManager::idle()
    {
        // 使用 std::unique_ptr 动态分配了一个大小为 MAX_EVENTS 的 epoll_event 数组，用于存储从 epoll_wait 获取的事件。
        static std::vector<epoll_event> events;

        while (true)
        {
            //LOG_DEBUG(LOG_ROOT()) << "name = " << getName() << " idle enters in thread: " << Thread::GetName() << std::endl;
            if (stopping())
            {
                //std::cout << "name = " << getName() << " idle exits in thread: " << Thread::GetThreadId() << std::endl;
                break;
            }

            // blocked at epoll_wait
            int rt = 0;
            while (true)
            {
                static const uint64_t MAX_TIMEOUT = 5000;
                uint64_t next_timeout = _tfd.getNextTimer();            
                next_timeout = std::min(next_timeout, MAX_TIMEOUT); 

                rt=_epoll.wait(events, (int)next_timeout); 
                if (rt < 0 && errno == EINTR) // 所rt小于0代表无限阻塞，errno是EINTR(表示信号中断)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }; 
            LOG_DEBUG(LOG_ROOT()) << "epoll wait... n: " << rt;
            std::vector<std::function<void()>>  cbs;   
            _tfd.listcb(cbs);
            if (!cbs.empty())
            {
                for (const auto &cb : cbs)
                {
                    push(cb); // 将所有超时的回调函数添加到调度器的调度队列中。
                }
                cbs.clear();
            }

            for (int i = 0; i < rt; ++i)
            {
                epoll_event &event = events[i];

                // tickle event
                // 检查当前事件是否是 tickle 事件（即用于唤醒空闲线程的事件）。
                if (event.data.fd == _tfd.fd())
                {
                    uint8_t dummy[256];
                    // edge triggered -> exhaust
                    while (read(_tfd.fd(), dummy, sizeof(dummy)) > 0);
                    continue;
                }

                LOG_INFO(LOG_ROOT()) << "event trigger fd: " << event.data.fd << " events: " << event.events;

                FdContext *fd_ctx = (FdContext *)event.data.ptr; 
                std::lock_guard<std::mutex> lock(fd_ctx->_mutex);

                // 如果当前事件是错误或挂起（EPOLLERR 或 EPOLLHUP），则将其转换为可读或可写事件（EPOLLIN 或 EPOLLOUT），以便后续处理。
                if (event.events & (EPOLLERR | EPOLLHUP))
                {
                    event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->_event;
                }
                int real_events = NONE;
                if (event.events & EPOLLIN)
                {
                    real_events |= READ;
                }
                if (event.events & EPOLLOUT)
                {
                    real_events |= WRITE;
                }

                if ((fd_ctx->_event & real_events) == NONE)
                {
                    continue;
                }

                // 这里进行取反就是计算剩余未发送的的事件
                int left_events = (fd_ctx->_event & ~real_events);

                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events; 

                _epoll.ctlEvent(fd_ctx->_fd, &event, op); // 根据之前计算的操作（op），调用 epoll_ctl 更新或删除 epoll 监听，如果失败，打印错误并继续处理下一个事件。
                // 触发事件，事件的执行
                if (real_events & READ)
                {
                    fd_ctx->triggerEvent(READ);
                    --_pendingEventCount;
                }
                if (real_events & WRITE)
                {
                    fd_ctx->triggerEvent(WRITE);
                    --_pendingEventCount;
                }
            } 
            co_yield 0;
        }
    }
}