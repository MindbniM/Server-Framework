#pragma once
#include"log.h"
#include<chrono>
#include<shared_mutex>
#include<unistd.h>
#include"util.h"
namespace MindbniM
{
    class TimerManager;

    /**
     * @brief 定时任务
     */
    struct Timer : public std::enable_shared_from_this<Timer>
    {
        using ptr=std::shared_ptr<Timer>;

        /**
         * @brief 构造函数
         * @param[in] ms 间隔时间
         * @param[in] cb 到时回调函数
         * @param[in] recurring 是否循环
         * @param[in] manager 所属定时器
         */
        Timer(std::chrono::milliseconds ms,std::function<void()> cb,bool recurring,TimerManager* manager);

        /**
         * @brief 从对于的定时器中删除自己
         */
        bool cancel();

        /**
         * @brief 更新任务的触发时间, 从调用开始计算
         */
        bool refresh();

        /**
         * @brief 清理资源
         */
        void clear();

        /**
         * @brief 调整任务
         * @param[in] ms 新的间隔时间
         * @param[in] from_now 是否从现在开始计算
         */
        bool reset(std::chrono::milliseconds ms,bool from_now);

        uint64_t _id;                                   //定时任务id
        bool _recurring;                                //是否是循环任务
        std::chrono::milliseconds _ms;                  //间隔时间
        std::chrono::steady_clock::time_point _timeOut; //触发时间, 使用单调时钟, 避免系统时钟意外错误
        std::function<void()> _cb;                      //回调函数
        TimerManager* _manager=nullptr;                 //所属的定时器
    };

    /**
     * @brief 用于set比较
     */
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
    
    /**
     * @brief 定时器
     */
    class TimerManager
    {
    public:

        /**
         * @brief 添加定时任务
         */
        Timer::ptr addTimer(std::chrono::milliseconds ms,std::function<void()> cb,bool recurring=false);
        void addTimer(Timer::ptr timer);

        /**
         * @brief 删除定时任务
         */
        bool erase(Timer::ptr timer);

        /**
         * @brief 条件任务封装
         */
        static void onTimer(std::function<void()> cb,std::weak_ptr<void> weak_cond);

        /**
         * @brief 返回距离最近任务的时间
         */
        uint64_t getNextTimer();

        /**
         * @brief 判断一个任务是否存在
         */
        bool count(Timer::ptr);

        /**
         * @brief 获取超时任务回调列表
         * @param[out] cbs 任务列表
         */
        void listcb(std::vector<std::function<void()>>& cbs);

        /**
         * @brief 添加条件定时任务
         */
        Timer::ptr addConditionTimer(std::chrono::milliseconds ms,std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring=false);

        /**
         * @brief 当一个被插入的任务是最小的, 应该对某些关注最短时间的进行调整
         */
        virtual void onTimerInsertedAtFront() = 0;

    protected:
        std::shared_mutex _mutex;                   //读写锁
        std::set<Timer::ptr,__TimeComp__> _timers;  //任务集合
    };

    /**
     * @brief 联动epoll
     */
    class TimerFd : public TimerManager
    {
    public:
        TimerFd();

        /**
         * @brief 返回读fd
         */
        int fd() const {return _readfd;}

        /**
         * @brief 写数据触发epoll
         */
        virtual void onTimerInsertedAtFront() override;
    private:
        int _readfd;
        int _writefd;
    };
}