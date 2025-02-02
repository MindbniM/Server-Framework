#pragma once
#include "log.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <optional>
#include "util.h"

namespace MindbniM
{
    class Epoll
    {
    public:
        explicit Epoll(int maxEvents = 1024)
            : _epollFd(epoll_create1(0)), _maxEvents(maxEvents)
        {
            ASSERT(_epollFd < 0, "system", "Failed to create epoll instance", std::runtime_error)
        }

        ~Epoll()
        {
            if (_epollFd >= 0)
            {
                close(_epollFd);
            }
        }

        void addEvent(int fd, uint32_t events)
        {
            ctlEvent(fd, events, EPOLL_CTL_ADD);
        }

        void modEvent(int fd, uint32_t events)
        {
            ctlEvent(fd, events, EPOLL_CTL_MOD);
        }

        void delEvent(int fd)
        {
            ctlEvent(fd, 0, EPOLL_CTL_DEL);
        }

        int wait(std::vector<epoll_event>& events,int timeoutMs = -1)
        {
            events.resize(_maxEvents);
            int n = epoll_wait(_epollFd, events.data(), _maxEvents, timeoutMs);
            ASSERT(n < 0, "system", "epoll_wait failed", std::runtime_error)
            return n;
        }
        void ctlEvent(int fd, uint32_t events, int operation)
        {
            struct epoll_event ev = {};
            ev.events = events;
            ev.data.fd = fd;
            int n = epoll_ctl(_epollFd, operation, fd, &ev);
            ASSERT(n < 0, "root", std::to_string(fd)+" "+strerror(errno), std::runtime_error)
        }

        int _epollFd;
        int _maxEvents;
    };
}
