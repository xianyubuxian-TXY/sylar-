#include "log.h"
#include<map>
#include<tuple>
#include<iostream>
#include<functional>
namespace sylar{
//--------------------------LogLevel-------------------------
const char* LogLevel::ToString(LogLevel::Level level){
    switch(level)
    {
    /*
    case LogLevel::DEBUG:
        return "DEBUG";
        break;
    */
#define XX(name)\
    case LogLevel::name:\
        return #name;\
        break;

    //通过宏生成的5个分支
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
//undef：如果不 #undef XX，后面代码中任何出现 XX(...) 的地方都会被替换成 case LogLevel::... 这种写法
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}



//-------------------------LogEvent------------------------
LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,uint32_t elapse,uint32_t thread_id
    ,uint32_t fiber_id,uint64_t time)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_logger(logger)
    ,m_level(level)
{}

void LogEvent::format(const char* fmt,...)
{   
    va_list al;
    va_start(al,fmt);
    format(fmt,al);
    va_end(al);
}

void LogEvent::format(const char* fmt,va_list al)
{
    char* buf=nullptr;
    int len=vasprintf(&buf,fmt,al);
    if(len!=-1){
        m_ss<<std::string(buf,len);
        free(buf);
    }
}

//-------------------------LogEventWrap---------------------------
LogEventWarp::LogEventWarp(LogEvent::ptr e)
    :m_event(e)
{}

LogEventWarp::~LogEventWarp()
{
    m_event->getLogger()->log(m_event->getLevel(),m_event);
}

std::stringstream& LogEventWarp::getSS()
{
    return m_event->getSS();    
}

LogEvent::ptr& LogEventWarp::getEvent()
{
    return m_event;
}



//------------------------------LogFormatter---------------------------------
LogFormatter::LogFormatter(const std::string& pattern)
:m_pattern(pattern)
{
    init();
}

//对event格式进行解析
std::string LogFormatter::format(Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event){
    std::stringstream ss;
    for(auto& it:m_items){
        it->format(ss,logger,level,event);
    }
    return ss.str();
}

//对应%m：消息体
class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string& fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getContent();
    }
};

//对应%p：level
class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string& fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<LogLevel::ToString(level);
    }
};

//对应%r：Elapse
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string& fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<LogLevel::ToString(level);
    }
};

//对应%c：日志名称
class NameFormatItem : public LogFormatter::FormatItem{
public:
    NameFormatItem(const std::string& fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<logger->getName();
    }
};

//对应%t：线程id
class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string& fmt):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<event->getThreadId();
    }
};

//对应%d：时间
class DataTimeFormatItem : public LogFormatter::FormatItem{
public:
    DataTimeFormatItem(const std::string& format="%Y-%m-%d %H:%M:%S")
    :FormatItem(format)
    ,m_format(format)
    {
        if(m_format.empty())
        {
            m_format="%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
        struct tm tm;
        time_t time=event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        strftime(buf,sizeof(buf),m_format.c_str(),&tm);
        os<<buf;
    }
private:
    //时间解析格式："%Y-%m-%d %H:%M:%S"（默认）
    std::string m_format;
};

//对应%f：文件名
class FilenameFormatItem : public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string& fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<event->getFile();
}
};

//对应%l：行号
class LineFormatItem : public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string& fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<event->getLine();
}
};

//对应%n：回车换行
class NewLineFormatItem : public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<std::endl;
    }
};

//输出普通字符串
class StringFormatItem : public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string& str="")
        :FormatItem(str),m_string(str)
        {}

    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<m_string;
}
private:
    std::string m_string;
};


//对应%T：制表符
class TabFormatItem : public LogFormatter::FormatItem{
public:
    TabFormatItem(const std::string fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<"\t";
    }
};

//对应%F：制表符
class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string fmt=""):FormatItem(fmt){}
    void format(std::ostream& os,Logger::ptr logger,LogLevel::Level level,LogEvent::ptr event) override{
            os<<event->getFiberId();
    }
};

//对日志进行解析:允许的三个格式化字符串：%xxx——>%d  %xxx{xxx}——>%d{5}   %%——>转义%
void LogFormatter::init(){
    //三元组(key,fmt,type):  key:占位符—>如"%d"中的"d"  fmt:对占位符的限定(参数)->如"5"  type->类型：0->普通字符串  1->含有占位符的字符串  2->格式错误的字符串
    //"%t{%Y-%m-%d} %s:%% message" ==> ("t","%Y-%m-%d",1)  (" ", "",0)  ("s","",1)  (":% message", "",0)
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;                   // 累积文字片段
    const std::string &pat = m_pattern;
    size_t len = pat.size();

    for (size_t i = 0; i < len; /* i 在下面调整 */) {
        // —— 1. 文字片段
        if (pat[i] != '%') {
            nstr.push_back(pat[i]);
            ++i;
            continue;
        }

        // —— 2. 处理 "%%" 转义
        if (i + 1 < len && pat[i+1] == '%') {
            nstr.push_back('%');
            i += 2;
            continue;
        }

        // —— 如果前面有文字，先 flush 到 vec
        if (!nstr.empty()) {
            vec.emplace_back(nstr, std::string(), 0);
            nstr.clear();
        }

        // —— 3. 解析 %xxx 或 %xxx{fmt}
        size_t n = i + 1;
        std::string key="";   // 比如 "d", "s", "t"
        std::string fmt="";   // 比如 "5", "%Y-%m-%d"

        // 先读 key:直到遇到“非字母”字符  isalpha->判断是否为“字母”
        while (n < len && std::isalpha((unsigned char)pat[n])) {
            key.push_back(pat[n]);
            ++n;
        }
        // 如果紧接着是 '{'，再读 fmt
        if (n < len && pat[n] == '{') {
            ++n;  // 跳过 '{'
            size_t start = n;
            while (n < len && pat[n] != '}') {
                ++n;
            }
            // 找到右花括号
            if (n < len && pat[n] == '}') {
                fmt = pat.substr(start, n - start);
                ++n;  // 跳过 '}'
            } else {
                // 没闭合，视为错误
                vec.emplace_back("<<pattern_error>>", std::string(), 2);
                i = n;
                continue;
            }
        }

        // 推入格式化项
        vec.emplace_back(key, fmt, 1);
        // 更新 i 到已消费位置
        i = n;
    }

    // 剩余文字 flush
    if (!nstr.empty()) {
        vec.emplace_back(nstr, std::string(), 0);
    }

    std::map<std::string,std::function<FormatItem::ptr(const std::string& str)>> s_format_items={
        //{"m",[](const std::string& fmt){return FormatItem::ptr(new MessageFormatItem(fmt));}}
    #define XX(str,C)\
        {#str,[](const std::string& fmt){return FormatItem::ptr(new C(fmt));}}

        XX(m,MessageFormatItem),    //%m -- 消息体
        XX(p,LevelFormatItem),      //%p -- level
        XX(r,ElapseFormatItem),     //%r -- 启动后的时间
        XX(c,NameFormatItem),       //%c -- 日志名称
        XX(t,ThreadIdFormatItem),   //%t -- 线程id
        XX(n,NewLineFormatItem),    //%n -- 回车换行
        XX(d,DataTimeFormatItem),   //%d -- 时间
        XX(f,FilenameFormatItem),   //%f -- 文件名
        XX(l,LineFormatItem),       //%l -- 行号
        XX(T,TabFormatItem),        //%T -- 制表符
        XX(F,FiberIdFormatItem)     //%F -- 协程id
    #undef XX
    };

    
    for(auto& i:vec){
        if(std::get<2>(i)==0 || std::get<2>(i)==2){
            //type=0：普通字符串  type=2：解析错误（直接存杰克）
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }
        else{
            //type!=0：需要解析的占位符
            auto it=s_format_items.find(std::get<0>(i));
            //根据占位符到s_format_items中寻找对应的解析模块
            if(it==s_format_items.end()){
                //占位符不存在
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %"+std::get<0>(i)+">>")));
            }
            else{
                //占位符存在，以 fmt为参数，调用占位符对应的解析模块进行解析
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
        //std::cout<<"("<<std::get<0>(i)<<") - ("<<std::get<1>(i)<<") - ("<<std::get<2>(i)<<")"<<std::endl;
    }
    //std::cout<<m_items.size()<<std::endl;
}

//------------------------------------------LogAppender------------------------------
FileLogAppender::FileLogAppender(const std::string& filename,std::ios_base::openmode mode)
:m_filename(filename)
{
    m_filestream.open(filename,mode);
}

void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,std::shared_ptr<LogEvent> event){
    if(level>=m_level)
    {
        m_filestream<<m_formatter->format(logger,level,event);
    }
}

//重新打开文件
bool FileLogAppender::reopen()
{   
    if(m_filestream)
    {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;
}

//输出到控制台
void StdoutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel::Level level,std::shared_ptr<LogEvent> event){
    if(level>=m_level)
    {
        std::cout<<m_formatter->format(logger,level,event);
    }
}


//--------------------------Logger---------------------------------
Logger::Logger(const std::string& name) //name="root" 默认
    :m_name(name)
    ,m_level(LogLevel::DEBUG)
    {
    //reset 通常用于智能指针（如 std::unique_ptr 或 std::shared_ptr）来释放其管理的资源，方便地恢复到初始状态
    //日志默认格式：%d:时间 {%Y-%m-%d %H:%M:%S}：时间格式  %T:制表符  %t:线程id  %F:协程id  
                //%p：level %c:日志名 %f：文件名 %l:行号 %m:日志内容 %n:回车换行符
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender){

    if(!appender->getFormatter()){
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
    for(auto it=m_appenders.begin();it!=m_appenders.end();++it){
        if(appender==*it)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level,LogEvent::ptr event){
    if(level>=m_level)
    {
        auto self=shared_from_this();
        //遍历每个appender，用appender去输出（appender是虚基类）
        for(auto &it:m_appenders)
        {
            it->log(self,level,event);
        }
    }
}

void Logger::debug(LogEvent::ptr event){

}

void Logger::info(LogEvent::ptr event){

}

void Logger::warn(LogEvent::ptr event){

}

void Logger::error(LogEvent::ptr event){

}

void Logger::fatal(LogEvent::ptr event){

}

//------------------------LogManager--------------------------------
LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    auto it=m_loggers.find(name);
    return it==m_loggers.end()?m_root:it->second;
}

void LoggerManager::init()
{

}

};
