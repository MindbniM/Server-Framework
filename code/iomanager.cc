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
            LOG_WARNING(LOG_NAME("system")) << "unknow event";
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
        }
        if (ev._task._coroutine)
        {
            ev._root->push(ev._task._coroutine);
        }
    }
    FdContext::FdContext(int fd, Event event) : _fd(fd), _event(event)
    {
    }
    FdContext::~FdContext()
    {
        clear();
        if (_fd > 0)
        {
            ::close(_fd);
        }
    }
    IoManager::IoManager(int threads, bool use_call, const std::string &name,bool auto_close)
        : Schedule(threads, use_call, name,auto_close)
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
                _fdcontexts[i] = std::make_shared<FdContext>();
                _fdcontexts[i]->_fd = i;
            }
        }
    }
    bool IoManager::delEvent(int fd, Event event)
    {
        FdContext::ptr p = nullptr;
        std::shared_lock<std::shared_mutex> rlock(_mutex);
        if ((int)_fdcontexts.size()<fd)
            return false;
        p = _fdcontexts[fd];
        rlock.unlock();
        if (!(p->_event & event))
            return false;
        Event nevent = (Event)((p->_event) & (~event));
        int op = nevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        _epoll.ctlEvent(fd, EPOLLET | (int)event | EPOLLEXCLUSIVE, op);
        --_pendingEventCount;
        p->_event = nevent;
        p->getContext(event).clear();
        return true;
    }
    bool IoManager::cencelEvent(int fd, Event event)
    {
        FdContext::ptr p = nullptr;
        std::shared_lock<std::shared_mutex> rlock(_mutex);
        if ((int)_fdcontexts.size()<fd)
            return false;
        p = _fdcontexts[fd];
        rlock.unlock();
        if (!(p->_event & event))
            return false;
        Event nevent = (Event)((p->_event) & (~event));
        int op = nevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        _epoll.ctlEvent(fd, EPOLLET | (int)event | EPOLLEXCLUSIVE, op);
        --_pendingEventCount;
        p->triggerEvent(event);
        return true;
    }
    bool IoManager::cencelAll(int fd)
    {
        FdContext::ptr p = nullptr;
        std::shared_lock<std::shared_mutex> rlock(_mutex);
        if ((int)_fdcontexts.size()<fd)
            return false;
        p = _fdcontexts[fd];
        rlock.unlock();
        if (!p->_event)
            return false;
        _epoll.delEvent(fd);
        if (p->_event & READ)
        {
            p->triggerEvent(READ);
            --_pendingEventCount;
        }
        if (p->_event & WRITE)
        {
            p->triggerEvent(WRITE);
            --_pendingEventCount;
        }
        return true;
    }
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
        // std::cout<<"idle to epoll_wait"<<std::endl;
        //LOG_DEBUG(LOG_ROOT()) << "idle wait";
        std::vector<epoll_event> events;
        while (1)
        {
            LOG_DEBUG(LOG_ROOT()) << "IoManager::idle thread is " << Thread::GetName();
            if (stopping())
            {
                LOG_DEBUG(LOG_ROOT()) << "IoManager is exits " << Thread::GetName();
                co_return;
            }
            int n = 0;
            while (1)
            {
                static uint64_t MaxTimeOut = 5000;
                uint64_t next = _tfd.getNextTimer();
                next = std::min(next, MaxTimeOut);
                n = _epoll.wait(events, next);
                if (n < 0 && errno == EINTR)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            std::vector<std::function<void()>> cbs;
            _tfd.listcb(cbs);
            if (!cbs.empty())
            {
                push(cbs.begin(), cbs.end());
                cbs.clear();
            }
            for (int i = 0; i < n; i++)
            {
                epoll_event &ev = events[i];
                if (ev.data.fd == _tfd.fd())
                {
                    char temp[256];
                    while (read(_tfd.fd(), temp, sizeof(temp)) > 0) ;
                    continue;
                }
                std::shared_lock<std::shared_mutex> rlock(_mutex);
                FdContext::ptr p = _fdcontexts[ev.data.fd];
                rlock.unlock();
                if (ev.events & (EPOLLERR | EPOLLHUP))
                {
                    ev.events |= (EPOLLIN | EPOLLOUT) & p->_event;
                }
                int revent = 0;
                if (ev.events & EPOLLIN)
                {
                    revent |= READ;
                }
                if (ev.events & EPOLLOUT)
                {
                    revent |= WRITE;
                }
                if ((p->_event & revent) == NONE)
                {
                    continue;
                }
                int mevent = p->_event & (~revent);
                int op = mevent ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                _epoll.ctlEvent(ev.data.fd, mevent | EPOLLET | EPOLLEXCLUSIVE, op);
                if (mevent & READ)
                {
                    p->triggerEvent(READ);
                    --_pendingEventCount;
                }
                if (mevent & WRITE)
                {
                    p->triggerEvent(WRITE);
                    --_pendingEventCount;
                }
            }
            co_yield 0;
        }
    }
}