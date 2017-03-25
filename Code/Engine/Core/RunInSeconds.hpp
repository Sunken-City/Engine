#pragma once
#include "Engine/Time/Time.hpp"
#include "Memory/UntrackedAllocator.hpp"
#include <vector>

typedef void(*RunAfterSecondsFunction)(void*);
struct RunAfterSecondsCallbackData;

//GLOBAL VARIABLES/////////////////////////////////////////////////////////////////////
extern std::vector<RunAfterSecondsCallbackData, UntrackedAllocator<RunAfterSecondsCallbackData>> g_runAfterSecondsCallbackFunctions;
extern std::vector<RunAfterSecondsCallbackData, UntrackedAllocator<RunAfterSecondsCallbackData>> g_newlyAddedRunAfterSecondsCallbackFunctions;
extern bool g_isDispatchingRunAfterSeconds;

//-----------------------------------------------------------------------------------
struct RunAfterSecondsCallbackData
{
    RunAfterSecondsCallbackData()
    {
        m_dispatchedTimestampSeconds = GetCurrentTimeSeconds();
    }

    RunAfterSecondsCallbackData(RunAfterSecondsFunction functionPointer, void* data, float secondsToWait)
        : m_function(functionPointer)
        , m_data(data)
        , m_secondsToWait(secondsToWait)
    {
        m_dispatchedTimestampSeconds = GetCurrentTimeSeconds();
    }

    RunAfterSecondsFunction m_function = nullptr;
    void* m_data = nullptr;
    double m_dispatchedTimestampSeconds;
    float m_secondsToWait;

    bool operator< (const RunAfterSecondsCallbackData& other) { return this->m_secondsToWait < other.m_secondsToWait; };
};

//FUNCTIONS/////////////////////////////////////////////////////////////////////
void RunCallback(RunAfterSecondsCallbackData* callback);
void FlushRunAfterSecondsFunctions();
void DispatchRunAfterSeconds();

//-----------------------------------------------------------------------------------
template <typename CB>
void RunLater(void *data)
{
    // Cast this to CB type
    CB* callback = (CB*)data;
    (*callback)();

    delete callback;
}

//-----------------------------------------------------------------------------------
template <typename CB>
void RunAfterSeconds(CB callback, float secondsToWait)
{
    CB* copy = new CB(callback);

    RunAfterSecondsCallbackData cbd;
    cbd.m_function = RunLater<CB>;
    cbd.m_data = copy;
    cbd.m_secondsToWait = secondsToWait;

    g_isDispatchingRunAfterSeconds ? g_newlyAddedRunAfterSecondsCallbackFunctions.push_back(cbd) : g_runAfterSecondsCallbackFunctions.push_back(cbd);
}