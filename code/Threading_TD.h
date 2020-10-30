#pragma once

// TODO:: mm_sfence might not be the right instruction!
// INFO:: First: Compilerbarrier, second: processorbarrier
#define CompletePastWritesBeforeFutureWrites _WriteBarrier(); _mm_sfence(); 
#define CompletePastReadsBeforeFutureReads  _ReadBarrier();

// INFO:: Thread safe instructions: 
// - volatile - tells compiler that somebody at anytime can change this. So do a load everytime
// - InterlockedIncrement(LONG volatile *Value) - Increments given value
// - semaphore - a counter that helps keeping track of thread status sleep/awake
// - CreateSemaphoreEx() - returns handle that can be used for synchronisation functions (bottom msdn page)(access type: SEMAPHORE_ALL_ACCESS)
//      - WaitForSingleObjectEX - suspends thread the specified amount of time, until semaphore handle "triggers" 
//      - ReleaseSemaphore - Inreases semaphore count by specified amount, triggers for waitforsingle. When thread is released, it is decremented by 1
#define THREAD_COUNT 1

#define JOB_LIST_CALLBACK(name) void name(struct job_thread_info *ThreadInfo, void *Data)
typedef JOB_LIST_CALLBACK(job_list_callback);

#define JOB_ENTRY_DATA_SIZE 8*10
struct job_queue_entry
{
    job_list_callback *Callback;
    u8 Data[JOB_ENTRY_DATA_SIZE];
};

enum thread_state
{
    threadState_Inactive,
    threadState_Running,
    threadState_Finished,
};

#define MAX_ACTIVE_JOBS 256
struct circular_job_queue
{
    HANDLE Semaphore;
    u32 volatile CompletionGoal;
    u32 volatile CompletionCount;
    u32 volatile NextJobToWrite;
    u32 volatile NextJobToRead;
    job_queue_entry Entries[MAX_ACTIVE_JOBS];
};

struct job_thread_info
{
    u32 ThreadID;
    circular_job_queue *JobQueue;
    arena_allocator ScratchArena;
    
    job_list_callback *CurrentJob;
};


// ***********************************
// API *******************************
// ***********************************
internal void AddJobToQueue(circular_job_queue *JobQueue, job_list_callback *Callback, void *Data);


// Example work callback function: 
// internal JOB_LIST_CALLBACK(DoJobWork)
// {
//    Do Work
//    char *const String = (char *const)Data;
//    DebugLog(255, String, GetCurrentThreadId());
// }


// Sound Thread 

#define SOUND_THREAD_CALLBACK(name) void name(struct sound_thread *ThreadInfo)
typedef SOUND_THREAD_CALLBACK(sound_thread_callback);

#define SOUND_THREAD_DATA_SIZE 8*10
struct sound_thread_data
{
    sound_thread_callback *Callback;
    struct sound_thread *SoundThreadInfo;
};




