#include "StandardUtilities_TD.h"

inline u32
SafeTruncateUInt64(u64 Value)
{
    u32 Result = (u32)Value;
    return(Result);
}

inline void
PrintLastWindowsError()
{
    LPVOID MsgBuf;
    DWORD ERR = GetLastError(); 
    
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, ERR,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &MsgBuf, 0, NULL );
    DebugLog(2024, "AllocationError:: %s\n", (LPCTSTR)MsgBuf);
    Assert(false);
}

inline b32
CreateBucketAllocator(bucket_allocator *BucketAllocator, u32 FixedSize, u32 HalfTransientSize)
{
    b32 Result = true;
    
    if(BucketAllocator->Fixed.Bucket->Size != 0 ||
       BucketAllocator->Transient.Bucket[0].Size != 0 ||
       BucketAllocator->Transient.Bucket[1].Size != 0)
    {
        if(BucketAllocator->Fixed.Bucket->Memory)
        {
            VirtualFree(BucketAllocator->Fixed.Bucket->Memory, 0, MEM_RELEASE);
        }
        BucketAllocator->Fixed.Bucket->Position = 0;
        BucketAllocator->Transient.Bucket[0].Position = 0;
        BucketAllocator->Transient.Bucket[0].Count = 0;
        BucketAllocator->Transient.Bucket[1].Position = 0;
        BucketAllocator->Transient.Bucket[1].Count = 0;
        BucketAllocator->Transient.CurrentBucket = 0;
    }
    
    BucketAllocator->Fixed.IsFixedBucket = true;
    BucketAllocator->Fixed.Bucket->Size = FixedSize;
    BucketAllocator->Fixed.Bucket->Memory = (u8 *)VirtualAlloc(BaseMemoryAdress, FixedSize + 2*HalfTransientSize, 
                                                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#if DEBUG_TD
    if(!BucketAllocator->Fixed.Bucket->Memory)
    {
        u64 BA = Terabytes(2);
        For(10)
        {
            BA += Terabytes(1);
            BaseMemoryAdress = (LPVOID)BA;
            BucketAllocator->Fixed.Bucket->Memory = (u8 *)VirtualAlloc(BaseMemoryAdress, 
                                                                       FixedSize + 2*HalfTransientSize,
                                                                       MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if(BucketAllocator->Fixed.Bucket->Memory) break;
            DebugLog(255, "ATTENTION:: VirtualAlloc did not return memory adress. Trying again!\n");
        }
        
        if(!BucketAllocator->Fixed.Bucket->Memory) PrintLastWindowsError();
        DebugLog(255, "Memory adress was successfuly set.\n");
    }
#endif
    if(!BucketAllocator->Fixed.Bucket->Memory) Result = false;
    else
    {
        
        BucketAllocator->Transient.Bucket[0].Size = HalfTransientSize;
        BucketAllocator->Transient.Bucket[0].Memory = BucketAllocator->Fixed.Bucket->Memory + FixedSize;
        if(!BucketAllocator->Transient.Bucket[0].Memory) PrintLastWindowsError();
        
        BucketAllocator->Transient.Bucket[1].Size = HalfTransientSize;
        BucketAllocator->Transient.Bucket[1].Memory = BucketAllocator->Fixed.Bucket->Memory + (FixedSize+HalfTransientSize);
        if(!BucketAllocator->Transient.Bucket[1].Memory) PrintLastWindowsError();
        
        BucketAllocator->Transient.Parent = BucketAllocator;
        BucketAllocator->Fixed.Parent     = BucketAllocator;
    }
    return Result;
}

inline void
DestroyBucketAllocator(bucket_allocator *Bucket)
{
    VirtualFree(Bucket->Fixed.Bucket->Memory, 0, MEM_RELEASE);
    VirtualFree(Bucket->Transient.Bucket[0].Memory, 0, MEM_RELEASE);
    VirtualFree(Bucket->Transient.Bucket[1].Memory, 0, MEM_RELEASE);
}

inline void *
PushSizeOnBucket_(memory_bucket_container *BucketContainer, u32 Size)
{
    void *Result = 0;
    Assert(Size < BucketContainer->Bucket[BucketContainer->CurrentBucket].Size);
    
    if(BucketContainer->IsFixedBucket) 
    {
        Assert(BucketContainer->Bucket->Size > BucketContainer->Bucket->Position+Size);
        Result = BucketContainer->Bucket->Memory + BucketContainer->Bucket->Position;
        BucketContainer->Bucket->Position += Size;
        BucketContainer->Bucket->Count++;
    }
    else
    {
        memory_bucket *Bucket = BucketContainer->Bucket + BucketContainer->CurrentBucket;
        if(Bucket->Size  <= Bucket->Position+Size)
        {
            BucketContainer->CurrentBucket = 1 - BucketContainer->CurrentBucket;
            Bucket = BucketContainer->Bucket + BucketContainer->CurrentBucket;
            Bucket->Position = 0;
            Bucket->Count = 0;
        }
        
        Result = Bucket->Memory + Bucket->Position;
        Bucket->Position += Size;
        Bucket->Count++;
        
        ClearToGiven(Result, Size, 0);
    }
    
    return Result;
}

inline void
PopFromTransientBucket(memory_bucket_container *BucketContainer, void *Address)
{
    if(!BucketContainer->IsFixedBucket && Address)
    {
        memory_bucket *Bucket  = 0;
        memory_bucket *Bucket1 = BucketContainer->Bucket + 0;
        memory_bucket *Bucket2 = BucketContainer->Bucket + 1;
        
        Assert(Address >= Bucket1->Memory);
        Bucket = (Address >= Bucket1->Memory && Address < Bucket2->Memory) ? Bucket1 : Bucket2;
        
        Assert(Bucket->Count > 0);
        Bucket->Count--;
        if(Bucket->Count == 0)
        {
            Bucket->Position = 0;
        }
    }
}

inline void
PopFromTransientBucket(bucket_allocator *BucketAllocator, void *Address)
{
    PopFromTransientBucket(&BucketAllocator->Transient, Address);
}

inline b32 
IsAddressFixed(bucket_allocator *Bucket, void *Address)
{
    b32 Result = false;
    
    memory_bucket *Transient   = Bucket->Transient.Bucket + 0;
    
    Result = (Address >= Bucket->Fixed.Bucket->Memory && Address < Transient->Memory);
    
    return Result;
}

inline b32 
IsAddressTransient(bucket_allocator *Bucket, void *Address)
{
    b32 Result = false;
    
    memory_bucket *Transient   = Bucket->Transient.Bucket + 0;
    
    Result = !(Address >= Bucket->Fixed.Bucket->Memory && Address < Transient->Memory);
    
    return Result;
}

inline void *
PushSizeFromTransientToFixedBucket_(bucket_allocator *BucketAllocator, void *Address, u32 Size, b32 KeepInTransient)
{
    void *Result = PushSizeOnBucket(&BucketAllocator->Fixed, Size);
    
    memcpy_s(Result, Size, Address, Size);
    if(!KeepInTransient)
    {
        PopFromTransientBucket(BucketAllocator, Address);
    }
    
    return Result;
}

inline void *
ResizeTransientArray_(memory_bucket_container *BucketContainer, void *Address, u32 OriginalSize, u32 NewSize)
{
    void *Result = Address;
    
    if(!BucketContainer->IsFixedBucket)
    {
        if(OriginalSize >= NewSize)
        {
            Result = PushSizeOnBucket(BucketContainer, NewSize);
            memcpy_s(Result, OriginalSize,  Address, OriginalSize);
            PopFromTransientBucket(BucketContainer, Address);
        }
    }
    
    return Result;
}

inline void *
CopyTransientArray_(memory_bucket_container *BucketContainer, void *Address, u32 FromSize, u32 ToSize)
{
    void *Result = Address;
    
    if(!BucketContainer->IsFixedBucket)
    {
        Result = PushSizeOnBucket(BucketContainer, ToSize-FromSize);
        memcpy_s(Result, ToSize,  (u8 *)Address+FromSize, ToSize);
    }
    
    return Result;
}

inline r32 
BucketFillStatus(memory_bucket *Bucket)
{
    r32 Result = 0;
    
    Result = (1.0f/Bucket->Size)*Bucket->Position;
    
    return Result;
}

inline v3
BucketAllocatorFillStatus(bucket_allocator *BucketAlloc, u8 *OutString)
{
    v3 Result = {};
    
    Result.x = BucketFillStatus(BucketAlloc->Fixed.Bucket);
    Result.y = BucketFillStatus(&BucketAlloc->Transient.Bucket[0]);
    Result.z = BucketFillStatus(&BucketAlloc->Transient.Bucket[1]); 
    sprintf_s((char *)OutString, 100, "Bucket-Allocator fill status: \nFix: %.4f%%, Tran0: %.4f%%, Tran1: %.4f%%\n", Result.x, Result.y, Result.z);
    
    return Result;
}

inline void 
ClearToGiven_(void *Memory, u32 Size, i32 Given)
{
    memset(Memory, Given, Size);
}

inline void
SetReal32ArrayToGiven(r32 *Array, u32 Count, r32 Value)
{
    
    for(u32 Index = 0; Index < Count; ++Index)
    {
        *(Array++) = Value;
    }
}

internal bucket_allocator *
CreateSubAllocator(bucket_allocator *Bucket, u32 FixedSize, u32 HalfTransientSize)
{
    bucket_allocator *Result = PushStructOnBucket(&Bucket->Fixed, bucket_allocator);
    
    Result->Fixed.IsFixedBucket      = true;
    Result->Fixed.CurrentBucket      = 0;
    Result->Fixed.Parent             = Result;
    Result->Fixed.Bucket[0].Size     = FixedSize;
    Result->Fixed.Bucket[0].Position = 0;
    Result->Fixed.Bucket[0].Memory   = (u8 *)PushSizeOnBucket(&Bucket->Fixed, FixedSize);
    
    Result->Transient.IsFixedBucket = false;
    Result->Transient.CurrentBucket = 0;
    Result->Transient.Parent        = Result;
    For(2)
    {
        Result->Transient.Bucket[It].Size     = HalfTransientSize;
        Result->Transient.Bucket[It].Position = 0;
        Result->Transient.Bucket[It].Count    = 0;
        Result->Transient.Bucket[It].Memory   = (u8 *)PushSizeOnBucket(&Bucket->Fixed, HalfTransientSize);
    }
    
    return Result;
}


// HASH TABLE

inline hash_table
HashTable(memory_bucket_container *BucketContainer, u32 Size)
{
    hash_table Result    = {};
    Result._TableSize    = Size;
    Result.KeyValuePair  = PushArrayOnBucket(BucketContainer, Size, key_value_pair);
    ClearToGiven(Result.KeyValuePair, sizeof(key_value_pair)*Size, 0);
    Result.CollisionSize = (u32)(Size*0.2f);
    Result.Collisions    = PushArrayOnBucket(BucketContainer, Result.CollisionSize, key_value_pair);
    ClearToGiven(Result.Collisions, sizeof(key_value_pair)*Result.CollisionSize, 0);
    Result.IsTransient = true;
    return Result;
}

inline b32
AddToHashTable(hash_table *Table, u8 *Key, u32 Value)
{
    b32 Result = true;
    //Assert(StringLength(Key) <= HASH_TABLE_KEY_LENGTH);
    u32 HashKey = Hash_djb2((char *)Key) % Table->_TableSize;
    key_value_pair *Pair = Table->KeyValuePair + HashKey;
    while(*Pair->Key != 0)
    {
        if(StringCompare(Pair->Key, Key, 0, StringLength(Key)))
        {
            Result = false;
            break;
        }
        if(Pair->NextKeyValue)
        {
            Pair = Table->Collisions + (Pair->NextKeyValue - 1);
        }
        else
        {
            Assert(Table->CollisionSize > Table->CollisionCount);
            Pair->NextKeyValue = Table->CollisionCount + 1; // Note:: Adding one in order to have the 0 for _value not set_, need to subtract 1 on use!
            Pair = (Table->Collisions + Table->CollisionCount++);
            break;
        }
    }
    if(Result)
    {
        CopyString(Pair->Key, Key, (StringLength(Key)));
        Pair->Value = Value;
        Table->Count++;
    }
    return Result;
}

inline b32
GetFromHashTable(hash_table *Table, u8 *Key, u32 &Value)
{
    u32 Result = true;
    u32 HashKey = Hash_djb2((char *)Key) % Table->_TableSize;
    key_value_pair *Pair = Table->KeyValuePair + HashKey;
    if(*Pair->Key)
    {
        while(!StringCompare(Pair->Key, Key, 0, StringLength(Key)))
        {
            if(Pair->NextKeyValue)
            {
                Pair = Table->Collisions + (Pair->NextKeyValue - 1);
            }
            else
            {
                Result = false;
                Pair = 0;
                break;
            }
        }
        if(Pair)
        {
            Value = Pair->Value;
        }
    }
    else
    {
        Result = false;
    }
    return  Result;
}

inline b32
UpdateValueInHashTable(hash_table *Table, u8 *Key, u32 NewValue)
{
    u32 Result = false;
    u32 HashKey = Hash_djb2((char *)Key) % Table->_TableSize;
    key_value_pair *Pair = Table->KeyValuePair + HashKey;
    while(*Pair->Key != 0)
    {
        if(StringCompare(Pair->Key, Key, 0, StringLength(Key)))
        {
            Pair->Value = NewValue;
            Result = true;
            break;
        }
        if(Pair->NextKeyValue)
        {
            Pair = Table->Collisions + (Pair->NextKeyValue - 1);
        }
        else
        {
            break;
        }
    }
    return Result;
}

inline void
DeleteHashTableTransient(bucket_allocator *BucketAllocator, hash_table *Table)
{
    if(Table->IsTransient)
    {
        PopFromTransientBucket(BucketAllocator, Table->KeyValuePair);
        PopFromTransientBucket(BucketAllocator, Table->Collisions);
    }
}

inline u32 
Hash(u32 Key) 
{
    Key = ((Key >> 16) ^ Key) * 0x45d9f3b;
    Key = ((Key >> 16) ^ Key) * 0x45d9f3b;
    Key = (Key >> 16) ^ Key;
    return Key;
}

inline u32 
UnHash(u32 HashedKey) 
{
    HashedKey = ((HashedKey >> 16) ^ HashedKey) * 0x119de1f3;
    HashedKey = ((HashedKey >> 16) ^ HashedKey) * 0x119de1f3;
    HashedKey = (HashedKey >> 16) ^ HashedKey;
    return HashedKey;
}

inline u32
Hash_djb2(char *String)
{
    u32 HashVal = 5381;
    i32 C = 0;
    while (C = *String++)
    {
        HashVal = ((HashVal << 5) + HashVal) + C;
    }
    return HashVal;
}

inline void
SetRandomSeed(u64 NewSeed)
{
    RandomBaseSeed = NewSeed;
}

inline u64
GetNextSeedValue()
{
    u64 Result = 0;
    RandomBaseSeed = (RandomBaseSeed*PrimeBig)%PrimeHuge;
    Result = RandomBaseSeed;
    
    return Result;
}

internal r32
Random01()
{
    u64 SeedValue = GetNextSeedValue();
    Assert(SeedValue > 127);
    SeedValue = (((SeedValue*7927)%9833)*269) % MAX_UINT32;
    
    r32 Result = 0;
    u32 z1 = (u32)SeedValue, z2 = (u32)SeedValue, 
    z3 = (u32)SeedValue, z4 = (u32)SeedValue;
    u32 b;
    b  = ((z1 << 6) ^ z1) >> 13;
    z1 = ((z1 & 4294967294UL) << 18) ^ b;
    b  = ((z2 << 2) ^ z2) >> 27; 
    z2 = ((z2 & 4294967288UL) << 2) ^ b;
    b  = ((z3 << 13) ^ z3) >> 21;
    z3 = ((z3 & 4294967280UL) << 7) ^ b;
    b  = ((z4 << 3) ^ z4) >> 12;
    z4 = ((z4 & 4294967168UL) << 13) ^ b;
    Result = (r32)((z1 ^ z2 ^ z3 ^ z4) * 2.3283064365386963e-10);
    
    return(Result);
}

inline i32
RandomRange(i32 Minimum, i32 Maximum)
{
    r32 RandomVal01  = Random01();
    
    i32 Range = Maximum - Minimum;
    r32 TempMap = (RandomVal01 * (r32)Range);
    i32 Result = (i32)(TempMap + (r32)Minimum);
    
    return(Result);
}

inline i32
GetWeightedRandom01(u32 XTimesHigherPropabilityFor0)
{
    i32 Result = 0;
    
    u32 Range = 2 + XTimesHigherPropabilityFor0;
    i32 Rand = RandomRange(0, Range);
    if((u32)Rand == Range-1) Result = 1;
    
    return Result;
}

// HEAP

inline heap
CreateHeap(memory_bucket_container *BucketContainer, u32 MinHeapSize)
{
    heap Result = {};
    Result.Size = MinHeapSize;
    Result.Nodes = PushArrayOnBucket(BucketContainer, MinHeapSize, heap_node);
    ClearArrayToGiven(Result.Nodes, MinHeapSize, heap_node, 0);
    
    return Result;
}

inline heap_node
GetHeapMinimum(heap *Heap)
{
    Assert(Heap->Count > 0);
    return *Heap->Nodes;
}

inline b32
IsHeapEmpty(heap *Heap)
{
    return Heap->Count == 0;
}

internal void
SiftUpHeap(heap *Heap, u32 NodeID)
{
    u32 ParentID = 0;
    heap_node Tmp = {};
    
    if(NodeID != 0)
    {
        ParentID = ParentIndex(NodeID);
        if(Heap->Nodes[ParentID].ValueI32 > Heap->Nodes[NodeID].ValueI32)
        {
            Tmp = Heap->Nodes[ParentID];
            Heap->Nodes[ParentID] = Heap->Nodes[NodeID];
            Heap->Nodes[NodeID] = Tmp;
            SiftUpHeap(Heap, ParentID);
        }
    }
}

inline void
InsertIntoHeap(memory_bucket_container *BucketContainer, heap *Heap, i32 Value, void *Data)
{
    if(Heap->Count >= Heap->Size)
    {
        if(!BucketContainer->IsFixedBucket /*IsAddressTransient(Bucket, Heap->Nodes)*/)
        {
            ResizeTransientArray(BucketContainer, Heap->Nodes, Heap->Size, Heap->Size*2, heap_node);
            Heap->Size *= 2;
        }
        else
        {
            NotImplemented
        }
    }
    Heap->Nodes[Heap->Count].ValueI32 = Value;
    Heap->Nodes[Heap->Count].Data = Data;
    SiftUpHeap(Heap, Heap->Count++);
}

internal void
SiftDownHeap(heap *Heap, u32 NodeID)
{
    u32 LeftChildID  = LeftChildIndex(NodeID);
    u32 RightChildID = RightChildIndex(NodeID);
    u32 MinID        = 0;
    heap_node Tmp    = {};
    
    if(RightChildID >= Heap->Count)
    {
        if(LeftChildID >= Heap->Count) return;
        else MinID = LeftChildID;
    }
    else
    {
        if(Heap->Nodes[LeftChildID].ValueI32 <= Heap->Nodes[RightChildID].ValueI32)
            MinID = LeftChildID;
        else MinID = RightChildID;
    }
    
    if(Heap->Nodes[NodeID].ValueI32 > Heap->Nodes[MinID].ValueI32)
    {
        Tmp = Heap->Nodes[MinID];
        Heap->Nodes[MinID] = Heap->Nodes[NodeID];
        Heap->Nodes[NodeID] = Tmp;
        SiftDownHeap(Heap, MinID);
    }
}

inline heap_node
ExtractHeapMinimum(heap *Heap)
{
    Assert(Heap->Count > 0);
    heap_node Result = Heap->Nodes[0];
    Heap->Nodes[0] = Heap->Nodes[--Heap->Count];
    if(Heap->Count > 0) SiftDownHeap(Heap, 0);
    
    return Result;
}

inline void
DeleteTransientHeap(bucket_allocator *Bucket, heap *Heap)
{
    PopFromTransientBucket(Bucket, Heap->Nodes);
}

// Array operations
inline array_u32
CreateArray(memory_bucket_container *Bucket, u32 Length)
{
    array_u32 Result = {};
    
    Result.Count  = 0;
    Result.Length = Length;
    Result.Slot   = PushArrayOnBucket(Bucket, Length, u32);
    
    return Result;
}

inline void 
DestroyArray(memory_bucket_container *Bucket, array_u32 Array)
{
    PopFromTransientBucket(Bucket, Array.Slot);
}

inline void 
Put(array_u32 *Array, u32 Pos, u32 Item)
{
    Assert(Pos < Array->Length);
    Array->Slot[Pos] = Item;
}

inline u32
Get(array_u32 *Array, u32 Pos)
{
    Assert(Pos < Array->Length);
    return Array->Slot[Pos];
}

inline u32
Take(array_u32 *Array, u32 Pos)
{
    Assert(Pos < Array->Length);
    u32 Result = Array->Slot[Pos];
    
    u32 MoveSize  = (Array->Count-Pos)*sizeof(u32);
    
    u8 *GoalAddress  = ((u8 *)Array->Slot) + (Pos*sizeof(u32));
    u8 *StartAddress = GoalAddress + sizeof(u32);
    
    memmove_s(GoalAddress, MoveSize, StartAddress, MoveSize);
    Array->Count--;
    
    return Result;
}

inline void
ReplaceAt(array_u32 *Array, u32 Pos, u32 Item)
{
    Assert(Pos < Array->Length);
    Array->Slot[Pos] = Item;
}

inline void 
Push(array_u32 *Array, u32 Item)
{
    Array->Slot[Array->Count++] = Item;
    Assert(Array->Count <= Array->Length);
}

inline u32  
Pop(array_u32 *Array)
{
    Assert(Array->Count > 0);
    u32 Result = Array->Slot[--Array->Count];
    return Result;
}

inline void 
Reset(array_u32 *Array)
{
    Array->Count = 0;
}

inline void
Clear(array_u32 *Array, u32 Value)
{
    For(Array->Length)
    {
        Array->Slot[It] = Value;
    }
    Reset(Array);
}

inline b32  
Contains(array_u32 *Array, u32 Item)
{
    b32 Result = false;
    
    if(Array->Count > 0)
    {
        For(Array->Length)
        {
            if(Array->Slot[It] == Item)
            {
                Result = true;
                break;
            }
        }
    }
    return Result;
}

inline b32
Find(array_u32 *Array, u32 Item, u32 *Result)
{
    b32 Found = false;
    
    if(Array->Count > 0)
    {
        For(Array->Length)
        {
            if(Array->Slot[It] == Item)
            {
                *Result = It;
                Found = true;
                break;
            }
        }
    }
    
    return Found;
}

inline void
FindAndTake(array_u32 *Array, u32 Item)
{
    u32 ID = 0;
    if(Find(Array, Item, &ID))
    {
        Take(Array, ID);
    }
}

inline b32  
StackContains(array_u32 *Array, u32 Item)
{
    b32 Result = false;
    
    For(Array->Count)
    {
        if(Array->Slot[It] == Item)
        {
            Result = true;
            break;
        }
    }
    
    return Result;
}

inline b32
StackFind(array_u32 *Array, u32 Item, u32 *Result)
{
    b32 Found = false;
    
    For(Array->Count)
    {
        if(Array->Slot[It] == Item)
        {
            *Result = It;
            Found = true;
            break;
        }
    }
    
    return Found;
}

inline void
StackFindAndTake(array_u32 *Array, u32 Item)
{
    u32 ID = 0;
    if(StackFind(Array, Item, &ID))
    {
        Take(Array, ID);
    }
}

inline void
StackInsert(array_u32 *Array, u32 Pos, u32 Item)
{
    Assert(Pos < Array->Length);
    Assert(Array->Count < Array->Length);
    
    u32 MoveSize  = (Array->Count-(Pos-1))*sizeof(u32);
    
    u8 *GoalAddress  = ((u8 *)Array->Slot) + ((Pos+1)*sizeof(u32));
    u8 *StartAddress = GoalAddress - sizeof(u32);
    
    memmove_s(GoalAddress, MoveSize, StartAddress, MoveSize);
    Array->Count++;
    Put(Array, Pos, Item);
}

inline void
PushIfNotExist(array_u32 *Array, u32 Item)
{
    b32 Found = false;
    For(Array->Count)
    {
        if(Array->Slot[It] == Item)
        {
            Found = true;
            break;
        }
    }
    if(!Found)
    {
        Push(Array, Item);
    }
}

inline void
AppendArray(array_u32 *Array1, array_u32 *Array2)
{
    Assert(Array1->Count + Array2->Count <= Array1->Length);
    For(Array2->Count)
    {
        Push(Array1, Get(Array2, It));
    }
}

inline void 
MergeArrays(array_u32 *Array1, array_u32 *Array2)
{
    For(Array2->Count)
    {
        PushIfNotExist(Array1, Get(Array2, It));
    }
}

inline void
Switch(array_u32 *Array, u32 P1, u32 P2)
{
    Assert(P1 < Array->Length);
    Assert(P2 < Array->Length);
    u32 T = Array->Slot[P1];
    Array->Slot[P1] = Array->Slot[P2];
    Array->Slot[P2] = T;
}

inline void 
Copy(array_u32 *To, array_u32 *From)
{
    Assert(To->Length <= From->Length);
    
    For(From->Count)
    {
        To->Slot[It] = From->Slot[It];
    }
    To->Count = From->Count;
}

inline void
CutValueFromArray_(void *Array, u32 *ArraySize, u32 ID, u32 ElementSize)
{
    u32 CorrectedArraySize = (*ArraySize)*ElementSize;
    ID *= ElementSize;
    
    u8 *GoalAddress  = (u8 *)Array + ID;
    u8 *StartAddress = GoalAddress + ElementSize;
    
    memmove_s(GoalAddress, CorrectedArraySize, StartAddress, CorrectedArraySize);
    *ArraySize -= 1;
}

internal i32
QuickSortPartition(i32 Low, i32 High, array_u32 *SortArray, sort_info SortInfo) 
{ 
    i32 Pivot   = High;
    i32 SmallID = (Low - 1);
    
    for(i32 HighID = Low; HighID <= High- 1; HighID++) 
    { 
        if (SortInfo.CompareFunc(HighID, Pivot, SortInfo.Data)) 
        { 
            SmallID++;
            Switch(SortArray, SmallID, HighID); 
        } 
    } 
    Switch(SortArray, SmallID+1, High); 
    return (SmallID + 1); 
} 

internal void 
QuickSort(i32 Low, i32 High, array_u32 *SortArray, sort_info SortInfo) 
{ 
    if(Low < High)
    {
        i32 PartitionID = QuickSortPartition(Low, High, SortArray, SortInfo); 
        
        QuickSort(Low, PartitionID - 1, SortArray, SortInfo); 
        QuickSort(PartitionID + 1, High, SortArray, SortInfo); 
    }
} 

internal i32
GetSmallestEntryID(array_u32 *Array, u32 Count)
{
    i32 Result = -1;
    u32 MinEntry = MAX_UINT32;
    For(Count)
    {
        if(Get(Array, It) < MinEntry)
        {
            MinEntry = Get(Array,  It);
            Result = It;
        }
    }
    return Result;
}

internal void 
ShuffleStack(array_u32 *Array)
{
    For(Array->Count)
    {
        u32 RID = RandomRange(0, Array->Count);
        u32 Value = Array->Slot[RID];
        Array->Slot[RID] = Array->Slot[It];
        Array->Slot[It] = Value;
    }
}

internal void 
PrintArray(array_u32 Array)
{
    DebugLog(3, "[ ");
    For(Array.Count)
    {
        DebugLog(50, "%i ", Array.Slot[It]);
    }
    DebugLog(3, "]\n");
}


// 3-Way Quicksort ***********************************************
inline void
Swap(u32 *A, u32 *B)
{
    u32 T = *A;
    *A = *B;
    *B = T;
}

// This function partitions a[] in three parts 
// a) a[l..i] contains all elements smaller than pivot 
// b) a[i+1..j-1] contains all occurrences of pivot 
// c) a[j..r] contains all elements greater than pivot
internal void
Partition(array_u32 Array, i32 l, i32 r, i32 *i, i32 *j) 
{ 
    *i = l-1;
    *j = r; 
    i32 p = l-1;
    i32 q = r; 
    u32 v = Array.Slot[r]; 
    
    while (true) 
    { 
        // From left, find the first element smaller than 
        // or equal to v. This loop will definitely terminate 
        // as v is last element 
        while (Array.Slot[++(*i)] < v) ; 
        
        // From right, find the first element greater than or 
        // equal to v 
        while (v < Array.Slot[--(*j)]) 
            if (*j == l) 
            break; 
        
        // If i and j cross, then we are done 
        if (*i >= *j) break; 
        
        // Swap, so that smaller goes on left greater goes on right 
        Swap(Array.Slot+ *i, Array.Slot+ *j); 
        
        // Move all same left occurrence of pivot to beginning of 
        // array and keep count using p 
        if (Array.Slot[*i] == v) 
        { 
            p++; 
            Swap(Array.Slot + p, Array.Slot + *i); 
        } 
        
        // Move all same right occurrence of pivot to end of array 
        // and keep count using q 
        if (Array.Slot[*j] == v) 
        { 
            q--; 
            Swap(Array.Slot + *j, Array.Slot + q); 
        } 
    } 
    
    // Move pivot element to its correct index 
    Swap(Array.Slot+*i, Array.Slot+r); 
    
    // Move all left same occurrences from beginning 
    // to adjacent to arr[i] 
    *j = (*i)-1; 
    for (i32 k = l; k < p; k++, (*j)--) 
        Swap(Array.Slot+k, Array.Slot+*j); 
    
    // Move all right same occurrences from end 
    // to adjacent to arr[i] 
    *i = (*i)+1; 
    for (i32 k = r-1; k > q; k--, (*i)++) 
        Swap(Array.Slot+*i, Array.Slot+k); 
} 

// 3-way partition based quick sort 
inline void 
Quicksort3Recurse(array_u32 Array, i32 l, i32 r) 
{ 
    if (r <= l) return; 
    
    i32 i, j; 
    
    // Note that i and j are passed as reference 
    Partition(Array, l, r, &i, &j); 
    
    // Recur 
    Quicksort3Recurse(Array, l, j); 
    Quicksort3Recurse(Array, i, r); 
}

inline void 
QuickSort3(array_u32 Array, b32 SortOnLength_NotCount)
{
    if(SortOnLength_NotCount) Quicksort3Recurse(Array, 0, Array.Length-1);
    else Quicksort3Recurse(Array, 0, Array.Count-1);
}







