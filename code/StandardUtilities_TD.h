#pragma once
#include "Math_TD.h"

// HashTable
#define HASH_TABLE_KEY_LENGTH 30
struct key_value_pair
{
    u8 Key[HASH_TABLE_KEY_LENGTH];
    u32 Value;
    
    u32 NextKeyValue;
};

struct hash_table
{
    key_value_pair *KeyValuePair;
    
    u32 Count;
    u32 _TableSize;
    
    key_value_pair *Collisions;
    u32 CollisionCount;
    u32 CollisionSize;
    
    b32 IsTransient;
};

inline hash_table HashTable(arena_allocator *Arena, u32 Size);
inline b32 AddToHashTable(hash_table *Table, u8 *Key, u32 Value);
inline b32 GetFromHashTable(hash_table *Table, u8 *Key, u32 &Value);
inline b32 UpdateValueInHashTable(hash_table *Table, u8 *Key, u32 NewValue);
inline void DeleteHashTableTransient(arena_allocator *Arena, hash_table *Table);
inline u32 Hash(u32 Key);
inline u32 UnHash(u32 HashedKey);
inline u32 Hash_djb2(char *String);

// Min - Heap

struct heap_node
{
    union
    {
        i32 ValueI32;
        // u32 ValueU32;
        r32 ValueR32;
    };
    void *Data;
};

struct heap
{
    heap_node *Nodes;
    u32 Count;
    u32 Size;
};

#define LeftChildIndex(N) (2*N+1)
#define RightChildIndex(N) (2*N+2)
#define ParentIndex(N) ((N-1)/2)

inline heap CreateTransientHeap(arena_allocator *Arena, u32 MinHeapSize);
inline heap CreateFixedHeap(arena_allocator *Arena, u32 MinHeapSize);
inline void InsertIntoHeap(arena_allocator *Arena, heap *Heap, i32 Value, void* Data = 0);
inline heap_node ExtractHeapMinimum(heap *Heap);
inline heap_node GetHeapMinimum(heap *Heap);
inline b32 IsHeapEmpty(heap *Heap);

// Random
global_variable u64 RandomBaseSeed = 127;
global_variable u64 PrimeBig  = 10019059;
global_variable u64 PrimeHuge = 1000000007861;


internal r32 Random01();
inline   i32 RandomRange(i32 Minimum, i32 Maximum);
inline   i32 GetWeightedRandom01(u32 XTimesHigherPropabilityFor0);

// Array operations

struct array_u32
{
    // Head
    u32 Length;
    u32 Count;
    
    // Data
    u32 *Slot;
};

inline array_u32 CreateArray(arena_allocator *Arena, i32 Length);
inline void DestroyArray(arena_allocator *Arena, array_u32 Array);
inline void Put(array_u32 *Array, u32 Pos, u32 Item);
inline u32  Get(array_u32 *Array, u32 Pos);
inline u32  Take(array_u32 *Array, u32 Pos);
inline void Push(array_u32 *Array, u32 Item);
inline u32  Pop(array_u32 *Array);
inline void ReplaceAt(array_u32 *Array, u32 Pos, u32 Item);
inline void Reset(array_u32 *Array);
inline void Clear(array_u32 *Array, u32 Value = 0);
inline b32  Contains(array_u32 *Array, u32 Item);
inline b32  Find(array_u32 *Array, u32 Item, u32 *Result);
inline void FindAndTake(array_u32 *Array, u32 Item);
inline b32  StackContains(array_u32 *Array, u32 Item);
inline b32  StackFind(array_u32 *Array, u32 Item, u32 *Result);
inline void StackFindAndTake(array_u32 *Array, u32 Item);
inline void StackInsert(array_u32 *Array, u32 Pos, u32 Item);
inline void PushIfNotExist(array_u32 *Array, u32 Item);
inline void AppendArray(array_u32 *Array1, array_u32 *Array2);
inline void MergeArrays(array_u32 *Array1, array_u32 *Array2); // Slow
inline void Copy(array_u32 *To, array_u32 *From);
internal void ShuffleStack(array_u32 *Array);
internal void PrintArray(array_u32 Array);

#define CutValueFromArray(Array, ArraySize, ID, type) CutValueFromArray_((void *)Array, ArraySize, ID, sizeof(type));
inline void CutValueFromArray_(void *Array, u32 *ArraySize, u32 ID, u32 ElementSize);

inline   void QuickSort3(array_u32 Array, b32 SortOnLength_NotCount = false);

struct sort_info
{
    b32 (*CompareFunc)(i32 T1, i32 T2, void *Data);
    void *Data;
    void (*SwapFunc)(void *Array, i32 IndexA, i32 IndexB);
};

internal void QuickSort(i32 Low, i32 High, void *SortArray, sort_info SortInfo);
inline void Switch(void *Array, i32 P1, i32 P2);