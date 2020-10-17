#include "Allocator_TD.h"

inline arena *
CreateNewArena(u64 Size, b32 IsPrivate = false)
{
    u64 AdjustedSize = IsPrivate ? Size : Max(Size, ARENA_BASE_SIZE);
    
    AdjustedSize += sizeof(arena);
    u8 *Memory = (u8 *)VirtualAlloc(0, AdjustedSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    Assert(Memory);
    
    arena *Arena  = (arena *)Memory;
    Arena->Memory = Memory + sizeof(arena);
    Arena->Size   = AdjustedSize - sizeof(arena);
    
    return Arena;
}

inline void
DeleteMemory(arena_allocator *Allocator, void *Memory)
{
    arena *Arena = Allocator->Base;
    
    For(Allocator->ArenaCount)
    {
        if(Memory >= Arena->Memory && Memory < (Arena->Memory+Arena->Size))
        {
            Arena->Count--;
            Assert(Arena->Count >= 0);
            if(Arena->Count == 0) // If we completely emptied the arena, we can reset it!
            {
                Arena->Position = 0;
                Assert(Allocator->EmptyArenaCount <= MAX_EMPTY_ARENAS);
                // If we have more than the given limit or the arena size is small, delete it.
                if(Allocator->EmptyArenaCount == MAX_EMPTY_ARENAS ||
                   Arena->Size < ARENA_BASE_SIZE/2) 
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
                    else  Arena->Prev->Next = 0; // Arena is the last one.
                    VirtualFree(Arena->Memory, 0, MEM_RELEASE);
                }
                else Allocator->EmptyArenaCount++;
            }
            break;
        }
        Arena = Arena->Next;
    }
    Assert(Allocator->ArenaCount == 0 || Arena); // Given memory was not created with given allocator!
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

internal void *
AllocateMemory_(arena_allocator *Allocator, u64 Size)
{
    void *Result = 0;
    
    // For performance we save the last used arena as most of the time the new
    // allocation will fit in it.
    if(Allocator->LastUsed && (Allocator->LastUsed->Position+Size) < Allocator->LastUsed->Size)
    {
        Result = Allocator->LastUsed->Memory+Allocator->LastUsed->Position;
        Allocator->LastUsed->Position += Size;
        Allocator->LastUsed->Count++;
    }
    else
    {
        arena *Arena = Allocator->Base;
        
        Assert(Allocator->ArenaCount == 0 || Arena);
        For(Allocator->ArenaCount)
        {
            if((Arena->Position+Size) < Arena->Size)
            {
                Result = Arena->Memory+Arena->Position;
                Arena->Position += Size;
                Arena->Count++;
                break;
            }
            Arena = Arena->Next;
        }
        
        if(!Result) // If we didn't find any open arenas, we make a new one.
        {
            Arena = CreateNewArena(Size);
            AppendArena(&Allocator->Base, Arena);
            Allocator->ArenaCount++;
            
            Assert(Arena);
            Assert(Arena->Position == 0);
            Assert(Arena->Memory != 0);
            
            Result = Arena->Memory;
            Arena->Position += Size;
            Arena->Count++;
        }
        Allocator->LastUsed = Arena;
    }
    return Result;
}

internal void 
ResetTransientAllocator(arena_allocator *Allocator)
{
    arena *Arena = Allocator->TransientBase;
    if(Arena)
    {
        while(Arena->Next) Arena = Arena->Next;
        while(Arena)
        {
            Arena->Position = 0;
            Arena->Count    = 0;
            Arena->Next = 0;
            arena *Prev = Arena->Prev;
            Arena->Prev = 0;
            
            if(Prev) VirtualFree(Arena->Memory, 0, MEM_RELEASE);
            Arena = Prev;
        }
    }
}

internal void *
AllocateTransientMemory_(arena_allocator *Allocator, u64 Size)
{
    void *Result = 0;
    
    arena *Arena = Allocator->TransientBase;
    
    while(Arena)
    {
        if((Arena->Position+Size) < Arena->Size)
        {
            Result = Arena->Memory+Arena->Position;
            Arena->Position += Size;
            Arena->Count++;
            break;
        }
        Arena = Arena->Next;
    }
    
    if(!Result) // If we didn't find any open arenas, we make a new one.
    {
        Arena = CreateNewArena(Size);
        
        AppendArena(&Allocator->TransientBase, Arena);
        
        Assert(Arena);
        Assert(Arena->Position == 0);
        Assert(Arena->Memory != 0);
        
        Result = Arena->Memory;
        Arena->Position += Size;
        Arena->Count++;
    }
    return Result;
}

internal void *
AllocatePrivateMemory_(arena_allocator *Allocator, u64 Size)
{
    void *Result = 0;
    
    arena *Arena = CreateNewArena(Size, true);
    AppendArena(&Allocator->Base, Arena);
    Allocator->ArenaCount++;
    Assert(Arena);
    Assert(Arena->Position == 0);
    Assert(Arena->Memory != 0);
    
    Result = Arena->Memory;
    Arena->Position += Size;
    Arena->Count++;
    
    return Result;
}