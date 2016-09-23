#pragma once
#include "Engine/DataStructures/ThreadSafeQueue.hpp"
#include "Engine/DataStructures/ObjectPool.hpp"
#include <vector>
#include <thread>

//GLOBAL FUNCTIONS/////////////////////////////////////////////////////////////////////
unsigned int GetCoreCount();

struct Job;
typedef ThreadSafeQueue<Job> JobQueue;
typedef void(JobWorkFunction)(Job* job);
typedef void(JobCallbackFunction)(Job* job);

//-----------------------------------------------------------------------------------
struct Job
{
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    Job() : workFunction(nullptr), data(nullptr), finishedCallback(nullptr) {};

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void DoWork();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    JobWorkFunction* workFunction;
    JobCallbackFunction* finishedCallback;
    void* data;
};


//-----------------------------------------------------------------------------------
enum JobType
{
    GENERIC = 0,
    GENERIC_SLOW,
    NUM_TYPES
};

//-----------------------------------------------------------------------------------
class JobSystem
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    JobSystem(int numExtraThreads);
    ~JobSystem();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Initialize();
    void Shutdown();
    Job* CreateJob(JobWorkFunction* jobWorkFunction, void* data, JobCallbackFunction* finishedCallback = nullptr);
    void DispatchJob(JobType jobType, Job* jobToDispatch);
    void CreateAndDispatchJob(JobType jobType, JobWorkFunction* jobWorkFunction, void* data, JobCallbackFunction* finishedCallback = nullptr);
    void ReleaseJob(Job* finishedJob);

    //STATIC VARIABLES/////////////////////////////////////////////////////////////////////
    static JobSystem* instance;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    bool m_isRunning;
    std::vector<JobQueue*> m_jobQueues; // one per JobType

private:
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    int CalculateNumThreads(int numThreads);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<std::thread*> m_threadPool;
    ObjectPool<Job> m_jobAllocator;
    unsigned int m_numberOfThreads;
};

//-----------------------------------------------------------------------------------
class JobConsumer
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    JobConsumer(const std::vector<JobType>& subscribedQueues);
    ~JobConsumer();
    
    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    bool Consume();
    void ConsumeAll();
    void ConsumeForMilliseconds(unsigned int ms);

private:
    std::vector<JobQueue*> m_subscribedJobQueues;

};