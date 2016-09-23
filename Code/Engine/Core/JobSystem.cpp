#include "Engine/Core/JobSystem.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Input/Logging.hpp"
#include <atomic>

JobSystem* JobSystem::instance = nullptr;
std::atomic<int> gThreadNumber = 0;

//-----------------------------------------------------------------------------------
void GenericJobThread()
{
    //The order we construct these in is the order we prioritize them.
    std::vector<JobType> types;
    if (++gThreadNumber % 2 == 0)
    {
        types.push_back(GENERIC_SLOW);
        types.push_back(GENERIC);
    }
    else
    {
        types.push_back(GENERIC);
        types.push_back(GENERIC_SLOW);
    }
    JobConsumer consumer = JobConsumer(types);

    while (JobSystem::instance->m_isRunning)
    {
        consumer.ConsumeAll();
        Sleep(100);
        SwitchToThread();
    }

    //In case we got a job in after yielding and cleaning up
    consumer.ConsumeAll();
}

//-----------------------------------------------------------------------------------
unsigned int GetCoreCount()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (unsigned int)sysinfo.dwNumberOfProcessors;
}

//-----------------------------------------------------------------------------------
JobSystem::JobSystem(int numExtraThreads)
    : m_isRunning(false)
    , m_jobAllocator(1024)
    , m_numberOfThreads(0)
{
    unsigned int numJobTypes = (unsigned int)JobType::NUM_TYPES;
    for (unsigned int i = 0; i < numJobTypes; ++i)
    {
        m_jobQueues.push_back(new JobQueue());
    }

    //Calculate number of desired threads
    m_numberOfThreads = CalculateNumThreads(numExtraThreads);
}

//-----------------------------------------------------------------------------------
JobSystem::~JobSystem()
{
    for (JobQueue* queue : m_jobQueues)
    {
        delete queue;
    }
}

//-----------------------------------------------------------------------------------
void JobSystem::Initialize()
{
    // Spin up the desired number of threads for our thread pool
    m_isRunning = true;
    for (unsigned int i = 0; i < m_numberOfThreads; ++i)
    {
        std::thread* thread = new std::thread(GenericJobThread);
        m_threadPool.push_back(thread);
    }
}

//-----------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
    m_isRunning = false;

    //Stop all threads running
    for (std::thread* thread : m_threadPool)
    {
        while (!thread->joinable());
        thread->join();
        delete thread;
    }

    //Carry out all remaining tasks synchronously. 
    //This will catch any jobs that were put into queues that had no consumers <3
    std::vector<JobType> allTypes;
    unsigned int numJobTypes = (unsigned int)JobType::NUM_TYPES;
    for (unsigned int i = 0; i < numJobTypes; ++i)
    {
        allTypes.push_back((JobType)i);
    }
    JobConsumer jobCleanup(allTypes);
    jobCleanup.ConsumeAll();
}

//-----------------------------------------------------------------------------------
Job* JobSystem::CreateJob(JobWorkFunction* jobWorkFunction, void* data, JobCallbackFunction* finishedCallback)
{
    Job* newJob = m_jobAllocator.Alloc();
    newJob->workFunction = jobWorkFunction;
    newJob->data = data;
    newJob->finishedCallback = finishedCallback;
    return newJob;
}

//-----------------------------------------------------------------------------------
void JobSystem::DispatchJob(JobType jobType, Job* jobToDispatch)
{
    m_jobQueues[jobType]->Enqueue(jobToDispatch);
}

//-----------------------------------------------------------------------------------
void JobSystem::CreateAndDispatchJob(JobType jobType, JobWorkFunction* jobWorkFunction, void* data, JobCallbackFunction* finishedCallback /*= nullptr*/)
{
    DispatchJob(jobType, CreateJob(jobWorkFunction, data, finishedCallback));
}

//-----------------------------------------------------------------------------------
void JobSystem::ReleaseJob(Job* finishedJob)
{
    m_jobAllocator.Free(finishedJob);
}

//-----------------------------------------------------------------------------------
int JobSystem::CalculateNumThreads(int numThreads)
{
    int numCores = GetCoreCount();
    if (numThreads > 0)
    {
        return numThreads;
    }
    else
    {
        int numThreadsToUse = numThreads + numCores;
        return numThreadsToUse < 1 ? 1 : numThreadsToUse;
    }
}

//-----------------------------------------------------------------------------------
JobConsumer::JobConsumer(const std::vector<JobType>& subscribedQueues)
{
    for (JobType type : subscribedQueues)
    {
        m_subscribedJobQueues.push_back(JobSystem::instance->m_jobQueues[(unsigned int)type]);
    }
}

//-----------------------------------------------------------------------------------
JobConsumer::~JobConsumer()
{

}

//-----------------------------------------------------------------------------------
bool JobConsumer::Consume()
{
    Job* job;
    for (JobQueue* queue : m_subscribedJobQueues)
    {
        job = queue->Dequeue();
        if (job) 
        {
            job->DoWork();
            JobSystem::instance->ReleaseJob(job);
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------------
void JobConsumer::ConsumeAll()
{
    while (Consume());
}

//-----------------------------------------------------------------------------------
void JobConsumer::ConsumeForMilliseconds(unsigned int ms)
{
    double startTime = GetCurrentTimeMilliseconds();
    double currentTime = startTime;
    while (currentTime - startTime < ms && Consume())
    {
        currentTime = GetCurrentTimeMilliseconds();
    }
}

//-----------------------------------------------------------------------------------
void Job::DoWork()
{
    ASSERT_OR_DIE(workFunction != nullptr, "Work function was null for a job.");
    workFunction(this);
    if (finishedCallback != nullptr)
    {
        finishedCallback(this);
    }
}
