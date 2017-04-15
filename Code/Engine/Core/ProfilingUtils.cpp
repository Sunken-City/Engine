#include "Engine/Core/ProfilingUtils.h"
#include "Engine/Input/Logging.hpp"
#include "Engine/DataStructures/InPlaceLinkedList.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Input/Console.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
using namespace std::chrono;

std::vector<ProfileReportNode, UntrackedAllocator<ProfileReportNode>> g_profilingResults;
ProfilingSystem* ProfilingSystem::instance = nullptr;

#ifdef PROFILING_ENABLED

//-----------------------------------------------------------------------------------
ProfilingSystem::ProfilingSystem()
    : m_isEnabled(true)
    , m_intentToEnable(true)
    , m_currentFrameRoot(nullptr)
    , m_previousFrameRoot(nullptr)
    , m_activeSample(nullptr)
    , m_sampleAllocator(2048)
{
}

//-----------------------------------------------------------------------------------
ProfilingSystem::~ProfilingSystem()
{
    if (DeleteSampleTree(m_previousFrameRoot))
    {
        m_sampleAllocator.Free(m_previousFrameRoot);
    }
    if (DeleteSampleTree(m_currentFrameRoot))
    {
        m_sampleAllocator.Free(m_currentFrameRoot);
    }
}

//-----------------------------------------------------------------------------------
void ProfilingSystem::MarkFrame()
{
    EndPreviousFrame();

    m_isEnabled = m_intentToEnable;

    StartNewFrame();
}

//-----------------------------------------------------------------------------------
void ProfilingSystem::EndPreviousFrame()
{
    if (IsDisabled() || m_currentFrameRoot == nullptr) 
    {
        return;
    }

    //If this hits, we forgot to POP!  Bad Programmer, No Cookie!
    ASSERT_OR_DIE(m_activeSample == m_currentFrameRoot, "There was an active sample still on the profiling stack. (Did you forget to pop?)");

    //Delete old previous sample if we have one
    if (DeleteSampleTree(m_previousFrameRoot))
    {
        m_sampleAllocator.Free(m_previousFrameRoot);
    }
    m_previousFrameRoot = m_currentFrameRoot;

    PopSample();
}


//-----------------------------------------------------------------------------------
void ProfilingSystem::StartNewFrame()
{
    if (IsDisabled()) 
    {
        return;
    }

    //active_sample is what right now? nullptr!
    ASSERT_OR_DIE(m_activeSample == nullptr, "There was an active sample still on the profiling stack. (Did you forget to pop?)");

    //Start new sample tree
    this->PushSample("frame");
    m_currentFrameRoot = m_activeSample;
}

//-----------------------------------------------------------------------------------
void ProfilingSystem::PushSample(const char* id)
{
    if (IsDisabled()) 
    {
        return;
    }

    //Create a profiling node
    ProfileSample* newSample = m_sampleAllocator.Alloc<ProfileSample>();
    newSample->id = id;
    newSample->startCount = GetCurrentPerformanceCount();

    //Add new_sample as a child to active_sample
    if (m_activeSample)
    {
        AddInPlace(m_activeSample->children, newSample);
    }
    newSample->parent = m_activeSample;
    m_activeSample = newSample;
}

//-----------------------------------------------------------------------------------
void ProfilingSystem::PopSample(const char*)
{
    if (IsDisabled()) 
    {
        return;
    }
    ASSERT_OR_DIE(m_activeSample != nullptr, "There was no active sample, attempted to pop the bottom of the stack. (Did you pop too many times?)");

    //Update the end time on the active sample
    m_activeSample->endCount = GetCurrentPerformanceCount();
    m_activeSample = m_activeSample->parent;
}

//-----------------------------------------------------------------------------------
ProfileSample* ProfilingSystem::GetLastFrame()
{
    return m_previousFrameRoot;
}

//-----------------------------------------------------------------------------------
bool ProfilingSystem::DeleteSampleTree(ProfileSample* root)
{
    if (root == nullptr)
    {
        return false;
    }
    else
    {
        while (root->children != nullptr)
        {
            ProfileSample* child = root->children;
            DeleteSampleTree(child);
            RemoveInPlace(root->children, child);
            m_sampleAllocator.Free(child);
        }
        return true;
    }
}

//-----------------------------------------------------------------------------------
void ProfilingSystem::PrintNodeListView(ProfileSample* root, unsigned int depth)
{
    std::string tag = Stringf("%s%s", std::string(depth, '-').c_str(), root->id);
    float msTaken = root->GetDurationInSeconds() * 1000.0f;
    float percentageTaken = (root->GetDurationInSeconds() / GetLastFrame()->GetDurationInSeconds()) * 100.0f;
    Console::instance->PrintLine(Stringf("%-30s%12i%12i%12i%10.02fms%10.02f%%\n", tag.c_str(), root->numDrawCalls, root->numAllocs, root->sizeAllocs, msTaken, percentageTaken), RGBA(root->startCount, 1.0f, root->endCount, 1.0f));

    ProfileSample* currentChild = root->children;
    while (currentChild != nullptr)
    {
        PrintNodeListView(currentChild, depth + 1);
        currentChild = currentChild->next;
        if (currentChild == root->children)
        {
            break;
        }
    }
}

//-----------------------------------------------------------------------------------
void ProfilingSystem::PrintTreeListView()
{
    if (m_previousFrameRoot)
    {
        Console::instance->PrintLine(Stringf("%-30s%12s%12s%12s%12s%11s", "TAG", "NUM DRAWS", "NUM ALLOCS", "SIZE ALLOCS", "TIME", "%FRAME"));
        PrintNodeListView(m_previousFrameRoot, 0);
    }
}

//-----------------------------------------------------------------------------------
uint64_t GetCurrentPerformanceCount()
{
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter(&currentCount);
    return currentCount.QuadPart;
}

//---------------------------------------------------------------------------
double InitializePerformanceTime(LARGE_INTEGER& out_initialTime)
{
    LARGE_INTEGER countsPerSecond;
    QueryPerformanceFrequency(&countsPerSecond);
    QueryPerformanceCounter(&out_initialTime);
    return(1.0 / static_cast<double>(countsPerSecond.QuadPart));
}

//-----------------------------------------------------------------------------------
double PerformanceCountToSeconds(uint64_t& performanceCount)
{
    static LARGE_INTEGER initialTime;
    static double secondsPerCount = InitializePerformanceTime(initialTime);
    LONGLONG elapsedCountsSinceInitialTime = performanceCount;

    double currentSeconds = static_cast<double>(elapsedCountsSinceInitialTime) * secondsPerCount;
    return currentSeconds;
}

//-----------------------------------------------------------------------------------
void ProfileReportNode::AddSample(double sampleTime)
{
    m_lastSample = sampleTime;
    m_minSample = (sampleTime < m_minSample) ? sampleTime : m_minSample;
    m_maxSample = (sampleTime > m_maxSample) ? sampleTime : m_maxSample;
// 	double currentRollingAverage = m_averageSample;
// 	double currentRollingAverageExpanded = currentRollingAverage * m_numSamples;
    m_averageSample *= 0.97;
    m_averageSample += (0.03 * m_lastSample);
    m_numSamples++;
// 	m_averageSample = (currentRollingAverageExpanded + m_lastSample) / m_numSamples;
}

//-----------------------------------------------------------------------------------
ProfileLogSection::ProfileLogSection(const char* id)
    : m_id(id)
    , m_start(GetCurrentPerformanceCount())
{

}

//-----------------------------------------------------------------------------------
ProfileLogSection::~ProfileLogSection()
{
    static const float secondsToMs = 1000.0f;
    m_end = GetCurrentPerformanceCount();
    uint64_t duration = m_end - m_start;
    LogPrintf("[%s]: took %.02f ms\n", m_id, PerformanceCountToSeconds(duration) * secondsToMs);
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(toggleprofiling)
{
    UNUSED(args);
    bool willBeEnabled = !ProfilingSystem::instance->IsEnabled();
    ProfilingSystem::instance->SetEnabled(willBeEnabled);
    if (willBeEnabled)
    {
        Console::instance->PrintLine("Profiling Enabled!", RGBA::GBLIGHTGREEN);
    }
    else
    {
        Console::instance->PrintLine("Profiling Disabled!", RGBA::RED);
    }
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(printprofiling)
{
    UNUSED(args);
    ProfilingSystem::instance->PrintTreeListView();
}

#else
ProfilingSystem::ProfilingSystem() : m_sampleAllocator(0) {}
ProfilingSystem::~ProfilingSystem() {}
void ProfilingSystem::StartNewFrame() {}
void ProfilingSystem::EndPreviousFrame() {}
bool ProfilingSystem::DeleteSampleTree(ProfileSample*) { return false; }
void ProfilingSystem::MarkFrame() {}
void ProfilingSystem::PushSample(const char*) {}
void ProfilingSystem::PopSample(const char*) {}
void ProfilingSystem::PrintTreeListView() {}
ProfileSample* ProfilingSystem::GetLastFrame() { return nullptr; }
uint64_t GetCurrentPerformanceCount() { return -1; }
double PerformanceCountToSeconds(uint64_t&) { return -1; }
ProfileLogSection::ProfileLogSection(const char* id) {};
ProfileLogSection::~ProfileLogSection() {};
#endif // PROFILING_ENABLED
