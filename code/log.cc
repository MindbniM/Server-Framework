#include"log.h"
namespace MindbniM
{
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,const char* file, int line)
             :_logger(logger),_level(level),_file(file),_line(line)
    {
        _time=std::time(0);
        _threadId=0;
        _fiberId=0;
    }
    void LogEvent::format(const char* fmt,...)
    {
        va_list al;
        va_start(al,fmt);
        format(fmt,al);
        va_end(al);
    }
    void LogEvent::format(const char* fmt,va_list list)
    {
        char* str=nullptr;
        int len=vasprintf(&str,fmt,list);
        if(len>-1)
        {
            _ss<<std::string(str,len);
            free(str);
        }
    }
    Logger::Logger(const std::string& name,LogLevel::Level level):_name(name),_level(level)
    {
    }

    void Logger::log(LogEvent::ptr event)
    {
        if(event->_level>=_level)
        {
            for(auto& i:_appenders)
            {
                i->log(event);
            }
        }
    }
    void Logger::addAppender(LogAppender::ptr appender)
    {
        _appenders.push_back(appender);
    }
    void Logger::addAppender(const std::string& out)
    {
        if(out=="stdout")
        {
            addAppender(std::make_shared<StdoutAppender>());
        }
        else 
        {
            addAppender(std::make_shared<FileoutAppender>(out));
        }
    }
    void Logger::addAppender(const std::vector<std::string>& outs)
    {
        for(auto& s:outs)
        {
            addAppender(s);
        }
    }

    void Logger::delAppender(LogAppender::ptr appender)
    {
        std::vector<LogAppender::ptr>::iterator it=_appenders.begin();
        while(it!=_appenders.end())
        {
            if(*it==appender)
            {
                _appenders.erase(it);
                break;
            }
            it++;
        }
    }
    void Logger::clearAppender()
    {
        _appenders.clear();
    }
    LogEventWrap::~LogEventWrap()
    {
        _event->_logger->log(_event);
    }
    std::stringstream &LogEventWrap::getSS()
    {
        return _event->_ss;
    }
    LoggerManager::LoggerManager()
    {
        _root=std::make_shared<Logger>("root");
    }
    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        auto it=_loggers.find(name);
        if(it!=_loggers.end()) return it->second;
        return nullptr;
    }
    Logger::ptr LoggerManager::getRoot()
    {
        return _root;
    }
    LogAppender::LogAppender()
    {
        _format=std::make_shared<LogFormatter>();
    }
    void StdoutAppender::log(LogEvent::ptr event)
    {
        if(event->_level>=_level)
        {
            std::cout<<_format->format(event);
        }
    }
    FileoutAppender::FileoutAppender(const std::string& filename):_filename(filename),_file(filename)
    {}
    void FileoutAppender::reopen()
    {
        if(_file.is_open())
        {
            _file.close();
        }
        _file.open(_filename);
    }
    void FileoutAppender::log(LogEvent::ptr event)
    {
        if(event->_level>=_level)
        {
            _file<<_format->format(event);
        }
    }
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        virtual void format(std::ostringstream &os,  LogEvent::ptr event) override
        {
            os << event->_ss.str();
        }
    };

    std::string LogLevel::ToString(LogLevel::Level level)
    {
        switch(level)
        {
    #define XX(name) \
            case Level::name : \
            return #name; break; \

        XX(DEBUG)
        XX(INFO)
        XX(WARNING)
        XX(ERROR)
        XX(FATAL)
    #undef XX
        }
        return "UNKONW";
    }
    LogLevel::Level LogLevel::FromString(const std::string& str)
    {
        std::string le=str;
        std::transform(le.begin(),le.end(),le.begin(),::tolower);
    #define XX(str,level) if(le==str) return LogLevel::Level(LogLevel::Level::level);
        XX("info",INFO)
        XX("debug",DEBUG)
        XX("warning",WARNING)
        XX("error",ERROR)
        XX("fatal",FATAL)
    #undef XX
    }
    void LevelFormatItem::format(std::ostringstream &os,  LogEvent::ptr event) 
    {
        os << LogLevel::ToString(event->_level);
    }
    void LineFormatItem::format(std::ostringstream &os,  LogEvent::ptr event)
    {
        os << event->_line;
    }
    void FilenameFormatItem::format(std::ostringstream &os, LogEvent::ptr event) 
    {
        os << event->_file;
    }

    void ThreadIdFormatItem::format(std::ostringstream &os, LogEvent::ptr event)
    {
        os << event->_threadId;
    }
    void FiberIdFormatItem::format(std::ostringstream &os,  LogEvent::ptr event)
    {
        os << event->_fiberId;
    }
    void NameFormatItem::format(std::ostringstream &os,  LogEvent::ptr event)
    {
        os << event->_logger->getName();
    }
    void DateFormatItem::format(std::ostringstream &os, LogEvent::ptr event)
    {
        struct tm tm;
        time_t time = event->_time;
        localtime_r(&time, &tm);
        std::string str;
        for (int i = 0; i < _format.size(); ++i)
        {
            if (_format[i] != '%')
                str.push_back(_format[i]);
            else if (i + 1 < _format.size())
            {
                switch (_format[i + 1])
                {
                case 'Y':
                    str += std::to_string(tm.tm_year + 1900);
                    break;
                case 'm':
                    str += (tm.tm_mon + 1 < 10 ? "0" : "") + std::to_string(tm.tm_mon + 1);
                    break;
                case 'd':
                    str += (tm.tm_mday < 10 ? "0" : "") + std::to_string(tm.tm_mday);
                    break;
                case 'H':
                    str += (tm.tm_hour < 10 ? "0" : "") + std::to_string(tm.tm_hour);
                    break;
                case 'M':
                    str += (tm.tm_min < 10 ? "0" : "") + std::to_string(tm.tm_min);
                    break;
                case 'S':
                    str += (tm.tm_sec < 10 ? "0" : "") + std::to_string(tm.tm_sec);
                    break;
                default:
                    str.push_back('%');
                    str.push_back(_format[i + 1]);
                    break;
                }
                ++i;
            }
        }
        os << str;
    }
    void NewLineFormatItem::format(std::ostringstream &os,  LogEvent::ptr event)
    {
        os << std::endl;
    }
    void TabFormatItem::format(std::ostringstream &os,  LogEvent::ptr event)
    {
        os << "\t";
    }
    void StringFormatItem::format(std::ostringstream &os, LogEvent::ptr event)
    {
        os << _str;
    }
    std::string LogFormatter::format(LogEvent::ptr event)
    {
        std::ostringstream os;
        for (auto &fmat : _items)
        {
            fmat->format(os,event);
        }
        return os.str();
    }
    void LogFormatter::init()
    {
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (size_t i = 0; i < _format.size(); ++i)
        {
            if (_format[i] != '%')
            {
                nstr.append(1, _format[i]);
                continue;
            }

            if ((i + 1) < _format.size())
            {
                if (_format[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }

                size_t n = i + 1;
                int fmt_status = 0;
                size_t fmt_begin = 0;

                std::string str;
                std::string fmt;

                while (n < _format.size())
                {
                    if (!fmt_status && (!isalpha(_format[n]) && _format[n] != '{' && _format[n] != '}'))
                    {
                        str = _format.substr(i + 1, n - i - 1);
                        break;
                    }
                    if (fmt_status == 0)
                    { // 开始解析时间格式
                        if (_format[n] == '{')
                        {
                            str = _format.substr(i + 1, n - i - 1); // str = "d"
                            fmt_status = 1;
                            fmt_begin = n;
                            ++n;
                            continue;
                        }
                    }
                    else if (fmt_status == 1)
                    { // 结束解析时间格式
                        if (_format[n] == '}')
                        {
                            // fmt = %Y-%m-%d %H:%M:%S
                            fmt = _format.substr(fmt_begin + 1, n - fmt_begin - 1);
                            fmt_status = 0;
                            ++n;
                            break;
                        }
                    }
                    ++n;
                    if (n == _format.size())
                    { // 最后一个字符
                        if (str.empty())
                        {
                            str = _format.substr(i + 1);
                        }
                    }
                }
                if (fmt_status == 0)
                {
                    if (!nstr.empty())
                    {
                        vec.push_back(std::make_tuple(nstr, std::string(), 0)); // 将[ ]放入， type为0
                        nstr.clear();
                    }
                    vec.push_back(std::make_tuple(str, fmt, 1)); //(e.g.) ("d", %Y-%m-%d %H:%M:%S, 1) type为1
                    i = n - 1;                                   // 跳过已解析的字符，让i指向当前处理的字符，下个for循环会++i处理下个字符
                }
                else if (fmt_status == 1)
                {
                    std::cout << "Pattern parde error: " << _format << " - " << _format.substr(i) << std::endl;
                    _error = true;
                    vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
                }
            }
        }

        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0)); //(e.g.) 最后一个字符为[ ] :
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string &fmt)>> s_format_items = {
#define XX(str, C) \
    {#str, [](const std::string &fmt) { return FormatItem::ptr(new C(fmt)); }}

            XX(m, MessageFormatItem),  // m:消息
            XX(p, LevelFormatItem),    // p:日志级别
            XX(c, NameFormatItem),     // c:日志名称
            XX(t, ThreadIdFormatItem), // t:线程id
            XX(n, NewLineFormatItem),  // n:换行
            XX(d, DateFormatItem),     // d:时间
            XX(f, FilenameFormatItem), // f:文件名
            XX(l, LineFormatItem),     // l:行号
            XX(T, TabFormatItem),      // T:Tab
            XX(F, FiberIdFormatItem),  // F:协程id

#undef XX
        };

        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0)
            {
                _items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    _items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    _error = true;
                }
                else
                {
                    _items.push_back(it->second(std::get<1>(i)));
                }
            }
            // std::cout<<std::get<0>(i)<<" "<<std::get<1>(i)<<" "<<std::get<2>(i)<<std::endl;
        }
    }

}
