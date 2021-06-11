/* date = September 23rd 2020 9:18 am */
#ifndef _SOUND__GENERAL__T_D_H
#define _SOUND__GENERAL__T_D_H

struct  file_id        { i32 ID; }; // ::FILE_ID
struct  playlist_id    { i32 ID; }; // ::PLAYLIST_ID
typedef playlist_id batch_id;       // ::BATCH_ID
struct  displayable_id { i32 ID; }; // ::DISPLAYABLE_ID
typedef displayable_id select_id;
typedef displayable_id decode_id;

struct  array_file_id { array_u32 A; };
struct  array_playlist_id { array_u32 A; };
typedef array_playlist_id array_batch_id;

// FileID Array stuff
inline file_id Get(array_file_id  *A, decode_id ID)               { return {(i32)Get(&A->A, ID.ID)}; }
inline file_id Get(array_file_id  *A, playlist_id ID)             { return {(i32)Get(&A->A, ID.ID)}; }
inline void Push(array_file_id *Array, file_id ID)                { Push(&Array->A, ID.ID); }
inline void Copy(array_file_id *A1, array_file_id *A2)            { Copy(&A1->A, &A2->A); }
inline b32  Find(array_file_id *Array, file_id Item, u32 *Result) { return Find(&Array->A, Item.ID, Result); }
inline b32  StackFind(array_file_id *A, file_id Item, i32 *Result) { return StackFind(&A->A, Item.ID, (u32 *)Result); }
inline void Put(array_file_id *Array, displayable_id ID, file_id FileID)  { Put(&Array->A, ID.ID, FileID.ID); }
inline void AppendArray(array_file_id *A1, array_file_id *A2)     { AppendArray(&A1->A, &A2->A); }
inline void MergeArrays(array_file_id *A1, array_file_id *A2)     { MergeArrays(&A1->A, &A2->A); }
inline void PushIfNotExist(array_file_id *Array, file_id ID)      { PushIfNotExist(&Array->A, ID.ID); }


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

// DisplayableID stuff
inline displayable_id NewDisplayableID(i32 ID)                { return {ID}; }
inline b32 operator< (displayable_id ID1, displayable_id ID2) { return ID1.ID <  ID2.ID; }
inline b32 operator< (displayable_id ID1, i32 ID2)            { return ID1.ID <  ID2; }
inline b32 operator<=(displayable_id ID1, displayable_id ID2) { return ID1.ID <= ID2.ID; }
inline b32 operator<=(displayable_id ID1, i32 ID2)            { return ID1.ID <= ID2; }
inline b32 operator> (displayable_id ID1, displayable_id ID2) { return ID1.ID >  ID2.ID; }
inline b32 operator> (displayable_id ID1, i32 ID2)            { return ID1.ID >  ID2; }
inline b32 operator>=(displayable_id ID1, displayable_id ID2) { return ID1.ID >= ID2.ID; }
inline b32 operator>=(displayable_id ID1, i32 ID2)            { return ID1.ID >= ID2; }
inline b32 operator==(displayable_id ID1, displayable_id ID2) { return ID1.ID == ID2.ID; }
inline b32 operator==(displayable_id ID1, i32 ID2)            { return ID1.ID == ID2; }
inline b32 operator!=(displayable_id ID1, displayable_id ID2) { return ID1.ID != ID2.ID; }
inline b32 operator!=(displayable_id ID1, i32  ID2)           { return ID1.ID != ID2; }

// PlaylistID stuff
inline playlist_id NewPlaylistID(i32 ID)                           { return {ID}; }
inline playlist_id Get(array_playlist_id  *A, displayable_id ID)   { return {(i32)Get(&A->A, ID.ID)}; }
inline void        Push(array_playlist_id *A, playlist_id ID)      { Push(&A->A, ID.ID); }
inline playlist_id Take(array_playlist_id *A, displayable_id ID)   { return {(i32)Take(&A->A, ID.ID)}; }
inline b32  StackFind(array_playlist_id *A, playlist_id Item, i32 *Result) { return StackFind(&A->A, Item.ID, (u32 *)Result); }
inline void StackFindAndTake(array_playlist_id *A, playlist_id ID) { StackFindAndTake(&A->A, ID.ID); }
inline b32  StackContains(array_playlist_id *A, playlist_id ID)    { return StackContains(&A->A, ID.ID); }
inline void ShuffleStack(array_playlist_id *A)                     { ShuffleStack(&A->A); }
inline void Copy(array_playlist_id *A1, array_playlist_id *A2)     { Copy(&A1->A, &A2->A); }
inline void Reset(array_playlist_id *A)                            { Reset(&A->A); }
inline void Clear(array_playlist_id *A, u32 Value = 0)             { Clear(&A->A, Value); }
internal void QuickSort(i32 Low, i32 High, array_playlist_id *SortArray, sort_info SortInfo)
{ QuickSort(Low, High, &SortArray->A, SortInfo); }

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
inline select_id    NewSelectID(i32 ID) { return {ID}; }
inline decode_id    NewDecodeID(i32 ID) { return {ID}; }

#endif //_SOUND__GENERAL__T_D_H
