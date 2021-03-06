//Author: Hao Hongting <haohongting@live.com>
#ifndef __CXXLOG_H__
#define __CXXLOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include <thread>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/syscall.h>
#include <mutex> 

namespace cxxlog {

extern thread_local pid_t myThreadId_;

enum class LogLevel {
    TRACE = 0,
    DEBUG,     
    INFO,
    WARN,
    ERROR
};

// thread safe
class Logger {

public:
    ~Logger();
    //No Copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Open(const char *pathName);
    void Close();
    void SetRotateSize(size_t size);
    void SetForceFlushIntervel(unsigned int seconds);
    void SetLevel(LogLevel level);
    void SetStdout(bool on);
    void Log(LogLevel level, const char *format, ...);

    static Logger* GetInstance() {
        static Logger instance;
        return &instance;
    }
    
    pid_t ThreadId() { 
        if (myThreadId_ == 0) {
            myThreadId_ = Gettid();
        }
        return myThreadId_; 
    }
   
private:
    Logger();
    void FormatTimeStamp();
    void Write(LogLevel level);
    void Rotate();
    const char* MapLevel(LogLevel level);

    pid_t Gettid() {  
        return syscall(SYS_gettid);  
    }  

private:
    //log file
    std::string logPathName_;
    FILE *logFp_;
    char logBuf_[1024];

    //log option
    size_t rotateSize_;
    size_t currentSize_;
    LogLevel level_;
    bool stdOut_;

    //log timestamp
    struct timeval tv_;
    struct timeval lastTv_;
    time_t lastFlushSecs_;
    unsigned int forceFlushIntervel_;
    char timeBuf_[64];
    int timestampLen_;

    std::mutex mtx_;
};


}

#define CXXLOG_OPEN(pathName, stdOut) do { cxxlog::Logger::GetInstance()->Open(pathName); \
    cxxlog::Logger::GetInstance()->SetStdout(stdOut); }while(0)
#define CXXLOG_TRACE(...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::TRACE, __VA_ARGS__); }while(0)
#define CXXLOG_DEBUG(...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::DEBUG, __VA_ARGS__); }while(0)
#define CXXLOG_INFO(...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::INFO, __VA_ARGS__); }while(0)
#define CXXLOG_WARN(...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::WARN, __VA_ARGS__); }while(0)
#define CXXLOG_ERROR(...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::ERROR, __VA_ARGS__); }while(0)


#endif

