#include"thread.h"
#include"coroutine.hpp"
namespace  MindbniM
{
    class Schedule
    {
    public:
        using ptr=std::shared_ptr<Schedule>;
        Schedule(int threads=1,bool use_call=true,const std::string& name="Scheduler");
        void push(std::coroutine_handle<> coroutine);
        const std::string& getName() const {return _name;}
        static Schedule* GetThis();
        static void setScheduleCoroutine(std::coroutine_handle<> coroutine);
        void setThis();
        void start();
        void run();
        Schedule& operator=(Schedule&&)=delete;
    protected:
        Task<void> schedule();
    protected:
        std::mutex _mutex;
        int _threadCount;
        bool _useCall;
        std::vector<Thread::ptr> _threads;
        //就绪调度队列
        std::deque<std::coroutine_handle<>> _readyq;
        //std::priority_queue<TimeTask> _timeq;
        bool _stop=false;
        std::string _name;
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