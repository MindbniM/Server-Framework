#include "schedule.h"
namespace MindbniM
{
    static thread_local Schedule* t_schedule=nullptr;
    Schedule::Schedule(const std::string& name=""):_name(name)
    {
        auto root=schedule();
        _root.swap(root);
    }
    void Schedule::push(std::coroutine_handle<> coroutine,uint64_t thread_id=-1)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _readyq.push_front(coroutine);
    }
    Task<void> Schedule::schedule()
    {
    }
    Schedule* Schedule::GetThis()
    {
        return t_schedule;
    }
    void Schedule::run()
    {
        _root.resume();
    }
}