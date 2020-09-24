/* date = September 23rd 2020 9:18 am */
#ifndef _SOUND__GENERAL__T_D_H
#define _SOUND__GENERAL__T_D_H

struct file_id        { i32 ID; }; // ::FILE_ID
typedef file_id batch_id;          // ::BATCH_ID
struct playlist_id    { i32 ID; }; // ::PLAYLIST_ID
typedef playlist_id select_id;
typedef playlist_id decode_id;
struct displayable_id { i32 ID; }; // ::DISPLAYABLE_ID

struct array_file_id { array_u32 A; };
typedef array_file_id array_batch_id;

// FileID Array stuff
inline file_id Get(array_file_id *Array, playlist_id ID)          { return {(i32)Get(&Array->A, ID.ID)}; }
inline file_id Get(array_file_id *Array, displayable_id ID)       { return {(i32)Get(&Array->A, ID.ID)}; }
inline void Push(array_file_id *Array, file_id ID)                { Push(&Array->A, ID.ID); }
inline void Push(array_file_id *Array, displayable_id ID)         { Push(&Array->A, ID.ID); }
inline void Copy(array_file_id *A1, array_file_id *A2)            { Copy(&A1->A, &A2->A); }
inline b32  Find(array_file_id *Array, file_id Item, u32 *Result) { return Find(&Array->A, Item.ID, Result); }
inline void StackFindAndTake(array_file_id *Array, file_id ID)    { StackFindAndTake(&Array->A, ID.ID); }
inline b32 StackContains(array_file_id *Array, file_id ID)        { return StackContains(&Array->A, ID.ID); }
inline void Reset(array_file_id *Array)                           { Reset(&Array->A); }
inline void ShuffleStack(array_file_id *Array)                    { ShuffleStack(&Array->A); }
inline file_id Take(array_file_id *Array, playlist_id ID)         { return {(i32)Take(&Array->A, ID.ID)}; }
inline b32  StackFind(array_file_id *Array, file_id Item, i32 *Result) { return StackFind(&Array->A, Item.ID, (u32 *)Result); }
inline void Put(array_file_id *Array, playlist_id ID, file_id FileID)  { Put(&Array->A, ID.ID, FileID.ID); }
inline void AppendArray(array_file_id *A1, array_file_id *A2)     { AppendArray(&A1->A, &A2->A); }
internal void QuickSort(i32 Low, i32 High, array_file_id *SortArray, sort_info SortInfo)
{ QuickSort(Low, High, &SortArray->A, SortInfo); }

// FileID/BatchID stuff
inline file_id NewFileID(i32 ID)                { return {ID}; }
inline batch_id NewBatchID(i32 ID)              { return {ID}; }
inline b32 operator< (file_id ID1, file_id ID2) { return ID1.ID <  ID2.ID; }
inline b32 operator< (file_id ID1, i32 ID2)     { return ID1.ID <  ID2; }
inline b32 operator<=(file_id ID1, file_id ID2) { return ID1.ID <= ID2.ID; }
inline b32 operator<=(file_id ID1, i32 ID2)     { return ID1.ID <= ID2; }
inline b32 operator> (file_id ID1, file_id ID2) { return ID1.ID >  ID2.ID; }
inline b32 operator> (file_id ID1, i32 ID2)     { return ID1.ID >  ID2; }
inline b32 operator>=(file_id ID1, file_id ID2) { return ID1.ID >= ID2.ID; }
inline b32 operator>=(file_id ID1, i32 ID2)     { return ID1.ID >= ID2; }
inline b32 operator==(file_id ID1, file_id ID2) { return ID1.ID == ID2.ID; }
inline b32 operator==(file_id ID1, i32 ID2)     { return ID1.ID == ID2; }
inline b32 operator!=(file_id ID1, file_id ID2) { return ID1.ID != ID2.ID; }
inline b32 operator!=(file_id ID1, i32  ID2)    { return ID1.ID != ID2; }

// PlaylistID stuff
inline playlist_id NewPlaylistID(i32 ID)                { return {ID}; }
inline playlist_id NewPlaylistID(displayable_id ID)     { return {ID.ID}; }
inline b32 operator< (playlist_id ID1, playlist_id ID2) { return ID1.ID <  ID2.ID; }
inline b32 operator< (playlist_id ID1, i32 ID2)         { return ID1.ID <  ID2; }
inline b32 operator<=(playlist_id ID1, playlist_id ID2) { return ID1.ID <= ID2.ID; }
inline b32 operator<=(playlist_id ID1, i32 ID2)         { return ID1.ID <= ID2; }
inline b32 operator> (playlist_id ID1, playlist_id ID2) { return ID1.ID >  ID2.ID; }
inline b32 operator> (playlist_id ID1, i32 ID2)         { return ID1.ID >  ID2; }
inline b32 operator>=(playlist_id ID1, playlist_id ID2) { return ID1.ID >= ID2.ID; }
inline b32 operator>=(playlist_id ID1, i32 ID2)         { return ID1.ID >= ID2; }
inline b32 operator==(playlist_id ID1, playlist_id ID2) { return ID1.ID == ID2.ID; }
inline b32 operator==(playlist_id ID1, i32 ID2)         { return ID1.ID == ID2; }
inline b32 operator!=(playlist_id ID1, playlist_id ID2) { return ID1.ID != ID2.ID; }
inline b32 operator!=(playlist_id ID1, i32  ID2)        { return ID1.ID != ID2; }

// Miscellaneous
inline displayable_id NewDisplayableID(i32 ID) { return {ID}; }
inline select_id    NewSelectID(i32 ID) { return {ID}; }
inline decode_id    NewDecodeID(i32 ID) { return {ID}; }

#endif //_SOUND__GENERAL__T_D_H
