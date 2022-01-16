#ifndef _SOUND__U_I__T_D_H
#define _SOUND__U_I__T_D_H

#include "Sound_General_TD.h"
#include "Sound_Dragging.h"

#define MAX_DISPLAY_SONG_COUNT 24*3
#define MAX_DISPLAY_COUNT      200 // TODO:: @HardLimit

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
    NewStaticStringCompound("Text"), // 0
    NewStaticStringCompound("Error Text"), // 1
    NewStaticStringCompound("Foreground Text"), // 2
    NewStaticStringCompound("Foreground"), // 3
    NewStaticStringCompound("Slot"), // 4
    NewStaticStringCompound("Slider Background"), // 5
    NewStaticStringCompound("Slider Grab Thing"), // 6
    NewStaticStringCompound("Button Active"), // 7
    NewStaticStringCompound("Selected"), // 8
    NewStaticStringCompound("Playing Song"), // 9
};
global_variable string_c GlobalDefaultColorPaletteNames[] = 
{
    NewStaticStringCompound("Lush Green"),
    NewStaticStringCompound("Smoldering Red"),
    NewStaticStringCompound("Aquatic Blue"),
    NewStaticStringCompound("Deep Night Grey"),
    NewStaticStringCompound("Monochrome Grey"),
};

struct search_bar_btn_info
{
    struct renderer   *Renderer;
    struct search_bar *Search;
    struct music_info *MusicInfo;
    struct display_column *DisplayColumn;
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

struct display_column
{
    struct music_display_info *Base;
    enum column_type Type;
    
    r32 SlotWidth;
    r32 ColumnHeight;
    u32 Count;
    displayable_id SlotIDs[MAX_DISPLAY_COUNT];
    render_text   SlotText[MAX_DISPLAY_COUNT];
    entry_id      *SlotBGs[MAX_DISPLAY_COUNT];
    entry_id *Background;
    r32 DisplayCursor; // Y position in the total displayables
    entry_id *SlotBGAnchor; // Is only moved on resize, to be at the exact pos for the first BGRect.
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

struct display_column_song
{
    display_column Base;
    
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
    r32 BtnSize;
    r32 YearTextWidth;
    
    b32 IsSmallMode;
    
    struct mp3_file_info *FileInfo;
};
#define Parent(name)    &(name)->Base
#define ColumnExt(name) ((display_column_song *)(name))

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

struct shortcut_popups
{
    string_c PaletteSwap;          //  1
    string_c ColorPicker;          //  2
    string_c MusicPath;            //  3
    string_c SongPlay;             //  4
    string_c SongAddToNextUp;      //  5
    string_c SearchGenre;          //  6
    string_c SearchArtist;         //  7
    string_c SearchAlbum;          //  8
    string_c SearchSong;           //  9
    string_c Loop;                 // 10
    string_c Shuffle;              // 11
    string_c PlayPause;            // 12
    string_c Stop;                 // 13
    string_c Previous;             // 14
    string_c Next;                 // 15
    string_c Volume;               // 16
    string_c Timeline;             // 17
    string_c Help;                 // 18
    string_c Quit;                 // 19
    string_c SaveMusicPath;        // 20
    string_c CancelMusicPath;      // 21
    string_c RescanMetadata;       // 22
    string_c SavePalette;          // 23
    string_c CopyPalette;          // 24
    string_c DeletePalette;        // 25
    string_c CancelPicker;         // 26
    string_c AddPlaylist;          // 27
    string_c AddSelectionPlaylist; // 28
    string_c RenamePlaylist;       // 29
    string_c RemovePlaylist;       // 30
    string_c SearchPlaylists;      // 31
    
    u32 ActiveText; // Is one of the values from the strings above (1-31).
    b32 IsHovering;
    
    popup Popup;
    b32 IsActive;
};

enum playlist_btn_type
{
    playlistBtnType_Add,
    playlistBtnType_AddSelection,
    playlistBtnType_Remove,
    playlistBtnType_Rename,
};

struct playlist_ui
{
    entry_id *Panel;
    entry_id *PanelRightEdge;
    entry_id *BtnAnchor;
    button   *Add;
    button   *Remove;
    button   *Rename;
    button   *AddSelection;
    button_colors BtnColors;
    quit_animation AddCurtain;
    quit_animation AddSelectionCurtain;
    quit_animation RemoveCurtain;
    
    text_field RenameField;
};

struct music_display_info
{
    // General
    display_column Genre;
    display_column Artist;
    display_column Album;
    display_column_song Song;
    display_column *Columns[5]; // Easy acces through column_type.
    
    display_column Playlists;
    playlist_ui PlaylistUI;
    
    color_palette ColorPalette;
    u32 ColorPaletteID;
    
    i32 SearchIsActive;
    v2 MouseBtnDownLocation;
    
    // Foreground
    entry_id *EdgeBottom;
    entry_id *EdgeTop;
    entry_id *EdgeLeft;
    entry_id *EdgeRight;
    
    // Button general
    music_btn MusicBtnInfo;
    
    column_edge_drag PlaylistsGenreEdge;
    column_edge_drag GenreArtistEdge;
    column_edge_drag ArtistAlbumEdge;
    column_edge_drag AlbumSongEdge;
    
    // Control Panel
    button *PlayPause;
    button *Stop;
    button *Previous;
    button *Next;
    button *PaletteSwap;
    button *StyleSettings;
    button *Help;
    shortcut_popups Popups;
    slider Volume;
    
    // Music play control
    button *LoopPlaylist; 
    button *ShufflePlaylist;
    
    playing_song_panel PlayingSongPanel;
    music_path_ui MusicPath;
    
    drag_drop DragDrop;
    quit_animation Quit;
};

struct layout_definition
{
    // Window size
    i32 WindowWidth  = 1250; // 1416
    i32 WindowHeight = 800; // 1039
    
    // Foreground Edges
    r32 TopBorder     = 50;
    r32 BottomBorder  = 122;
    r32 LeftBorder    = 24;
    r32 RightBorder   = 24;
    
    // Column Initial edge X percentages
    r32 PlaylistsGenreXP = 0.17f;
    r32 GenreArtistXP    = 0.30f;
    r32 ArtistAlbumXP    = 0.50f;
    r32 AlbumSongXP      = 0.63f;
    
    // Drag and slide
    r32 DragEdgeWidth                   =  8;
    r32 HorizontalSliderHeight          = 26;
    r32 VerticalSliderWidth             = 26;
    r32 HorizontalSliderGrabThingBorder =  2;
    r32 VerticalSliderGrabThingBorder   =  2;
    
    // Slot
    r32 SlotHeight     = 6;
    r32 SongSlotHeight = 2;
    r32 SlotGap        = 3;
    
    r32 ColumnTextLeftBorder = 12;
    
    // Font
    r32 FontSizeSmall  = 24;
    r32 FontSizeMedium = 50;
    r32 FontSizeBig    = 75;
    
    // Column Text
    r32 SongXOffset           = 10;
    r32 SongAlbumXOffset      = 8;
    
    r32 SongPlayYOffset       = 5;
    r32 SlotTextYOffPercent   = 0.125f;
    
    // Search
    r32 SearchFieldHeight   = 0;
    
    // Button sizes
    r32 SearchButtonExtents    = 12;
    r32 SmallButtonExtents     = 16;
    r32 MediumButtonExtents    = 21;
    r32 LargeButtonExtents     = 24;
    r32 PlayPauseButtonExtents = 45;
    //r32 AddButtonExtents       = 22;
    
    // Button positions
    r32 TopLeftButtonGroupStartX = 40;
    r32 TopLeftButtonGroupGap    = 5;
    r32 TopLeftButtonGroupY      = 25;
    
    r32 ButtonGap        = 5;
    r32 PlayPauseButtonX = 116;
    r32 PlayPauseButtonY = 60;
    
    r32 StopNextPrevBtnYOffsetFromPlayPause = 20.75f;
    r32 LoopShuffleBtnYOffsetFromPlayPause  = 24;
    
    // Playlist panel
    r32 PlaylistPanelHeight   = 40;
    r32 PlaylistDividerHeight = 2;
    
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
    r32 MusicPathButtonYOffset    = 8;//19;
    r32 MusicPathButtonGap        = 10;
    r32 MusicPathLoadingBarHeight = 40;
    //r32 RescanButtonXOffset       = 204;
    //r32 RescanButtonYOffset       = 8;//55;
    
    // PlayingSongPanel
    r32 PanelXOffset = -10;
    r32 PanelBaseY   = 5;
    r32 PanelBaseX   = 440;
    r32 CurrentTimeTextXOffset  = 10;
    r32 DurationTimeTextYOffset = 15;
    r32 DurationTimeTextXOffset = 8;
    r32 PanelTextXOffset  = 8;
    r32 PanelTextYOffset  = -4;
    r32 PanelTextDepth    = -0.6f;
    
    // QuitCurtain
    r32 QuitCurtainAnimationTime       = 1;
    r32 QuitCurtainAnimationMultiplies = 1.75f;
    
    // User error text
    r32 ErrorMessageX           = 180;
    r32 ErrorMessageSmallTextY  = 12;
    r32 ErrorMessageMediumTextY = 13;
    r32 ErrorTextAnimationTime  = 2.5f;
    
    // Style settings window
    r32 SettingsWindowDepth   = -0.9f;
    r32 SettingsWindowWidth   = 900;
    r32 SettingsWindowHeight  = 475 + 25;
    r32 SettingsWindowBorder  = 4;
    r32 SettingsWindowBorderTop = 25;
    
    r32 ColorPickerOffsetX = -140;
    r32 ColorPickerOffsetY = 25;
    r32 ColorPickerWidth   = 620;
    r32 ColorPickerContentOffsetX = -100;
    r32 ColorPickerContentOffsetY = 25;
    r32 ColorPickerSpectrumWidth = 30;
    r32 ColorPickerColorFieldSide = 300;
    r32 ColorPickerPaletteColorsGap = 6;
    r32 ColorPickerColorOutline = 2;
    r32 ColorPickerButtonGapY = 4;
    r32 ColorPickerHSVTextOffset = 140;
    r32 ColorPickerHexTextOffset = 170;
    
    r32 PaletteNameFieldX = 500;
    r32 PaletteNameFieldIndent = 10;
    
    r32 SmallFontMin = 10;
    r32 SmallFontMax = 50;
    r32 MediumFontMin = 10;
    r32 MediumFontMax = 100;
    
    r32 FontSettingsOffsetX = 310;
    r32 FontSettingsOffsetY = 0;
    r32 FontSettingsBGWidth = 279;
    r32 FontSettingsContentOffsetX = 0;
    r32 FontSettingsContentOffsetY = 155;
    r32 FontSliderExtendsX  = 125;
    r32 FontSliderExtendsY  = 12.5f;
    r32 FontSliderGrabX     = 5;
    r32 FontSliderGrabY     = 15;
    r32 FontSizeSliderGap   = 5;
};

// For turning the raw data from EmbeddedResources into
// a loaded_bitmap struct.
#define NewBitmapData(name, colorFormat) \
{1, \
_Combine(name, _Width), \
_Combine(name, _Height), \
(u32 *)_Combine(name, _Data), \
colorFormat, \
ArrayCount(_Combine(name, _Data))\
}

inline void CreateSongButtonTextures(game_state *GS, display_column_song *SongColumn);
inline void UpdateColumnColor(display_column *DisplayColumn, struct playlist_column *PlaylistColumn);
internal void BringDisplayableEntryOnScreen(music_info *MusicInfo, display_column *DisplayColumn, playlist_id PlaylistID);
internal void BringDisplayableEntryOnScreenWithSortID(music_info *MusicInfo, display_column *DisplayColumn, batch_id SortID);
inline void ToggleSelection(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID);
internal void UpdatePlayingSongColor(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 FileID, v4 *Color);
internal void KeepPlayingSongOnScreen(renderer *Renderer, struct music_info *MusicInfo);
internal b32 UpdateDisplayColumn_(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn,
                                  playlist_column *PlaylistColumn, i32 ScrollAmount);
internal void ScrollDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn, r32 ScrollAmount);
internal void SetNewPlayingSong(renderer *Renderer, playing_song_panel *Panel, layout_definition *Layout, music_info *MusicInfo);


internal void SearchInDisplayable(music_info *MusicInfo, playlist_column *PlaylistColumn, struct search_bar *Search, mp3_file_info *FileInfo = 0);
internal void UpdateColumnVerticalSlider(display_column *DisplayColumn, u32 DisplayableCount);

inline string_c GetRandomExitMessage(game_state *GS, string_c *Language = NULL);
internal void ProcessEdgeDragOnResize(renderer *Renderer, music_display_info *DisplayInfo);
internal void UpdateSlots(game_state *GS, display_column *DisplayColumn);
inline color_palette *GetColorPalette(game_state *GS);


#endif //_SOUND__U_I__T_D_H
