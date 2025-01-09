#include"log.h"
using namespace MindbniM;
int main()
{
    LoggerManager manger;
    manger.InitRoot();
    LogAppender::ptr p=std::make_shared<StdoutAppender>();
    auto root=manger.getRoot();
    root->addAppender(p);
    LogEventWrap(LogEvent::ptr(new LogEvent(root, LogLevel::Level::DEBUG,__FILE__, __LINE__, 0, 0,time(0)))).getSS()<<"cnm";
    return 0;
}