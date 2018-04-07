//-----------------------------------------------------------------------------------------------
// Time.hpp
//
#pragma once
#ifndef included_Time
#define included_Time

//---------------------------------------------------------------------------
double GetCurrentTimeSeconds();
double GetCurrentTimeMilliseconds();
int GetTimeBasedSeed();
unsigned int GetFrameNumber();
void AdvanceFrameNumber();
float GetCurrentFrameRate();
void UpdateFrameRate(float deltaSeconds);

#endif
