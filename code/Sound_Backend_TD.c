#include "Sound_Backend_TD.h"
#include "GameBasics_TD.h"

inline void
AdvanceToNewline(u8 **String)
{
    while(*String[0] != '\n' && *String[0] != 0 ) (*String)++;
    (*String)++;
}

inline void
ClearFileInfoStruct(mp3_file_info *FileInfo)
{
    For(FileInfo->MaxCount) ResetStringCompound(FileInfo->Metadata[It].DurationString);
    ClearArray(FileInfo->FileName, FileInfo->MaxCount, string_c);
    ClearArray(FileInfo->SubPath, FileInfo->MaxCount, string_c);
    ClearArray(FileInfo->Metadata, FileInfo->MaxCount, mp3_metadata);
    FileInfo->Count = 0;
}

inline void // #ThreadedUse
CreateFileInfoStruct(mp3_file_info *FileInfo, u32 FileInfoCount)
{
    Assert(FileInfo->Count == 0);
    u32 MemorySize = FileInfoCount*sizeof(string_c)*2 + FileInfoCount*sizeof(mp3_metadata) + FileInfoCount*14;
    u8 *Memory = AllocateMemory(&GlobalGameState.JobThreadsArena, MemorySize, Private);
    
    FileInfo->FileName = (string_c *)Memory;
    Memory += FileInfoCount*sizeof(string_c);
    FileInfo->SubPath  = (string_c *)Memory;
    Memory += FileInfoCount*sizeof(string_c);
    FileInfo->Metadata = (mp3_metadata *)Memory;
    Memory += FileInfoCount*sizeof(mp3_metadata);
    
    For(FileInfoCount)
    {
        Assert(FileInfo->Metadata[It].DurationString.Pos == 0);
        FileInfo->Metadata[It].DurationString.S = Memory;
        FileInfo->Metadata[It].DurationString.Length = 13;
        Memory += 14;
    }
    FileInfo->MaxCount = FileInfoCount;
    FileInfo->Count    = 0;
}

inline void // #ThreadedUse
DeleteFileInfoStruct(mp3_file_info *FileInfo)
{
    For(FileInfo->Count)
    {
        DeleteStringCompound(&GlobalGameState.JobThreadsArena, FileInfo->SubPath+It);
        DeleteStringCompound(&GlobalGameState.JobThreadsArena, FileInfo->FileName+It);
    }
    // As I allocate all fileInfo memory as one big block, I just need to dealloc once!
    FreeMemory(&GlobalGameState.JobThreadsArena, FileInfo->FileName);
    FileInfo->Count = 0;
    FileInfo->MaxCount = 0;
    FileInfo->FileName = 0;
    FileInfo->SubPath  = 0;
    FileInfo->Metadata = 0;
}

inline void
ResizeFileInfo(arena_allocator *Arena, mp3_file_info *FileInfo, u32 NewMaxCount)
{
    Assert(NewMaxCount >= 0);
    string_c *OldFileNames    = FileInfo->FileName;
    string_c *OldSubPaths     = FileInfo->SubPath;
    mp3_metadata *OldMetadata = FileInfo->Metadata;
    
    FileInfo->FileName = AllocateArray(Arena, NewMaxCount, string_c);
    FileInfo->SubPath  = AllocateArray(Arena, NewMaxCount, string_c);
    FileInfo->Metadata = AllocateArray(Arena, NewMaxCount, mp3_metadata);
    
    for(u32 It = 0; 
        It < NewMaxCount && 
        It < FileInfo->Count; 
        It++)
    {
        FileInfo->FileName[It] = OldFileNames[It];
        FileInfo->SubPath[It]  = OldSubPaths[It];
        FileInfo->Metadata[It] = OldMetadata[It];
    }
    
    FileInfo->Count = Min((i32)NewMaxCount, (i32)FileInfo->Count);
    FileInfo->MaxCount = NewMaxCount;
}

inline mp3_info *
CreateMP3InfoStruct(arena_allocator *Arena, u32 FileInfoCount)
{
    mp3_info *Result = AllocateStruct(Arena, mp3_info);
    CreateFileInfoStruct(&Result->FileInfo, FileInfoCount);
    Result->DecodeInfo.FileID.A      = CreateArray(Arena, MAX_MP3_DECODE_COUNT);
    Result->DecodeInfo.LastTouched   = CreateArray(Arena, MAX_MP3_DECODE_COUNT);
    
#ifdef DECODE_STREAMING_TMP
    For(MAX_MP3_DECODE_COUNT)
    {
        Result->DecodeInfo.DecodedData[It].buffer = AllocateArray(Arena, 48000*2*DECODE_PRELOAD_SECONDS, i16);
    }
    
    Result->DecodeInfo.PlayingDecoded.buffer = AllocateArray(Arena, CURRENTLY_SUPPORTED_MAX_DECODED_FILE_SIZE/2, i16);
#endif
    return Result;
}

inline b32
IsSongDecoded(mp3_info *MP3Info, file_id FileID, i32 *DecodeID_out = 0)
{
    b32 Result = false;
    if(DecodeID_out) *DecodeID_out = -1;
    For(MP3Info->DecodeInfo.Count)
    {
        if(FileID == Get(&MP3Info->DecodeInfo.FileID, NewDecodeID(It))) 
        {
            Result = true;
            if(DecodeID_out) *DecodeID_out = It;
            break;
        }
    }
    return Result;
}

inline b32
IsSongDecoded(mp3_info *MP3Info, string_compound *FileName, i32 *DecodeID_out = 0)
{
    b32 Result = false;
    if(DecodeID_out) *DecodeID_out = -1;
    For(MP3Info->FileInfo.Count)
    {
        if(CompareStringCompounds(FileName, &MP3Info->FileInfo.FileName[It]))
        {
            Result = IsSongDecoded(MP3Info, NewFileID(It), DecodeID_out);
            break;
        }
    }
    return Result;
}

inline i32
GetSongDecodeID(mp3_info *MP3Info, string_compound *Name)
{
    i32 Result = -1;
    For(MP3Info->FileInfo.Count)
    {
        if(CompareStringAndCompound(Name, (u8 *)MP3Info->FileInfo.FileName[It].S))
        {
            for(i32 DecodeID = 0; DecodeID < (i32)MP3Info->DecodeInfo.Count; DecodeID++)
            {
                if(Get(&MP3Info->DecodeInfo.FileID, NewDecodeID(DecodeID)) == It) 
                {
                    Result = DecodeID;
                    break;
                }
            }
            break;
        }
    }
    return Result;
}

inline file_id
SongNameToFileID(mp3_file_info *MP3FileInfo, string_compound *Name)
{
    file_id Result = NewFileID(-1);
    For(MP3FileInfo->Count)
    {
        if(CompareStringAndCompound(Name, (u8 *)MP3FileInfo->FileName[It].S))
        {
            Result.ID = It;
            break;
        }
    }
    return Result;
}

inline file_id
SongNameToPlaylistID(mp3_info *MP3Info, string_compound *Name)
{
    file_id Result = NewFileID(-1);
    array_file_id *Playlist = &MP3Info->MusicInfo->Playlist.Songs;
    For(Playlist->A.Count)
    {
        file_id FileID = Get(Playlist, NewPlaylistID(It));
        if(CompareStringCompounds(Name, MP3Info->FileInfo.FileName+FileID.ID))
        {
            Result.ID = It;
            break;
        }
    }
    return Result;
}

inline playlist_id
FileIDToPlaylistID(play_list *Playlist, file_id FileID)
{
    playlist_id Result = NewPlaylistID(-1);
    if(FileID < 0) return Result;
    
    For(Playlist->Songs.A.Count)
    {
        playlist_id PID = NewPlaylistID(It);
        if(Get(&Playlist->Songs, PID) == FileID)
        {
            Result = PID;
            break;
        }
    }
    
    return Result;
}

inline file_id
PlaylistIDToFileID(play_list *Playlist, playlist_id PlaylistID)
{
    file_id FileID = NewFileID(-1);
    if(PlaylistID >= 0) FileID = Get(&Playlist->Songs, PlaylistID);
    return FileID;
}

inline playlist_id
OnScreenIDToPlaylistID(music_info *MusicInfo, u32 OnScreenID, file_id *FileID)
{
    playlist_id PlaylistID = NewPlaylistID(-1);
    file_id ID = NewFileID(-1);
    if(!FileID) FileID = &ID;
    *FileID  = Get(&MusicInfo->SortingInfo.Song.Displayable, MusicInfo->DisplayInfo.Song.Base.OnScreenIDs[OnScreenID]);
    StackFind(&MusicInfo->Playlist.Songs, *FileID, &PlaylistID.ID);
    
    return PlaylistID;
}

inline displayable_id
FileIDToSongDisplayableID(music_display_column *DisplayColumn, file_id FileID)
{
    displayable_id Result = NewDisplayableID(-1);
    if(FileID < 0) return Result;
    
    For(DisplayColumn->SortingInfo->Displayable.A.Count)
    {
        displayable_id DID = NewDisplayableID(It);
        if(Get(&DisplayColumn->SortingInfo->Displayable, DID) == FileID)
        {
            Result = DID;
            break;
        }
    }
    
    return Result;
}

// Acquires the corresponding Genre/Artist/Album DisplayID for given FileID
inline displayable_id
FileIDToColumnDisplayID(music_display_column *DisplayColumn, file_id FileID)
{
    displayable_id Result = NewDisplayableID(-1);
    column_sorting_info *SortingColumn = DisplayColumn->SortingInfo;
    string_c *CompareS = 0;
    switch(DisplayColumn->Type)
    {
        case columnType_Genre:
        {
            CompareS = &GlobalGameState.MP3Info->FileInfo.Metadata[FileID.ID].Genre;
        } break;
        case columnType_Artist:
        {
            CompareS = &GlobalGameState.MP3Info->FileInfo.Metadata[FileID.ID].Artist;
        } break;
        case columnType_Album:
        {
            CompareS = &GlobalGameState.MP3Info->FileInfo.Metadata[FileID.ID].Album;
        } break;
        
        case columnType_Song:
        {
            return FileIDToSongDisplayableID(DisplayColumn, FileID);
        } break;
        InvalidDefaultCase;
    }
    
    For(SortingColumn->Displayable.A.Count)
    {
        displayable_id DID = NewDisplayableID(It);
        string_c *Name  = SortingColumn->Batch.Names+Get(&SortingColumn->Displayable, DID).ID;
        if(CompareStringCompounds(CompareS, Name)) Result = DID;
        
    }
    
    return Result;
}

inline displayable_id
SortingIDToColumnDisplayID(music_display_column *DisplayColumn, batch_id BatchID)
{
    displayable_id Result = NewDisplayableID(-1);
    column_sorting_info *SortingColumn = DisplayColumn->SortingInfo;
    string_c *CompareS = 0;
    switch(DisplayColumn->Type)
    {
        case columnType_Genre:
        {
            CompareS = &SortingColumn->Batch.Names[BatchID.ID];
        } break;
        case columnType_Artist:
        {
            CompareS = &SortingColumn->Batch.Names[BatchID.ID];
        } break;
        case columnType_Album:
        {
            CompareS = &SortingColumn->Batch.Names[BatchID.ID];
        } break;
        
        InvalidDefaultCase;
    }
    
    For(SortingColumn->Displayable.A.Count)
    {
        displayable_id DID = NewDisplayableID(It);
        string_c *Name  = SortingColumn->Batch.Names+Get(&SortingColumn->Displayable, DID).ID;
        if(CompareStringCompounds(CompareS, Name)) Result = DID;
    }
    
    return Result;
}

inline file_id
GetNextSong(play_list *Playlist, playing_song *PlayingSong)
{
    file_id FileID = NewFileID(-1);
    if(PlayingSong->PlayUpNext) FileID = Get(&Playlist->UpNext, NewPlaylistID(0));
    else FileID = PlaylistIDToFileID(Playlist, PlayingSong->PlaylistID);
    return FileID; 
}

inline playlist_id
CheckForLooping(play_list *Playlist, playlist_id PlayingSongID, play_loop Looping)
{
    playlist_id Result = PlayingSongID;
    if(Looping == playLoop_Repeat) ; // PlaylistID does not change;
    else if(Looping == playLoop_Loop) Result.ID = (Result.ID+1)%Playlist->Songs.A.Count;
    else 
    {
        Result.ID++;
        if(Result >= (i32)Playlist->Songs.A.Count) Result.ID = -1;
    }
    return Result;
}

internal void
SetNextSong(play_list *Playlist, playing_song *PlayingSong, play_loop Looping)
{
    Assert(PlayingSong->PlaylistID >= -1);
    
    if(Playlist->UpNext.A.Count != 0) 
    {
        if(PlayingSong->PlayUpNext) 
        {
            Take(&Playlist->UpNext, NewPlaylistID(0));
            if(Playlist->UpNext.A.Count == 0) 
            {
                PlayingSong->PlayUpNext = false;
                PlayingSong->PlaylistID = CheckForLooping(Playlist, PlayingSong->PlaylistID, Looping);
                PlayingSong->FileID = PlaylistIDToFileID(Playlist, PlayingSong->PlaylistID);
            }
        }
        else
        {
            PlayingSong->PlayUpNext = true;
        }
    }
    else 
    {
        PlayingSong->PlaylistID = CheckForLooping(Playlist, PlayingSong->PlaylistID, Looping);
        PlayingSong->FileID = PlaylistIDToFileID(Playlist, PlayingSong->PlaylistID);
    }
}

inline playlist_id
GetSongAfterCurrent(play_list *Playlist, playlist_id PlaylistID)
{
    Assert(PlaylistID.ID > -1);
    playlist_id Result = NewPlaylistID((PlaylistID.ID+1)%Playlist->Songs.A.Count);
    return Result;
}

inline playlist_id
GetPreviousSong(play_list *Playlist, playlist_id PlaylistID)
{
    playlist_id Result = NewPlaylistID(-1);
    Assert(PlaylistID.ID >= -1);
    if(PlaylistID.ID != -1)
    {
        Result.ID = PlaylistID.ID - 1;
        if(Result.ID < 0) Result.ID = Playlist->Songs.A.Count-1;
    }
    
    return Result;
}

inline void
SetPreviousSong(play_list *Playlist, playing_song *PlayingSong, play_loop Looping)
{
    Assert(PlayingSong->PlaylistID >= -1);
    if(PlayingSong->PlaylistID != -1)
    {
        if(Looping == playLoop_Repeat) ;
        else if(Looping == playLoop_Loop) 
        {
            PlayingSong->PlaylistID.ID -= 1;
            PlayingSong->FileID = PlaylistIDToFileID(Playlist, PlayingSong->PlaylistID);
            if(PlayingSong->PlaylistID < 0) 
            {
                PlayingSong->PlaylistID.ID = Playlist->Songs.A.Count-1;
                PlayingSong->FileID = PlaylistIDToFileID(Playlist, PlayingSong->PlaylistID);
            }
        }
        else 
        {
            PlayingSong->PlaylistID.ID -= 1;
            PlayingSong->FileID = PlaylistIDToFileID(Playlist, PlayingSong->PlaylistID);
        }
    }
}

internal void
UseDisplayableAsPlaylist(music_info *MusicInfo)
{
    file_id FileID = PlaylistIDToFileID(&MusicInfo->Playlist, MusicInfo->PlayingSong.PlaylistID);
    
    Reset(&MusicInfo->Playlist.Songs);
    For(MusicInfo->SortingInfo.Song.Displayable.A.Count)
    {
        Push(&MusicInfo->Playlist.Songs, Get(&MusicInfo->SortingInfo.Song.Displayable, NewDisplayableID(It)));
    }
    
    // If the currently playing song is in the new playlist set PlaylistID.
    // If not, the music stops?
    playlist_id PlaylistID = NewPlaylistID(0);
    if(StackFind(&MusicInfo->Playlist.Songs, FileID, &PlaylistID.ID))
    {
        MusicInfo->PlayingSong.PlaylistID = PlaylistID;
    }
    else 
    {
        MusicInfo->PlayingSong.PlaylistID.ID = -1;
        MusicInfo->PlayingSong.DecodeID = -1;
    }
    
    MusicInfo->PlayingSong.FileID = FileID;
}

internal b32 // #ThreadedUse
FindAllMP3FilesInFolder(arena_allocator *TransientArena, string_compound *FolderPath, string_compound *SubPath,
                        mp3_file_info *ResultingFileInfo)
{
    b32 Result = false;
    string_compound FolderPathStar = NewStringCompound(TransientArena, 255);
    ConcatStringCompounds(3, &FolderPathStar, FolderPath, SubPath);
    AppendCharToCompound(&FolderPathStar, '*');
    
    string_w WideFolderPath = {};
    ConvertString8To16(TransientArena, &FolderPathStar, &WideFolderPath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WideFolderPath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        b32 HasNextFile = true;
        string_compound FileType = NewStringCompound(TransientArena, 16);
        
        while(HasNextFile && ResultingFileInfo->Count < ResultingFileInfo->MaxCount)
        {
            string_c FileName = {};
            ConvertString16To8(TransientArena, FileData.cFileName, &FileName);
            
            if     (CompareStringAndCompound(&FileName, (u8 *)".") ) {}
            else if(CompareStringAndCompound(&FileName, (u8 *)"..")) {}
            else
            {
                if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    i32 PathLength = FileName.Pos+SubPath->Pos+1;
                    string_compound NewSubFolderPath = NewStringCompound(TransientArena, PathLength);
                    ConcatStringCompounds(3, &NewSubFolderPath, SubPath, &FileName);
                    AppendCharToCompound(&NewSubFolderPath, '\\');
                    
                    FindAllMP3FilesInFolder(TransientArena, FolderPath, &NewSubFolderPath, ResultingFileInfo);
                    DeleteStringCompound(TransientArena, &NewSubFolderPath);
                }
                else
                {
                    i32 LastDot = FindLastOccurrenceOfCharInStringCompound(&FileName, '.');
                    if(LastDot > 0)
                    {
                        PasteStringCompoundIntoCompound(&FileType, 0, &FileName, LastDot+1, FileName.Pos-(LastDot+1));
                        u8 *MP3Extension = (u8 *)"mp3";
                        if(CompareStringAndCompound(&FileType, MP3Extension))
                        {
                            ResultingFileInfo->SubPath[ResultingFileInfo->Count] = 
                                NewStringCompound(&GlobalGameState.JobThreadsArena, SubPath->Pos);
                            AppendStringCompoundToCompound(ResultingFileInfo->SubPath+ResultingFileInfo->Count, SubPath);
                            
                            ResultingFileInfo->FileName[ResultingFileInfo->Count] =
                                NewStringCompound(&GlobalGameState.JobThreadsArena, FileName.Pos);
                            AppendStringCompoundToCompound(ResultingFileInfo->FileName+ResultingFileInfo->Count, &FileName);
                            
                            ResultingFileInfo->Count++;
                            Assert(ResultingFileInfo->Count <= MAX_MP3_INFO_COUNT);
                            Result = true;
                        }
                        ResetStringCompound(FileType);
                    }
                }
            }
            HasNextFile = FindNextFileW(FileHandle, &FileData);
            DeleteStringCompound(TransientArena, &FileName);
        } 
        DeleteStringCompound(TransientArena, &FileType);
    }
    
    //DebugLog(555, "Found %i songs in folder %s.\n", ResultingFileInfo->Count, FolderPathStar.S);
    
    DeleteStringW(TransientArena, &WideFolderPath);
    DeleteStringCompound(TransientArena, &FolderPathStar);
    return Result;
}

inline b32
FoundAllMetadata(i32 Flags)
{
    b32 Result = (Flags == (  metadata_Title 
                            | metadata_Artist 
                            | metadata_Album 
                            | metadata_Genre 
                            | metadata_Track 
                            | metadata_Year
                            | metadata_Duration));
    return Result;
}

internal void
MetadataID3v1(arena_allocator *Arena, read_file_result *File, mp3_metadata *Metadata)
{
    if(File->Size >= 128)
    {
        u8 *Current = (u8 *)File->Data + (File->Size-128);
        
        if(*Current++ == 'T' &&
           *Current++ == 'A' &&
           *Current++ == 'G')
        {
            u8 Title[31] = {};
            u8 Artist[31] = {};
            u8 Album[31] = {};
            u8 Year[5] = {};
            u8 Track = 0;
            u8 Genre;
            
            For(30) Title[It] = *Current++;
            For(30) Artist[It] = *Current++;
            For(30) Album[It] = *Current++;
            For(4)  Year[It] = *Current++;
            Current += 28;
            if(*Current++ == 0) Track = *Current;
            Current++;
            Genre = *Current++;
            
            if(~Metadata->FoundFlags & metadata_Title &&
               StringLength(Title) != 0)
            {
                Metadata->Title  = NewStringCompound(Arena, StringLength(Title));
                AppendStringToCompound(&Metadata->Title, Title);
                EatLeadingSpaces(&Metadata->Title);
                EatTrailingSpaces(&Metadata->Title);
                Metadata->FoundFlags |= metadata_Title;
            }
            if(~Metadata->FoundFlags & metadata_Artist &&
               StringLength(Artist) != 0)
            {
                Metadata->Artist = NewStringCompound(Arena, StringLength(Artist));
                AppendStringToCompound(&Metadata->Artist, Artist);
                EatLeadingSpaces(&Metadata->Artist);
                EatTrailingSpaces(&Metadata->Artist);
                Metadata->FoundFlags |= metadata_Artist;
            }
            if(~Metadata->FoundFlags & metadata_Album &&
               StringLength(Album) != 0)
            {
                Metadata->Album  = NewStringCompound(Arena, StringLength(Album));
                AppendStringToCompound(&Metadata->Album, Album);
                EatLeadingSpaces(&Metadata->Album);
                EatTrailingSpaces(&Metadata->Album);
                Metadata->FoundFlags |= metadata_Album;
            }
            
            if(~Metadata->FoundFlags & metadata_Year)
            {
                u8 Length = 0;
                u32 Y = ProcessNextU32InString(Year, '\0', Length);
                if(Length > 0)
                {
                    Metadata->YearString = NewStringCompound(Arena, 4);
                    AppendStringToCompound(&Metadata->YearString, Year);
                    Metadata->Year = Y;
                    Metadata->FoundFlags |= metadata_Year;
                }
            }
            
            if(~Metadata->FoundFlags & metadata_Genre)
            {
                if(Genre < ArrayCount(GenreTypes))
                {
                    Metadata->Genre = GenreTypes[Genre];
                    Metadata->FoundFlags |= metadata_Genre;
                }
            }
            
            if(~Metadata->FoundFlags & metadata_Track)
            {
                Metadata->TrackString = NewStringCompound(Arena, 4);
                I32ToString(&Metadata->TrackString, (Track<<0));
                Metadata->Track = Track;
                Metadata->FoundFlags |= metadata_Track;
            }
        }
    }
}

internal i32
MetadataID3v2_Helper(arena_allocator *Arena, u8 *C, u8 *Frame, string_compound *S, u32 MinorVersion)
{
    // Header:
    // 4 Bytes | 4 Bytes | 2 Bytes
    // Tag ID  | Size    | Flags
    i32 Result = 0;
    u32 VersionTagLength    = (MinorVersion == 2) ? 3 : 4;
    u32 VersionHeaderLength = (MinorVersion == 2) ? 6 : 10;
    if(StringCompare(C, Frame, 0, VersionTagLength))
    {
        i32 Length = 0;
        if(MinorVersion == 2) Length =            C[3]<<16 | C[4]<<8 | C[5]<<0;
        else                  Length = C[4]<<24 | C[5]<<16 | C[6]<<8 | C[7]<<0;
        
        if(Length <= 0) ;
        else if(Length > 2500)
        {
            char B[255];
            sprintf_s(B, "Error:: Could not load %s metadata. Tag size was %i, must be erroneous !\n", Frame, Length);
            OutputDebugStringA(B);
        }
        else
        {
            *S = NewStringCompound(Arena, Length);
            C = C+VersionHeaderLength;
            Result += VersionHeaderLength;
            // TODO:: This is a stupid hack. Read unicode wide and convert it to utf-8...
            // for both cases (ID3v2_2, ID3v2_3)
            if(*C == 0 || *C == 3) // ISO-8859-1 encoding
            {
                C++;
                Length--;
                Result++;
            }
            else if(*C == 1 || *C == 2) // unicode encoding (skipping Byte order mark (BOM)) 
            {
                C++;
                Length--;
                Result++;
                if(C[0] == 0xFF && C[1] == 0xFE) // _may_ have a unicode NULL (FF FE) after BOM
                {
                    C += 2; 
                    Length -= 2;
                    Result += 2;
                }
            }
            if(Length > 0)
            {
                For((u32)Length)
                {
                    if(*C != '\0') S->S[S->Pos++] = *C;
                    C++;
                }
                EatLeadingSpaces(S);
                
                Result += Length - 1; // -1 to not skip anything when increasing It in For loop (parent)
            }
            else Result = 0;
        }
    }
    return Result;
}

internal u32
MetadataID3v2_3(arena_allocator *MainArena, arena_allocator *TransientArena, 
                read_file_result *File, mp3_metadata *Metadata)
{
    // Spec: http://id3.org/id3v2.3.0
    // Header:
    // 3 Byte | 2 Byte | 1 Byte | 4 Byte
    // Tag:ID3| Version| Flags  | Size
    
    u32 DataLength = 0;
    u8 *TITLE = 0, *ALBUM = 0, *ARTIST = 0, *GENRE = 0, *TRACK = 0, *YEAR = 0, *DURATION = 0;
    
    u8 *Current = (u8 *)File->Data;
    u8 MinorVersion = Current[3];
    string_c ID3S = NewStaticStringCompound("ID3");
    if(CompareStringAndCompound(&ID3S, Current, 3))
    {
        if(MinorVersion == 2)
        {
            TITLE  = (u8 *)"TT2";
            ALBUM  = (u8 *)"TAL";
            ARTIST = (u8 *)"TP1";
            GENRE  = (u8 *)"TCO";
            TRACK  = (u8 *)"TRK";
            YEAR   = (u8 *)"TYE";
            DURATION = (u8 *)"TLE";
        }
        else if(MinorVersion == 3 || MinorVersion == 4)
        {
            TITLE  = (u8 *)"TIT2";
            ALBUM  = (u8 *)"TALB";
            ARTIST = (u8 *)"TPE1";
            GENRE  = (u8 *)"TCON";
            TRACK  = (u8 *)"TRCK";
            YEAR   = (MinorVersion == 3) ? (u8 *)"TYER" : (u8 *)"TDRC";
            DURATION = (u8 *)"TLEN";
        }
        else return DataLength;
        
        Current += 3+2+1; // Skip header bytes, to size
        // NOTE:: For size only the seven lower bits are used...
        DataLength  = Current[0]<<21 | Current[1]<<14 | Current[2]<<7 | Current[3]<<0;
        
        Current += 4;
        i32 JmpIter = 0;
        u32 LoopSize = (DataLength < File->Size) ? DataLength : File->Size;
        For(LoopSize)
        {
            if(~Metadata->FoundFlags & metadata_Title)
            {
                JmpIter = MetadataID3v2_Helper(MainArena, Current+It, TITLE, &Metadata->Title, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Title;
            }
            if(~Metadata->FoundFlags & metadata_Album)
            {
                JmpIter = MetadataID3v2_Helper(MainArena, Current+It, ALBUM, &Metadata->Album, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Album;
            }
            if(~Metadata->FoundFlags & metadata_Artist)
            {
                JmpIter = MetadataID3v2_Helper(MainArena, Current+It, ARTIST, &Metadata->Artist, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Artist;
            }
            if(~Metadata->FoundFlags & metadata_Genre)
            {
                string_compound GenreNr = {};
                JmpIter = MetadataID3v2_Helper(TransientArena, Current+It, GENRE, &GenreNr, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Genre;
                if(Metadata->FoundFlags & metadata_Genre)
                {
                    if(GenreNr.S[0] == '(')
                    {
                        u8 L = 0;
                        u32 GNr = ProcessNextU32InString(GenreNr.S+1, ')', L);
                        Metadata->Genre = NewStringCompound(MainArena, GenreTypes[GNr].Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreTypes[GNr]);
                    }
                    else if(IsStringCompANumber(&GenreNr))
                    {
                        u8 L = 0;
                        u32 GNr = ProcessNextU32InString(GenreNr.S, '\0', L);
                        Metadata->Genre = NewStringCompound(MainArena, GenreTypes[GNr].Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreTypes[GNr]);
                    }
                    else 
                    {
                        Metadata->Genre = NewStringCompound(MainArena, GenreNr.Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreNr);
                    }
                    
                    DeleteStringCompound(TransientArena, &GenreNr);
                }
            }
            
            if(~Metadata->FoundFlags & metadata_Track)
            {
                string_compound Track = {};
                JmpIter = MetadataID3v2_Helper(TransientArena, Current+It, TRACK, &Track, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Track;
                if(Metadata->FoundFlags & metadata_Track)
                {
                    i32 SlashPos = FindFirstOccurrenceOfCharInStringCompound(&Track, '/');
                    if(SlashPos != -1) Track.Pos = SlashPos;
                    Metadata->Track = ConvertU32FromString(Track.S, Track.Pos);
                    Assert(Metadata->Track < 10000);
                    Metadata->TrackString = NewStringCompound(MainArena, 4);
                    I32ToString(&Metadata->TrackString, Metadata->Track);
                    
                    DeleteStringCompound(TransientArena, &Track);
                }
            }
            if(~Metadata->FoundFlags & metadata_Year)
            {
                string_compound Year = {};
                JmpIter = MetadataID3v2_Helper(TransientArena, Current+It, YEAR, &Year, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Year;
                if(Metadata->FoundFlags & metadata_Year)
                {
                    if(Year.Pos == 0) Metadata->Year = 0;
                    else Metadata->Year = ConvertU32FromString(Year.S, 4);
                    Assert(Metadata->Year < 10000);
                    Metadata->YearString = NewStringCompound(MainArena, 4);
                    I32ToString(&Metadata->YearString, Metadata->Year);
                    DeleteStringCompound(TransientArena, &Year);
                }
            }
            if(~Metadata->FoundFlags & metadata_Duration)
            {
                string_c Duration = {};
                JmpIter = MetadataID3v2_Helper(TransientArena, Current+It, DURATION, &Duration, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Duration;
                if(Metadata->FoundFlags & metadata_Duration)
                {
                    if(Duration.Pos == 0) Metadata->Duration = 0;
                    else Metadata->Duration = ConvertU32FromString(Duration.S, Duration.Pos);
                    //DebugLog(20, "Duration: %i\n", Metadata->Duration);
                    DeleteStringCompound(TransientArena, &Duration);
                }
            }
            if(FoundAllMetadata(Metadata->FoundFlags)) break;
        }
    }
    return DataLength;
}

inline u32
ExtractMetadata(arena_allocator *MainArena, arena_allocator *TransientArena, read_file_result *File, mp3_metadata *Metadata)
{
    u32 MetadataSize3 = 0;
    u32 MetadataSize2 = 0;
    if(!FoundAllMetadata(Metadata->FoundFlags)) MetadataSize3 = MetadataID3v2_3(MainArena, TransientArena, File, Metadata);
    //if(!FoundAllMetadata(Metadata->FoundFlags)) MetadataID3v1(MainArena, File, Metadata);
    return (MetadataSize3 > 0) ? MetadataSize3 : MetadataSize2;
}

inline void
TouchDecoded(mp3_decode_info *DecodeInfo, u32 DecodeID)
{
    Put(&DecodeInfo->LastTouched, DecodeID, DecodeInfo->TouchCount++);
}

inline i32
GetNextDecodeIDToEvict(mp3_decode_info *DecodeInfo, array_u32 *IgnoreDecodeIDs = 0)
{
    i32 Result = -1;
    if(DecodeInfo->Count < MAX_MP3_DECODE_COUNT)
    {
        Result = DecodeInfo->Count++;
        DecodeInfo->FileID.A.Count++;
        DecodeInfo->LastTouched.Count++;
    }
    else
    {
        if(IgnoreDecodeIDs)
        {
            For(IgnoreDecodeIDs->Count) TouchDecoded(DecodeInfo, Get(IgnoreDecodeIDs, It));
        }
        Result = GetSmallestEntryID(&DecodeInfo->LastTouched, DecodeInfo->Count);
        
        playing_song *PlayingSong = &GlobalGameState.MusicInfo.PlayingSong;
        if(PlayingSong->PlaylistID >= 0 && PlayingSong->DecodeID == Result)
        {
            Put(&DecodeInfo->LastTouched, Result, DecodeInfo->TouchCount++);
            Result = GetSmallestEntryID(&DecodeInfo->LastTouched, DecodeInfo->Count);
            Assert(PlayingSong->DecodeID != Result);
        }
    }
    Assert(Result >= 0);
    return Result;
}

inline void
CreateSongDurationForMetadata(mp3_info *MP3Info, file_id FileID, i32 DecodeID)
{
    if(~MP3Info->FileInfo.Metadata[FileID.ID].FoundFlags & metadata_Duration)
    {
        mp3_metadata *MD = &MP3Info->FileInfo.Metadata[FileID.ID];
        mp3dec_file_info_t *DInfo = &MP3Info->DecodeInfo.DecodedData[DecodeID];
        
        MD->Duration = (u32)DInfo->samples/DInfo->channels/DInfo->hz*1000;
        MillisecondsToMinutes(MD->Duration, &MD->DurationString);
        MD->FoundFlags |= metadata_Duration;
    }
}

internal error_item
LoadAndDecodeMP3Data(arena_allocator *ScratchArena, mp3_info *MP3Info, file_id FileID, i32 DecodeID)
{
    error_item Result = {(load_error_codes)DecodeID, FileID};
    string_compound FilePath = NewStringCompound(ScratchArena, 255);
    ConcatStringCompounds(4, &FilePath, &MP3Info->FolderPath, 
                          MP3Info->FileInfo.SubPath + FileID.ID, 
                          MP3Info->FileInfo.FileName + FileID.ID);
    
    read_file_result FileData = {};
    if(ReadEntireFile(ScratchArena, &FileData, FilePath.S))
    {
        if(FileData.Size != 0)
        {
            mp3dec_t Dec = {};
            mp3dec_init(&Dec);
            
            free(MP3Info->DecodeInfo.DecodedData[DecodeID].buffer);
            mp3dec_load_buf(&Dec, FileData.Data, FileData.Size, MP3Info->DecodeInfo.DecodedData+DecodeID, NULL, NULL);
            
            i32 DecodeResult = MP3Info->DecodeInfo.DecodedData[DecodeID].samples ? 0 : -1;
            if(DecodeResult)
            {
                Result.Code = loadErrorCode_DecodingFailed;
                OutputDebugStringA("Error.\n");
            }
            else
            {
                Put(&MP3Info->DecodeInfo.FileID, NewDecodeID(DecodeID), FileID);
                TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
                DebugLog(255, "Done loading mp3 file with name %s.\n", MP3Info->FileInfo.Metadata[FileID.ID].Title.S);
                
                // Check if song duration was already saved, if not, create it.
                CreateSongDurationForMetadata(MP3Info, FileID, DecodeID);
            }
        }
        else 
        {
            Result.Code = loadErrorCode_EmptyFile;
        }
        FreeFileMemory(ScratchArena, FileData.Data);
    }
    else 
    {
        OutputDebugStringA("ERROR:: Failed to load file.\n");
        Result.Code = loadErrorCode_FileLoadFailed;
    }
    DeleteStringCompound(ScratchArena, &FilePath);
    
    if(DecodeID >= 0) MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = false;
    return Result;
}

internal error_item
LoadAndDecodeMP3Data(arena_allocator *ScratchArena, mp3_info *MP3Info, file_id FileID)
{
    i32 DecodeID = GetNextDecodeIDToEvict(&MP3Info->DecodeInfo);
    return LoadAndDecodeMP3Data(ScratchArena, MP3Info, FileID, DecodeID);
}

internal mp3dec_file_info_t
DecodeMP3StartFrames(mp3dec_t *MP3Decoder, read_file_result File, i16 Buffer[], u32 DecodeFrameAmount){
    mp3dec_file_info_t Result = {Buffer};
    
    /*typedef struct
   {
       int frame_bytes;
       int channels;
       int hz;
       int layer;
       int bitrate_kbps;
   } mp3dec_frame_info_t;*/
    mp3dec_frame_info_t FrameInfo;
    //i16 PCM[MINIMP3_MAX_SAMPLES_PER_FRAME];
    
    i16 *RunningBuffer = Buffer;
    For(DecodeFrameAmount)
    {
        // mp3dec_decode_frame result:
        // 0:    No MP3 data was found in the input buffer
        // 384:  Layer 1
        // 576:  MPEG 2 Layer 3
        // 1152: Otherwise
        u32 SampleCount = mp3dec_decode_frame(MP3Decoder, File.Data, File.Size, RunningBuffer, &FrameInfo);
        
        if(SampleCount > 0 && FrameInfo.frame_bytes > 0) // Succesfull decode.
        {
            File.Data += FrameInfo.frame_bytes;
            File.Size -= FrameInfo.frame_bytes;
            RunningBuffer  += SampleCount*FrameInfo.channels;
            Result.samples += SampleCount*FrameInfo.channels;
            
            if(It == 0)
            {
                Result.channels = FrameInfo.channels;
                Result.hz = FrameInfo.hz;
                Result.layer = FrameInfo.layer;
                Result.avg_bitrate_kbps = FrameInfo.bitrate_kbps;
            }
            Assert((MINIMP3_MAX_SAMPLES_PER_FRAME*DecodeFrameAmount) >= Result.samples);
        }
        else if(SampleCount == 0 && FrameInfo.frame_bytes > 0) // Decoder skipped ID3/invalid data (no samples were decoded).
        {
            File.Data += FrameInfo.frame_bytes;
            File.Size -= FrameInfo.frame_bytes;
            It--; // Try again to decode frame.
        }
        else if(SampleCount == 0 && FrameInfo.frame_bytes == 0) // Nothing was there to be decoded or skipped. EOF
        {
            break;
        }
        else 
        {
            //Assert(SampleCount != 0 || FrameInfo.frame_bytes != 0); // Insufficient data.
            Assert(false); // Double Assert just to know if there can be another case I did not catch.
        }
    }
    
    return Result;
}

internal error_item
LoadAndDecodeMP3StartFrames(arena_allocator *FixArena, arena_allocator *ScratchArena, i32 SecondsToDecode, file_id FileID, 
                            i32 DecodeID, mp3dec_file_info_t *DecodeResult)
{
    // 1. Build the complete filepath
    // 2. Extract the size of the metadata
    // 3. Decode first frame to get information of the file
    // 4. Calculate maximum size to load for wanted amount
    // 5. Load and decode the stuff
    error_item Result = {(load_error_codes)DecodeID, FileID};
    
    mp3_info *MP3Info = GlobalGameState.MP3Info;
    //mp3dec_file_info_t *DecodeResult = MP3Info->DecodeInfo.DecodedData + DecodeID;
    i16 *ExistingBuffer = DecodeResult->buffer;
    
    // 1.
    NewEmptyLocalString(FilePath, 255);
    ConcatStringCompounds(4, &FilePath, &MP3Info->FolderPath, MP3Info->FileInfo.SubPath + FileID.ID, MP3Info->FileInfo.FileName + FileID.ID);
    
    
    mp3dec_t MP3Decoder = {};
    mp3dec_init(&MP3Decoder);
    
    // 2.
    u32 MetadataSize    = ExtractMetadataSize(ScratchArena, &FilePath);
    u32 ReadAmount      = MetadataSize + MINIMP3_MAX_SAMPLES_PER_FRAME + 512;
    i16 *TmpInfoStorage = AllocateArray(ScratchArena, ReadAmount, i16);
    
    read_file_result File = {};
    if(ReadBeginningOfFile(ScratchArena, &File, FilePath.S, ReadAmount))
    {
        // 3. 
        *DecodeResult = DecodeMP3StartFrames(&MP3Decoder, File, TmpInfoStorage, 1);
        FreeFileMemory(ScratchArena, File.Data);
        
        u32 DecodeSampleAmount = DecodeResult->hz*SecondsToDecode; 
        // samples has channels already in it. But I need the frame amount which is channel indipendent.
        u32 DecodeFrameAmount  = DecodeSampleAmount/(u32)(DecodeResult->samples/DecodeResult->channels);
        // 4. This should always be enough, as the mp3 file should be smaller, as it is compressed...
        ReadAmount = DecodeSampleAmount*DecodeResult->channels*sizeof(i16) + DecodeFrameAmount*sizeof(u32) + MetadataSize; 
        if(ReadBeginningOfFile(ScratchArena, &File, FilePath.S, ReadAmount))
        {
            Assert(DecodeResult->hz <= 48000); // DecodeData[] in Decode_info size calc. is based on this as a max!
            Assert(ExistingBuffer);
            
            // 5.
            *DecodeResult = DecodeMP3StartFrames(&MP3Decoder, File, ExistingBuffer, DecodeFrameAmount);
            
            if(DecodeResult->samples == 0)
            {
                Result.Code = loadErrorCode_DecodingFailed;
            }
            else
            {
                Put(&MP3Info->DecodeInfo.FileID, NewDecodeID(DecodeID), FileID);
                TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
                DebugLog(255, "Done loading mp3 file with name %s.\n", MP3Info->FileInfo.Metadata[FileID.ID].Title.S);
                
                CreateSongDurationForMetadata(MP3Info, FileID, DecodeID);
            }
            
            FreeFileMemory(ScratchArena, File.Data);
        }
        else
        {
            Result.Code = loadErrorCode_FileLoadFailed;
        }
    }
    else
    {
        Result.Code = loadErrorCode_FileLoadFailed;
    }
    
    FreeMemory(ScratchArena, TmpInfoStorage);
    
    if(DecodeID >= 0) MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = false;
    return Result;
}

internal u32 // #ThreadedUse
ExtractMetadataSize(arena_allocator *TransientArena, string_c *CompletePath)
{
    u32 Result = 0;
    
    read_file_result FileData = {};
    // NOTE:: Two staged approach: try loading only header and extract tag length.
    // if not ID3v2/3 metadata, then load entire file for ID3v1 
    // 
    // Spec: http://id3.org/id3v2.3.0
    // Header:
    // 3 Byte | 2 Byte | 1 Byte | 4 Byte
    // Tag:ID3| Version| Flags  | Size
    if(ReadBeginningOfFile(TransientArena, &FileData, CompletePath->S, 10)) 
    {
        u8 *Current = (u8 *)FileData.Data;
        Current[3] = 0;
        string_c ID3S = NewStaticStringCompound("ID3");
        if(CompareStringAndCompound(&ID3S, Current))
        {
            Current += 3+2+1; // Skip header bytes, to size.
            // NOTE:: For size only the seven lower bits are used...
            Result  = Current[0]<<21 | Current[1]<<14 | Current[2]<<7 | Current[3]<<0;
            Result += 10; // Size excludes the header... so add it!
        }
        FreeFileMemory(TransientArena, FileData.Data);
    }
    
    return Result;
}

internal void // #ThreadedUse
CrawlFileForMetadata(arena_allocator *TransientArena, mp3_metadata *MD, string_c *FilePath)
{
    u32 DataLength = ExtractMetadataSize(TransientArena, FilePath);
    read_file_result FileData = {};
    if(DataLength > 0 && ReadBeginningOfFile(TransientArena, &FileData, FilePath->S, DataLength)) 
    {
        ExtractMetadata(&GlobalGameState.JobThreadsArena, TransientArena, &FileData, MD);
        FreeFileMemory(TransientArena, FileData.Data);
    }
    
    if(!FoundAllMetadata(MD->FoundFlags))
    {
        if(ReadEndOfFile(TransientArena, &FileData, FilePath->S, 128))
        {
            MetadataID3v1(&GlobalGameState.JobThreadsArena, &FileData, MD);
            FreeFileMemory(TransientArena, FileData.Data);
        }
    }
}

internal void // #ThreadedUse
CrawlFilesForMetadata(arena_allocator *TransientArena, mp3_file_info *FileInfo, 
                      string_c *FolderPath, u32 *CurrentCrawlCount = 0)
{
    OutputDebugStringA("\n");
    string_compound FilePath = NewStringCompound(TransientArena, 255);
    
    For(FileInfo->Count)
    {
        if(FoundAllMetadata(FileInfo->Metadata[It].FoundFlags)) continue;
        ConcatStringCompounds(4, &FilePath, FolderPath, FileInfo->SubPath+It, FileInfo->FileName+It);
        
        CrawlFileForMetadata(TransientArena, FileInfo->Metadata+It, &FilePath);
        
        WipeStringCompound(&FilePath);
        if(CurrentCrawlCount)
        {
            *CurrentCrawlCount = It;
        }
        if(It%100==0) 
        {
            DebugLog(500, "%i of %i\n", It, FileInfo->Count);
        }
    }
    DeleteStringCompound(TransientArena, &FilePath);
}

internal void // #ThreadedUse
MillisecondsToMinutes(u32 Millis, string_c *Out)
{
    r32 Seconds = Millis/1000.0f;
    r32 Minutes = Seconds/60.0f;
    i32 RestSeconds = Floor(60.0f*Abs(Minutes-Floor(Minutes)));
    
    Assert(RestSeconds < 60.0f);
    char B[10];
    if(Floor(Minutes) < 10) sprintf_s(B, "0%i:", Floor(Minutes));
    else             sprintf_s(B, "%i:", Floor(Minutes));
    char B2[3];
    if(RestSeconds < 10) sprintf_s(B2, "0%i", RestSeconds);
    else                 sprintf_s(B2, "%i", RestSeconds);
    
    AppendStringToCompound(Out, (u8 *)B);
    AppendStringToCompound(Out, (u8 *)B2);
}

inline i32
AddSongToSortBatch(arena_allocator *Arena, sort_batch *Batch, string_c *InsertCheck)
{
    i32 Result = -1;
    For(Batch->BatchCount)
    {
        if(CompareStringCompounds(Batch->Names+It, InsertCheck))
        {
            Result = It;
            break;
        }
    }
    if(Result < 0)
    {
        Batch->Names[Batch->BatchCount] = NewStringCompound(Arena, InsertCheck->Pos);
        AppendStringCompoundToCompound(Batch->Names+Batch->BatchCount, InsertCheck);
        Result = Batch->BatchCount++;
    }
    return Result;
}

inline void
InitializeSortBatch(arena_allocator *Arena, sort_batch *Batch, u32 BatchCount)
{
    Batch->Genre  = AllocateArray(Arena, BatchCount, array_batch_id);
    Batch->Artist = AllocateArray(Arena, BatchCount, array_batch_id);
    Batch->Album  = AllocateArray(Arena, BatchCount, array_batch_id);
    Batch->Song   = AllocateArray(Arena, BatchCount, array_file_id);
    Batch->Names  = AllocateArray(Arena, BatchCount, string_c);
    Batch->MaxBatches = BatchCount;
    Batch->BatchCount = 0;
}

internal void
CreateMusicSortingInfo()
{
    mp3_info *MP3Info = GlobalGameState.MP3Info;
    music_sorting_info *SortInfo = &MP3Info->MusicInfo->SortingInfo;
    mp3_file_info *FileInfo = &MP3Info->FileInfo;
    arena_allocator *FixArena = &GlobalGameState.FixArena;
    arena_allocator *ScratchArena = &GlobalGameState.ScratchArena;
    
    // TODO:: Push this from transient to fixed with correct size?
    sort_batch Genre  = {};
    InitializeSortBatch(FixArena, &Genre, 256);
    sort_batch Artist = {};
    InitializeSortBatch(FixArena, &Artist, 512);
    sort_batch Album  = {};
    InitializeSortBatch(FixArena, &Album, 1024);
    
    song_sort_info *SortBatchInfo = AllocateArray(FixArena, FileInfo->Count, song_sort_info);
    
    u32 *Genre_CountForBatches  = AllocateArray(ScratchArena, Genre.MaxBatches, u32);
    u32 *Artist_CountForBatches = AllocateArray(ScratchArena, Artist.MaxBatches, u32);
    u32 *Album_CountForBatches  = AllocateArray(ScratchArena, Album.MaxBatches, u32);
    
    // NOTE:: Filling the first slot with the _no_genre/artist/album_ 
    string_c Empty = {};
    AddSongToSortBatch(FixArena, &Genre, &Empty);
    AddSongToSortBatch(FixArena, &Artist, &Empty);
    AddSongToSortBatch(FixArena, &Album, &Empty);
    
    for(u32 FileID = 0; FileID < FileInfo->Count; FileID++)
    {
        mp3_metadata *MD = FileInfo->Metadata + FileID;
        
        // NOTE:: Fills the Batches with all different genres, artists, albums and gives the batch id back
        i32 GenreID  = AddSongToSortBatch(FixArena, &Genre, &MD->Genre);
        i32 ArtistID = AddSongToSortBatch(FixArena, &Artist, &MD->Artist);
        i32 AlbumID  = AddSongToSortBatch(FixArena, &Album, &MD->Album);
        Assert(GenreID >= 0 && ArtistID >= 0 && AlbumID >= 0);
        
        // NOTE:: For each song note the corresponding IDs for the genre, artist, album
        SortBatchInfo[FileID].GenreBatchID  = GenreID;
        SortBatchInfo[FileID].ArtistBatchID = ArtistID;
        SortBatchInfo[FileID].AlbumBatchID  = AlbumID;
        
        // NOTE:: Counts how many entries there are in each different genre, artist, album
        Genre_CountForBatches[GenreID]++;
        Artist_CountForBatches[ArtistID]++;
        Album_CountForBatches[AlbumID]++;
    }
    
    For(Genre.BatchCount)
    {
        Genre.Artist[It].A = CreateArray(FixArena, Genre_CountForBatches[It]);
        Genre.Album[It].A  = CreateArray(FixArena, Genre_CountForBatches[It]);
        Genre.Song[It].A   = CreateArray(FixArena, Genre_CountForBatches[It]);
    }
    For(Artist.BatchCount)
    {
        Artist.Album[It].A = CreateArray(FixArena, Artist_CountForBatches[It]);
        Artist.Song[It].A  = CreateArray(FixArena, Artist_CountForBatches[It]);
    }
    For(Album.BatchCount)
    {
        Album.Song[It].A = CreateArray(FixArena, Album_CountForBatches[It]);
        if(It == 0) Album.Genre[It].A = CreateArray(FixArena, 100);
        else Album.Genre[It].A = CreateArray(FixArena, 10);
    }
    
    FreeMemory(ScratchArena, Genre_CountForBatches);
    FreeMemory(ScratchArena, Artist_CountForBatches);
    FreeMemory(ScratchArena, Album_CountForBatches);
    
    
    for(u32 FileID = 0; FileID < FileInfo->Count; FileID++)
    {
        u32 GenreBatchID  = SortBatchInfo[FileID].GenreBatchID;
        u32 ArtistBatchID = SortBatchInfo[FileID].ArtistBatchID;
        u32 AlbumBatchID  = SortBatchInfo[FileID].AlbumBatchID;
        
        PushIfNotExist(&Genre.Artist[GenreBatchID].A, ArtistBatchID);
        PushIfNotExist(&Genre.Album[GenreBatchID].A, AlbumBatchID);
        PushIfNotExist(&Genre.Song[GenreBatchID].A, FileID);
        
        PushIfNotExist(&Artist.Album[ArtistBatchID].A, AlbumBatchID);
        PushIfNotExist(&Artist.Song[ArtistBatchID].A, FileID);
        
        PushIfNotExist(&Album.Genre[AlbumBatchID].A, GenreBatchID);
        PushIfNotExist(&Album.Song[AlbumBatchID].A, FileID);
    }
    
    music_info *MusicInfo = MP3Info->MusicInfo;
    MusicInfo->SortingInfo.Genre.Selected.A  = CreateArray(FixArena, Genre.BatchCount);
    MusicInfo->SortingInfo.Artist.Selected.A = CreateArray(FixArena, Artist.BatchCount);
    MusicInfo->SortingInfo.Album.Selected.A  = CreateArray(FixArena, Album.BatchCount);
    MusicInfo->SortingInfo.Song.Selected.A   = CreateArray(FixArena, FileInfo->Count);
    
    MusicInfo->SortingInfo.Genre.Displayable.A  = CreateArray(FixArena, Genre.BatchCount);
    For(Genre.BatchCount)
    {
        Push(&MusicInfo->SortingInfo.Genre.Displayable, NewFileID(It));
    }
    MusicInfo->SortingInfo.Artist.Displayable.A = CreateArray(FixArena, Artist.BatchCount);
    MusicInfo->SortingInfo.Album.Displayable.A  = CreateArray(FixArena, Album.BatchCount);
    MusicInfo->SortingInfo.Song.Displayable.A   = CreateArray(FixArena, MP3Info->FileInfo.Count);
    
    MusicInfo->SortingInfo.Genre.Base = &MusicInfo->SortingInfo;
    MusicInfo->SortingInfo.Artist.Base = &MusicInfo->SortingInfo;
    MusicInfo->SortingInfo.Album.Base = &MusicInfo->SortingInfo;
    MusicInfo->SortingInfo.Song.Base = &MusicInfo->SortingInfo;
    MusicInfo->SortingInfo.Genre.Batch  = Genre;
    MusicInfo->SortingInfo.Artist.Batch = Artist;
    MusicInfo->SortingInfo.Album.Batch  = Album;
    DebugLog(255, "GenreCount: %i, ArtistCount: %i, AlbumCount: %i\n", Genre.BatchCount, Artist.BatchCount, Album.BatchCount);
}

inline void
SetSelectionArray(music_display_column *DisplayColumn, column_sorting_info *SortingColumn, u32 ColumnDisplayID)
{
    if(!GlobalGameState.Input.Pressed[KEY_CONTROL_LEFT] && !GlobalGameState.Input.Pressed[KEY_CONTROL_RIGHT])
    {
        if(!IsSelected(DisplayColumn, SortingColumn, ColumnDisplayID) || SortingColumn->Selected.A.Count > 1)
        {
            ClearSelection(SortingColumn);
        }
    }
    ToggleSelection(DisplayColumn, SortingColumn, ColumnDisplayID);
}

internal column_type // returns none if not in any rect, and corresponding column if it is
UpdateSelectionArray(column_sorting_info *SortingColumn, music_display_column *DisplayColumn)
{
    column_type Result = columnType_None;
    DisplayColumn->LastClickSlotID = -1;
    for(u32 It = 0; 
        It < SortingColumn->Displayable.A.Count && It < DisplayColumn->Count;
        It++)
    {
        if(IsInRect(DisplayColumn->BGRects[It], GlobalGameState.Input.MouseP))
        {
            SetSelectionArray(DisplayColumn, SortingColumn, It);
            Result = DisplayColumn->Type;
            DisplayColumn->LastClickSlotID = It;
            break;
        }
    }
    return Result;
}

internal column_type
SelectAllOrNothing(music_display_column *DisplayColumn, column_sorting_info *SortingColumn)
{
    column_type Result = columnType_None;
    b32 SelectAll = false;
    for(u32 It = 0; 
        It < SortingColumn->Displayable.A.Count && It < DisplayColumn->Count;
        It++)
    {
        if(IsInRect(DisplayColumn->BGRects[It], GlobalGameState.Input.MouseP))
        {
            if((i32)It == DisplayColumn->LastClickSlotID) // Clicked on same slot == is a proper double click
            {
                SelectAll = IsSelected(DisplayColumn, SortingColumn, It);
                
                Reset(&SortingColumn->Selected);
                For(SortingColumn->Displayable.A.Count, Select)
                {
                    SetSelection(DisplayColumn, SortingColumn, NewDisplayableID(SelectIt), SelectAll);
                }
            }
            else // Not a proper double click, do same as UpdateSelectionArray
            {
                SetSelectionArray(DisplayColumn, SortingColumn, It);
            }
            
            Result = DisplayColumn->Type;
            DisplayColumn->LastClickSlotID = It;
            break;
        }
    }
    
    return Result;
}

internal void
PropagateSelectionChange(music_sorting_info *SortingInfo)
{
    
    array_batch_id *Genre  = &SortingInfo->Genre.Selected;
    array_batch_id *Artist = &SortingInfo->Artist.Selected;
    array_batch_id *Album  = &SortingInfo->Album.Selected;
    
    if(Genre->A.Count > 0)
    {
        For(Artist->A.Count)
        {
            b32 Found = false;
            for(u32 GenreID = 0; GenreID < Genre->A.Count; GenreID++)
            {
                array_file_id *GenreArtists = SortingInfo->Genre.Batch.Artist+Get(Genre, NewSelectID(GenreID)).ID;
                if(StackContains(GenreArtists, Get(Artist, NewSelectID(It))))
                {
                    Found = true;
                    break;
                }
            }
            if(!Found)
            {
                Take(Artist, NewSelectID(It));
                It--;
            }
        }
        
    }
    
    if(Artist->A.Count > 0 || Genre->A.Count > 0)
    {
        For(Album->A.Count)
        {
            b32 FoundInArtist = false;
            b32 FoundInGenre  = false;
            
            if(Artist->A.Count > 0)
            {
                for(u32 ArtistID = 0; ArtistID < Artist->A.Count; ArtistID++)
                {
                    array_file_id *ArtistAlbums = SortingInfo->Artist.Batch.Album + Get(Artist, NewSelectID(ArtistID)).ID;
                    if(StackContains(ArtistAlbums, Get(Album, NewSelectID(It))))
                    {
                        FoundInArtist = true;
                        break;
                    }
                }
            }
            else FoundInArtist = true;
            if(Genre->A.Count > 0)
            {
                for(u32 GenreID = 0; GenreID < Genre->A.Count; GenreID++)
                {
                    array_file_id *GenreAlbums = SortingInfo->Genre.Batch.Album+ Get(Genre, NewSelectID(GenreID)).ID;
                    if(StackContains(GenreAlbums, Get(Album, NewSelectID(It))))
                    {
                        FoundInGenre = true;
                        break;
                    }
                }
            }
            else FoundInGenre = true;
            if(FoundInArtist && FoundInGenre) ;
            else
            {
                Take(Album, NewSelectID(It));
                It--;
            }
        }
    }
}

internal void
MergeDisplayArrays(array_file_id *A1, array_file_id *A2, u32 CheckValue)
{
    For(A2->A.Count)
    {
        u32 ID = A2->A.Slot[It];
        Assert(A1->A.Length > ID);
        if(A1->A.Slot[ID] == CheckValue) 
        {
            Put(&A1->A, ID, ID);
            A1->A.Count++;
        }
    }
}

inline void
PutAndCheck(array_file_id *A1, file_id ID, u32 CheckValue)
{
    Assert((i32)A1->A.Length > ID.ID);
    if(A1->A.Slot[ID.ID] == CheckValue) 
    {
        Put(&A1->A, ID.ID, ID.ID);
        A1->A.Count++;
    }
}

internal void
RemoveCheckValueFromArray(array_u32 Array, u32 CheckValue)
{
    u32 BackIt = Array.Length-1;
    For(Array.Count)
    {
        if(Array.Slot[It] != CheckValue) continue;
        while(Array.Slot[BackIt] == CheckValue) BackIt--;
        if(BackIt < Array.Count) break;
        
        Array.Slot[It] = Array.Slot[BackIt--];
    }
}

internal void
FillDisplayables(music_sorting_info *SortingInfo, mp3_file_info *MP3FileInfo, music_display_info *DisplayInfo)
{
    // NOTE:: This may work now... I actually only do it for selected artist properly.
    // It should (most likely) be done with selected album as well! 
    b32 DoArtist = !DisplayInfo->Artist.Search.TextField.IsActive;
    b32 DoAlbum  = !DisplayInfo->Album.Search.TextField.IsActive;
    b32 DoSong   = !DisplayInfo->Song.Base.Search.TextField.IsActive;
    
    // For performance I use the displayable lists not as stacks in this procedure.
    // I fill them with a value they can never be, put the values to insert at the 
    // array position it itself points to (so duplicates are just written twice at 
    // the same location) and in the end remove the 'gaps' to re-stack-afy them.
    u32 CheckValue = MP3FileInfo->Count+1;
    if(DoArtist) Clear(&SortingInfo->Artist.Displayable, CheckValue);
    if(DoAlbum)  Clear(&SortingInfo->Album.Displayable, CheckValue);
    if(DoSong)   Clear(&SortingInfo->Song.Displayable, CheckValue);
    
    For(SortingInfo->Genre.Selected.A.Count) // Going through Genres
    {
        batch_id BatchID = Get(&SortingInfo->Genre.Selected, NewSelectID(It));
        sort_batch *GenreBatch = &SortingInfo->Genre.Batch;
        
        if(DoArtist) MergeDisplayArrays(&SortingInfo->Artist.Displayable, GenreBatch->Artist+BatchID.ID, CheckValue);
        if(SortingInfo->Artist.Selected.A.Count == 0)
        {
            if(DoAlbum) MergeDisplayArrays(&SortingInfo->Album.Displayable, GenreBatch->Album+BatchID.ID, CheckValue);
            if(SortingInfo->Album.Selected.A.Count == 0)
            {
                if(DoSong) MergeDisplayArrays(&SortingInfo->Song.Displayable, GenreBatch->Song+BatchID.ID, CheckValue);
            }
        }
    }
    
    For(SortingInfo->Artist.Selected.A.Count) // Going through Artists
    {
        batch_id BatchID = Get(&SortingInfo->Artist.Selected, NewSelectID(It));
        sort_batch *ArtistBatch = &SortingInfo->Artist.Batch;
        
        // For every album in batch, look if it is one of the selected genres. if not, skip.
        For(ArtistBatch->Album[BatchID.ID].A.Count, Album)
        {
            sort_batch *AlbumBatch = &SortingInfo->Album.Batch;
            select_id AlbumSelectID = NewSelectID(AlbumIt);
            batch_id AlbumBatchID = Get(ArtistBatch->Album+BatchID.ID, AlbumSelectID);
            For(AlbumBatch->Genre[AlbumBatchID.ID].A.Count, Genre)
            {
                if(StackContains(&SortingInfo->Genre.Selected, Get(AlbumBatch->Genre+AlbumBatchID.ID, NewSelectID(GenreIt))) ||
                   SortingInfo->Genre.Selected.A.Count == 0)
                {
                    if(DoAlbum) PutAndCheck(&SortingInfo->Album.Displayable, 
                                            Get(ArtistBatch->Album+BatchID.ID, AlbumSelectID), CheckValue);
                    // If no album is selected, fill song list from the verified album songs and
                    // check that _only_ songs which are also corresponding to the artist are inserted.
                    if(SortingInfo->Album.Selected.A.Count == 0)
                    {
                        For(AlbumBatch->Song[AlbumBatchID.ID].A.Count, Song)
                        {
                            file_id FileID = Get(AlbumBatch->Song+AlbumBatchID.ID, NewSelectID(SongIt));
                            if(StackContains(ArtistBatch->Song+BatchID.ID, FileID))
                            {
                                if(DoSong) PutAndCheck(&SortingInfo->Song.Displayable, FileID, CheckValue);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    if(DoSong) 
    {
        For(SortingInfo->Album.Selected.A.Count) // Going through Albums
        {
            batch_id BatchID = Get(&SortingInfo->Album.Selected, NewSelectID(It));
            
            MergeDisplayArrays(&SortingInfo->Song.Displayable, SortingInfo->Album.Batch.Song+BatchID.ID, CheckValue);
        }
    }
    
    timer Timer = StartTimer();
#if 0
    if(SortingInfo->Artist.Displayable.A.Count > 0) QuickSort3(SortingInfo->Artist.Displayable.A, true);
    if(SortingInfo->Album.Displayable.A.Count > 0) QuickSort3(SortingInfo->Album.Displayable.A, true);
    if(SortingInfo->Song.Displayable.A.Count > 0) QuickSort3(SortingInfo->Song.Displayable.A, true);
#else
    if(SortingInfo->Artist.Displayable.A.Count > 0) RemoveCheckValueFromArray(SortingInfo->Artist.Displayable.A, CheckValue);
    if(SortingInfo->Album.Displayable.A.Count > 0) RemoveCheckValueFromArray(SortingInfo->Album.Displayable.A, CheckValue);
    if(SortingInfo->Song.Displayable.A.Count > 0) RemoveCheckValueFromArray(SortingInfo->Song.Displayable.A, CheckValue);
#endif
    SnapTimer(&Timer);
    
    if(SortingInfo->Genre.Selected.A.Count == 0)
    {
        if(DoArtist) 
        {
            For(SortingInfo->Artist.Batch.BatchCount)
            {
                Push(&SortingInfo->Artist.Displayable, NewDisplayableID(It));
            }
        }
        if(SortingInfo->Artist.Selected.A.Count == 0)
        {
            if(DoAlbum) 
            {
                For(SortingInfo->Album.Batch.BatchCount)
                {
                    Push(&SortingInfo->Album.Displayable, NewDisplayableID(It));
                }
            }
            if(SortingInfo->Album.Selected.A.Count == 0)
            {
                if(DoSong) 
                {
                    For(MP3FileInfo->Count)
                    {
                        Push(&SortingInfo->Song.Displayable, NewDisplayableID(It));
                    }
                }
            }
        }
    }
}

internal void
UpdateSelectionChanged(renderer *Renderer, music_info *MusicInfo, mp3_info *MP3Info, column_type Type)
{
    music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
    music_sorting_info *SortingInfo = &MusicInfo->SortingInfo;
    
    FillDisplayables(SortingInfo, &MP3Info->FileInfo, &MusicInfo->DisplayInfo);
    SortDisplayables(SortingInfo, &MP3Info->FileInfo);
    UseDisplayableAsPlaylist(MusicInfo);
    
    if(MusicInfo->IsShuffled) ShuffleStack(&MusicInfo->SortingInfo.Song.Displayable);
    DisplayInfo->Song.Base.DisplayCursor = 0;
    
    displayable_id ArtistDisplayID = NewDisplayableID(0);
    r32 SelectArtistStart = 0;
    displayable_id AlbumDisplayID = NewDisplayableID(0);
    r32 SelectAlbumStart  = 0;
    switch(Type) // nothing to do for columnType_Genre
    {
        case columnType_Album:
        {
            AlbumDisplayID    = DisplayInfo->Album.OnScreenIDs[0];
            SelectAlbumStart  = GetLocalPosition(DisplayInfo->Album.BGRects[0]).y;
        } // Through
        case columnType_Artist:
        {
            ArtistDisplayID   = DisplayInfo->Artist.OnScreenIDs[0];
            SelectArtistStart = GetLocalPosition(DisplayInfo->Artist.BGRects[0]).y; 
        } break;
    }
    
    MoveDisplayColumn(Renderer, &DisplayInfo->Artist, ArtistDisplayID, SelectArtistStart);
    MoveDisplayColumn(Renderer, &DisplayInfo->Album, AlbumDisplayID, SelectAlbumStart);
    MoveDisplayColumn(Renderer, &DisplayInfo->Song.Base);
    
    UpdateSelectionColors(MusicInfo);
    UpdateVerticalSliders(Renderer, DisplayInfo, SortingInfo);
}

internal void
SearchInDisplayable(column_sorting_info *ColumnSortInfo, search_bar *Search, mp3_file_info *FileInfo)
{
    Reset(&ColumnSortInfo->Displayable);
    
    For(Search->InitialDisplayables.A.Count)
    {
        file_id FileID = Get(&Search->InitialDisplayables, NewDisplayableID(It));
        string_c *Name = 0;
        if(FileInfo) 
        {
            Name = &FileInfo->Metadata[FileID.ID].Title;
            if(Name->Pos == 0) Name = FileInfo->FileName+FileID.ID;
        }
        else Name = ColumnSortInfo->Batch.Names+FileID.ID;
        
        if(ContainsAB_CaseInsensitive(Name, &Search->TextField.TextString))
        {
            Push(&ColumnSortInfo->Displayable, FileID);
        }
    }
}

internal i32 // 1 = A Higher, 0 = A == B, -1 = B Higher
CompareAB(string_c *A, string_c *B)
{
    i32 Result = 0;
    
    for(u32 It = 0; It < A->Pos && It < B->Pos; It++)
    {
        u8 A1 = A->S[It];
        u8 B1 = B->S[It];
        
        if(A1 >= 'a' && A1 <= 'z') A1 -= 32;
        if(B1 >= 'a' && B1 <= 'z') B1 -= 32;
        
        if(A1 < B1)
        {
            Result = 1;
            break;
        }
        else if(A1 > B1)
        {
            Result = -1;
            break;
        }
    }
    if(A->Pos == 0) Result = 1;
    if(B->Pos == 0) Result = -1;
    
    return Result;
}

internal b32
IsHigherInAlphabet(i32 T1, i32 T2, void *Data)
{
    b32 Result = false;
    
    column_sorting_info *SortingColumn = (column_sorting_info *)Data;
    string_c *A = SortingColumn->Batch.Names+Get(&SortingColumn->Displayable, NewDisplayableID(T1)).ID;
    string_c *B = SortingColumn->Batch.Names+Get(&SortingColumn->Displayable, NewDisplayableID(T2)).ID;
    
    if(B->Pos > 0) 
    {
        i32 CompResult = CompareAB(A, B);
        Result = (CompResult >= 0);
    }
    
    return Result;
}

struct sort_song_column_info
{
    mp3_file_info *FileInfo;
    array_u32 *SongDisplayable;
};
internal b32
CompareSongDisplayable(i32 T1, i32 T2, void *Data)
{
    b32 Result = false;
    
    sort_song_column_info *SortInfo = (sort_song_column_info *)Data;
    mp3_metadata *A = SortInfo->FileInfo->Metadata+Get(SortInfo->SongDisplayable, T1);
    mp3_metadata *B = SortInfo->FileInfo->Metadata+Get(SortInfo->SongDisplayable, T2);
    
    i32 CompResult = CompareAB(&A->Artist, &B->Artist);
    if(CompResult == 0)
    {
        CompResult = CompareAB(&A->Album, &B->Album);
        if(CompResult == 0)
        {
            Result = (A->Track <= B->Track);
        }
        else Result = (CompResult == 1);
    }
    else Result = (CompResult == 1);
    
    return Result;
}

internal void
SortDisplayables(music_sorting_info *SortingInfo, mp3_file_info *MP3FileInfo)
{
    column_sorting_info *Genre  = &SortingInfo->Genre;
    column_sorting_info *Artist = &SortingInfo->Artist;
    column_sorting_info *Album  = &SortingInfo->Album;
    column_sorting_info *Song   = &SortingInfo->Song;
    
    QuickSort(0, Genre->Displayable.A.Count-1, &Genre->Displayable, {IsHigherInAlphabet, Genre});
    QuickSort(0, Artist->Displayable.A.Count-1, &Artist->Displayable, {IsHigherInAlphabet, Artist});
    QuickSort(0, Album->Displayable.A.Count-1, &Album->Displayable, {IsHigherInAlphabet, Album});
    
    sort_song_column_info SortSongInfo;
    SortSongInfo.FileInfo = MP3FileInfo;
    SortSongInfo.SongDisplayable = &Song->Displayable.A;
    QuickSort(0, Song->Displayable.A.Count-1, &Song->Displayable, {CompareSongDisplayable, &SortSongInfo});
}

/*internal void
RetraceFilePath(arena_allocator *Arena, mp3_info *MP3Info, file_id FileID)
{
    mp3_file_info TmpFileInfo = CreateFileInfoStruct(&Arena->Transient, 1000);
    string_compound FilePath = NewStringCompound(&Arena->Transient, 255);
    ConcatStringCompounds(2, &FilePath, &MP3Info->FolderPath);
    string_compound SubPath = NewStringCompound(&Arena->Transient, 255);;
    ConcatStringCompounds(2, &SubPath, MP3Info->FileInfo.SubPath + FileID.ID);
    
    FindAllMP3FilesInFolder(&Arena->Transient, &Arena->Transient, &FilePath, &SubPath, &TmpFileInfo);
    CrawlFilesForMetadata(&Arena->Transient, &Arena->Transient, &TmpFileInfo, &MP3Info->FolderPath);
    
    mp3_metadata *OldMD = MP3Info->FileInfo.Metadata+FileID.ID;
    b32 Found = false;
    For(TmpFileInfo.Count)
    {
        mp3_metadata *FoundMD = TmpFileInfo.Metadata+It;
        
        if(!Found &&
           CompareStringCompounds(&OldMD->Title, &FoundMD->Title) &&
           CompareStringCompounds(&OldMD->Artist, &FoundMD->Artist) &&
           CompareStringCompounds(&OldMD->Album, &FoundMD->Album))
        {
            string_c *NewFileName = MP3Info->FileInfo.FileName+FileID.ID;
            string_c *NewSubPath = MP3Info->FileInfo.SubPath+FileID.ID;
            
            if(NewFileName->Length < TmpFileInfo.FileName[It].Pos)
            {
                *NewFileName = NewStringCompound(&Arena->Fixed, TmpFileInfo.FileName[It].Pos);
            }
            else ResetStringCompound(*NewFileName);
            AppendStringCompoundToCompound(NewFileName, TmpFileInfo.FileName+It);
            
            if(NewSubPath->Length < TmpFileInfo.SubPath[It].Pos)
            {
                *NewSubPath = NewStringCompound(&Arena->Fixed, TmpFileInfo.SubPath[It].Pos);
            }
            else ResetStringCompound(*NewSubPath);
            AppendStringCompoundToCompound(NewSubPath, TmpFileInfo.SubPath+It);
        }
        
        DeleteStringCompound(&Arena->Transient, TmpFileInfo.FileName+It);
        DeleteStringCompound(&Arena->Transient, TmpFileInfo.SubPath+It);
        DeleteStringCompound(&Arena->Transient, &FoundMD->Title);
        DeleteStringCompound(&Arena->Transient, &FoundMD->Artist);
        DeleteStringCompound(&Arena->Transient, &FoundMD->Album);
        DeleteStringCompound(&Arena->Transient, &FoundMD->Genre);
        DeleteStringCompound(&Arena->Transient, &FoundMD->TrackString);
        DeleteStringCompound(&Arena->Transient, &FoundMD->YearString);
    }
    DeleteStringCompound(&Arena->Transient, &FilePath);
    DeleteStringCompound(&Arena->Transient, &SubPath);
    DeleteFileInfoStruct(&Arena->Transient, &TmpFileInfo);
}
*/

internal void
FinishChangeSong(game_state *GameState, playing_song *Song)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    GameState->MusicInfo.CurrentlyChangingSong = false;
    
    file_id FileID = GetNextSong(&MusicInfo->Playlist, Song);
    
    mp3dec_file_info_t *DInfo = &GameState->MP3Info->DecodeInfo.DecodedData[Song->DecodeID];
    Assert(DInfo->layer == 3);
    
    PushSoundBufferClear(GameState->SoundThreadInterface);
    PushSongChanged(GameState->SoundThreadInterface, DInfo);
    
    column_sorting_info *SortingInfo = &MusicInfo->SortingInfo.Song;
    UpdatePlayingSongColor(&MusicInfo->DisplayInfo.Song.Base, SortingInfo, FileID, 
                           &MusicInfo->DisplayInfo.Song.Base.Base->ColorPalette.PlayingSong);
    
    
    mp3_metadata *MD = &GameState->MP3Info->FileInfo.Metadata[FileID.ID];
    DebugLog(1255, "Nr.%i: %s (%s) by %s \n%i - %s - %s - %i Hz\n", MD->Track, MD->Title.S, MD->Album.S, MD->Artist.S, MD->Year, MD->Genre.S, MD->DurationString.S, DInfo->hz);
    
    char WinText[512];
    sprintf_s(WinText, "%s (%s)\n", MD->Title.S, MD->Artist.S);
    SetWindowText(GameState->Renderer.Window.WindowHandle, WinText);
}

internal void
ChangeSong(game_state *GameState, playing_song *Song)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    
    file_id FileID = GetNextSong(&MusicInfo->Playlist, Song);
    
    if(FileID >= 0)
    {
        GameState->MusicInfo.CurrentlyChangingSong = true;
        Song->DecodeID = AddJob_LoadMP3(GameState, &GameState->JobQueue, FileID, 0, 1000000);
        
        Assert(Song->DecodeID >= 0);
        if(!GameState->MP3Info->DecodeInfo.CurrentlyDecoding[Song->DecodeID])
        {
            FinishChangeSong(GameState, Song);
        }
    }
    else 
    {
        // Song is not getting changed. No song to change to.
        GameState->MusicInfo.IsPlaying = false;
        PushNewPlayedTime(GameState->SoundThreadInterface, 0);
        PushSoundBufferClear(GameState->SoundThreadInterface);
    }
    
    SetTheNewPlayingSong(&GameState->Renderer, &GameState->MusicInfo.DisplayInfo.PlayingSongPanel, MusicInfo);
}

inline void
HandleChangeToNextSong(game_state *GameState)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    SetNextSong(&MusicInfo->Playlist, &MusicInfo->PlayingSong, MusicInfo->Looping);
    ChangeSong(GameState, &MusicInfo->PlayingSong);
    if(MusicInfo->PlayingSong.PlaylistID >= 0) KeepPlayingSongOnScreen(&GameState->Renderer, MusicInfo);
    AddJob_NextUndecodedInPlaylist();
}

internal void
ApplyNewMetadata(game_state *GameState, music_info *MusicInfo)
{
    MusicInfo->Playlist.Songs.A  = CreateArray(&GameState->FixArena, GameState->MP3Info->FileInfo.Count+200);
    MusicInfo->Playlist.UpNext.A = CreateArray(&GameState->FixArena, 200);
    
    MusicInfo->PlayingSong = {-1, -1, -1, 0};
    CreateMusicSortingInfo();
    FillDisplayables(&MusicInfo->SortingInfo, &GameState->MP3Info->FileInfo, &MusicInfo->DisplayInfo);
    SortDisplayables(&MusicInfo->SortingInfo, &GameState->MP3Info->FileInfo);
    UseDisplayableAsPlaylist(MusicInfo);
    UpdateAllDisplayColumns(GameState);
    SaveMP3LibraryFile(GameState, GameState->MP3Info);
    
    MusicInfo->DisplayInfo.Genre.Search.InitialDisplayables.A = CreateArray(&GameState->FixArena, 
                                                                            MusicInfo->SortingInfo.Genre.Displayable.A.Length);
    MusicInfo->DisplayInfo.Artist.Search.InitialDisplayables.A = CreateArray(&GameState->FixArena, 
                                                                             MusicInfo->SortingInfo.Artist.Displayable.A.Length);
    MusicInfo->DisplayInfo.Album.Search.InitialDisplayables.A = CreateArray(&GameState->FixArena, 
                                                                            MusicInfo->SortingInfo.Album.Displayable.A.Length);
    MusicInfo->DisplayInfo.Song.Base.Search.InitialDisplayables.A = CreateArray(&GameState->FixArena, 
                                                                                MusicInfo->SortingInfo.Song.Displayable.A.Length);
    
    MusicInfo->DisplayInfo.Genre.DisplayCursor = 0;
    MusicInfo->DisplayInfo.Artist.DisplayCursor = 0;
    MusicInfo->DisplayInfo.Album.DisplayCursor = 0;
    MusicInfo->DisplayInfo.Song.Base.DisplayCursor = 0;
    
    UpdateVerticalSliders(&GameState->Renderer, &MusicInfo->DisplayInfo, &MusicInfo->SortingInfo);
    UpdateHorizontalSliders(&GameState->Renderer, &MusicInfo->DisplayInfo, &MusicInfo->SortingInfo);
    
    AddJobs_LoadOnScreenMP3s(GameState, &GameState->JobQueue);
}

internal b32
CreateNewMetadata(game_state *GameState)
{
    b32 Result = true;
    music_info *MusicInfo = &GameState->MusicInfo;
    string_c SubPath = {};
    DeleteFileInfoStruct(&GameState->MP3Info->FileInfo);
    CreateFileInfoStruct(&GameState->MP3Info->FileInfo, MAX_MP3_INFO_COUNT);
    Result = FindAllMP3FilesInFolder(&GameState->ScratchArena, &GameState->MP3Info->FolderPath,
                                     &SubPath, &GameState->MP3Info->FileInfo);
    CrawlFilesForMetadata(&GameState->ScratchArena, &GameState->MP3Info->FileInfo, &GameState->MP3Info->FolderPath);
    
    ApplyNewMetadata(GameState, MusicInfo);
    return Result;
}













