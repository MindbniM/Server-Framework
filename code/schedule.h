#include"thread.h"
#include"coroutine.hpp"
namespace  MindbniM
{
    class Schedule
    {
    public:
        using Fiber=std::coroutine_handle<Task<void>::promise_type>;
        using ptr=std::shared_ptr<Schedule>;
        struct TaskAndF
        {
            using CallBack=std::function<void()>;
            TaskAndF()=default;
            TaskAndF(Fiber coroutine):_coroutine(coroutine)
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
            Fiber _coroutine=nullptr;
            CallBack _cb=nullptr;
        };
        Schedule(int threads=1,bool use_call=true,const std::string& name="Scheduler");
        template<class F>
        void push(F task)
        {
            bool tick=false;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                TaskAndF t(task);
                if(t._cb||t._coroutine)
                {
                    tick=_readyq.empty();
                    _readyq.push_front(t);
                }
            }
            if(tick) tickle();
        }
        const std::string& getName() const {return _name;}
        static Schedule* GetThis();
        static void setScheduleCoroutine(Fiber coroutine);
        void setThis();
        void start();
        void run();
        void stop();
        bool stopping();
        Schedule& operator=(Schedule&&)=delete;
        virtual ~Schedule();
    protected:
        virtual void tickle();
        Task<void> schedule();
        Task<void> idle();
    protected:
        std::mutex _mutex;
        int _threadCount;
        bool _useCall;                      //main线程是否参与调度
        std::vector<Thread::ptr> _threads;
        std::deque<TaskAndF> _readyq;
        bool _stop=false;
        std::string _name;
        std::atomic<uint64_t> _idleCount={0};
        std::atomic<uint64_t> _activeCount={0};
    };


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