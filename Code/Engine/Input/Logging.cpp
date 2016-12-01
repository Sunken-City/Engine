#include "Engine/Input/Logging.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Memory/Callstack.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Input/Console.hpp"
#include <chrono>
#include <ctime>
#include <algorithm>
#include <iomanip>

extern bool g_isQuitting;
//Define an APP_NAME globally somewhere so that we know what to name the file for the logger.
//This works for all of my current projects, but will probably get out of date at some time later once MainWin32 gets nuked c:
extern const char* APP_NAME;
Logger* Logger::instance = nullptr;
const int LOGF_STACK_LOCAL_TEMP_LENGTH = 2048;

//-----------------------------------------------------------------------------------
void LoggerThreadMain()
{
    while (!g_isQuitting)
    {
        SwitchToThread();
        LogMessage* message = nullptr;

        if (Logger::instance->HasMessages())
        {
            message = Logger::instance->DequeueMessage();
        }

        if (message)
        {
            Logger::instance->Log(message);
            if (message->callstack)
            {
                Callstack* callstack = message->callstack;
                CallstackLine* callstackLines = CallstackGetLines(callstack);
                Logger::instance->Log(">>>Callstack:\n//-----------------------------------------------------------------------------------\n");
                for (unsigned int i = 0; i < callstack->frameCount; ++i)
                {
                    Logger::instance->Log(Stringf("%s(%i): %s\n", callstackLines[i].filename, callstackLines[i].line, callstackLines[i].functionName).c_str());
                }
                Logger::instance->Log("//-----------------------------------------------------------------------------------\n\n");
            }
            delete message;
        }
    }
}

//-----------------------------------------------------------------------------------
Logger::Logger()
    : m_file(nullptr)
{
    CreateLogFile();
    CleanUpOldLogFiles();
}

//-----------------------------------------------------------------------------------
Logger::~Logger()
{
    //Wait for the thread to finish shutting down, then continue.
    if (m_loggingThread.joinable())
    {
        m_loggingThread.join();
    }
    //Flush any remaining things in the queue before we destory the system.
    Logger::instance->FlushLog();
    fclose(m_file);
    Logger::instance->CopyLogFileAsLatest();
}

//-----------------------------------------------------------------------------------
void Logger::CleanUpOldLogFiles()
{
    std::vector<std::string> logFiles = EnumerateFiles("Logs", "*.txt", true);
    if (logFiles.size() > MAX_LOG_HISTORY)
    {
        std::sort(logFiles.begin(), logFiles.end());
        int lastIndexToKeep = logFiles.size() - MAX_LOG_HISTORY;
        for (int i = 0; i < lastIndexToKeep; ++i)
        {
            std::wstring wideFilePath = L"Logs\\" + std::wstring(logFiles[i].begin(), logFiles[i].end());
            DeleteFile(wideFilePath.c_str());
        }
    }
}

//-----------------------------------------------------------------------------------
void Logger::CopyLogFileAsLatest()
{
    std::wstring wideFilePath = std::wstring(m_fileName.begin(), m_fileName.end());
    CopyFile(wideFilePath.c_str(), L"LatestLogFile.txt", false);
}

//-----------------------------------------------------------------------------------
void Logger::CreateLogFile()
{
    std::time_t timeNow = std::time(nullptr);
    //Supressed because localtime_s isn't actually accessible because I don't know why try for yourself. >:I
    #pragma warning(suppress: 4996)
    std::tm time = *std::localtime(&timeNow);
    std::string timestamp = Stringf("%02i-%i-%i %i.%02i.%02i", time.tm_mon + 1, time.tm_mday, time.tm_year + 1900, time.tm_hour, time.tm_min, time.tm_sec);
    EnsureDirectoryExists("Logs");
    m_fileName = Stringf("Logs\\%s Log %s.txt", APP_NAME, timestamp.c_str());
    errno_t errorCode = fopen_s(&m_file, m_fileName.c_str(), "wb");
    if (errorCode != 0x0)
    {
        ERROR_AND_DIE("Unable to open LogFile.txt for write");
    };
}

//-----------------------------------------------------------------------------------
void Logger::EnqueueMessage(LogMessage* msg)
{
    m_loggingQueue.Enqueue(msg);
}

//-----------------------------------------------------------------------------------
LogMessage* Logger::DequeueMessage()
{
    LogMessage* front = m_loggingQueue.Dequeue();
    return front;
}

//-----------------------------------------------------------------------------------
void Logger::FlushLog()
{
    LogMessage* message;
    unsigned int numMessages = m_loggingQueue.Size();
    for (unsigned int i = 0; i < numMessages; ++i)
    {
        message = DequeueMessage();
        Log(message);
        delete message;
    }

    fflush(m_file);
}


//-----------------------------------------------------------------------------------
bool Logger::HasMessages()
{
    return m_loggingQueue.Size() > 0;
}

//-----------------------------------------------------------------------------------
void Logger::StartLoggingThread()
{
    m_loggingThread = std::thread(LoggerThreadMain);
}

//-----------------------------------------------------------------------------------
void Logger::Log(const char* message)
{
    fwrite(message, sizeof(unsigned char), strlen(message), m_file);
    #ifdef FORWARD_LOG_TO_OUTPUT_WINDOW
    {
        DebuggerPrintf(message);
    }
    #endif // FORWARD_LOG_TO_OUTPUT_WINDOW
    #ifdef FORWARD_LOG_TO_CONSOLE
    {
        if (Console::instance != nullptr)
        {
            Console::instance->PrintLine(message, RGBA::CERULEAN);
        }
    }
    #endif // FORWARD_LOG_TO_CONSOLE
}

//-----------------------------------------------------------------------------------
void Logger::Log(LogMessage* message)
{
    fwrite(message->formattedMessage, sizeof(unsigned char), strlen(message->formattedMessage), m_file);
    #ifdef FORWARD_LOG_TO_OUTPUT_WINDOW
    {
        DebuggerPrintf(message->formattedMessage);
    }
    #endif // FORWARD_LOG_TO_OUTPUT_WINDOW
    #ifdef FORWARD_LOG_TO_CONSOLE
    {
        if (Console::instance != nullptr)
        {
            Console::instance->PrintLine(message->formattedMessage, RGBA::CERULEAN);
        }
    }
    #endif // FORWARD_LOG_TO_CONSOLE
}

//-----------------------------------------------------------------------------------
static void LogPrintf(LogLevel level, const char* format, va_list args)
{
    if (static_cast<int>(level) < LOG_LEVEL_THRESHOLD)
    {
        return;
    }
    LogMessage* msg = new LogMessage();
    msg->formattedMessage = new char[LOGF_STACK_LOCAL_TEMP_LENGTH];
    vsnprintf_s(msg->formattedMessage, LOGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, args);
    msg->formattedMessage[LOGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

    //Send message off to I/O thread
    Logger::instance->EnqueueMessage(msg);

}

//-----------------------------------------------------------------------------------
void LogPrintf(const char* format, ...)
{
    va_list variableArgumentList;
    va_start(variableArgumentList, format);
    LogPrintf(LogLevel::DEFAULT, format, variableArgumentList);
    va_end(variableArgumentList);
}

//-----------------------------------------------------------------------------------
void LogPrintf(LogLevel level, const char* format, ...)
{
    va_list variableArgumentList;
    va_start(variableArgumentList, format);
    LogPrintf(level, format, variableArgumentList);
    va_end(variableArgumentList);
}

//-----------------------------------------------------------------------------------
void LogPrintfWithCallstack(LogLevel level, const char* format, ...)
{
    if (static_cast<int>(level) < LOG_LEVEL_THRESHOLD)
    {
        return;
    }
    va_list variableArgumentList;
    va_start(variableArgumentList, format); 
    LogMessage* msg = new LogMessage();
    msg->callstack = AllocateCallstack();
    msg->formattedMessage = new char[LOGF_STACK_LOCAL_TEMP_LENGTH];
    vsnprintf_s(msg->formattedMessage, LOGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList);
    msg->formattedMessage[LOGF_STACK_LOCAL_TEMP_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

    //Send message off to I/O thread
    Logger::instance->EnqueueMessage(msg);
    va_end(variableArgumentList);
}

//-----------------------------------------------------------------------------------
void LoggerStartup()
{
    if (Logger::instance)
    {
        ERROR_AND_DIE("Logger was already initialized!");
    }
    else
    {
        Logger::instance = new Logger();
        Logger::instance->StartLoggingThread();
    }
}

//-----------------------------------------------------------------------------------
void LoggerShutdown()
{
    if (Logger::instance)
    {
        delete Logger::instance;
        Logger::instance = nullptr;
    }
    else
    {
        ERROR_AND_DIE("Logger was not initialized!");
    }
}

//-----------------------------------------------------------------------------------
LogMessage::LogMessage()
    : formattedMessage(nullptr)
    , callstack(nullptr) 
{

};

//-----------------------------------------------------------------------------------
LogMessage::~LogMessage()
{
    if (formattedMessage)
    {
        delete[] formattedMessage;
    }
    if (callstack)
    {
        FreeCallstack(callstack);
    }
};
