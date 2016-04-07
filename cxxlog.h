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
        pthread_once(&pOnce_, &Logger::Init);
        assert(instance_ != NULL);
        return instance_;
    }
    
    static void Init() {
        instance_ = new Logger();
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
    //singleton
    static pthread_once_t pOnce_;
    static Logger *instance_;

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
#define CXXLOG_TRACE(format,...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::TRACE, format, __VA_ARGS__); }while(0)
#define CXXLOG_DEBUG(format,...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::DEBUG, format, __VA_ARGS__); }while(0)
#define CXXLOG_INFO(format,...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::INFO, format, __VA_ARGS__); }while(0)
#define CXXLOG_WARN(format,...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::WARN, format, __VA_ARGS__); }while(0)
#define CXXLOG_ERROR(format,...) do { cxxlog::Logger::GetInstance()->Log(cxxlog::LogLevel::ERROR, format, __VA_ARGS__); }while(0)


#endif

