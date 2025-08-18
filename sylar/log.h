#pragma once
#include<memory>
#include<string>
#include<vector>
#include<list>
#include<map>
#include<sstream>
#include<fstream>
#include<time.h>
#include<thread>
#include "util.h"
#include "singleton.h"

/*
用法：
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    SYLAR_LOG_INFO(logger)<<"test macro";
*/
#define SYLAR_LOG_LEVEL(logger,level)\
    if(logger->getLevel()<=level) \
        sylar::LogEventWarp(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level,__FILE__,__LINE__,0,sylar::getThreadId(),\
                                    sylar::getFiberId(),time(0)))).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger,sylar::LogLevel::FATAL)


/*
用法：
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    SYLAR_LOG_FMT_ERROR(logger,"test macro fmt error %s","aa");
*/
#define SYLAR_LOG_FMT_LEVEL(logger,level,fmt,...)\
    if(logger->getLevel()<=level)\
        sylar::LogEventWarp(sylar::LogEvent::ptr(new sylar::LogEvent(logger,level, \
                            __FILE__,__LINE__,0,sylar::getThreadId(),\
                            sylar::getFiberId(),time(0)))).getEvent()->format(fmt,__VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::DEBUG,fmt,__VA__ARGS__)
#define SYLAR_LOG_FMT_INFO(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::INFO,fmt,__VA__ARGS__)
#define SYLAR_LOG_FMT_WARN(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::WARN,fmt,__VA__ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::ERROR,fmt,__VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger,fmt,...) SYLAR_LOG_FMT_LEVEL(logger,sylar::LogLevel::FATAL,fmt,__VA__ARGS__)

namespace sylar{
class Logger;

//日志级别
class LogLevel{
public:
    enum Level{
        DEBUG=1,
        INFO=2,
        WARN=3,
        ERROR=4,
        FATAL=5
    };

    static const char* ToString(LogLevel::Level level);
};


//日志事件
class LogEvent{
public:
    using ptr=std::shared_ptr<LogEvent>;
    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,
            const char* file,int32_t m_line,uint32_t elapse,
            uint32_t thread_id,uint32_t fiber_id,uint64_t time);

    const char* getFile()const {return m_file;}
    int32_t getLine() const{return m_line;}
    int32_t getElapse()const{return m_elapse;}
    int32_t getThreadId()const{return m_threadId;}
    int32_t getFiberId()const{return m_fiberId;}
    int64_t getTime()const{return m_time;}
    std::string getContent()const{return m_ss.str();}
    std::shared_ptr<Logger> getLogger() const {return m_logger;}
    LogLevel::Level getLevel() const{return m_level;}

    std::stringstream& getSS() {return m_ss;}
    void format(const char* fmt,...);
    void format(const char* fmt,va_list al);
    
private:
    //哪个文件，第几行调用了log
    const char* m_file=nullptr; //文件名
    int32_t m_line=0; //行号
    int32_t m_elapse=0; //程序启动开始到现在的毫秒数
    int32_t m_threadId=0; //线程id
    int32_t m_fiberId=0; //协程id
    uint64_t m_time=0; //时间戳
    std::stringstream m_ss; //存储用户的日志信息

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

//当LogEventWarp析构时，会获取LogEvent中的m_logger，调用log方法输出日志信息
class LogEventWarp{
public:
    LogEventWarp(LogEvent::ptr e);
    ~LogEventWarp();

    std::stringstream& getSS();
    LogEvent::ptr& getEvent();
private:
    LogEvent::ptr m_event;
};


//日志格式器
class LogFormatter{
public:
    using ptr=std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern="%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%T%f:%l%T%m%n");

    //遍历m_items，调用每个子模块重载的format方法，将获取的属性同一输出的参数os中，再将作为返回值，即可完成日志的解析
    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);
public:
    //格式化子模块：不同的子模块对于不同的占位符，通过重载format方法获取不同日志属性
    class FormatItem{
    public:
        using ptr=std::shared_ptr<FormatItem>;
        FormatItem(const std::string& fmt=""){};
        virtual ~FormatItem(){};
       
        virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel::Level level,std::shared_ptr<LogEvent> event)=0;
    };

    //根据”日志格式“m_pattern格式化后，获取对于子模块存入m_items
    void init();
private:
    std::string m_pattern; //格式模式：在init中会根据m_pattern中的占位符，生成不同的”子模块“存储在m_items中
    std::vector<FormatItem::ptr> m_items; //子模块：可通过调用重载的format方法获取对于日志属性
};

//日志输出地(基类)
class LogAppender{
public:
    using ptr=std::shared_ptr<LogAppender>;
    virtual ~LogAppender(){}

    //输出
    virtual void log(std::shared_ptr<Logger> logger,LogLevel::Level level,std::shared_ptr<LogEvent> event)=0;
    
    //设置/获取日志格式
    void setFormatter(LogFormatter::ptr formatter){m_formatter=formatter;}
    LogFormatter::ptr getFormatter(){return m_formatter;}

    void setLevel(LogLevel::Level level){m_level=level;}
    LogLevel::Level getLevel(){return m_level;}
protected:
    LogLevel::Level m_level=LogLevel::DEBUG; //级别
    LogFormatter::ptr m_formatter; //LogFormatter句柄
};


//日志器
class Logger:public std::enable_shared_from_this<Logger>{
public: 
    using ptr=std::shared_ptr<Logger>;

    Logger(const std::string& name="root");
    //添加/删除appender
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    //设置/获取level
    void setLevel(LogLevel::Level level) {m_level=level;};
    LogLevel::Level getLevel() const {return m_level;};

    //输出
    void log(LogLevel::Level level,LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    std::string getName()const{return m_name;}
private:
    std::string m_name;  //日志名称
    LogLevel::Level m_level;  //日志级别
    std::list<LogAppender::ptr> m_appenders;  //Appender集合
    LogFormatter::ptr m_formatter; //解析器
};

//输出到控制台的Appender类
class StdoutLogAppender:public LogAppender{
public:
    using ptr=std::shared_ptr<StdoutLogAppender>;
    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,std::shared_ptr<LogEvent> event) override; 
private:

};

//输出到文件的Appender类
class FileLogAppender:public LogAppender{
public:
    using ptr=std::shared_ptr<FileLogAppender>;
    //默认为mode=std::ios::out:覆盖写入
    FileLogAppender(const std::string& filename,std::ios_base::openmode mode=std::ios::out);

    void log(std::shared_ptr<Logger> logger,LogLevel::Level level,std::shared_ptr<LogEvent> event) override;

    //重新打开文件（打开成功，返回true）
    bool reopen();
private:
    std::string m_filename; //文件名
    std::ofstream m_filestream; //文件输出流
};

class LoggerManager{
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);
    void init();
private:
    std::map<std::string,Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

using LoggerMgr=Singleton<LoggerManager>;

};