#include"thread.h"
#include"coroutine.hpp"
namespace  MindbniM
{
    class Schedule
    {
    public:
        using ptr=std::shared_ptr<Schedule>;
        Schedule(const std::string& name="");
        void push(std::coroutine_handle<> coroutine,uint64_t thread_id=-1);
        const std::string& getName() const {return _name;}
        std::coroutine_handle<> getRoot() const {return _root._coroutine;}
        static Schedule* GetThis();
        void run();
        Schedule& operator=(Schedule&&)=delete;
    protected:
        Task<void> schedule();
    protected:
        std::mutex _mutex;
        //就绪调度队列
        std::deque<std::coroutine_handle<>> _readyq;
        //std::priority_queue<TimeTask> _timeq;
        //线程的调度协程
        Task<void> _root;
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