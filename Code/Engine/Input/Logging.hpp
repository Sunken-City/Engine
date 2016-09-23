#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>
#include <deque>
#include "Engine/Core/Memory/UntrackedAllocator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/DataStructures/ThreadSafeQueue.hpp"

struct Callstack;

//-----------------------------------------------------------------------------------
enum class LogLevel
{
    ALL = 0,
    VERBOSE = 0,
    DEFAULT,
    WARNING,
    SEVERE,
    NONE,
    NUM_LOG_LEVELS
};

//-----------------------------------------------------------------------------------
struct LogMessage
{
    LogMessage();
    ~LogMessage();
    char* formattedMessage;
    Callstack* callstack;
};

//-----------------------------------------------------------------------------------
class Logger
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Logger();
    ~Logger();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void EnqueueMessage(LogMessage* msg);
    LogMessage* DequeueMessage();
    void FlushLog();
    bool HasMessages();
    void StartLoggingThread();
    void Log(LogMessage* message);
    void Log(const char* message);
    void CreateLogFile();
    void CleanUpOldLogFiles();
    void CopyLogFileAsLatest();

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static Logger* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::thread m_loggingThread;
    FILE* m_file;
    std::string m_fileName;

private:
    ThreadSafeQueue<LogMessage> m_loggingQueue;
};

//GLOBAL FUNCTIONS/////////////////////////////////////////////////////////////////////
void LogPrintf(const char* format, ...);
void LogPrintf(LogLevel level, const char* format, ...);
void LogPrintfWithCallstack(LogLevel level, const char* format, ...);
void LoggerStartup();
void LoggerShutdown();