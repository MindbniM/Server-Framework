#include"thread.h"
#include<concepts>
#include"coroutine.hpp"
namespace  MindbniM
{
    template<class F>
    concept TaskType=std::same_as<F, std::function<void()>> || std::same_as<F, std::coroutine_handle<Task<void>::promise_type>>;




    /**
     * @brief 任务,可以是协程和回调函数
     */
    struct TaskAndF
    {
        using CallBack=std::function<void()>;
        TaskAndF()=default;
        TaskAndF(std::coroutine_handle<Task<void>::promise_type> coroutine):_coroutine(coroutine)
        {}
        TaskAndF(CallBack cb)
        {
            _cb.swap(cb);
        }
        void clear()
        {
            _coroutine=nullptr;
            _cb=nullptr;
        }
        std::coroutine_handle<Task<void>::promise_type> _coroutine=nullptr;
        CallBack _cb=nullptr;
    };

    /**
     * @brief 协程调度器, 采用简单的先入先出调度
     */
    class Schedule
    {
    public:
        using Fiber=std::coroutine_handle<Task<void>::promise_type>;
        using ptr=std::shared_ptr<Schedule>;

        /**
         * @brief 构造函数
         * @param[in] threads 调度线程数
         * @param[in] use_call 是否将主线程参与调度
         * @param[in] name 调度器名
         */
        Schedule(int threads=1,bool use_call=true,const std::string& name="Scheduler");

        /**
         * @brief 加入任务
         * @tparam F 可以是协程和回调函数
         */
        template<TaskType F>
        void push(F task)
        {
            bool tick=false;
            {
                TaskAndF t(task);
                std::unique_lock<std::mutex> lock(_mutex);
                if(t._cb||t._coroutine)
                {
                    tick=_readyq.empty();
                    _readyq.push_front(t);
                }
            }
            if(tick) tickle();
        }

        template<class InputIterator>
        void push(InputIterator first,InputIterator last)
        {
            bool tick=false;
            {
                std::vector<TaskAndF> t;
                while(first!=last)
                {
                    if(*first)
                    t.emplace_back(*first);
                }
                std::unique_lock<std::mutex> lock(_mutex);
                tick=_readyq.empty();
                _readyq.insert(_readyq.begin(),t.begin(),t.end());
            }
            if(tick) tickle();
        }

        /**
         * @brief 获取调度器名
         */
        const std::string& getName() const {return _name;}

        /**
         * @brief 获取当前线程的调度器
         */
        static Schedule* GetThis();

        /**
         * @brief 修改当前线程的调度协程句柄
         */
        static void setScheduleCoroutine(Fiber coroutine);

        /**
         * @brief 设置当前调度器为当前线程调度器
         */
        void setThis();

        /**
         * @brief 启动调度器
         */
        void start();

        /**
         * @brief 调度
         */
        void run();

        /**
         * @brief 停止调度器, 如果启用useCall, 那么主线程开始切换到调度协程调度
         */
        void stop();

        /**
         * @brief 调度器是否停止
         */
        virtual bool stopping();
        Schedule& operator=(Schedule&&)=delete;
        virtual ~Schedule();
    protected:

        /**
         * @brief 通知其他线程有任务
         */
        virtual void tickle();

        /**
         * @brief 调度协程
         */
        Task<void> schedule();

        /**
         * @brief idle协程
         */
        virtual Task<void> idle();
    protected:
        std::mutex _mutex;                      //互斥锁
        int _threadCount;                       //线程数量
        bool _useCall;                          //main线程是否参与调度
        std::vector<Thread::ptr> _threads;      //线程池
        std::deque<TaskAndF> _readyq;           //调度队列
        bool _stop=false;                       //是否停止调度器
        std::string _name;                      //调度器名
        std::atomic<uint64_t> _idleCount={0};   //休眠线程数
        std::atomic<uint64_t> _activeCount={0}; //活跃线程数
    };


    /**
     * @brief 此等待体是为了当协程休眠时自动加入定时任务... 未完善
     */
    struct SleepAwaiter
    {
        SleepAwaiter(std::chrono::system_clock::duration time):_time(std::chrono::system_clock::now()+time)
        {
        }
        bool await_ready() const {return false;}
        void await_suspend(std::coroutine_handle<> coroutine) const
        {
            //Schedule::GetInstance().push(_time,coroutine);
        }
        void await_resume()const {}

        std::chrono::system_clock::time_point _time;
    };
}