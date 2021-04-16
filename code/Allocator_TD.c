#include "Allocator_TD.h"

#include "Math_TD.h"

inline arena *
CreateNewArena(u64 Size)
{
    u64 AdjustedSize = Size;
    
    AdjustedSize += sizeof(arena);
    u8 *Memory = (u8 *)VirtualAlloc(0, AdjustedSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Assert(Memory);
    
    arena *Arena  = (arena *)Memory;
    Arena->Memory = Memory + sizeof(arena);
    Arena->Size   = AdjustedSize - sizeof(arena);
    
    return Arena;
}

inline HANDLE
NewMutex()
{
    return CreateMutexEx(0, 0, 0, MUTEX_ALL_ACCESS);
}

inline void
Lock(HANDLE Mutex)
{
    WaitForSingleObjectEx(Mutex, INFINITE, false);
}

inline void
Unlock(HANDLE Mutex)
{
    ReleaseMutex(Mutex);
}

inline void
FreeMemory(arena_allocator *Allocator, void *Memory)
{
    if(!Memory) return;
    if(Allocator->Flags & arenaFlags_IsThreaded) Lock(Allocator->Mutex);
    arena *Arena = Allocator->Base;
    
    Assert(Allocator->ArenaCount != 0);
    For(Allocator->ArenaCount)
    {
        if(Memory >= Arena->Memory && Memory < (Arena->Memory+Arena->Size))
        {
            Arena->Count--;
            Assert(Arena->Count >= 0);
            if(Arena->Count == 0) // If we completely emptied the arena, we can reset it!
            {
                Arena->Position = 0;
                Assert(Allocator->EmptyArenaCount >= 0);
                // If we have more than the given limit or the arena size is small, delete it.
                if(Allocator->EmptyArenaCount == Allocator->MaxEmptyArenaCount || Arena->Size < ARENA_BASE_SIZE/2) 
                {
                    if(Arena->Prev && Arena->Next) // Arena is in the middle.
                    {
                        Arena->Prev->Next = Arena->Next;
                        Arena->Next->Prev = Arena->Prev;
                    }
                    else if(Arena->Next) // Arena is the first one.
                    {
                        Arena->Next->Prev = 0;
                        Allocator->Base = Arena->Next;
                    }
                    else if(Arena->Prev) Arena->Prev->Next = 0; // Arena is the last one.
                    
                    Allocator->ArenaCount--;
                    Assert(Allocator->ArenaCount >= 0);
                    if(Allocator->ArenaCount == 0) Allocator->Base = 0;
                    
                    VirtualFree(Arena->Memory, 0, MEM_RELEASE);
                    Allocator->LastUsed = 0;
#if DEBUG_TD
                    Allocator->DebugData.ArenaFreeCount++;
#endif
                }
                else Allocator->EmptyArenaCount++;
                
                
#if DEBUG_TD
                Allocator->DebugData.FreeCount++;
                arena *DebugArena = Allocator->Base;
                For(Allocator->ArenaCount)
                {
                    r32 FillP = DebugArena->Position/(r32)DebugArena->Size;
                    Allocator->DebugData.CurrentFillP[It] = FillP;
                    DebugArena = DebugArena->Next;
                }
#endif
                
            }
            break;
        }
        Arena = Arena->Next;
    }
    Assert(Allocator->ArenaCount == 0 || Arena); // Given memory was not created with given allocator!
    if(Allocator->Flags & arenaFlags_IsThreaded) Unlock(Allocator->Mutex);
}

inline void
AppendArena(arena **AppendTo, arena *Arena)
{
    arena *LastArena = *AppendTo;
    if(LastArena)
    {
        while(LastArena->Next) LastArena = LastArena->Next;
        LastArena->Next = Arena;
        Arena->Prev = LastArena;
    }
    else *AppendTo = Arena;
}

inline void *
RetrieveMemoryFromArena(arena *Arena, u64 Size)
{
    Assert(Arena);
    Assert(Arena->Memory != 0);
    Assert(Arena->Position+Size <= Arena->Size);
    
    void *Result = Arena->Memory+Arena->Position;
    Arena->Position += Size;
    Arena->Count++;
    
    return Result;
}

internal void *
AllocateMemory_(arena_allocator *Allocator, u64 Size)
{
    void *Result = 0;
    
    if(Allocator->Flags & arenaFlags_IsThreaded) 
    {
        if(!Allocator->Mutex) Allocator->Mutex = NewMutex();
        Lock(Allocator->Mutex);
    }
    
    // For performance we save the last used arena as most of the time the new
    // allocation will fit in it.
    if(Allocator->LastUsed && (Allocator->LastUsed->Position+Size) < Allocator->LastUsed->Size)
    {
        if(Allocator->LastUsed->Position == 0) Allocator->EmptyArenaCount--;
        Result = RetrieveMemoryFromArena(Allocator->LastUsed, Size);
    }
    else
    {
        arena *Arena = Allocator->Base;
        
        Assert(Allocator->ArenaCount == 0 || Arena);
        For(Allocator->ArenaCount)
        {
            if((Arena->Position+Size) < Arena->Size)
            {
                if(Arena->Position == 0) Allocator->EmptyArenaCount--;
                Result = RetrieveMemoryFromArena(Arena, Size);
                break;
            }
            Arena = Arena->Next;
        }
        
        if(!Result) // If we didn't find any open arenas, we make a new one.
        {
            u64 ArenaSize = Max(Size, Allocator->ArenaBaseSize ? Allocator->ArenaBaseSize : ARENA_BASE_SIZE);
            Arena = CreateNewArena(ArenaSize);
            AppendArena(&Allocator->Base, Arena);
            Allocator->ArenaCount++;
            
            Assert(Arena->Position == 0);
            
            Result = RetrieveMemoryFromArena(Arena, Size);
            
#if DEBUG_TD
            Allocator->DebugData.ArenaCreationCount++;
#endif
        }
        Allocator->LastUsed = Arena;
    }
    
    ClearMemory(Result, Size); // TODO:: Think about to, at least, ask to _not_ clear memory.
    Assert(Allocator->EmptyArenaCount >= 0);
    
#if DEBUG_TD
    Allocator->DebugData.AllocationCount++;
    Allocator->DebugData.MaxAllocationSize = Max(Allocator->DebugData.MaxAllocationSize, Size);
    Allocator->DebugData.MaxArenaCount = Max(Allocator->DebugData.MaxArenaCount, Allocator->ArenaCount);
    arena *DebugArena = Allocator->Base;
    For(Allocator->ArenaCount)
    {
        r32 FillP = DebugArena->Position/(r32)DebugArena->Size;
        Allocator->DebugData.MaxFillPercentage[It] = Max(Allocator->DebugData.MaxFillPercentage[It], FillP);
        Allocator->DebugData.CurrentFillP[It] = FillP;
        DebugArena = DebugArena->Next;
    }
#endif
    
    if(Allocator->Flags & arenaFlags_IsThreaded) Unlock(Allocator->Mutex);
    return Result;
}

internal void *
ReallocateMemory_(arena_allocator *Allocator, void *Memory, u64 OldSize, u64 NewSize)
{
    void *Result = 0;
    
    Result = AllocateMemory_(Allocator, NewSize);
    for(u32 It = 0; It < OldSize && It < NewSize; It++) 
    {
        ((u8 *)Result)[It] = ((u8 *)Memory)[It];
    }
    FreeMemory(Allocator, Memory);
    
    return Result;
}

internal void 
ResetMemoryArena(arena_allocator *Allocator)
{
    if(Allocator->Flags & arenaFlags_IsThreaded) Lock(Allocator->Mutex);
    
    Assert(Allocator->Flags & arenaFlags_IsTransient);
    arena *Arena = Allocator->Base;
    if(Arena)
    {
        while(Arena->Next) Arena = Arena->Next;
        while(Arena)
        {
            arena *Prev = Arena->Prev;
            
            if(Prev) VirtualFree(Arena->Memory, 0, MEM_RELEASE);
            else
            {
                Arena->Position = 0;
                Arena->Count    = 0;
                Arena->Next     = 0;
            }
            Arena = Prev;
        }
        Allocator->ArenaCount      = 1;
        Allocator->EmptyArenaCount = 1;
        Allocator->LastUsed        = Allocator->Base;
    }
    
    if(Allocator->Flags & arenaFlags_IsThreaded) Unlock(Allocator->Mutex);
}

internal void *
AllocateMemory_Private(arena_allocator *Allocator, u64 Size)
{
    void *Result = 0;
    
    if(Allocator->Flags & arenaFlags_IsThreaded) Lock(Allocator->Mutex);
    
    arena *Arena = CreateNewArena(Size);
    AppendArena(&Allocator->Base, Arena);
    Allocator->ArenaCount++;
    
    Assert(Arena->Position == 0);
    Result = RetrieveMemoryFromArena(Arena, Size);
    
    if(Allocator->Flags & arenaFlags_IsThreaded) Unlock(Allocator->Mutex);
    
#if DEBUG_TD
    Allocator->DebugData.AllocationCount++;
    Allocator->DebugData.PrivateAllocationCount++;
    Allocator->DebugData.MaxAllocationSize = Max(Allocator->DebugData.MaxAllocationSize, Size);
    Allocator->DebugData.MaxArenaCount = Max(Allocator->DebugData.MaxArenaCount, Allocator->ArenaCount);
    Allocator->DebugData.ArenaCreationCount++;
#endif
    
    return Result;
}

internal void *
ReallocateMemory_Private(arena_allocator *Allocator, void *Memory, u64 OldSize, u64 NewSize)
{
    void *Result = 0;
    
    Result = AllocateMemory_Private(Allocator, NewSize);
    for(u32 It = 0; It < OldSize && It < NewSize; It++) 
    {
        ((u8 *)Result)[It] = ((u8 *)Memory)[It];
    }
    FreeMemory(Allocator, Memory);
    
    return Result;
}


inline void 
Clear_(void *Memory, u64 Size)
{
    memset(Memory, 0, Size);
}




#if DEBUG_TD

internal void // Call only once per frame for each arena!
CollectArenaDebugFrameData(arena_debug_data *Data)
{
    Data->FrameCount++;
    
    u64 AllocationDiff = Data->AllocationCount - Data->PrevAllocationCount;
    Data->PrevAllocationCount = Data->AllocationCount;
    
    Data->HighestAllocationCountInFrame = Max(Data->HighestAllocationCountInFrame, AllocationDiff);
    Data->LowestAllocationCountInFrame = Max(Data->LowestAllocationCountInFrame, AllocationDiff);
}


#endif






















