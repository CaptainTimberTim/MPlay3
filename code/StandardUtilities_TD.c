#include "StandardUtilities_TD.h"

inline void
PrintLastWindowsError()
{
    LPVOID MsgBuf;
    DWORD ERR = GetLastError(); 
    
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, ERR,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &MsgBuf, 0, NULL );
    DebugLog(2024, "AllocationError:: %ls\n", (LPWSTR)MsgBuf);
    Assert(false);
}

// HASH TABLE

inline hash_table
HashTable(arena_allocator *Arena, u32 Size)
{
    hash_table Result    = {};
    Result._TableSize    = Size;
    Result.KeyValuePair  = AllocateArray(Arena, Size, key_value_pair);
    ClearArray(Result.KeyValuePair, Size, key_value_pair);
    Result.CollisionSize = (u32)(Size*0.2f);
    Result.Collisions    = AllocateArray(Arena, Result.CollisionSize, key_value_pair);
    ClearArray(Result.Collisions, Result.CollisionSize, key_value_pair);
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
DeleteHashTableTransient(arena_allocator *Arena, hash_table *Table)
{
    if(Table->IsTransient)
    {
        FreeMemory(Arena, Table->KeyValuePair);
        FreeMemory(Arena, Table->Collisions);
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
CreateHeap(arena_allocator *Arena, u32 MinHeapSize)
{
    heap Result = {};
    Result.Size = MinHeapSize;
    Result.Nodes = AllocateArray(Arena, MinHeapSize, heap_node);
    ClearArray(Result.Nodes, MinHeapSize, heap_node);
    
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
InsertIntoHeap(arena_allocator *Arena, heap *Heap, i32 Value, void *Data)
{
    if(Heap->Count >= Heap->Size)
    {
        if(Arena->Flags & arenaFlags_IsTransient)
        {
            //ResizeTransientArray(BucketContainer, Heap->Nodes, Heap->Size, Heap->Size*2, heap_node);
            Assert(false); // TODO:: Fix this, #ArenaAllocator
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
DeleteTransientHeap(arena_allocator *Arena, heap *Heap)
{
    FreeMemory(Arena, Heap->Nodes);
}

// Array operations
inline array_u32
CreateArray(arena_allocator *Arena, u32 Length)
{
    array_u32 Result = {};
    
    Result.Count  = 0;
    Result.Length = Length;
    Result.Slot   = AllocateArray(Arena, Length, u32);
    
    return Result;
}

inline void 
DestroyArray(arena_allocator *Arena, array_u32 Array)
{
    FreeMemory(Arena, Array.Slot);
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







