//-----------------------------------------------------------------------------------------------
// Time.cpp

//-----------------------------------------------------------------------------------------------
#include "Engine/Time/Time.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

unsigned int g_frameCounter = 0;
float g_averageDeltaSeconds = 0.0f;

//---------------------------------------------------------------------------
double InitializeTime( LARGE_INTEGER& out_initialTime )
{
    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency( &countsPerSecond );
    QueryPerformanceCounter( &out_initialTime );
    return( 1.0 / static_cast< double >( countsPerSecond.QuadPart ) );
}

//---------------------------------------------------------------------------
double GetCurrentTimeSeconds()
{
    static LARGE_INTEGER initialTime;
    static double secondsPerCount = InitializeTime( initialTime );
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter( &currentCount );
    LONGLONG elapsedCountsSinceInitialTime = currentCount.QuadPart - initialTime.QuadPart;

    double currentSeconds = static_cast< double >( elapsedCountsSinceInitialTime ) * secondsPerCount;
    return currentSeconds;
}

//-----------------------------------------------------------------------------------
double GetCurrentTimeMilliseconds()
{
    return GetCurrentTimeSeconds() * 1000.0f;
}

//-----------------------------------------------------------------------------------
int GetTimeBasedSeed()
{
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter(&currentCount);
    return static_cast<int>(currentCount.QuadPart);
}

//-----------------------------------------------------------------------------------
unsigned int GetFrameNumber()
{
    return g_frameCounter;
}

//-----------------------------------------------------------------------------------
void AdvanceFrameNumber()
{
    ++g_frameCounter;
}

//-----------------------------------------------------------------------------------
void UpdateFrameRate(float deltaSeconds)
{
    g_averageDeltaSeconds = (deltaSeconds * 0.1f) + (g_averageDeltaSeconds * 0.9f);
}

//-----------------------------------------------------------------------------------
float GetCurrentFrameRate()
{
    return 1.0f / g_averageDeltaSeconds;
}
