#pragma once
#include "Renderer_TD.h"
#include "Sound_Thread_TD.h"
#include "Sound_Backend_TD.h"
#include "Sound_Jobs.h"
#include "Sound_Serialization.h"


enum cursor_state
{
    cursorState_Arrow,
    cursorState_Drag,
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
    
    button *Cancel;
    button *New;
    button *Save;
    button *Remove;
    quit_animation NewAnim;
    quit_animation SaveAnim;
    quit_animation RemoveAnim;
};

enum color_picker_anim_btn
{
    colorPickerAnimBtn_New,
    colorPickerAnimBtn_Save,
    colorPickerAnimBtn_Remove
};

struct game_state
{
    string_c SettingsPath;
    string_c LibraryPath;
    string_c PlaylistPath;
    string_c FontPath;
    
    arena_allocator FixArena;     // Never gets reset (except if a bucket is emptied again)
    arena_allocator ScratchArena; // Gets reset every frame.
    
    layout_definition Layout;
    
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
    arena_allocator SoundThreadArena;
    sound_thread_interface *SoundThreadInterface;
    
    arena_allocator JobThreadsArena;
    HANDLE JobHandles[THREAD_COUNT];
    job_thread_info JobInfos[THREAD_COUNT];
    circular_job_queue JobQueue;
    
    crawl_thread CrawlInfo;
    check_music_path CheckMusicPath;
    
    color_picker ColorPicker;
    
    thread_error_list ThreadErrorList;
};

internal loaded_bitmap LoadImage_STB(u8 *Path);
internal loaded_bitmap LoadImage_STB(read_file_result Memory);
inline void FreeImage_STB(loaded_bitmap Bitmap);

#if DEBUG_TD
struct timer
{
    u32 Count;
    i64 LastSnap;
    i64 Total;
    b32 Paused;
};

#define TIMER_MAX_COUNT 200
global_variable hash_table _debugTimerTable = {};
global_variable timer      _debugTimers[TIMER_MAX_COUNT] = {};
global_variable u32        _debugTimerCount = 0;

inline void InitTimers();
inline void _StartTimer(u8 *Name);
#define StartTimer(Name)   _StartTimer((u8 *)(Name))   /* Starts a new timer, if it exists already, it gets unpaused. */
inline void _RestartTimer(u8 *Name);
#define RestartTimer(Name) _RestartTimer((u8 *)(Name)) /* Let's you restart a timer with the given name. */
inline void _PauseTimer(u8 *Name);
#define PauseTimer(Name) _PauseTimer((u8 *)(Name)) /* Stops the timer, can be unpaused with StartTimer. */
inline void _SnapTimer(u8 *Name);
#define SnapTimer(Name)    _SnapTimer((u8 *)(Name)) /* Creates a snapshot and prints the time since last snapshot and total.*/

#else
#define InitTimers() 
#define StartTimer(n)
#define RestartTimer(n)
#define PauseTimer(n)
#define SnapTimer(n)
#endif

// Color Picker
inline void SetActive(color_picker *ColorPicker, b32 Activate);
