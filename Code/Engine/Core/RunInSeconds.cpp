#include "Engine/Core/RunInSeconds.hpp"

std::vector<RunAfterSecondsCallbackData, UntrackedAllocator<RunAfterSecondsCallbackData>> g_runAfterSecondsCallbackFunctions;
std::vector<RunAfterSecondsCallbackData, UntrackedAllocator<RunAfterSecondsCallbackData>> g_newlyAddedRunAfterSecondsCallbackFunctions;
extern bool g_isDispatchingRunAfterSeconds = false;

//-----------------------------------------------------------------------------------
void RunCallback(RunAfterSecondsCallbackData* callback)
{
    callback->m_function(callback->m_data);
}

//-----------------------------------------------------------------------------------
void FlushRunAfterSecondsFunctions()
{
    while (g_runAfterSecondsCallbackFunctions.size() > 0)
    {
        RunCallback(&g_runAfterSecondsCallbackFunctions[0]);
        g_runAfterSecondsCallbackFunctions.erase(g_runAfterSecondsCallbackFunctions.begin());
    }
}

//-----------------------------------------------------------------------------------
//This is what runs the functions, needs to be called somewhere.
void DispatchRunAfterSeconds()
{
    unsigned int size = g_runAfterSecondsCallbackFunctions.size();
    g_isDispatchingRunAfterSeconds = true;

    for (auto iter = g_runAfterSecondsCallbackFunctions.begin(); iter != g_runAfterSecondsCallbackFunctions.end();)
    {
        if (GetCurrentTimeSeconds() - iter->m_dispatchedTimestampSeconds > iter->m_secondsToWait)
        {
            RunAfterSecondsCallbackData& callbackData = *iter;
            RunCallback(&callbackData);
            iter = g_runAfterSecondsCallbackFunctions.erase(iter);
        }
        if (iter == g_runAfterSecondsCallbackFunctions.end())
        {
            break;
        }
        ++iter;
    }

    g_isDispatchingRunAfterSeconds = false;
    for (auto iter = g_newlyAddedRunAfterSecondsCallbackFunctions.begin(); iter != g_newlyAddedRunAfterSecondsCallbackFunctions.end(); ++iter)
    {
        g_runAfterSecondsCallbackFunctions.push_back(*iter);
    }
    g_newlyAddedRunAfterSecondsCallbackFunctions.clear();
}