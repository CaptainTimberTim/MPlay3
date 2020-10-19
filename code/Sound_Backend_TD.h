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

// TODO::
//
// - generate huge amount of fake mp3 files and test with those!
// - sub-pixel accuracy? 
// - redraw only when necessary!
//      - stop rendering when no user input and no song is playing
// - Solidify playlists
//      - show current playlist button
// - -/+ for volume
// - if glyphs not existing for font, use a backup font?
//      - fix all the non-ascii problems...
// - think about doing more than just a quad for text?
// - files bigger than 64MB*2 just assert
//      - skyrim atmospheres is 74MB as mp3 and decompressed ~400MB! Backend thread has not enough memory for that!
// - fix cursor change on column edge drag
// - still hardcapped at 10k mp3 files
// - go through and remove all unnecassary gamestate/renderer/info juggling
// - Switch openGL to directX?
// - Add key shortcut button
// - Implement partially decoding mp3s
//      - Implement better memory management
// - properly round for song panel time and slider

// - remove all GlobalGameState references from UI.c

#include "Sound_UI_TD.h"

global_variable string_c LIBRARY_FILE_NAME = NewStaticStringCompound("MPlay3Library.save");
global_variable u32 CURRENT_LIBRARY_VERSION = 3;

#define DECODE_STREAMING_TMP_
#define CURRENTLY_SUPPORTED_MAX_DECODED_FILE_SIZE Gigabytes(1)

enum column_type
{
    columnType_None,
    columnType_Song,
    columnType_Genre,
    columnType_Artist,
    columnType_Album,
};

struct sort_batch
{
    // These arrays contain the id for the other columns respectively
    array_batch_id *Genre; 
    array_batch_id *Artist;
    array_batch_id *Album;
    array_file_id  *Song;
    
    string_c       *Names; // ::BATCH_ID
    u32 BatchCount;
    u32 MaxBatches;
}; 

struct column_sorting_info
{
    struct music_sorting_info *Base;
    sort_batch Batch;
    array_file_id Selected; // stores _FileIDs_ for song column and sortBatchIDs for the rest!
    array_file_id Displayable; // ::DISPLAYABLE_ID,  stores _FileIDs_ for song column and sortBatchIDs for the rest!
};

struct music_sorting_info
{
    column_sorting_info Genre;
    column_sorting_info Artist;
    column_sorting_info Album;
    column_sorting_info Song;
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

struct play_list // TODO::PLAYLIST_DISPLAYABLE -> everywhere were this is used as the same thing
{
    array_file_id Songs;
    array_file_id UpNext;
    array_file_id Previous;
};

struct music_info
{
    b32 IsShuffled;
    play_loop Looping;
    b32 IsPlaying;
    
    play_list Playlist;  // ::PLAYLIST_ID
    
    music_sorting_info SortingInfo;
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
#define MAX_MP3_DECODE_COUNT 10
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
    mp3_metadata *Metadata;
    u32 Count;
    u32 MaxCount;
};

struct mp3_decode_info
{
    mp3dec_file_info_t PlayingDecoded;
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

inline displayable_id FileIDToColumnDisplayID(music_display_column *DisplayColumn, file_id FileID);
internal b32  IsHigherInAlphabet(i32 T1, i32 T2, void *Data);
internal u32  ExtractMetadataSize(arena_allocator *Arena, string_c *CompletePath);



global_variable string_compound GenreTypes[] =
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
