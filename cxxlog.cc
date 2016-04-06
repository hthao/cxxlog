#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include "cxxlog.h"

namespace cxxlog {

Logger *Logger::instance_ = NULL;
pthread_once_t Logger::pOnce_ = PTHREAD_ONCE_INIT;
thread_local pid_t myThreadId_ = 0;

Logger::Logger()
    :logPathName_(""),
    logFp_(NULL),
    rotateSize_(1024*1024*10),
    currentSize_(0),
    level_(LogLevel::TRACE),
    stdOut_(false),
    lastFlushSecs_(0),
    forceFlushIntervel_(5) 
{
    // default rotate size is 10MB.
    // default forceFlushIntervel is 5 seconds.
}

Logger::~Logger()
{
    Close();
}

void Logger::Open(const char *pathName)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (logFp_) {
        fprintf(stderr, "cxxlog, Logger is already opened as %s\n", logPathName_.c_str());
        return;
    }

    logPathName_.assign(pathName);
    char pid[32];
    snprintf(pid, 32, ".%d", (int)::getpid());
    logPathName_.append(pid);

    logFp_ = ::fopen(logPathName_.c_str(), "a+");
    if (!logFp_) {
        perror("Logger open failed");
        return;
    }
}

void Logger::Close()
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (logFp_) {
        ::fclose(logFp_);
        logFp_ = NULL;
    }
    logPathName_ = "";
    rotateSize_ = 10*1024*1024;
    currentSize_ = 0;
    level_ = LogLevel::TRACE;
    stdOut_ = false;
    lastFlushSecs_ = 0;
    forceFlushIntervel_ = 5;
}

void Logger::SetRotateSize(size_t size)
{
    std::lock_guard<std::mutex> lock(mtx_);
    rotateSize_ = size;
}

void Logger::SetLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mtx_);
    level_ = level;
}

void Logger::SetForceFlushIntervel(unsigned int seconds)
{
    std::lock_guard<std::mutex> lock(mtx_);
    forceFlushIntervel_ = seconds;
}

void Logger::SetStdout(bool on)
{
    std::lock_guard<std::mutex> lock(mtx_);
    stdOut_ = on;
}

void Logger::Log(LogLevel level, const char *format, ...)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (!logFp_) {
        fprintf(stderr,"Logger is not opened!!\n");
        return;
    }

    if (level < level_) {
        return;
    }

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(logBuf_, 1024, format, argptr);
    va_end(argptr);

    Write(level);
    Rotate();
}

const char* Logger::MapLevel(LogLevel level)
{
    if (level == LogLevel::TRACE) {
        return "TRACE";
    } else if (level == LogLevel::DEBUG) {
        return "DEBUG";
    } else if (level == LogLevel::INFO) {
        return "INFO";
    } else if (level == LogLevel::WARN) {
        return "WARNING";
    } else if (level == LogLevel::ERROR) {
        return "ERROR";
    } else {
        return "Unknown Level";
    }
}

void Logger::FormatTimeStamp()
{
    ::gettimeofday(&tv_, NULL);
    int offset = ::strftime(timeBuf_, 64, "%Y%m%d %H:%M:%S.", ::localtime(&tv_.tv_sec));
    snprintf(timeBuf_ + offset, 64 - offset, "%03d", (int)tv_.tv_usec/1000);
}

void Logger::Write(LogLevel level)
{
    FormatTimeStamp();
    int size = fprintf(logFp_, "%s [%d] <%s> %s", timeBuf_, ThreadId(), MapLevel(level), logBuf_);

    if (stdOut_) {
        fprintf(stdout, "%s [%d] <%s> %s", timeBuf_, ThreadId(), MapLevel(level), logBuf_);
    }

    currentSize_ += size;
    ::memset(timeBuf_, 0, 64);
    ::memset(logBuf_, 0, 1024);
 
    //force flush.
    if (::difftime(tv_.tv_sec, lastFlushSecs_) > (double)forceFlushIntervel_) {
        ::fflush(logFp_);
        lastFlushSecs_ = tv_.tv_sec;
    }
}

void Logger::Rotate()
{
    if (currentSize_ > rotateSize_) {
        ::fclose(logFp_);

        std::string oldLogFile = logPathName_;
        oldLogFile.append(".old");
        ::rename(logPathName_.c_str(), oldLogFile.c_str());

        logFp_ = ::fopen(logPathName_.c_str(), "a+");
        currentSize_ = 0;
    }
}

}
