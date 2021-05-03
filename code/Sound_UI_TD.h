#ifndef _SOUND__U_I__T_D_H
#define _SOUND__U_I__T_D_H

#include "Sound_General_TD.h"

#define MAX_DISPLAY_SONG_COUNT   24
#define MAX_DISPLAY_COUNT  80

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
};

struct search_bar_btn_info
{
    struct renderer   *Renderer;
    struct search_bar *Search;
    struct music_info *MusicInfo;
    struct music_display_column *DisplayColumn;
    enum column_type   ColumnType;
};

struct search_bar
{
    text_field TextField;
    button *Button;
    array_playlist_id InitialDisplayables;
};

struct column_colors
{
    v3 *Slot;
    v3 *Text;
    v3 *Background;
    v3 *SliderGrabThing;
    v3 *SliderBG;
    v3 *Selected;
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
    
    column_colors Colors;
    
    entry_id *LeftBorder;
    entry_id *RightBorder;
    entry_id *TopBorder;
    entry_id *BottomBorder;
    
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

struct edge_chain
{
    // These arrays are a list of consecutive edges 
    // that can be dragged and should be pushed by
    // each other. For now we have only max 3, so
    // that is the limit. These _need_ to have at
    // least a length of 1 -as the bordering edge- 
    // where the dragged edge has its limit.
#define EDGE_CHAIN_MAX_COUNT 4
    entry_id *Edges[EDGE_CHAIN_MAX_COUNT];
    r32     Offsets[EDGE_CHAIN_MAX_COUNT];
    r32   *XPercent[EDGE_CHAIN_MAX_COUNT-1];
    u32 Count;
};
struct column_edge_drag
{
    edge_chain LeftEdgeChain;
    edge_chain RightEdgeChain;
    r32 *XPercent;
    
    struct music_info *MusicInfo;
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
    
    music_display_column Playlists;
    
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
    
    drag_edge PlaylistsGenre;
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

struct layout_definition
{
    // General
    r32 TopBorder     = 50;
    r32 BottomBorder  = 120;
    r32 LeftBorder    = 24;
    r32 RightBorder   = 24;
    
    // Drag and slide
    r32 DragEdgeWidth          = 8;
    r32 HorizontalSliderHeight = 26;
    r32 VerticalSliderWidth    = 26;
    r32 HorizontalSliderGrabThingBorder = 4;
    r32 VerticalSliderGrabThingBorder   = 4;
    
    // Slot
    r32 SlotHeight     = 30;
    r32 SongSlotHeight = 100;
    r32 SlotGap        = 3;
    
    r32 SmallTextLeftBorder = 12;
    r32 BigTextLeftBorder   = 32;
    
    // Font
    r32 FontSizeSmall  = 24;
    r32 FontSizeMedium = 50;
    r32 FontSizeBig    = 75;
    
    // Column Text
    r32 SongXOffset           =  92;
    r32 SongAlbumXOffset      = (SongXOffset+70);
    r32 SongTrackXOffset      = -18;
    r32 SongPlayButtonXOffset =  40;
    r32 SongPlayButtonYOffset = -22;
    r32 SongFirstRowYOffset   =  32;
    r32 SongSecondRowYOffset  = -8;
    r32 SongThirdRowYOffset   = -33;
    
    
    // Search
    r32 SearchFieldHeight   = 50;
    
    // Button sizes
    r32 SearchButtonExtents    = 12;
    r32 SmallButtonExtents     = 16;
    r32 MediumButtonExtents    = 21;
    r32 LargeButtonExtents     = 24;
    r32 PlayPauseButtonExtents = 45;
    r32 AddButtonExtents       = 22;
    
    // Button positions
    r32 TopLeftButtonGroupStartX = 40;
    r32 TopLeftButtonGroupGap    = 5;
    r32 TopLeftButtonGroupY      = 25;
    
    r32 ButtonGap        = 5;
    r32 PlayPauseButtonX = 116;
    r32 PlayPauseButtonY = 60;
    
    r32 StopNextPrevBtnYOffsetFromPlayPause = 20.75f;
    r32 LoopShuffleBtnYOffsetFromPlayPause  = 24;
    
    // Volume slider
    r32 VolumeGrabThingWidth = 10;
    
    // Timeline slider
    r32 TimelineXGap   = 10;
    r32 TimelineWidth  = 400;
    r32 TimelineHeight = 10;
    r32 TimelineGrapThingWidth  = 10;
    r32 TimelineGrapThingHeight = 60;
    
    // Music path
    r32 MusicPathHeightOffset     = 200;
    r32 MusicPathHeightScaler     = 0.86f; // 0..1
    r32 MusicPathTextFieldHeight  = 50;
    r32 MusicPathBGTransparency   = 0.75f;
    r32 MusicPathButtonYOffset    = 19;
    r32 MusicPathButtonGap        = 10;
    r32 MusicPathLoadingBarHeight = 40;
    r32 RescanButtonXOffset       = 204;
    r32 RescanButtonYOffset       = 55;
    
    // PlayingSongPanel
    r32 PlayingSongPanelXOffset = -10;
    r32 PlayingSongPanelBaseY   = 20;
    r32 CurrentTimeTextXOffset  = 10;
    r32 CurrentTimeTextYOffset  = 19;
    r32 DurationTimeTextYOffset = 72;
    
    r32 PlayingSongTextXOffset       = 440;
    r32 PlayingSongTextTitleYOffset  = 15;
    r32 PlayingSongTextTitleXOffset  = 40;
    r32 PlayingSongTextTrackYOffset  = 8;
    r32 PlayingSongTextArtistYOffset = -20;
    r32 PlayingSongTextAlbumYOffset  = -40;
    r32 PlayingSongTextAlbumXOffset  = 60;
    r32 PlayingSongTextGenreYOffset  = -60;
    
    // QuitCurtain
    r32 QuitCurtainAnimationTime       = 1;
    r32 QuitCurtainAnimationMultiplies = 1.75f;
    
    // User error text
    r32 ErrorTextAnimationTime    = 2.5f;
};

inline void UpdateColumnColor(music_display_column *DisplayColumn, struct playlist_column *PlaylistColumn);
internal void BringDisplayableEntryOnScreen(music_info *MusicInfo, music_display_column *DisplayColumn, playlist_id PlaylistID);
internal void BringDisplayableEntryOnScreenWithSortID(music_info *MusicInfo, music_display_column *DisplayColumn, batch_id SortID);
inline void ToggleSelection(music_display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID);
internal void UpdatePlayingSongColor(music_display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 FileID, v4 *Color);
internal void KeepPlayingSongOnScreen(renderer *Renderer, struct music_info *MusicInfo);
internal b32 UpdateDisplayColumn_(renderer *Renderer, music_info *MusicInfo, music_display_column *DisplayColumn,
                                  playlist_column *PlaylistColumn, i32 ScrollAmount);
internal void ScrollDisplayColumn(renderer *Renderer, music_info *MusicInfo, music_display_column *DisplayColumn, r32 ScrollAmount);
internal void SetTheNewPlayingSong(renderer *Renderer, playing_song_panel *Panel, layout_definition *Layout, music_info *MusicInfo);


internal void SearchInDisplayable(music_info *MusicInfo, playlist_column *PlaylistColumn, struct search_bar *Search, mp3_file_info *FileInfo = 0);
internal void UpdateColumnVerticalSlider(music_display_column *DisplayColumn, u32 DisplayableCount);
inline void PushUserErrorMessage(string_c *String);







#endif //_SOUND__U_I__T_D_H
