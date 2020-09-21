#pragma once
#include "Renderer_TD.h"
#include "Sound_Backend_TD.h"
#include "Sound_TD.h"

global_variable string_c SETTINGS_FILE_NAME NewStaticStringCompound("MPlay3Settings.save");

#define LIBRARY_CURRENT_VERSION 3
#define SETTINGS_CURRENT_VERSION 3
struct settings
{
    r32 Volume = 0.5f;
    i32 PlayingSongID = -1;
    u32 ColorPaletteID = 0;
    r32 GenreArtistEdgeXPercent = -1.0f;
    r32 ArtistAlbumEdgeXPercent = -1.0f;
    r32 AlbumSongEdgeXPercent = -1.0f;
    i32 WindowDimX = 1416;
    i32 WindowDimY = 1039;
    b32 Looping = false;
    b32 Shuffle = false;
};

enum cursor_state
{
    cursorState_Arrow,
    cursorState_Drag,
};

struct game_state
{
    string_c DataPath;
    bucket_allocator Bucket;
    
    input_info Input;
    
    // Time management
    time_management Time;
    
    renderer Renderer;
    music_info MusicInfo;
    mp3_info *MP3Info;
    
    drag_list DragableList;
    cursor_state CursorState;
    
    // Threading stuff
    bucket_allocator SoundThreadBucket;
    sound_thread_interface *SoundThreadInterface;
    circular_job_queue JobQueue;
    
    crawl_thread CrawlInfo;
    check_music_path CheckMusicPath;
};

internal loaded_bitmap LoadImage_STB(u8 *Path);
inline void FreeImage_STB(loaded_bitmap Bitmap);