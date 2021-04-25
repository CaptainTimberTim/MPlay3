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

// TODO::
//
// - generate huge amount of fake mp3 files and test with those!
// - sub-pixel accuracy? 
// - Solidify playlists
//      - show current playlist button
// - if glyphs not existing for font, use a backup font?
//      - fix all the non-ascii problems...
// - think about doing more than just a quad for text?
// - still hardcapped at 10k mp3 files
// - go through and remove all unnecassary gamestate/renderer/info juggling
// - Switch openGL to directX?
// - Add key shortcut button
// - properly round for song panel time and slider
// - fix track number alignment for 100th

// - remove all GlobalGameState references from UI.c
// - redraw only when necessary!
//      - stop rendering when no user input and no song is playing
// - stop always jumping the column to the start on i.e. search end
// - color picker pops into original center. should at least be current center.


// - fix issues regarding handmade network comment
//       - Find issue with drawing order bug. This one I have no clue right now...
// - When having the drag edges close to a side and then making the windows smaller pushes them onto each other
// - maybe have the icons in the exact size we use them...
// - two color palettes get created when coying one.

// - MP3 V0 crashes?
// - Everywhere where both display_column and sortin_info is given, just give display_column, as it has a pointer to sort.

#include "Sound_UI_TD.h"

global_variable string_c LIBRARY_FILE_NAME = NewStaticStringCompound("MPlay3Library.save");
global_variable u32 CURRENT_LIBRARY_VERSION = 3;

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
    columnType_None,
};

struct sort_batch
{
    // These arrays of arrays contain the id for the other columns respectively
    array_batch_id *Genre; 
    array_batch_id *Artist;
    array_batch_id *Album;
    array_file_id  *Song;
    
    string_c       *Names; // ::BATCH_ID
    u32 BatchCount;
    u32 MaxBatches;
}; 

struct song_sort_info
{
    u32 GenreBatchID;
    u32 ArtistBatchID;
    u32 AlbumBatchID;
};

enum play_loop
{
    playLoop_NoLoop,
    playLoop_Loop,
    playLoop_Repeat
};

struct playing_song
{
    playlist_id PlaylistID;
    file_id FileID;
    i32 DecodeID;
    
    b32 PlayUpNext; // should only be set in SetNextSong/and OnSongPlayPressed
};

struct playlist_column
{
    column_type   Type;
    array_file_id Selected;    // stores _FileIDs_ for song column and sortBatchIDs for the rest!
    array_file_id Displayable; // ::DISPLAYABLE_ID,  stores _FileIDs_ for song column and sortBatchIDs for the rest!
    
    union {
        sort_batch Batch;      // Used for Genre, Artist, Album column_types.
        array_file_id FileIDs; // Used for Song column_type.
    };
};

inline struct mp3_metadata *GetMetadata(playlist_column *SongColumn, mp3_file_info *FileInfo, displayable_id ID);

struct playlist_info
{
    union {
        struct {
            playlist_column Genre;
            playlist_column Artist;
            playlist_column Album;
            playlist_column Song;
        };
        playlist_column Columns[4];
    };
};
internal playlist_info *CreateEmptyPlaylist(arena_allocator *Arena, music_info *MusicInfo, u32 FileInfoCount, 
                                            i32 GenreBatchCount = -1, i32 ArtistBatchCount = -1, i32 AlbumBatchCount = -1);

struct playlist_array
{
    playlist_info *List;
    u32 Count;
    u32 MaxCount;
};

struct play_list // TODO::PLAYLIST_DISPLAYABLE -> everywhere were this is used as the same thing
{ // TODO:: Rename this to something like prev_and_upcoming_songs...
    array_file_id Songs; // TODO:: This can be "Displayable" again
    array_file_id UpNext;
    array_file_id Previous;
};

struct music_info
{
    b32 IsShuffled;
    play_loop Looping;
    b32 IsPlaying;
    
    play_list Playlist;  // ::PLAYLIST_ID
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

#define MAX_MP3_INFO_COUNT 10000
#define MAX_MP3_DECODE_COUNT 50
#define DECODE_PRELOAD_SECONDS 5

enum metadata_flags
{
    metadata_Title  = 1<<0,
    metadata_Artist = 1<<1,
    metadata_Album  = 1<<2,
    metadata_Genre  = 1<<3,
    metadata_Track  = 1<<4,
    metadata_Year   = 1<<5,
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
    string_compound DurationString;
    i32 FoundFlags;
};

struct mp3_file_info //::FILE_ID
{
    string_c     *FileName;
    string_c     *SubPath;
    mp3_metadata *Metadata; // TODO:: Change name and fix all places where now FileIDs in playlist_info should be used...
    u32 Count;
    u32 MaxCount;
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
    array_file_id FileID; // size: MAX_MP3_DECODE_COUNT
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

inline displayable_id FileIDToColumnDisplayID(music_info *MusicInfo, music_display_column *DisplayColumn, file_id FileID);
internal b32  IsHigherInAlphabet(i32 T1, i32 T2, void *Data);
internal u32  ExtractMetadataSize(arena_allocator *Arena, string_c *CompletePath);



