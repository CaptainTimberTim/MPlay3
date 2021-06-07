/* date = October 13th 2020 9:33 am */
#ifndef _ALLOCATOR__T_D_H
#define _ALLOCATOR__T_D_H

#if DEBUG_TD // INFO:: Debug data for arenas
#define MAX_DEBUG_ARENA_COUNT 80
struct arena_debug_data
{
    struct arena_allocator *Parent;
    
    u32 FrameCount;
    
    u32 AllocationCount;
    u32 PrevAllocationCount; // Last frame
    u32 FreeCount;
    
    u32 PrivateAllocationCount;
    
    u64 MaxAllocationSize;
    
    u64 HighestAllocationCountInFrame;
    u64 LowestAllocationCountInFrame;
    
    u32 MaxArenaCount;
    r32 MaxFillPercentage[MAX_DEBUG_ARENA_COUNT];
    r32 CurrentFillP[MAX_DEBUG_ARENA_COUNT];
    
    u32 ArenaCreationCount;
    u32 ArenaFreeCount;
};

#endif

#define ARENA_BASE_SIZE Megabytes(64)
struct arena
{
    struct arena *Prev;
    struct arena *Next;
    
    u32 Count;
    u64 Position;
    u64 Size;
    u8 *Memory;
};

enum arena_flags
{
    arenaFlags_IsTransient = 1<<0,
    arenaFlags_IsThreaded  = 1<<1,
};

// ZII: Zero is initialization!
// Means that creating this struct with everything zero 
// and just starting to use it is the intended way. 
// No addidional initialization is needed.
struct arena_allocator
{
    i32 Flags;
    u32 ArenaBaseSize;
    u32 MaxEmptyArenaCount;
    
    u32 EmptyArenaCount;
    
    u32 ArenaCount;
    arena *Base;
    arena *LastUsed;
    
    HANDLE Mutex; // Only used when Flag IsThreaded is set.
    
#if DEBUG_TD
    struct arena_debug_data DebugData;
#endif
};
#define CreateArenaAllocator() {}

inline void FreeMemory(arena_allocator *Allocator, void *Memory);

// TODO:: Switch to i64 to catch uninitialized stuff?!

// This can be called:
// 1. Without VA_ARGS argument, then it is normal memory allocation
// 2. With the keyword Private, then it becomes private allocation
// Each procdeure is briefly described below.
#define AllocateMemory(Allocator, Size,        ...) (u8 *)  AllocateMemory_##__VA_ARGS__(Allocator, Size)
#define AllocateArray(Allocator, Count, Type,  ...) (Type *)AllocateMemory_##__VA_ARGS__(Allocator, (Count)*sizeof(Type))
#define AllocateStruct(Allocator, Type,        ...) (Type *)AllocateMemory_##__VA_ARGS__(Allocator, sizeof(Type))

#define ReallocateMemory(Allocator, Memory, OldSize, NewSize, ...) \
(u8 *)ReallocateMemory_##__VA_ARGS__(Allocator, Memory, OldSize, NewSize)
#define ReallocateArray(Allocator, Memory, OldCount, NewCount, Type, ...) \
(Type *)ReallocateMemory_##__VA_ARGS__(Allocator, Memory, (OldCount)*sizeof(Type), (NewCount)*sizeof(Type))

// Normal arena allocations. When created is persistent until DeleteMemory is called.
internal void *AllocateMemory_(arena_allocator *Allocator, u64 Size);
internal void *ReallocateMemory_(arena_allocator *Allocator, void *Memory, u64 OldSize, u64 NewSize);

// Allocates a new arena that is exactly the given size. This is mainly useful for
// allocating big memory blocks that _might_ get deleted at some point and 
// having the whole arena empty again and not locking a big chunk of memory, 
// because of some small persistent allocation. 
internal void *AllocateMemory_Private(arena_allocator *Allocator, u64 Size);
internal void *ReallocateMemory_Private(arena_allocator *Allocator, void *Memory, u64 OldSize, u64 NewSize);

internal void ResetMemoryArena(arena_allocator *Allocator);

#define ClearMemory(Memory, Size)       Clear_(Memory, Size)
#define ClearArray(Memory, Count, Type) Clear_(Memory, (Count)*sizeof(Type))
inline void Clear_(void *Memory, u64 Size);







#endif //_ALLOCATOR__T_D_H
