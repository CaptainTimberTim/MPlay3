#pragma once
#include "Math_TD.h"

#if DEBUG_TD
LPVOID BaseMemoryAdress = (LPVOID)Terabytes(2);
#else
LPVOID BaseMemoryAdress = 0;
#endif

inline u32 SafeTruncateUInt64(u64 Value);

struct memory_bucket
{
    u32 Size;
    u32 Position;
    u32 Count; // NOTE:: Only used for transient buckets
    u8 *Memory;
};

struct memory_bucket_container
{
    b32 IsFixedBucket;
    memory_bucket Bucket[2];
    b32 CurrentBucket;
    struct bucket_allocator *Parent;
};

struct bucket_allocator
{
    memory_bucket_container Fixed;
    memory_bucket_container Transient; 
};

internal b32 CreateBucketAllocator(bucket_allocator *BucketAllocator, u32 FixedSize, u32 HalfTransientSize);
inline void DestroyBucketAllocator(bucket_allocator *Bucket);

#define PushStructOnBucket(Bucket, type) (type *)PushSizeOnBucket_(Bucket, sizeof(type))
#define PushArrayOnBucket(Bucket, Count, type) (type *)PushSizeOnBucket_(Bucket, (Count)*sizeof(type))
#define PushSizeOnBucket(Bucket, Size) PushSizeOnBucket_(Bucket, Size)
inline void *PushSizeOnBucket_(memory_bucket_container *Bucket, u32 Size);
inline void PopFromTransientBucket(memory_bucket_container *BucketContainer, void *Address);

inline void PopFromTransientBucket(bucket_allocator *BucketAllocator, void *Address);
#define PushStructFromTransientToFixedBucket(Bucket, Address, type, Keep) (type *)PushSizeFromTransientToFixedBucket_(Bucket, (void *)(Address), sizeof(type), Keep)
#define PushArrayFromTransientToFixedBucket(Bucket, Address, Count, type, Keep) (type *)PushSizeFromTransientToFixedBucket_(Bucket, (void *)(Address), (Count)*sizeof(type), Keep)
#define PushSizeFromTransientToFixedBucket(Bucket, Address, Size, Keep) PushSizeFromTransientToFixedBucket_(Bucket, (void *)(Address), Size, Keep)
inline void *PushSizeFromTransientToFixedBucket_(bucket_allocator *BucketAllocator, void *Address, u32 Size, b32 KeepInTransient = false);

#define ResizeTransientArray(Bucket, Address, OCount, NCount, type) (type *)ResizeTransientArray_(Bucket, (void *)(Address), (OCount)*sizeof(type), (NCount)*sizeof(type))
inline void *ResizeTransientArray_(memory_bucket_container *BucketContainer, void *Address, u32 OriginalSize, u32 NewSize);

#define CopyTransientArray(Bucket, Address, FromCount, ToCount, type) (type *)CopyTransientArray_(Bucket, (void *)(Address), (FromCount)*sizeof(type), (ToCount)*sizeof(type))
inline void *CopyTransientArray_(memory_bucket_container *BucketContainer, void *Address, u32 FromSize, u32 ToSize);

inline r32 BucketFillStatus(memory_bucket *Bucket);
inline v3 BucketAllocatorFillStatus(bucket_allocator *BucketAlloc, u8 *OutString);

#define ClearToGiven(Memory, Size, Given) ClearToGiven_((void *)(Memory), Size, Given)
#define ClearArrayToGiven(Memory, Count, Type, Given) ClearToGiven_((void *)(Memory), Count*sizeof(Type), Given)
inline void ClearToGiven_(void *Memory, u32 Size, i32 Given);

inline b32 IsAddressFixed(bucket_allocator *Bucket, void *Address);

internal bucket_allocator *CreateSubAllocator(bucket_allocator *Bucket, u32 FixedSize, u32 HalfTransientSize);

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

inline hash_table HashTable(memory_bucket_container *BucketContainer, u32 Size);
inline b32 AddToHashTable(hash_table *Table, u8 *Key, u32 Value);
inline b32 GetFromHashTable(hash_table *Table, u8 *Key, u32 &Value);
inline b32 UpdateValueInHashTable(hash_table *Table, u8 *Key, u32 NewValue);
inline void DeleteHashTableTransient(bucket_allocator *BucketAllocator, hash_table *Table);
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

inline heap CreateTransientHeap(bucket_allocator *Bucket, u32 MinHeapSize);
inline heap CreateFixedHeap(bucket_allocator *Bucket, u32 MinHeapSize);
inline void InsertIntoHeap(memory_bucket_container *BucketContainer, heap *Heap, i32 Value, void* Data = 0);
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

inline array_u32 CreateArray(memory_bucket_container *Bucket, u32 Length);
inline void DestroyArray(memory_bucket_container *Bucket, array_u32 Array);
inline void Put(array_u32 *Array, u32 Pos, u32 Item);
inline u32  Get(array_u32 *Array, u32 Pos);
inline u32  Take(array_u32 *Array, u32 Pos);
inline void Push(array_u32 *Array, u32 Item);
inline u32  Pop(array_u32 *Array);
inline void ReplaceAt(array_u32 *Array, u32 Pos, u32 Item);
inline void Reset(array_u32 *Array);
inline void Clear(array_u32 *Array);
inline b32  Contains(array_u32 *Array, u32 Item);
inline b32  Find(array_u32 *Array, u32 Item, u32 *Result);
inline void FindAndTake(array_u32 *Array, u32 Item);
inline b32  StackContains(array_u32 *Array, u32 Item);
inline b32  StackFind(array_u32 *Array, u32 Item, u32 *Result);
inline void StackFindAndTake(array_u32 *Array, u32 Item);
inline void StackInsert(array_u32 *Array, u32 Pos, u32 Item);
inline void PushIfNotExist(array_u32 *Array, u32 Item);
inline void AppendArray(array_u32 *Array1, array_u32 *Array2);
inline void Switch(array_u32 *Array, u32 P1, u32 P2);
inline void Copy(array_u32 *To, array_u32 *From);
internal void ShuffleStack(array_u32 *Array);
internal void PrintArray(array_u32 Array);

#define CutValueFromArray(Array, ArraySize, ID, type) CutValueFromArray_((void *)Array, ArraySize, ID, sizeof(type));
inline void CutValueFromArray_(void *Array, u32 *ArraySize, u32 ID, u32 ElementSize);

struct sort_info
{
    b32 (*CompareFunc)(i32 T1, i32 T2, void *Data);
    void *Data;
};

internal void QuickSort(i32 Low, i32 High, array_u32 *SortArray, sort_info SortInfo);
