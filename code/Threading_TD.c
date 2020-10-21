#include "Threading_TD.h"

#define AddJobToQueue(Queue, Callback, DataStruct) AddJobToQueue_(Queue, Callback, &(DataStruct), sizeof(DataStruct))
internal void 
AddJobToQueue_(circular_job_queue *JobQueue, job_list_callback *Callback, void *Data, u32 DataSize)
{
    // NOTE:: If you want multiple producer; multiple worker, then you need to
    // switch to interlockedCompareExchange. With single producer its not 
    // necessery.
    u32 NewNextEntryToWrite = (JobQueue->NextJobToWrite+1)%MAX_ACTIVE_JOBS;
    Assert(NewNextEntryToWrite != JobQueue->NextJobToRead);
    job_queue_entry *Entry = JobQueue->Entries + JobQueue->NextJobToWrite;
    Entry->Callback = Callback;
    
    Assert(DataSize <= JOB_ENTRY_DATA_SIZE);
    For(DataSize) Entry->Data[It] = ((u8 *)Data)[It];
    
    JobQueue->CompletionGoal++;
    
    CompletePastWritesBeforeFutureWrites;
    //InterlockedIncrement((LONG volatile *)&JobQueue->NextJobToWrite);
    JobQueue->NextJobToWrite = NewNextEntryToWrite;
    
    // Activates one sleeping thread
    ReleaseSemaphore(JobQueue->Semaphore, 1, 0);
}

internal b32
DoNextJobQueueEntry(job_thread_info *Info)
{
    b32 WeShouldSleep = false;
    
    circular_job_queue *JobQueue = Info->JobQueue;
    
    u32 OriginalNextJobToRead = JobQueue->NextJobToRead;
    u32 NewNextEntryToRead = (OriginalNextJobToRead+1)%MAX_ACTIVE_JOBS;
    if(OriginalNextJobToRead != JobQueue->NextJobToWrite)
    {
        u32 Index = InterlockedCompareExchange((LONG volatile *)&JobQueue->NextJobToRead, 
                                               NewNextEntryToRead, 
                                               OriginalNextJobToRead);
        if(Index == OriginalNextJobToRead)
        {
            job_queue_entry Entry = JobQueue->Entries[Index];
            ResetMemoryArena(&Info->ScratchArena);
            Info->CurrentJob = Entry.Callback;
            
            Entry.Callback(Info, Entry.Data);
            
            Info->CurrentJob = 0;
            InterlockedIncrement((LONG volatile *)&JobQueue->CompletionCount);
        }
    }
    else 
    {
        // We should only sleep if OriginalNextJobToRead and JobQueue->NextJobToWrite
        // are equal. Not if we actually do a job and not if we failed to get 
        // a job, but thought there was one.
        WeShouldSleep = true;
    }
    
    return WeShouldSleep;
}

internal DWORD WINAPI
JobThreadProc(LPVOID Data)
{
    DWORD Result = 0;
    job_thread_info *Info = (job_thread_info *)Data;
    
    while(true)
    {
        if(DoNextJobQueueEntry(Info))
        {
            // Sleep
            WaitForSingleObjectEx(Info->JobQueue->Semaphore, INFINITE, false);
        }
    }
    
    return Result;
}

inline void
EmptyJobQueueWhenPossible(circular_job_queue *JobQueue)
{
    if(JobQueue->CompletionGoal == JobQueue->CompletionCount &&
       JobQueue->CompletionGoal && JobQueue->CompletionCount)
    {
        JobQueue->CompletionGoal  = 0;
        JobQueue->CompletionCount = 0;
        DebugLog(255, "Successfully reset circular job queue counter!\n");
    }
}

internal void
InitializeJobThreads(HANDLE *JobHandles, circular_job_queue *JobQueue, job_thread_info *JobThreadInfos)
{
    JobQueue->Semaphore = CreateSemaphoreEx(0, 0, THREAD_COUNT, 0, 0, SEMAPHORE_ALL_ACCESS);
    
    For(THREAD_COUNT)
    {
        JobThreadInfos[It].ThreadID = It;
        JobThreadInfos[It].JobQueue = JobQueue;
        JobThreadInfos[It].ScratchArena = {arenaFlags_IsTransient};
        
        JobHandles[It] = CreateThread(0, 0, JobThreadProc, (LPVOID)(JobThreadInfos+It), 0, 0);
    }
}

internal b32
FindJobThreadStopAndRestartIt(HANDLE *ThreadHandles, job_thread_info *ThreadInfos, job_list_callback *JobCallback)
{
    b32 Result = false;
    
    For(THREAD_COUNT)
    {
        if(ThreadInfos[It].CurrentJob)
        {
            if(*ThreadInfos[It].CurrentJob == JobCallback)
            {
                Assert(TerminateThread(ThreadHandles[It], 0));
                
                ThreadInfos[It].CurrentJob = 0;
                ThreadHandles[It] = CreateThread(0, 0, JobThreadProc, (LPVOID)(ThreadInfos+It), 0, 0);
                
                Result = true;
            }
        }
    }
    
    return Result;
}

// Test code
internal JOB_LIST_CALLBACK(DoJobWork)
{
    // Do Work
    string_c *String = (string_c *)Data;
    DebugLog(255, (char *const)String->S, ThreadInfo->ThreadID);
}


// Sound Thread

inline LONGLONG GetWallClock();
inline r32 GetSecondsElapsed(i64 PerfCountFrequency, LONGLONG Start, LONGLONG End);

internal DWORD WINAPI
SoundThreadProc(LPVOID Data)
{
    sound_thread_data *ThreadData = (sound_thread_data *)Data;
    ThreadData->Callback(ThreadData->SoundThreadInfo);
    
    return 0;
}

internal void
InitializeSoundThread(sound_thread_data *SoundThreadData, sound_thread_callback *Callback, struct sound_thread *Data)
{
    
    SoundThreadData->Callback = Callback;
    SoundThreadData->SoundThreadInfo = Data;
    
    CreateThread(0, 0, SoundThreadProc, (LPVOID)SoundThreadData, 0, 0);
}

















