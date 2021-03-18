#ifndef _SOUND__U_I__T_D_H
#define _SOUND__U_I__T_D_H

#include "Sound_General_TD.h"

#define MAX_DISPLAY_SONG_COUNT   24
#define MAX_DISPLAY_COUNT  80
#define SLOT_DISTANCE_PIXEL 3

#define DOUBLE_CLICK_TIME 0.5f

#define DEFAULT_COLOR_PALETTE_COUNT 5 // Amount of complete color palettes that currently exist.
#define PALETTE_COLOR_AMOUNT 10 // Amount of colors per palette.
union color_palette
{
    // If a new color is added, increment the PALETTE_COLOR_AMOUNT define
    struct
    {
        v3 Text; // 1
        v3 ErrorText; // 2
        v3 ForegroundText; // 3
        v3 Foreground; // 4
        v3 Slot; // 5
        v3 SliderBackground; // 6
        v3 SliderGrabThing; // 7
        v3 ButtonActive; // 8
        v3 Selected; // 9
        v3 PlayingSong; // 10
    };
    v3 Colors[PALETTE_COLOR_AMOUNT];
};
global_variable string_c GlobalPaletteColorNames[] = 
{
    NewStaticStringCompound("Text"),
    NewStaticStringCompound("Error Text"),
    NewStaticStringCompound("Foreground Text"),
    NewStaticStringCompound("Foreground"),
    NewStaticStringCompound("Slot"),
    NewStaticStringCompound("Slider Background"),
    NewStaticStringCompound("Slider Grab Thing"),
    NewStaticStringCompound("Button Active"),
    NewStaticStringCompound("Selected"),
    NewStaticStringCompound("Playing Song"),
};
global_variable string_c GlobalDefaultColorPaletteNames[] = 
{
    NewStaticStringCompound("Lush Green"),
    NewStaticStringCompound("Smoldering Red"),
    NewStaticStringCompound("Aquatic Blue"),
    NewStaticStringCompound("Deep Night Grey"),
    NewStaticStringCompound("Monochrome Grey"),
};

struct drag_slider_data
{
    struct music_info *MusicInfo;
    struct music_display_column *DisplayColumn;
    struct column_sorting_info *SortingColumn;
};

struct search_bar_btn_info
{
    struct renderer *Renderer;
    struct music_display_info *DisplayInfo;
    struct search_bar *Search;
    struct music_display_column *DisplayColumn;
    struct array_file_id *Displayable;
    struct column_sorting_info *SortingInfo;
};

struct search_bar
{
    text_field TextField;
    button *Button;
    array_file_id InitialDisplayables;
};

struct music_display_column
{
    struct music_display_info *Base;
    enum column_type Type;
    
    r32 SlotHeight;
    r32 SlotWidth;
    r32 ColumnHeight;
    u32 Count;
    displayable_id OnScreenIDs [MAX_DISPLAY_COUNT];
    render_text Text    [MAX_DISPLAY_COUNT];
    entry_id *BGRects   [MAX_DISPLAY_COUNT];
    entry_id *Background;
    r32 TextX;
    r32 DisplayCursor; // Y position in the total displayables
    entry_id *BGRectAnchor; // Is only moved on resize, to be at the exact pos for the first BGRect.
    r32 ZValue;
    
    entry_id *LeftBorder;
    entry_id *RightBorder;
    entry_id *TopBorder;
    entry_id *BottomBorder;
    struct column_sorting_info *SortingInfo;
    
    slider SliderHorizontal;
    slider SliderVertical;
    
    entry_id *BetweenSliderRect;
    search_bar Search;
    search_bar_btn_info SearchInfo;
    
    r64 LastClickTime;
    i32 LastClickSlotID;
};

struct song_play_btn
{
    u32 DisplayID;
    struct game_state *GameState;
};

struct display_column_song_extension
{
    music_display_column Base;
    
    render_text SongTitle  [MAX_DISPLAY_SONG_COUNT];
    render_text SongArtist [MAX_DISPLAY_SONG_COUNT];
    render_text SongAlbum  [MAX_DISPLAY_SONG_COUNT];
    render_text SongGenre  [MAX_DISPLAY_SONG_COUNT];
    render_text SongTrack  [MAX_DISPLAY_SONG_COUNT];
    render_text SongYear   [MAX_DISPLAY_SONG_COUNT];
    
    button *Play[MAX_DISPLAY_SONG_COUNT];
    struct song_play_btn PlayBtnData[MAX_DISPLAY_SONG_COUNT];
    button  *Add[MAX_DISPLAY_SONG_COUNT];
    u32 PlayPauseGLID;
    u32 AddGLID;
    
    struct mp3_file_info *FileInfo;
};
#define Parent(name) &(name)->Base
#define ColumnExt(name)  ((display_column_song_extension *)(name))

#define SONG_TITLE_X_OFFSET 55
#define SONG_ARTIST_X_OFFSET 105
#define SONG_ALBUM_X_OFFSET  (SONG_ARTIST_X_OFFSET+70)
#define SONG_TRACK_X_OFFSET  -5
#define SONG_PLAY_BUTTON_X_OFFSET  40
#define SONG_PLAY_BUTTON_Y_OFFSET  -22
#define SONG_FIRST_ROW_Y_OFFSET  27
#define SONG_SECOND_ROW_Y_OFFSET -8
#define SONG_THIRD_ROW_Y_OFFSET -33

struct playing_song_panel
{
    struct mp3_info *MP3Info;
    r32 SongDuration;
    
    render_text DurationText;
    render_text CurrentTimeText;
    string_c CurrentTimeString;
    u32 CurrentTime;
    render_text Title;
    render_text Artist;
    render_text Genre;
    render_text Album;
    render_text Year;
    render_text Track;
    
    slider Timeline;
    b32 TimelineFreezeChange;
};

struct drag_edge
{
    entry_id *Edge;
    r32 XPercent;
    r32 OriginalYHeight;
};

struct music_btn
{
    struct game_state *GameState;
    struct playing_song *PlayingSong;
};

struct music_path_ui
{
    button *Button; 
    text_field TextField;
    
    render_text CurrentPath;
    render_text Output;
    string_c OutputString;
    entry_id *Background;
    button *Save;
    button *Quit;
    
    i32 CrawlThreadStateCount;
    r32 dWaitThenCancelTime;
    
    loading_bar LoadingBar;
    
    // Rescan
    button *Rescan;
};

struct user_error_text
{
    render_text Message;
    b32 IsAnimating;
    r32 dAnim;
    r32 AnimTime;
};

struct shortcut_popups
{
    string_c PaletteSwap; // 1
    string_c ColorPicker; // 2
    string_c MusicPath; // 3
    string_c SongPlay; // 4
    string_c SongAddToNextUp; // 5
    string_c SearchGenre; // 6
    string_c SearchArtist; // 7
    string_c SearchAlbum; // 8
    string_c SearchSong; // 9
    string_c Loop; // 10
    string_c Shuffle; // 11
    string_c PlayPause; // 12
    string_c Stop; // 13
    string_c Previous; // 14
    string_c Next; // 15
    string_c Volume; // 16
    string_c Timeline; // 17
    string_c Help; // 18
    string_c Quit; // 19
    string_c SaveMusicPath; // 20
    string_c CancelMusicPath; // 21
    string_c RescanMetadata; // 22
    string_c SavePalette; // 23
    string_c CopyPalette; // 24
    string_c DeletePalette; // 25
    string_c CancelPicker; // 26
    u32 ActiveText; // Is one of the values from the strings above (1-26).
    b32 IsHovering;
    
    popup Popup;
    b32 IsActive;
};

struct music_display_info
{
    // General
    music_display_column Genre;
    music_display_column Artist;
    music_display_column Album;
    display_column_song_extension Song;
    
    color_palette ColorPalette;
    u32 ColorPaletteID;
    
    i32 SearchIsActive;
    
    // Foreground
    entry_id *EdgeBottom;
    entry_id *EdgeTop;
    entry_id *EdgeLeft;
    entry_id *EdgeRight;
    
    // Button general
    music_btn MusicBtnInfo;
    
    drag_edge GenreArtist;
    drag_edge ArtistAlbum;
    drag_edge AlbumSong;
    
    // Control Panel
    button *PlayPause;
    button *Stop;
    button *Previous;
    button *Next;
    button *PaletteSwap;
    button *ColorPicker;
    button *Help;
    shortcut_popups Popups;
    slider Volume;
    
    // Music play control
    button *LoopPlaylist; 
    button *ShufflePlaylist;
    
    playing_song_panel PlayingSongPanel;
    music_path_ui MusicPath;
    
    quit_animation Quit;
    
    user_error_text UserErrorText;
};



inline void UpdateColumnColor(music_display_column *DisplayColumn, struct column_sorting_info *SortingColumn);
internal void BringDisplayableEntryOnScreen(music_display_column *DisplayColumn, file_id FileID);
internal void BringDisplayableEntryOnScreenWithSortID(music_display_column *DisplayColumn, batch_id SortID);
inline void ToggleSelection(music_display_column *DisplayColumn, column_sorting_info *SortColumn, u32 ColumnDisplayID);
internal void UpdatePlayingSongColor(music_display_column *DisplayColumn, column_sorting_info *SortColumnInfo, u32 FileID, v4 *Color);
internal void KeepPlayingSongOnScreen(renderer *Renderer, struct music_info *MusicInfo);
internal b32 UpdateDisplayColumn_(renderer *Renderer, music_info *MusicInfo, music_display_column *DisplayColumn,
                                  column_sorting_info *SortingColumn, i32 ScrollAmount);
internal void ScrollDisplayColumn(renderer *Renderer, music_display_column *DisplayColumn, r32 ScrollAmount);
internal void SetTheNewPlayingSong(renderer *Renderer, playing_song_panel *Panel, music_info *MusicInfo);


internal void SearchInDisplayable(column_sorting_info *ColumnSortInfo, struct search_bar *Search, mp3_file_info *FileInfo = 0);
internal void UpdateColumnVerticalSlider(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *ColumnSorting);
inline void PushUserErrorMessage(string_c *String);







#endif //_SOUND__U_I__T_D_H
