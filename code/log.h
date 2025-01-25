#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <functional>
#include <ctime>
#include <cstdarg>
#include "singleton.h"
#include "yaml-cpp/yaml.h"
#include <mutex>
#include "spinlock.h"
namespace MindbniM
{
/**
 * @brief 以流式方式将日志等级level的日志写入到logger
 */
#define LOG_LEVEL(logger, level)     \
    if (logger->getLevel() <= level)    MindbniM::LogEventWrap(std::make_shared<LogEvent>(logger, level, __FILE__, __LINE__)).getSS()

/**
 * @brief 以流式方式将日志等级debug的日志写入到logger
 */
#define LOG_DEBUG(logger) LOG_LEVEL(logger, LogLevel::Level::DEBUG)

/**
 * @brief 以流式方式将日志等级info的日志写入到logger
 */
#define LOG_INFO(logger) LOG_LEVEL(logger, LogLevel::Level::INFO)

/**
 * @brief 以流式方式将日志等级warning的日志写入到logger
 */
#define LOG_WARNING(logger) LOG_LEVEL(logger, LogLevel::Level::WARNING)

/**
 * @brief 以流式方式将日志等级error的日志写入到logger
 */
#define LOG_ERROR(logger) LOG_LEVEL(logger, LogLevel::Level::ERROR)

/**
 * @brief 以流式方式将日志等级fatal的日志写入到logger
 */
#define LOG_FATAL(logger) LOG_LEVEL(logger, LogLevel::Level::FATAL)

/**
 * @brief 以格式化方式将日志等级level的日志写入到logger
 */
#define LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if (level >= logger->getLevel())           \
    MindbniM::LogEventWrap(std::make_shared<LogEvent>(logger, level, __FILE__, __LINE__)).getEvent()->format(fmt, ##__VA_ARGS__)

/**
 * @brief 以格式化方式将日志等级debug的日志写入到logger
 */
#define LOG_FMT_DEBUG(logger,fmt,...) LOG_FMT_LEVEL(logger,LogLevel::Level::DEBUG,fmt,##__VA_ARGS__)

/**
 * @brief 以格式化方式将日志等级info的日志写入到logger
 */
#define LOG_FMT_INFO(logger,fmt,...) LOG_FMT_LEVEL(logger,LogLevel::Level::INFO,fmt,##__VA_ARGS__)

/**
 * @brief 以格式化方式将日志等级warning的日志写入到logger
 */
#define LOG_FMT_WARNING(logger,fmt,...) LOG_FMT_LEVEL(logger,LogLevel::Level::WARNING,fmt,##__VA_ARGS__)

/**
 * @brief 以格式化方式将日志等级error的日志写入到logger
 */
#define LOG_FMT_ERROR(logger,fmt,...) LOG_FMT_LEVEL(logger,LogLevel::Level::ERROR,fmt,##__VA_ARGS__)

/**
 * @brief 以格式化方式将日志等级fatal的日志写入到logger
 */
#define LOG_FMT_FATAL(logger,fmt,...) LOG_FMT_LEVEL(logger,LogLevel::Level::FATAL,fmt,##__VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define LOG_ROOT() MindbniM::LoggerMgr::GetInstance()->getRoot()

/**
 * @brief 获取日志器
 */
#define LOG_NAME(name) MindbniM::LoggerMgr::GetInstance()->getLogger(name)


    /**
     * @brief 日志级别
     */
    class LogLevel
    {
    public:
        enum Level
        {
            DEBUG = 1,
            INFO = 2,
            WARNING = 3,
            ERROR = 4,
            FATAL = 5
        };
        /**
         * @brief 日志级别转换字符串
         */
        std::string static ToString(LogLevel::Level level);

        /**
         * @brief 字符串 -> 日志级别转
         */
        LogLevel::Level static FromString(const std::string& str);
    };

    class Logger;

    /**
     * @brief 日志事件
     */
    class LogEvent
    {
    public:
        using ptr = std::shared_ptr<LogEvent>;
        /**
         * @brief 初始化日志事件
         * @note
         * - 后续应添加关于线程id获取和协程id获取的回调函数
         */
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int line);
        LogEvent() {};

        /**
         * @brief 格式化到日志流
         */
        void format(const char *fmt, ...);
        void format(const char *fmt, va_list list);

        const char *_file = nullptr;     // 文件名
        int32_t _line = 0;               // 行号
        uint32_t _time = 0;              // 时间
        uint32_t _threadId = 0;          // 线程id
        uint32_t _fiberId = 0;           // 协程id
        LogLevel::Level _level;          // 日志等级
        std::stringstream _ss;           // 日志内容流
        std::shared_ptr<Logger> _logger; // 所属日志器
    };
    /**
     * @brief 日志事件包装器
     * 通过临时对象销毁输出日志
     */
    class LogEventWrap
    {
    public:
        /**
         * @brief 构造函数
         * @param[in] e 日志事件
         */
        LogEventWrap(LogEvent::ptr e) : _event(e)
        {
        }

        /**
         * @brief 析构函数
         * 析构时自动将日志输出
         */
        ~LogEventWrap();
        /**
         * @brief 获取日志事件
         */
        LogEvent::ptr getEvent() const { return _event; }

        /**
         * @brief 获取日志内容流引用
         */
        std::stringstream &getSS();

    private:
        LogEvent::ptr _event; // 日志事件
    };

    /**
     * @brief 默认日志格式
     */
    const std::string DEFAULT_FORMAT = "[%p][%d{%Y-%m-%d %H:%M:%S}][%f:%l]%m%n";

    /**
     * @brief 日志格式器
     *
     *  格式："%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     *  %m 消息
     *  %p 日志级别
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     * 默认格式     [%p][%d{%Y-%m-%d %H:%M:%S}][%f : %l]%m%n;
     *
     */
    class LogFormatter
    {
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        /**
         * @brief 对给出的日志格式初始化_items
         */
        LogFormatter(const std::string &formatstr = DEFAULT_FORMAT) : _format(formatstr) { init(); }
        /**
         * @brief 上层总解析
         */
        std::string format(LogEvent::ptr event);
        /**
         * @brief 日志解析方法
         */
        void init();

        std::string getFormat()const{return _format;}

        /**
         * @brief 日志内容项格式化
         */
        class FormatItem
        {
        public:
            using ptr = std::shared_ptr<FormatItem>;
            virtual void format(std::ostringstream &os, LogEvent::ptr event) = 0;
            virtual ~FormatItem() {}
        };

    private:
        std::string _format;                 // 格式
        std::vector<FormatItem::ptr> _items; // 日志格式解析后需要的方法
        bool _error = false;                 // 日志格式是否错误
    };

    /**
     * @brief 日志等级格式化
     */
    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 行号格式化
     */
    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 文件名格式化
     */
    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 线程ID格式化
     */
    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 协程ID格式化
     */
    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 所属日志器名格式化
     */
    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 日期格式化
     */
    class DateFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateFormatItem(const std::string format = "%Y-%m-%d %H:%M:%S") : _format(format) {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;

    private:
        std::string _format;
    };

    /**
     * @brief 空行格式化
     */
    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 制表符格式化
     */
    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        TabFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;
    };

    /**
     * @brief 日志内容格式化
     */
    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str = "") : _str(str) {}
        virtual void format(std::ostringstream &os, LogEvent::ptr event) override;

    private:
        std::string _str;
    };

    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
    public:
        using ptr = std::shared_ptr<LogAppender>;

        /**
         * @brief 默认初始化格式和最低日志等级
         */
        LogAppender();

        /**
         * @brief 写入日志
         */
        virtual void log(LogEvent::ptr event) = 0;

        /**
         * @brief 获取日志格式
         */
        LogFormatter::ptr getFormat();

        /**
         * @brief 修改日志格式
         */
        void setFormat(LogFormatter::ptr format) ;

        /**
         * @brief 获取日志输出地的名称
         */
        virtual std::string getout()=0;

        virtual ~LogAppender() {}

        /**
         * @brief 从YAML中读取配置
         */
        bool FromYaml(const YAML::Node& root);

        /**
         * @brief 格式化到YAML
         */
        bool ToYaml(YAML::Node& root);
    protected:
        LogLevel::Level _level;
        LogFormatter::ptr _format;
        Spinlock _mutex;
    };

    /**
     * @brief 日志器
     */
    class Logger
    {
    public:
        using ptr = std::shared_ptr<Logger>;

        Logger(const std::string &name = "root", LogLevel::Level level = LogLevel::Level::DEBUG);

        /**
         * @brief 向所有日志输出地输出日志
         */
        void log(LogEvent::ptr event);

        /**
         * @brief 添加一个日志输出地
         */
        void addAppender(LogAppender::ptr appender);

        /**
         * @brief 添加一个日志输出地
         * @param[in] out 输出地名称[stdout/filename]
         */
        void addAppender(const std::string& out);

        /**
         * @brief 添加一个日志输出地
         * @param[in] out 输出地名称[stdout/filename]
         * @param[in] fomatter 日志格式
         */
        void addAppender(const std::string& out,const std::string& formatter);

        /**
         * @brief 添加多个日志输出地
         */
        void addAppender(const std::vector<std::string>& outs);

        /**
         * @brief 获取一个日志输出地, 可能进行修改
         */
        LogAppender::ptr getAppender(const std::string& out);

        /**
         * @brief 删除一个日志输出地
         */
        void delAppender(LogAppender::ptr appender);

        /**
         * @brief 删除所有日志输出地
         */
        void clearAppender();

        LogLevel::Level getLevel() const { return _level; }
        void setLevel(LogLevel::Level level) { _level = level; }

        std::string getName() const { return _name; }

        /**
         * @brief 从YAML中读取配置
         */
        bool FromYaml(const YAML::Node& root);

        /**
         * @brief 格式化到YAML
         */
        bool ToYaml(YAML::Node& root);
    private:
        std::string _name;                                              //日志器名称
        LogLevel::Level _level;                                         //日志最低等级
        std::unordered_map<std::string,LogAppender::ptr> _appenders;    //日志输出地集合
        Spinlock _mutex;
    };

    /**
     * @brief 日志器管理器
     */
    class LoggerManager
    {
    public:
        LoggerManager();
        
        /**
         * @brief 获取指定日志器, 如果不存在就创建
         */
        Logger::ptr getLogger(const std::string &name);

        /**
         * @brief 获取root日志器
         */
        Logger::ptr getRoot();

        /**
         * @brief 从YAML中读取配置
         */
        bool FromYaml(const YAML::Node& root);

        /**
         * @brief 格式化到YAML
         */
        bool ToYaml(YAML::Node& root);

    private:
        Logger::ptr _root;                                          //root日志器
        std::unordered_map<std::string, Logger::ptr> _loggers;      //日志器集合
        Spinlock _mutex;
    };

    /**
     * @brief 标准输出
     */
    class StdoutAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<StdoutAppender>;
        virtual void log(LogEvent::ptr event) override;
        virtual std::string getout() override;
    };

    /**
     * @brief 文件输出
     */
    class FileoutAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<FileoutAppender>;

        FileoutAppender(const std::string &filename);

        /**
         * @brief 重新打开文件
         */
        void reopen();
        virtual void log(LogEvent::ptr event) override;
        virtual std::string getout() override;

    private:
        std::string _filename;
        std::ofstream _file;
    };

    using LoggerMgr=Singleton<LoggerManager>;


}
