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
struct ProfileReportNode
{
public:
    ProfileReportNode() : m_lastSample(0.0), m_numSamples(0), m_minSample(0.0), m_maxSample(0.0), m_averageSample(0.0) {};
    void AddSample(double sampleTime);

    double m_lastSample;
    unsigned long long m_numSamples;
    double m_minSample;
    double m_maxSample;
    double m_averageSample;
    uint64_t m_start;
    uint64_t m_end;
};

//-----------------------------------------------------------------------------------
struct ProfileSample
{
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    ProfileSample() : id(nullptr), startCount(0), endCount(0), parent(nullptr), children(nullptr), prev(nullptr), next(nullptr) {};
    inline double GetDurationInSeconds() { return PerformanceCountToSeconds(endCount) - PerformanceCountToSeconds(startCount); };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    const char* id;
    uint64_t startCount;
    uint64_t endCount;
    ProfileSample* parent;
    ProfileSample* children;
    //Sibling pointers
    ProfileSample* prev;
    ProfileSample* next;
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