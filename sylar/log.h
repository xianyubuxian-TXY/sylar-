#pragma once

#include <memory>
#include <string>
namespace sylar{

//日志事件类
class LogEvent{
public:
    using ptr=std::shared_ptr<LogEvent>;
    LogEvent();
private:
    const char* m_file=nullptr; //文件名
    int32_t m_line=0; //行号
    int32_t m_elapse=0; //程序启动开始到现在的毫秒数
    int32_t m_threadId=0; //线程id
    int32_t m_fiberId=0; //协程id
    uint64_t m_time=0; //时间戳
    std::string m_content; //内容
};

//日志级别的枚举类型
enum class LogLevel{
    DEBUG=1,
    INFO=2,
    WARN=3,
    ERROR=4,
    FATAL=5
};

//日志格式类型
class LogFormatter{
public:
    using ptr=std::shared_ptr<LogFormatter>;

    std::string format(LogEvent::ptr event);
};

//日志输出地(因为输出地有多种，所以做基类)
class LogAppender{
public:
    using ptr=std::shared_ptr<LogAppender>;
    //虚函数
    virtual ~LogAppender(){}

    void log(LogLevel level,LogEvent::ptr event);
};

//日志器
class Logger{
public: 
    using ptr=std::shared_ptr<Logger>;

    Logger(const std::string& name="root");
    void log(LogLevel level,LogEvent::ptr event);
private:
    std::string m_name;
    LogLevel m_level;
    LogAppender::ptr m_logAppenderPtr;
};

//输出到控制台的Appender
class StdoutLogAppender:public LogAppender{
};

//输出到文件的Appender
class FileLogAppender:public LogAppender{
};

};