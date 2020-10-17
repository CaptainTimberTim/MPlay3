#include "Sound_TD.h"
#include "GameBasics_TD.h"

inline void
AdvanceToNewline(u8 **String)
{
    while(*String[0] != '\n' && *String[0] != 0 ) (*String)++;
    (*String)++;
}

inline mp3_file_info
CreateFileInfoStruct(memory_bucket_container *Bucket, u32 FileInfoCount)
{
    mp3_file_info Result = {};
    Result.FileName = PushArrayOnBucket(Bucket, FileInfoCount, string_c);
    Result.SubPath  = PushArrayOnBucket(Bucket, FileInfoCount, string_c);
    Result.Metadata = PushArrayOnBucket(Bucket, FileInfoCount, mp3_metadata);
    Result.MaxCount = FileInfoCount;
    
    return Result;
}

inline void
DeleteFileInfoStruct(memory_bucket_container *Bucket, mp3_file_info *FileInfo)
{
    PopFromTransientBucket(Bucket, FileInfo->FileName);
    PopFromTransientBucket(Bucket, FileInfo->SubPath);
    PopFromTransientBucket(Bucket, FileInfo->Metadata);
}

inline void
ResizeFileInfo(memory_bucket_container *Bucket, mp3_file_info *FileInfo, u32 NewMaxCount)
{
    Assert(NewMaxCount >= 0);
    string_c *OldFileNames    = FileInfo->FileName;
    string_c *OldSubPaths     = FileInfo->SubPath;
    mp3_metadata *OldMetadata = FileInfo->Metadata;
    
    FileInfo->FileName = PushArrayOnBucket(Bucket, NewMaxCount, string_c);
    FileInfo->SubPath  = PushArrayOnBucket(Bucket, NewMaxCount, string_c);
    FileInfo->Metadata = PushArrayOnBucket(Bucket, NewMaxCount, mp3_metadata);
    
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
CreateMP3InfoStruct(memory_bucket_container *Bucket, u32 FileInfoCount)
{
    mp3_info *Result = PushStructOnBucket(Bucket, mp3_info);
    Result->FileInfo = CreateFileInfoStruct(Bucket, FileInfoCount);
    Result->DecodeInfo.FileID.A      = CreateArray(Bucket, MAX_MP3_DECODE_COUNT);
    Result->DecodeInfo.LastTouched   = CreateArray(Bucket, MAX_MP3_DECODE_COUNT);
    
#ifdef DECODE_STREAMING_TMP
    For(MAX_MP3_DECODE_COUNT)
    {
        Result->DecodeInfo.DecodedData[It].buffer = PushArrayOnBucket(Bucket, 48000*2*DECODE_PRELOAD_SECONDS, i16);
    }
    
    Result->DecodeInfo.PlayingDecoded.buffer = PushArrayOnBucket(Bucket, CURRENTLY_SUPPORTED_MAX_DECODED_FILE_SIZE/2, i16);
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

internal b32
FindAllMP3FilesInFolder(memory_bucket_container *FixedBucket, memory_bucket_container *TransientBucket,
                        string_compound *FolderPath, string_compound *SubPath,
                        mp3_file_info *ResultingFileInfo)
{
    b32 Result = false;
    string_compound FolderPathStar = NewStringCompound(TransientBucket, 255);
    ConcatStringCompounds(3, &FolderPathStar, FolderPath, SubPath);
    AppendCharToCompound(&FolderPathStar, '*');
    
    string_w WideFolderPath = {};
    ConvertString8To16(TransientBucket, &FolderPathStar, &WideFolderPath);
    
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
        string_compound FileType = NewStringCompound(TransientBucket, 16);
        
        while(HasNextFile && ResultingFileInfo->Count < ResultingFileInfo->MaxCount)
        {
            string_c FileName = {};
            ConvertString16To8(TransientBucket, FileData.cFileName, &FileName);
            
            if     (CompareStringAndCompound(&FileName, (u8 *)".") ) {}
            else if(CompareStringAndCompound(&FileName, (u8 *)"..")) {}
            else
            {
                if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    i32 PathLength = FileName.Pos+SubPath->Pos+1;
                    string_compound NewSubFolderPath = NewStringCompound(TransientBucket, PathLength);
                    ConcatStringCompounds(3, &NewSubFolderPath, SubPath, &FileName);
                    AppendCharToCompound(&NewSubFolderPath, '\\');
                    
                    FindAllMP3FilesInFolder(FixedBucket, TransientBucket, FolderPath, &NewSubFolderPath, ResultingFileInfo);
                    DeleteStringCompound(TransientBucket, &NewSubFolderPath);
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
                                NewStringCompound(FixedBucket, SubPath->Pos);
                            AppendStringCompoundToCompound(ResultingFileInfo->SubPath+ResultingFileInfo->Count, SubPath);
                            
                            ResultingFileInfo->FileName[ResultingFileInfo->Count] =
                                NewStringCompound(FixedBucket, FileName.Pos);
                            AppendStringCompoundToCompound(ResultingFileInfo->FileName+ResultingFileInfo->Count, &FileName);
                            
                            ResultingFileInfo->Count++;
                            Result = true;
                        }
                        ResetStringCompound(FileType);
                    }
                }
            }
            HasNextFile = FindNextFileW(FileHandle, &FileData);
            DeleteStringCompound(TransientBucket, &FileName);
        } 
        DeleteStringCompound(TransientBucket, &FileType);
    }
    
    //DebugLog(555, "Found %i songs in folder %s.\n", ResultingFileInfo->Count, FolderPathStar.S);
    
    DeleteStringW(TransientBucket, &WideFolderPath);
    DeleteStringCompound(TransientBucket, &FolderPathStar);
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
MetadataID3v1(memory_bucket_container *Bucket, read_file_result *File, mp3_metadata *Metadata)
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
                Metadata->Title  = NewStringCompound(Bucket, StringLength(Title));
                AppendStringToCompound(&Metadata->Title, Title);
                EatLeadingSpaces(&Metadata->Title);
                EatTrailingSpaces(&Metadata->Title);
                Metadata->FoundFlags |= metadata_Title;
            }
            if(~Metadata->FoundFlags & metadata_Artist &&
               StringLength(Artist) != 0)
            {
                Metadata->Artist = NewStringCompound(Bucket, StringLength(Artist));
                AppendStringToCompound(&Metadata->Artist, Artist);
                EatLeadingSpaces(&Metadata->Artist);
                EatTrailingSpaces(&Metadata->Artist);
                Metadata->FoundFlags |= metadata_Artist;
            }
            if(~Metadata->FoundFlags & metadata_Album &&
               StringLength(Album) != 0)
            {
                Metadata->Album  = NewStringCompound(Bucket, StringLength(Album));
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
                    Metadata->YearString = NewStringCompound(Bucket, 4);
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
                Metadata->TrackString = NewStringCompound(Bucket, 4);
                I32ToString(&Metadata->TrackString, (Track<<0));
                Metadata->Track = Track;
                Metadata->FoundFlags |= metadata_Track;
            }
        }
    }
}

internal i32
MetadataID3v2_Helper(memory_bucket_container *Bucket, u8 *C, u8 *Frame, string_compound *S, u32 MinorVersion)
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
            *S = NewStringCompound(Bucket, Length);
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
MetadataID3v2_3(memory_bucket_container *MainBucket, memory_bucket_container *TransientBucket, 
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
                JmpIter = MetadataID3v2_Helper(MainBucket, Current+It, TITLE, &Metadata->Title, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Title;
            }
            if(~Metadata->FoundFlags & metadata_Album)
            {
                JmpIter = MetadataID3v2_Helper(MainBucket, Current+It, ALBUM, &Metadata->Album, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Album;
            }
            if(~Metadata->FoundFlags & metadata_Artist)
            {
                JmpIter = MetadataID3v2_Helper(MainBucket, Current+It, ARTIST, &Metadata->Artist, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Artist;
            }
            if(~Metadata->FoundFlags & metadata_Genre)
            {
                string_compound GenreNr = {};
                JmpIter = MetadataID3v2_Helper(TransientBucket, Current+It, GENRE, &GenreNr, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Genre;
                if(Metadata->FoundFlags & metadata_Genre)
                {
                    if(GenreNr.S[0] == '(')
                    {
                        u8 L = 0;
                        u32 GNr = ProcessNextU32InString(GenreNr.S+1, ')', L);
                        Metadata->Genre = NewStringCompound(MainBucket, GenreTypes[GNr].Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreTypes[GNr]);
                    }
                    else if(IsStringCompANumber(&GenreNr))
                    {
                        u8 L = 0;
                        u32 GNr = ProcessNextU32InString(GenreNr.S, '\0', L);
                        Metadata->Genre = NewStringCompound(MainBucket, GenreTypes[GNr].Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreTypes[GNr]);
                    }
                    else 
                    {
                        Metadata->Genre = NewStringCompound(MainBucket, GenreNr.Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreNr);
                    }
                    
                    DeleteStringCompound(TransientBucket, &GenreNr);
                }
            }
            
            if(~Metadata->FoundFlags & metadata_Track)
            {
                string_compound Track = {};
                JmpIter = MetadataID3v2_Helper(TransientBucket, Current+It, TRACK, &Track, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Track;
                if(Metadata->FoundFlags & metadata_Track)
                {
                    i32 SlashPos = FindFirstOccurrenceOfCharInStringCompound(&Track, '/');
                    if(SlashPos != -1) Track.Pos = SlashPos;
                    Metadata->Track = ConvertU32FromString(Track.S, Track.Pos);
                    Assert(Metadata->Track < 10000);
                    Metadata->TrackString = NewStringCompound(MainBucket, 4);
                    I32ToString(&Metadata->TrackString, Metadata->Track);
                    
                    DeleteStringCompound(TransientBucket, &Track);
                }
            }
            if(~Metadata->FoundFlags & metadata_Year)
            {
                string_compound Year = {};
                JmpIter = MetadataID3v2_Helper(TransientBucket, Current+It, YEAR, &Year, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Year;
                if(Metadata->FoundFlags & metadata_Year)
                {
                    if(Year.Pos == 0) Metadata->Year = 0;
                    else Metadata->Year = ConvertU32FromString(Year.S, 4);
                    Assert(Metadata->Year < 10000);
                    Metadata->YearString = NewStringCompound(MainBucket, 4);
                    I32ToString(&Metadata->YearString, Metadata->Year);
                    DeleteStringCompound(TransientBucket, &Year);
                }
            }
            if(~Metadata->FoundFlags & metadata_Duration)
            {
                string_c Duration = {};
                JmpIter = MetadataID3v2_Helper(TransientBucket, Current+It, DURATION, &Duration, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Duration;
                if(Metadata->FoundFlags & metadata_Duration)
                {
                    if(Duration.Pos == 0) Metadata->Duration = 0;
                    else Metadata->Duration = ConvertU32FromString(Duration.S, Duration.Pos);
                    //DebugLog(20, "Duration: %i\n", Metadata->Duration);
                    DeleteStringCompound(TransientBucket, &Duration);
                }
            }
            if(FoundAllMetadata(Metadata->FoundFlags)) break;
        }
    }
    return DataLength;
}

inline u32
ExtractMetadata(memory_bucket_container *MainBucket, memory_bucket_container *TransientBucket, read_file_result *File, mp3_metadata *Metadata)
{
    u32 MetadataSize3 = 0;
    u32 MetadataSize2 = 0;
    if(!FoundAllMetadata(Metadata->FoundFlags)) MetadataSize3 = MetadataID3v2_3(MainBucket, TransientBucket, File, Metadata);
    //if(!FoundAllMetadata(Metadata->FoundFlags)) MetadataID3v1(MainBucket, File, Metadata);
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
        if(GlobalGameState.MusicInfo.PlayingSong.PlaylistID >= 0 && GlobalGameState.MusicInfo.PlayingSong.DecodeID == Result)
        {
            if(Get(&DecodeInfo->FileID, NewDecodeID(Result)) == PlaylistIDToFileID(&GlobalGameState.MusicInfo.Playlist,
                                                                                   GlobalGameState.MusicInfo.PlayingSong.PlaylistID))
                DebugLog(255, "ERROR:: STAGE 1: Should never be the same here!\n");
            Put(&DecodeInfo->LastTouched, Result, DecodeInfo->TouchCount++);
            Result = GetSmallestEntryID(&DecodeInfo->LastTouched, DecodeInfo->Count);
            
        }
    }
    Assert(Result >= 0);
    return Result;
}

inline void
CreateSongDurationForMetadata(bucket_allocator *Bucket, mp3_info *MP3Info, file_id FileID, i32 DecodeID)
{
    if(~MP3Info->FileInfo.Metadata[FileID.ID].FoundFlags & metadata_Duration)
    {
        mp3_metadata *MD = &MP3Info->FileInfo.Metadata[FileID.ID];
        mp3dec_file_info_t *DInfo = &MP3Info->DecodeInfo.DecodedData[DecodeID];
        
        MD->Duration = (u32)DInfo->samples/DInfo->channels/DInfo->hz*1000;
        MillisecondsToMinutes(&Bucket->Fixed, MD->Duration, &MD->DurationString);
        MD->FoundFlags |= metadata_Duration;
    }
}

internal error_item
LoadAndDecodeMP3Data(bucket_allocator *Bucket, mp3_info *MP3Info, file_id FileID, 
                     i32 DecodeID, b32 DoMetadataExtraction = true)
{
    error_item Result = {(load_error_codes)DecodeID, FileID};
    string_compound FilePath = NewStringCompound(&Bucket->Transient, 255);
    ConcatStringCompounds(4, &FilePath, &MP3Info->FolderPath, 
                          MP3Info->FileInfo.SubPath + FileID.ID, 
                          MP3Info->FileInfo.FileName + FileID.ID);
    
    read_file_result FileData = {};
    if(ReadEntireFile(&Bucket->Transient, &FileData, FilePath.S))
    {
        if(FileData.Size != 0)
        {
            if(DoMetadataExtraction)
            {
                Assert(false); // Is this actually used? Does not seem like it!
                ExtractMetadata(&Bucket->Fixed, &Bucket->Transient, &FileData, &MP3Info->FileInfo.Metadata[FileID.ID]);
            }
            
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
                CreateSongDurationForMetadata(Bucket, MP3Info, FileID, DecodeID);
            }
        }
        else 
        {
            Result.Code = loadErrorCode_EmptyFile;
        }
        FreeFileMemory(&Bucket->Transient, FileData.Data);
    }
    else 
    {
        OutputDebugStringA("ERROR:: Failed to load file.\n");
        Result.Code = loadErrorCode_FileLoadFailed;
    }
    DeleteStringCompound(&Bucket->Transient, &FilePath);
    
    if(DecodeID >= 0) MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = false;
    return Result;
}

internal error_item
LoadAndDecodeMP3Data(bucket_allocator *Bucket, mp3_info *MP3Info, file_id FileID, b32 DoMetadataExtraction = true)
{
    i32 DecodeID = GetNextDecodeIDToEvict(&MP3Info->DecodeInfo);
    return LoadAndDecodeMP3Data(Bucket, MP3Info, FileID, DecodeID, DoMetadataExtraction);
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
LoadAndDecodeMP3StartFrames(bucket_allocator *Bucket, i32 SecondsToDecode, file_id FileID, 
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
    u32 MetadataSize    = ExtractMetadataSize(&Bucket->Transient, &FilePath);
    u32 ReadAmount      = MetadataSize + MINIMP3_MAX_SAMPLES_PER_FRAME + 512;
    i16 *TmpInfoStorage = PushArrayOnBucket(&Bucket->Transient, ReadAmount, i16);
    
    read_file_result File = {};
    if(ReadBeginningOfFile(&Bucket->Transient, &File, FilePath.S, ReadAmount))
    {
        // 3. 
        *DecodeResult = DecodeMP3StartFrames(&MP3Decoder, File, TmpInfoStorage, 1);
        FreeFileMemory(&Bucket->Transient, File.Data);
        
        u32 DecodeSampleAmount = DecodeResult->hz*SecondsToDecode; 
        // samples has channels already in it. But I need the frame amount which is channel indipendent.
        u32 DecodeFrameAmount  = DecodeSampleAmount/(u32)(DecodeResult->samples/DecodeResult->channels);
        // 4. This should always be enough, as the mp3 file should be smaller, as it is compressed...
        ReadAmount = DecodeSampleAmount*DecodeResult->channels*sizeof(i16) + DecodeFrameAmount*sizeof(u32) + MetadataSize; 
        if(ReadBeginningOfFile(&Bucket->Transient, &File, FilePath.S, ReadAmount))
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
                
                CreateSongDurationForMetadata(Bucket, MP3Info, FileID, DecodeID);
            }
            
            FreeFileMemory(&Bucket->Transient, File.Data);
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
    
    PopFromTransientBucket(&Bucket->Transient, TmpInfoStorage);
    
    if(DecodeID >= 0) MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = false;
    return Result;
}

internal u32 
ExtractMetadataSize(memory_bucket_container *TransientBucket, string_c *CompletePath)
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
    if(ReadBeginningOfFile(TransientBucket, &FileData, CompletePath->S, 10)) 
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
        FreeFileMemory(TransientBucket, FileData.Data);
    }
    
    return Result;
}

internal void
CrawlFileForMetadata(memory_bucket_container *MainBucket, memory_bucket_container *TransientBucket, 
                     mp3_metadata *MD, string_c *FilePath)
{
    u32 DataLength = ExtractMetadataSize(TransientBucket, FilePath);
    read_file_result FileData = {};
    if(DataLength > 0 && ReadBeginningOfFile(TransientBucket, &FileData, FilePath->S, DataLength)) 
    {
        ExtractMetadata(MainBucket, TransientBucket, &FileData, MD);
        FreeFileMemory(TransientBucket, FileData.Data);
    }
    
    if(!FoundAllMetadata(MD->FoundFlags))
    {
        if(ReadEndOfFile(TransientBucket, &FileData, FilePath->S, 128))
        {
            MetadataID3v1(MainBucket, &FileData, MD);
            FreeFileMemory(TransientBucket, FileData.Data);
        }
    }
}

internal void
CrawlFilesForMetadata(memory_bucket_container *MainBucket, memory_bucket_container *TransientBucket, 
                      mp3_file_info *FileInfo, string_c *FolderPath, u32 *CurrentCrawlCount = 0)
{
    OutputDebugStringA("\n");
    string_compound FilePath = NewStringCompound(TransientBucket, 255);
    
    For(FileInfo->Count)
    {
        if(FoundAllMetadata(FileInfo->Metadata[It].FoundFlags)) continue;
        ConcatStringCompounds(4, &FilePath, FolderPath, FileInfo->SubPath+It, FileInfo->FileName+It);
        
        CrawlFileForMetadata(MainBucket, TransientBucket, FileInfo->Metadata+It, &FilePath);
        
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
    DeleteStringCompound(TransientBucket, &FilePath);
}

internal void
RemoveFileFromInfo(mp3_file_info *FileInfo, u32 RemoveID)
{
    Assert(FileInfo->Count > 0 && RemoveID >= 0);
    Assert(RemoveID < FileInfo->Count);
    RemoveItem(FileInfo->FileName, FileInfo->Count, RemoveID, string_c);
    RemoveItem(FileInfo->SubPath, FileInfo->Count, RemoveID, string_c);
    RemoveItem(FileInfo->Metadata, FileInfo->Count, RemoveID, mp3_metadata);
    FileInfo->Count--;
}

internal b32
AddFileToInfo(bucket_allocator *Bucket, mp3_file_info *FileInfo, string_c *SubPath, string_c *FileName)
{
    b32 Result = false;
    if(FileInfo->Count < FileInfo->MaxCount)
    {
        Result = true;
        FileInfo->SubPath[FileInfo->Count] = NewStringCompound(&GlobalGameState.Bucket.Fixed, SubPath->Pos);
        AppendStringCompoundToCompound(FileInfo->SubPath+FileInfo->Count, SubPath);
        FileInfo->FileName[FileInfo->Count] = NewStringCompound(&GlobalGameState.Bucket.Fixed, FileName->Pos);
        AppendStringCompoundToCompound(FileInfo->FileName+FileInfo->Count, FileName);
        
        string_c FilePath = NewStringCompound(&Bucket->Transient, 255);
        ConcatStringCompounds(4, &FilePath, &GlobalGameState.MP3Info->FolderPath, FileInfo->SubPath+FileInfo->Count, 
                              FileInfo->FileName+FileInfo->Count);
        CrawlFileForMetadata(&Bucket->Fixed, &Bucket->Transient, FileInfo->Metadata+FileInfo->Count, &FilePath);
        DeleteStringCompound(&Bucket->Transient, &FilePath);
        FileInfo->Count++;
    }
    return Result;
}

internal void
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

internal void
MillisecondsToMinutes(memory_bucket_container *Bucket, u32 Millis, string_c *Out)
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
    
    *Out = NewStringCompound(Bucket, StringLength((u8 *)B)+2);
    AppendStringToCompound(Out, (u8 *)B);
    AppendStringToCompound(Out, (u8 *)B2);
}

// NOTE:: Settings file specifications
//
// MPlay3Settings
// Version 4
// 
// Volume: <Between 0 and 1>
// LastPlayingSong: <FileName>
// ColorPalette: <ID>
// GenreArtistEdgeXPercent: <Between 0 and 1>
// ArtistAlbumEdgeXPercent: <Between 0 and 1>
// AlbumSongEdgeXPercent: <Between 0 and 1>
// WindowDimX: <Between GlobalMinWindowWidth and MAX_I32>
// WindowDimY: <Between GlobalMinWindowHeight and MAX_I32>
// Looping: <0/1>
// Shuffle: <0/1>
// Palette: <Name>  // Palette with all following color values can occur multiple times
// Text: <R255> <G255> <B255>
// ForegroundText: <R255> <G255> <B255>
// ErrorText: <R255> <G255> <B255>
// Foreground: <R255> <G255> <B255>
// Slot: <R255> <G255> <B255>
// SliderBackground: <R255> <G255> <B255>
// SliderGrabThing: <R255> <G255> <B255>
// ButtonActive: <R255> <G255> <B255>
// Selected: <R255> <G255> <B255>
// PlayingSong: <R255> <G255> <B255>

inline void
ProcessNextPaletteColor(u8 **C, u8 *ColorName, v3 *Color)
{
    u8 Length = 0;
    *C += StringLength(ColorName);
    Color->r = (r32)ProcessNextI32InString(*C, ' ', Length);
    *C += Length+1;
    Color->g = (r32)ProcessNextI32InString(*C, ' ', Length);
    *C += Length+1;
    Color->b = (r32)ProcessNextI32InString(*C, '\n', Length);
    
    AdvanceToNewline(C);
}

internal settings
TryLoadSettingsFile(game_state *GameState)
{
    settings Result = {};
    
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+SETTINGS_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, SETTINGS_FILE_NAME);
    if(ReadEntireFile(&GameState->Bucket.Transient, &Data, FilePath.S))
    {
        u8 *C = Data.Data;
        
        if(StringCompare(C, (u8 *)"MPlay3Settings", 0, StringLength((u8 *)"MPlay3Settings")))
        {
            AdvanceToNewline(&C);
            
            u8 *VersionString = (u8 *)"Version ";
            u32 VersionLength = StringLength(VersionString);
            
            if(StringCompare(C, VersionString, 0, VersionLength) &&
               CharToU32(C[VersionLength]) == SETTINGS_CURRENT_VERSION)
            {
                AdvanceToNewline(&C);
                AdvanceToNewline(&C);
                u8 Length;
                
                C += StringLength((u8 *)"Volume: ");
                Result.Volume = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"LastPlayingSong: ");
                Result.PlayingSongID.ID = ProcessNextI32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"ColorPalette: ");
                Result.ColorPaletteID = ProcessNextU32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"GenreArtistEdgeXPercent: ");
                Result.GenreArtistEdgeXPercent = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"ArtistAlbumEdgeXPercent: ");
                Result.ArtistAlbumEdgeXPercent = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"AlbumSongEdgeXPercent: ");
                Result.AlbumSongEdgeXPercent = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"WindowDimensionX: ");
                Result.WindowDimX = ProcessNextU32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"WindowDimensionY: ");
                Result.WindowDimY = ProcessNextU32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"Looping: ");
                Result.Looping = ProcessNextB32InString(C);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"Shuffle: ");
                Result.Shuffle = ProcessNextB32InString(C);
                AdvanceToNewline(&C);
                
                // Prescan for color palette count
                u8 *PreC = C;
                Result.PaletteCount = 0;
                while(StringCompare(PreC, (u8 *)"Palette: ", 0, 9))
                {
                    For(11) AdvanceToNewline(&PreC);
                    Result.PaletteCount++;
                }
                Result.PaletteMaxCount = Result.PaletteCount+10;
                Result.PaletteNames  = PushArrayOnBucket(&GameState->Bucket.Fixed, Result.PaletteMaxCount, string_c);
                Result.Palettes      = PushArrayOnBucket(&GameState->Bucket.Fixed, Result.PaletteMaxCount, color_palette);
                
                string_c PaletteName = NewStringCompound(&GameState->Bucket.Transient, 100);
                For(Result.PaletteCount)
                {
                    C += StringLength((u8 *)"Palette: ");
                    CopyStringToCompound(&PaletteName, C, (u8)'\n');
                    Result.PaletteNames[It] = NewStringCompound(&GameState->Bucket.Fixed, PaletteName.Pos);
                    AppendStringCompoundToCompound(Result.PaletteNames+It, &PaletteName);
                    ResetStringCompound(PaletteName);
                    
                    color_palette *Palette = Result.Palettes+It;
                    AdvanceToNewline(&C);
                    
                    ProcessNextPaletteColor(&C, (u8 *)"Text: ", &Palette->Text);
                    ProcessNextPaletteColor(&C, (u8 *)"ForegroundText: ", &Palette->ForegroundText);
                    ProcessNextPaletteColor(&C, (u8 *)"ErrorText: ", &Palette->ErrorText);
                    ProcessNextPaletteColor(&C, (u8 *)"Foreground: ", &Palette->Foreground);
                    ProcessNextPaletteColor(&C, (u8 *)"Slot: ", &Palette->Slot);
                    ProcessNextPaletteColor(&C, (u8 *)"SliderBackground: ", &Palette->SliderBackground);
                    ProcessNextPaletteColor(&C, (u8 *)"SliderGrabThing: ", &Palette->SliderGrabThing);
                    ProcessNextPaletteColor(&C, (u8 *)"ButtonActive: ", &Palette->ButtonActive);
                    ProcessNextPaletteColor(&C, (u8 *)"Selected: ", &Palette->Selected);
                    ProcessNextPaletteColor(&C, (u8 *)"PlayingSong: ", &Palette->PlayingSong);
                }
                DeleteStringCompound(&GameState->Bucket.Transient, &PaletteName);
            }
            else
            {
                DebugLog(255, "WARNING:: Library file had wrong version. Needs Version %i!\n", SETTINGS_CURRENT_VERSION);
            }
            
            // If anything is wrong with the saved values, just use the default percentages
            if(Result.GenreArtistEdgeXPercent < 0.037143f || Result.GenreArtistEdgeXPercent > 0.920000f || 
               Result.ArtistAlbumEdgeXPercent < 0.058571f || Result.ArtistAlbumEdgeXPercent > 0.941429f ||
               Result.AlbumSongEdgeXPercent   < 0.087143f || Result.AlbumSongEdgeXPercent   > 0.962857f || 
               Result.GenreArtistEdgeXPercent >= Result.ArtistAlbumEdgeXPercent ||
               Result.ArtistAlbumEdgeXPercent >= Result.AlbumSongEdgeXPercent) 
            {
                Result.GenreArtistEdgeXPercent = 0.2f;
                Result.ArtistAlbumEdgeXPercent = 0.4f;
                Result.AlbumSongEdgeXPercent   = 0.6f;
            }
            
            if(Result.WindowDimX < GlobalMinWindowWidth)  Result.WindowDimX = GlobalMinWindowWidth;
            if(Result.WindowDimY < GlobalMinWindowHeight) Result.WindowDimY = GlobalMinWindowHeight;
        }
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
    
    return Result;
}

internal void
SaveSettingsFile(game_state *GameState, settings *Settings)
{
    string_c SaveData = NewStringCompound(&GameState->Bucket.Transient, 50000);
    
    AppendStringToCompound(&SaveData, (u8 *)"MPlay3Settings\nVersion ");
    I32ToString(&SaveData, SETTINGS_CURRENT_VERSION);
    AppendStringToCompound(&SaveData, (u8 *)"\n\n");
    
    NewLocalString(FileVolume,       50, "Volume: ");
    NewLocalString(FileLastSong,     50, "LastPlayingSong: ");
    NewLocalString(FileColorPalette, 50, "ColorPalette: ");
    NewLocalString(FileGenreArtist,  50, "GenreArtistEdgeXPercent: ");
    NewLocalString(FileArtistAlbum,  50, "ArtistAlbumEdgeXPercent: ");
    NewLocalString(FileAlbumSong,    50, "AlbumSongEdgeXPercent: ");
    NewLocalString(WindowDimX,       50, "WindowDimensionX: ");
    NewLocalString(WindowDimY,       50, "WindowDimensionY: ");
    NewLocalString(Looping,          50, "Looping: ");
    NewLocalString(Shuffle,          50, "Shuffle: ");
    
    v2i Dim = GetWindowSize();
    R32ToString(&FileVolume, GameState->SoundThreadInterface->ToneVolume);
    I32ToString(&FileLastSong, GameState->MusicInfo.PlayingSong.FileID.ID);
    I32ToString(&FileColorPalette, GameState->MusicInfo.DisplayInfo.ColorPaletteID);
    R32ToString(&FileGenreArtist, GameState->MusicInfo.DisplayInfo.GenreArtist.XPercent);
    R32ToString(&FileArtistAlbum, GameState->MusicInfo.DisplayInfo.ArtistAlbum.XPercent);
    R32ToString(&FileAlbumSong, GameState->MusicInfo.DisplayInfo.AlbumSong.XPercent);
    I32ToString(&WindowDimX, Dim.x);
    I32ToString(&WindowDimY, Dim.y);
    I32ToString(&Looping, GameState->MusicInfo.Looping == playLoop_Loop);
    I32ToString(&Shuffle, GameState->MusicInfo.IsShuffled);
    
    string_c LB = NewStaticStringCompound("\n");
    ConcatStringCompounds(21, &SaveData, &FileVolume, &LB, &FileLastSong, &LB, &FileColorPalette, &LB, &FileGenreArtist, &LB, &FileArtistAlbum, &LB, &FileAlbumSong, &LB, &WindowDimX, &LB, &WindowDimY, &LB, &Looping, &LB, &Shuffle, &LB);
    
    string_c Palette           = NewStaticStringCompound("Palette: ");
    string_c P_Text            = NewStaticStringCompound("\nText: ");
    string_c P_ForegroundText  = NewStaticStringCompound("\nForegroundText: ");
    string_c P_ErrorText       = NewStaticStringCompound("\nErrorText: ");
    string_c P_Foreground      = NewStaticStringCompound("\nForeground: ");
    string_c P_Slot            = NewStaticStringCompound("\nSlot: ");
    string_c P_SliderBG        = NewStaticStringCompound("\nSliderBackground: ");
    string_c P_SliderGrabThing = NewStaticStringCompound("\nSliderGrabThing: ");
    string_c P_ButtonActive    = NewStaticStringCompound("\nButtonActive: ");
    string_c P_Selected        = NewStaticStringCompound("\nSelected: ");
    string_c P_PlayingSong     = NewStaticStringCompound("\nPlayingSong: ");
    
    For(Settings->PaletteCount)
    {
        ConcatStringCompounds(3, &SaveData, &Palette, Settings->PaletteNames+It);
        
        AppendStringCompoundToCompound(&SaveData, &P_Text);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Text));
        AppendStringCompoundToCompound(&SaveData, &P_ForegroundText);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].ForegroundText));
        AppendStringCompoundToCompound(&SaveData, &P_ErrorText);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].ErrorText));
        AppendStringCompoundToCompound(&SaveData, &P_Foreground);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Foreground));
        AppendStringCompoundToCompound(&SaveData, &P_Slot);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Slot));
        AppendStringCompoundToCompound(&SaveData, &P_SliderBG);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].SliderBackground));
        AppendStringCompoundToCompound(&SaveData, &P_SliderGrabThing);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].SliderGrabThing));
        AppendStringCompoundToCompound(&SaveData, &P_ButtonActive);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].ButtonActive));
        AppendStringCompoundToCompound(&SaveData, &P_Selected);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Selected));
        AppendStringCompoundToCompound(&SaveData, &P_PlayingSong);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].PlayingSong));
        AppendCharToCompound(&SaveData, '\n');
    }
    
    
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+SETTINGS_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, SETTINGS_FILE_NAME);
    if(WriteEntireFile(&GameState->Bucket, FilePath.S, SaveData.Pos, SaveData.S))
    {
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
    DeleteStringCompound(&GameState->Bucket.Transient, &SaveData);
}

inline void
ApplySettings(game_state *GameState, settings Settings)
{
    ChangeVolume(GameState, Settings.Volume);
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->PlayingSong.PlaylistID = FileIDToPlaylistID(&MusicInfo->Playlist, Settings.PlayingSongID);
    if(Settings.PlayingSongID >= (i32)GameState->MP3Info->FileInfo.Count) Settings.PlayingSongID.ID = -1;
    MusicInfo->PlayingSong.FileID = Settings.PlayingSongID;
    
    ChangeSong(GameState, &MusicInfo->PlayingSong); 
    
    
    MusicInfo->DisplayInfo.ColorPaletteID = Settings.ColorPaletteID; 
    UpdateColorPalette(&MusicInfo->DisplayInfo, false);
    
    MusicInfo->DisplayInfo.GenreArtist.XPercent = Settings.GenreArtistEdgeXPercent;
    MusicInfo->DisplayInfo.ArtistAlbum.XPercent = Settings.ArtistAlbumEdgeXPercent;
    MusicInfo->DisplayInfo.AlbumSong.XPercent = Settings.AlbumSongEdgeXPercent;
    
    ProcessEdgeDragOnResize(&GameState->Renderer, &MusicInfo->DisplayInfo);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Genre, &MusicInfo->SortingInfo.Genre);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Artist, &MusicInfo->SortingInfo.Artist);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Album, &MusicInfo->SortingInfo.Album);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Song.Base, &MusicInfo->SortingInfo.Song);
    UpdateHorizontalSliders(&GameState->Renderer, &MusicInfo->DisplayInfo, &MusicInfo->SortingInfo);
    
    if(Settings.PlayingSongID >= 0)
    {
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Genre, Settings.PlayingSongID);
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Artist, Settings.PlayingSongID);
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Album, Settings.PlayingSongID);
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Song.Base, Settings.PlayingSongID);
    }
    
    ApplyWindowResize(GameState->Renderer.Window.WindowHandle, Settings.WindowDimX, Settings.WindowDimY, true);
    
    if(Settings.Looping) 
    {
        OnLoopPlaylistToggleOn(&MusicInfo->DisplayInfo.MusicBtnInfo);
        ToggleButtonVisuals(MusicInfo->DisplayInfo.LoopPlaylist, true);
    }
    if(Settings.Shuffle) 
    {
        OnShufflePlaylistToggleOn(&MusicInfo->DisplayInfo.MusicBtnInfo);
        ToggleButtonVisuals(MusicInfo->DisplayInfo.ShufflePlaylist, true);
    }
}

// NOTE:: Library file specification ************************
// 
// MP3Lib 
// Version 3
// P: PATH
// C: COUNT
//
// P: SUB_PATH_1
//
// >FILE_1_NAME
// >FILE_1_TITLE
// >FILE_1_ARTIST
// >FILE_1_ALBUM
// >FILE_1_GENRE
// >FILE_1_TRACK
// >FILE_1_YEAR
// >FILE_1_DURATION
// 
// >FILE_2_NAME
// ...
// >FILE_1_DURATION
//
// P: SUB_PATH_2
//
// >FILE_1_NAME
// ...
// >FILE_1_DURATION
// 

inline u32
CountToNewline(u8 *C)
{
    u32 Count = 0;
    while(*C != '\r' && *C++ != '\n') Count++;
    return Count;
}

inline u32
CopyUntilNewline(u8 *C, string_c *Result)
{
    u32 Count = 0;
    while(*C != '\r' && *C != '\n')
    {
        AppendCharToCompound(Result, *C++);
        Count++;
    }
    if(*C == '\r') Count++;
    return ++Count;
}

internal b32
ConfirmLibraryWithCorrectVersionExists(game_state *GameState, u32 VersionToCheckFor, u32 *FileInfoCount)
{
    b32 Result = false;
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    u32 BegginingCount = StringLength((u8 *)"MP3Lib\nVersion XX\nP: \nC: XXXXX\n");
    if(ReadBeginningOfFile(&GameState->Bucket.Transient, &Data, FilePath.S, BegginingCount+255))
    {
        u8 *C = Data.Data;
        u8 *LibString = (u8 *)"MP3Lib";
        if(StringCompare(C, LibString, 0, StringLength(LibString)))
        {
            C += StringLength(LibString)+1; // MP3Lib
            if(C[0] == '\r' || C[0] == '\n') C += 1;
            
            u8 *VersionString = (u8 *)"Version ";
            if(StringCompare(C, VersionString, 0, StringLength(VersionString)))
            {
                C += StringLength(VersionString); // Version 
                u8 L;
                if(ProcessNextU32InString(C, '-', L) == VersionToCheckFor)
                {
                    Result = true;
                    AdvanceToNewline(&C);
                    AdvanceToNewline(&C);
                    C += StringLength((u8 *)"C: ");
                    *FileInfoCount = ProcessNextU32InString(C, '-', L);
                }
            }
        }
        FreeFileMemory(&GameState->Bucket.Transient, Data.Data);
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
    
    return Result;
}

internal b32
CompareMP3LibraryFileSavedPath(game_state *GameState, string_c *PathToCompare)
{
    b32 Result = false;
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(ReadEntireFile(&GameState->Bucket.Transient, &Data, FilePath.S))
    {
        u8 *C = Data.Data;
        if(StringCompare(C, (u8 *)"MP3Lib", 0, 6))
        {
            C += 7; // MP3Lib
            C += 10; // Version 3
            C += 3; // P:_
            string_c FolderPath = NewStringCompound(&GameState->Bucket.Transient, 255);
            C += CopyUntilNewline(C, &FolderPath);
            
            if(CompareStringCompounds(&FolderPath, PathToCompare))
            {
                Result = true;
            }
            
            DeleteStringCompound(&GameState->Bucket.Transient, &FolderPath);
        }
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
    
    return Result;
}

internal void
LoadMP3LibraryFile(game_state *GameState, mp3_info *Info)
{
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(ReadEntireFile(&GameState->Bucket.Transient, &Data, FilePath.S))
    {
        mp3_file_info *MP3FileInfo = &Info->FileInfo;
        
        u8 *C = Data.Data;
        
        u8 *LibID = (u8 *)"MP3Lib";
        u8 *VersionString = (u8 *)"Version ";
        
        b32 FileIsCorrect = StringCompare(C, LibID, 0, StringLength(LibID));
        AdvanceToNewline(&C);
        FileIsCorrect = FileIsCorrect && StringCompare(C, VersionString, 0, StringLength(VersionString));
        C += StringLength(VersionString);
        u8 N = 0;
        u32 Version = ProcessNextU32InString(C, '\n', N);
        FileIsCorrect = FileIsCorrect && LIBRARY_CURRENT_VERSION == Version;
        
        if(!FileIsCorrect)
        {
            DebugLog(255, "WARNING:: Library file had wrong version. Needs Version %i!\n", LIBRARY_CURRENT_VERSION);
        }
        else
        {
            AdvanceToNewline(&C);
            C += 3;
            Info->FolderPath = NewStringCompound(&GameState->Bucket.Fixed, 255);
            C += CopyUntilNewline(C, &Info->FolderPath);
            
            C += 3;
            string_c CountS = NewStringCompound(&GameState->Bucket.Transient, CountToNewline(C));
            C += CopyUntilNewline(C, &CountS);
            u32 SongCount = ConvertU32FromString(CountS.S, CountS.Pos);
            DeleteStringCompound(&GameState->Bucket.Transient, &CountS);
            
            AdvanceToNewline(&C);
            string_c CurrentSubPath = NewStringCompound(&GameState->Bucket.Transient, 255);
            For(SongCount)
            {
                if(*C == 'P')
                {
                    C += 3;
                    WipeStringCompound(&CurrentSubPath);
                    C += CopyUntilNewline(C, &CurrentSubPath);
                    AdvanceToNewline(&C);
                }
                
                mp3_metadata *MD = MP3FileInfo->Metadata + It;
                
                MP3FileInfo->SubPath[It] = NewStringCompound(&GameState->Bucket.Fixed, CurrentSubPath.Pos);
                AppendStringCompoundToCompound(MP3FileInfo->SubPath+It, &CurrentSubPath);
                MP3FileInfo->FileName[It] = NewStringCompound(&GameState->Bucket.Fixed, CountToNewline(++C));
                C += CopyUntilNewline(C, MP3FileInfo->FileName+It);
                
                u32 Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Title = NewStringCompound(&GameState->Bucket.Fixed, Count);
                    C += CopyUntilNewline(C, &MD->Title);
                    MD->FoundFlags |= metadata_Title;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Artist = NewStringCompound(&GameState->Bucket.Fixed, Count);
                    C += CopyUntilNewline(C, &MD->Artist);
                    MD->FoundFlags |= metadata_Artist;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Album = NewStringCompound(&GameState->Bucket.Fixed, Count);
                    C += CopyUntilNewline(C, &MD->Album);
                    MD->FoundFlags |= metadata_Album;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Genre = NewStringCompound(&GameState->Bucket.Fixed, Count);
                    C += CopyUntilNewline(C, &MD->Genre);
                    MD->FoundFlags |= metadata_Genre;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->TrackString = NewStringCompound(&GameState->Bucket.Fixed, Count);
                    C += CopyUntilNewline(C, &MD->TrackString);
                    u8 Length = 0;
                    MD->Track = ProcessNextU32InString(MD->TrackString.S, '\0', Length);
                    MD->FoundFlags |= metadata_Track;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->YearString = NewStringCompound(&GameState->Bucket.Fixed, Count);
                    C += CopyUntilNewline(C, &MD->YearString);
                    u8 Length = 0;
                    MD->Year = ProcessNextU32InString(MD->YearString.S, '\0', Length);
                    MD->FoundFlags |= metadata_Year;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    string_c Duration = NewStringCompound(&GameState->Bucket.Transient, Count);
                    C += CopyUntilNewline(C, &Duration);
                    u8 Length = 0;
                    MD->Duration = ProcessNextU32InString(Duration.S, '\0', Length);
                    MillisecondsToMinutes(&GameState->Bucket.Fixed, MD->Duration, &MD->DurationString);
                    DeleteStringCompound(&GameState->Bucket.Transient, &Duration);
                    MD->FoundFlags |= metadata_Duration;
                }
                else AdvanceToNewline(&C);
                
                MP3FileInfo->Count++;
                AdvanceToNewline(&C);
            }
            DeleteStringCompound(&GameState->Bucket.Transient, &CurrentSubPath);
        }
        FreeFileMemory(&GameState->Bucket.Transient, Data.Data);
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
}


internal void
SaveMP3LibraryFile(game_state *GameState, mp3_info *Info)
{
    string_c NL    = NewStaticStringCompound("\n");
    string_c Colon = NewStaticStringCompound("P: ");
    string_c Inset = NewStaticStringCompound(">");
    
    mp3_file_info *MP3FileInfo = &Info->FileInfo;
    if(MP3FileInfo->Count == 0) return;
    u32 StringSize = (30+255+5) + 7*255*MP3FileInfo->Count;
    string_c SaveData = NewStringCompound(&GameState->Bucket.Transient, StringSize);
    
    AppendStringToCompound(&SaveData, (u8 *)"MP3Lib\n");
    AppendStringToCompound(&SaveData, (u8 *)"Version 3\n");
    ConcatStringCompounds(4, &SaveData, &Colon, &Info->FolderPath, &NL);
    char CountS[25];
    sprintf_s(CountS, "C: %i\n\n", MP3FileInfo->Count);
    AppendStringToCompound(&SaveData, (u8 *)CountS);
    
    u32 WrittenDataCount = 0;
    b32 *Written = PushArrayOnBucket(&GameState->Bucket.Transient, Info->FileInfo.MaxCount, b32);
    string_c CurrentSubPath = NewStringCompound(&GameState->Bucket.Transient, 255);
    while(WrittenDataCount < MP3FileInfo->Count)
    {
        AppendStringCompoundToCompound(&SaveData, &Colon);
        For(MP3FileInfo->Count)
        {
            if(!Written[It])
            {
                ConcatStringCompounds(4, &SaveData, MP3FileInfo->SubPath+It, &NL, &NL);
                AppendStringCompoundToCompound(&CurrentSubPath, MP3FileInfo->SubPath+It);
                break;
            }
        }
        For(MP3FileInfo->Count)
        {
            if(!Written[It] && 
               CompareStringCompounds(&CurrentSubPath, MP3FileInfo->SubPath+It))
            {
                mp3_metadata *MD = MP3FileInfo->Metadata+It;
                
                ConcatStringCompounds(4, &SaveData, Inset, MP3FileInfo->FileName[It], NL);
                if(MD->FoundFlags & metadata_Title)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Title, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Artist)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Artist, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Album)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Album, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Genre)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Genre, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Track && MD->Track > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Track);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Year && MD->Year > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Year);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Duration && MD->Duration > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n\n", MD->Duration);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(4, &SaveData, Inset, NL, NL);
                
                Written[It] = true;
                WrittenDataCount++;
            }
        }
        
        WipeStringCompound(&CurrentSubPath);
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &CurrentSubPath);
    PopFromTransientBucket(&GameState->Bucket.Transient, Written);
    
    // TODO:: Rename old save file as backup, before writing and after successful write delete the old one.
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(WriteEntireFile(&GameState->Bucket, FilePath.S, SaveData.Pos, SaveData.S))
    {
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
    DeleteStringCompound(&GameState->Bucket.Transient, &SaveData);
}

internal void
WipeMP3LibraryFile(game_state *GameState)
{
    string_c FilePath = NewStringCompound(&GameState->Bucket.Transient, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(WriteEntireFile(&GameState->Bucket, FilePath.S, 0, 0))
    {
    }
    DeleteStringCompound(&GameState->Bucket.Transient, &FilePath);
}

inline i32
AddSongToSortBatch(memory_bucket_container *Bucket, sort_batch *Batch, string_c *InsertCheck)
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
        Batch->Names[Batch->BatchCount] = NewStringCompound(Bucket, InsertCheck->Pos);
        AppendStringCompoundToCompound(Batch->Names+Batch->BatchCount, InsertCheck);
        Result = Batch->BatchCount++;
    }
    return Result;
}

inline void
InitializeSortBatch(memory_bucket_container *Bucket, sort_batch *Batch, u32 BatchCount)
{
    Batch->Genre  = PushArrayOnBucket(Bucket, BatchCount, array_batch_id);
    Batch->Artist = PushArrayOnBucket(Bucket, BatchCount, array_batch_id);
    Batch->Album  = PushArrayOnBucket(Bucket, BatchCount, array_batch_id);
    Batch->Song   = PushArrayOnBucket(Bucket, BatchCount, array_file_id);
    Batch->Names  = PushArrayOnBucket(Bucket, BatchCount, string_c);
    Batch->MaxBatches = BatchCount;
    Batch->BatchCount = 0;
}

internal void
CreateMusicSortingInfo(bucket_allocator *Bucket, mp3_info *MP3Info)
{
    music_sorting_info *SortInfo = &MP3Info->MusicInfo->SortingInfo;
    mp3_file_info *FileInfo = &MP3Info->FileInfo;
    
    // TODO:: Push this from transient to fixed with correct size?
    sort_batch Genre  = {};
    InitializeSortBatch(&Bucket->Fixed, &Genre, 256);
    sort_batch Artist = {};
    InitializeSortBatch(&Bucket->Fixed, &Artist, 512);
    sort_batch Album  = {};
    InitializeSortBatch(&Bucket->Fixed, &Album, 1024);
    
    song_sort_info *SortBatchInfo = PushArrayOnBucket(&Bucket->Fixed, FileInfo->Count, song_sort_info);
    
    u32 *Genre_CountForBatches  = PushArrayOnBucket(&Bucket->Transient, Genre.MaxBatches, u32);
    u32 *Artist_CountForBatches = PushArrayOnBucket(&Bucket->Transient, Artist.MaxBatches, u32);
    u32 *Album_CountForBatches  = PushArrayOnBucket(&Bucket->Transient, Album.MaxBatches, u32);
    
    // NOTE:: Filling the first slot with the _no_genre/artist/album_ 
    string_c Empty = {};
    AddSongToSortBatch(&Bucket->Fixed, &Genre, &Empty);
    AddSongToSortBatch(&Bucket->Fixed, &Artist, &Empty);
    AddSongToSortBatch(&Bucket->Fixed, &Album, &Empty);
    
    for(u32 FileID = 0; FileID < FileInfo->Count; FileID++)
    {
        mp3_metadata *MD = FileInfo->Metadata + FileID;
        
        // NOTE:: Fills the Batches with all different genres, artists, albums and gives the batch id back
        i32 GenreID  = AddSongToSortBatch(&Bucket->Fixed, &Genre, &MD->Genre);
        i32 ArtistID = AddSongToSortBatch(&Bucket->Fixed, &Artist, &MD->Artist);
        i32 AlbumID  = AddSongToSortBatch(&Bucket->Fixed, &Album, &MD->Album);
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
        Genre.Artist[It].A = CreateArray(&Bucket->Fixed, Genre_CountForBatches[It]);
        Genre.Album[It].A  = CreateArray(&Bucket->Fixed, Genre_CountForBatches[It]);
        Genre.Song[It].A   = CreateArray(&Bucket->Fixed, Genre_CountForBatches[It]);
    }
    For(Artist.BatchCount)
    {
        Artist.Album[It].A = CreateArray(&Bucket->Fixed, Artist_CountForBatches[It]);
        Artist.Song[It].A  = CreateArray(&Bucket->Fixed, Artist_CountForBatches[It]);
    }
    For(Album.BatchCount)
    {
        Album.Song[It].A = CreateArray(&Bucket->Fixed, Album_CountForBatches[It]);
        if(It == 0) Album.Genre[It].A = CreateArray(&Bucket->Fixed, 100);
        else Album.Genre[It].A = CreateArray(&Bucket->Fixed, 10);
    }
    
    PopFromTransientBucket(&Bucket->Transient, Genre_CountForBatches);
    PopFromTransientBucket(&Bucket->Transient, Artist_CountForBatches);
    PopFromTransientBucket(&Bucket->Transient, Album_CountForBatches);
    
    
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
    MusicInfo->SortingInfo.Genre.Selected.A  = CreateArray(&Bucket->Fixed, Genre.BatchCount);
    MusicInfo->SortingInfo.Artist.Selected.A = CreateArray(&Bucket->Fixed, Artist.BatchCount);
    MusicInfo->SortingInfo.Album.Selected.A  = CreateArray(&Bucket->Fixed, Album.BatchCount);
    MusicInfo->SortingInfo.Song.Selected.A   = CreateArray(&Bucket->Fixed, FileInfo->Count);
    
    MusicInfo->SortingInfo.Genre.Displayable.A  = CreateArray(&Bucket->Fixed, Genre.BatchCount);
    For(Genre.BatchCount)
    {
        Push(&MusicInfo->SortingInfo.Genre.Displayable, NewFileID(It));
    }
    MusicInfo->SortingInfo.Artist.Displayable.A = CreateArray(&Bucket->Fixed, Artist.BatchCount);
    MusicInfo->SortingInfo.Album.Displayable.A  = CreateArray(&Bucket->Fixed, Album.BatchCount);
    MusicInfo->SortingInfo.Song.Displayable.A   = CreateArray(&Bucket->Fixed, MP3Info->FileInfo.Count);
    
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

internal void
RetraceFilePath(bucket_allocator *Bucket, mp3_info *MP3Info, file_id FileID)
{
    mp3_file_info TmpFileInfo = CreateFileInfoStruct(&Bucket->Transient, 1000);
    string_compound FilePath = NewStringCompound(&Bucket->Transient, 255);
    ConcatStringCompounds(2, &FilePath, &MP3Info->FolderPath);
    string_compound SubPath = NewStringCompound(&Bucket->Transient, 255);;
    ConcatStringCompounds(2, &SubPath, MP3Info->FileInfo.SubPath + FileID.ID);
    
    FindAllMP3FilesInFolder(&Bucket->Transient, &Bucket->Transient, &FilePath, &SubPath, &TmpFileInfo);
    CrawlFilesForMetadata(&Bucket->Transient, &Bucket->Transient, &TmpFileInfo, &MP3Info->FolderPath);
    
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
                *NewFileName = NewStringCompound(&Bucket->Fixed, TmpFileInfo.FileName[It].Pos);
            }
            else ResetStringCompound(*NewFileName);
            AppendStringCompoundToCompound(NewFileName, TmpFileInfo.FileName+It);
            
            if(NewSubPath->Length < TmpFileInfo.SubPath[It].Pos)
            {
                *NewSubPath = NewStringCompound(&Bucket->Fixed, TmpFileInfo.SubPath[It].Pos);
            }
            else ResetStringCompound(*NewSubPath);
            AppendStringCompoundToCompound(NewSubPath, TmpFileInfo.SubPath+It);
        }
        
        DeleteStringCompound(&Bucket->Transient, TmpFileInfo.FileName+It);
        DeleteStringCompound(&Bucket->Transient, TmpFileInfo.SubPath+It);
        DeleteStringCompound(&Bucket->Transient, &FoundMD->Title);
        DeleteStringCompound(&Bucket->Transient, &FoundMD->Artist);
        DeleteStringCompound(&Bucket->Transient, &FoundMD->Album);
        DeleteStringCompound(&Bucket->Transient, &FoundMD->Genre);
        DeleteStringCompound(&Bucket->Transient, &FoundMD->TrackString);
        DeleteStringCompound(&Bucket->Transient, &FoundMD->YearString);
    }
    DeleteStringCompound(&Bucket->Transient, &FilePath);
    DeleteStringCompound(&Bucket->Transient, &SubPath);
    DeleteFileInfoStruct(&Bucket->Transient, &TmpFileInfo);
}

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
    MusicInfo->Playlist.Songs.A  = CreateArray(&GameState->Bucket.Fixed, GameState->MP3Info->FileInfo.Count+200);
    MusicInfo->Playlist.UpNext.A = CreateArray(&GameState->Bucket.Fixed, 200);
    
    MusicInfo->PlayingSong = {-1, -1, -1, 0};
    CreateMusicSortingInfo(&GameState->Bucket, GameState->MP3Info);
    FillDisplayables(&MusicInfo->SortingInfo, &GameState->MP3Info->FileInfo, &MusicInfo->DisplayInfo);
    SortDisplayables(&MusicInfo->SortingInfo, &GameState->MP3Info->FileInfo);
    UseDisplayableAsPlaylist(MusicInfo);
    UpdateAllDisplayColumns(GameState);
    SaveMP3LibraryFile(GameState, GameState->MP3Info);
    
    MusicInfo->DisplayInfo.Genre.Search.InitialDisplayables.A = CreateArray(&GameState->Bucket.Fixed, 
                                                                            MusicInfo->SortingInfo.Genre.Displayable.A.Length);
    MusicInfo->DisplayInfo.Artist.Search.InitialDisplayables.A = CreateArray(&GameState->Bucket.Fixed, 
                                                                             MusicInfo->SortingInfo.Artist.Displayable.A.Length);
    MusicInfo->DisplayInfo.Album.Search.InitialDisplayables.A = CreateArray(&GameState->Bucket.Fixed, 
                                                                            MusicInfo->SortingInfo.Album.Displayable.A.Length);
    MusicInfo->DisplayInfo.Song.Base.Search.InitialDisplayables.A = CreateArray(&GameState->Bucket.Fixed, 
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
    GameState->MP3Info->FileInfo = CreateFileInfoStruct(&GameState->Bucket.Fixed, MAX_MP3_INFO_COUNT);
    Result = FindAllMP3FilesInFolder(&GameState->Bucket.Fixed, &GameState->Bucket.Transient, &GameState->MP3Info->FolderPath,
                                     &SubPath, &GameState->MP3Info->FileInfo);
    CrawlFilesForMetadata(&GameState->Bucket.Fixed, &GameState->Bucket.Transient, &GameState->MP3Info->FileInfo,
                          &GameState->MP3Info->FolderPath);
    
    ApplyNewMetadata(GameState, MusicInfo);
    return Result;
}

inline void 
ReplaceFolderPath(mp3_info *MP3Info, string_c *NewPath)
{
    ResetStringCompound(MP3Info->FolderPath);
    AppendStringCompoundToCompound(&MP3Info->FolderPath, NewPath);
}

internal void
HandleActiveMusicPath(music_display_info *DisplayInfo, input_info *Input, crawl_thread_out *CrawlInfoOut)
{
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    
    if(CrawlInfoOut->DoneFolderSearch && MusicPath->CrawlThreadStateCount == 2) 
    {
        if(CrawlInfoOut->ThreadIsRunning) MusicPath->CrawlThreadStateCount = 5;
        else MusicPath->CrawlThreadStateCount = 4;
    }
    if(CrawlInfoOut->DoneCrawling && MusicPath->CrawlThreadStateCount == 6) MusicPath->CrawlThreadStateCount = 7;
    
    switch(MusicPath->CrawlThreadStateCount)
    {
        case 0: // #Many: Just opened, no thread running yet
        {
            if(Input->KeyChange[KEY_ENTER] == KeyDown) OnMusicPathSavePressed(&DisplayInfo->MusicBtnInfo);
            ProcessMusicPath(&GlobalGameState.Renderer, GlobalGameState.Time.dTime, Input, &MusicPath->TextField);
            ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, MusicPath->Save);
            ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, MusicPath->Quit);
            ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, MusicPath->Rescan);
        } break;
        
        case 1: // #Once: Started search
        {
            SetActive(&MusicPath->LoadingBar, true);
            UpdateLoadingBar(&MusicPath->LoadingBar, 0.1f);
            MusicPath->CrawlThreadStateCount++;
        } // #Through
        
        case 2: // #Many: Is searching
        {
            // Animate indeterminite loading bar
            if(MusicPath->dWaitThenCancelTime >= 1.0f)
            {
                MusicPath->dWaitThenCancelTime = 0.0f;
                UpdateIndeterminiteLoadingBar(&MusicPath->LoadingBar, 1);
            }
            else 
            {
                MusicPath->dWaitThenCancelTime += GlobalGameState.Time.dTime/1.0f;
                UpdateIndeterminiteLoadingBar(&MusicPath->LoadingBar, MusicPath->dWaitThenCancelTime);
            }
        } break;
        
        case 4: // #Once: Searching done. Nothing found. Done.
        {
            TestFolderSearchDone(&DisplayInfo->MusicPath, CrawlInfoOut->TestCount);
            MusicPath->dWaitThenCancelTime = 0.0f;
            SetActive(&MusicPath->LoadingBar, false);
            
            CrawlInfoOut->DoneFolderSearch = false;
            SetActive(&MusicPath->LoadingBar, false);
            MusicPath->CrawlThreadStateCount = 0;
        } break;
        
        case 5: // #Once: Searching done. Started crawling
        {
            TestFolderSearchDone(&DisplayInfo->MusicPath, CrawlInfoOut->TestCount);
            MusicPath->dWaitThenCancelTime = 0.0f;
            SetActive(&MusicPath->LoadingBar, false);
            
            SetActive(&MusicPath->LoadingBar, true);
            MusicPath->CrawlThreadStateCount++;
        } // #Through
        
        case 6: // #Many: Is crawling
        {
            r32 CurrentPercentage = CrawlInfoOut->CurrentCount/(r32)CrawlInfoOut->TestCount;
            UpdateLoadingBar(&MusicPath->LoadingBar, CurrentPercentage);
        } break;
        
        case 7: // #Once: Done crawling
        {
            ApplyNewMetadata(&GlobalGameState, &GlobalGameState.MusicInfo);
            FinishedSettingUpMusicPath(&GlobalGameState, &DisplayInfo->MusicPath);
            UpdateLoadingBar(&MusicPath->LoadingBar, 1);
            MusicPath->CrawlThreadStateCount++;
        } // #Through
        
        case 8: // #Many: Wait and display
        {
            // Wait for user to maybe read/see the last info.
            if(MusicPath->dWaitThenCancelTime >= 1.0f) // #Once: Done with _wait and display_
            {
                MusicPath->dWaitThenCancelTime = 0.0f;
                OnMusicPathQuitPressed(&DisplayInfo->MusicBtnInfo);
                SetActive(&MusicPath->LoadingBar, false);
                MusicPath->CrawlThreadStateCount = 0;
            }
            else MusicPath->dWaitThenCancelTime += GlobalGameState.Time.dTime/1.5f;
        } break;
        
        InvalidDefaultCase;
    }
    
}

internal void
LoadNewMetadata_Thread(bucket_allocator *Bucket, crawl_thread *CrawlInfo)
{
    mp3_info *MP3Info = CrawlInfo->MP3Info;
    
    string_c SubPath = {};
    mp3_file_info Test = CreateFileInfoStruct(&Bucket->Transient, MAX_MP3_INFO_COUNT);
    FindAllMP3FilesInFolder(&Bucket->Transient, &Bucket->Transient, &CrawlInfo->TestPath, &SubPath, &Test);
    CrawlInfo->Out->TestCount = Test.Count;
    CrawlInfo->Out->DoneFolderSearch = true;
    
    if(Test.Count > 0) 
    {
        ReplaceFolderPath(MP3Info, &CrawlInfo->TestPath);
        
        MP3Info->FileInfo = CreateFileInfoStruct(&GlobalGameState.Bucket.Fixed, MAX_MP3_INFO_COUNT);
        SubPath = {};
        FindAllMP3FilesInFolder(&GlobalGameState.Bucket.Fixed, &Bucket->Transient, &MP3Info->FolderPath, 
                                &SubPath, &MP3Info->FileInfo);
        
        CrawlFilesForMetadata(&GlobalGameState.Bucket.Fixed, &Bucket->Transient, &MP3Info->FileInfo, &MP3Info->FolderPath, 
                              &CrawlInfo->Out->CurrentCount);
        CrawlInfo->Out->DoneCrawling = true;
    }
    
    DeleteFileInfoStruct(&Bucket->Transient, &Test);
    CrawlInfo->Out->ThreadIsRunning = false;
}

internal void
CheckForMusicPathMP3sChanged_End(check_music_path *CheckMusicPath, music_path_ui *MusicPath)
{
    mp3_file_info *FileInfo = &GlobalGameState.MP3Info->FileInfo;
    mp3_file_info *TestInfo = &CheckMusicPath->TestInfo;
    
    For(CheckMusicPath->RemoveIDs.Count)
    {
        RemoveFileFromInfo(FileInfo, Get(&CheckMusicPath->RemoveIDs, It));
    }
    
    if(CheckMusicPath->AddTestInfoIDs.Count > 0)
    {
        ResizeFileInfo(&GlobalGameState.Bucket.Fixed, FileInfo, FileInfo->MaxCount+CheckMusicPath->AddTestInfoIDs.Count);
    }
    For(CheckMusicPath->AddTestInfoIDs.Count)
    {
        u32 NewIt = Get(&CheckMusicPath->AddTestInfoIDs, It);
        b32 R = AddFileToInfo(&GlobalGameState.Bucket, FileInfo, TestInfo->SubPath+NewIt, TestInfo->FileName+NewIt);
        Assert(R);
    }
    
    if(CheckMusicPath->RemoveIDs.Count > 0 || CheckMusicPath->AddTestInfoIDs.Count > 0) 
    {
        ApplyNewMetadata(&GlobalGameState, &GlobalGameState.MusicInfo);
        DebugLog(255, "NOTE:: Removed %i songs, added %i songs.\n", CheckMusicPath->RemoveIDs.Count,
                 CheckMusicPath->AddTestInfoIDs.Count)
    }
    else DebugLog(50, "NOTE:: No songs were added or removed!\n");
    
    if(MusicPath->TextField.IsActive)
    {
        ResetStringCompound(MusicPath->OutputString);
        AppendStringToCompound(&MusicPath->OutputString, (u8 *)"Removed ");
        I32ToString(&MusicPath->OutputString, CheckMusicPath->RemoveIDs.Count);
        AppendStringToCompound(&MusicPath->OutputString, (u8 *)" songs and added ");
        I32ToString(&MusicPath->OutputString, CheckMusicPath->AddTestInfoIDs.Count);
        AppendStringToCompound(&MusicPath->OutputString, (u8 *)" songs from old path.");
        
        RemoveRenderText(&MusicPath->Output);
        CreateRenderText(&GlobalGameState.Renderer, GlobalGameState.Renderer.FontInfo.MediumFont, &MusicPath->OutputString,
                         &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.ForegroundText, &MusicPath->Output, -0.6f-0.001f, MusicPath->TextField.LeftAlign, V2(0, -175));
    }
    
    DestroyArray(&GlobalGameState.Bucket.Transient, CheckMusicPath->AddTestInfoIDs);
    DestroyArray(&GlobalGameState.Bucket.Transient, CheckMusicPath->RemoveIDs);
    DeleteFileInfoStruct(&GlobalGameState.Bucket.Transient, TestInfo);
    
    CheckMusicPath->State = threadState_Inactive;
}

inline b32
IsHigher(i32 T1, i32 T2, void *Data)
{
    array_u32 *Array = (array_u32 *)Data;
    
    return Get(Array, T1) > Get(Array, T2);
}

internal void
CheckForMusicPathMP3sChanged_Thread(bucket_allocator *Bucket, check_music_path *CheckMusicPath)
{
    string_c SubPath = {};
    mp3_file_info *TestInfo = &CheckMusicPath->TestInfo;
    FindAllMP3FilesInFolder(&Bucket->Transient, &Bucket->Transient, &CheckMusicPath->MP3Info->FolderPath, &SubPath, TestInfo);
    
    mp3_file_info *FileInfo = &CheckMusicPath->MP3Info->FileInfo;
    
    // Find which files are missing and remove them from library
    For(FileInfo->Count, MD)
    {
        b32 FileFound = false;
        For(TestInfo->Count)
        {
            if(CompareStringCompounds(FileInfo->SubPath+MDIt, TestInfo->SubPath+It))
            {
                if(CompareStringCompounds(FileInfo->FileName+MDIt, TestInfo->FileName+It))
                {
                    FileFound = true;
                    break;
                }
            }
        }
        
        if(!FileFound) Push(&CheckMusicPath->RemoveIDs, MDIt);
    }
    QuickSort(0, CheckMusicPath->RemoveIDs.Count-1, &CheckMusicPath->RemoveIDs, {IsHigher, &CheckMusicPath->RemoveIDs});
    
    if(TestInfo->Count != FileInfo->Count)
    {
        // Find which files are new and add them to library
        For(TestInfo->Count, New)
        {
            b32 FileFound = false;
            For(FileInfo->Count)
            {
                if(CompareStringCompounds(FileInfo->SubPath+It, TestInfo->SubPath+NewIt))
                {
                    if(CompareStringCompounds(FileInfo->FileName+It, TestInfo->FileName+NewIt))
                    {
                        FileFound = true;
                        break;
                    }
                }
            }
            
            if(!FileFound) Push(&CheckMusicPath->AddTestInfoIDs, NewIt);
        }
    }
    CheckMusicPath->State = threadState_Finished;
}

// ******************
// Threading ********
// ******************
internal JOB_LIST_CALLBACK(JobLoadAndDecodeMP3File)
{
    job_load_decode_mp3 *JobInfo = (job_load_decode_mp3 *)Data;
    
    
#ifdef DECODE_STREAMING_TMP
    mp3dec_file_info_t *DecodeResult = 0;
    
    if(JobInfo->PreloadSeconds == DECODE_PRELOAD_SECONDS) 
        DecodeResult = JobInfo->MP3Info->DecodeInfo.DecodedData + JobInfo->DecodeID;
    else DecodeResult = &JobInfo->MP3Info->DecodeInfo.PlayingDecoded;
    
    error_item Result = LoadAndDecodeMP3StartFrames(ThreadInfo->Bucket, JobInfo->PreloadSeconds, JobInfo->FileID, 
                                                    JobInfo->DecodeID, DecodeResult);
#else 
    error_item Result = LoadAndDecodeMP3Data(ThreadInfo->Bucket, JobInfo->MP3Info, JobInfo->FileID, JobInfo->DecodeID, false);
#endif
    
    if(Result.Code < 0) PushErrorMessageFromThread(Result);
}

internal JOB_LIST_CALLBACK(JobLoadNewMetadata)
{
    crawl_thread *JobInfo = (crawl_thread *)Data;
    LoadNewMetadata_Thread(ThreadInfo->Bucket, JobInfo);
}

internal JOB_LIST_CALLBACK(JobCheckMusicPathChanged)
{
    check_music_path *JobInfo = *((check_music_path **)Data);
    CheckForMusicPathMP3sChanged_Thread(ThreadInfo->Bucket, JobInfo);
}

internal b32
AddJob_LoadMetadata(game_state *GameState)
{
    b32 Result = false;
    
    if(!GameState->CrawlInfo.Out->ThreadIsRunning)
    {
        Result = true;
        
        GameState->MusicInfo.IsPlaying = false;
        PushSoundBufferClear(GameState->SoundThreadInterface);
        
        GameState->CrawlInfo.Out->DoneFolderSearch = false;
        GameState->CrawlInfo.Out->DoneCrawling     = false;
        GameState->CrawlInfo.Out->ThreadIsRunning  = true;
        GameState->CrawlInfo.Out->CurrentCount     = 0;
        
        AddJobToQueue(&GameState->JobQueue, JobLoadNewMetadata, GameState->CrawlInfo);
    }
    
    return Result;
}

internal void
AddJob_CheckMusicPathChanged(check_music_path *CheckMusicPath)
{
    CheckMusicPath->MP3Info        = GlobalGameState.MP3Info;
    CheckMusicPath->TestInfo       = CreateFileInfoStruct(&GlobalGameState.Bucket.Transient, MAX_MP3_INFO_COUNT);
    CheckMusicPath->RemoveIDs      = CreateArray(&GlobalGameState.Bucket.Transient, MAX_MP3_INFO_COUNT);
    CheckMusicPath->AddTestInfoIDs = CreateArray(&GlobalGameState.Bucket.Transient, MAX_MP3_INFO_COUNT);
    CheckMusicPath->State          = threadState_Running;
    AddJobToQueue(&GlobalGameState.JobQueue, JobCheckMusicPathChanged, CheckMusicPath);
}

internal i32
AddJob_LoadMP3(game_state *GameState, circular_job_queue *JobQueue, file_id FileID, 
               array_u32 *IgnoreDecodeIDs, i32 PreloadSeconds)
{
    mp3_info *MP3Info = GameState->MP3Info;
    if(FileID < 0) return -1;
    
    i32 DecodeID = -1;
    if(!IsSongDecoded(GameState->MP3Info, FileID, &DecodeID))
    {
        DecodeID = GetNextDecodeIDToEvict(&MP3Info->DecodeInfo, IgnoreDecodeIDs);
        Assert(DecodeID >= 0);
        if(!MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID])
        {
            // If this gets called twice, the second time IsSongDecoded will 
            // definately be true, even if job is not done. 
            Put(&MP3Info->DecodeInfo.FileID, NewDecodeID(DecodeID), FileID);
            MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = true;
            
            job_load_decode_mp3 Data = {MP3Info, FileID, DecodeID, PreloadSeconds};
            AddJobToQueue(JobQueue, JobLoadAndDecodeMP3File, Data);
        }
    }
    
    Assert(DecodeID >= 0);
    TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
    return DecodeID;
}

internal void
AddJobs_LoadMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs)
{
    mp3_info *MP3Info = GameState->MP3Info;
    play_list *Playlist = &GameState->MusicInfo.Playlist;
    playing_song Song = GameState->MusicInfo.PlayingSong;
    if(Song.PlaylistID < 0) return;
    
    if(Song.PlaylistID >= 0)
    {
        u32 PlaylistSize = Playlist->Songs.A.Count + Playlist->UpNext.A.Count;
        b32 DoNext = true;
        playlist_id CurrentNext = GetPreviousSong(Playlist, Song.PlaylistID);
        playlist_id CurrentPrev = Song.PlaylistID;
        for(u32 It = 0; 
            It < PlaylistSize && It < MAX_MP3_DECODE_COUNT;
            It++)
        {
            if(DoNext)
            {
                CurrentNext.ID = (CurrentNext.ID+1)%Playlist->Songs.A.Count;
                file_id FileID = PlaylistIDToFileID(Playlist, CurrentNext);
                AddJob_LoadMP3(GameState, JobQueue, FileID, IgnoreDecodeIDs);
            }
            else
            {
                CurrentPrev = GetPreviousSong(Playlist, CurrentPrev);
                file_id FileID = PlaylistIDToFileID(Playlist, CurrentPrev);
                AddJob_LoadMP3(GameState, JobQueue, FileID, IgnoreDecodeIDs);
            }
            DoNext = !DoNext;
        }
    }
}

internal void
AddJobs_LoadOnScreenMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs)
{
    music_display_column *DisplayColumn = &GameState->MusicInfo.DisplayInfo.Song.Base;
    play_list *Playlist = &GameState->MusicInfo.Playlist;
    
    u32 IgnoreCount = 0;
    if(IgnoreDecodeIDs) IgnoreCount = Min(IgnoreDecodeIDs->Count, MAX_MP3_DECODE_COUNT);
    for(u32 It = 0; 
        It < Playlist->Songs.A.Count &&
        It < DisplayColumn->Count && 
        It < MAX_MP3_DECODE_COUNT - IgnoreCount;
        It++)
    {
        file_id FileID = PlaylistIDToFileID(Playlist, NewPlaylistID(DisplayColumn->OnScreenIDs[It]));
        AddJob_LoadMP3(GameState, JobQueue, FileID, IgnoreDecodeIDs);
    }
}

internal b32
AddJob_NextUndecodedInPlaylist()
{
    b32 Result = false;
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    if(MusicInfo->PlayingSong.PlaylistID < 0) return Result;
    
    playlist_id StartPlaylistID = NewPlaylistID(0);
    if(MusicInfo->PlayingSong.PlaylistID >= 0) StartPlaylistID = MusicInfo->PlayingSong.PlaylistID;
    
    for(u32 It = StartPlaylistID.ID + 1; 
        It < MusicInfo->Playlist.Songs.A.Count; 
        It++)
    {
        u32 DecodeID = 0;
        playlist_id PID = NewPlaylistID(It);
        if(!Find(&DecodeInfo->FileID, Get(&MusicInfo->Playlist.Songs, PID), &DecodeID))
        {
            Result = true;
            AddJob_LoadMP3(&GlobalGameState, &GlobalGameState.JobQueue, Get(&MusicInfo->Playlist.Songs, PID));
            break;
        }
        else TouchDecoded(DecodeInfo, DecodeID);
    }
    
    // If we did not find anything, meaning nothing needs to be decoded, then we check if
    // we want to loop and try to find something to decode at the beginning of the playlist.
    if(!Result && MusicInfo->Looping == playLoop_Loop)
    {
        Assert(StartPlaylistID.ID >= 0);
        For((u32)StartPlaylistID.ID)
        {
            u32 DecodeID = 0;
            playlist_id PID = NewPlaylistID(It);
            if(!Find(&DecodeInfo->FileID, Get(&MusicInfo->Playlist.Songs, PID), &DecodeID))
            {
                Result = true;
                AddJob_LoadMP3(&GlobalGameState, &GlobalGameState.JobQueue, Get(&MusicInfo->Playlist.Songs, PID));
                break;
            }
            else TouchDecoded(DecodeInfo, DecodeID);
        }
    }
    
    return Result;
}

internal void
PushErrorMessageFromThread(error_item Error)
{
    WaitForSingleObjectEx(GlobalGameState.ThreadErrorList.Mutex, INFINITE, false);
    if(GlobalGameState.ThreadErrorList.Count < MAX_THREAD_ERRORS)
    {
        GlobalGameState.ThreadErrorList.Errors[GlobalGameState.ThreadErrorList.Count++] = Error;
        GlobalGameState.ThreadErrorList.RemoveDecode = true;
    }
    ReleaseMutex(GlobalGameState.ThreadErrorList.Mutex);
}

internal error_item
PopErrorMessageFromThread()
{
    error_item Result = {loadErrorCode_NoError, {-1}};
    WaitForSingleObjectEx(GlobalGameState.ThreadErrorList.Mutex, INFINITE, false);
    if(GlobalGameState.ThreadErrorList.Count > 0) 
    {
        Result = GlobalGameState.ThreadErrorList.Errors[--GlobalGameState.ThreadErrorList.Count];
    }
    ReleaseMutex(GlobalGameState.ThreadErrorList.Mutex);
    return Result;
}

inline void
RemoveDecodeFails()
{
    WaitForSingleObjectEx(GlobalGameState.ThreadErrorList.Mutex, INFINITE, false);
    
    u32 DecodeID = 0;
    For(GlobalGameState.ThreadErrorList.Count)
    {
        if(Find(&GlobalGameState.MP3Info->DecodeInfo.FileID, GlobalGameState.ThreadErrorList.Errors[It].ID, &DecodeID))
        {
            Put(&GlobalGameState.MP3Info->DecodeInfo.FileID.A, DecodeID, MAX_UINT32);
            Put(&GlobalGameState.MP3Info->DecodeInfo.LastTouched, DecodeID, 0);
            if(GlobalGameState.MusicInfo.PlayingSong.DecodeID == (i32)DecodeID) 
            {
                GlobalGameState.MusicInfo.PlayingSong.PlaylistID.ID = -1;
                GlobalGameState.MusicInfo.PlayingSong.FileID.ID = -1;
                GlobalGameState.MusicInfo.PlayingSong.DecodeID = -1;
            }
        }
    }
    
    GlobalGameState.ThreadErrorList.RemoveDecode = false;
    ReleaseMutex(GlobalGameState.ThreadErrorList.Mutex);
}

internal void
ProcessThreadErrors()
{
    if(GlobalGameState.ThreadErrorList.Count)
    {
        if(GlobalGameState.ThreadErrorList.RemoveDecode) RemoveDecodeFails();
        
        if(!GlobalGameState.MusicInfo.DisplayInfo.UserErrorText.IsAnimating)
        {
            error_item NextError = PopErrorMessageFromThread();
            switch(NextError.Code)
            {
                case loadErrorCode_DecodingFailed:
                {
                    string_c ErrorMsg = NewStringCompound(&GlobalGameState.Bucket.Transient, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not decode song. Is file corrupted? (");
                    AppendStringCompoundToCompound(&ErrorMsg, GlobalGameState.MP3Info->FileInfo.FileName + NextError.ID.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    PushUserErrorMessage(&ErrorMsg);
                    DeleteStringCompound(&GlobalGameState.Bucket.Transient, &ErrorMsg);
                } break;
                
                case loadErrorCode_FileLoadFailed:
                {
                    string_c ErrorMsg = NewStringCompound(&GlobalGameState.Bucket.Transient, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not load song from disk. If files were moved, do a retrace. (");
                    AppendStringCompoundToCompound(&ErrorMsg, GlobalGameState.MP3Info->FileInfo.FileName + NextError.ID.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    PushUserErrorMessage(&ErrorMsg);
                    DeleteStringCompound(&GlobalGameState.Bucket.Transient, &ErrorMsg);
                } break;
                
                case loadErrorCode_EmptyFile:
                {
                    string_c ErrorMsg = NewStringCompound(&GlobalGameState.Bucket.Transient, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not load song. File was empty. (");
                    AppendStringCompoundToCompound(&ErrorMsg, GlobalGameState.MP3Info->FileInfo.FileName + NextError.ID.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    PushUserErrorMessage(&ErrorMsg);
                    DeleteStringCompound(&GlobalGameState.Bucket.Transient, &ErrorMsg);
                } break;
            }
        }
    }
}













