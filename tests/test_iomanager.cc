#include"iomanager.h"
#include"config.hpp"
#include"awaiter.h"
using namespace MindbniM;
Task<void> func2()
{
    LOG_DEBUG(LOG_ROOT())<<"func2 start";
    co_await SleepAwaiter(std::chrono::milliseconds(1000*3));
    LOG_DEBUG(LOG_ROOT())<<"func2 end";
}
Task<void> func1()
{
    LOG_DEBUG(LOG_ROOT())<<"func1 start";
    co_await SleepAwaiter(std::chrono::milliseconds(1000*5));
    LOG_DEBUG(LOG_ROOT())<<"func1 end";
}
int main()
{
    YAML::Node root=YAML::LoadFile("/home/mindbnim/Server-Framework/bin/conf/log.yml");
    Config::LoadFromYaml(root);
    IoManager io;
    set_hook_enable(true);
    auto t1=func1(),t2=func2();
    sleep(5);
    t1.get_coroutine().promise().set_managed_by_schedule();
    t2.get_coroutine().promise().set_managed_by_schedule();
    io.push(t2.get_coroutine());
    io.push(t1.get_coroutine());
    LOG_DEBUG(LOG_ROOT())<<"start";
    return 0;
}