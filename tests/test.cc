#include<iostream>
#include "log.h"
#include "util.h"
int main(){
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender)); //添加StdoutLogAppender

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt")); //创建FileLogAppender
    file_appender->setLevel(sylar::LogLevel::ERROR); //设置appender的level

    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%m%n")); //创建formatter，日志格式"%d%T%m%n"
    file_appender->setFormatter(fmt); //appender设置formatter

    logger->addAppender(file_appender); //添加FileLogAppender

    //sylar::LogEvent::ptr event(new sylar::LogEvent(__FILE__,__LINE__,0,1,2,time(0))); //创建LogEvent
    //event->getSS()<<"hello sylar log"; //设置LogEvent的消息体

    //logger->log(sylar::LogLevel::DEBUG,event); //调用log方法，在m_level<=level的appender中输出"%d%T%m%n"对应的日志信息

    std::cout<<"hello sylar log"<<std::endl;
    
    SYLAR_LOG_INFO(logger)<<"test macro";
    SYLAR_LOG_ERROR(logger)<<"test macro error";
    
    SYLAR_LOG_FMT_ERROR(logger,"test macro fmt error %s","aa");

    auto l=sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l)<<"xxx";
    return 0;
}