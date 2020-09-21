#pragma once
// TODO::
//
// - think about making song display MDs next to each other (genre in the artist row) an if 
//   it goes over the edge, drop it into the new line
// - sub-pixel accuracy? 
// - ability to change slot height with auto text fit?
// - add option to rescan files for new .mp3 added by user
// - album image as panel background
// - redraw only when necessary!
// - lyrics window when file exists?
//
// - generate huge amount of fake mp3 files and test with those!
// - show current playlist button
// - files bigger than 64MB just assert
// - invalid songs in library checker
// - keep currently playing list
// - make render text message logging on top of window!
// - -/+ for volume
// - fix all the non-ascii problems...
// - add a keyboard button which toggles the shortcuts for every element
// - solidify playlist stuff?
// - switch fileID being a u32 to a struct file_id
// - think about doing more than just a quad for text?
// - make a seperate thread for metadata crawl
// - think about selecting all entries that are visible when pressing enter during search

// - still hardcapped atz 10k mp3 files
// - go through and remove all gamestate/renderer/info juggling
// - if glyphs not existing for font, use a backup font?
// - stop rendering when no user input and no song is playing
// - Switch openGL to directX?
// - Add "want to quit" popup when esc. OR, make animation that slides down window or something when holding escape pressed!
// - Add key shortcut button
// - double click song select?
//      - overhaul song select, is not working sometimes.
//        May have to do with color update?
//      - if song selected, pressing the big play should start it

#include "Sound_UI_TD.h"

global_variable string_c LIBRARY_FILE_NAME = NewStaticStringCompound("MPlay3Library.save");
global_variable u32 CURRENT_LIBRARY_VERSION = 3;

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
    array_u32 *Genre;
    array_u32 *Artist;
    array_u32 *Album;
    array_u32 *Song;
    
    string_c  *Names;
    u32 BatchCount;
    u32 MaxBatches;
}; 

struct column_sorting_info
{
    struct music_sorting_info *Base;
    sort_batch Batch;
    array_u32 Selected;
    array_u32 Displayable; // Stores PlaylistIDs
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
    i32 PlaylistID;
    i32 FileID;
    i32 DecodeID;
    
    b32 PlayUpNext; // should only be set in SetNextSong
};

struct play_list
{
    array_u32 Songs; 
    array_u32 UpNext;
    array_u32 Previous;
};

struct music_info
{
    b32 IsShuffled;
    play_loop Looping;
    b32 IsPlaying;
    
    play_list Playlist;
    
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

struct mp3_file_info
{
    string_c     *FileName;
    string_c     *SubPath;
    mp3_metadata *Metadata;
    u32 Count;
    u32 MaxCount;
};

struct mp3_decode_info
{
    mp3dec_file_info_t DecodedData[MAX_MP3_DECODE_COUNT];
    array_u32 FileID; // size: MAX_MP3_DECODE_COUNT
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


// Threading

struct job_load_decode_mp3
{
    mp3_info *MP3Info;
    u32 FileID;
    i32 DecodeID;
};


struct crawl_thread_out
{
    b32 ThreadIsRunning;
    b32 DoneFolderSearch;
    b32 DoneCrawling;
    
    u32 TestCount;
    u32 CurrentCount;
};

struct crawl_thread
{
    mp3_info *MP3Info;
    string_c TestPath;
    
    crawl_thread_out *Out;
};

struct check_music_path
{
    thread_state State;
    mp3_info *MP3Info;
    mp3_file_info TestInfo;
    array_u32 RemoveIDs;
    array_u32 AddTestInfoIDs;
};


internal void ChangeSong(game_state *GameState, playing_song *Song);
internal i32 AddJob_LoadMP3(game_state *GameState, circular_job_queue *JobQueue, i32 FileID, array_u32 *IgnoreDecodeIDs = 0);
inline i32 FileIDToColumnDisplayID(music_display_column *DisplayColumn, i32 FileID);
internal b32 AddJob_NextUndecodedInPlaylist();
internal void AddJob_CheckMusicPathChanged(check_music_path *CheckMusicPath);


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
