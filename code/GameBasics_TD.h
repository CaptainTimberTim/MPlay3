#pragma once
#include "Renderer_TD.h"
#include "Sound_Backend_TD.h"
#include "Sound_TD.h"

global_variable string_c SETTINGS_FILE_NAME NewStaticStringCompound("MPlay3Settings.save");

#define LIBRARY_CURRENT_VERSION 3
#define SETTINGS_CURRENT_VERSION 4
struct settings
{
    r32 Volume = 0.5f;
    file_id PlayingSongID = {-1};
    u32 ColorPaletteID = 0;
    r32 GenreArtistEdgeXPercent = -1.0f;
    r32 ArtistAlbumEdgeXPercent = -1.0f;
    r32 AlbumSongEdgeXPercent = -1.0f;
    i32 WindowDimX = 1416;
    i32 WindowDimY = 1039;
    b32 Looping = false;
    b32 Shuffle = false;
    
    string_c *PaletteNames;
    color_palette *Palettes;
    u32 PaletteCount;
    u32 PaletteMaxCount;
};

enum cursor_state
{
    cursorState_Arrow,
    cursorState_Drag,
};

struct timer
{
    u32 Count;
    i64 Start;
    i64 LastSnap;
};

struct palette_color
{
    entry_id *Outline;
    entry_id *Preview;
    render_text Name;
};

struct color_picker
{
    b32 IsActive;
    
    loaded_bitmap Bitmap;
    u32 GLID; 
    entry_id *TextureEntry;
    b32 IsPickingColor;
    
    slider ColorSpectrum;
    render_text RGBText;
    
    entry_id *PickDot;
    entry_id *InnerDot;
    entry_id *InnerInnerDot;
    v3 SelectedColor;
    
    entry_id *MoveNob;
    b32 IsMoving;
    v2 MoveOffset;
    entry_id *Background;
    v2 _BGOffset;
    
    palette_color PaletteColors[PALETTE_COLOR_AMOUNT];
    u32 ActiveColor;
    entry_id *ActiveColorBG;
    u32 CurrentColorPaletteID;
    text_field PaletteName;
    
    button *New;
    button *Save;
    button *Cancel;
    button *Remove;
    quit_animation NewAnim;
    quit_animation SaveAnim;
    quit_animation RemoveAnim;
};

struct game_state
{
    string_c DataPath;
    bucket_allocator Bucket;
    
    input_info Input;
    
    settings Settings;
    
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
    
    color_picker ColorPicker;
    
    thread_error_list ThreadErrorList;
};

internal loaded_bitmap LoadImage_STB(u8 *Path);
inline void FreeImage_STB(loaded_bitmap Bitmap);

#if DEBUG_TD
inline timer StartTimer();
inline void SnapTimer(timer *Timer);
#else
#define StartTimer() {}
#define SnapTimer(f) 
#endif

// Color Picker
inline void SetActive(color_picker *ColorPicker, b32 Activate);
