/* date = October 13th 2020 9:33 am */
#ifndef _ALLOCATOR__T_D_H
#define _ALLOCATOR__T_D_H

#define ARENA_BASE_SIZE Megabytes(256)
struct arena
{
    struct arena *Prev;
    struct arena *Next;
    
    u32 Count;
    u64 Position;
    u64 Size;
    u8 *Memory;
};

// ZII: Zero is initialization!
// Means that creating this struct with everything zero 
// and just starting to use it is the intended way. 
// No addidional initialization is needed.
struct arena_allocator
{
    u32 ArenaCount;
    u32 EmptyArenaCount;
    u32 MaxEmptyArenaCount; // Zero leaves one empty arena!
    
    arena *Base;
    arena *LastUsed;
};
#define CreateArenaAllocator() {}

inline void FreeMemory(arena_allocator *Allocator, void *Memory);

// This can be called:
// 1. Without VA_ARGS argument, then it is normal memory allocation
// 2. With the keyword Private, then it becomes private allocation
// Each procdeure is briefly described below.
#define AllocateMemory(Allocator, Size,        ...) (u8 *)  AllocateMemory_##__VA_ARGS__(Allocator, Size)
#define AllocateArray (Allocator, Count, Type, ...) (Type *)AllocateMemory_##__VA_ARGS__(Allocator, (Count)*sizeof(Type))
#define AllocateStruct(Allocator, Type,        ...) (Type *)AllocateMemory_##__VA_ARGS__(Allocator, sizeof(Type))


// Normal arena allocations. When created is persistent until DeleteMemory is called.
internal void *AllocateMemory_(arena_allocator *Allocator, u64 Size);

// Allocates a new arena that is exactly the given size. This is mainly useful for
// allocating big memory blocks that _might_ get deleted at some point and 
// having the whole arena empty again and not locking a big chunk of memory, 
// because of some small persistent allocation. 
internal void *AllocateMemory_Private(arena_allocator *Allocator, u64 Size);

internal void ResetMemoryArena(arena_allocator *Allocator);









#endif //_ALLOCATOR__T_D_H
