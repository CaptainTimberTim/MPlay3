/* date = October 13th 2020 9:33 am */
#ifndef _ALLOCATOR__T_D_H
#define _ALLOCATOR__T_D_H

#define ARENA_BASE_SIZE Megabytes(256)
#define MAX_EMPTY_ARENAS 2
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
    
    arena *Base;
    arena *LastUsed;
    
    arena *TransientBase;
};
#define CreateArenaAllocator() {}

inline   void  DeleteMemory(arena_allocator *Allocator, void *Memory);

internal void *AllocateMemory_(arena_allocator *Allocator, u64 Size);
// Normal arena allocations. When created is persistent until DeleteMemory is called.
#define AllocateMemory(Allocator, Size)         (u8 *)AllocateMemory_(Allocator, Size)
#define AllocateArray(Allocator, Count, Type) (Type *)AllocateMemory_(Allocator, Count*sizeof(Type))
#define AllocateStruct(Allocator, Type)       (Type *)AllocateMemory_(Allocator, sizeof(Type))

// Allocates a new arena that is exactly the given size. This is mainly useful for
// allocating big memory blocks that _might_ get deleted at some point and 
// having the whole arena empty again and not locking a big chunk of memory, 
// because of some small persistent allocation. 
internal void *AllocatePrivateMemory_(arena_allocator *Allocator, u64 Size);
#define AllocatePrivateMemory(Allocator, Size)         (u8 *)AllocatePrivateMemory_(Allocator, Size)
#define AllocatePrivateArray(Allocator, Count, Type) (Type *)AllocatePrivateMemory_(Allocator, Count*sizeof(Type))
#define AllocatePrivateStruct(Allocator, Type)       (Type *)AllocatePrivateMemory_(Allocator, sizeof(Type))

internal void *AllocateTransientMemory_(arena_allocator *Allocator, u64 Size);
// Transient arena allocations. Creates temporary memory, that is _only_ valid until ResetTransientAllocator is called.
#define AllocateTransientMemory(Allocator, Size)         (u8 *)AllocateTransientMemory_(Allocator, Size)
#define AllocateTransientArray(Allocator, Count, Type) (Type *)AllocateTransientMemory_(Allocator, Count*sizeof(Type))
#define AllocateTransientStruct(Allocator, Type)       (Type *)AllocateTransientMemory_(Allocator, sizeof(Type))
internal void ResetTransientAllocator(arena_allocator *Allocator);









#endif //_ALLOCATOR__T_D_H
