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
// - loading metadata could be "interactive" and all songs already loaded be available already?
// - adjust many sizes, text for example is too big imo. Which happened because of the bad rendering, which is now fixed.

// List of Tags to search for in the code:
// - TODO::    , Stuff where I want to go and make something better later/clean it up/make it more save/etc.
// - @HardLimit, is a label where I set a hard limit, the user could exceed and needs to be dealt with.
// - @SLOW     , is a label where I think it's unnecessary slow.
// - @Layout   , where the Layout struct should hold the magic values.
//
// - generate huge amount of fake mp3 files and test with those!
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
// - Cleanup of FillDisplayables. Or at least find out what exactly I am doing and comment that!?
// - Sort out depth everywhere in the application
// - Fix stutter on first play of song.
// - Make it possible to close colorpicker with the colorpicker button
// - Add a on_screen_id for tha visuals, to have typechecking on it! (like displayable_id, etc.).
// - glScissor is the keyword for cutting of text after column!
// - Print user error when save files could not be correctly loaded.

// PLAYLIST:
// - make dragged song slot small like the other columns, after it is ripped off?
// - Should 'Rename' button still work for 'All' as it isn't really required to be called that.
// - Add drag&drop for sorting playlist slots?

// - Add tooltips for new stuff.
// - InitialDisplayable count for playlist is capped to 250, should be expandable.
// - SavePlaylist:: It seems the subpathing is not perfectly sorted. Maybe bunch them up and try to have each subpath only once.
// - SavePlaylist:: When we use this now. It is possible that a file already exists with this name and we just overwrite it. 
// - Save active playlist in settings (@TemporaryDisable::).

#include "Sound_UI_TD.h"

struct column_info
{
    struct renderer             *Renderer;
    struct music_display_info   *DisplayInfo;
    struct music_info           *MusicInfo;
    struct display_column *DisplayColumn;
    
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

#define PLAYLIST_MAX_NAME_LENGTH 100
struct sort_batch
{
    // These arrays of arrays contain the id for the other columns respectively.
    // Example for ArtistSortBatch: Band AC/DC is the first entry in Names, index 0. 
    // The genre array with index 0 contains all genres which AC/DC had in their 
    // metadata (i.e. heavy metal). The Artist array is empty, as the sort_batch
    // itself is for artists. Album contains all of their albums at index 0 and
    // finally, Song contains all songs. The entries in these arrays are all indexes
    // for the corresponding other batch. Genre contains indexes to the "Names" field
    // in the Genre sort_batch, and so forth.
    array_batch_id   *Genre; 
    array_batch_id  *Artist;
    array_batch_id   *Album;
    array_playlist_id *Song;
    
    string_c         *Names; // ::BATCH_ID
    u32 BatchCount;
    u32 MaxBatches;
}; 

struct playlist_column
{
    column_type   Type;
    array_playlist_id Selected;    // stores playlist for song column and sortBatchIDs for the rest!
    array_playlist_id Displayable; // ::DISPLAYABLE_ID,  stores _PlaylistIDs_ for song column and sortBatchIDs for the rest!
    
    union {
        sort_batch Batch;      // Used for Genre, Artist, Album, Playlists column_types.
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
            // This is done with the procedure "SyncPlaylists_playlist_column".
            playlist_column Playlists;
        };
        playlist_column Columns[5];
    };
    string_c Filename;
};
internal playlist_info *CreateEmptyPlaylist(arena_allocator *Arena, music_info *MusicInfo, i32 SongIDCount = -1, i32 GenreBatchCount = -1, i32 ArtistBatchCount = -1, i32 AlbumBatchCount = -1);
void SyncPlaylists_playlist_column(music_info *MusicInfo);

struct playlist_array
{
    // The ID for this List is the same for Names in sort_batch.
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

inline displayable_id PlaylistIDToColumnDisplayID(music_info *MusicInfo, display_column *DisplayColumn, playlist_id PlaylistID);
internal b32  IsHigherInAlphabet(i32 T1, i32 T2, void *Data);
internal u32  ExtractMetadataSize(arena_allocator *Arena, string_c *CompletePath);

internal void CreatePlaylistsSortingInfo(playlist_column *Playlists);
internal void SwitchPlaylistFromDisplayID(display_column *DisplayColumn, u32 ColumnDisplayID);

