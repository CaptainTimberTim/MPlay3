#pragma once
// IDEAS::
//
// - think about making song display MDs next to each other (genre in the artist row) an if 
//   it goes over the edge, drop it into the new line
// - ability to change slot height with auto text fit?
// - album image as panel background
// - lyrics window when file exists?
// - think about selecting all entries that are visible when pressing enter during search
// - if song selected, pressing the big play should start it?
// - Think about if search in song column should work differently, or search through Title/artist/album
// - Make a long list of 'goodbyes' for the quit curtain.
// - loading metadata could be "interactive" and all songs already loaded be available already?

// TODO::
//
// - generate huge amount of fake mp3 files and test with those!
// - Solidify playlists
//      - show current playlist button
// - still hardcapped at 10k mp3 files - Fixed, TEST?!
// - go through and remove all unnecassary gamestate/renderer/info juggling
// - Switch openGL to directX?
// - properly round for song panel time and slider

// - remove all GlobalGameState references from UI.c
// - redraw only when necessary!
//      - stop rendering when no user input and no song is playing
// - stop always jumping the column to the start on i.e. search end
// - color picker pops into original center. should at least be current center.
// - UpNextList is limited to 200

// - fix issues regarding handmade network comment
//       - Find issue with drawing order bug. This one I have no clue right now...
// - When having the drag edges close to a side and then making the windows smaller pushes them onto each other

// - On large files when preload is not enough, it _seldom_crashes when using the already decoded data...
// - MP3 V0 crashes?
// - Everywhere where both display_column and sortin_info is given, just give display_column, as it has a pointer to sort.
// - selecting and deselecting stuff (in combination with search) is buggy.

// - Cleanup all the StringCompound procedures... Their names are sooo stupidly long..
// - Print user error when save files could not be correctly loaded.
// - Cleanup of FillDisplayables. Or at least find out what exactly I am doing and comment that!?

// PLAYLIST:
// - what happens when search is open
// - add playlist save file (?), or add to existing save file
// - Add playlist column visuals
//    - Add buttons to delete and create playlits
//    - Add way to remove or add songs to playlists
// - InitialDisplayable count for playlist is capped to 250, should be expandable.
// - Playlist playlist selected array should only be length 1
// - Preload songs when switching playlists.

#include "Sound_UI_TD.h"

struct column_info
{
    struct renderer             *Renderer;
    struct music_display_info   *DisplayInfo;
    struct music_info           *MusicInfo;
    struct music_display_column *DisplayColumn;
    
    struct playlist_column      *PlaylistColumn;
};

enum column_type // Is used to index arrays!
{
    columnType_Genre = 0,
    columnType_Artist,
    columnType_Album,
    columnType_Song,
    columnType_Playlists,
    columnType_None,
};

enum play_loop
{
    playLoop_NoLoop,
    playLoop_Loop,
    playLoop_Repeat
};

struct song_sort_info
{
    u32 GenreBatchID;
    u32 ArtistBatchID;
    u32 AlbumBatchID;
};

struct sort_batch
{
    // These arrays of arrays contain the id for the other columns respectively
    array_batch_id *Genre; 
    array_batch_id *Artist;
    array_batch_id *Album;
    array_playlist_id  *Song;
    
    string_c       *Names; // ::BATCH_ID
    u32 BatchCount;
    u32 MaxBatches;
}; 

struct playlist_column
{
    column_type   Type;
    array_playlist_id Selected;    // stores playlist for song column and sortBatchIDs for the rest!
    array_playlist_id Displayable; // ::DISPLAYABLE_ID,  stores _FileIDs_ for song column and sortBatchIDs for the rest!
    
    union {
        sort_batch Batch;      // Used for Genre, Artist, Album column_types.
        array_file_id FileIDs; // ::FILE_ID Used for Song column_type/Acces this with any playlist_id to get to mp3_file_info.
    };
};

inline struct mp3_metadata *GetMetadata(playlist_column *SongColumn, mp3_file_info *FileInfo, displayable_id ID);
inline struct mp3_metadata *GetMetadata(playlist_column *SongColumn, mp3_file_info *FileInfo, playlist_id ID);
inline string_c *           GetSongFileName(playlist_column *SongColumn, mp3_file_info *FileInfo, playlist_id FileID);

struct playlist_info
{
    union {
        struct {
            playlist_column Genre;
            playlist_column Artist;
            playlist_column Album;
            playlist_column Song;
            // That the playlist information is part of playlist_info is hacky, 
            // but it allows using all the display column stuff, which we want.
            // The biggest drawback for now is, that each new playlist has a
            // _copy_ of this Playlists, which needs to be the same for all.
            // That means we need to keep all playlists in sync. with each other, 
            // which is only keeping the .Counts updated, as all memory pointers
            // point to the same location (so it is not as bad).
            playlist_column Playlists;
        };
        playlist_column Columns[5];
    };
};
internal playlist_info *CreateEmptyPlaylist(arena_allocator *Arena, music_info *MusicInfo, i32 SongIDCount = -1, i32 GenreBatchCount = -1, i32 ArtistBatchCount = -1, i32 AlbumBatchCount = -1);
void SyncPlaylists_playlist_column(music_info *MusicInfo);

struct playlist_array
{
    playlist_info *List;
    u32 Count;
    u32 MaxCount;
};

struct playing_song
{
    displayable_id DisplayableID;
    playlist_id PlaylistID;
    i32 DecodeID;
    
    b32 PlayUpNext; // should only be set in SetNextSong/and OnSongPlayPressed
};

struct music_info
{
    b32 IsShuffled;
    play_loop Looping;
    b32 IsPlaying;
    
    array_playlist_id UpNextList;
    
    playlist_info *Playlist_; // Actual playlist. This can be switched out.
    playlist_array Playlists;
    
    music_display_info DisplayInfo;
    
    playing_song PlayingSong;
    b32 CurrentlyChangingSong;
};

struct scroll_load_info
{
    r32 dTime;
    r32 WaitTime;
    b32 LoadFinished;
};

#define MAX_MP3_INFO_COUNT 1000
#define MAX_MP3_INFO_STEP 1000
#define MAX_MP3_DECODE_COUNT 50
#define DECODE_PRELOAD_SECONDS 5

enum metadata_flags
{
    metadata_Title    = 1<<0,
    metadata_Artist   = 1<<1,
    metadata_Album    = 1<<2,
    metadata_Genre    = 1<<3,
    metadata_Track    = 1<<4,
    metadata_Year     = 1<<5,
    metadata_Duration = 1<<6,
};

struct mp3_metadata
{
    string_compound Title;
    string_compound Artist;
    string_compound Album;
    string_compound Genre;
    u32 Track;
    string_compound TrackString;
    u32 Year;
    string_compound YearString;
    u32 Duration;
    i32 FoundFlags;
};

struct mp3_file_info //::MAPPED FILE_ID
{
    string_c     *FileNames_;
    string_c     *SubPath;
    mp3_metadata *Metadata;
    // NoHash:: u32          *Hashes;  // Hashes for specific file identification.
    u32 Count_;
    u32 MaxCount_;
};

struct playing_decoded
{
    mp3dec_file_info_t Data;
    i32 DecodeID;
    b32 volatile CurrentlyDecoding;
};

struct mp3_decode_info
{
    playing_decoded PlayingDecoded;
    b32 volatile CancelDecoding; // Exclusively written in main thread.
    
    mp3dec_file_info_t DecodedData[MAX_MP3_DECODE_COUNT];
    array_file_id FileIDs; // size: MAX_MP3_DECODE_COUNT
    u32 Count;
    
    b32 volatile CurrentlyDecoding[MAX_MP3_DECODE_COUNT];
    array_u32 LastTouched; // size: MAX_MP3_DECODE_COUNT
    u32 TouchCount;
};

struct mp3_info
{
    string_c        FolderPath;
    mp3_file_info   FileInfo;
    mp3_decode_info DecodeInfo;
    music_info     *MusicInfo;
};


internal void ChangeSong(game_state *GameState, playing_song *Song);

inline displayable_id PlaylistIDToColumnDisplayID(music_info *MusicInfo, music_display_column *DisplayColumn, playlist_id PlaylistID);
internal b32  IsHigherInAlphabet(i32 T1, i32 T2, void *Data);
internal u32  ExtractMetadataSize(arena_allocator *Arena, string_c *CompletePath);

internal void CreatePlaylistsSortingInfo(playlist_column *Playlists);
internal void SwitchPlaylistFromDisplayID(music_display_column *DisplayColumn, u32 ColumnDisplayID);

