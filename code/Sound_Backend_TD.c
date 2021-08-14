#include "Sound_Backend_TD.h"
#include "GameBasics_TD.h"
internal u32 CreateHash(string_c Name, u64 CreationDate);
internal void SavePlaylist(game_state *GS, playlist_info *Playlist);
internal void DeletePlaylist(game_state *GS, playlist_info *Playlist);
internal void AllocatePlaylist(arena_allocator *Arena, playlist_info *Playlist, i32 SongIDCount, i32 GenreBatchCount, i32 ArtistBatchCount, i32 AlbumBatchCount);
internal void UpdatePlaylistScreenName(game_state *GS, playlist_info *Playlist);
internal void LoadAllPlaylists(game_state *GS);
internal void RemovePlaylist(game_state *GS, playlist_info *Playlist, b32 DeleteSaveFile);
inline i32 GetPlaylistID(music_info *MusicInfo, playlist_info *Playlist);
internal playlist_info *GetPlaylist(game_state *GS, string_c PlaylistName);
internal void SwitchPlaylist(game_state *GS, playlist_info *Playlist);

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

inline string_c *
GetSongSubPath(playlist_column *SongColumn, mp3_file_info *FileInfo, playlist_id PlaylistID)
{
    string_c *Result = 0;
    Assert(PlaylistID >= 0);
    Assert(SongColumn->Type == columnType_Song);
    
    Result = FileInfo->SubPath + Get(&SongColumn->FileIDs.A, PlaylistID.ID);
    
    return Result;
}

inline file_id
GetFileID(playlist_column *SongColumn, playlist_id PlaylistID)
{
    Assert(SongColumn->Type == columnType_Song);
    Assert(PlaylistID >= 0);
    file_id Result = NewFileID(Get(&SongColumn->FileIDs.A, PlaylistID.ID));
    return Result;
}

inline i32
GetOnScreenID(display_column *DisplayColumn, displayable_id DisplayableID)
{
    i32 Result = -1;
    
    For(DisplayColumn->Count)
    {
        if(DisplayColumn->OnScreenIDs[It] == DisplayableID)
        {
            Result = It;
            break;
        }
    }
    
    return Result;
}

internal i32
GetOnScreenID(game_state *GS, column_type Type, displayable_id DisplayableID)
{
    i32 Result = -1;
    
    display_column *DisplayColumn = 0;
    switch(Type)
    {
        case columnType_Playlists:
        {
            DisplayColumn = &GS->MusicInfo.DisplayInfo.Playlists;
        } break;
        case columnType_Genre:
        {
            DisplayColumn = &GS->MusicInfo.DisplayInfo.Genre;
        } break;
        case columnType_Artist:
        {
            DisplayColumn = &GS->MusicInfo.DisplayInfo.Artist;
        } break;
        case columnType_Album:
        {
            DisplayColumn = &GS->MusicInfo.DisplayInfo.Album;
        } break;
        case columnType_Song:
        {
            DisplayColumn = &GS->MusicInfo.DisplayInfo.Song.Base;
        } break;
        InvalidDefaultCase;
    }
    
    For(DisplayColumn->Count)
    {
        if(DisplayColumn->OnScreenIDs[It] == DisplayableID)
        {
            Result = It;
            break;
        }
    }
    
    return Result;
}

inline string_c *
GetName(playlist_column *PlaylistColumn, displayable_id DID)
{
    string_c *Result = NULL;
    
    Assert(PlaylistColumn->Type != columnType_Song);
    Assert(DID < PlaylistColumn->Displayable.A.Count);
    
    Result = PlaylistColumn->Batch.Names + Get(&PlaylistColumn->Displayable, DID).ID;
    
    return Result;
}

inline string_c *
GetName(game_state *GS, playlist_column *PlaylistColumn, playlist_id PLID)
{
    string_c *Result = NULL;
    
    if(PlaylistColumn->Type == columnType_Song)
    {
        Result = &GS->MP3Info->FileInfo.Metadata[Get(&PlaylistColumn->FileIDs, PLID).ID ].Title;
    }
    else
    {
        Result = PlaylistColumn->Batch.Names + PLID.ID;
    }
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
    Result->DecodeInfo.FileIDs.A   = CreateArray(Arena, MAX_MP3_DECODE_COUNT);
    Result->DecodeInfo.LastTouched = CreateArray(Arena, MAX_MP3_DECODE_COUNT);
    
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

inline displayable_id
GetDisplayableID(array_playlist_id *Displayable, playlist_id PlaylistID)
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
GetDisplayableID(music_info *MusicInfo, playlist_id PlaylistID)
{
    array_playlist_id *Displayable = &MusicInfo->Playlist_->Song.Displayable;
    return GetDisplayableID(Displayable, PlaylistID);
}

inline playlist_id
GetPlaylistID(playlist_column *SongColumn, file_id FileID)
{
    playlist_id Result = NewPlaylistID(-1);
    StackFind(&SongColumn->FileIDs, FileID, &Result.ID);
    return Result;
}

inline playlist_id
GetPlaylistID(playlist_column *PLColumn, displayable_id DID)
{
    playlist_id Result = NewPlaylistID(-1);
    Result = Get(&PLColumn->Displayable, DID);
    return Result;
}

inline playlist_id
GetPlaylistID(music_info *MusicInfo, displayable_id DisplayableID)
{
    playlist_id PlaylistID = NewPlaylistID(-1);
    if(DisplayableID >= 0) PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, DisplayableID);
    return PlaylistID;
}

inline file_id
GetFileID(mp3_file_info *FileInfo, string_c *SubPath, string_c *Filename, file_id StartID = {0})
{
    file_id Result = NewFileID(-1);
    for(u32 It = StartID.ID; It < FileInfo->Count_; ++It)
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

inline file_id
GetFirstFileID(mp3_file_info *FileInfo, string_c *SubPath)
{
    file_id Result = NewFileID(-1);
    For(FileInfo->Count_)
    {
        if(CompareStringCompounds(SubPath, FileInfo->SubPath+It))
        {
            Result.ID = It;
            break;
        }
    }
    return Result;
}

inline file_id
GetFileID(music_info *MusicInfo, displayable_id DisplayableID)
{
    file_id FileID = NewFileID(-1);
    if(DisplayableID >= 0) {
        playlist_id PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, DisplayableID);
        FileID = NewFileID(Get(&MusicInfo->Playlist_->Song.FileIDs.A, PlaylistID.ID));
    }
    return FileID;
}

inline displayable_id
GetDisplayableID(music_info *MusicInfo, u32 OnScreenID, playlist_id *PlaylistID)
{
    displayable_id DisplayableID = NewDisplayableID(-1);
    playlist_id ID = Get(&MusicInfo->Playlist_->Song.Displayable, MusicInfo->DisplayInfo.Song.Base.OnScreenIDs[OnScreenID]);
    StackFind(&MusicInfo->Playlist_->Song.Displayable, ID, &DisplayableID.ID);
    
    if(PlaylistID) *PlaylistID = ID;
    return DisplayableID;
}

// Acquires the corresponding Genre/Artist/Album DisplayID for given FileID
inline displayable_id
PlaylistIDToColumnDisplayID(music_info *MusicInfo, display_column *DisplayColumn, playlist_id PlaylistID)
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
            return GetDisplayableID(&MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable, PlaylistID);
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
SortingIDToColumnDisplayID(playlist_info *Playlist, display_column *DisplayColumn, batch_id BatchID)
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
UpdatePlayingSong(music_info *MusicInfo)
{
    // If the currently playing song is in the new displayable list set DisplayableID.
    // If not, the music stops.
    if(MusicInfo->PlayingSong.PlaylistID < 0)    return;
    if(MusicInfo->PlayingSong.DisplayableID < 0) return;
    
    displayable_id DisplayableID = GetDisplayableID(MusicInfo, MusicInfo->PlayingSong.PlaylistID);
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
    else PlaylistID = GetPlaylistID(MusicInfo, MusicInfo->PlayingSong.DisplayableID);
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
    play_loop Looping = MusicInfo->Playlist_->Looping;
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
                PlayingSong->PlaylistID = GetPlaylistID(MusicInfo, PlayingSong->DisplayableID);
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
        PlayingSong->PlaylistID = GetPlaylistID(MusicInfo, PlayingSong->DisplayableID);
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
    play_loop Looping = MusicInfo->Playlist_->Looping;
    playing_song *PlayingSong = &MusicInfo->PlayingSong;
    Assert(PlayingSong->DisplayableID >= -1);
    
    if(PlayingSong->DisplayableID != -1)
    {
        if(Looping == playLoop_Repeat) ;
        else if(Looping == playLoop_Loop) 
        {
            PlayingSong->DisplayableID.ID -= 1;
            PlayingSong->PlaylistID = GetPlaylistID(MusicInfo, PlayingSong->DisplayableID);
            if(PlayingSong->DisplayableID < 0) 
            {
                PlayingSong->DisplayableID.ID = MusicInfo->Playlist_->Song.Displayable.A.Count-1;
                PlayingSong->PlaylistID = GetPlaylistID(MusicInfo, PlayingSong->DisplayableID);
            }
        }
        else 
        {
            PlayingSong->DisplayableID.ID -= 1;
            PlayingSong->PlaylistID = GetPlaylistID(MusicInfo, PlayingSong->DisplayableID);
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
            DebugLog(255, "ERROR:: Could not load %s metadata. Tag size was %i, must be erroneous !\n", Frame, Length);
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

internal void
ResetAllDecodedIDs(game_state *GS)
{
    mp3_decode_info *DecodeInfo = &GS->MP3Info->DecodeInfo;
    if(DecodeInfo->PlayingDecoded.CurrentlyDecoding)
    {
        DebugLog(235, "NOTE:: Canceled previous loading of song to load the new playing song!\n");
        
        DecodeInfo->CancelDecoding = true;
        while(DecodeInfo->PlayingDecoded.CurrentlyDecoding) Sleep(1); // Loop 1 ms between checks. Seems fine for now.
        DecodeInfo->CancelDecoding = false;
    }
    
    For(DecodeInfo->Count)
    {
        Put(&DecodeInfo->FileIDs.A, It, MAX_UINT32);
    }
}

internal i32
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
    else                    sprintf_s(B, "%i:", Floor(Minutes));
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
        if(Batch->Song[It].A.Length   > 0) DestroyArray(Arena, &Batch->Song[It].A);
        if(Batch->Album[It].A.Length  > 0) DestroyArray(Arena, &Batch->Album[It].A);
        if(Batch->Artist[It].A.Length > 0) DestroyArray(Arena, &Batch->Artist[It].A);
        if(Batch->Genre[It].A.Length  > 0) DestroyArray(Arena, &Batch->Genre[It].A);
    }
    FreeMemory(Arena, Batch->Genre);
    FreeMemory(Arena, Batch->Artist);
    FreeMemory(Arena, Batch->Album);
    FreeMemory(Arena, Batch->Song);
    FreeMemory(Arena, Batch->Names);
}

internal void
CreateMusicSortingInfo(playlist_info *Playlist, b32 IsFirstInitialization = false)
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
        // Every album has at least one genre associated with it. If the metadata
        // did not have an entry for genre, the empty genre will be assigned to it.
        // As the first album (index 0) is the empty album, it can have actually 
        // many genres, thats why it has a higher limit (should be done dynamically
        // at some point).
        if(It == 0) Album.Genre[It].A = CreateArray(FixArena, 100); // @HardLimit
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
    
    Assert(Playlist);
    if(IsFirstInitialization)
    {
        // We Set MusicInfo->Playlists_ twice, because CreateEmpty will also set the
        // Playlists sorting info for the new playlist, which it will copy from the 
        // base playlist. That causes a problem when creating the base playlist 
        // (what we are doing right now) and therefore we create the sorting and then
        // set the partially done base playlist beforehand.
        MusicInfo->Playlist_ = Playlist; 
        CreatePlaylistsSortingInfo(&Playlist->Playlists);
        *Playlist = *CreateEmptyPlaylist(FixArena, MusicInfo, FileInfo->Count_, 
                                         Genre.BatchCount, Artist.BatchCount, Album.BatchCount);
    }
    else
    {
        if(Playlist->Song.Displayable.A.Count > 0)
        {
            DeletePlaylist(&GlobalGameState, Playlist);
        }
        AllocatePlaylist(FixArena, Playlist, FileInfo->Count_, Genre.BatchCount, Artist.BatchCount, Album.BatchCount);
    }
    
    For(Genre.BatchCount)
    {
        Push(&Playlist->Genre.Displayable, NewPlaylistID(It));
    }
    
    Playlist->Genre.Batch  = Genre;
    Playlist->Artist.Batch = Artist;
    Playlist->Album.Batch  = Album;
    Playlist->Song.FileIDs = SongFileIDs;
    
    DebugLog(255, "GenreCount: %i, ArtistCount: %i, AlbumCount: %i\n", Genre.BatchCount, Artist.BatchCount, Album.BatchCount);
}

inline void
SetSelectionArray(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID)
{
    b32 ControlIsPressed = GlobalGameState.Input.Pressed[KEY_CONTROL_LEFT] || GlobalGameState.Input.Pressed[KEY_CONTROL_RIGHT];
    b32 ShiftIsPressed   = GlobalGameState.Input.Pressed[KEY_SHIFT_LEFT]   || GlobalGameState.Input.Pressed[KEY_SHIFT_RIGHT];
    
    if(ControlIsPressed)
    {
        ToggleSelection(DisplayColumn, PlaylistColumn, ColumnDisplayID);
    }
    else if(ShiftIsPressed)
    {
        // If shift is pressed, we do the standard thing of selecting every slot inbetween the new selected and the 
        // previous selected.
        
        displayable_id NewSelectedDisplayableID = DisplayColumn->OnScreenIDs[ColumnDisplayID];
        displayable_id LastSelectedDisplayableID;
        // If nothing was selected previously, just use the same ID for both and just select the one.
        if(PlaylistColumn->Selected.A.Count > 0)
        {
            playlist_id LastSelectedID = NewPlaylistID(Get(&PlaylistColumn->Selected.A, PlaylistColumn->Selected.A.Count-1));
            if(!StackFind(&PlaylistColumn->Displayable, LastSelectedID, &LastSelectedDisplayableID.ID)) Assert(false);
        }
        else LastSelectedDisplayableID = NewSelectedDisplayableID;
        
        i32 SmallerID = (LastSelectedDisplayableID.ID+1 > NewSelectedDisplayableID.ID) 
            ? (NewSelectedDisplayableID.ID) : (LastSelectedDisplayableID.ID+1);
        i32 LargerID  = (LastSelectedDisplayableID.ID > NewSelectedDisplayableID.ID+1) 
            ? (LastSelectedDisplayableID.ID) : (NewSelectedDisplayableID.ID+1);
        for(i32 It = SmallerID; It < LargerID; ++It)
        {
            u32 PlaylistID = Get(&PlaylistColumn->Displayable.A, It);
            PushIfNotExist(&PlaylistColumn->Selected.A, PlaylistID);
        }
    }
    else 
    {
        // If it is not selected or something else was selected clear it
        if(!IsSelected(DisplayColumn, PlaylistColumn, ColumnDisplayID) || PlaylistColumn->Selected.A.Count > 1)
        {
            ClearSelection(PlaylistColumn);
        }
        ToggleSelection(DisplayColumn, PlaylistColumn, ColumnDisplayID);
    }
}

inline void
SwitchSelection(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID)
{
    ClearSelection(PlaylistColumn);
    Select(DisplayColumn, PlaylistColumn, ColumnDisplayID);
}

inline void
SwitchSelection(playlist_column *PlaylistColumn, playlist_id PLID)
{
    ClearSelection(PlaylistColumn);
    Select(PlaylistColumn, PLID);
}

internal column_type // returns none if not in any rect, and corresponding column if it is
UpdateSelectionArray(playlist_column *PlaylistColumn, display_column *DisplayColumn, v2 MouseBtnDownLocation)
{
    column_type Result = columnType_None;
    DisplayColumn->LastClickSlotID = -1;
    for(u32 It = 0; 
        It < PlaylistColumn->Displayable.A.Count && 
        It < DisplayColumn->Count;
        ++It)
    {
        if(IsInRect(DisplayColumn->BGRects[It], GlobalGameState.Input.MouseP) && 
           IsInRect(DisplayColumn->BGRects[It], MouseBtnDownLocation))
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
SelectAllOrNothing(display_column *DisplayColumn, playlist_column *PlaylistColumn)
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
            For(Genre->A.Count, Genre)
            {
                array_batch_id *GenreArtists = MusicInfo->Playlist_->Genre.Batch.Artist+Get(Genre, NewSelectID(GenreIt)).ID;
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
    // the same location) and in the end remove the 'gaps' to re-stack-ify them.
    u32 CheckValue = MP3FileInfo->Count_+1;
    if(DoArtist) Clear(&Playlist->Artist.Displayable, CheckValue);
    if(DoAlbum)  Clear(&Playlist->Album.Displayable, CheckValue);
    if(DoSong)   Clear(&Playlist->Song.Displayable, CheckValue);
    
    if(Playlist->Genre.Selected.A.Count > 0)
    {
        For(Playlist->Genre.Selected.A.Count) // Going through Genres
        {
            batch_id BatchID = Get(&Playlist->Genre.Selected, NewSelectID(It));
            sort_batch *GenreBatch = &Playlist->Genre.Batch;
            
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
        
        if(Playlist->Artist.Displayable.A.Count > 0) 
            RemoveCheckValueFromArray(Playlist->Artist.Displayable.A, CheckValue);
    }
    else // If we have no genre selected, we can fill the artist column with every artist.
    {
        if(DoArtist) 
        {
            For(Playlist->Artist.Batch.BatchCount)
            {
                Push(&Playlist->Artist.Displayable, NewPlaylistID(It));
            }
        }
    }
    
    sort_batch  *GenreBatch = &Playlist->Genre.Batch;
    sort_batch *ArtistBatch = &Playlist->Artist.Batch;
    sort_batch  *AlbumBatch = &Playlist->Album.Batch;
    
    // Go through all selected artists...
    For(Playlist->Artist.Selected.A.Count)
    {
        select_id               SelectedArtistID = NewSelectID(It);
        batch_id                   ArtistBatchID = Get(&Playlist->Artist.Selected, SelectedArtistID);
        array_batch_id *AlbumsFromSelectedArtist = ArtistBatch->Album + ArtistBatchID.ID;
        
        // For every artist go through all their albums...
        For(AlbumsFromSelectedArtist->A.Count, Album)
        {
            select_id AlbumFromSelectedArtistID = NewSelectID(AlbumIt);
            batch_id               AlbumBatchID = Get(AlbumsFromSelectedArtist, AlbumFromSelectedArtistID);
            array_batch_id *GenresOfAlbumFromSelectedArtist = AlbumBatch->Genre+AlbumBatchID.ID;
            
            // Now go through all genres which are part of the album...(almost always 1)
            For(GenresOfAlbumFromSelectedArtist->A.Count, Genre)
            {
                select_id GenreOfAlbumFromSelectedArtistID = NewSelectID(GenreIt);
                batch_id GenreBatchID = Get(GenresOfAlbumFromSelectedArtist, GenreOfAlbumFromSelectedArtistID);
                
                // Only go on if no genre is selected or the genre of this album is selected...
                if(Playlist->Genre.Selected.A.Count == 0 || StackContains(&Playlist->Genre.Selected, GenreBatchID))
                {
                    if(DoAlbum) PutAndCheck(&Playlist->Album.Displayable, AlbumBatchID, CheckValue);
                    // If no album is selected, fill song list from the current album and
                    // check that _only_ songs which are also corresponding to the artist _and_
                    // selected genres are inserted.
                    if(Playlist->Album.Selected.A.Count == 0 && DoSong)
                    {
                        For(AlbumBatch->Song[AlbumBatchID.ID].A.Count, Song)
                        {
                            playlist_id PlaylistID = Get(AlbumBatch->Song+AlbumBatchID.ID, NewSelectID(SongIt));
                            // Check if song is in artist.
                            if(StackContains(ArtistBatch->Song+ArtistBatchID.ID, PlaylistID))
                            {
                                if(Playlist->Genre.Selected.A.Count > 0)
                                {
                                    // Check if song is in selected genre
                                    // @Slow::
                                    For(Playlist->Genre.Selected.A.Count)
                                    {
                                        select_id SelectedGenreID = NewSelectID(It);
                                        batch_id     GenreBatchID = Get(&Playlist->Genre.Selected, SelectedGenreID);
                                        
                                        if(StackContains(GenreBatch->Song+GenreBatchID.ID, PlaylistID))
                                        {
                                            PutAndCheck(&Playlist->Song.Displayable, PlaylistID, CheckValue);
                                        }
                                    }
                                }
                                else PutAndCheck(&Playlist->Song.Displayable, PlaylistID, CheckValue);
                            }
                        }
                    }
                    // TODO:: @FixCreateSortingInfo 
                    // Check the 'empty' category if we actually can always break, because every instance 
                    // of GenreOfAlbumFromSelectedArtist has all songs inside. so it will never be necessary to
                    // go through the other counts, if we did it once. But then we should check why we even have
                    // more than one? bzw. why is it that there are all songs, of multiple genres, in every instance
                    // of album-genre counts.
                    break;
                }
            }
        }
    }
    if(Playlist->Album.Displayable.A.Count > 0) RemoveCheckValueFromArray(Playlist->Album.Displayable.A, CheckValue);
    if(Playlist->Genre.Selected.A.Count  == 0 && 
       Playlist->Artist.Selected.A.Count == 0)
    {
        if(DoAlbum) 
        {
            For(Playlist->Album.Batch.BatchCount)
            {
                Push(&Playlist->Album.Displayable, NewPlaylistID(It));
            }
        }
    }
    
    if(DoSong) 
    {
        RestartTimer("Proper Album select ********");
        For(Playlist->Album.Selected.A.Count) // Going through Albums
        {
            // @Slow::
            select_id                 SelectedAlbumID = NewSelectID(It);
            batch_id                     AlbumBatchID = Get(&Playlist->Album.Selected, SelectedAlbumID);
            array_playlist_id *SongsFromSelectedAlbum = AlbumBatch->Song + AlbumBatchID.ID;
            
            For(SongsFromSelectedAlbum->A.Count, Song)
            {
                select_id SongSelectID = NewSelectID(SongIt);
                playlist_id PlaylistID = Get(SongsFromSelectedAlbum, SongSelectID);
                
                // We have to check if the songs of the album are actually from the artists that 
                // are in the displayable list or selected. This is very slow, as we are three For
                // loops deep here. And all three arrays which are looped can be pretty large...
                array_playlist_id *ArtistsToCheck = &Playlist->Artist.Selected;
                if(ArtistsToCheck->A.Count == 0) ArtistsToCheck = &Playlist->Artist.Displayable;
                For(ArtistsToCheck->A.Count, ArtistCheck)
                {
                    select_id SelectedArtistID = NewSelectID(ArtistCheckIt);
                    batch_id     ArtistBatchID = Get(ArtistsToCheck, SelectedArtistID);
                    
                    if(StackContains(ArtistBatch->Song+ArtistBatchID.ID, PlaylistID))
                    {
                        PutAndCheck(&Playlist->Song.Displayable, PlaylistID, CheckValue);
                    }
                }
            }
            
            //MergeDisplayArrays(&Playlist->Song.Displayable, SongsFromSelectedAlbum, CheckValue);
        }
        SnapTimer("Proper Album select ********");
        
        if(Playlist->Song.Displayable.A.Count > 0) RemoveCheckValueFromArray(Playlist->Song.Displayable.A, CheckValue);
        if(Playlist->Genre.Selected.A.Count  == 0 && 
           Playlist->Artist.Selected.A.Count == 0 &&
           Playlist->Album.Selected.A.Count  == 0)
        {
            For(Playlist->Song.FileIDs.A.Count)
            {
                Push(&Playlist->Song.Displayable, NewPlaylistID(It));
            }
        }
    }
    
}

internal void
UpdateSortingInfoChangedVisuals(renderer *Renderer, music_info *MusicInfo, music_display_info *DisplayInfo, column_type Type)
{
    
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
UpdateSortingInfoChangedWithoutVisuals(renderer *Renderer, music_info *MusicInfo, mp3_info *MP3Info)
{
    music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
    playlist_info *Playlist         = MusicInfo->Playlist_;
    
    FillDisplayables(MusicInfo, &MP3Info->FileInfo, &MusicInfo->DisplayInfo);
    
    SortDisplayables(MusicInfo, &MP3Info->FileInfo);
    if(Playlist->IsShuffled) ShufflePlaylist(Playlist, true);
    
    UpdatePlayingSong(MusicInfo);
    
}

internal void
UpdateSortingInfoChanged(renderer *Renderer, music_info *MusicInfo, mp3_info *MP3Info, column_type Type)
{
    UpdateSortingInfoChangedWithoutVisuals(Renderer, MusicInfo, MP3Info);
    UpdateSortingInfoChangedVisuals(Renderer, MusicInfo, &MusicInfo->DisplayInfo, Type);
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

enum string_compare_result
{
    stringCompare_FirstLower  = -1,
    stringCompare_Same        =  0,
    stringCompare_FirstHigher =  1,
    stringCompare_BothEmpty   =  2, 
};

internal string_compare_result
CompareAB(string_c *A, string_c *B)
{
    string_compare_result Result = stringCompare_Same;
    if(A->Pos == 0 && B->Pos == 0) return stringCompare_BothEmpty; // Early out
    
    for(u32 It = 0; 
        It < A->Pos && 
        It < B->Pos; 
        ++It)
    {
        u8 A1 = A->S[It];
        u8 B1 = B->S[It];
        
        if(A1 >= 'a' && A1 <= 'z') A1 -= 32;
        if(B1 >= 'a' && B1 <= 'z') B1 -= 32;
        
        if(A1 < B1)
        {
            Result = stringCompare_FirstHigher;
            break;
        }
        else if(A1 > B1)
        {
            Result = stringCompare_FirstLower;
            break;
        }
    }
    if(A->Pos == 0) Result = stringCompare_FirstHigher;
    if(B->Pos == 0) Result = stringCompare_FirstLower;
    
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

inline i32
MetadataCompare(mp3_metadata *A, mp3_metadata *B)
{
    i32 Result = -1;
    
    string_compare_result CompArtist = CompareAB(&A->Artist, &B->Artist);
    if(CompArtist == stringCompare_Same || 
       CompArtist == stringCompare_BothEmpty)
    {
        if(A->Year == B->Year)
        {
            string_compare_result CompAlbum = CompareAB(&A->Album, &B->Album);
            if(CompAlbum == stringCompare_Same || 
               CompAlbum == stringCompare_BothEmpty)
            {
                if(A->Track == B->Track) ; // Both are exactly the same.
                else if(CompAlbum == stringCompare_BothEmpty && CompArtist == stringCompare_BothEmpty) ;
                else Result = (A->Track < B->Track);
            }
            else Result = (CompAlbum == stringCompare_FirstHigher);
        }
        else Result = A->Year < B->Year;
    }
    else Result = (CompArtist == stringCompare_FirstHigher);
    
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
            playlist_id PLA = NewPlaylistID(Get(&SongPlaylist->Displayable.A, HighID));
            playlist_id PLB = NewPlaylistID(Get(&SongPlaylist->Displayable.A, Pivot));
            mp3_metadata *A = GetMetadata(SongPlaylist, FileInfo, PLA);
            mp3_metadata *B = GetMetadata(SongPlaylist, FileInfo, PLB);
            i32 CompResult = MetadataCompare(A, B);
            if(CompResult == 1) 
            { 
                SmallID++;
                Switch(&SongPlaylist->Displayable.A, SmallID, HighID); 
            }
            else if(CompResult == -1)
            {
                // If both have exactly the same Metadata, we check for the subpath
                // the files are in and then the filename itself.
                string_c *ASubPath = GetSongSubPath(SongPlaylist, FileInfo, PLA);
                string_c *BSubPath = GetSongSubPath(SongPlaylist, FileInfo, PLB);
                string_compare_result SubPathComp = CompareAB(ASubPath, BSubPath);
                if(SubPathComp == stringCompare_FirstHigher)
                {
                    SmallID++;
                    Switch(&SongPlaylist->Displayable.A, SmallID, HighID); 
                }
                else if(SubPathComp == stringCompare_Same || SubPathComp == stringCompare_BothEmpty)
                {
                    string_c *AName = GetSongFileName(SongPlaylist, FileInfo, PLA);
                    string_c *BName = GetSongFileName(SongPlaylist, FileInfo, PLB);
                    if(CompareAB(AName, BName) == stringCompare_FirstHigher)
                    {
                        SmallID++;
                        Switch(&SongPlaylist->Displayable.A, SmallID, HighID); 
                    }
                }
            }
        } 
        Switch(&SongPlaylist->Displayable.A, SmallID+1, High); 
        i32 PartitionID = (SmallID + 1); 
        
        QuickSortSongColumn(Low, PartitionID - 1, FileInfo, SongPlaylist); 
        QuickSortSongColumn(PartitionID + 1, High, FileInfo, SongPlaylist); 
    }
}

inline void Switch(void *Array, i32 P1, i32 P2);
internal void
SortDisplayables(music_info *MusicInfo, mp3_file_info *MP3FileInfo)
{
    array_playlist_id *Genre  = &MusicInfo->Playlist_->Genre.Displayable;
    array_playlist_id *Artist = &MusicInfo->Playlist_->Artist.Displayable;
    array_playlist_id *Album  = &MusicInfo->Playlist_->Album.Displayable;
    sort_blob GenreBlob  = {&MusicInfo->Playlist_->Genre.Batch, Genre};
    sort_blob ArtistBlob = {&MusicInfo->Playlist_->Artist.Batch, Artist};
    sort_blob AlbumBlob  = {&MusicInfo->Playlist_->Album.Batch, Album};
    QuickSort(0, (i32)Genre->A.Count-1,  Genre,  {IsHigherInAlphabet, &GenreBlob, Switch});
    QuickSort(0, (i32)Artist->A.Count-1, Artist, {IsHigherInAlphabet, &ArtistBlob, Switch});
    QuickSort(0, (i32)Album->A.Count-1,  Album,  {IsHigherInAlphabet, &AlbumBlob, Switch});
    
    RestartTimer("SortSongs");
    // TODO:: This is a f*cking stupid hack... Why is it so slow
    // for switching in a playlist, but not when switching to the
    // 'main' playlist...
    ShuffleStack(&MusicInfo->Playlist_->Song.Displayable.A);
    QuickSortSongColumn(0, MusicInfo->Playlist_->Song.Displayable.A.Count-1, MP3FileInfo, &MusicInfo->Playlist_->Song);
    SnapTimer("SortSongs");
}

internal void
FinishChangeEntireSong(game_state *GS, playing_song *Song)
{
    mp3dec_file_info_t *DInfo = &GS->MP3Info->DecodeInfo.PlayingDecoded.Data;
    if(DInfo->hz == 0 && DInfo->channels == 0 && DInfo->samples == 0)
    {
        //PushErrorMessage(GS, NewStaticStringCompound("ERROR:: Song could not be loaded properly!"));
        PushSoundBufferClear(GS->SoundThreadInterface);
        GS->MusicInfo.IsPlaying = false;
        PushIsPlaying(GS->SoundThreadInterface, false);
        GS->MusicInfo.PlayingSong.DisplayableID.ID = -1;
        GS->MusicInfo.PlayingSong.PlaylistID.ID = -1;
        GS->MusicInfo.PlayingSong.DecodeID = -1;
        SetTheNewPlayingSong(&GS->Renderer, &GS->MusicInfo.DisplayInfo.PlayingSongPanel, &GS->Layout, &GS->MusicInfo);
    }
    else PushSongLoadingComplete(GlobalGameState.SoundThreadInterface, DInfo);
}

internal void
FinishChangeSong(game_state *GameState, playing_song *Song)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    
    playlist_id PlaylistID = GetNextSong(MusicInfo);
    
    mp3dec_file_info_t *DInfo = &GameState->MP3Info->DecodeInfo.DecodedData[Song->DecodeID];
    Assert(DInfo->layer == 3);
    
    PushSoundBufferClear(GameState->SoundThreadInterface);
    
    if(DInfo->hz == 0 && DInfo->channels == 0 && DInfo->samples == 0) 
        PushErrorMessage(GameState, NewStaticStringCompound("ERROR:: Song could not be loaded properly!"));
    else
    {
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
}

internal void
ChangeSong(game_state *GameState, playing_song *Song)
{
    music_info *MusicInfo = &GameState->MusicInfo;
    
    playlist_id PlaylistID = GetNextSong(MusicInfo);
    
    if(PlaylistID >= 0)
    {
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
        DebugLog(60, "#RandomStop: ChangeSong, PlaylistID is < 0\n");
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
    playlist_info *AllPlaylist = MusicInfo->Playlists.List+0;
    
    CreateMusicSortingInfo(AllPlaylist);
    UpdatePlaylistScreenName(GameState, AllPlaylist);
    
    // To make our lives easier, we just save what playlist we are currently
    // on, delete all playlists and reload them. This discards all, now maybe non existent, 
    // songs from the playlist. After that, we switch back to the playlist we were on before.
    i32 PlaylistID = GetPlaylistID(&GameState->MusicInfo, MusicInfo->Playlist_);
    NewLocalString(PLName, PLAYLIST_MAX_NAME_LENGTH, AllPlaylist->Playlists.Batch.Names[PlaylistID].S);
    i32 NameEndP = FindLastOccurrenceOfCharInStringCompound(&PLName, '(');
    Assert(NameEndP >= 0);
    PLName.Pos = NameEndP-1;
    
    For(MusicInfo->Playlists.Count-1)
    { 
        RemovePlaylist(GameState, MusicInfo->Playlists.List + (It+1), false);
    }
    LoadAllPlaylists(GameState);
    
    playlist_info *ActivePlaylist = GetPlaylist(GameState, PLName);
    if(ActivePlaylist) 
    {
        if(!CompareStringAndCompound(&PLName, (u8 *)"All")) SwitchPlaylist(GameState, ActivePlaylist);
        else UpdateSortingInfoChangedVisuals(&GameState->Renderer, MusicInfo, &MusicInfo->DisplayInfo, columnType_Playlists);
    }
    else SwitchPlaylist(GameState, AllPlaylist);
    // ****
    
    if(MusicInfo->Playlist_ == AllPlaylist) // Also do this if active playlist was changed!
    {
        MusicInfo->PlayingSong = {-1, -1, -1, 0};
        FillDisplayables(MusicInfo, &GameState->MP3Info->FileInfo, &MusicInfo->DisplayInfo);
        SortDisplayables(MusicInfo, &GameState->MP3Info->FileInfo);
        UpdatePlayingSong(MusicInfo);
        UpdateAllDisplayColumns(GameState);
        
        MusicInfo->DisplayInfo.Genre.DisplayCursor = 0;
        MusicInfo->DisplayInfo.Artist.DisplayCursor = 0;
        MusicInfo->DisplayInfo.Album.DisplayCursor = 0;
        MusicInfo->DisplayInfo.Song.Base.DisplayCursor = 0;
        
        UpdateVerticalSliders(MusicInfo);
        UpdateHorizontalSliders(MusicInfo);
        
        AddJobs_LoadOnScreenMP3s(GameState, &GameState->JobQueue);
    }
    
    SaveMP3LibraryFile(GameState, GameState->MP3Info);
    
    DestroyArray(&GameState->FixArena, &MusicInfo->DisplayInfo.Genre.Search.InitialDisplayables.A);
    DestroyArray(&GameState->FixArena, &MusicInfo->DisplayInfo.Artist.Search.InitialDisplayables.A);
    DestroyArray(&GameState->FixArena, &MusicInfo->DisplayInfo.Album.Search.InitialDisplayables.A);
    DestroyArray(&GameState->FixArena, &MusicInfo->DisplayInfo.Song.Base.Search.InitialDisplayables.A);
    MusicInfo->DisplayInfo.Genre.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Genre.Displayable.A.Length);
    MusicInfo->DisplayInfo.Artist.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Artist.Displayable.A.Length);
    MusicInfo->DisplayInfo.Album.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Album.Displayable.A.Length);
    MusicInfo->DisplayInfo.Song.Base.Search.InitialDisplayables.A = 
        CreateArray(&GameState->FixArena, MusicInfo->Playlist_->Song.Displayable.A.Length);
    
}


// *****************************************************************************
// New Playlist stuff **********************************************************
// *****************************************************************************

internal void
CreatePlaylistsSortingInfo(playlist_column *Playlists)
{
    Playlists->Type             = columnType_Playlists;
    Playlists->Selected.A       = CreateArray(&GlobalGameState.FixArena, 1);
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

internal void
AllocatePlaylist(arena_allocator *Arena, playlist_info *Playlist, i32 SongIDCount, i32 GenreBatchCount, i32 ArtistBatchCount, i32 AlbumBatchCount)
{
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
}

internal playlist_info *
CreateEmptyPlaylist(arena_allocator *Arena, music_info *MusicInfo, i32 SongIDCount/*default -1*/, i32 GenreBatchCount/*default -1*/, i32 ArtistBatchCount/*default -1*/, i32 AlbumBatchCount/*default -1*/)
{
    if(MusicInfo->Playlists.Count >= MusicInfo->Playlists.MaxCount)
    {
        MusicInfo->Playlists.MaxCount = MusicInfo->Playlists.Count*2;
        MusicInfo->Playlists.List = ReallocateArray(Arena, MusicInfo->Playlists.List, MusicInfo->Playlists.Count,
                                                    MusicInfo->Playlists.MaxCount, playlist_info);
    }
    Assert(MusicInfo->Playlists.Count < MusicInfo->Playlists.MaxCount);
    playlist_info *Playlist = MusicInfo->Playlists.List+MusicInfo->Playlists.Count++;
    
    // First entry in Playlists.List is the 'everything' list.
    if(GenreBatchCount < 0)  GenreBatchCount  = MusicInfo->Playlists.List[0].Genre.Batch.BatchCount; 
    if(ArtistBatchCount < 0) ArtistBatchCount = MusicInfo->Playlists.List[0].Artist.Batch.BatchCount;
    if(AlbumBatchCount < 0)  AlbumBatchCount  = MusicInfo->Playlists.List[0].Album.Batch.BatchCount;
    if(SongIDCount < 0)      SongIDCount      = MusicInfo->Playlists.List[0].Song.FileIDs.A.Count;
    
    AllocatePlaylist(Arena, Playlist, SongIDCount, GenreBatchCount, ArtistBatchCount, AlbumBatchCount);
    
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
    
    playlist_info   *Playlist     = MusicInfo->Playlists.List+0;
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
        // Every album has at least one genre associated with it. If the metadata
        // did not have an entry for genre, the empty genre will be assigned to it.
        // As the first album (index 0) is the empty album, it can have actually 
        // many genres, thats why it has a higher limit (should be done dynamically
        // at some point).
        if(It == 0) Album.Genre[It].A = CreateArray(FixArena, 100); // @HardLimit
        else Album.Genre[It].A = CreateArray(FixArena, 100);
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
        
        PushIfNotExist(&Album.Genre[AlbumBatchID].A, GenreBatchID);
        PushIfNotExist(&Album.Song[AlbumBatchID].A, It);
    }
    
    Reset(&NewPlaylist->Genre.Displayable);
    Reset(&NewPlaylist->Artist.Displayable);
    Reset(&NewPlaylist->Album.Displayable);
    Reset(&NewPlaylist->Song.Displayable);
    
    For(Genre.BatchCount)  Push(&NewPlaylist->Genre.Displayable, NewPlaylistID(It));
    For(Artist.BatchCount) Push(&NewPlaylist->Artist.Displayable, NewPlaylistID(It));
    For(Album.BatchCount)  Push(&NewPlaylist->Album.Displayable, NewPlaylistID(It));
    
    if(NewPlaylist->Genre.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Genre.Batch);
    if(NewPlaylist->Artist.Batch.BatchCount > 0) FreeSortBatch(FixArena, &NewPlaylist->Artist.Batch);
    if(NewPlaylist->Album.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Album.Batch);
    if(NewPlaylist->Song.FileIDs.A.Count    > 0) DestroyArray(FixArena, &NewPlaylist->Song.FileIDs.A);
    
    // Now fill playlist_colum song
    NewPlaylist->Song.FileIDs.A = CreateArray(FixArena, Playlist->Song.FileIDs.A.Count);
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
    
    playlist_info       *Playlist = MusicInfo->Playlist_;
    playlist_info    *AllPlaylist = MusicInfo->Playlists.List+0;
    arena_allocator     *FixArena = &GlobalGameState.FixArena;
    arena_allocator *ScratchArena = &GlobalGameState.ScratchArena;
    
    // If the user has nothing selected in the Song column, then
    // we use the whole song displayable list. If they have something
    // selected, then we only use those.
    array_playlist_id *SongList = 0;
    if(Playlist->Song.Selected.A.Count > 0) SongList = &Playlist->Song.Selected;
    else                                    SongList = &Playlist->Song.Displayable;
    
    sort_batch Genre  = {};
    InitializeSortBatch(FixArena, &Genre,  AllPlaylist->Genre.Batch.BatchCount);
    sort_batch Artist = {};
    InitializeSortBatch(FixArena, &Artist, AllPlaylist->Artist.Batch.BatchCount);
    sort_batch Album  = {};
    InitializeSortBatch(FixArena, &Album,  AllPlaylist->Album.Batch.BatchCount);
    
    song_sort_info *SortBatchInfo = AllocateArray(ScratchArena, AllPlaylist->Song.Displayable.A.Count, song_sort_info);
    u32 *Genre_CountForBatches    = AllocateArray(ScratchArena, Genre.MaxBatches, u32);
    u32 *Artist_CountForBatches   = AllocateArray(ScratchArena, Artist.MaxBatches, u32);
    u32 *Album_CountForBatches    = AllocateArray(ScratchArena, Album.MaxBatches, u32);
    
    For(SongList->A.Count)
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
        // Every album has at least one genre associated with it. If the metadata
        // did not have an entry for genre, the empty genre will be assigned to it.
        // As the first album (index 0) is the empty album, it can have actually 
        // many genres, thats why it has a higher limit (should be done dynamically
        // at some point).
        if(It == 0) Album.Genre[It].A = CreateArray(FixArena, 100); // @HardLimit
        else Album.Genre[It].A = CreateArray(FixArena, 100);
    }
    
    FreeMemory(ScratchArena, Genre_CountForBatches);
    FreeMemory(ScratchArena, Artist_CountForBatches);
    FreeMemory(ScratchArena, Album_CountForBatches);
    
    For(SongList->A.Count)
    {
        u32 GenreBatchID  = SortBatchInfo[It].GenreBatchID;
        u32 ArtistBatchID = SortBatchInfo[It].ArtistBatchID;
        u32 AlbumBatchID  = SortBatchInfo[It].AlbumBatchID;
        
        PushIfNotExist(&Genre.Artist[GenreBatchID].A, ArtistBatchID);
        PushIfNotExist(&Genre.Album[GenreBatchID].A, AlbumBatchID);
        PushIfNotExist(&Genre.Song[GenreBatchID].A, It);
        
        PushIfNotExist(&Artist.Album[ArtistBatchID].A, AlbumBatchID);
        PushIfNotExist(&Artist.Song[ArtistBatchID].A, It);
        
        PushIfNotExist(&Album.Genre[AlbumBatchID].A, GenreBatchID);
        PushIfNotExist(&Album.Song[AlbumBatchID].A, It);
    }
    
    Reset(&NewPlaylist->Genre.Displayable);
    Reset(&NewPlaylist->Artist.Displayable);
    Reset(&NewPlaylist->Album.Displayable);
    Reset(&NewPlaylist->Song.Displayable);
    
    For(Genre.BatchCount)  Push(&NewPlaylist->Genre.Displayable, NewPlaylistID(It));
    For(Artist.BatchCount) Push(&NewPlaylist->Artist.Displayable, NewPlaylistID(It));
    For(Album.BatchCount)  Push(&NewPlaylist->Album.Displayable, NewPlaylistID(It));
    
    if(NewPlaylist->Genre.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Genre.Batch);
    if(NewPlaylist->Artist.Batch.BatchCount > 0) FreeSortBatch(FixArena, &NewPlaylist->Artist.Batch);
    if(NewPlaylist->Album.Batch.BatchCount  > 0) FreeSortBatch(FixArena, &NewPlaylist->Album.Batch);
    if(NewPlaylist->Song.FileIDs.A.Count    > 0) DestroyArray(FixArena, &NewPlaylist->Song.FileIDs.A);
    
    // Now fill playlist_column song
    NewPlaylist->Song.FileIDs.A = CreateArray(FixArena, AllPlaylist->Song.FileIDs.A.Count);
    For(SongList->A.Count)
    {
        playlist_id PLID = Get(SongList, NewDisplayableID(It));
        file_id      FID = GetFileID(&Playlist->Song, PLID);
        Push(&NewPlaylist->Song.Displayable, NewPlaylistID(It));
        Push(&NewPlaylist->Song.FileIDs, FID);
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

internal i32
GetOnScreenID(game_state *GS, playlist_info *Playlist)
{
    i32 Result = -1;
    
    playlist_column    *PlaylistsColumn = &GS->MusicInfo.Playlist_->Playlists;
    display_column *DisplayColumn = &GS->MusicInfo.DisplayInfo.Playlists;
    
    For(DisplayColumn->Count)
    {
        playlist_id  PlaylistID      = Get(&PlaylistsColumn->Displayable, DisplayColumn->OnScreenIDs[It]);
        playlist_info *CheckPlaylist = GS->MusicInfo.Playlists.List + PlaylistID.ID;
        if(Playlist == CheckPlaylist) 
        {
            Result = It;
            break;
        }
    }
    
    return Result;
}

inline string_c *
GetPlaylistName(music_info *MusicInfo, playlist_info *Playlist)
{
    string_c *Result = 0;
    
    i32 PlaylistID = GetPlaylistID(MusicInfo, Playlist);
    Result = Playlist->Playlists.Batch.Names+PlaylistID;
    
    return Result;
}

internal playlist_info *
GetPlaylist(game_state *GS, string_c PlaylistName)
{
    playlist_info *Result = 0;
    
    For(GS->MusicInfo.Playlists.Count)
    {
        playlist_info *NextPlaylist = GS->MusicInfo.Playlists.List+It;
        NewLocalString(NextName, PLAYLIST_MAX_NAME_LENGTH+10, GetPlaylistName(&GS->MusicInfo, NextPlaylist)->S);
        
        i32 NameEndP = FindLastOccurrenceOfCharInStringCompound(&NextName, '(');
        Assert(NameEndP >= 0);
        NextName.Pos = NameEndP-1;
        if(CompareStringCompounds(&PlaylistName, &NextName))
        {
            Result = NextPlaylist;
            break;
        }
    }
    
    return Result;
}

internal void
UpdatePlaylistScreenName(game_state *GS, playlist_info *Playlist)
{
    display_column *DisplayColumn = &GS->MusicInfo.DisplayInfo.Playlists;
    i32 OnScreenID = GetOnScreenID(GS, Playlist);
    i32 PlaylistID = GetPlaylistID(&GS->MusicInfo, Playlist);
    
    RemoveRenderText(&GS->Renderer, DisplayColumn->Text + OnScreenID);
    NewLocalString(ScreenName, PLAYLIST_MAX_NAME_LENGTH + 10, Playlist->Playlists.Batch.Names[PlaylistID].S);
    i32 Pos = FindLastOccurrenceOfCharInStringCompound(&ScreenName, '(');
    if(Pos >= 0) ScreenName.Pos = Pos+1;
    else AppendStringToCompound(&ScreenName, (u8 *)" (");
    I32ToString(&ScreenName, Playlist->Song.FileIDs.A.Count);
    AppendCharToCompound(&ScreenName, ')');
    
    ResetStringCompound(Playlist->Playlists.Batch.Names[PlaylistID]);
    AppendStringCompoundToCompound(Playlist->Playlists.Batch.Names+PlaylistID, &ScreenName);
    
    RenderText(GS, font_Small, &ScreenName, DisplayColumn->Colors.Text,
               DisplayColumn->Text+OnScreenID,  DisplayColumn->ZValue-0.01f, DisplayColumn->BGRects[OnScreenID]);
    Translate(DisplayColumn->Text+OnScreenID, V2(0, 3));
    ResetColumnText(DisplayColumn, Playlist->Playlists.Displayable.A.Count);
}

internal void
InsertSlotIntoPlaylist(game_state *GS, playlist_info *IntoPlaylist, column_type Type, displayable_id DisplayableID)
{
    Assert(Type != columnType_Playlists);
    Assert(Type != columnType_None);
    array_file_id FileIDs = IntoPlaylist->Song.FileIDs;
    
    if(Type == columnType_Song)
    {
        file_id FileID = GetFileID(&GS->MusicInfo, DisplayableID);
        if(!StackContains(&FileIDs.A, FileID.ID)) Push(&FileIDs, FileID);
    }
    else
    {
        playlist_info     *FromPlaylist = GS->MusicInfo.Playlist_;
        playlist_column *PlaylistColumn = FromPlaylist->Columns + Type;
        
        array_playlist_id Songs = PlaylistColumn->Batch.Song[Get(&PlaylistColumn->Displayable, DisplayableID).ID];
        For(Songs.A.Count)
        {
            playlist_id PlaylistID = Get(&Songs, NewDisplayableID(It));
            file_id FileID = NewFileID(Get(&FromPlaylist->Song.FileIDs.A, PlaylistID.ID));
            
            if(!StackContains(&FileIDs.A, FileID.ID)) Push(&FileIDs, FileID);
        }
    }
    
    FillPlaylistWithFileIDs(&GS->MusicInfo, &GS->MP3Info->FileInfo, IntoPlaylist, FileIDs);
    SyncPlaylists_playlist_column(&GS->MusicInfo);
    UpdatePlaylistScreenName(GS, IntoPlaylist);
    
    SavePlaylist(GS, IntoPlaylist);
}

internal void
InsertSlotsIntoPlaylist(game_state *GS, playlist_info *IntoPlaylist, column_type Type, array_u32 DisplayableIDs)
{
    Assert(Type != columnType_Playlists);
    Assert(Type != columnType_None);
    array_file_id FileIDs = IntoPlaylist->Song.FileIDs;
    RestartTimer("InsertSlots");
    
    For(DisplayableIDs.Count)
    {
        displayable_id DisplayableID = NewDisplayableID(Get(&DisplayableIDs, It));
        if(Type == columnType_Song)
        {
            file_id FileID = GetFileID(&GS->MusicInfo, DisplayableID);
            if(!StackContains(&FileIDs.A, FileID.ID)) Push(&FileIDs, FileID);
        }
        else
        {
            playlist_info     *FromPlaylist = GS->MusicInfo.Playlist_;
            playlist_column *PlaylistColumn = FromPlaylist->Columns + Type;
            
            array_playlist_id Songs = PlaylistColumn->Batch.Song[Get(&PlaylistColumn->Displayable, DisplayableID).ID];
            For(Songs.A.Count)
            {
                playlist_id PlaylistID = Get(&Songs, NewDisplayableID(It));
                file_id FileID = NewFileID(Get(&FromPlaylist->Song.FileIDs.A, PlaylistID.ID));
                
                if(!StackContains(&FileIDs.A, FileID.ID)) Push(&FileIDs, FileID);
            }
        }
    }
    SnapTimer("InsertSlots");
    FillPlaylistWithFileIDs(&GS->MusicInfo, &GS->MP3Info->FileInfo, IntoPlaylist, FileIDs);
    UpdatePlaylistScreenName(GS, IntoPlaylist);
    SyncPlaylists_playlist_column(&GS->MusicInfo);
    SnapTimer("InsertSlots");
    SavePlaylist(GS, IntoPlaylist);
    SnapTimer("InsertSlots");
}

struct selection_on_remove
{
    string_c **SelectedNames;
    u32 *SelectedCounts;
    u32 *DisplayableCounts;
};

internal void
CollectSelectedNamesForPrevColumns(game_state *GS, playlist_info *Playlist, column_type Type,
                                   selection_on_remove *SelectData_out)
{
    // We save the slot-names of every selection from the column the user removes from, backwards
    // to the first one (genre) -except when the user removes from song column, then we consider
    // only those columns that come after, because songs cannot be removed through cascading through 
    // previous columns- we need the names, because we have no other exact identification atm.
    u32 TypeLength = (Type < columnType_Song) ? Type+1 : Type;
    selection_on_remove *Result = SelectData_out;
    // Save information for all prev columns
    Result->SelectedCounts      = AllocateArray(&GS->ScratchArena, TypeLength, u32);
    Result->DisplayableCounts   = AllocateArray(&GS->ScratchArena, TypeLength, u32);
    Result->SelectedNames       = AllocateArray(&GS->ScratchArena, TypeLength, string_c *);
    For((u32)TypeLength, Type/*It*/)
    {
        Assert(TypeIt != columnType_Song);
        playlist_column *GetNamePLColumn = Playlist->Columns+TypeIt;
        Result->SelectedCounts[TypeIt]    = GetNamePLColumn->Selected.A.Count;
        Result->DisplayableCounts[TypeIt] = GetNamePLColumn->Displayable.A.Count;
        Result->SelectedNames[TypeIt]     = AllocateArray(&GS->ScratchArena, Result->SelectedCounts[TypeIt], string_c);
        For(Result->SelectedCounts[TypeIt])
        {
            string_c            *Name = GetName(GS, GetNamePLColumn, Get(&GetNamePLColumn->Selected, NewSelectID(It)));
            Result->SelectedNames[TypeIt][It] = NewStringCompound(&GS->ScratchArena, Name->Pos);
            CopyIntoCompound(Result->SelectedNames[TypeIt]+It, Name);
        }
    }
}

internal void
RemoveMissingSelected(game_state *GS, playlist_info *Playlist, column_type Type, selection_on_remove *SelectData)
{
    // @Slow:: If we have many things selected, we go through all the things and make string comps for them...
    u32 TypeLength = (Type < columnType_Song) ? Type+1 : Type;
    For((u32)TypeLength, Type/*It*/)
    {
        Assert(TypeIt != columnType_Song);
        playlist_column *CheckNamePLColumn = Playlist->Columns+TypeIt;
        string_c            *SelectedNames = SelectData->SelectedNames[TypeIt];
        
        if(SelectData->DisplayableCounts[TypeIt] == CheckNamePLColumn->Displayable.A.Count) continue;
        
        for(i32 SelectIt = SelectData->SelectedCounts[TypeIt]-1; SelectIt >= 0; --SelectIt)
        {
            b32 Found = false;
            // This cannot be the song column, as we do not consider the 
            // 'currently removing from' column, and go backwards from it.
            For(CheckNamePLColumn->Displayable.A.Count, D)
            {
                displayable_id DID = NewDisplayableID(DIt);
                string_c *Name = GetName(CheckNamePLColumn, DID);
                if(CompareStringCompounds(Name, SelectedNames+SelectIt))
                {
                    Put(&CheckNamePLColumn->Selected.A, SelectIt, GetPlaylistID(CheckNamePLColumn, DID).ID);
                    Found = true;
                    break;
                }
            }
            if(!Found)
            {
                // We go through the selected backwards to savely remove the
                // entries from this list and keep it stable, while we still
                // go through it.
                Take(&CheckNamePLColumn->Selected.A, SelectIt);
            }
        }
        
        // Additionally, for each columnthat changes the Displayable Count, we need to 
        // update the saved OnScreenIDs.
        display_column *DColumn = GS->MusicInfo.DisplayInfo.Columns[TypeIt];
        For(DColumn->Count) DColumn->OnScreenIDs[It] = NewDisplayableID(0);
        SetLocalPositionY(DColumn->BGRects[0], 0.0f);
    }
}

internal void
RemoveSlotFromPlaylist(game_state *GS, column_type Type, displayable_id DisplayableID)
{
    RestartTimer("RemoveSlotFromPlaylist");
    Assert(Type != columnType_Playlists);
    Assert(Type != columnType_None);
    playlist_info     *FromPlaylist = GS->MusicInfo.Playlist_;
    playlist_column *PlaylistColumn = FromPlaylist->Columns + Type;
    array_file_id FileIDs;
    
    selection_on_remove SelectData = {};
    CollectSelectedNamesForPrevColumns(GS, FromPlaylist, Type, &SelectData);
    
    if(Type == columnType_Song)
    {
        FileIDs = FromPlaylist->Song.FileIDs;
        file_id FileID = GetFileID(&GS->MusicInfo, DisplayableID);
        StackFindAndTake(&FileIDs.A, FileID.ID);
    }
    else
    {
        // As we manipulate the array we also read from, we need to copy it.
        // @Slow
        FileIDs.A = Copy(&GS->ScratchArena, FromPlaylist->Song.FileIDs.A);
        
        // We can just use Displayable directly, as we refill it later anyway.
        array_playlist_id *Displayable = &FromPlaylist->Song.Displayable;
        array_playlist_id Songs = PlaylistColumn->Batch.Song[Get(&PlaylistColumn->Displayable, DisplayableID).ID];
        For(Songs.A.Count)
        {
            playlist_id PlaylistID = Get(&Songs, NewDisplayableID(It));
            
            if(StackFindAndTake(&Displayable->A, PlaylistID.ID))
            {
                file_id FileID = NewFileID(Get(&FromPlaylist->Song.FileIDs.A, PlaylistID.ID));
                StackFindAndTake(&FileIDs.A, FileID.ID);
            }
        }
    }
    
    FillPlaylistWithFileIDs(&GS->MusicInfo, &GS->MP3Info->FileInfo, FromPlaylist, FileIDs);
    UpdatePlaylistScreenName(GS, FromPlaylist);
    SyncPlaylists_playlist_column(&GS->MusicInfo);
    KillSearch(GS);
    UpdateSortingInfoChangedWithoutVisuals(&GS->Renderer, &GS->MusicInfo, GS->MP3Info);
    RemoveMissingSelected(GS, FromPlaylist, Type, &SelectData);
    UpdateSortingInfoChanged(&GS->Renderer, &GS->MusicInfo, GS->MP3Info, Type);
    
    SavePlaylist(GS, FromPlaylist);
    SnapTimer("RemoveSlotFromPlaylist");
}

internal void
RemoveSlotsFromPlaylist(game_state *GS, column_type Type, array_u32 DisplayableIDs)
{
    RestartTimer("RemoveSlotsFromPlaylist");
    Assert(Type != columnType_Playlists);
    Assert(Type != columnType_None);
    playlist_info     *FromPlaylist = GS->MusicInfo.Playlist_;
    playlist_column *PlaylistColumn = FromPlaylist->Columns + Type;
    
    selection_on_remove SelectData = {};
    CollectSelectedNamesForPrevColumns(GS, FromPlaylist, Type, &SelectData);
    
    array_file_id FileIDs;
    // As we manipulate the array we also read from, we need to copy it.
    FileIDs.A = CreateArray(&GS->ScratchArena, FromPlaylist->Song.FileIDs.A.Length);
    Copy(&FileIDs.A, &FromPlaylist->Song.FileIDs.A);
    
    // We can just use Displayable directly, as we refill it later anyway.
    array_playlist_id *Displayable = &FromPlaylist->Song.Displayable;
    
    For(DisplayableIDs.Count)
    {
        displayable_id DID = NewDisplayableID(Get(&DisplayableIDs, It));
        if(Type == columnType_Song)
        {
            file_id FileID = GetFileID(&GS->MusicInfo, DID);
            StackFindAndTake(&FileIDs.A, FileID.ID);
        }
        else
        {
            array_playlist_id Songs = PlaylistColumn->Batch.Song[Get(&PlaylistColumn->Displayable, DID).ID];
            For(Songs.A.Count)
            {
                playlist_id PlaylistID = Get(&Songs, NewDisplayableID(It));
                if(StackFindAndTake(&Displayable->A, PlaylistID.ID))
                {
                    file_id FileID = NewFileID(Get(&FromPlaylist->Song.FileIDs.A, PlaylistID.ID));
                    StackFindAndTake(&FileIDs.A, FileID.ID);
                }
            }
        }
    }
    
    FillPlaylistWithFileIDs(&GS->MusicInfo, &GS->MP3Info->FileInfo, FromPlaylist, FileIDs);
    UpdatePlaylistScreenName(GS, FromPlaylist);
    SyncPlaylists_playlist_column(&GS->MusicInfo);
    KillSearch(GS);
    UpdateSortingInfoChangedWithoutVisuals(&GS->Renderer, &GS->MusicInfo, GS->MP3Info);
    RemoveMissingSelected(GS, FromPlaylist, Type, &SelectData);
    UpdateSortingInfoChanged(&GS->Renderer, &GS->MusicInfo, GS->MP3Info, Type);
    
    SavePlaylist(GS, FromPlaylist);
    SnapTimer("RemoveSlotsFromPlaylist");
    
}

inline void
AddPlaylistToSortingColumn(music_info *MusicInfo, playlist_id PlaylistID, string_c PlaylistName)
{
    Assert(PlaylistID >= 0);
    
    playlist_column *Playlists  = &MusicInfo->Playlist_->Playlists;
    
    Assert(PlaylistID == Playlists->Batch.BatchCount);
    Assert(Playlists->Batch.BatchCount < Playlists->Batch.MaxBatches);
    Push(&Playlists->Displayable, PlaylistID);
    Playlists->Batch.Names[Playlists->Batch.BatchCount] = NewStringCompound(&GlobalGameState.FixArena, 
                                                                            PLAYLIST_MAX_NAME_LENGTH);
    AppendStringCompoundToCompound(Playlists->Batch.Names+Playlists->Batch.BatchCount++, &PlaylistName);
}

inline void
AddPlaylistToSortingColumn(music_info *MusicInfo, playlist_info *Playlist, string_c PlaylistName)
{
    playlist_id PlaylistID = NewPlaylistID(GetPlaylistID(MusicInfo, Playlist));
    AddPlaylistToSortingColumn(MusicInfo, PlaylistID, PlaylistName);
}

internal void // Without changing visuals
SwitchPlaylistFromPlaylistID(display_column *DisplayColumn, playlist_id PlaylistID)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    
    SyncPlaylists_playlist_column(MusicInfo); // We need to sync before switching to the new playlist.
    
    MusicInfo->Playlist_ = MusicInfo->Playlists.List + PlaylistID.ID;
    MusicInfo->PlayingSong.DisplayableID.ID = -1;
    MusicInfo->PlayingSong.PlaylistID.ID    = -1;
    MusicInfo->PlayingSong.DecodeID         = -1;
    
    ToggleButtonVisuals(MusicInfo->DisplayInfo.ShufflePlaylist, MusicInfo->Playlist_->IsShuffled);
    ToggleButtonVisuals(MusicInfo->DisplayInfo.LoopPlaylist, MusicInfo->Playlist_->Looping);
    
    if(PlaylistID == 0) // Playlist All
    {
        SetDisabled(MusicInfo->DisplayInfo.PlaylistUI.Remove, true, &MusicInfo->DisplayInfo.ColorPalette.ForegroundText);
        SetDisabled(MusicInfo->DisplayInfo.PlaylistUI.Rename, true, &MusicInfo->DisplayInfo.ColorPalette.ForegroundText);
        ResetBtnState(MusicInfo->DisplayInfo.PlaylistUI.Remove);
        ResetBtnState(MusicInfo->DisplayInfo.PlaylistUI.Rename);
    }
    else
    {
        SetDisabled(MusicInfo->DisplayInfo.PlaylistUI.Remove, false, &MusicInfo->DisplayInfo.ColorPalette.ButtonActive);
        SetDisabled(MusicInfo->DisplayInfo.PlaylistUI.Rename, false, &MusicInfo->DisplayInfo.ColorPalette.ButtonActive);
    }
}

inline void // Without changing visuals
SwitchPlaylistFromDisplayID(display_column *DisplayColumn, u32 ColumnDisplayID)
{
    InterruptSearch(&GlobalGameState.Renderer, &GlobalGameState.MusicInfo);
    
    playlist_id PlaylistID = Get(&GlobalGameState.MusicInfo.Playlist_->Playlists.Displayable, 
                                 DisplayColumn->OnScreenIDs[ColumnDisplayID]);
    SwitchPlaylistFromPlaylistID(DisplayColumn, PlaylistID);
}

internal void // With visuals
SwitchPlaylist(game_state *GS, playlist_info *Playlist)
{
    music_display_info *DisplayInfo = &GS->MusicInfo.DisplayInfo;
    
    InterruptSearch(&GS->Renderer, &GS->MusicInfo);
    
    // MoveDisplayColumn fills the OnScreenIDs array, which is needed for switching visuals...
    MoveDisplayColumn(&GS->Renderer, &GS->MusicInfo, &DisplayInfo->Playlists);
    playlist_id PlaylistID = NewPlaylistID(GetPlaylistID(&GS->MusicInfo, Playlist));
    SwitchSelection(&Playlist->Playlists, PlaylistID);
    SwitchPlaylistFromPlaylistID(&DisplayInfo->Playlists, PlaylistID);
    SortDisplayables(&GS->MusicInfo, &GS->MP3Info->FileInfo);
    if(Playlist->IsShuffled) ShufflePlaylist(GS->MusicInfo.Playlist_, true);
    UpdateSortingInfoChangedVisuals(&GS->Renderer, &GS->MusicInfo, DisplayInfo, columnType_Playlists);
}

inline void
SyncPlaylists_playlist_column(music_info *MusicInfo)
{
    For(MusicInfo->Playlists.Count)
    {
        MusicInfo->Playlists.List[It].Playlists = MusicInfo->Playlist_->Playlists;
    }
}

inline void
SyncPlaylists_playlist_column(music_info *MusicInfo, playlist_info *SyncTo)
{
    For(MusicInfo->Playlists.Count)
    {
        if(SyncTo != MusicInfo->Playlists.List + It)
            MusicInfo->Playlists.List[It].Playlists = SyncTo->Playlists;
    }
}

internal void
OnNewPlaylistClick(void *Data)
{
    game_state *GS = (game_state *)Data;
    
    playlist_info *NewPlaylist = CreateEmptyPlaylist(&GS->FixArena, &GS->MusicInfo);
    
    NewLocalString(PlaylistName, 30, "New Playlist");
    I32ToString(&PlaylistName, GS->MusicInfo.Playlists.Count-1);
    AppendStringToCompound(&PlaylistName, (u8 *)" (0)");
    
    
    FillPlaylistWithFileIDs(&GS->MusicInfo, &GS->MP3Info->FileInfo, NewPlaylist, {});
    AddPlaylistToSortingColumn(&GS->MusicInfo, NewPlaylist, PlaylistName);
    UpdateSortingInfoChanged(&GS->Renderer, &GS->MusicInfo, GS->MP3Info, columnType_Playlists);
    
    SyncPlaylists_playlist_column(&GS->MusicInfo);
    SavePlaylist(GS, NewPlaylist);
}

internal void
OnNewPlaylistWithSelectionClick(void *Data)
{
    game_state *GS = (game_state *)Data;
    
    playlist_info *NewPlaylist = CreateEmptyPlaylist(&GS->FixArena, &GS->MusicInfo);
    
    NewLocalString(PlaylistName, 30, "New Playlist");
    I32ToString(&PlaylistName, GS->MusicInfo.Playlists.Count-1);
    AppendStringToCompound(&PlaylistName, (u8 *)" (");
    // If the user has nothing selected in the Song column, then
    // we use the whole song displayable list. If they have something
    // selected, then we only use those.
    u32 Count = 0;
    if(GS->MusicInfo.Playlist_->Song.Selected.A.Count > 0) Count = GS->MusicInfo.Playlist_->Song.Selected.A.Count;
    else                                                   Count = GS->MusicInfo.Playlist_->Song.Displayable.A.Count;
    I32ToString(&PlaylistName, Count);
    AppendCharToCompound(&PlaylistName, ')');
    
    FillPlaylistWithCurrentSelection(&GS->MusicInfo, &GS->MP3Info->FileInfo, NewPlaylist);
    AddPlaylistToSortingColumn(&GS->MusicInfo, NewPlaylist, PlaylistName);
    UpdateSortingInfoChanged(&GS->Renderer, &GS->MusicInfo, GS->MP3Info, columnType_Playlists);
    
    SyncPlaylists_playlist_column(&GS->MusicInfo);
    SwitchPlaylist(GS, NewPlaylist);
    SavePlaylist(GS, NewPlaylist);
}

internal void
DeletePlaylist(game_state *GS, playlist_info *Playlist)
{
    
    DestroyArray(&GS->FixArena, &Playlist->Genre.Selected.A);
    DestroyArray(&GS->FixArena, &Playlist->Artist.Selected.A);
    DestroyArray(&GS->FixArena, &Playlist->Album.Selected.A);
    DestroyArray(&GS->FixArena, &Playlist->Song.Selected.A);
    
    DestroyArray(&GS->FixArena, &Playlist->Genre.Displayable.A);
    DestroyArray(&GS->FixArena, &Playlist->Artist.Displayable.A);
    DestroyArray(&GS->FixArena, &Playlist->Album.Displayable.A);
    DestroyArray(&GS->FixArena, &Playlist->Song.Displayable.A);
    
    if(Playlist->Genre.Batch.BatchCount  > 0) FreeSortBatch(&GS->FixArena, &Playlist->Genre.Batch);
    if(Playlist->Artist.Batch.BatchCount > 0) FreeSortBatch(&GS->FixArena, &Playlist->Artist.Batch);
    if(Playlist->Album.Batch.BatchCount  > 0) FreeSortBatch(&GS->FixArena, &Playlist->Album.Batch);
    if(Playlist->Song.FileIDs.A.Count    > 0) DestroyArray(&GS->FixArena,  &Playlist->Song.FileIDs.A);
}

internal void
RemovePlaylist(game_state *GS, playlist_info *Playlist, b32 DeleteSaveFile)
{
    i32 PlaylistID = GetPlaylistID(&GS->MusicInfo, Playlist);
    Assert(PlaylistID >= 0);
    if(PlaylistID == 0) return; // Cannot delete the first playlist.
    
    DeletePlaylist(GS, Playlist);
    
    RemoveItem(Playlist->Playlists.Batch.Names, Playlist->Playlists.Batch.BatchCount, PlaylistID, string_c);
    --Playlist->Playlists.Batch.BatchCount;
    StackFindAndTake(&Playlist->Playlists.Displayable.A, PlaylistID);
    // Now we need to decrease the value of every entry in displayables by one
    // from all that are higher than PlaylistID to realign the 'ID's again.
    For(Playlist->Playlists.Displayable.A.Count)
    {
        u32 ID = Get(&Playlist->Playlists.Displayable.A, It);
        if(ID > (u32)PlaylistID) 
        {
            Put(&Playlist->Playlists.Displayable.A, It, ID-1);
        }
    }
    SyncPlaylists_playlist_column(&GS->MusicInfo, Playlist);
    SwitchPlaylist(GS, GS->MusicInfo.Playlists.List+(PlaylistID%Playlist->Playlists.Batch.BatchCount));
    
    if(DeleteSaveFile) DeleteFile(&GS->ScratchArena, Playlist->Filename_);
    
    // Needs to be last, as this removes the place where the actual playlist was stored.
    // All accesses need to happen beforehand.
    RemoveItem(GS->MusicInfo.Playlists.List, GS->MusicInfo.Playlists.Count, PlaylistID, playlist_info);
    --GS->MusicInfo.Playlists.Count;
    
    UpdateSortingInfoChanged(&GS->Renderer, &GS->MusicInfo, GS->MP3Info, columnType_Playlists);
}

internal void
OnRemovePlaylistClick(void *Data)
{
    game_state *GS = (game_state *)Data;
    RemovePlaylist(GS, GS->MusicInfo.Playlist_, true);
}

internal void
OnRenamePlaylistClick(void *Data)
{
    game_state          *GS = (game_state *)Data;
    text_field   *TextField = &GS->MusicInfo.DisplayInfo.PlaylistUI.RenameField;
    playlist_info *Playlist = GS->MusicInfo.Playlist_;
    display_column *DColumn = &GS->MusicInfo.DisplayInfo.Playlists;
    i32          OnScreenID = GetOnScreenID(GS, Playlist);
    
    b32 ActivateTextfield = !TextField->IsActive;
    SetActive(TextField, ActivateTextfield);
    // SetActive(DColumn->Text+OnScreenID, !ActivateTextfield); // Actually nice to hover and see original name.
    
    if(ActivateTextfield)
    {
        SetParent(TextField, DColumn->BGRects[OnScreenID]);
        SetSize(TextField->Background, V2(DColumn->SlotWidth, GetSize(TextField->Background).y));
        SetLocalPosition(TextField->LeftAlign, V2(DColumn->SlotWidth*-0.5f, 0));
        ResetStringCompound(TextField->TextString);
        UpdateTextField(&GS->Renderer, TextField);
    }
}

inline void
SetPlaylistRenameActive(game_state *GS, b32 Activate)
{
    text_field *TextField = &GS->MusicInfo.DisplayInfo.PlaylistUI.RenameField;
    if(TextField->IsActive != Activate) OnRenamePlaylistClick(GS);
}

internal void
SaveNewPlaylistName(game_state *GS)
{
    text_field   *TextField = &GS->MusicInfo.DisplayInfo.PlaylistUI.RenameField;
    playlist_info *Playlist = GS->MusicInfo.Playlist_;
    i32 PlaylistID          = GetPlaylistID(&GS->MusicInfo, Playlist);
    
    ResetStringCompound(Playlist->Playlists.Batch.Names[PlaylistID]);
    CopyIntoCompound(Playlist->Playlists.Batch.Names+PlaylistID, &TextField->TextString);
    
    UpdatePlaylistScreenName(GS, Playlist);
    
    SavePlaylist(GS, Playlist);
}

internal void
ShufflePlaylist(playlist_info *Playlist, b32 UsePrevShuffle)
{
    u64 Seed = GetCurrentRandomSeed();
    // If ShuffleSeed is zero, it is the first time being shuffled. Then we
    // always want to shuffle normally, regardless of UsePrevShuffle.
    if(UsePrevShuffle && Playlist->ShuffleSeed != 0) 
        SetRandomSeed(Playlist->ShuffleSeed);
    else Playlist->ShuffleSeed = Seed;
    
    ShuffleStack(&Playlist->Song.Displayable.A);
    
    if(UsePrevShuffle && Playlist->ShuffleSeed != 0) 
        SetRandomSeed(Seed);
}