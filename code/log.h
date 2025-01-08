#pragma once
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<fstream>
#include<sstream>
#include<map>
#include<functional>
namespace MindbniM
{
    class Logger;

    /**
     * @brief 日志级别
     */
    class LogLevel
    {
    public:
        enum Level
        {
            DEBUG=1,
            INFO=2,
            WARNING=3,
            ERROR=4,
            FATAL=5
        };
        std::string static ToString(LogLevel::Level level);
    };
    
    
    class LogEvent
    {
    public:
        using ptr=std::shared_ptr<LogEvent>;

        const char* _file = nullptr;      //文件名
        int32_t _line = 0;                //行号
        uint32_t _time=0;                 //时间
        uint32_t _threadId=0;             //线程id
        uint32_t _fiberId=0;              //协程id
        std::string _message;             //日志内容
        std::shared_ptr<Logger> _logger;  // 所属日志器
    };

      //日志格式器
     class LogFormatter
     {
     public:
        using ptr = std::shared_ptr<LogFormatter>;
        // 对给出的日志格式初始化m_items
        LogFormatter(const std::string &formatstr) : _format(formatstr) { init(); }
        // 上层总解析
        std::string format(LogLevel::Level level, LogEvent::ptr event);
        void init();
        // 日志解析方法
        class FormatItem
        {
        public:
            using ptr = std::shared_ptr<FormatItem>;
            virtual void format(std::ostringstream &os, LogLevel::Level level, LogEvent::ptr event) = 0;
            virtual ~FormatItem() {}
        };
    
      private:
        std::string _format;                 // 格式
        std::vector<FormatItem::ptr> _items; // 日志格式解析后需要的方法
        bool _error = false;                 // 日志格式是否错误
   };                              

    //日志输出地
    class LogAppender
    {
    public:
        using ptr=std::shared_ptr<LogAppender>;
        virtual void log(LogLevel::Level level,LogEvent::ptr event);
        virtual ~LogAppender();
    protected:
        LogLevel::Level _level;
        LogFormatter::ptr _format;
    };

    //日志器
    class Logger
    {
    public:
        using ptr=std::shared_ptr<Logger>;
        Logger(const std::string& name="root");
        void log(LogLevel::Level level,LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warning(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);

        LogLevel::Level getLevel() const {return _level;}
        void setLevel(LogLevel::Level level) {_level=level;}

        std::string getName() const {return _name;}
    private:
        std::string _name;
        LogLevel::Level _level;
        std::vector<LogAppender::ptr> _appenders;
    };


    //标准输出地
    class StdoutAppender : public LogAppender
    {
    public:
        using ptr=std::shared_ptr<StdoutAppender>;
        virtual void log(LogLevel::Level level,LogEvent::ptr event) override;
    };
    //文件输出地
    class FileoutAppender : public LogAppender
    {
    public:
        using ptr=std::shared_ptr<FileoutAppender>;

        FileoutAppender(const std::string& filename);
        void reopen();
        virtual void log(LogLevel::Level level,LogEvent::ptr event) override;
    private:
        std::string _filename;
        std::ofstream _file;
    };
}
