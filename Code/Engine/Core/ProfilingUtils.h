#pragma once
#include "Engine/Core/Memory/UntrackedAllocator.hpp"
#include "Engine/DataStructures/ObjectPool.hpp"
#include <chrono>
#include <vector>

struct ProfileLogSection;
struct ProfileReportNode;

#define COMBINE1(X,Y) X##Y  // helper macro
#define COMBINE(X,Y) COMBINE1(X,Y)
#define PROFILE_LOG_SECTION(ID) ProfileLogSection COMBINE(profileLogSection, __LINE__)(ID);

//GLOBAL FUNCTIONS/////////////////////////////////////////////////////////////////////
uint64_t GetCurrentPerformanceCount();
double PerformanceCountToSeconds(uint64_t& performanceCount);

//GLOBAL VARIABLES/////////////////////////////////////////////////////////////////////
extern uint64_t g_profilingStartTime;
extern uint64_t g_profilingEndTime;
extern std::vector<ProfileReportNode, UntrackedAllocator<ProfileReportNode>> g_profilingResults;

//STRUCTS/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------
struct ProfileSample
{
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    ProfileSample() : id(nullptr), startCount(0), endCount(0), parent(nullptr), children(nullptr), prev(nullptr), next(nullptr) {};
    inline double GetDurationInSeconds() { return PerformanceCountToSeconds(endCount) - PerformanceCountToSeconds(startCount); };
    inline void AddAllocation(size_t allocationSize) { sizeAllocs += allocationSize; ++numAllocs; };
    //inline void GetDurationInSeconds(ProfileSample* other) { PerformanceCountToSeconds(endCount) - PerformanceCountToSeconds(startCount); };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    uint64_t startCount;
    uint64_t endCount;
    ProfileSample* parent;
    ProfileSample* children;
    //Sibling pointers
    ProfileSample* prev;
    ProfileSample* next;
    const char* id;
    size_t sizeAllocs = 0;
    size_t numAllocs = 0;
    size_t numDrawCalls = 0;
    //unsigned int numCalls;
    //double averageTime = -1.0;
};

//-----------------------------------------------------------------------------------
struct ProfileReportNode
{
public:
    ProfileReportNode() : m_lastTime(0.0), m_numSamples(0), m_minTime(0.0), m_maxTime(0.0), m_averageTime(0.0) {};
    void AddSample(ProfileSample* otherSample);
    void CalculatePercentage(double frameTime) { m_framePercentage = static_cast<float>(m_totalTime / frameTime); };
    inline bool operator<(const ProfileReportNode& other) { return (m_totalSelfTime > other.m_totalSelfTime); }; //This is intentional for sorting yes I'm evil.
    double GetTimeForChildren(ProfileSample* otherSample);
    const char* m_id;
    double m_lastTime;
    unsigned long long m_numSamples;
    double m_minTime;
    double m_maxTime;
    double m_averageTime;
    double m_totalChildTime = 0.0;
    double m_totalSelfTime = 0.0;
    double m_totalTime = 0.0;
    size_t m_sizeAllocs = 0;
    size_t m_numAllocs = 0;
    size_t m_numDrawCalls = 0;
    float m_framePercentage = 0.0f;
    uint64_t m_start;
    uint64_t m_end;
};

//-----------------------------------------------------------------------------------
class ProfilingSystem
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    ProfilingSystem();
    ~ProfilingSystem();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void MarkFrame();
    void PushSample(const char* id);
    //The sample id here is unused, just used for readability for pushing and popping samples.
    void PopSample(const char* id = "");
    void PrintTreeListView();
    bool AddProfileNode(ProfileSample* root);
    void GenerateProfilingReport();
    double GetAverageFrameDuration();
    ProfileSample* GetLastFrame();
    inline bool IsEnabled() const { return m_isEnabled; };
    inline bool IsDisabled() const { return !m_isEnabled; };
    inline void SetEnabled(bool enabled) { m_intentToEnable = enabled; };

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static ProfilingSystem* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    ProfileSample* m_currentFrameRoot;
    ProfileSample* m_activeSample;
    ProfileSample* m_previousFrameRoot;
    ObjectPool<ProfileSample> m_sampleAllocator;

private:
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void StartNewFrame();
    void EndPreviousFrame();
    bool DeleteSampleTree(ProfileSample* root);
    void PrintNodeListView(ProfileSample* root, unsigned int depth);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    double m_rollingAverageFrametime = 0.0f;
    bool m_isEnabled;
    bool m_intentToEnable;
};

//-----------------------------------------------------------------------------------
struct ProfileLogSection
{
    ProfileLogSection(const char* id);
    ~ProfileLogSection();

    const char* m_id;
    uint64_t m_start;
    uint64_t m_end;
};