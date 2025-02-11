#pragma once
#include "schedule.h"
#include "timer.h"
#include "epoll.h"
namespace MindbniM
{
    /**
     * @brief 事件类型
     */
    enum Event
    {
        NONE = 0,
        READ = EPOLLIN,
        WRITE = EPOLLOUT
    };

    /**
     * @brief 事件内容
     */
    struct EventContext
    {
        void clear()
        {
            _task.clear();
            _root = nullptr;
        }
        TaskAndF _task;  // 任务
        Schedule *_root; // 关联的调度器
        bool _recurring = false;//是否循环监控
    };

    /**
     * @brief 文件描述符关联事件
     */
    struct FdContext
    {
        FdContext()=default;
        FdContext(int fd,Event event);

        /**
         * @brief 清理资源
         */
        ~FdContext();

        /**
         * @brief 获取一个事件的内容
         */
        EventContext &getContext(Event ev);

        /**
         * @brief 触发一个事件,使其添加到调度器
         */
        void triggerEvent(Event event);

        /**
         * @brief 清空所有事件内容
         */
        void clear();

        int _fd=-1;                 //文件描述符
        std::mutex _mutex;          //互斥锁
        EventContext _read;         //读事件
        EventContext _write;        //写事件
        Event _event = Event::NONE; //事件类型
    };
    class IoManager : public Schedule
    {
    public:
        /**
         * @brief 构造方法
         * @param[in] threads 工作线程数
         * @param[in] use_call main线程是否参与调度
         * @param[in] name 调度器名称
         * @param[in] auto_close 是否自动关闭
         */
        IoManager(int threads=1,bool use_call=true,const std::string& name="Scheduler",bool auto_close=false);
        
        /**
         * @brief 调用stop停止调度器
         */
        ~IoManager();

        /**
         * @brief 扩容
         */
        void contextResize(size_t size);

        /**
         * @brief 新增一个文件描述符事件
         * @tparam F 可以是协程和回调类型
         * @param[in] fd 文件描述符
         * @param[in] event 事件类型
         * @param[in] cb 执行的协程或回调
         */
        template<TaskType F>
        bool addEvent(int fd,Event event,F cb,bool recurring=false);

        /**
        * @brief 删除事件
        * @attention 不会触发事件
        */
        bool delEvent(int fd,Event event);

        /**
         * @brief 取消事件
        * @attention 会触发事件
         */
        bool cancelEvent(int fd,Event event);

        /**
         * @brief 取消一个文件描述符上的所有事件
        * @attention 会触发事件
         */
        bool cancelAll(int fd);

        /**
         * @brief 提前唤醒在idle协程处epoll_wait的线程, 提示有任务
         */
        virtual void tickle() override;

        /**
         * @brief 是否停止
         */
        virtual bool stopping() override;

        /**
         * @brief idle协程, 没有任务时执行
         */
        virtual Task<void> idle() override;

        /**
         * @brief 获取当前的调度器
         */
        static IoManager* GetThis();

        TimerFd& getTimerManager() {return _tfd;}

    private:
        Epoll _epoll;                                       //epoll句柄
        std::shared_mutex _mutex;                           //读写锁
        std::atomic<int> _pendingEventCount={0};            //当前等待执行的任务数量
        std::vector<FdContext*> _fdcontexts;            //Fd事件储存
        TimerFd _tfd;                                       //定时器
    };

    //template<TaskType F>
    //bool IoManager::addEvent(int fd,Event event,F cb)
    //{
    //    FdContext::ptr p=nullptr;
    //    std::shared_lock<std::shared_mutex> rlock(_mutex);
    //    if((int)_fdcontexts.size()>fd)
    //    {
    //        p=_fdcontexts[fd];
    //        rlock.unlock();
    //    }
    //    else 
    //    {
    //        rlock.unlock();
    //        std::unique_lock<std::shared_mutex> wlock(_mutex);
    //        contextResize(fd * 1.5);
    //        p=_fdcontexts[fd];
    //    }
    //    //if(p->_event&event)
    //    //{
    //    //    LOG_INFO(LOG_ROOT())<<fd<<" 事件重复 "<<p->_event<<" "<<event;
    //    //    return false;
    //    //}
    //    LOG_INFO(LOG_ROOT())<<"add "<<fd<<" "<<event<<"("<<EPOLLIN<<" "<<EPOLLOUT<<") ";
    //    _epoll.ctlEvent(fd,EPOLLET|(int)event|p->_event,EPOLL_CTL_ADD);
    //    _pendingEventCount++;
    //    int _ev_=p->_event;
    //    _ev_|=(int)event;
    //    p->_event=(Event)_ev_;
    //    //LOG_DEBUG(LOG_ROOT())<<_name<<" add event fd:"<<fd<<" "<<p->_event;
    //    EventContext& ev=p->getContext(event);
    //    ev._root=Schedule::GetThis();
    //    ev._task=TaskAndF(cb);
    //    return true;
    //}
    template<TaskType F>
    bool IoManager::addEvent(int fd,Event event,F cb,bool recurring)
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
            std::unique_lock<std::shared_mutex> write_lock(_mutex);
            contextResize(fd * 1.5);
            fd_ctx = _fdcontexts[fd];
        }
        std::lock_guard<std::mutex> lock(fd_ctx->_mutex);
 
        if(fd_ctx->_event & event)
        {
            return false;
        }
 
        LOG_INFO(LOG_ROOT()) << "add " << fd << " " << event << "(" << EPOLLIN << " " << EPOLLOUT << ")";
        int op = fd_ctx->_event ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.events   = EPOLLET | fd_ctx->_event | event;
        epevent.data.ptr = fd_ctx;

        _epoll.ctlEvent(fd,&epevent,op);
        ++_pendingEventCount;

        fd_ctx->_event = (Event)(fd_ctx->_event | event);
 
        EventContext& event_ctx = fd_ctx->getContext(event);
        event_ctx._root = Schedule::GetThis();
        event_ctx._task = TaskAndF(cb);
        event_ctx._recurring = recurring;
        return 0;
    }
 

}