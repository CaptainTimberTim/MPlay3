#include "Sound_Backend_TD.h"
#include "GameBasics_TD.h"
internal u32 CreateHash(string_c Name, u64 CreationDate);

// This is done in such a way because having the large list
// as a global variable destroys the CodeGeneration stage
// of the compiler in optimized build.
#define GENRE_TYPE_COUNT 192
struct genre_types_list
{
    string_compound G[GENRE_TYPE_COUNT];
};
global_variable genre_types_list GenreTypesList;

internal void
PrepareGenreTypesList()
{
    // These are all genres that were available for the ID3v1 metadata.
    string_compound GenreTypes[] =
    {
        NewStaticStringCompound("Blues"),
        NewStaticStringCompound("Classic rock"),
        NewStaticStringCompound("Country"),
        NewStaticStringCompound("Dance"),
        NewStaticStringCompound("Disco"),
        NewStaticStringCompound("Funk"),
        NewStaticStringCompound("Grunge"),
        NewStaticStringCompound("Hip-Hop"),
        NewStaticStringCompound("Jazz"),
        NewStaticStringCompound("Metal"),
        NewStaticStringCompound("New Age"),
        NewStaticStringCompound("Oldies"),
        NewStaticStringCompound("Other"),
        NewStaticStringCompound("Pop"),
        NewStaticStringCompound("Rhythm and Blues"),
        NewStaticStringCompound("Rap"),
        NewStaticStringCompound("Reggae"),
        NewStaticStringCompound("Rock"),
        NewStaticStringCompound("Techno"),
        NewStaticStringCompound("Industrial"),
        NewStaticStringCompound("Alternative"),
        NewStaticStringCompound("Ska"),
        NewStaticStringCompound("Death metal"),
        NewStaticStringCompound("Pranks"),
        NewStaticStringCompound("Soundtrack"),
        NewStaticStringCompound("Euro-Techno"),
        NewStaticStringCompound("Ambient"),
        NewStaticStringCompound("Trip-Hop"),
        NewStaticStringCompound("Vocal"),
        NewStaticStringCompound("Jazz & Funk"),
        NewStaticStringCompound("Fusion"),
        NewStaticStringCompound("Trance"),
        NewStaticStringCompound("Classical"),
        NewStaticStringCompound("Instrumental"),
        NewStaticStringCompound("Acid"),
        NewStaticStringCompound("House"),
        NewStaticStringCompound("Game"),
        NewStaticStringCompound("Sound clip"),
        NewStaticStringCompound("Gospel"),
        NewStaticStringCompound("Noise"),
        NewStaticStringCompound("Alternative Rock"),
        NewStaticStringCompound("Bass"),
        NewStaticStringCompound("Soul"),
        NewStaticStringCompound("Punk"),
        NewStaticStringCompound("Space"),
        NewStaticStringCompound("Meditative"),
        NewStaticStringCompound("Instrumental Pop"),
        NewStaticStringCompound("Instrumental Rock"),
        NewStaticStringCompound("Ethnic"),
        NewStaticStringCompound("Gothic"),
        NewStaticStringCompound("Darkwave"),
        NewStaticStringCompound("Techno-Industrial"),
        NewStaticStringCompound("Electronic"),
        NewStaticStringCompound("Pop-Folk"),
        NewStaticStringCompound("Eurodance"),
        NewStaticStringCompound("Dream"),
        NewStaticStringCompound("Southern Rock"),
        NewStaticStringCompound("Comedy"),
        NewStaticStringCompound("Cult"),
        NewStaticStringCompound("Gangsta"),
        NewStaticStringCompound("Top 40"),
        NewStaticStringCompound("Christian Rap"),
        NewStaticStringCompound("Pop/Funk"),
        NewStaticStringCompound("Jungle"),
        NewStaticStringCompound("Native US"),
        NewStaticStringCompound("Cabaret"),
        NewStaticStringCompound("New Wave"),
        NewStaticStringCompound("Psychedelic"),
        NewStaticStringCompound("Rave"),
        NewStaticStringCompound("Show tunes"),
        NewStaticStringCompound("Trailer"),
        NewStaticStringCompound("Lo-Fi"),
        NewStaticStringCompound("Tribal"),
        NewStaticStringCompound("Acid Punk"),
        NewStaticStringCompound("Acid Jazz"),
        NewStaticStringCompound("Polka"),
        NewStaticStringCompound("Retro"),
        NewStaticStringCompound("Musical"),
        NewStaticStringCompound("Rock ’n’ Roll"),
        NewStaticStringCompound("Hard Rock"),
        NewStaticStringCompound("Folk"),
        NewStaticStringCompound("Folk-Rock"),
        NewStaticStringCompound("National Folk"),
        NewStaticStringCompound("Swing"),
        NewStaticStringCompound("Fast Fusion"),
        NewStaticStringCompound("Bebop"),
        NewStaticStringCompound("Latin"),
        NewStaticStringCompound("Revival"),
        NewStaticStringCompound("Celtic"),
        NewStaticStringCompound("Bluegrass"),
        NewStaticStringCompound("Avantgarde"),
        NewStaticStringCompound("Gothic Rock"),
        NewStaticStringCompound("Progressive Rock"),
        NewStaticStringCompound("Psychedelic Rock"),
        NewStaticStringCompound("Symphonic Rock"),
        NewStaticStringCompound("Slow rock"),
        NewStaticStringCompound("Big Band"),
        NewStaticStringCompound("Chorus"),
        NewStaticStringCompound("Easy Listening"),
        NewStaticStringCompound("Acoustic"),
        NewStaticStringCompound("Humour"),
        NewStaticStringCompound("Speech"),
        NewStaticStringCompound("Chanson"),
        NewStaticStringCompound("Opera"),
        NewStaticStringCompound("Chamber music"),
        NewStaticStringCompound("Sonata"),
        NewStaticStringCompound("Symphony"),
        NewStaticStringCompound("Booty bass"),
        NewStaticStringCompound("Primus"),
        NewStaticStringCompound("Porn grooveb"),
        NewStaticStringCompound("Satire"),
        NewStaticStringCompound("Slow jam"),
        NewStaticStringCompound("Club"),
        NewStaticStringCompound("Tango"),
        NewStaticStringCompound("Samba"),
        NewStaticStringCompound("Folklore"),
        NewStaticStringCompound("Ballad"),
        NewStaticStringCompound("Power ballad"),
        NewStaticStringCompound("Rhythmic Soul"),
        NewStaticStringCompound("Freestyle"),
        NewStaticStringCompound("Duet"),
        NewStaticStringCompound("Punk Rock"),
        NewStaticStringCompound("Drum solo"),
        NewStaticStringCompound("A cappella"),
        NewStaticStringCompound("Euro-House"),
        NewStaticStringCompound("Dance Hall"),
        NewStaticStringCompound("Goa"),
        NewStaticStringCompound("Drum & Bass"),
        NewStaticStringCompound("Club-House"),
        NewStaticStringCompound("Hardcore Techno"),
        NewStaticStringCompound("Terror"),
        NewStaticStringCompound("Indie"),
        NewStaticStringCompound("BritPop"),
        NewStaticStringCompound("Negerpunk"),
        NewStaticStringCompound("Polsk Punk"),
        NewStaticStringCompound("Beat"),
        NewStaticStringCompound("Christian Gangsta Rap"),
        NewStaticStringCompound("Heavy Metal"),
        NewStaticStringCompound("Black Metal"),
        NewStaticStringCompound("Crossover"),
        NewStaticStringCompound("Contemporary Christian"),
        NewStaticStringCompound("Christian rock"),
        NewStaticStringCompound("Merengue"),
        NewStaticStringCompound("Salsa"),
        NewStaticStringCompound("Thrash Metal"),
        NewStaticStringCompound("Anime"),
        NewStaticStringCompound("Jpop"),
        NewStaticStringCompound("Synthpop"),
        NewStaticStringCompound("Abstract"),
        NewStaticStringCompound("Art Rock"),
        NewStaticStringCompound("Baroque"),
        NewStaticStringCompound("Bhangra"),
        NewStaticStringCompound("Big beat"),
        NewStaticStringCompound("Breakbeat"),
        NewStaticStringCompound("Chillout"),
        NewStaticStringCompound("Downtempo"),
        NewStaticStringCompound("Dub"),
        NewStaticStringCompound("EBM"),
        NewStaticStringCompound("Eclectic"),
        NewStaticStringCompound("Electro"),
        NewStaticStringCompound("Electroclash"),
        NewStaticStringCompound("Emo"),
        NewStaticStringCompound("Experimental"),
        NewStaticStringCompound("Garage"),
        NewStaticStringCompound("Global"),
        NewStaticStringCompound("IDM"),
        NewStaticStringCompound("Illbient"),
        NewStaticStringCompound("Industro-Goth"),
        NewStaticStringCompound("Jam Band"),
        NewStaticStringCompound("Krautrock"),
        NewStaticStringCompound("Leftfield"),
        NewStaticStringCompound("Lounge"),
        NewStaticStringCompound("Math Rock"),
        NewStaticStringCompound("New Romantic"),
        NewStaticStringCompound("Nu-Breakz"),
        NewStaticStringCompound("Post-Punk"),
        NewStaticStringCompound("Post-Rock"),
        NewStaticStringCompound("Psytrance"),
        NewStaticStringCompound("Shoegaze"),
        NewStaticStringCompound("Space Rock"),
        NewStaticStringCompound("Trop Rock"),
        NewStaticStringCompound("World Music"),
        NewStaticStringCompound("Neoclassical"),
        NewStaticStringCompound("Audiobook"),
        NewStaticStringCompound("Audio theatre"),
        NewStaticStringCompound("Neue Deutsche Welle"),
        NewStaticStringCompound("Podcast"),
        NewStaticStringCompound("Indie-Rock"),
        NewStaticStringCompound("G-Funk"),
        NewStaticStringCompound("Dubstep"),
        NewStaticStringCompound("Garage Rock"),
        NewStaticStringCompound("Psybient")
    };
    For(ArrayCount(GenreTypes))
    {
        GenreTypesList.G[It] = GenreTypes[It];
    }
}

inline playlist_array
CreatePlaylistList(arena_allocator *Arena, u32 Count)
{
    playlist_array Result = {NULL, 0, Count};
    Result.List = AllocateArray(Arena, Count, playlist_info);
    return Result;
}

inline mp3_metadata *
GetMetadata(playlist_column *SongColumn, mp3_file_info *FileInfo, displayable_id ID)
{
    mp3_metadata *Result = 0;
    Assert(ID.ID >= 0);
    
    Assert(SongColumn->Type == columnType_Song);
    
    playlist_id PlaylistID = Get(&SongColumn->Displayable, ID);
    Result = FileInfo->Metadata + Get(&SongColumn->FileIDs.A, PlaylistID.ID);
    
    return Result;
}

inline mp3_metadata *
GetMetadata(playlist_column *SongColumn, mp3_file_info *FileInfo, playlist_id PlaylistID)
{
    mp3_metadata *Result = 0;
    Assert(PlaylistID >= 0);
    
    Assert(SongColumn->Type == columnType_Song);
    Result = FileInfo->Metadata + Get(&SongColumn->FileIDs.A, PlaylistID.ID);
    
    return Result;
}

inline string_c *
GetSongFileName(playlist_column *SongColumn, mp3_file_info *FileInfo, playlist_id PlaylistID)
{
    string_c *Result = 0;
    Assert(PlaylistID >= 0);
    Assert(SongColumn->Type == columnType_Song);
    
    Result = FileInfo->FileNames_ + Get(&SongColumn->FileIDs.A, PlaylistID.ID);
    
    return Result;
}

inline file_id
FileIDFromPlaylistID(playlist_column *SongColumn, playlist_id PlaylistID)
{
    Assert(PlaylistID >= 0);
    file_id Result = NewFileID(Get(&SongColumn->FileIDs.A, PlaylistID.ID));
    return Result;
}

inline playlist_id
PlaylistIDFromFileID(playlist_column *SongColumn, file_id FileID)
{
    playlist_id Result = NewPlaylistID(-1);
    StackFind(&SongColumn->FileIDs, FileID, &Result.ID);
    return Result;
}



inline void
AdvanceToNewline(u8 **String)
{
    while(*String[0] != 0 && *(String[0])++ != '\n') ;
}

inline void
ClearFileInfoStruct(mp3_file_info *FileInfo)
{
    For(FileInfo->Count_) 
    {
        if(!FileInfo->Metadata) break;
        else if(!(FileInfo->Metadata+It)) break;
    }
    ClearArray(FileInfo->FileNames_, FileInfo->MaxCount_, string_c);
    ClearArray(FileInfo->SubPath,    FileInfo->MaxCount_, string_c);
    ClearArray(FileInfo->Metadata,   FileInfo->MaxCount_, mp3_metadata);
    // NoHash:: ClearArray(FileInfo->Hashes,     FileInfo->MaxCount_, u32);
    FileInfo->Count_ = 0;
}

inline void // #ThreadedUse
CreateFileInfoStruct(mp3_file_info *FileInfo, u32 FileInfoCount)
{
    u32 MemorySize = FileInfoCount*sizeof(string_c)*2 + FileInfoCount*sizeof(mp3_metadata);
    // NoHash:: + FileInfoCount*sizeof(u32);
    u8 *Memory = AllocateMemory(&GlobalGameState.JobThreadsArena, MemorySize, Private);
    
    FileInfo->FileNames_ = (string_c *)Memory;
    Memory += FileInfoCount*sizeof(string_c);
    FileInfo->SubPath    = (string_c *)Memory;
    Memory += FileInfoCount*sizeof(string_c);
    FileInfo->Metadata   = (mp3_metadata *)Memory;
    Memory += FileInfoCount*sizeof(mp3_metadata);
    // NoHash:: FileInfo->Hashes  = (u32 *)Memory;
    Memory += FileInfoCount*sizeof(u32);
    
    FileInfo->MaxCount_ = FileInfoCount;
    FileInfo->Count_    = 0;
}

inline void // #ThreadedUse
DeleteFileInfoStruct(mp3_file_info *FileInfo)
{
    For(FileInfo->Count_)
    {
        DeleteStringCompound(&GlobalGameState.JobThreadsArena, FileInfo->SubPath+It);
        DeleteStringCompound(&GlobalGameState.JobThreadsArena, FileInfo->FileNames_+It);
    }
    // As I allocate all fileInfo memory as one big block, I just need to dealloc once!
    if(FileInfo->MaxCount_) FreeMemory(&GlobalGameState.JobThreadsArena, FileInfo->FileNames_);
    FileInfo->Count_     = 0;
    FileInfo->MaxCount_  = 0;
    FileInfo->FileNames_ = 0;
    FileInfo->SubPath    = 0;
    FileInfo->Metadata   = 0;
    // NoHash:: FileInfo->Hashes  = 0;
}

inline void // #ThreadedUse
ResizeFileInfo(mp3_file_info *FileInfo, u32 NewMaxCount)
{
    Assert(NewMaxCount >= 0);
    string_c    *OldFileNames = FileInfo->FileNames_;
    string_c     *OldSubPaths = FileInfo->SubPath;
    mp3_metadata *OldMetadata = FileInfo->Metadata;
    // NoHash:: u32            *OldHashes = FileInfo->Hashes;
    i32              OldCount = FileInfo->Count_;
    
    CreateFileInfoStruct(FileInfo, NewMaxCount);
    
    for(u32 It = 0; 
        It < NewMaxCount && 
        It < (u32)OldCount; 
        ++It)
    {
        FileInfo->FileNames_[It] = OldFileNames[It];
        FileInfo->SubPath[It]    = OldSubPaths[It];
        FileInfo->Metadata[It]   = OldMetadata[It];
        // NoHash:: FileInfo->Hashes[It]     = OldHashes[It];
    }
    
    FileInfo->Count_ = Min((i32)NewMaxCount, OldCount);
    FileInfo->MaxCount_ = NewMaxCount;
    if(OldCount) FreeMemory(&GlobalGameState.JobThreadsArena, OldFileNames);
}

inline mp3_info *
CreateMP3InfoStruct(arena_allocator *Arena, u32 FileInfoCount)
{
    mp3_info *Result = AllocateStruct(Arena, mp3_info);
    CreateFileInfoStruct(&Result->FileInfo, FileInfoCount);
    Result->DecodeInfo.FileIDs.A = CreateArray(Arena, MAX_MP3_DECODE_COUNT);
    Result->DecodeInfo.LastTouched   = CreateArray(Arena, MAX_MP3_DECODE_COUNT);
    
    return Result;
}

inline b32
IsSongDecoded(mp3_info *MP3Info, file_id FileID, i32 *DecodeID_out = 0)
{
    b32 Result = false;
    if(DecodeID_out) *DecodeID_out = -1;
    For(MP3Info->DecodeInfo.Count)
    {
        if(FileID == Get(&MP3Info->DecodeInfo.FileIDs, NewDecodeID(It))) 
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
    For(MP3Info->FileInfo.Count_)
    {
        if(CompareStringCompounds(FileName, &MP3Info->FileInfo.FileNames_[It]))
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
    For(MP3Info->FileInfo.Count_)
    {
        if(CompareStringAndCompound(Name, (u8 *)MP3Info->FileInfo.FileNames_[It].S))
        {
            for(i32 DecodeID = 0; DecodeID < (i32)MP3Info->DecodeInfo.Count; ++DecodeID)
            {
                if(Get(&MP3Info->DecodeInfo.FileIDs, NewDecodeID(DecodeID)) == It)
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

inline playlist_id
SongNameToDisplayableID(mp3_info *MP3Info, string_compound *Name)
{
    playlist_id Result = NewPlaylistID(-1);
    array_playlist_id *Displayable = &MP3Info->MusicInfo->Playlist_->Song.Displayable;
    For(Displayable->A.Count)
    {
        playlist_id PlaylistID = Get(Displayable, NewDisplayableID(It));
        if(CompareStringCompounds(Name, GetSongFileName(&MP3Info->MusicInfo->Playlist_->Song, &MP3Info->FileInfo, PlaylistID)))
        {
            Result.ID = It;
            break;
        }
    }
    return Result;
}

inline file_id
FileIDFromFilePath(mp3_file_info *FileInfo, string_c *SubPath, string_c *Filename)
{
    file_id Result = NewFileID(-1);
    For(FileInfo->Count_)
    {
        if(CompareStringCompounds(SubPath, FileInfo->SubPath+It))
        {
            if(CompareStringCompounds(Filename, FileInfo->FileNames_+It))
            {
                Result.ID = It;
                break;
            }
        }
    }
    return Result;
}

inline displayable_id
DisplayableIDFromPlaylistID(array_playlist_id *Displayable, playlist_id PlaylistID)
{
    displayable_id Result = NewDisplayableID(-1);
    if(PlaylistID < 0) return Result;
    
    For(Displayable->A.Count)
    {
        displayable_id DID = NewDisplayableID(It);
        if(Get(Displayable, DID) == PlaylistID)
        {
            Result = DID;
            break;
        }
    }
    
    return Result;
}

inline displayable_id
DisplayableIDFromPlaylistID(music_info *MusicInfo, playlist_id PlaylistID)
{
    array_playlist_id *Displayable = &MusicInfo->Playlist_->Song.Displayable;
    return DisplayableIDFromPlaylistID(Displayable, PlaylistID);
}

inline playlist_id
PlaylistIDFromDisplayableID(music_info *MusicInfo, displayable_id DisplayableID)
{
    playlist_id PlaylistID = NewPlaylistID(-1);
    if(DisplayableID >= 0) PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, DisplayableID);
    return PlaylistID;
}

inline file_id
FileIDFromDisplayableID(music_info *MusicInfo, displayable_id DisplayableID)
{
    file_id FileID = NewFileID(-1);
    if(DisplayableID >= 0) {
        playlist_id PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, DisplayableID);
        FileID = NewFileID(Get(&MusicInfo->Playlist_->Song.FileIDs.A, PlaylistID.ID));
    }
    return FileID;
}

inline displayable_id
DisplayableIDFromOnScreenID(music_info *MusicInfo, u32 OnScreenID, playlist_id *PlaylistID)
{
    displayable_id DisplayableID = NewDisplayableID(-1);
    playlist_id ID = Get(&MusicInfo->Playlist_->Song.Displayable, MusicInfo->DisplayInfo.Song.Base.OnScreenIDs[OnScreenID]);
    StackFind(&MusicInfo->Playlist_->Song.Displayable, ID, &DisplayableID.ID);
    
    if(PlaylistID) *PlaylistID = ID;
    return DisplayableID;
}

// Acquires the corresponding Genre/Artist/Album DisplayID for given FileID
inline displayable_id
PlaylistIDToColumnDisplayID(music_info *MusicInfo, music_display_column *DisplayColumn, playlist_id PlaylistID)
{
    displayable_id Result = NewDisplayableID(-1);
    string_c *CompareS = 0;
    switch(DisplayColumn->Type)
    {
        case columnType_Genre:
        {
            CompareS = &GetMetadata(&MusicInfo->Playlist_->Song, &GlobalGameState.MP3Info->FileInfo, PlaylistID)->Genre;
        } break;
        case columnType_Artist:
        {
            CompareS = &GetMetadata(&MusicInfo->Playlist_->Song, &GlobalGameState.MP3Info->FileInfo, PlaylistID)->Artist;
        } break;
        case columnType_Album:
        {
            CompareS = &GetMetadata(&MusicInfo->Playlist_->Song, &GlobalGameState.MP3Info->FileInfo, PlaylistID)->Album;
        } break;
        
        case columnType_Song:
        {
            return DisplayableIDFromPlaylistID(&MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable, PlaylistID);
        } break;
        InvalidDefaultCase;
    }
    
    playlist_column *Playlist = MusicInfo->Playlist_->Columns + DisplayColumn->Type;
    For(Playlist->Displayable.A.Count)
    {
        displayable_id DID = NewDisplayableID(It);
        string_c *Name  = Playlist->Batch.Names+Get(&Playlist->Displayable, DID).ID;
        if(CompareStringCompounds(CompareS, Name)) Result = DID;
        
    }
    
    return Result;
}

inline displayable_id
SortingIDToColumnDisplayID(playlist_info *Playlist, music_display_column *DisplayColumn, batch_id BatchID)
{
    displayable_id Result = NewDisplayableID(-1);
    
    array_playlist_id *Displayable = &Playlist->Columns[DisplayColumn->Type].Displayable;
    string_c *CompareS = 0;
    switch(DisplayColumn->Type)
    {
        case columnType_Genre:
        {
            CompareS = &Playlist->Genre.Batch.Names[BatchID.ID];
        } break;
        case columnType_Artist:
        {
            CompareS = &Playlist->Artist.Batch.Names[BatchID.ID];
        } break;
        case columnType_Album:
        {
            CompareS = &Playlist->Album.Batch.Names[BatchID.ID];
        } break;
        
        InvalidDefaultCase;
    }
    
    For(Displayable->A.Count)
    {
        displayable_id DID = NewDisplayableID(It);
        string_c *Name  = Playlist->Columns[DisplayColumn->Type].Batch.Names+Get(Displayable, DID).ID;
        if(CompareStringCompounds(CompareS, Name)) Result = DID;
    }
    
    return Result;
}

internal void
UpdatePlayingSongForSelectionChange(music_info *MusicInfo)
{
    // If the currently playing song is in the new displayable list set DisplayableID.
    // If not, the music stops.
    if(MusicInfo->PlayingSong.PlaylistID < 0)    return;
    if(MusicInfo->PlayingSong.DisplayableID < 0) return;
    
    displayable_id DisplayableID = DisplayableIDFromPlaylistID(MusicInfo, MusicInfo->PlayingSong.PlaylistID);
    if(DisplayableID >= 0)
    {
        MusicInfo->PlayingSong.DisplayableID = DisplayableID;
    }
    else 
    {
        MusicInfo->PlayingSong.DisplayableID.ID = -1;
        MusicInfo->PlayingSong.DecodeID = -1;
    }
}

inline playlist_id
GetNextSong(music_info *MusicInfo)
{
    playlist_id PlaylistID = NewPlaylistID(-1);
    if(MusicInfo->PlayingSong.PlayUpNext) PlaylistID = Get(&MusicInfo->UpNextList, NewDisplayableID(0));
    else PlaylistID = PlaylistIDFromDisplayableID(MusicInfo, MusicInfo->PlayingSong.DisplayableID);
    return PlaylistID; 
}

inline displayable_id
CheckForLooping(u32 DisplayableCount, displayable_id PlayingSongID, play_loop Looping)
{
    displayable_id Result = PlayingSongID;
    if(Looping == playLoop_Repeat) ; // DisplayableID does not change;
    else if(Looping == playLoop_Loop) Result.ID = (Result.ID+1)%DisplayableCount;
    else 
    {
        Result.ID++;
        if(Result >= (i32)DisplayableCount) Result.ID = -1;
    }
    return Result;
}

internal void
SetNextSong(music_info *MusicInfo)
{
    playing_song *PlayingSong = &MusicInfo->PlayingSong ;
    play_loop Looping = MusicInfo->Looping;
    Assert(PlayingSong->DisplayableID >= -1);
    
    if(MusicInfo->UpNextList.A.Count != 0) 
    {
        if(PlayingSong->PlayUpNext) 
        {
            Take(&MusicInfo->UpNextList, NewDisplayableID(0));
            if(MusicInfo->UpNextList.A.Count == 0) 
            {
                PlayingSong->PlayUpNext = false;
                PlayingSong->DisplayableID = CheckForLooping(MusicInfo->Playlist_->Song.Displayable.A.Count, 
                                                             PlayingSong->DisplayableID, Looping);
                PlayingSong->PlaylistID = PlaylistIDFromDisplayableID(MusicInfo, PlayingSong->DisplayableID);
            }
        }
        else
        {
            PlayingSong->PlayUpNext = true;
        }
    }
    else 
    {
        PlayingSong->DisplayableID = CheckForLooping(MusicInfo->Playlist_->Song.Displayable.A.Count, 
                                                     PlayingSong->DisplayableID, Looping);
        PlayingSong->PlaylistID = PlaylistIDFromDisplayableID(MusicInfo, PlayingSong->DisplayableID);
    }
}

inline displayable_id
GetSongAfterCurrent(u32 DisplayableCount, displayable_id DisplayableID)
{
    Assert(DisplayableCount >= 0);
    Assert(DisplayableID.ID > -1);
    Assert(DisplayableID.ID < (i32)DisplayableCount);
    
    displayable_id Result = NewDisplayableID((DisplayableID.ID+1)%DisplayableCount);
    return Result;
}

inline displayable_id
GetPreviousSong(u32 DisplayableCount, displayable_id DisplayableID)
{
    displayable_id Result = NewDisplayableID(-1);
    
    Assert(DisplayableCount >= 0);
    Assert(DisplayableID.ID >= -1);
    Assert(DisplayableID.ID < (i32)DisplayableCount);
    
    Result.ID = DisplayableID.ID - 1;
    if(Result.ID < 0) Result.ID = DisplayableCount-1;
    
    return Result;
}

inline void
SetPreviousSong(music_info *MusicInfo)
{
    play_loop Looping = MusicInfo->Looping;
    playing_song *PlayingSong = &MusicInfo->PlayingSong;
    Assert(PlayingSong->DisplayableID >= -1);
    
    if(PlayingSong->DisplayableID != -1)
    {
        if(Looping == playLoop_Repeat) ;
        else if(Looping == playLoop_Loop) 
        {
            PlayingSong->DisplayableID.ID -= 1;
            PlayingSong->PlaylistID = PlaylistIDFromDisplayableID(MusicInfo, PlayingSong->DisplayableID);
            if(PlayingSong->DisplayableID < 0) 
            {
                PlayingSong->DisplayableID.ID = MusicInfo->Playlist_->Song.Displayable.A.Count-1;
                PlayingSong->PlaylistID = PlaylistIDFromDisplayableID(MusicInfo, PlayingSong->DisplayableID);
            }
        }
        else 
        {
            PlayingSong->DisplayableID.ID -= 1;
            PlayingSong->PlaylistID = PlaylistIDFromDisplayableID(MusicInfo, PlayingSong->DisplayableID);
        }
    }
}

internal b32 // #ThreadedUse
FindAllMP3FilesInFolder(arena_allocator *TransientArena, string_compound *FolderPath, string_compound *SubPath, mp3_file_info *ResultingFileInfo)
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
        
        while(HasNextFile)
        {
            if(ResultingFileInfo->Count_ >= ResultingFileInfo->MaxCount_)
            {
                DebugLog(80, "NOTE:: Increasing FileInfo size from %i to %i.\n", ResultingFileInfo->MaxCount_, ResultingFileInfo->MaxCount_ + MAX_MP3_INFO_STEP);
                ResizeFileInfo(ResultingFileInfo, ResultingFileInfo->MaxCount_ + MAX_MP3_INFO_STEP);
            }
            
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
                    
                    FindAllMP3FilesInFolder(TransientArena, FolderPath, 
                                            &NewSubFolderPath, ResultingFileInfo);
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
                            ResultingFileInfo->SubPath[ResultingFileInfo->Count_] = 
                                NewStringCompound(&GlobalGameState.JobThreadsArena, SubPath->Pos);
                            AppendStringCompoundToCompound(ResultingFileInfo->SubPath+ResultingFileInfo->Count_, SubPath);
                            
                            ResultingFileInfo->FileNames_[ResultingFileInfo->Count_] =
                                NewStringCompound(&GlobalGameState.JobThreadsArena, FileName.Pos);
                            AppendStringCompoundToCompound(ResultingFileInfo->FileNames_+ResultingFileInfo->Count_, &FileName);
                            
                            // NoHash:: u64 CreationTime = (u64)FileData.ftCreationTime.dwHighDateTime << 32 | FileData.ftCreationTime.dwLowDateTime;
                            // NoHash:: ResultingFileInfo->Hashes[ResultingFileInfo->Count_] = CreateHash(FileName, CreationTime);
                            
                            
                            ResultingFileInfo->Count_++;
                            Assert(ResultingFileInfo->Count_ <= ResultingFileInfo->MaxCount_);
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

inline void
TryInsertFoundMetadataID3v1(arena_allocator *Arena, mp3_metadata *MD, string_c *MDOutString, metadata_flags CheckFlag, u8 *ID3Value)
{
    if(~MD->FoundFlags & CheckFlag && StringLength(ID3Value) != 0)
    {
        u8 Temp[31*2] = {};
        
        u32 Count = 0;
        For(30)
        {
            if(ID3Value[It] != '\0') 
            {
                if(     ID3Value[It] < 128) Temp[Count++] = ID3Value[It];
                else if(ID3Value[It] < 256)
                {
                    // NOTE:: This conversion to utf-8 is from: https://stackoverflow.com/a/4059934
                    Temp[Count++] = 0xc2 + (ID3Value[It] > 0xbf);
                    Temp[Count++] = (ID3Value[It] & 0x3f) + 0x80;
                }
                else Temp[Count++] = ID3Value[It]; // TODO:: Do we really want to do this? I am not sure if this is even possible...
            }
        }
        
        *MDOutString = NewStringCompound(Arena, Count);
        AppendStringToCompound(MDOutString, Temp);
        EatLeadingSpaces(MDOutString);
        EatTrailingSpaces(MDOutString);
        MD->FoundFlags |= CheckFlag;
    }
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
            if(*Current++ == 0) Track = *Current; // NOTE:: If a track number is stored, this byte contains a binary 0.
            Current++;
            Genre = *Current++;
            
            TryInsertFoundMetadataID3v1(Arena, Metadata, &Metadata->Title, metadata_Title, Title);
            TryInsertFoundMetadataID3v1(Arena, Metadata, &Metadata->Artist, metadata_Artist, Artist);
            TryInsertFoundMetadataID3v1(Arena, Metadata, &Metadata->Album, metadata_Album, Album);
            
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
                if(Genre < GENRE_TYPE_COUNT)
                {
                    Metadata->Genre = GenreTypesList.G[Genre];
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
    // 2.3 Header:
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
            DebugLog(255, "Error:: Could not load %s metadata. Tag size was %i, must be erroneous !\n", Frame, Length);
        }
        else
        {
            C      += VersionHeaderLength;
            Result += VersionHeaderLength;
            
            // $00: ISO-8859-1 [ISO-8859-1]. Terminated with $00.
            // $01: UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM. All strings in the same frame SHALL have the same byteorder. Terminated with $00 00.
            // $02: UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM. Terminated with $00 00.
            // $03: UTF-8 [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.
            u8 Encoding = *C;
            ++C; --Length; ++Result;
            
            switch(Encoding)
            {
                case 0: { // ISO-8859-1
                    *S = NewStringCompound(Arena, Length*2);
                    if(Length > 0)
                    {
                        For((u32)Length)
                        {
                            if(*C == 0) ; // Do nothing
                            else if(*C < 128) S->S[S->Pos++] = *C;
                            else
                            {
                                // NOTE:: This conversion to utf-8 is from: https://stackoverflow.com/a/4059934
                                S->S[S->Pos++] = 0xc2 + (*C > 0xbf);
                                S->S[S->Pos++] = (*C & 0x3f) + 0x80;
                            }
                            C++;
                        }
                        EatLeadingSpaces(S);
                        Result += Length - 1; // -1 to not skip anything when increasing It in For loop (parent)
                    }
                    else Result = 0;
                } break;
                
                case 1: {  // UTF-16 Unicode with BOM
                    // Byte-order. But I could not find, if the MSDN procedure will 
                    // consider it, I found no way to give it the byte order...
                    if(C[0] == 0xFF && C[1] == 0xFE || 
                       C[0] == 0xFE && C[1] == 0xFF )
                    {
                        C += 2; Length -= 2; Result += 2;
                    }
                } // #Through
                case 2: {  // UTF-16 Unicode without BOM
                    if(Length > 0) 
                    {
                        // We put a definite ending to the string, in order to
                        // make sure the procedure ConvertString16To8 ends correctly.
                        wchar_t *CC  = (wchar_t *)C;
                        wchar_t Save = CC[Length/2]; 
                        CC[Length/2] = 0;
                        
                        ConvertString16To8(Arena, CC, S);
                        CC[Length/2] = Save;
                        Result += Length - 1;
                    }
                    else Result = 0;
                } break;
                
                case 3: { // UTF-8
                    *S = NewStringCompound(Arena, Length);
                    if(Length > 0)
                    {
                        For((u32)Length)
                        {
                            if(*C == 0) ; // Do nothing
                            else S->S[S->Pos++] = *C;
                            C++;
                        }
                        EatLeadingSpaces(S);
                        Result += Length - 1; // -1 to not skip anything when increasing It in For loop (parent)
                    }
                    else Result = 0;
                } break;
                
                InvalidDefaultCase;
            }
        }
    }
    return Result;
}

internal u32
MetadataID3v2_3(arena_allocator *MainArena, arena_allocator *ScratchArena, 
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
                if(JmpIter > 0) 
                {
                    Metadata->FoundFlags |= metadata_Artist;
                }
            }
            if(~Metadata->FoundFlags & metadata_Genre)
            {
                string_compound GenreNr = {};
                JmpIter = MetadataID3v2_Helper(ScratchArena, Current+It, GENRE, &GenreNr, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Genre;
                if(Metadata->FoundFlags & metadata_Genre)
                {
                    if(GenreNr.S[0] == '(')
                    {
                        u8 L = 0;
                        u32 GNr = ProcessNextU32InString(GenreNr.S+1, ')', L);
                        Metadata->Genre = NewStringCompound(MainArena, GenreTypesList.G[GNr].Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreTypesList.G[GNr]);
                    }
                    else if(IsStringCompANumber(&GenreNr))
                    {
                        u8 L = 0;
                        u32 GNr = ProcessNextU32InString(GenreNr.S, '\0', L);
                        Metadata->Genre = NewStringCompound(MainArena, GenreTypesList.G[GNr].Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreTypesList.G[GNr]);
                    }
                    else 
                    {
                        Metadata->Genre = NewStringCompound(MainArena, GenreNr.Pos);
                        AppendStringCompoundToCompound(&Metadata->Genre, &GenreNr);
                    }
                    
                    DeleteStringCompound(ScratchArena, &GenreNr);
                }
            }
            if(~Metadata->FoundFlags & metadata_Track)
            {
                string_compound Track = {};
                JmpIter = MetadataID3v2_Helper(ScratchArena, Current+It, TRACK, &Track, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Track;
                if(Metadata->FoundFlags & metadata_Track)
                {
                    i32 SlashPos = FindFirstOccurrenceOfCharInStringCompound(&Track, '/');
                    if(SlashPos != -1) Track.Pos = SlashPos;
                    Metadata->Track = Clamp(ConvertU32FromString(&Track), 0, 9999);
                    Assert(Metadata->Track < 10000);
                    Metadata->TrackString = NewStringCompound(MainArena, 4);
                    I32ToString(&Metadata->TrackString, Metadata->Track);
                    
                    DeleteStringCompound(ScratchArena, &Track);
                }
            }
            if(~Metadata->FoundFlags & metadata_Year)
            {
                string_compound Year = {};
                JmpIter = MetadataID3v2_Helper(ScratchArena, Current+It, YEAR, &Year, MinorVersion);
                It += JmpIter;
                Assert(It < LoopSize);
                if(JmpIter > 0) Metadata->FoundFlags |= metadata_Year;
                if(Metadata->FoundFlags & metadata_Year)
                {
                    Metadata->Year = Clamp(ConvertU32FromString(&Year), 0, 9999);
                    Assert(Metadata->Year < 10000);
                    Metadata->YearString = NewStringCompound(MainArena, 4);
                    I32ToString(&Metadata->YearString, Metadata->Year);
                    DeleteStringCompound(ScratchArena, &Year);
                }
            }
            
#if 0 // Many duration tags are errounious and that causes some timeline problems. I rather always calculate the duration when the song is loaded the first time.
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
#endif
            if(FoundAllMetadata(Metadata->FoundFlags)) break;
        }
    }
    return DataLength;
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
        DecodeInfo->FileIDs.A.Count++;
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
        if(PlayingSong->DisplayableID >= 0 && PlayingSong->DecodeID == Result)
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
CreateSongDurationForMetadata(mp3_info *MP3Info, i32 MappedFileID, i32 DecodeID)
{
    mp3_metadata *MD = MP3Info->FileInfo.Metadata+MappedFileID;
    if(~MD->FoundFlags & metadata_Duration)
    {
        //mp3dec_file_info_t *DInfo = &MP3Info->DecodeInfo.DecodedData[DecodeID];
        mp3dec_file_info_t *DInfo = &MP3Info->DecodeInfo.PlayingDecoded.Data;
        
        MD->Duration = (u32)DInfo->samples/DInfo->channels/DInfo->hz*1000;
        MD->FoundFlags |= metadata_Duration;
    }
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
        FreeFileMemory(TransientArena, FileData);
    }
    
    return Result;
}

internal void // #ThreadedUse
CrawlFileForMetadata(arena_allocator *TransientArena, mp3_metadata *MD, string_c *FilePath, string_c FileName)
{
    u32 DataLength = ExtractMetadataSize(TransientArena, FilePath);
    read_file_result FileData = {};
    if(DataLength > 0 && ReadBeginningOfFile(TransientArena, &FileData, FilePath->S, DataLength)) 
    {
        MetadataID3v2_3(&GlobalGameState.JobThreadsArena, TransientArena, &FileData, MD);
        FreeFileMemory(TransientArena, FileData);
    }
    
    if(!FoundAllMetadata(MD->FoundFlags))
    {
        if(ReadEndOfFile(TransientArena, &FileData, FilePath->S, 128))
        {
            MetadataID3v1(&GlobalGameState.JobThreadsArena, &FileData, MD);
            FreeFileMemory(TransientArena, FileData);
        }
    }
    
    if(MD->Title.Pos == 0)
    {
        // If we did not get a title, we use the filename.
        FileName.Pos -= 4; // Local variable, so no problem!
        Assert(FileName.Pos > 0);
        MD->Title = NewStringCompound(&GlobalGameState.JobThreadsArena, FileName.Pos);
        AppendStringCompoundToCompound(&MD->Title, &FileName);
        MD->FoundFlags |= metadata_Title;
        DebugLog(255, "Title: %s\n", MD->Title.S);
    }
}

internal void // #ThreadedUse
CrawlFilesForMetadata(arena_allocator *TransientArena, mp3_file_info *FileInfo, u32 StartIt, u32 EndIt,
                      string_c *FolderPath, u32 *CurrentCrawlCount = 0)
{
    string_compound FilePath = NewStringCompound(TransientArena, 255);
    
    for(u32 It = StartIt; It < EndIt; ++It)
    {
        if(FoundAllMetadata(FileInfo->Metadata[It].FoundFlags)) continue;
        ConcatStringCompounds(4, &FilePath, FolderPath, FileInfo->SubPath+It, FileInfo->FileNames_+It);
        
        CrawlFileForMetadata(TransientArena, FileInfo->Metadata+It, &FilePath, FileInfo->FileNames_[It]);
        
        WipeStringCompound(&FilePath);
        if(CurrentCrawlCount)
        {
            ++(*CurrentCrawlCount);// = It;
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
    Batch->Song   = AllocateArray(Arena, BatchCount, array_playlist_id);
    Batch->Names  = AllocateArray(Arena, BatchCount, string_c);
    Batch->MaxBatches = BatchCount;
    Batch->BatchCount = 0;
}

inline void
ReallocateSortBatch(arena_allocator *Arena, sort_batch *Batch, u32 NewCount)
{
    DebugLog(50, "Reallocating %p from %i to %i\n", Batch, Batch->MaxBatches, NewCount);
    Batch->Genre  = ReallocateArray(Arena, Batch->Genre,  Batch->MaxBatches, NewCount, array_batch_id);
    Batch->Artist = ReallocateArray(Arena, Batch->Artist, Batch->MaxBatches, NewCount, array_batch_id);
    Batch->Album  = ReallocateArray(Arena, Batch->Album,  Batch->MaxBatches, NewCount, array_batch_id);
    Batch->Song   = ReallocateArray(Arena, Batch->Song,   Batch->MaxBatches, NewCount, array_playlist_id);
    Batch->Names  = ReallocateArray(Arena, Batch->Names,  Batch->MaxBatches, NewCount, string_c);
    Batch->MaxBatches = NewCount;
}

inline void
FreeSortBatch(arena_allocator *Arena, sort_batch *Batch)
{
    For(Batch->BatchCount)
    {
        if(Batch->Song[It].A.Length   > 0) DestroyArray(Arena, Batch->Song[It].A);
        if(Batch->Album[It].A.Length  > 0) DestroyArray(Arena, Batch->Album[It].A);
        if(Batch->Artist[It].A.Length > 0) DestroyArray(Arena, Batch->Artist[It].A);
        if(Batch->Genre[It].A.Length  > 0) DestroyArray(Arena, Batch->Genre[It].A);
    }
    FreeMemory(Arena, Batch->Genre);
    FreeMemory(Arena, Batch->Artist);
    FreeMemory(Arena, Batch->Album);
    FreeMemory(Arena, Batch->Song);
    FreeMemory(Arena, Batch->Names);
}

internal void
CreateMusicSortingInfo()
{
    mp3_info        *MP3Info      = GlobalGameState.MP3Info;
    mp3_file_info   *FileInfo     = &MP3Info->FileInfo;
    music_info      *MusicInfo    = MP3Info->MusicInfo;
    arena_allocator *FixArena     = &GlobalGameState.FixArena;
    arena_allocator *ScratchArena = &GlobalGameState.ScratchArena;
    
    array_file_id SongFileIDs = {};
    SongFileIDs.A = CreateArray(FixArena, FileInfo->Count_);
    For(FileInfo->Count_) Push(&SongFileIDs, NewFileID(It));
    
    sort_batch Genre  = {};
    InitializeSortBatch(FixArena, &Genre, 100);
    sort_batch Artist = {};
    InitializeSortBatch(FixArena, &Artist, 500);
    sort_batch Album  = {};
    InitializeSortBatch(FixArena, &Album, 1000);
    
    song_sort_info *SortBatchInfo = AllocateArray(ScratchArena, FileInfo->Count_, song_sort_info);
    
    u32 *Genre_CountForBatches  = AllocateArray(ScratchArena, Genre.MaxBatches, u32);
    u32 *Artist_CountForBatches = AllocateArray(ScratchArena, Artist.MaxBatches, u32);
    u32 *Album_CountForBatches  = AllocateArray(ScratchArena, Album.MaxBatches, u32);
    
    // NOTE:: Filling the first slot with the _no_genre/artist/album_ 
    string_c Empty = {};
    AddSongToSortBatch(FixArena, &Genre, &Empty);
    AddSongToSortBatch(FixArena, &Artist, &Empty);
    AddSongToSortBatch(FixArena, &Album, &Empty);
    
    For(SongFileIDs.A.Count, File)
    {
        i32 FileID = Get(&SongFileIDs.A, FileIt);
        mp3_metadata *MD = FileInfo->Metadata + FileID;
        
        // NOTE:: Check if sort_batches are full and realloc if necessary
        if(Genre.BatchCount  >= Genre.MaxBatches-1)  ReallocateSortBatch(FixArena, &Genre,  Genre.MaxBatches*2);
        if(Artist.BatchCount >= Artist.MaxBatches-1) ReallocateSortBatch(FixArena, &Artist, Artist.MaxBatches*2);
        if(Album.BatchCount  >= Album.MaxBatches-1)  ReallocateSortBatch(FixArena, &Album,  Album.MaxBatches*2);
        
        // NOTE:: Fills the Batches with all different genres, artists, albums and gives the batch id back
        i32 GenreID  = AddSongToSortBatch(FixArena, &Genre, &MD->Genre);
        i32 ArtistID = AddSongToSortBatch(FixArena, &Artist, &MD->Artist);
        i32 AlbumID  = AddSongToSortBatch(FixArena, &Album, &MD->Album);
        Assert(GenreID >= 0 && ArtistID >= 0 && AlbumID >= 0);
        
        // NOTE:: For each song note the corresponding IDs for the genre, artist, album
        SortBatchInfo[FileIt].GenreBatchID  = GenreID;
        SortBatchInfo[FileIt].ArtistBatchID = ArtistID;
        SortBatchInfo[FileIt].AlbumBatchID  = AlbumID;
        
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
        // TODO:: If I delete this, we fail in FillDisplayables... But why exactly
        // do I do this? Thats all too complicated and I did not properly comment it...
        if(It == 0) Album.Genre[It].A = CreateArray(FixArena, 100);
        else Album.Genre[It].A = CreateArray(FixArena, 10);
    }
    
    FreeMemory(ScratchArena, Genre_CountForBatches);
    FreeMemory(ScratchArena, Artist_CountForBatches);
    FreeMemory(ScratchArena, Album_CountForBatches);
    
    For(SongFileIDs.A.Count, File)
    {
        u32 GenreBatchID  = SortBatchInfo[FileIt].GenreBatchID;
        u32 ArtistBatchID = SortBatchInfo[FileIt].ArtistBatchID;
        u32 AlbumBatchID  = SortBatchInfo[FileIt].AlbumBatchID;
        
        PushIfNotExist(&Genre.Artist[GenreBatchID].A, ArtistBatchID);
        PushIfNotExist(&Genre.Album[GenreBatchID].A, AlbumBatchID);
        PushIfNotExist(&Genre.Song[GenreBatchID].A, FileIt);
        
        PushIfNotExist(&Artist.Album[ArtistBatchID].A, AlbumBatchID);
        PushIfNotExist(&Artist.Song[ArtistBatchID].A, FileIt);
        
        PushIfNotExist(&Album.Genre[AlbumBatchID].A, GenreBatchID);
        PushIfNotExist(&Album.Song[AlbumBatchID].A, FileIt);
    }
    
    // We Set MusicInfo->Playlists_ twice, because CreateEmpty will also set the
    // Playlists sorting info for the new playlist, which it will copy from the 
    // base playlist. That causes a problem when creating the base playlist 
    // (what we are doing right now) and therefore we create the sorting and then
    // set the partially done base playlist beforehand.
    MusicInfo->Playlist_ = MusicInfo->Playlists.List+0; 
    CreatePlaylistsSortingInfo(&MusicInfo->Playlist_->Playlists);
    MusicInfo->Playlist_ = CreateEmptyPlaylist(FixArena, MusicInfo, FileInfo->Count_, 
                                               Genre.BatchCount, Artist.BatchCount, Album.BatchCount);
    For(Genre.BatchCount)
    {
        Push(&MusicInfo->Playlist_->Genre.Displayable, NewPlaylistID(It));
    }
    
    MusicInfo->Playlist_->Genre.Batch  = Genre;
    MusicInfo->Playlist_->Artist.Batch = Artist;
    MusicInfo->Playlist_->Album.Batch  = Album;
    MusicInfo->Playlist_->Song.FileIDs = SongFileIDs;
    
    DebugLog(255, "GenreCount: %i, ArtistCount: %i, AlbumCount: %i\n", Genre.BatchCount, Artist.BatchCount, Album.BatchCount);
}

inline void
SetSelectionArray(music_display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID)
{
    // If no control key is pressed...
    if(!GlobalGameState.Input.Pressed[KEY_CONTROL_LEFT] && !GlobalGameState.Input.Pressed[KEY_CONTROL_RIGHT])
    {
        // ...and it is not selected or something else was selected clear it
        if(!IsSelected(DisplayColumn, PlaylistColumn, ColumnDisplayID) || PlaylistColumn->Selected.A.Count > 1)
        {
            ClearSelection(PlaylistColumn);
        }
    }
    ToggleSelection(DisplayColumn, PlaylistColumn, ColumnDisplayID);
}

inline void
SwitchSelection(music_display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID)
{
    ClearSelection(PlaylistColumn);
    Select(DisplayColumn, PlaylistColumn, ColumnDisplayID);
}

internal column_type // returns none if not in any rect, and corresponding column if it is
UpdateSelectionArray(playlist_column *PlaylistColumn, music_display_column *DisplayColumn)
{
    column_type Result = columnType_None;
    DisplayColumn->LastClickSlotID = -1;
    for(u32 It = 0; 
        It < PlaylistColumn->Displayable.A.Count && 
        It < DisplayColumn->Count;
        ++It)
    {
        if(IsInRect(DisplayColumn->BGRects[It], GlobalGameState.Input.MouseP))
        {
            Result = DisplayColumn->Type;
            DisplayColumn->LastClickSlotID = It;
            
            if(Result == columnType_Playlists) 
            {
                SwitchSelection(DisplayColumn, PlaylistColumn, It);
                SwitchPlaylistFromDisplayID(DisplayColumn, It);
            }
            else SetSelectionArray(DisplayColumn, PlaylistColumn, It);
            break;
        }
    }
    return Result;
}

internal column_type
SelectAllOrNothing(music_display_column *DisplayColumn, playlist_column *PlaylistColumn)
{
    column_type Result = columnType_None;
    b32 SelectAll = false;
    for(u32 It = 0; 
        It < PlaylistColumn->Displayable.A.Count && 
        It < DisplayColumn->Count;
        ++It)
    {
        if(IsInRect(DisplayColumn->BGRects[It], GlobalGameState.Input.MouseP))
        {
            if((i32)It == DisplayColumn->LastClickSlotID) // Clicked on same slot == is a proper double click
            {
                SelectAll = IsSelected(DisplayColumn, PlaylistColumn, It);
                
                Reset(&PlaylistColumn->Selected);
                For(PlaylistColumn->Displayable.A.Count, Select)
                {
                    SetSelection(DisplayColumn, PlaylistColumn, NewDisplayableID(SelectIt), SelectAll);
                }
            }
            else // Not a proper double click, do same as UpdateSelectionArray
            {
                SetSelectionArray(DisplayColumn, PlaylistColumn, It);
            }
            
            Result = DisplayColumn->Type;
            DisplayColumn->LastClickSlotID = It;
            break;
        }
    }
    
    return Result;
}

internal void
PropagateSelectionChange(music_info *MusicInfo)
{
    
    array_batch_id *Genre  = &MusicInfo->Playlist_->Genre.Selected;
    array_batch_id *Artist = &MusicInfo->Playlist_->Artist.Selected;
    array_batch_id *Album  = &MusicInfo->Playlist_->Album.Selected;
    
    if(Genre->A.Count > 0)
    {
        For(Artist->A.Count)
        {
            b32 Found = false;
            for(u32 GenreID = 0; GenreID < Genre->A.Count; GenreID++)
            {
                array_batch_id *GenreArtists = MusicInfo->Playlist_->Genre.Batch.Artist+Get(Genre, NewSelectID(GenreID)).ID;
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
                    array_batch_id *ArtistAlbums = MusicInfo->Playlist_->Artist.Batch.Album + Get(Artist, NewSelectID(ArtistID)).ID;
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
                    array_batch_id *GenreAlbums = MusicInfo->Playlist_->Genre.Batch.Album+ Get(Genre, NewSelectID(GenreID)).ID;
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
MergeDisplayArrays(array_playlist_id *A1, array_playlist_id *A2, u32 CheckValue)
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
PutAndCheck(array_playlist_id *Displayable, playlist_id ID, u32 CheckValue)
{
    Assert((i32)Displayable->A.Length > ID.ID);
    if(Displayable->A.Slot[ID.ID] == CheckValue) 
    {
        Put(&Displayable->A, ID.ID, ID.ID);
        Displayable->A.Count++;
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
FillDisplayables(music_info *MusicInfo, mp3_file_info *MP3FileInfo, music_display_info *DisplayInfo)
{
    playlist_info *Playlist = MusicInfo->Playlist_;
    // NOTE:: This may work now... I actually only do it for selected artist properly.
    // It should (most likely) be done with selected album as well! 
    b32 DoArtist = !DisplayInfo->Artist.Search.TextField.IsActive;
    b32 DoAlbum  = !DisplayInfo->Album.Search.TextField.IsActive;
    b32 DoSong   = !DisplayInfo->Song.Base.Search.TextField.IsActive;
    
    // For performance I use the displayable lists not as stacks in this procedure.
    // I fill them with a value they can never be, put the values to insert at the 
    // array position it itself points to (so duplicates are just written twice at 
    // the same location) and in the end remove the 'gaps' to re-stack-afy them.
    u32 CheckValue = MP3FileInfo->Count_+1;
    if(DoArtist) Clear(&Playlist->Artist.Displayable, CheckValue);
    if(DoAlbum)  Clear(&Playlist->Album.Displayable, CheckValue);
    if(DoSong)   Clear(&Playlist->Song.Displayable, CheckValue);
    
    For(Playlist->Genre.Selected.A.Count) // Going through Genres
    {
        batch_id BatchID = Get(&Playlist->Genre.Selected, NewSelectID(It));
        sort_batch *GenreBatch = &MusicInfo->Playlist_->Genre.Batch;
        
        if(DoArtist) MergeDisplayArrays(&Playlist->Artist.Displayable, GenreBatch->Artist+BatchID.ID, CheckValue);
        if(Playlist->Artist.Selected.A.Count == 0)
        {
            if(DoAlbum) MergeDisplayArrays(&Playlist->Album.Displayable, GenreBatch->Album+BatchID.ID, CheckValue);
            if(Playlist->Album.Selected.A.Count == 0)
            {
                if(DoSong) MergeDisplayArrays(&Playlist->Song.Displayable, GenreBatch->Song+BatchID.ID, CheckValue);
            }
        }
    }
    
    For(Playlist->Artist.Selected.A.Count) // Going through Artists
    {
        batch_id BatchID = Get(&Playlist->Artist.Selected, NewSelectID(It));
        sort_batch *ArtistBatch = &MusicInfo->Playlist_->Artist.Batch;
        
        // For every album in batch, look if it is one of the selected genres. if not, skip.
        For(ArtistBatch->Album[BatchID.ID].A.Count, Album)
        {
            sort_batch *AlbumBatch = &MusicInfo->Playlist_->Album.Batch;
            select_id AlbumSelectID = NewSelectID(AlbumIt);
            batch_id AlbumBatchID = Get(ArtistBatch->Album+BatchID.ID, AlbumSelectID);
            
            // TODO:: What am I doing here exactly? This defenitely needs to be cleaned up...
            For(AlbumBatch->Genre[AlbumBatchID.ID].A.Count, Genre)
            {
                if(StackContains(&Playlist->Genre.Selected, Get(AlbumBatch->Genre+AlbumBatchID.ID, NewSelectID(GenreIt))) ||
                   Playlist->Genre.Selected.A.Count == 0)
                {
                    if(DoAlbum) PutAndCheck(&Playlist->Album.Displayable, 
                                            Get(ArtistBatch->Album+BatchID.ID, AlbumSelectID), CheckValue);
                    // If no album is selected, fill song list from the verified album songs and
                    // check that _only_ songs which are also corresponding to the artist are inserted.
                    if(Playlist->Album.Selected.A.Count == 0)
                    {
                        For(AlbumBatch->Song[AlbumBatchID.ID].A.Count, Song)
                        {
                            playlist_id PlaylistID = Get(AlbumBatch->Song+AlbumBatchID.ID, NewSelectID(SongIt));
                            if(StackContains(ArtistBatch->Song+BatchID.ID, PlaylistID))
                            {
                                if(DoSong) PutAndCheck(&Playlist->Song.Displayable, PlaylistID, CheckValue);
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
        For(Playlist->Album.Selected.A.Count) // Going through Albums
        {
            batch_id BatchID = Get(&Playlist->Album.Selected, NewSelectID(It));
            
            MergeDisplayArrays(&Playlist->Song.Displayable, MusicInfo->Playlist_->Album.Batch.Song+BatchID.ID, CheckValue);
        }
    }
    
#if 0
    if(Playlist->Artist.Displayable.A.Count > 0) QuickSort3(Playlist->Artist.Displayable.A, true);
    if(Playlist->Album.Displayable.A.Count > 0) QuickSort3(Playlist->Album.Displayable.A, true);
    if(Playlist->Song.Displayable.A.Count > 0) QuickSort3(Playlist->Song.Displayable.A, true);
#else
    if(Playlist->Artist.Displayable.A.Count > 0) RemoveCheckValueFromArray(Playlist->Artist.Displayable.A, CheckValue);
    if(Playlist->Album.Displayable.A.Count > 0) RemoveCheckValueFromArray(Playlist->Album.Displayable.A, CheckValue);
    if(Playlist->Song.Displayable.A.Count > 0) RemoveCheckValueFromArray(Playlist->Song.Displayable.A, CheckValue);
#endif
    
    if(Playlist->Genre.Selected.A.Count == 0)
    {
        if(DoArtist) 
        {
            For(MusicInfo->Playlist_->Artist.Batch.BatchCount)
            {
                Push(&Playlist->Artist.Displayable, NewPlaylistID(It));
            }
        }
        if(Playlist->Artist.Selected.A.Count == 0)
        {
            if(DoAlbum) 
            {
                For(MusicInfo->Playlist_->Album.Batch.BatchCount)
                {
                    Push(&Playlist->Album.Displayable, NewPlaylistID(It));
                }
            }
            if(Playlist->Album.Selected.A.Count == 0)
            {
                if(DoSong) 
                {
                    For(Playlist->Song.FileIDs.A.Count)
                    {
                        Push(&Playlist->Song.Displayable, NewPlaylistID(It));
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
    playlist_info *Playlist         = MusicInfo->Playlist_;
    
    FillDisplayables(MusicInfo, &MP3Info->FileInfo, &MusicInfo->DisplayInfo);
    if(MusicInfo->IsShuffled) ShuffleStack(&Playlist->Song.Displayable);
    else                      SortDisplayables(MusicInfo, &MP3Info->FileInfo);
    
    UpdatePlayingSongForSelectionChange(MusicInfo);
    DisplayInfo->Song.Base.DisplayCursor = 0;
    
    displayable_id GenreDisplayID = NewDisplayableID(0);
    r32 SelectGenreStart = 0;
    displayable_id ArtistDisplayID = NewDisplayableID(0);
    r32 SelectArtistStart = 0;
    displayable_id AlbumDisplayID = NewDisplayableID(0);
    r32 SelectAlbumStart  = 0;
    switch(Type)
    {
        case columnType_Album:
        {
            AlbumDisplayID    = DisplayInfo->Album.OnScreenIDs[0];
            SelectAlbumStart  = GetLocalPosition(DisplayInfo->Album.BGRects[0]).y;
        } // #Through
        case columnType_Artist:
        {
            ArtistDisplayID   = DisplayInfo->Artist.OnScreenIDs[0];
            SelectArtistStart = GetLocalPosition(DisplayInfo->Artist.BGRects[0]).y; 
        } // #Through
        case columnType_Genre:
        {
            GenreDisplayID    = DisplayInfo->Genre.OnScreenIDs[0];
            SelectGenreStart  = GetLocalPosition(DisplayInfo->Genre.BGRects[0]).y;
        } break;
    }
    
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Genre, GenreDisplayID, SelectGenreStart);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Artist, ArtistDisplayID, SelectArtistStart);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Album, AlbumDisplayID, SelectAlbumStart);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Song.Base);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Playlists);
    
    UpdateSelectionColors(MusicInfo);
    UpdateVerticalSliders(MusicInfo);
}

internal void
SearchInDisplayable(music_info *MusicInfo, playlist_column *PlaylistColumn, search_bar *Search, mp3_file_info *FileInfo)
{
    array_playlist_id *Displayable = &PlaylistColumn->Displayable;
    Reset(Displayable);
    
    if(FileInfo) // Searching in Song Column
    {
        For(Search->InitialDisplayables.A.Count)
        {
            playlist_id PlaylistID   = Get(&Search->InitialDisplayables, NewDisplayableID(It));
            mp3_metadata *MD = GetMetadata(&MusicInfo->Playlist_->Song, FileInfo, PlaylistID);
            
            string_c *Name = &MD->Title;
            if(Name->Pos == 0) Name = GetSongFileName(&MusicInfo->Playlist_->Song, FileInfo, PlaylistID);
            if(ContainsAB_CaseInsensitive(Name, &Search->TextField.TextString))
            {
                Push(Displayable, PlaylistID);
            }
            else
            {
                Name = &MD->Artist;
                if(Name->Pos > 0 &&
                   ContainsAB_CaseInsensitive(Name, &Search->TextField.TextString))
                {
                    Push(Displayable, PlaylistID);
                }
                else
                {
                    Name = &MD->Album;
                    if(Name->Pos > 0 &&
                       ContainsAB_CaseInsensitive(Name, &Search->TextField.TextString))
                    {
                        Push(Displayable, PlaylistID);
                    }
                }
            }
        }
    }
    else // Searching in non-song column.
    {
        For(Search->InitialDisplayables.A.Count)
        {
            playlist_id PlaylistID = Get(&Search->InitialDisplayables, NewDisplayableID(It));
            if(ContainsAB_CaseInsensitive(MusicInfo->Playlist_->Columns[PlaylistColumn->Type].Batch.Names+PlaylistID.ID,
                                          &Search->TextField.TextString))
            {
                Push(Displayable, PlaylistID);
            }
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

struct sort_blob
{
    sort_batch    *Batch;
    array_playlist_id *Displayable;
};
internal b32
IsHigherInAlphabet(i32 T1, i32 T2, void *Data)
{
    b32 Result = false;
    
    sort_blob *Blob = (sort_blob *)Data;
    string_c *A = Blob->Batch->Names+Get(Blob->Displayable, NewDisplayableID(T1)).ID;
    string_c *B = Blob->Batch->Names+Get(Blob->Displayable, NewDisplayableID(T2)).ID;
    
    if(B->Pos > 0) 
    {
        i32 CompResult = CompareAB(A, B);
        Result = (CompResult >= 0);
    }
    
    return Result;
}

inline b32
MetadataCompare(mp3_metadata *A, mp3_metadata *B)
{
    b32 Result = false;
    
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
QuickSortSongColumn(i32 Low, i32 High, mp3_file_info *FileInfo, playlist_column *SongPlaylist)
{
    if(Low < High)
    {
        i32 Pivot   = High;
        i32 SmallID = (Low - 1);
        
        for(i32 HighID = Low; HighID <= High - 1; HighID++) 
        { 
            mp3_metadata *A = GetMetadata(SongPlaylist, FileInfo, NewPlaylistID(Get(&SongPlaylist->Displayable.A, HighID)));
            mp3_metadata *B = GetMetadata(SongPlaylist, FileInfo, NewPlaylistID(Get(&SongPlaylist->Displayable.A, Pivot)));
            if (MetadataCompare(A, B)) 
            { 
                SmallID++;
                Switch(&SongPlaylist->Displayable.A, SmallID, HighID); 
            } 
        } 
        Switch(&SongPlaylist->Displayable.A, SmallID+1, High); 
        i32 PartitionID = (SmallID + 1); 
        
        QuickSortSongColumn(Low, PartitionID - 1, FileInfo, SongPlaylist); 
        QuickSortSongColumn(PartitionID + 1, High, FileInfo, SongPlaylist); 
    }
}

internal void
SortDisplayables(music_info *MusicInfo, mp3_file_info *MP3FileInfo)
{
    array_playlist_id *Genre  = &MusicInfo->Playlist_->Genre.Displayable;
    array_playlist_id *Artist = &MusicInfo->Playlist_->Artist.Displayable;
    array_playlist_id *Album  = &MusicInfo->Playlist_->Album.Displayable;
    sort_blob GenreBlob  = {&MusicInfo->Playlist_->Genre.Batch, Genre};
    sort_blob ArtistBlob = {&MusicInfo->Playlist_->Artist.Batch, Artist};
    sort_blob AlbumBlob  = {&MusicInfo->Playlist_->Album.Batch, Album};
    QuickSort(0, Genre->A.Count-1,  Genre,  {IsHigherInAlphabet, &GenreBlob});
    QuickSort(0, Artist->A.Count-1, Artist, {IsHigherInAlphabet, &ArtistBlob});
    QuickSort(0, Album->A.Count-1,  Album,  {IsHigherInAlphabet, &AlbumBlob});
    
    RestartTimer("SortSongs");
    // TODO:: This is a f*cking stupid hack... Why is it so slow
    // for switching in a playlist, but not when switching to the
    // 'main' playlist...
    ShuffleStack(&MusicInfo->Playlist_->Song.Displayable);
    QuickSortSongColumn(0, MusicInfo->Playlist_->Song.Displayable.A.Count-1, MP3FileInfo, &MusicInfo->Playlist_->Song);
    SnapTimer("SortSongs");
}

internal void
FinishChangeEntireSong(playing_song *Song)
{
    mp3dec_file_info_t *DInfo = &GlobalGameState.MP3Info->DecodeInfo.PlayingDecoded.Data;
    PushSongLoadingComplete(GlobalGameState.SoundThreadInterface, DInfo);
}

internal void
FinishChangeSong(game_state *GameState, playing_song *Song)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    GameState->MusicInfo.CurrentlyChangingSong = false;
    
    playlist_id PlaylistID = GetNextSong(MusicInfo);
    
    mp3dec_file_info_t *DInfo = &GameState->MP3Info->DecodeInfo.DecodedData[Song->DecodeID];
    Assert(DInfo->layer == 3);
    
    PushSoundBufferClear(GameState->SoundThreadInterface);
    PushSongChanged(GameState->SoundThreadInterface, DInfo);
    
    UpdatePlayingSongColor(&MusicInfo->DisplayInfo.Song.Base, &MusicInfo->Playlist_->Song, PlaylistID, 
                           &MusicInfo->DisplayInfo.Song.Base.Base->ColorPalette.PlayingSong);
    
    
    mp3_metadata *MD = GetMetadata(&GameState->MusicInfo.Playlist_->Song, &GameState->MP3Info->FileInfo, PlaylistID);
    DebugLog(1255, "Nr.%i: %s (%s) by %s \n%i - %s - %i - %i Hz\n", MD->Track, MD->Title.S, MD->Album.S, MD->Artist.S, MD->Year, MD->Genre.S, MD->Duration, DInfo->hz);
    
    char WinText[512];
    Assert(MD->Title.Pos > 0);
    if(MD->Artist.Pos > 0) sprintf_s(WinText, "%s (%s)\n", MD->Title.S, MD->Artist.S);
    else                   sprintf_s(WinText, "%s\n", MD->Title.S);
    
    string_w WWinText = {};
    ConvertString8To16(&GameState->ScratchArena, (u8 *)WinText, &WWinText);
    SetWindowTextW(GameState->Renderer.Window.WindowHandle, WWinText.S);
}

internal void
ChangeSong(game_state *GameState, playing_song *Song)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    
    playlist_id PlaylistID = GetNextSong(MusicInfo);
    
    if(PlaylistID >= 0)
    {
        GameState->MusicInfo.CurrentlyChangingSong = true;
        
        Song->DecodeID = AddJob_LoadNewPlayingSong(&GameState->JobQueue, PlaylistID);
        
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
    
    SetTheNewPlayingSong(&GameState->Renderer, &GameState->MusicInfo.DisplayInfo.PlayingSongPanel, &GameState->Layout, MusicInfo);
}

inline void
HandleChangeToNextSong(game_state *GameState)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    SetNextSong(MusicInfo);
    ChangeSong(GameState, &MusicInfo->PlayingSong);
    if(MusicInfo->PlayingSong.DisplayableID >= 0) KeepPlayingSongOnScreen(&GameState->Renderer, MusicInfo);
    AddJob_NextUndecodedInPlaylist();
}

internal void
ApplyNewMetadata(game_state *GameState, music_info *MusicInfo)
{
    MusicInfo->UpNextList.A = CreateArray(&GameState->FixArena, 200);
    
    MusicInfo->PlayingSong = {-1, -1, -1, 0};
    CreateMusicSortingInfo();
    FillDisplayables(MusicInfo, &GameState->MP3Info->FileInfo, &MusicInfo->DisplayInfo);
    SortDisplayables(MusicInfo, &GameState->MP3Info->FileInfo);
    UpdatePlayingSongForSelectionChange(MusicInfo);
    UpdateAllDisplayColumns(GameState);
    SaveMP3LibraryFile(GameState, GameState->MP3Info);
    
    MusicInfo->DisplayInfo.Genre.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Genre.Displayable.A.Length);
    MusicInfo->DisplayInfo.Artist.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Artist.Displayable.A.Length);
    MusicInfo->DisplayInfo.Album.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Album.Displayable.A.Length);
    MusicInfo->DisplayInfo.Song.Base.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Song.Displayable.A.Length);
    
    MusicInfo->DisplayInfo.Genre.DisplayCursor = 0;
    MusicInfo->DisplayInfo.Artist.DisplayCursor = 0;
    MusicInfo->DisplayInfo.Album.DisplayCursor = 0;
    MusicInfo->DisplayInfo.Song.Base.DisplayCursor = 0;
    
    UpdateVerticalSliders(MusicInfo);
    UpdateHorizontalSliders(MusicInfo);
    
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
    InvalidCodePath;
    //CrawlFilesForMetadata(&GameState->ScratchArena, &GameState->MP3Info->FileInfo, &GameState->MP3Info->FolderPath);
    
    ApplyNewMetadata(GameState, MusicInfo);
    return Result;
}


// *****************************************************************************
// New Playlist stuff **********************************************************
// *****************************************************************************

internal void
CreatePlaylistsSortingInfo(playlist_column *Playlists)
{
    Playlists->Type             = columnType_Playlists;
    Playlists->Selected.A       = CreateArray(&GlobalGameState.FixArena, 250);
    Playlists->Displayable.A    = CreateArray(&GlobalGameState.FixArena, 250);
    
    Playlists->Batch.Names      = AllocateArray(&GlobalGameState.FixArena, 250, string_c);
    Playlists->Batch.MaxBatches = 250;
    
    
    Push(&Playlists->Displayable, {0});
    string_c *Name = Playlists->Batch.Names+Playlists->Batch.BatchCount++;
    *Name = NewStringCompound(&GlobalGameState.FixArena, 4+8);
    AppendStringToCompound(Name, (u8 *)"All (");
    I32ToString(Name, GlobalGameState.MP3Info->FileInfo.Count_);
    AppendCharToCompound(Name, ')');
}

internal playlist_info *
CreateEmptyPlaylist(arena_allocator *Arena, music_info *MusicInfo, i32 SongIDCount/*default -1*/, i32 GenreBatchCount/*default -1*/, i32 ArtistBatchCount/*default -1*/, i32 AlbumBatchCount/*default -1*/)
{
    Assert(MusicInfo->Playlists.Count < MusicInfo->Playlists.MaxCount);
    playlist_info *Playlist = MusicInfo->Playlists.List+MusicInfo->Playlists.Count++;
    
    // First entry in Playlists.List is the 'everything' list.
    if(GenreBatchCount < 0)  GenreBatchCount  = MusicInfo->Playlists.List[0].Genre.Batch.BatchCount; 
    if(ArtistBatchCount < 0) ArtistBatchCount = MusicInfo->Playlists.List[0].Artist.Batch.BatchCount;
    if(AlbumBatchCount < 0)  AlbumBatchCount  = MusicInfo->Playlists.List[0].Album.Batch.BatchCount;
    if(SongIDCount < 0)      SongIDCount      = MusicInfo->Playlists.List[0].Song.FileIDs.A.Count;
    
    Playlist->Genre.Type    = columnType_Genre;
    Playlist->Artist.Type   = columnType_Artist;
    Playlist->Album.Type    = columnType_Album;
    Playlist->Song.Type     = columnType_Song;
    
    Playlist->Genre.Selected.A  = CreateArray(Arena, GenreBatchCount);
    Playlist->Artist.Selected.A = CreateArray(Arena, ArtistBatchCount);
    Playlist->Album.Selected.A  = CreateArray(Arena, AlbumBatchCount);
    Playlist->Song.Selected.A   = CreateArray(Arena, SongIDCount);
    
    Playlist->Genre.Displayable.A  = CreateArray(Arena, GenreBatchCount);
    Playlist->Artist.Displayable.A = CreateArray(Arena, ArtistBatchCount);
    Playlist->Album.Displayable.A  = CreateArray(Arena, AlbumBatchCount);
    Playlist->Song.Displayable.A   = CreateArray(Arena, SongIDCount);
    
    Playlist->Playlists = MusicInfo->Playlist_->Playlists;
    
    return Playlist;
}

internal void
FillPlaylistWithFileIDs(music_info *MusicInfo, mp3_file_info *FileInfo, playlist_info *NewPlaylist, array_file_id FileIDs)
{
    // TODO:: Copy pasta from CreateMusicSortingInfo and FillPlaylistWithCurrentSelection
    // TODO:: Copy pasta from CreateMusicSortingInfo and FillPlaylistWithCurrentSelection
    // TODO:: Copy pasta from CreateMusicSortingInfo and FillPlaylistWithCurrentSelection
    
    // NOTE:: We look only at the Song column and put everything 
    // in the other colums based on what we find there.
    
    playlist_info   *Playlist     = MusicInfo->Playlist_;
    arena_allocator *FixArena     = &GlobalGameState.FixArena;
    arena_allocator *ScratchArena = &GlobalGameState.ScratchArena;
    
    sort_batch Genre  = {};
    InitializeSortBatch(FixArena, &Genre,  Playlist->Genre.Batch.BatchCount);
    sort_batch Artist = {};
    InitializeSortBatch(FixArena, &Artist, Playlist->Artist.Batch.BatchCount);
    sort_batch Album  = {};
    InitializeSortBatch(FixArena, &Album,  Playlist->Album.Batch.BatchCount);
    
    song_sort_info *SortBatchInfo = AllocateArray(ScratchArena, FileIDs.A.Count, song_sort_info);
    u32 *Genre_CountForBatches    = AllocateArray(ScratchArena, Genre.MaxBatches, u32);
    u32 *Artist_CountForBatches   = AllocateArray(ScratchArena, Artist.MaxBatches, u32);
    u32 *Album_CountForBatches    = AllocateArray(ScratchArena, Album.MaxBatches, u32);
    
    For(FileIDs.A.Count)
    {
        mp3_metadata *MD = FileInfo->Metadata + Get(&FileIDs.A, It);
        
        // NOTE:: Fills the Batches with all different genres, artists, albums and gives the batch id back
        i32 GenreID  = AddSongToSortBatch(FixArena, &Genre, &MD->Genre);
        i32 ArtistID = AddSongToSortBatch(FixArena, &Artist, &MD->Artist);
        i32 AlbumID  = AddSongToSortBatch(FixArena, &Album, &MD->Album);
        Assert(GenreID >= 0 && ArtistID >= 0 && AlbumID >= 0);
        
        // NOTE:: For each song note the corresponding IDs for the genre, artist, album
        SortBatchInfo[It].GenreBatchID  = GenreID;
        SortBatchInfo[It].ArtistBatchID = ArtistID;
        SortBatchInfo[It].AlbumBatchID  = AlbumID;
        
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
    }
    
    FreeMemory(ScratchArena, Genre_CountForBatches);
    FreeMemory(ScratchArena, Artist_CountForBatches);
    FreeMemory(ScratchArena, Album_CountForBatches);
    
    For(FileIDs.A.Count)
    {
        u32 GenreBatchID  = SortBatchInfo[It].GenreBatchID;
        u32 ArtistBatchID = SortBatchInfo[It].ArtistBatchID;
        u32 AlbumBatchID  = SortBatchInfo[It].AlbumBatchID;
        
        PushIfNotExist(&Genre.Artist[GenreBatchID].A, ArtistBatchID);
        PushIfNotExist(&Genre.Album[GenreBatchID].A, AlbumBatchID);
        PushIfNotExist(&Genre.Song[GenreBatchID].A, It);
        
        PushIfNotExist(&Artist.Album[ArtistBatchID].A, AlbumBatchID);
        PushIfNotExist(&Artist.Song[ArtistBatchID].A, It);
        
        PushIfNotExist(&Album.Song[AlbumBatchID].A, It);
    }
    
    
    For(Genre.BatchCount)  Push(&NewPlaylist->Genre.Displayable, NewPlaylistID(It));
    For(Artist.BatchCount) Push(&NewPlaylist->Artist.Displayable, NewPlaylistID(It));
    For(Album.BatchCount)  Push(&NewPlaylist->Album.Displayable, NewPlaylistID(It));
    
    if(NewPlaylist->Genre.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Genre.Batch);
    if(NewPlaylist->Artist.Batch.BatchCount > 0) FreeSortBatch(FixArena, &NewPlaylist->Artist.Batch);
    if(NewPlaylist->Album.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Album.Batch);
    if(NewPlaylist->Song.FileIDs.A.Count    > 0) DestroyArray(FixArena, NewPlaylist->Song.FileIDs.A);
    
    // Now fill playlist_colum song
    NewPlaylist->Song.FileIDs.A = CreateArray(FixArena, FileIDs.A.Count); // TODO:: Think about making it the max size during creation.
    For(FileIDs.A.Count)
    {
        Push(&NewPlaylist->Song.Displayable.A, It);
        Push(&NewPlaylist->Song.FileIDs.A, Get(&FileIDs.A, It));
    }
    
    NewPlaylist->Genre.Batch  = Genre;
    NewPlaylist->Artist.Batch = Artist;
    NewPlaylist->Album.Batch  = Album;
    
}

internal void
FillPlaylistWithCurrentSelection(music_info *MusicInfo, mp3_file_info *FileInfo, playlist_info *NewPlaylist)
{
    // TODO:: Copy pasta from CreateMusicSortingInfo
    // TODO:: Copy pasta from CreateMusicSortingInfo
    // TODO:: Copy pasta from CreateMusicSortingInfo
    
    // NOTE:: We look only at the Song column and put everything 
    // in the other colums based on what we find there.
    
    playlist_info   *Playlist     = MusicInfo->Playlist_;
    arena_allocator *FixArena     = &GlobalGameState.FixArena;
    arena_allocator *ScratchArena = &GlobalGameState.ScratchArena;
    
    sort_batch Genre  = {};
    InitializeSortBatch(FixArena, &Genre,  Playlist->Genre.Batch.BatchCount);
    sort_batch Artist = {};
    InitializeSortBatch(FixArena, &Artist, Playlist->Artist.Batch.BatchCount);
    sort_batch Album  = {};
    InitializeSortBatch(FixArena, &Album,  Playlist->Album.Batch.BatchCount);
    
    song_sort_info *SortBatchInfo = AllocateArray(ScratchArena, Playlist->Song.Displayable.A.Count, song_sort_info);
    u32 *Genre_CountForBatches    = AllocateArray(ScratchArena, Genre.MaxBatches, u32);
    u32 *Artist_CountForBatches   = AllocateArray(ScratchArena, Artist.MaxBatches, u32);
    u32 *Album_CountForBatches    = AllocateArray(ScratchArena, Album.MaxBatches, u32);
    
    For(Playlist->Song.Displayable.A.Count)
    {
        mp3_metadata *MD = GetMetadata(&Playlist->Song, FileInfo, NewDisplayableID(It));
        
        // NOTE:: Fills the Batches with all different genres, artists, albums and gives the batch id back
        i32 GenreID  = AddSongToSortBatch(FixArena, &Genre, &MD->Genre);
        i32 ArtistID = AddSongToSortBatch(FixArena, &Artist, &MD->Artist);
        i32 AlbumID  = AddSongToSortBatch(FixArena, &Album, &MD->Album);
        Assert(GenreID >= 0 && ArtistID >= 0 && AlbumID >= 0);
        
        // NOTE:: For each song note the corresponding IDs for the genre, artist, album
        SortBatchInfo[It].GenreBatchID  = GenreID;
        SortBatchInfo[It].ArtistBatchID = ArtistID;
        SortBatchInfo[It].AlbumBatchID  = AlbumID;
        
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
    }
    
    FreeMemory(ScratchArena, Genre_CountForBatches);
    FreeMemory(ScratchArena, Artist_CountForBatches);
    FreeMemory(ScratchArena, Album_CountForBatches);
    
    For(Playlist->Song.Displayable.A.Count)
    {
        u32 GenreBatchID  = SortBatchInfo[It].GenreBatchID;
        u32 ArtistBatchID = SortBatchInfo[It].ArtistBatchID;
        u32 AlbumBatchID  = SortBatchInfo[It].AlbumBatchID;
        
        PushIfNotExist(&Genre.Artist[GenreBatchID].A, ArtistBatchID);
        PushIfNotExist(&Genre.Album[GenreBatchID].A, AlbumBatchID);
        PushIfNotExist(&Genre.Song[GenreBatchID].A, It);
        
        PushIfNotExist(&Artist.Album[ArtistBatchID].A, AlbumBatchID);
        PushIfNotExist(&Artist.Song[ArtistBatchID].A, It);
        
        PushIfNotExist(&Album.Song[AlbumBatchID].A, It);
    }
    
    
    For(Genre.BatchCount)  Push(&NewPlaylist->Genre.Displayable, NewPlaylistID(It));
    For(Artist.BatchCount) Push(&NewPlaylist->Artist.Displayable, NewPlaylistID(It));
    For(Album.BatchCount)  Push(&NewPlaylist->Album.Displayable, NewPlaylistID(It));
    
    if(NewPlaylist->Genre.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Genre.Batch);
    if(NewPlaylist->Artist.Batch.BatchCount > 0) FreeSortBatch(FixArena, &NewPlaylist->Artist.Batch);
    if(NewPlaylist->Album.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Album.Batch);
    if(NewPlaylist->Song.FileIDs.A.Count    > 0) DestroyArray(FixArena, NewPlaylist->Song.FileIDs.A);
    
    // Now fill playlist_colum song
    NewPlaylist->Song.FileIDs.A = CreateArray(FixArena, Playlist->Song.Displayable.A.Count); // TODO:: Think about making it the max size during creation.
    For(Playlist->Song.Displayable.A.Count)
    {
        Push(&NewPlaylist->Song.Displayable.A,                            Get(&Playlist->Song.Displayable.A, It));
        Push(&NewPlaylist->Song.FileIDs.A, Get(&Playlist->Song.FileIDs.A, Get(&Playlist->Song.Displayable.A, It)));
    }
    
    NewPlaylist->Genre.Batch  = Genre;
    NewPlaylist->Artist.Batch = Artist;
    NewPlaylist->Album.Batch  = Album;
}

inline i32
GetPlaylistID(music_info *MusicInfo, playlist_info *Playlist)
{
    i32 Result = -1;
    
    For(MusicInfo->Playlists.Count)
    {
        if(Playlist == MusicInfo->Playlists.List + It) // Pointer compare
        {
            Result = It;
            break;
        }
    }
    Assert(Result >= 0);
    
    return Result;
}

inline void
AddPlaylistToColumn(music_info *MusicInfo, playlist_id PlaylistID, string_c PlaylistName)
{
    Assert(PlaylistID >= 0);
    
    playlist_column *Playlists  = &MusicInfo->Playlist_->Playlists;
    
    Assert(PlaylistID == Playlists->Batch.BatchCount);
    Assert(Playlists->Batch.BatchCount < Playlists->Batch.MaxBatches);
    Push(&Playlists->Displayable, PlaylistID);
    Playlists->Batch.Names[Playlists->Batch.BatchCount] = NewStringCompound(&GlobalGameState.FixArena, PlaylistName.Pos);
    AppendStringCompoundToCompound(Playlists->Batch.Names+Playlists->Batch.BatchCount++, &PlaylistName);
}

inline void
AddPlaylistToColumn(music_info *MusicInfo, playlist_info *Playlist, string_c PlaylistName)
{
    playlist_id PlaylistID = NewPlaylistID(GetPlaylistID(MusicInfo, Playlist));
    AddPlaylistToColumn(MusicInfo, PlaylistID, PlaylistName);
}

internal void 
SwitchPlaylistFromDisplayID(music_display_column *DisplayColumn, u32 ColumnDisplayID)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    
    playlist_id PlaylistID = Get(&MusicInfo->Playlist_->Playlists.Displayable, DisplayColumn->OnScreenIDs[ColumnDisplayID]);
    
    SyncPlaylists_playlist_column(MusicInfo); // We need to sync before switching to the new playlist.
    
    MusicInfo->Playlist_ = MusicInfo->Playlists.List + PlaylistID.ID;
    MusicInfo->PlayingSong.DisplayableID.ID = -1;
    MusicInfo->PlayingSong.PlaylistID.ID    = -1;
    MusicInfo->PlayingSong.DecodeID         = -1;
}

inline void
SyncPlaylists_playlist_column(music_info *MusicInfo)
{
    For(MusicInfo->Playlists.Count)
    {
        MusicInfo->Playlists.List[It].Playlists = MusicInfo->Playlist_->Playlists;
    }
}






