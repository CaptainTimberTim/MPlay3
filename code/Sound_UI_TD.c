#include "Sound_UI_TD.h"
inline playlist_id GetNextSong(music_info *MusicInfo);
inline displayable_id GetSongAfterCurrent(u32 DisplayableCount, displayable_id DisplayableID);
inline displayable_id GetPreviousSong(u32 DisplayableCount, displayable_id DisplayableID);
inline void SetPreviousSong(music_info *MusicInfo);

internal void MillisecondsToMinutes(u32 Millis, string_c *Out);
internal void MoveDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn, 
                                displayable_id DisplayableStartID = {0}, r32 StartY = 0);
internal void SortDisplayables(music_info *MusicInfo, mp3_file_info *MP3FileInfo);
inline mp3_info *CreateMP3InfoStruct(arena_allocator *Arena, u32 FileInfoCount);
inline void CreateFileInfoStruct(mp3_file_info *FileInfo, u32 FileInfoCount);
inline void DeleteFileInfoStruct(mp3_file_info *FileInfo);
inline void ReplaceFolderPath(mp3_info *MP3Info, string_c *NewPath);
inline void SetSelectionArray(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID);
internal void UpdateSortingInfoChanged(renderer *Renderer, music_info *MusicInfo, mp3_info *MP3Info, column_type Type);
inline void SwitchSelection(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 ColumnDisplayID);
inline displayable_id SortingIDToColumnDisplayID(playlist_info *Playlist, display_column *DisplayColumn, batch_id BatchID);
inline void HandleChangeToNextSong(game_state *GameState);
internal void AddJobs_LoadOnScreenMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs = 0);
internal void AddJobs_LoadMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs = 0);
internal i32 AddJob_LoadNewPlayingSong(circular_job_queue *JobQueue, playlist_id PlaylistID);
internal b32 AddJob_LoadMetadata(game_state *GameState);
internal column_type UpdateSelectionArray(playlist_column *PlaylistColumn, display_column *DisplayColumn, v2 MouseBtnDownLocation);
internal column_type SelectAllOrNothing(display_column *DisplayColumn, playlist_column *PlaylistColumn);
internal void PropagateSelectionChange(music_info *SortingInfo);
internal void UpdatePlayingSong(music_info *MusicInfo);
internal void InsertSlotIntoPlaylist(game_state *GS, playlist_info *Playlist, column_type Type, displayable_id DisplayableID);
internal void InsertSlotsIntoPlaylist(game_state *GS, playlist_info *IntoPlaylist, column_type Type, array_u32 DisplayableIDs);
internal void RemoveSlotFromPlaylist(game_state *GS, column_type Type, displayable_id DisplayableID);
internal void RemoveSlotsFromPlaylist(game_state *GS, column_type Type, array_u32 DisplayableIDs);
internal void OnNewPlaylistClick(void *Data);
internal void OnNewPlaylistWithSelectionClick(void *Data);
internal void OnRemovePlaylistClick(void *Data);
internal void OnRenamePlaylistClick(void *Data);
inline r32 GetXTextPosition(entry_id *Background, r32 XOffset);
internal void ProcessWindowResizeForDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn);

inline file_id        GetFileID(music_info *MusicInfo, displayable_id DisplayableID);
inline playlist_id    GetPlaylistID(music_info *MusicInfo, displayable_id DisplayableID);
inline playlist_id    GetPlaylistID(playlist_column *SongColumn, file_id FileID);
inline displayable_id GetDisplayableID(music_info *MusicInfo, u32 OnScreenID, playlist_id *PlaylistID = 0);
inline displayable_id GetDisplayableID(array_playlist_id *Displayable, playlist_id PlaylistID);
inline   i32          GetSlotID(display_column *DisplayColumn, displayable_id DisplayableID);
internal i32          GetSlotID(game_state *GS, column_type Type, displayable_id DisplayableID);
internal i32          GetSlotID(game_state *GS, playlist_info *Playlist);
inline string_c *     GetName(playlist_column *PlaylistColumn, displayable_id DID);
inline void OnAnimationDone(void *Data);
internal void ResetColumnText(display_column *DisplayColumn, u32 DisplayableCount);

inline r32
SlotHeight(display_column *DisplayColumn)
{
    font_sizes FontSizes =  GlobalGameState.Renderer.FontAtlas.FontSizes;
    r32 SmallFont = FontSizes.Sizes[font_Small].Size;
    if(DisplayColumn->Type != columnType_Song)
    {
        return SmallFont + GlobalGameState.Layout.SlotHeight;
    }
    else
    {
        r32 MediumFont = FontSizes.Sizes[font_Medium].Size;
        if(ColumnExt(DisplayColumn)->IsSmallMode)
        {
            r32 BtnMin = GlobalGameState.Layout.AddButtonExtents*2 + GlobalGameState.Layout.SongPlayYOffset;
            return Max(MediumFont, BtnMin) + GlobalGameState.Layout.SlotGap + GlobalGameState.Layout.SongPlayYOffset;
        }
        else
        {
            r32 Height = SmallFont*2 + MediumFont + GlobalGameState.Layout.SongSlotHeight;
            return Max(Height, GlobalGameState.Layout.AddButtonExtents*2 + GlobalGameState.Layout.SlotGap*2 + MediumFont);
        }
    }
}

inline r32
GetSongButtonYOffset(layout_definition *Layout)
{
    r32 CurSlotHeight = SlotHeight(&GlobalGameState.MusicInfo.DisplayInfo.Song.Base);
    return -CurSlotHeight*0.5f + Layout->AddButtonExtents + Layout->SongPlayYOffset + Layout->SlotGap*0.5f;
}

inline r32
GetSongRow1YOffset(layout_definition *Layout, string_c Text)
{
    if(Text.Pos == 0) return 0;
    r32 Ascent = GetFontAscent(&GlobalGameState, font_Medium, Text);
    r32 CurSlotHeight = SlotHeight(&GlobalGameState.MusicInfo.DisplayInfo.Song.Base);
    return CurSlotHeight*0.5f - Layout->SlotGap - Ascent;
}

inline r32
GetSongRow2YOffset(layout_definition *Layout, string_c Text)
{
    if(Text.Pos == 0) return 0;
    r32 Descent = GetFontDescent(&GlobalGameState, font_Medium, Text);
    r32 Ascent  = GetFontAscent(&GlobalGameState, font_Small, Text);
    return GetSongRow1YOffset(Layout, Text) - Descent*-1 - Ascent + Layout->SlotGap;
}

inline r32
GetSongRow3YOffset(layout_definition *Layout, string_c Text)
{
    if(Text.Pos == 0) return 0;
    font_metrics Metrics  = GetFontMetrics(&GlobalGameState, font_Small, Text);
    return GetSongRow2YOffset(Layout, Text) - Metrics.RowGap;
}

inline r32
GetBottomPanelHeight(layout_definition *Layout)
{
    r32 FontSmall  = GetFontSize(&GlobalGameState.Renderer, font_Small).Size;
    r32 FontMedium = GetFontSize(&GlobalGameState.Renderer, font_Medium).Size;
    
    return Max(Layout->BottomBorder, FontSmall*3 + FontMedium);
}

inline u32 
CountPossibleDisplayedSlots(renderer *Renderer, display_column *DisplayColumn)
{
    u32 Result = 0;
    r32 Height = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    Result = Ceiling(Height/SlotHeight(DisplayColumn))+1;
    return Result;
}

inline void
CreateDisplayBackgroundRect(renderer *Renderer, display_column *DisplayColumn, u32 ID, r32 ZValue, entry_id *Parent)
{
    r32 SlotGap = GlobalGameState.Layout.SlotGap;
    v2 HalfDim  = V2(DisplayColumn->SlotWidth, SlotHeight(DisplayColumn)-SlotGap)/2;
    r32 YDown   = (ID > 0) ? -SlotHeight(DisplayColumn) : 0;
    DisplayColumn->SlotBGs[ID] = CreateRenderRect(Renderer, {-HalfDim, HalfDim}, ZValue, Parent, DisplayColumn->Colors.Slot);
    SetLocalPosition(DisplayColumn->SlotBGs[ID], V2(0, YDown));
}

internal void 
SetSongSlotHeightMode(game_state *GS, b32 MakeSmall)
{
    display_column_song *SongColumn = &GS->MusicInfo.DisplayInfo.Song;
    display_column *DisplayColumn = &SongColumn->Base;
    SongColumn->IsSmallMode = MakeSmall;
    
    r32 AnchorY = GetPosition(DisplayColumn->TopBorder).y - 
        GetSize(DisplayColumn->TopBorder).y/2.0f - SlotHeight(DisplayColumn)/2.0f;
    SetPosition(DisplayColumn->SlotBGAnchor, V2(0, AnchorY));
    UpdateOriginalPosition(&GS->Renderer.TransformList, DisplayColumn->SlotBGAnchor);
    
    r32 YDown = 0;
    r32 CurSlotHeight = SlotHeight(&SongColumn->Base);
    For(MAX_DISPLAY_COUNT)
    {
        if(SongColumn->Base.SlotBGs[It])
        {
            SetSize(SongColumn->Base.SlotBGs[It], V2(SongColumn->Base.SlotWidth, CurSlotHeight-GS->Layout.SlotGap));
            SetLocalPosition(SongColumn->Base.SlotBGs[It], V2(0, YDown));
            
            r32 YOff = GetSongButtonYOffset(&GS->Layout);
            SetLocalPosition(SongColumn->Play[It], V2(GetLocalPosition(SongColumn->Play[It]).x, YOff));
            YDown = -CurSlotHeight; // First iteration 0, after that, the actual YDown we want.
        }
    }
    
    ProcessWindowResizeForDisplayColumn(&GS->Renderer, &GS->MusicInfo, &SongColumn->Base);
    
}

inline r32
GetInitialYForDisplayColumnSlot(display_column *DisplayColumn)
{
    r32 Result = 0;
    Result  = GetLocalPosition(DisplayColumn->TopBorder).y;
    Result -= GetSize(DisplayColumn->TopBorder).y/2;
    Result -= SlotHeight(DisplayColumn)/2;
    return Result;
}

inline void
OnSongPlayPressed(void *Data)
{
    song_play_btn *PlayInfo = (song_play_btn *)Data;
    music_info *MusicInfo = &PlayInfo->GameState->MusicInfo;
    display_column *SongColumn = &MusicInfo->DisplayInfo.Song.Base;
    b32 *IsPlaying = &MusicInfo->IsPlaying;
    
    if(!IsInRect(SongColumn->Background, GlobalGameState.Input.MouseP)) return;
    
    UpdatePlayingSong(MusicInfo);
    
    playlist_id PlaylistID = {};
    displayable_id DisplayableID = GetDisplayableID(MusicInfo, PlayInfo->DisplayID, &PlaylistID);
    //displayable_id DisplayableID = {SongColumn->OnScreenIDs[PlayInfo->DisplayID].ID}; // TODO::PLAYLIST_DISPLAYABLE
    
    if(MusicInfo->PlayingSong.DisplayableID != DisplayableID)
    {
        *IsPlaying = true;
        MusicInfo->PlayingSong.DisplayableID = DisplayableID;
        MusicInfo->PlayingSong.PlaylistID = PlaylistID;
        if(MusicInfo->PlayingSong.PlayUpNext)
        {
            MusicInfo->PlayingSong.PlayUpNext = false; 
            Take(&MusicInfo->UpNextList.A, 0);
        }
        ChangeSong(PlayInfo->GameState, &MusicInfo->PlayingSong);
        
        AddJob_LoadMP3(&PlayInfo->GameState->JobQueue, GetPlaylistID(MusicInfo, DisplayableID));
        AddJobs_LoadMP3s(PlayInfo->GameState, &PlayInfo->GameState->JobQueue);
    }
    else 
    {
        *IsPlaying = !*IsPlaying;
        if(!*IsPlaying) PushSoundBufferClear(PlayInfo->GameState->SoundThreadInterface);
    }
}

inline void
OnSongAddPressed(void *Data)
{
    song_play_btn *PlayInfo = (song_play_btn *)Data;
    music_info *MusicInfo = &PlayInfo->GameState->MusicInfo;
    display_column *SongColumn = &MusicInfo->DisplayInfo.Song.Base;
    
    if(!IsInRect(SongColumn->Background, GlobalGameState.Input.MouseP)) return;
    
    playlist_id PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, SongColumn->SlotIDs[PlayInfo->DisplayID]);
    Push(&MusicInfo->UpNextList, PlaylistID);
    if(MusicInfo->UpNextList.A.Count < MAX_MP3_DECODE_COUNT-1)
    {
        AddJob_LoadMP3(&PlayInfo->GameState->JobQueue, PlaylistID);
    }
}

inline void
CreateSongButtons(renderer *Renderer, display_column_song *SongColumn, u32 ID)
{
    layout_definition *Layout = &GlobalGameState.Layout;
    color_palette *Palette    = &SongColumn->Base.Base->ColorPalette;
    u32 ButtonID              = SongColumn->Base.Base->PlayPause->Entry->ID->TexID;
    r32 Depth                 = SongColumn->Base.ZValue - 0.01f;
    
    //r32 FontHeight            = GetFontSize(Renderer, font_Small).Size;
    r32 HalfSize = Layout->AddButtonExtents; // FontHeight;
    rect Rect = {{-HalfSize, -HalfSize}, {HalfSize, HalfSize}};
    SongColumn->Play[ID] = NewButton(Renderer, Rect, Depth, false, ButtonID, SongColumn->PlayPauseGLID, Renderer->ButtonColors, 
                                     SongColumn->Base.SlotBGs[ID]);
    Translate(SongColumn->Play[ID], V2(0, GetSongButtonYOffset(Layout)));
    SongColumn->PlayBtnData[ID] = {ID, &GlobalGameState};
    SongColumn->Play[ID]->OnPressed = {OnSongPlayPressed, &SongColumn->PlayBtnData[ID]};
    
    SongColumn->Add[ID] = NewButton(Renderer, Rect, Depth, false, ButtonID, SongColumn->AddGLID, 
                                    Renderer->ButtonColors, SongColumn->Play[ID]->Entry);
    Translate(SongColumn->Add[ID], V2(HalfSize*2 + Layout->TopLeftButtonGroupGap, 0));
    
    SongColumn->Add[ID]->OnPressed = {OnSongAddPressed, &SongColumn->PlayBtnData[ID]};
}

inline void
SetSongButtonsActive(display_column_song *SongColumn, u32 ID, b32 IsActive)
{
    if(SongColumn->Play[ID]) SetActive(SongColumn->Play[ID], IsActive);
    if(SongColumn->Add[ID]) SetActive(SongColumn->Add[ID], IsActive);
}

inline b32
IsSearchOpen(music_display_info *DisplayInfo)
{
    b32 Result = false;
    
    Result = DisplayInfo->Genre.Search.TextField.IsActive 
        || DisplayInfo->Artist.Search.TextField.IsActive 
        || DisplayInfo->Album.Search.TextField.IsActive
        || DisplayInfo->Song.Base.Search.TextField.IsActive
        || DisplayInfo->Playlists.Search.TextField.IsActive;
    
    return Result;
}

inline column_type
GetOpenSearch(music_display_info *DisplayInfo)
{
    column_type Result = columnType_None;
    
    if(DisplayInfo->Genre.Search.TextField.IsActive)          Result = columnType_Genre;
    else if(DisplayInfo->Artist.Search.TextField.IsActive)    Result = columnType_Artist;
    else if(DisplayInfo->Album.Search.TextField.IsActive)     Result = columnType_Album;
    else if(DisplayInfo->Song.Base.Search.TextField.IsActive) Result = columnType_Song;
    else if(DisplayInfo->Playlists.Search.TextField.IsActive) Result = columnType_Playlists;
    
    return Result;
}

inline void
KillSearch(game_state *GS)
{
    music_display_info *DisplayInfo = &GS->MusicInfo.DisplayInfo;
    column_type Type = GetOpenSearch(DisplayInfo);
    if(Type != columnType_None)
    {
        search_bar *Search = &DisplayInfo->Columns[Type]->Search;
        
        DisplayInfo->SearchIsActive = -1;
        RemoveRenderText(&GS->Renderer, &Search->TextField.Text);
        SetActive(&Search->TextField, !Search->TextField.IsActive);
        ResetStringCompound(Search->TextField.TextString);
        Search->TextField.dBackspacePress = 0;
    }
}

inline void
CopyBackDisplayable(display_column *DisplayColumn)
{
    Copy(&DisplayColumn->SearchInfo.MusicInfo->Playlist_->Columns[DisplayColumn->SearchInfo.ColumnType].Displayable,
         &DisplayColumn->Search.InitialDisplayables);
    MoveDisplayColumn(DisplayColumn->SearchInfo.Renderer, DisplayColumn->SearchInfo.MusicInfo, DisplayColumn);
}

internal void
OnSearchPressed(void *Data)
{
    search_bar_btn_info *SearchInfo = (search_bar_btn_info *)Data;
    search_bar *Search = SearchInfo->Search;
    music_display_info *DisplayInfo = &SearchInfo->MusicInfo->DisplayInfo;
    array_playlist_id *Displayable = &SearchInfo->MusicInfo->Playlist_->Columns[SearchInfo->ColumnType].Displayable;
    
    if(DisplayInfo->SearchIsActive < 0) 
    {
        DisplayInfo->SearchIsActive = SearchInfo->ColumnType;
        UpdateTextField(SearchInfo->Renderer, &Search->TextField);
        Copy(&Search->InitialDisplayables, Displayable);
    }
    else
    {
        if(DisplayInfo->SearchIsActive == SearchInfo->ColumnType) 
        {
            CopyBackDisplayable(SearchInfo->DisplayColumn);
            DisplayInfo->SearchIsActive = -1;
            RemoveRenderText(SearchInfo->Renderer, &Search->TextField.Text);
        }
        else 
        {
            // Deactivating previous search.
            if(DisplayInfo->SearchIsActive == columnType_Genre)
            {
                CopyBackDisplayable(&DisplayInfo->Genre);
                OnSearchPressed(&DisplayInfo->Genre.SearchInfo);
            }
            else if(DisplayInfo->SearchIsActive == columnType_Artist)
            {
                CopyBackDisplayable(&DisplayInfo->Artist);
                OnSearchPressed(&DisplayInfo->Artist.SearchInfo);
            }
            else if(DisplayInfo->SearchIsActive == columnType_Album)
            {
                CopyBackDisplayable(&DisplayInfo->Album);
                OnSearchPressed(&DisplayInfo->Album.SearchInfo);
            }
            else if(DisplayInfo->SearchIsActive == columnType_Song)
            {
                CopyBackDisplayable(&DisplayInfo->Song.Base);
                OnSearchPressed(&DisplayInfo->Song.Base.SearchInfo);
            }
            else if(DisplayInfo->SearchIsActive == columnType_Playlists)
            {
                CopyBackDisplayable(&DisplayInfo->Playlists);
                OnSearchPressed(&DisplayInfo->Playlists.SearchInfo);
            }
            
            DisplayInfo->SearchIsActive = SearchInfo->DisplayColumn->Type;
            UpdateTextField(SearchInfo->Renderer, &Search->TextField);
            Copy(&Search->InitialDisplayables, Displayable);
        }
        UpdatePlayingSong(&GlobalGameState.MusicInfo);
        UpdateColumnVerticalSlider(SearchInfo->DisplayColumn, Displayable->A.Count);
    }
    
    SetActive(&Search->TextField, !Search->TextField.IsActive);
    
    ResetStringCompound(Search->TextField.TextString);
    Search->TextField.dBackspacePress = 0;
}

inline void
ResetSearchList(renderer *Renderer, display_column *DisplayColumn)
{
    array_playlist_id *Displayable = &DisplayColumn->SearchInfo.MusicInfo->Playlist_->Columns[DisplayColumn->SearchInfo.ColumnType].Displayable;
    Copy(Displayable, &DisplayColumn->Search.InitialDisplayables);
    MoveDisplayColumn(Renderer, DisplayColumn->SearchInfo.MusicInfo, DisplayColumn);
    OnSearchPressed(&DisplayColumn->SearchInfo);
}

internal void
InterruptSearch(renderer *Renderer, music_info *MusicInfo)
{
    display_column *DisplayColumn = 0;
    switch(GetOpenSearch(&MusicInfo->DisplayInfo))
    {
        case columnType_Genre: 
        {
            DisplayColumn = &MusicInfo->DisplayInfo.Genre;
        } break;
        case columnType_Artist: 
        {
            DisplayColumn = &MusicInfo->DisplayInfo.Artist;
        } break;
        case columnType_Album: 
        {
            DisplayColumn = &MusicInfo->DisplayInfo.Album;
        } break;
        case columnType_Song: 
        {
            DisplayColumn = &MusicInfo->DisplayInfo.Song.Base;
        } break;
        case columnType_Playlists: 
        {
            DisplayColumn = &MusicInfo->DisplayInfo.Playlists;
        } break;
    }
    if(DisplayColumn) ResetSearchList(Renderer, DisplayColumn);
}

internal void
ProcessActiveSearch(column_info ColumnInfo, r32 dTime, input_info *Input, mp3_file_info *FileInfo = 0)
{
    Assert(ColumnInfo.PlaylistColumn);
    renderer                *Renderer  = ColumnInfo.Renderer;
    display_column      *DisplayColumn = ColumnInfo.DisplayColumn;
    playlist_column    *PlaylistColumn = ColumnInfo.PlaylistColumn;
    text_field_flag_result FieldResult = ProcessTextField(Renderer, dTime, Input, &DisplayColumn->Search.TextField);
    
    if(FieldResult.Flag & processTextField_TextChanged)
    {
        SearchInDisplayable(ColumnInfo.MusicInfo, PlaylistColumn, &DisplayColumn->Search, FileInfo);
        MoveDisplayColumn(Renderer, ColumnInfo.MusicInfo, DisplayColumn);
        UpdateColumnVerticalSlider(DisplayColumn, PlaylistColumn->Displayable.A.Count);
    }
    if(FieldResult.Flag & processTextField_Confirmed)
    {
        array_playlist_id *Displayable = &DisplayColumn->SearchInfo.MusicInfo->Playlist_->Columns[DisplayColumn->SearchInfo.ColumnType].Displayable;
        if(ColumnInfo.MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable.A.Count == 1) 
        {
            SetSelectionArray(DisplayColumn, PlaylistColumn, 0);
            PropagateSelectionChange(&GlobalGameState.MusicInfo);
            UpdateColumnColor(DisplayColumn, PlaylistColumn);
        }
        ResetSearchList(Renderer, DisplayColumn);
        UpdateSortingInfoChanged(Renderer, &GlobalGameState.MusicInfo, GlobalGameState.MP3Info, DisplayColumn->Type);
        
        playlist_id SelectedID = Get(&PlaylistColumn->Selected, NewSelectID(0));
        if(DisplayColumn->Type == columnType_Song) 
            BringDisplayableEntryOnScreen(ColumnInfo.MusicInfo, DisplayColumn, SelectedID);
        else BringDisplayableEntryOnScreenWithSortID(DisplayColumn->SearchInfo.MusicInfo, DisplayColumn, SelectedID);
    }
}

internal void
ProcessAllSearchBars(game_state *GS)
{
    music_info *MusicInfo = &GS->MusicInfo;
    column_info ColumnInfo = {&GS->Renderer, &MusicInfo->DisplayInfo, MusicInfo, /*Rest gets filled below*/};
    
    if(MusicInfo->DisplayInfo.Genre.Search.TextField.IsActive)
    {
        ColumnInfo.DisplayColumn  = &MusicInfo->DisplayInfo.Genre;
        ColumnInfo.PlaylistColumn = &MusicInfo->Playlist_->Genre;
        ProcessActiveSearch(ColumnInfo, GS->Time.dTime, &GS->Input);
    }
    else if(MusicInfo->DisplayInfo.Artist.Search.TextField.IsActive)
    {
        ColumnInfo.DisplayColumn  = &MusicInfo->DisplayInfo.Artist;
        ColumnInfo.PlaylistColumn = &MusicInfo->Playlist_->Artist;
        ProcessActiveSearch(ColumnInfo, GS->Time.dTime, &GS->Input);
    }
    else if(MusicInfo->DisplayInfo.Album.Search.TextField.IsActive)
    {
        ColumnInfo.DisplayColumn  = &MusicInfo->DisplayInfo.Album;
        ColumnInfo.PlaylistColumn = &MusicInfo->Playlist_->Album;
        ProcessActiveSearch(ColumnInfo, GS->Time.dTime, &GS->Input);
    }
    else if(MusicInfo->DisplayInfo.Song.Base.Search.TextField.IsActive)
    {
        ColumnInfo.DisplayColumn  = &MusicInfo->DisplayInfo.Song.Base;
        ColumnInfo.PlaylistColumn = &MusicInfo->Playlist_->Song;
        ProcessActiveSearch(ColumnInfo, GS->Time.dTime, &GS->Input, &GS->MP3Info->FileInfo);
    }
    else if(MusicInfo->DisplayInfo.Playlists.Search.TextField.IsActive)
    {
        ColumnInfo.DisplayColumn  = &MusicInfo->DisplayInfo.Playlists;
        ColumnInfo.PlaylistColumn = &MusicInfo->Playlist_->Playlists;
        ProcessActiveSearch(ColumnInfo, GS->Time.dTime, &GS->Input);
    }
}

internal search_bar
CreateSearchBar(column_info ColumnInfo, arena_allocator *Arena, entry_id *Parent, layout_definition *Layout)
{
    search_bar Result = {};
    renderer *Renderer = ColumnInfo.Renderer;
    r32 BtnDepth        = -0.395f;
    r32 SearchExt       = Layout->SearchButtonExtents;
    
#if RESOURCE_PNG
    u32 SearchID = DecodeAndCreateGLTexture(Search_Icon_DataCount, (u8 *)Search_Icon_Data);
#else
    loaded_bitmap Bitmap   = {1, Search_Icon_Width, Search_Icon_Height, (u32 *)Search_Icon_Data, colorFormat_RGBA, ArrayCount(Search_Icon_Data)};
    u32 SearchID = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
#endif
    
    Result.Button = NewButton(Renderer, {{-SearchExt, -SearchExt},{SearchExt, SearchExt}}, 
                              BtnDepth, false, Renderer->ButtonBaseID, SearchID, Renderer->ButtonColors, Parent);
    Translate(Result.Button, V2(1, -1));
    ColumnInfo.DisplayColumn->SearchInfo = 
    {
        Renderer, &ColumnInfo.DisplayColumn->Search, ColumnInfo.MusicInfo, ColumnInfo.DisplayColumn, ColumnInfo.DisplayColumn->Type
    };
    Result.Button->OnPressed = {OnSearchPressed, &ColumnInfo.DisplayColumn->SearchInfo};
    
    Assert(ColumnInfo.PlaylistColumn);
    Result.InitialDisplayables.A = CreateArray(Arena, ColumnInfo.PlaylistColumn->Displayable.A.Length);
    
    v2 TextFieldSize = V2(ColumnInfo.DisplayColumn->SlotWidth/2.0f, Layout->SearchFieldHeight);
    Result.TextField = CreateTextField(Renderer, Arena, TextFieldSize, 
                                       ColumnInfo.DisplayColumn->ZValue-0.029f, (u8 *)"Search...", ColumnInfo.DisplayColumn->SliderHorizontal.Background, 
                                       &ColumnInfo.DisplayInfo->ColorPalette.Text,
                                       &ColumnInfo.DisplayInfo->ColorPalette.ButtonActive, font_Medium);
    Translate(&Result.TextField, V2(0, Layout->SearchFieldHeight/2 + 
                                    GetExtends(ColumnInfo.DisplayColumn->SliderHorizontal.Background).y));
    
    return Result;
}

internal void
CreateDisplayColumn(column_info ColumnInfo, arena_allocator *Arena, column_type Type, column_colors ColumnColors, r32 ZValue, 
                    entry_id *LeftBorder, entry_id *RightBorder, layout_definition *Layout)
{
    music_display_info *DisplayInfo = ColumnInfo.DisplayInfo;
    renderer *Renderer = ColumnInfo.Renderer;
    display_column *DisplayColumn = ColumnInfo.DisplayColumn;
    
    // NOTE:: This should only happen at the very beginning, before a window resize
    DisplayColumn->Base        = DisplayInfo;
    DisplayColumn->Type        = Type;
    DisplayColumn->ZValue      = ZValue;
    DisplayColumn->LeftBorder  = LeftBorder;
    DisplayColumn->Colors      = ColumnColors;
    
    if(Type == columnType_Playlists) DisplayColumn->TopBorder = DisplayInfo->PlaylistUI.Panel;
    else DisplayColumn->TopBorder = DisplayInfo->EdgeTop;
    // NOTE:: Right and bottom border is a slider, therefore set later in procedure
    
    DisplayColumn->TextX = Layout->SmallTextLeftBorder;
    if(Type == columnType_Song) 
        DisplayColumn->TextX = Layout->BigTextLeftBorder;
    
    // Creating horizontal slider 
    r32 SliderHoriHeight = Layout->HorizontalSliderHeight;
    r32 SliderVertWidth = Layout->VerticalSliderWidth;
    r32 InsetHori = Layout->HorizontalSliderGrabThingBorder;
    DisplayColumn->SlotWidth = (GetRect(RightBorder).Min.x - SliderVertWidth) - GetRect(LeftBorder).Max.x;
    DisplayColumn->SlotWidth = Max(DisplayColumn->SlotWidth, 0.1f);
    
    v2 BottomLeft = V2(GetRect(LeftBorder).Max.x, GetRect(DisplayInfo->EdgeBottom).Max.y);
    DisplayColumn->SliderHorizontal.Background = 
        CreateRenderRect(Renderer, {BottomLeft, BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight)},
                         DisplayColumn->ZValue-0.03f, 0, ColumnColors.SliderBG);
    
    rect GrabThingRect = {BottomLeft+V2(0, InsetHori), BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight-InsetHori)};
    DisplayColumn->SliderHorizontal.GrabThing  = CreateRenderRect(Renderer, GrabThingRect, DisplayColumn->ZValue-0.031f, 0, ColumnColors.SliderGrabThing);
    
    
    DisplayColumn->BottomBorder = DisplayColumn->SliderHorizontal.Background;
    r32 ColumnHeight = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    // If we have a height of 0, we 'deadlock' our sizes. As changing the scale means multiplying the original size, 
    // which is not going to do anything if the original size is 0.
    ColumnHeight = Max(ColumnHeight, 0.1f);
    DisplayColumn->ColumnHeight = ColumnHeight;
    
    // Creating vertical slider
    v2 BottomLeft2 = BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight);
    v2 VertSliderExtends = V2(SliderVertWidth, ColumnHeight)*0.5f;
    r32 InsetVert = Layout->VerticalSliderGrabThingBorder;
    DisplayColumn->SliderVertical.Background = 
        CreateRenderRect(Renderer, {-VertSliderExtends, VertSliderExtends}, DisplayColumn->ZValue-0.0311f,
                         RightBorder, ColumnColors.SliderBG);
    SetPosition(DisplayColumn->SliderVertical.Background, BottomLeft2+VertSliderExtends);
    
    DisplayColumn->SliderVertical.GrabThing  = 
        CreateRenderRect(Renderer, {-VertSliderExtends+V2(InsetVert, 0), VertSliderExtends-V2(InsetVert, 0)},
                         ZValue-0.0312f, RightBorder, ColumnColors.SliderGrabThing);
    SetPosition(DisplayColumn->SliderVertical.GrabThing, BottomLeft2+VertSliderExtends);
    
    DisplayColumn->RightBorder = DisplayColumn->SliderVertical.Background;
    
    // Creating Slot background and slots
    rect BGRect = {BottomLeft+V2(0, SliderHoriHeight), BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight+ColumnHeight)};
    DisplayColumn->Background = CreateRenderRect(Renderer, BGRect, ZValue+0.01f, 0, ColumnColors.Background);
    
    DisplayColumn->Count  = CountPossibleDisplayedSlots(Renderer, DisplayColumn);
    
    DisplayColumn->SlotBGAnchor = CreateRenderRect(Renderer, V2(0), 0, ColumnColors.Background, 0);
    r32 AnchorY = GetPosition(DisplayColumn->TopBorder).y-GetSize(DisplayColumn->TopBorder).y/2.0f-
        SlotHeight(DisplayColumn)/2.0f;
    SetPosition(DisplayColumn->SlotBGAnchor, V2(0, AnchorY));
    TranslateWithScreen(&Renderer->TransformList, DisplayColumn->SlotBGAnchor, fixedTo_Top);
    
    entry_id *Parent = DisplayColumn->SlotBGAnchor;
    For(DisplayColumn->Count)
    {
        CreateDisplayBackgroundRect(Renderer, DisplayColumn, It, ZValue, Parent);
        Parent = DisplayColumn->SlotBGs[It];
    }
    
    if(Type == columnType_Song) 
    {
#if RESOURCE_PNG
        ColumnExt(DisplayColumn)->PlayPauseGLID = DecodeAndCreateGLTexture(PlayPause_Icon_DataCount, (u8 *)PlayPause_Icon_Data);
        ColumnExt(DisplayColumn)->AddGLID = DecodeAndCreateGLTexture(AddSong_Icon_DataCount, (u8 *)AddSong_Icon_Data);
#else
        loaded_bitmap Bitmap = {1, PlayPause_Icon_Width, PlayPause_Icon_Height, (u32 *)PlayPause_Icon_Data, colorFormat_RGBA, ArrayCount(PlayPause_Icon_Data)};
        ColumnExt(DisplayColumn)->PlayPauseGLID = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
        Bitmap = {1, AddSong_Icon_Width, AddSong_Icon_Height, (u32 *)AddSong_Icon_Data, colorFormat_RGBA, ArrayCount(AddSong_Icon_Data)};
        ColumnExt(DisplayColumn)->AddGLID = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
#endif
        
#if 0
        loaded_bitmap PlayPauseIcon             = CreatePlayPauseIcon(&GlobalGameState, PlayPause_Icon_Height);
        ColumnExt(DisplayColumn)->PlayPauseGLID = CreateGLTexture(PlayPauseIcon, true);
#endif
        
        loaded_bitmap PlusIcon = CreatePlusIcon(&GlobalGameState, AddSong_Icon_Height);
        ColumnExt(DisplayColumn)->AddGLID = CreateGLTexture(PlusIcon, true);
        
        For(DisplayColumn->Count)
        {
            CreateSongButtons(Renderer, ColumnExt(DisplayColumn), It);
        }
    }
    
    // Creating Search bar and miscelanious
    r32 HalfHori = SliderHoriHeight/2;
    r32 HalfVert = SliderVertWidth/2;
    rect BetweenRect = {{-HalfVert, -HalfHori},{HalfVert, HalfHori}};
    DisplayColumn->BetweenSliderRect = CreateRenderRect(Renderer, BetweenRect, -0.392f, RightBorder, 
                                                        &DisplayColumn->Base->ColorPalette.Foreground);
    
    DisplayColumn->Search = CreateSearchBar(ColumnInfo, Arena, DisplayColumn->BetweenSliderRect, Layout);
}

internal r32
CalcTextOverhang(entry_id *Rect, render_text Text)
{
    r32 Result = 0;
    
    if(Text.Count > 0) 
    {
        v2 TextT = GetPosition(&Text, Text.Count-1);
        v2 TextP = TextT+V2(15,0);
        Result = DistanceToRectEdge(Rect, TextP);
    }
    
    return Result;
}

internal r32
CalcTextOverhang(display_column *DisplayColumn, render_text *TextArray, u32 DisplayableCount)
{
    r32 Result = 0;
    
    r32 MaxOverhang = 0.0f;
    for(u32 It = 0;
        It < DisplayColumn->Count && 
        It < DisplayableCount;
        It++)
    {
        if(TextArray[It].Count > 0) 
        {
            v2 TextT = GetPosition(&TextArray[It], TextArray[It].Count-1);
            v2 TextP = TextT+V2(15,0);
            r32 Overhang = DistanceToRectEdge(DisplayColumn->SlotBGs[It], TextP);
            if(Overhang > MaxOverhang)
            {
                MaxOverhang = Overhang;
            }
        }
    }
    
    Result = MaxOverhang;
    
    return Result;
}

internal r32
CalcTextOverhangPercentage(display_column *DisplayColumn, render_text *TextArray, u32 DisplayableCount)
{
    r32 Result = CalcTextOverhang(DisplayColumn, TextArray, DisplayableCount);
    Result = (1.0f/DisplayColumn->SlotWidth) * Result;
    return Result;
}

internal r32
CalcSongTextOverhangPercentage(display_column *DisplayColumn, u32 DisplayableCount)
{
    r32 Result = 0;
    display_column_song *Song = ColumnExt(DisplayColumn);
    
    r32 TitleOverhang = CalcTextOverhangPercentage(DisplayColumn, Song->SongTitle, DisplayableCount);
    r32 ArtistOverhang = CalcTextOverhangPercentage(DisplayColumn, Song->SongArtist, DisplayableCount);
    r32 AlbumOverhang = CalcTextOverhangPercentage(DisplayColumn, Song->SongAlbum, DisplayableCount);
    
    Result = Max(TitleOverhang, Max(ArtistOverhang, AlbumOverhang));
    
    return Result;
}

inline r32
GetXTextPosition(entry_id *Background, r32 XOffset)
{
    r32 Result =  GetPosition(Background).x - GetSize(Background).x/2 + XOffset;
    return Result;
}

inline r32
GetDisplayableHeight(u32 DisplayableCount, r32 SlotHeight)
{
    return DisplayableCount*SlotHeight;
}

inline b32 
IsInOnScreenList(display_column *DisplayColumn, displayable_id PlaylistID, u32 *OnScreenID = 0)
{
    b32 Result = false;
    
    For(DisplayColumn->Count)
    {
        if(PlaylistID == DisplayColumn->SlotIDs[It].ID) // TODO::PLAYLIST_DISPLAYABLE
        {
            Result = true;
            if(OnScreenID) *OnScreenID = It;
            break;
        }
    }
    
    return Result;
}

inline void
SetSelection(display_column *DisplayColumn, playlist_column *PlaylistColumn, displayable_id ID, b32 Select)
{
    Assert(ID.ID < (i32)PlaylistColumn->Displayable.A.Count);
    
    playlist_id PlaylistID = Get(&PlaylistColumn->Displayable, ID);
    if(Select) Push(&PlaylistColumn->Selected, PlaylistID);
    else StackFindAndTake(&PlaylistColumn->Selected, PlaylistID);
}

inline void
Select(playlist_column *PlaylistColumn, playlist_id PLID)
{
    Assert(PLID < PlaylistColumn->Batch.BatchCount);
    Push(&PlaylistColumn->Selected, PLID);
}

inline void
Select(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 SlotIDsID)
{
    Assert(DisplayColumn->Count > SlotIDsID && PlaylistColumn->Displayable.A.Count > SlotIDsID);
    
    playlist_id PlaylistID = Get(&PlaylistColumn->Displayable, DisplayColumn->SlotIDs[SlotIDsID]);
    Push(&PlaylistColumn->Selected, PlaylistID);
}

inline void
Deselect(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 SlotIDsID)
{
    Assert(DisplayColumn->Count > SlotIDsID && PlaylistColumn->Displayable.A.Count > SlotIDsID);
    
    playlist_id PlaylistID  = Get(&PlaylistColumn->Displayable, DisplayColumn->SlotIDs[SlotIDsID]);
    StackFindAndTake(&PlaylistColumn->Selected, PlaylistID);
}

inline b32
IsSelected(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 SlotIDsID)
{
    Assert(DisplayColumn->Count > SlotIDsID && PlaylistColumn->Displayable.A.Count > SlotIDsID);
    
    playlist_id PlaylistID  = Get(&PlaylistColumn->Displayable, DisplayColumn->SlotIDs[SlotIDsID]);
    return StackContains(&PlaylistColumn->Selected, PlaylistID);
}

inline void
ToggleSelection(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 SlotIDsID)
{
    if(IsSelected(DisplayColumn, PlaylistColumn, SlotIDsID)) Deselect(DisplayColumn, PlaylistColumn, SlotIDsID);
    else Select(DisplayColumn, PlaylistColumn, SlotIDsID);
}

inline void
ClearSelection(playlist_column *PlaylistColumn)
{
    Reset(&PlaylistColumn->Selected);
}

internal void
UpdateDisplayColumnColor(display_column *DisplayColumn, playlist_column *PlaylistColumn)
{
    Assert(DisplayColumn->Type != columnType_Song);
    for(u32 It = 0; 
        It < DisplayColumn->Count &&
        It < PlaylistColumn->Displayable.A.Count; 
        It++)
    {
        if(IsSelected(DisplayColumn, PlaylistColumn, It))
        {
            SetColor(DisplayColumn->SlotBGs[It], DisplayColumn->Colors.Selected);
        }
        else SetColor(DisplayColumn->SlotBGs[It], DisplayColumn->Colors.Slot); 
    }
}

internal void
UpdatePlayingSongColor(display_column *DisplayColumn, playlist_column *PlaylistColumn, playlist_id PlaylistID, v3 *Color)
{
    for(u32 It = 0; 
        It < DisplayColumn->Count &&
        It < PlaylistColumn->Displayable.A.Count; 
        It++)
    {
        playlist_id ActualID  = Get(&PlaylistColumn->Displayable, DisplayColumn->SlotIDs[It]);
        if(PlaylistID == ActualID)
        {
            SetColor(DisplayColumn->SlotBGs[It], Color);
        }
        else if(StackContains(&PlaylistColumn->Selected, ActualID))
        {
            SetColor(DisplayColumn->SlotBGs[It], DisplayColumn->Colors.Selected);
        }
        else SetColor(DisplayColumn->SlotBGs[It], DisplayColumn->Colors.Slot); 
    }
}

inline void
UpdateColumnColor(display_column *DisplayColumn, playlist_column *PlaylistColumn)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    if(DisplayColumn->Type == columnType_Song)
    {
        playlist_id PlaylistID = {-1};
        if(MusicInfo->PlayingSong.DisplayableID >= 0) 
            PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, MusicInfo->PlayingSong.DisplayableID);
        UpdatePlayingSongColor(DisplayColumn, PlaylistColumn, PlaylistID, &MusicInfo->DisplayInfo.ColorPalette.PlayingSong);
    }
    else
    {
        UpdateDisplayColumnColor(DisplayColumn, PlaylistColumn);
    }
}

internal void
UpdateSelectionColors(music_info *MusicInfo)
{
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Genre, &MusicInfo->Playlist_->Genre);
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Artist, &MusicInfo->Playlist_->Artist);
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Album, &MusicInfo->Playlist_->Album);
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Playlists, &MusicInfo->Playlist_->Playlists);
    
    if(MusicInfo->PlayingSong.DisplayableID >= 0)
    {
        playlist_id PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, MusicInfo->PlayingSong.DisplayableID);
        UpdatePlayingSongColor(&MusicInfo->DisplayInfo.Song.Base, &MusicInfo->Playlist_->Song, PlaylistID, &MusicInfo->DisplayInfo.ColorPalette.PlayingSong);
    }
}

inline void
ChangeDisplayColumnCount(renderer *Renderer, display_column *DisplayColumn, u32 NewCount)
{
    For(NewCount)
    {
        if(!DisplayColumn->SlotBGs[It]) 
        {
            Assert(It > 0);
            CreateDisplayBackgroundRect(Renderer, DisplayColumn, It, DisplayColumn->ZValue, DisplayColumn->SlotBGs[It-1]);
            if(DisplayColumn->Type == columnType_Song) CreateSongButtons(Renderer, ColumnExt(DisplayColumn), It);
        }
    }
    
    for(u32 ID = NewCount; ID < DisplayColumn->Count; ID++)
    {
        if(DisplayColumn->Type == columnType_Song) 
        {
            SetActive(ColumnExt(DisplayColumn)->SongTitle+ID, false);
            SetActive(ColumnExt(DisplayColumn)->SongArtist+ID, false);
            SetActive(ColumnExt(DisplayColumn)->SongAlbum+ID, false);
            SetActive(ColumnExt(DisplayColumn)->SongTrack+ID, false);
            SetActive(ColumnExt(DisplayColumn)->SongYear+ID, false);
            
            SetSongButtonsActive(ColumnExt(DisplayColumn), ID, false);
        }
        else SetActive(DisplayColumn->SlotText+ID, false);
        if(DisplayColumn->SlotBGs[ID]) DisplayColumn->SlotBGs[ID]->ID->Render = false;
    }
    
    DisplayColumn->Count = NewCount;
}

inline r32
GetYearWidth(display_column_song *SongColumn)
{
    r32 Result = 0;
    For(SongColumn->Base.Count)
    {
        if(SongColumn->SongYear[It].Count == 6) // "2006 |" -> 6 Characters
        {
            Result = SongColumn->SongYear[It].Extends.x;
            break;
        }
    }
    
    return Result;
}

internal void
ResetColumnText(display_column *DisplayColumn, u32 DisplayableCount)
{
    layout_definition *Layout = &GlobalGameState.Layout;
    r32 YearX = (DisplayColumn->Type == columnType_Song) ? GetYearWidth(ColumnExt(DisplayColumn)) : -1;
    
    for(u32 It = 0;
        It < DisplayColumn->Count &&
        It < DisplayableCount; 
        ++It)
    {
        r32 TextX = GetXTextPosition(DisplayColumn->SlotBGs[It], DisplayColumn->TextX);
        if(DisplayColumn->Type == columnType_Song)
        {
            display_column_song *SongColumn = ColumnExt(DisplayColumn);
            
            SetPositionX(SongColumn->SongTitle+It,  TextX+Layout->SongXOffset);
            SetPositionX(SongColumn->SongArtist+It, TextX+Layout->SongXOffset);
            SetPositionX(SongColumn->SongTrack+It,  TextX+Layout->SongTrackXOffset);
            SetPositionX(SongColumn->SongYear+It,   TextX+Layout->SongXOffset);
            r32 YearRightX = GetPosition(SongColumn->SongYear[It].Base).x + YearX + Layout->SongAlbumXOffset;
            SetPositionX(SongColumn->SongAlbum+It,  YearRightX);
            
            SetActive(SongColumn->SongTitle+It, true);
            SetActive(SongColumn->SongArtist+It, true);
            SetActive(SongColumn->SongAlbum+It, true);
            SetActive(SongColumn->SongGenre+It, true);
            SetActive(SongColumn->SongTrack+It, true);
            SetActive(SongColumn->SongYear+It, true);
            
            r32 NewBtnX = GetPosition(DisplayColumn->LeftBorder).x;
            SetPositionX(SongColumn->Play[It]->Entry, NewBtnX+Layout->SongPlayButtonXOffset);
            SetSongButtonsActive(SongColumn, It, true);
        }
        else
        {
            SetPositionX(DisplayColumn->SlotText+It, TextX);
            SetActive(DisplayColumn->SlotText+It, true);
        }
    }
}

inline void
RemoveSongText(display_column *DisplayColumn, u32 ID)
{
    display_column_song *DisplaySong = ColumnExt(DisplayColumn);
    RemoveRenderText(&GlobalGameState.Renderer, DisplaySong->SongTitle+ID);
    RemoveRenderText(&GlobalGameState.Renderer, DisplaySong->SongArtist+ID);
    RemoveRenderText(&GlobalGameState.Renderer, DisplaySong->SongAlbum+ID);
    RemoveRenderText(&GlobalGameState.Renderer, DisplaySong->SongTrack+ID);
    RemoveRenderText(&GlobalGameState.Renderer, DisplaySong->SongYear+ID);
}

inline void
UpdateSongText(playlist_column *PlaylistColumn, display_column *DisplayColumn, u32 ID, displayable_id NextID)
{
    Assert(DisplayColumn->Type == columnType_Song);
    
    layout_definition *Layout = &GlobalGameState.Layout;
    display_column_song *SongColumn = ColumnExt(DisplayColumn);
    mp3_metadata *MD   = GetMetadata(PlaylistColumn, SongColumn->FileInfo, NextID);
    
    v2 SongP = {Layout->SongXOffset, GetSongRow1YOffset(Layout, MD->Title)};
    RenderText(&GlobalGameState, font_Medium, &MD->Title, DisplayColumn->Colors.Text,
               SongColumn->SongTitle+ID, -0.12001f, DisplayColumn->SlotBGs[ID], SongP);
    SetScissor(SongColumn->SongTitle+ID, DisplayColumn->SlotBGs[ID]);
    
    if(!SongColumn->IsSmallMode)
    {
        string_c YearAddon = NewStringCompound(&GlobalGameState.ScratchArena, 10);
        string_c Addon = NewStaticStringCompound(" |");
        if(MD->YearString.Pos < 4) 
        {
            string_c FakeYear = NewStaticStringCompound(" ");
            //ConcatStringCompounds(3, &YearAddon, &FakeYear, &Addon);
            CopyIntoCompound(&YearAddon, &FakeYear);
        }
        else if(MD->YearString.Pos > 4);
        else ConcatStringCompounds(3, &YearAddon, &MD->YearString, &Addon);
        v2 YearP = {Layout->SongXOffset, GetSongRow2YOffset(Layout, YearAddon)};
        RenderText(&GlobalGameState, font_Small, &YearAddon, DisplayColumn->Colors.Text,
                   SongColumn->SongYear+ID, -0.12f, DisplayColumn->SlotBGs[ID], YearP);
        SetScissor(SongColumn->SongYear+ID, DisplayColumn->SlotBGs[ID]);
        
        v2 AlbumP = {0, YearP.y}; 
        RenderText(&GlobalGameState, font_Small, &MD->Album, DisplayColumn->Colors.Text,
                   SongColumn->SongAlbum+ID, -0.12f, DisplayColumn->SlotBGs[ID], AlbumP);
        SetScissor(SongColumn->SongAlbum+ID, DisplayColumn->SlotBGs[ID]);
        
        v2 ArtistP = {Layout->SongXOffset, GetSongRow3YOffset(Layout, MD->Artist)};
        RenderText(&GlobalGameState, font_Small, &MD->Artist, DisplayColumn->Colors.Text,
                   SongColumn->SongArtist+ID, -0.12f, DisplayColumn->SlotBGs[ID], ArtistP);
        SetScissor(SongColumn->SongArtist+ID, DisplayColumn->SlotBGs[ID]);
        
        v2 TrackP = {Layout->SongTrackXOffset, SongP.y};
        RenderText(&GlobalGameState, font_Medium, &MD->TrackString, DisplayColumn->Colors.Text, SongColumn->SongTrack+ID, -0.12f, 
                   DisplayColumn->SlotBGs[ID], TrackP);
        SetScissor(SongColumn->SongTrack+ID, DisplayColumn->SlotBGs[ID]);
        
        DeleteStringCompound(&GlobalGameState.ScratchArena, &YearAddon);
    }
    SetScissor(SongColumn->Play[ID], DisplayColumn->SlotBGs[ID]);
    SetScissor(SongColumn->Add[ID], DisplayColumn->SlotBGs[ID]);
}

internal void
MoveDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn,
                  displayable_id DisplayableStartID, r32 StartY)
{ 
    playlist_column *PlaylistColumn = MusicInfo->Playlist_->Columns + DisplayColumn->Type;
    sort_batch      *SortBatch      = &PlaylistColumn->Batch;
    // FileOrDisplayableStartID is either a fileID when the colum is the song column, 
    // or a displayID when it is another.
    DisplayColumn->DisplayCursor = DisplayableStartID.ID*SlotHeight(DisplayColumn) + StartY;
    
    For(DisplayColumn->Count)
    {
        if(DisplayColumn->Type == columnType_Song)
        {
            RemoveSongText(DisplayColumn, It);
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, false);
        }
        else RemoveRenderText(Renderer, DisplayColumn->SlotText+It);
        DisplayColumn->SlotBGs[It]->ID->Render = false;
    }
    
    displayable_id NextID = DisplayableStartID;
    SetLocalPosition(DisplayColumn->SlotBGs[0], V2(0, StartY));
    
    for(u32 It = 0; 
        It < PlaylistColumn->Displayable.A.Count && 
        It < DisplayColumn->Count;
        It++)
    {
        DisplayColumn->SlotBGs[It]->ID->Render = true;
        
        if(DisplayColumn->Type == columnType_Song) 
        {
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, true);
            UpdateSongText(PlaylistColumn, DisplayColumn, It, NextID);
        }
        else
        {
            string_c *Name  = GetName(PlaylistColumn, NextID);
            Assert(Name);
            RenderText(&GlobalGameState, font_Small, Name, DisplayColumn->Colors.Text,
                       DisplayColumn->SlotText+It,  DisplayColumn->ZValue-0.01f, DisplayColumn->SlotBGs[It]);
            SetScissor(DisplayColumn->SlotText+It, DisplayColumn->SlotBGs[It]);
            
            r32 CurrentOffsetY = -SlotHeight(DisplayColumn)*0.5f - GetFontDescent(&GlobalGameState, font_Small, *Name) + GlobalGameState.Layout.SlotHeight*0.5f;
            Translate(DisplayColumn->SlotText+It, V2(0, CurrentOffsetY));
        }
        DisplayColumn->SlotIDs[It] = NextID;
        // #LastSlotOverflow, The last ID is not visible when the column is at the bottom, 
        // but the NextID would be out of displayable range.
        if(NextID.ID+1 < (i32)PlaylistColumn->Displayable.A.Count) NextID.ID++; 
    }
    ResetColumnText(DisplayColumn, PlaylistColumn->Displayable.A.Count);
    UpdateColumnColor(DisplayColumn, PlaylistColumn);
    
    UpdateColumnVerticalSliderPosition(DisplayColumn, PlaylistColumn->Displayable.A.Count);
}

internal void
ScrollDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn, r32 ScrollAmount)
{
    playlist_column *PlaylistColumn = MusicInfo->Playlist_->Columns + DisplayColumn->Type;
    r32 CurrentSlotHeight = SlotHeight(DisplayColumn);
    r32 TotalHeight   = GetDisplayableHeight(PlaylistColumn->Displayable.A.Count, CurrentSlotHeight);
    // DisplayCursor is the very top position. Therefore MaxHeight needs to be reduced by VisibleHeight
    TotalHeight = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    i32 PrevCursorID = Floor(DisplayColumn->DisplayCursor/CurrentSlotHeight);
    DisplayColumn->DisplayCursor = Clamp(DisplayColumn->DisplayCursor+ScrollAmount, 0.0f, TotalHeight);
    
    r32 NewY = Mod(DisplayColumn->DisplayCursor, CurrentSlotHeight);
    i32 NewCursorID = Floor(DisplayColumn->DisplayCursor/CurrentSlotHeight);
    NewCursorID     = Min(NewCursorID, Max(0, (i32)PlaylistColumn->Displayable.A.Count-1));
    i32 CursorDiff  = NewCursorID - PrevCursorID;
    
    if(CursorDiff != 0)
    {
        i32 MaximumID = Max(0, (i32)PlaylistColumn->Displayable.A.Count - (i32)DisplayColumn->Count + 1);
        displayable_id DID = NewDisplayableID(Min(NewCursorID, MaximumID));
        Assert(DID >= 0);
        MoveDisplayColumn(Renderer, MusicInfo, DisplayColumn, DID, NewY);
    }
    else
    {
        SetLocalPosition(DisplayColumn->SlotBGs[0], V2(0, NewY));
    }
}

inline void
UpdateAllDisplayColumns(game_state *GameState)
{
    renderer *Renderer = &GameState->Renderer;
    music_display_info *DisplayInfo = &GameState->MusicInfo.DisplayInfo;
    
    MoveDisplayColumn(Renderer, &GameState->MusicInfo, &DisplayInfo->Song.Base);
    MoveDisplayColumn(Renderer, &GameState->MusicInfo, &DisplayInfo->Genre);
    MoveDisplayColumn(Renderer, &GameState->MusicInfo, &DisplayInfo->Artist);
    MoveDisplayColumn(Renderer, &GameState->MusicInfo, &DisplayInfo->Album);
}

internal void
BringDisplayableEntryOnScreen(music_info *MusicInfo, display_column *DisplayColumn, playlist_id PlaylistID)
{
    playlist_column *PlaylistColumn = MusicInfo->Playlist_->Columns + DisplayColumn->Type;
    displayable_id DisplayID = PlaylistIDToColumnDisplayID(MusicInfo, DisplayColumn, PlaylistID);
    i32 MaximumID = Max(0, (i32)PlaylistColumn->Displayable.A.Count-(i32)DisplayColumn->Count+1);
    DisplayID.ID = Min(DisplayID.ID, MaximumID);
    
    MoveDisplayColumn(&GlobalGameState.Renderer, MusicInfo, DisplayColumn, DisplayID, 0);
}

internal void
BringDisplayableEntryOnScreenWithSortID(music_info *MusicInfo, display_column *DisplayColumn, batch_id BatchID)
{
    playlist_column *PlaylistColumn = MusicInfo->Playlist_->Columns + DisplayColumn->Type;
    displayable_id DisplayID = SortingIDToColumnDisplayID(MusicInfo->Playlist_, DisplayColumn, BatchID);
    i32 MaximumID = Max(0, (i32)PlaylistColumn->Displayable.A.Count-(i32)DisplayColumn->Count+1);
    DisplayID.ID = Min(DisplayID.ID, MaximumID);
    
    MoveDisplayColumn(&GlobalGameState.Renderer, MusicInfo, DisplayColumn, DisplayID, 0);
}

internal void
KeepPlayingSongOnScreen(renderer *Renderer, music_info *MusicInfo)
{
    if(MusicInfo->PlayingSong.DisplayableID < 0) return;
    display_column *DisplayColumn = &MusicInfo->DisplayInfo.Song.Base;
    u32 OnScreenID = 0;
    if(IsInOnScreenList(DisplayColumn, MusicInfo->PlayingSong.DisplayableID, &OnScreenID))
    {
        if(IsIntersectingRectButTopShowing(DisplayColumn->SlotBGs[OnScreenID], DisplayColumn->BottomBorder))
        {
            ScrollDisplayColumn(Renderer, MusicInfo, DisplayColumn, SlotHeight(DisplayColumn));
        }
        else if(IsIntersectingRectButBottomShowing(DisplayColumn->SlotBGs[OnScreenID], DisplayColumn->TopBorder))
        {
            ScrollDisplayColumn(Renderer, MusicInfo, DisplayColumn, -SlotHeight(DisplayColumn));
        }
    }
}

internal void
UpdateSlots(game_state *GS, display_column *DisplayColumn)
{
    v2 Dim = V2(DisplayColumn->SlotWidth, SlotHeight(DisplayColumn) - GS->Layout.SlotGap);
    For(DisplayColumn->Count)
    {
        r32 YDown = (It > 0) ? -SlotHeight(DisplayColumn) : 0;
        SetLocalPosition(DisplayColumn->SlotBGs[It], V2(0, YDown));
        SetSize(DisplayColumn->SlotBGs[It], Dim);
        if(DisplayColumn->Type == columnType_Song) 
        {
            display_column_song *SongColumn = ColumnExt(DisplayColumn);
            r32 YOff = GetSongButtonYOffset(&GS->Layout);
            SetLocalPosition(SongColumn->Play[It], V2(GetLocalPosition(SongColumn->Play[It]).x, YOff));
        }
    }
}

internal void
FitDisplayColumnIntoSlot(renderer *Renderer, display_column *DisplayColumn, u32 DisplayableCount)
{
    // Calc new column scale.
    DisplayColumn->ColumnHeight = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    DisplayColumn->SlotWidth    = WidthBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->RightBorder);
    // We need a minimal size, 0 is produces bugs.
    DisplayColumn->ColumnHeight = Max(DisplayColumn->ColumnHeight, 0.1f);
    DisplayColumn->SlotWidth    = Max(DisplayColumn->SlotWidth, 0.1f);
    
    // Calc new column translation.
    r32 CenterX = CenterXBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->RightBorder);
    r32 CenterY = CenterYBetweenRects(DisplayColumn->BottomBorder, DisplayColumn->TopBorder);
    
    SetSize(DisplayColumn->Background, V2(DisplayColumn->SlotWidth, DisplayColumn->ColumnHeight));
    SetLocalPosition(DisplayColumn->Background, V2(CenterX, CenterY));
    
    // Calc slot and text positions.
    SetPosition(DisplayColumn->SlotBGAnchor, V2(CenterX, GetPosition(DisplayColumn->SlotBGAnchor).y));
    UpdateSlots(&GlobalGameState, DisplayColumn);
    ResetColumnText(DisplayColumn, DisplayableCount);
    
    // Calc slider fit.
    r32 BGSizeY = GetSize(DisplayColumn->SliderHorizontal.Background).y;
    r32 BGPosY  = GetLocalPosition(DisplayColumn->SliderHorizontal.Background).y;
    SetSize(DisplayColumn->SliderHorizontal.Background, V2(DisplayColumn->SlotWidth, BGSizeY));
    SetPosition(DisplayColumn->SliderHorizontal.Background, V2(CenterX, BGPosY));
    
    slider *VSlider = &DisplayColumn->SliderVertical;
    SetSize(VSlider->Background, V2(GetSize(VSlider->Background).x, DisplayColumn->ColumnHeight));
    UpdateColumnVerticalSlider(DisplayColumn, DisplayableCount);
    
    // Calc new position for search button rects.
    v2 BetweenParentSize = GetSize(GetParent(DisplayColumn->BetweenSliderRect));
    v2 BetweenSize       = GetSize(DisplayColumn->BetweenSliderRect);
    r32 BetweenWidth  = BetweenParentSize.x*0.5f + BetweenSize.x*0.5f;
    r32 BetweenHeight = BetweenParentSize.y*0.5f - BetweenSize.y*0.5f;
    SetLocalPosition(DisplayColumn->BetweenSliderRect, V2(-BetweenWidth, -BetweenHeight));
    
    // Search bar.
    SetSize(DisplayColumn->Search.TextField.Background,
            V2(DisplayColumn->SlotWidth, GetSize(DisplayColumn->Search.TextField.Background).y)); // @Layout
    SetLocalPosition(DisplayColumn->Search.TextField.LeftAlign, V2(DisplayColumn->SlotWidth*-0.5f, 0));
    
    // Fitting the playlist panel at this point, as nothing depends on it being set directly.
    // This is a good place to set it, as it is technically part of the column itself.
    if(DisplayColumn->Type == columnType_Playlists)
    {
        playlist_ui *PlaylistUI = &DisplayColumn->Base->PlaylistUI;
        r32 PLCenterX = CenterXBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->Base->PlaylistsGenreEdge.Edge);
        r32 PLWidth   = WidthBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->Base->PlaylistsGenreEdge.Edge);
        SetSize(PlaylistUI->Panel, V2(PLWidth, GetSize(PlaylistUI->Panel).y));
        SetLocalPosition(PlaylistUI->Panel, V2(PLCenterX, GetLocalPosition(PlaylistUI->Panel).y));
        
        r32 PanelEdgeX = GetSize(PlaylistUI->Panel).x*0.5f - GetSize(PlaylistUI->PanelRightEdge).x*0.5f;
        SetLocalPosition(PlaylistUI->PanelRightEdge, V2(PanelEdgeX,0));
        
        text_field *RenameField = &PlaylistUI->RenameField;
        if(RenameField->IsActive)
        {
            SetSize(RenameField->Background, V2(DisplayColumn->SlotWidth, GetSize(RenameField->Background).y));
            SetLocalPosition(RenameField->LeftAlign, V2(DisplayColumn->SlotWidth*-0.5f, 0));
        }
    }
}

internal void
ProcessWindowResizeForDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn)
{
    array_playlist_id *Displayable = &MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable;
    
    u32 NewDisplayCount = CountPossibleDisplayedSlots(Renderer, DisplayColumn);
    ChangeDisplayColumnCount(Renderer, DisplayColumn, NewDisplayCount);
    
    FitDisplayColumnIntoSlot(Renderer, DisplayColumn, Displayable->A.Count);
    MoveDisplayColumn(Renderer, MusicInfo, DisplayColumn, DisplayColumn->SlotIDs[0],GetLocalPosition(DisplayColumn->SlotBGs[0]).y);
    
    // I do this to fix that if the column is at the bottom and the window gets bigger, the
    // slots will be stopped at the edge. Without this, the new visible slots will be the same
    // ID. See: #LastSlotOverflow in MoveDisplayColumn
    drag_slider_data Data = { MusicInfo, DisplayColumn };
    OnVerticalSliderDrag(Renderer, GetPosition(DisplayColumn->SliderVertical.GrabThing), 0, &Data);
}

inline void
OnNextSong(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    
    HandleChangeToNextSong(GameState);
    if(Info->PlayingSong->DisplayableID == -1) 
    {
        GameState->MusicInfo.IsPlaying = false;
    }
}

inline void
OnPreviousSong(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    
    if(GameState->SoundThreadInterface->CurrentPlaytime < 5.0f) // TODO:: Actually save current play time locally. now it is not really thread save
    {
        SetPreviousSong(MusicInfo);
        ChangeSong(GameState, Info->PlayingSong);
        KeepPlayingSongOnScreen(&GameState->Renderer, MusicInfo);
    }
    else
    {
        ChangeSong(GameState, Info->PlayingSong);
    }
}

inline void
OnPlayPauseSongToggleOn(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    
    if(Info->GameState->MusicInfo.PlayingSong.DisplayableID < 0) 
    {
        Info->GameState->MusicInfo.PlayingSong.DisplayableID.ID = 0;
        Info->GameState->MusicInfo.PlayingSong.PlaylistID = GetPlaylistID(&Info->GameState->MusicInfo, NewDisplayableID(0));
        ChangeSong(Info->GameState, &Info->GameState->MusicInfo.PlayingSong);
    }
    
    Info->GameState->MusicInfo.IsPlaying = true;
    
}

inline void
OnPlayPauseSongToggleOff(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    
    Info->GameState->MusicInfo.IsPlaying = false;
    PushSoundBufferClear(Info->GameState->SoundThreadInterface);
}

inline void
OnStopSong(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    
    //if(Info->PlayingSong->DisplayableID >= 0)
    {
        ChangeSong(GameState, Info->PlayingSong);
        Info->GameState->MusicInfo.IsPlaying = false;
        PushSoundBufferClear(GameState->SoundThreadInterface);
        
        GameState->MusicInfo.DisplayInfo.PlayPause->State = buttonState_Unpressed;
        SetColor(GameState->MusicInfo.DisplayInfo.PlayPause->Entry, GameState->MusicInfo.DisplayInfo.PlayPause->BaseColor);
    }
}

inline void
OnLoopPlaylistToggleOn(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    
    GameState->MusicInfo.Playlist_->Looping = playLoop_Loop;
    
    SaveLoopingState(GameState, GameState->MusicInfo.Playlist_);
}

inline void
OnLoopPlaylistToggleOff(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    
    GameState->MusicInfo.Playlist_->Looping = playLoop_NoLoop;
    
    SaveLoopingState(GameState, GameState->MusicInfo.Playlist_);
}

inline void
OnShufflePlaylistToggleOn(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->Playlist_->IsShuffled = true;
    
    ShufflePlaylist(MusicInfo->Playlist_, false);
    UpdatePlayingSong(MusicInfo);
    MoveDisplayColumn(&GameState->Renderer, MusicInfo, &MusicInfo->DisplayInfo.Song.Base);
    
    SaveShuffledState(GameState, MusicInfo->Playlist_);
}

inline void
OnShufflePlaylistToggleOff(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->Playlist_->IsShuffled = false;
    
    SortDisplayables(MusicInfo, &GameState->MP3Info->FileInfo);
    UpdatePlayingSong(MusicInfo);
    MoveDisplayColumn(&GameState->Renderer, MusicInfo, &MusicInfo->DisplayInfo.Song.Base);
    
    SaveShuffledState(GameState, MusicInfo->Playlist_);
}

inline void
CreateBasicColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(11, 15, 14);
    Palette->Text             = Color(144,176,128);//Color(165, 189, 184);
    Palette->Selected         = Color(8, 30, 8);//Color(13, 40, 13);
    Palette->Foreground       = Color(3, 25, 3);
    Palette->ForegroundText   = Color(144,176,128);
    Palette->SliderBackground = Color(3, 22, 3);
    Palette->SliderGrabThing  = Color(10, 40, 3);
    Palette->ButtonActive     = Color(25, 56, 18);
    Palette->PlayingSong      = Color(23, 45, 23);
    Palette->ErrorText        = Color(170, 11, 22);
}

inline void
CreateEvilColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(11, 15, 14);
    Palette->Text             = Color(221,179,177);
    Palette->Selected         = Color(8, 5, 5);
    Palette->Foreground       = Color(31, 9, 9);
    Palette->ForegroundText   = Color(221,179,177);
    Palette->SliderBackground = Color(28, 6, 6);
    Palette->SliderGrabThing  = Color(51, 9, 9);
    Palette->ButtonActive     = Color(61, 15, 15);
    Palette->PlayingSong      = Color(23, 20, 20);
    Palette->ErrorText        = Color(170, 11, 22);
}

inline void
CreateAquaColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(3,7,22);
    Palette->Text             = Color(81,154,173);
    Palette->Selected         = Color(50,54,78);
    Palette->Foreground       = Color(0,14,48);
    Palette->ForegroundText   = Color(81,154,173);
    Palette->SliderBackground = Color(3,18,58);
    Palette->SliderGrabThing  = Color(50,54,78);
    Palette->ButtonActive     = Color(70,68,94);
    Palette->PlayingSong      = Color(35,39,63);
    Palette->ErrorText        = Color(170, 11, 22);
}

inline void
CreateMonochromeColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(151,153,153);
    Palette->Text             = Color(17,10,10);
    Palette->Selected         = Color(220,220,220);
    Palette->Foreground       = Color(30,30,30);
    Palette->ForegroundText   = Palette->Slot;//Color(57,50,50);
    Palette->SliderBackground = Color(34,33,34);
    Palette->SliderGrabThing  = Color(70,73,73);
    Palette->ButtonActive     = Color(86,88,88);
    Palette->PlayingSong      = Color(235,235,235);
    Palette->ErrorText        = Color(170, 11, 22);
}

inline void
CreateMonoInvertedColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(30,30,30);
    Palette->Text             = Color(220,220,220);
    Palette->Selected         = Color(53,50,50);
    Palette->Foreground       = Color(151,153,153);
    Palette->ForegroundText   = Palette->Slot;//Color(100,100,100);
    Palette->SliderBackground = Color(131,133,133);
    Palette->SliderGrabThing  = Color(70,73,73);
    Palette->ButtonActive     = Color(78,80,80);
    Palette->PlayingSong      = Color(38,35,35);
    Palette->ErrorText        = Color(170, 11, 22);
}

inline void
CreateCustomColorPalette(color_palette *Palette, u32 CustomID)
{
    *Palette = GlobalGameState.Settings.Palettes[CustomID];
    
    r32 Div = 255.0f;
    Palette->Slot             /= Div;
    Palette->Text             /= Div;
    Palette->Selected         /= Div;
    Palette->Foreground       /= Div;
    Palette->ForegroundText   /= Div;
    Palette->SliderBackground /= Div;
    Palette->SliderGrabThing  /= Div;
    Palette->ButtonActive     /= Div;
    Palette->PlayingSong      /= Div;
    Palette->ErrorText        /= Div;
}

inline void
UpdateColorPalette(music_display_info *DisplayInfo, b32 GoToNextPalette)
{
    u32 PaletteAmount = DEFAULT_COLOR_PALETTE_COUNT+GlobalGameState.Settings.PaletteCount;
    if(GoToNextPalette) DisplayInfo->ColorPaletteID = ++DisplayInfo->ColorPaletteID%PaletteAmount;
    else if(DisplayInfo->ColorPaletteID >= PaletteAmount) DisplayInfo->ColorPaletteID = 0;
    
    switch(DisplayInfo->ColorPaletteID)
    {
        case 0: CreateBasicColorPalette(&DisplayInfo->ColorPalette); break;
        case 1: CreateEvilColorPalette(&DisplayInfo->ColorPalette);  break;
        case 2: CreateAquaColorPalette(&DisplayInfo->ColorPalette); break;
        case 3: CreateMonochromeColorPalette(&DisplayInfo->ColorPalette); break;
        case 4: CreateMonoInvertedColorPalette(&DisplayInfo->ColorPalette); break;
        default:
        {
            u32 CustomPaletteID = DisplayInfo->ColorPaletteID-DEFAULT_COLOR_PALETTE_COUNT;
            CreateCustomColorPalette(&DisplayInfo->ColorPalette, CustomPaletteID);
        }
    }
}

inline b32
IsColorPaletteDefault()
{
    return (GlobalGameState.MusicInfo.DisplayInfo.ColorPaletteID < DEFAULT_COLOR_PALETTE_COUNT);
}

internal void
RemoveCustomColorPalette(u32 PaletteID)
{
    serialization_settings *Settings = &GlobalGameState.Settings;
    PaletteID -= DEFAULT_COLOR_PALETTE_COUNT;
    Assert(PaletteID >= 0);
    Assert(PaletteID < Settings->PaletteCount);
    
    RemoveItem(Settings->Palettes, Settings->PaletteCount, PaletteID, color_palette);
    RemoveItem(Settings->PaletteNames, Settings->PaletteCount, PaletteID, string_c);
    
    Settings->PaletteCount--;
    Assert(Settings->PaletteCount >= 0);
}

internal void
AddCustomColorPalette(color_palette *ColorPalette, string_c *Name)
{
    serialization_settings *Settings = &GlobalGameState.Settings;
    if(Settings->PaletteCount+1 >= Settings->PaletteMaxCount)
    {
        NewLocalString(ErrorMsg, 255, "ERROR:: Created too many color palettes at once. Restart App if you want more!");
        PushErrorMessage(&GlobalGameState, ErrorMsg);
    }
    else
    {
        Settings->PaletteNames[Settings->PaletteCount] = NewStringCompound(&GlobalGameState.FixArena, 100);
        if(Name->Pos >= 100) Name->Pos = 100;
        AppendStringCompoundToCompound(Settings->PaletteNames+Settings->PaletteCount, Name);
        For(PALETTE_COLOR_AMOUNT)
        {
            Settings->Palettes[Settings->PaletteCount].Colors[It] = ColorPalette->Colors[It]*255.0f;
        }
        Settings->PaletteCount++;
        // TODO:: Maybe write it out immidiately that it cannot get lost?
    }
}

inline void
OnPaletteSwap(void *Data)
{
    music_display_info *DisplayInfo = &((music_btn *)Data)->GameState->MusicInfo.DisplayInfo;
    UpdateColorPalette(DisplayInfo, true);
}

inline string_c *
GetCurrentPaletteName()
{
    string_c *Result = 0;
    
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    if(DisplayInfo->ColorPaletteID < DEFAULT_COLOR_PALETTE_COUNT) 
        Result = GlobalDefaultColorPaletteNames + DisplayInfo->ColorPaletteID;
    else Result = GlobalGameState.Settings.PaletteNames + (DisplayInfo->ColorPaletteID-DEFAULT_COLOR_PALETTE_COUNT);
    
    return Result;
}

inline void
OnMusicPathPressed(void *Data)
{
    music_btn *MusicBtnInfo = (music_btn *)Data;
    music_display_info *DisplayInfo = &MusicBtnInfo->GameState->MusicInfo.DisplayInfo;
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    renderer *Renderer = &MusicBtnInfo->GameState->Renderer;
    
    SetActive(&MusicPath->TextField, !MusicPath->TextField.IsActive);
    if(MusicPath->TextField.IsActive)
    {
        UpdateTextField(&MusicBtnInfo->GameState->Renderer, &MusicPath->TextField);
        
        string_c PathText = NewStringCompound(&MusicBtnInfo->GameState->ScratchArena, 255+12);
        AppendStringToCompound(&PathText, (u8 *)"Old Path:     ");
        if(MusicBtnInfo->GameState->MP3Info->FolderPath.Pos == 0)
            AppendStringToCompound(&PathText, (u8 *)" - ");
        else AppendStringCompoundToCompound(&PathText, &MusicBtnInfo->GameState->MP3Info->FolderPath);
        RemoveRenderText(Renderer, &MusicPath->CurrentPath);
        r32 TextY = GetSize(MusicPath->TextField.Background).y*0.5f - GetFontDescent(MusicBtnInfo->GameState, font_Medium, PathText);
        RenderText(&GlobalGameState, font_Medium, &PathText, &DisplayInfo->ColorPalette.ForegroundText, &MusicPath->CurrentPath, -0.6f-0.001f, MusicPath->TextField.LeftAlign, V2(0, TextY));
        DeleteStringCompound(&MusicBtnInfo->GameState->ScratchArena, &PathText);
    }
    else 
    {
        RemoveRenderText(Renderer, &MusicPath->TextField.Text);
        SetActive(&MusicPath->LoadingBar, false);
    }
    MusicPath->Background->ID->Render = MusicPath->TextField.IsActive;
    SetActive(&MusicPath->CurrentPath, MusicPath->TextField.IsActive);
    SetActive(MusicPath->Save, MusicPath->TextField.IsActive);
    SetActive(MusicPath->Quit, MusicPath->TextField.IsActive);
    SetActive(MusicPath->Rescan, MusicPath->TextField.IsActive);
    RemoveRenderText(Renderer, &MusicPath->Output);
    
    ResetStringCompound(MusicPath->TextField.TextString);
    MusicPath->TextField.dBackspacePress = 0;
}

internal void
TestFolderSearchDone(music_path_ui *MusicPath, u32 FoundCount)
{
    ResetStringCompound(MusicPath->OutputString);
    AppendStringToCompound(&MusicPath->OutputString, (u8 *)"Found ");
    I32ToString(&MusicPath->OutputString, FoundCount);
    AppendStringToCompound(&MusicPath->OutputString, (u8 *)" .mp3 files at given folder.");
    if(FoundCount > 0) 
    {
        AppendStringToCompound(&MusicPath->OutputString, (u8 *) "\n\nStarting to collect metadata.\nThis may take some time dependent on the amount of mp3 files.");
    }
    
    RemoveRenderText(&GlobalGameState.Renderer, &MusicPath->Output);
    RenderText(&GlobalGameState, font_Medium, &MusicPath->OutputString,
               &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.ForegroundText, &MusicPath->Output, -0.6f-0.001f, MusicPath->TextField.LeftAlign, V2(0, -175));
}

inline void
OnMusicPathSavePressed(void *Data)
{
    music_btn *MusicBtnInfo = (music_btn *)Data;
    game_state *GameState = MusicBtnInfo->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    music_path_ui *MusicPath = &MusicInfo->DisplayInfo.MusicPath;
    
    ResetStringCompound(GameState->CrawlInfo.TestPath);
    AppendStringCompoundToCompound(&GameState->CrawlInfo.TestPath, &MusicPath->TextField.TextString);
    // Add '\' if non was given
    if(GameState->CrawlInfo.TestPath.Pos > 0 &&
       GameState->CrawlInfo.TestPath.S[GameState->CrawlInfo.TestPath.Pos-1] != '\\')
    {
        AppendCharToCompound(&GameState->CrawlInfo.TestPath, '\\'); 
    }
    
    MusicPath->CrawlThreadStateCount = 1;
    
    AddJob_LoadMetadata(GameState);
}

inline void
OnMusicPathQuitPressed(void *Data)
{
    OnMusicPathPressed(Data);
}

inline void
FinishedSettingUpMusicPath(game_state *GameState, music_path_ui *MusicPath)
{
    AppendStringToCompound(&MusicPath->OutputString, (u8 *)"\n\nFinished!");
    RemoveRenderText(&GameState->Renderer, &MusicPath->Output);
    RenderText(GameState, font_Medium, &MusicPath->OutputString,
               &GameState->MusicInfo.DisplayInfo.ColorPalette.ForegroundText, &MusicPath->Output, -0.6f-0.001f, MusicPath->TextField.LeftAlign, V2(0, -175)); // @Layout
}

inline void
ProcessMusicPath(renderer *Renderer, r32 dTime, input_info *Input, text_field *TextField)
{
    text_field_flag_result FieldResult = ProcessTextField(Renderer, dTime, Input, TextField);
    
    if(FieldResult.Flag & processTextField_TextChanged)
    {
    }
    if(FieldResult.Flag & processTextField_Confirmed)
    {
    }
}

inline void
OnRescanPressed(void *Data)
{
    AddJob_CheckMusicPathChanged(&GlobalGameState.CheckMusicPath);
}

inline void
OnShortcutHelpOn(void *Data)
{
    shortcut_popups *Popups = (shortcut_popups *)Data;
    Popups->IsActive = true;
    ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, &Popups->Popup, Popups->Help, 
               GetFontSize(&GlobalGameState.Renderer, font_Medium));
    Popups->ActiveText = 18;
    Popups->IsHovering = false;
}

inline void
OnShortcutHelpOff(void *Data)
{
    shortcut_popups *Popups = (shortcut_popups *)Data;
    Popups->IsActive = false;
    SetActive(&Popups->Popup, false);
}

internal void
InitializeDisplayInfo(music_display_info *DisplayInfo, game_state *GameState, mp3_info *MP3Info, 
                      layout_definition *Layout)
{
    color_palette *Palette = &DisplayInfo->ColorPalette;
    renderer *Renderer = &GameState->Renderer;
    r32 WWidth = (r32)Renderer->Window.FixedDim.Width;
    r32 WHeight = (r32)Renderer->Window.FixedDim.Height;
    
    DisplayInfo->Song.FileInfo = &MP3Info->FileInfo;
    CreateBasicColorPalette(&DisplayInfo->ColorPalette);
    
    r32 TopBorder    = Layout->TopBorder;
    r32 BottomBorder = GetBottomPanelHeight(Layout);
    r32 LeftBorder   = Layout->LeftBorder;
    r32 RightBorder  = Layout->RightBorder;
    
    DisplayInfo->EdgeTop = CreateRenderRect(Renderer, {{0, WHeight-TopBorder},{WWidth, WHeight}}, -0.5f,
                                            0, &Palette->Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeTop, fixedTo_TopCenter, scaleAxis_X);
    DisplayInfo->EdgeBottom = CreateRenderRect(Renderer, {{0, 0},{WWidth, BottomBorder}}, -0.5f,
                                               0, &Palette->Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeBottom, fixedTo_BottomCenter, scaleAxis_X);
    
    r32 HeightEdge = HeightBetweenRects(DisplayInfo->EdgeTop, DisplayInfo->EdgeBottom);
    
    DisplayInfo->EdgeLeft = CreateRenderRect(Renderer, {{0,BottomBorder},{LeftBorder,WHeight-TopBorder}}, -0.5f,
                                             0, &Palette->Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeLeft, fixedTo_MiddleLeft, scaleAxis_Y);
    DisplayInfo->EdgeRight = CreateRenderRect(Renderer, {{WWidth-RightBorder,BottomBorder},{WWidth,WHeight-TopBorder}}, 
                                              -0.5f, 0, &Palette->Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeRight, fixedTo_MiddleRight, scaleAxis_Y);
    
    
    // Buttons *********************************************
    
    Renderer->ButtonColors = 
    {
        &Palette->SliderGrabThing,
        &Palette->ButtonActive,
        &Palette->SliderBackground, 
        &Palette->Text,
    };
    
    // NOTE:: This data is included binary data, which is generated from the actual image files and put into C-Arrays.
#if RESOURCE_PNG
    
    Renderer->ButtonBaseID = DecodeAndCreateGLTexture(Background_Icon_DataCount, (u8 *)Background_Icon_Data);
    u32 LoopID             = DecodeAndCreateGLTexture(Loop_Icon_DataCount, (u8 *)Loop_Icon_Data);
    u32 LoopPressedID      = DecodeAndCreateGLTexture(Loop_Pressed_Icon_DataCount, (u8 *)Loop_Pressed_Icon_Data);
    u32 RandomizeID        = DecodeAndCreateGLTexture(Randomize_Icon_DataCount, (u8 *)Randomize_Icon_Data);
    u32 RandomizePressedID = DecodeAndCreateGLTexture(Randomize_Pressed_Icon_DataCount, (u8 *)Randomize_Pressed_Icon_Data);
    u32 PlayID             = DecodeAndCreateGLTexture(Play_Icon_DataCount, (u8 *)Play_Icon_Data);
    u32 PauseID            = DecodeAndCreateGLTexture(Pause_Icon_DataCount, (u8 *)Pause_Icon_Data);
    u32 StopID             = DecodeAndCreateGLTexture(Stop_Icon_DataCount, (u8 *)Stop_Icon_Data);
    u32 NextID             = DecodeAndCreateGLTexture(Next_Icon_DataCount, (u8 *)Next_Icon_Data);
    u32 PreviousID         = DecodeAndCreateGLTexture(Previous_Icon_DataCount, (u8 *)Previous_Icon_Data);
    u32 MusicPathID        = DecodeAndCreateGLTexture(MusicPath_Icon_DataCount, (u8 *)MusicPath_Icon_Data);
    u32 ConfirmID          = DecodeAndCreateGLTexture(Confirm_Icon_DataCount, (u8 *)Confirm_Icon_Data);
    u32 CancelID           = DecodeAndCreateGLTexture(Cancel_Icon_DataCount, (u8 *)Cancel_Icon_Data);
    u32 PaletteID          = DecodeAndCreateGLTexture(PaletteSwap_Icon_DataCount, (u8 *)PaletteSwap_Icon_Data);
    u32 RescanID           = DecodeAndCreateGLTexture(Rescan_Icon_DataCount, (u8 *)Rescan_Icon_Data);
    u32 ColorPickerID      = DecodeAndCreateGLTexture(ColorPicker_Icon_DataCount, (u8 *)ColorPicker_Icon_Data);
    u32 ShortcutID         = DecodeAndCreateGLTexture(Help_Icon_DataCount, (u8 *)Help_Icon_Data);
    u32 ShortcutPressedID  = DecodeAndCreateGLTexture(Help_Pressed_Icon_DataCount, (u8 *)Help_Pressed_Icon_Data);
    u32 PLAddID            = DecodeAndCreateGLTexture(Playlist_Add_Icon_DataCount, (u8 *)Playlist_Add_Icon_Data);
    u32 PLAddSelectionID   = DecodeAndCreateGLTexture(Playlist_AddSelection_Icon_DataCount, 
                                                      (u8 *)Playlist_AddSelection_Icon_Data);
    u32 PLRemoveID         = DecodeAndCreateGLTexture(Playlist_Remove_Icon_DataCount, (u8 *)Playlist_Remove_Icon_Data);
    u32 PLRenameID         = DecodeAndCreateGLTexture(Playlist_Rename_Icon_DataCount, (u8 *)Playlist_Rename_Icon_Data);
#else
    arena_allocator *Scratch = &GameState->ScratchArena;
    bitmap_color_format cF = colorFormat_RGBA;
    
    Renderer->ButtonBaseID = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Background_Icon, cF));
    u32 LoopID             = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Loop_Icon, cF));
    u32 LoopPressedID      = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Loop_Pressed_Icon, cF));
    u32 RandomizeID        = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Randomize_Icon, cF));
    u32 RandomizePressedID = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Randomize_Pressed_Icon, cF));
    u32 PlayID             = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Play_Icon, cF));
    u32 PauseID            = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Pause_Icon, cF));
    u32 StopID             = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Stop_Icon, cF));
    u32 NextID             = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Next_Icon, cF));
    u32 PreviousID         = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Previous_Icon, cF));
    u32 MusicPathID        = DecodeAndCreateGLTexture(Scratch, NewBitmapData(MusicPath_Icon, cF));
    u32 ConfirmID          = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Confirm_Icon, cF));
    u32 CancelID           = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Cancel_Icon, cF));
    u32 PaletteID          = DecodeAndCreateGLTexture(Scratch, NewBitmapData(PaletteSwap_Icon, cF));
    u32 RescanID           = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Rescan_Icon, cF));
    //u32 ColorPickerID      = DecodeAndCreateGLTexture(Scratch, NewBitmapData(ColorPicker_Icon, cF));
    u32 StyleSettingsID    = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Style_Settings_Icon, cF));
    u32 ShortcutID         = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Help_Icon, cF));
    u32 ShortcutPressedID  = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Help_Pressed_Icon, cF));
    u32 PLAddID            = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Playlist_Add_Icon, cF));
    u32 PLAddSelectionID   = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Playlist_AddSelection_Icon, cF));
    u32 PLRemoveID         = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Playlist_Remove_Icon, cF));
    u32 PLRenameID         = DecodeAndCreateGLTexture(Scratch, NewBitmapData(Playlist_Rename_Icon, cF));
#endif
    
    r32 BtnDepth         = -0.6f;
    v2  PlayPauseP       = V2(Layout->PlayPauseButtonX, Layout->PlayPauseButtonY);
    r32 ButtonUpperLeftX = Layout->TopLeftButtonGroupStartX;
    r32 StepX            = Layout->SmallButtonExtents*2 + Layout->TopLeftButtonGroupGap;
    r32 Gap              = Layout->ButtonGap;
    
    r32 SmallRectS  = Layout->SmallButtonExtents;
    r32 MediumRectS = Layout->MediumButtonExtents;
    r32 LargeRectS  = Layout->LargeButtonExtents;
    r32 PlayPauseS  = Layout->PlayPauseButtonExtents;
    r32 TinyRectS   = Layout->SearchButtonExtents;
    rect PlayPauseRect = {{-PlayPauseS, -PlayPauseS}, {PlayPauseS,PlayPauseS}};
    rect LargeBtnRect  = {{-LargeRectS, -LargeRectS}, {LargeRectS, LargeRectS}};
    rect MediumBtnRect = {{-MediumRectS,-MediumRectS},{MediumRectS,MediumRectS}};
    rect SmallBtnRect  = {{-SmallRectS, -SmallRectS}, {SmallRectS,SmallRectS}};
    rect TinyBtnRect   = {{-TinyRectS, -TinyRectS}, {TinyRectS,TinyRectS}};
    
    DisplayInfo->PlayPause = NewButton(Renderer, PlayPauseRect, BtnDepth, true, Renderer->ButtonBaseID, 
                                       PlayID, Renderer->ButtonColors, 0, PauseID);
    SetLocalPosition(DisplayInfo->PlayPause, PlayPauseP);
    DisplayInfo->PlayPause->OnPressed = {OnPlayPauseSongToggleOn, &DisplayInfo->MusicBtnInfo};
    DisplayInfo->PlayPause->OnPressedToggleOff = {OnPlayPauseSongToggleOff, &DisplayInfo->MusicBtnInfo};
    
    r32 PlayPauseAndBtnX = PlayPauseP.x + PlayPauseS + LargeRectS;
    r32 LargeBtnDist = LargeRectS*2;
    r32 SNPBtnY = PlayPauseP.y + Layout->StopNextPrevBtnYOffsetFromPlayPause;
    
    DisplayInfo->Stop = NewButton(Renderer, LargeBtnRect, BtnDepth, false, Renderer->ButtonBaseID, StopID, Renderer->ButtonColors, 0);
    SetLocalPosition(DisplayInfo->Stop, V2(PlayPauseAndBtnX + Gap, SNPBtnY));
    DisplayInfo->Stop->OnPressed = {OnStopSong, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->Previous = NewButton(Renderer, LargeBtnRect, BtnDepth, false, 
                                      Renderer->ButtonBaseID, PreviousID, Renderer->ButtonColors, 0);
    SetLocalPosition(DisplayInfo->Previous, V2(PlayPauseAndBtnX + Gap + LargeBtnDist + Gap, SNPBtnY));
    DisplayInfo->Previous->OnPressed = {OnPreviousSong, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->Next = NewButton(Renderer, LargeBtnRect, BtnDepth, false, 
                                  Renderer->ButtonBaseID, NextID, Renderer->ButtonColors, 0);
    SetLocalPosition(DisplayInfo->Next, V2(PlayPauseAndBtnX + Gap + (LargeBtnDist + Gap)*2, SNPBtnY));
    DisplayInfo->Next->OnPressed = {OnNextSong, &DisplayInfo->MusicBtnInfo};
    
    r32 LSBtnYOffset = Layout->LoopShuffleBtnYOffsetFromPlayPause;
    DisplayInfo->ShufflePlaylist = NewButton(Renderer, MediumBtnRect, BtnDepth, true, Renderer->ButtonBaseID, 
                                             RandomizeID, Renderer->ButtonColors, 0, RandomizePressedID);
    SetLocalPosition(DisplayInfo->ShufflePlaylist, V2(PlayPauseP.x - (PlayPauseS+MediumRectS+Gap), 
                                                      PlayPauseP.y + LSBtnYOffset - (MediumRectS*2 + Gap)));
    DisplayInfo->ShufflePlaylist->OnPressed = {OnShufflePlaylistToggleOn, &DisplayInfo->MusicBtnInfo};
    DisplayInfo->ShufflePlaylist->OnPressedToggleOff = {OnShufflePlaylistToggleOff, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->LoopPlaylist = NewButton(Renderer, MediumBtnRect, BtnDepth, true, 
                                          Renderer->ButtonBaseID, LoopID, Renderer->ButtonColors, 0, LoopPressedID);
    SetLocalPosition(DisplayInfo->LoopPlaylist, V2(PlayPauseP.x - (PlayPauseS+MediumRectS+Gap), 
                                                   PlayPauseP.y + LSBtnYOffset));
    DisplayInfo->LoopPlaylist->OnPressed = {OnLoopPlaylistToggleOn,  &DisplayInfo->MusicBtnInfo};
    DisplayInfo->LoopPlaylist->OnPressedToggleOff = {OnLoopPlaylistToggleOff, &DisplayInfo->MusicBtnInfo};
    
    
    
    // All the Column stuff ******************************
    music_info *MusicInfo    = &GameState->MusicInfo;
    playlist_info *Playlist  = MusicInfo->Playlist_;
    column_colors ColumnColors = {
        &Palette->Slot,
        &Palette->Text,
        &Palette->Foreground,
        &Palette->SliderGrabThing,
        &Palette->SliderBackground,
        &Palette->Selected,
    };
    column_colors PLColumnColors = {
        &Palette->SliderBackground,
        &Palette->ForegroundText,
        &Palette->Slot,
        &Palette->SliderGrabThing,
        &Palette->SliderBackground,
        &Palette->Foreground,
    };
    
    r32 DragEdgeMinY    = BottomBorder;
    r32 DragEdgeMaxY    = WHeight-TopBorder;
    r32 PlaylistsGenreX = Layout->PlaylistsGenreXP*WWidth;
    r32 GenreArtistX    = Layout->GenreArtistXP*WWidth;
    r32 ArtistAlbumX    = Layout->ArtistAlbumXP*WWidth;
    r32 AlbumSongX      = Layout->AlbumSongXP*WWidth;
    CreateColumnDragEdges(GameState, DragEdgeMinY, DragEdgeMaxY, PlaylistsGenreX, GenreArtistX, ArtistAlbumX, AlbumSongX);
    
    // PlaylistsUI extra stuff ****
    playlist_ui *PlaylistUI = &DisplayInfo->PlaylistUI;
    PlaylistUI->BtnColors = {
        &Palette->Text,
        &Palette->SliderBackground, 
        &Palette->ButtonActive,
        &Palette->SliderGrabThing,
    };
    
    
    r32 PlaylistColumnDepth = 0.0f;
    r32 PanelDepth = PlaylistColumnDepth-0.012f;
    r32 PanelYMax  = WHeight - TopBorder;
    r32 PanelYMin  = PanelYMax - Layout->PlaylistPanelHeight;
    PlaylistUI->Panel = CreateRenderRect(Renderer, {{LeftBorder,PanelYMin},{PlaylistsGenreX,PanelYMax}}, 
                                         PanelDepth, 0, &Palette->Slot);
    TranslateWithScreen(&Renderer->TransformList, PlaylistUI->Panel, fixedTo_Top);
    
    PlaylistUI->PanelRightEdge = CreateRenderRect(Renderer, 
                                                  {{PlaylistsGenreX - Layout->VerticalSliderWidth,PanelYMin}, {PlaylistsGenreX,PanelYMax}}, 
                                                  PanelDepth-0.0021f, 0, &Palette->Foreground);
    SetParent(PlaylistUI->PanelRightEdge, PlaylistUI->Panel);
    
    PlaylistUI->BtnAnchor = CreateRenderRect(Renderer, V2(0), 0, &Palette->Foreground, 0);
    v2 AnchorP = GetPosition(PlaylistUI->Panel) + V2(-GetSize(PlaylistUI->Panel).x*0.5f + SmallRectS, 0);
    SetPosition(PlaylistUI->BtnAnchor, AnchorP);
    TranslateWithScreen(&Renderer->TransformList, PlaylistUI->BtnAnchor, fixedTo_TopLeft);
    
    r32 PLBtnYOff = Layout->PlaylistDividerHeight/2.0f;
    PlaylistUI->Add = NewButton(Renderer, TinyBtnRect, PanelDepth-0.002f, false, 
                                Renderer->ButtonBaseID, PLAddID, PlaylistUI->BtnColors, PlaylistUI->BtnAnchor);
    SetLocalPosition(PlaylistUI->Add, V2(0, -PLBtnYOff));
    PlaylistUI->Add->OnPressed = {OnAnimationDone, &PlaylistUI->AddCurtain.Activated};
    
    PlaylistUI->AddSelection = NewButton(Renderer, TinyBtnRect, PanelDepth-0.002f, false, Renderer->ButtonBaseID, 
                                         PLAddSelectionID, PlaylistUI->BtnColors, PlaylistUI->BtnAnchor);
    SetLocalPosition(PlaylistUI->AddSelection, V2(SmallRectS*2 + Layout->TopLeftButtonGroupGap, -PLBtnYOff));
    PlaylistUI->AddSelection->OnPressed = {OnAnimationDone, &PlaylistUI->AddSelectionCurtain.Activated};
    
    PlaylistUI->Remove = NewButton(Renderer, TinyBtnRect, PanelDepth-0.002f, false, 
                                   Renderer->ButtonBaseID, PLRemoveID, PlaylistUI->BtnColors, PlaylistUI->BtnAnchor);
    SetLocalPosition(PlaylistUI->Remove, V2(SmallRectS*2*3 + Layout->TopLeftButtonGroupGap*3, -PLBtnYOff));
    PlaylistUI->Remove->OnPressed = {OnAnimationDone, &PlaylistUI->RemoveCurtain.Activated};
    
    PlaylistUI->Rename = NewButton(Renderer, TinyBtnRect, PanelDepth-0.002f, false, 
                                   Renderer->ButtonBaseID, PLRenameID, PlaylistUI->BtnColors, PlaylistUI->BtnAnchor);
    SetLocalPosition(PlaylistUI->Rename, V2(SmallRectS*2*2 + Layout->TopLeftButtonGroupGap*2, -PLBtnYOff));
    PlaylistUI->Rename->OnPressed = {OnRenamePlaylistClick, GameState};
    
    r32 RenameHeight = SlotHeight(&DisplayInfo->Playlists);
    PlaylistUI->RenameField = CreateTextField(Renderer, &GameState->FixArena, V2(200.0f, RenameHeight - Layout->SlotGap), 
                                              PanelDepth-0.0019f, (u8 *)"New name...", 0, 
                                              PLColumnColors.Text, PLColumnColors.Slot, font_Small, PLAYLIST_MAX_NAME_LENGTH);
    UpdateTextField(Renderer, &PlaylistUI->RenameField);
    SetActive(&PlaylistUI->RenameField, false);
    
    string_c AddText          = NewStaticStringCompound("New Playlist.");
    string_c AddSelectionText = NewStaticStringCompound("New Playlist\nwith selection.");
    string_c RemoveText       = NewStaticStringCompound("Delete selected Playlist?");
    CreateQuitAnimation(&PlaylistUI->AddCurtain,          V2(1), &AddText,          0.8f, font_Small, 0, PlaylistColumnDepth-0.02f);
    CreateQuitAnimation(&PlaylistUI->AddSelectionCurtain, V2(1), &AddSelectionText, 0.8f, font_Small, 0, PlaylistColumnDepth-0.02f);
    CreateQuitAnimation(&PlaylistUI->RemoveCurtain,       V2(1), &RemoveText,       1.2f, font_Small, 0, PlaylistColumnDepth-0.02f);
    
    
    //                         ****
    
    DisplayInfo->Columns[0] = &DisplayInfo->Genre;
    DisplayInfo->Columns[1] = &DisplayInfo->Artist;
    DisplayInfo->Columns[2] = &DisplayInfo->Album;
    DisplayInfo->Columns[3] = &DisplayInfo->Song.Base;
    DisplayInfo->Columns[4] = &DisplayInfo->Playlists;
    
    column_info GenreColumn  = {Renderer, DisplayInfo, MusicInfo, &DisplayInfo->Genre, &Playlist->Genre};
    column_info ArtistColumn = {Renderer, DisplayInfo, MusicInfo, &DisplayInfo->Artist, &Playlist->Artist};
    column_info AlbumColumn  = {Renderer, DisplayInfo, MusicInfo, &DisplayInfo->Album, &Playlist->Album};
    column_info SongColumn   = {Renderer, DisplayInfo, MusicInfo, Parent(&DisplayInfo->Song), &Playlist->Song};
    column_info PlaylistsColumn = {Renderer, DisplayInfo, MusicInfo, &DisplayInfo->Playlists, &Playlist->Playlists};
    
    CreateDisplayColumn(GenreColumn, &GameState->FixArena, columnType_Genre, ColumnColors, -0.025f, 
                        DisplayInfo->PlaylistsGenreEdge.Edge, DisplayInfo->GenreArtistEdge.Edge, Layout);
    CreateDisplayColumn(ArtistColumn, &GameState->FixArena, columnType_Artist, ColumnColors, -0.05f, 
                        DisplayInfo->GenreArtistEdge.Edge, DisplayInfo->ArtistAlbumEdge.Edge, Layout);
    CreateDisplayColumn(AlbumColumn, &GameState->FixArena, columnType_Album, ColumnColors, -0.075f, 
                        DisplayInfo->ArtistAlbumEdge.Edge, DisplayInfo->AlbumSongEdge.Edge, Layout);
    CreateDisplayColumn(SongColumn, &GameState->FixArena, columnType_Song, ColumnColors, -0.1f, 
                        DisplayInfo->AlbumSongEdge.Edge, DisplayInfo->EdgeRight, Layout);
    CreateDisplayColumn(PlaylistsColumn, &GameState->FixArena, columnType_Playlists, PLColumnColors, PlaylistColumnDepth, 
                        DisplayInfo->EdgeLeft, DisplayInfo->PlaylistsGenreEdge.Edge, Layout);
    
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Playlists);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Genre);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Artist);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Album);
    MoveDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Song.Base);
    
    ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Playlists);
    ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Genre);
    ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Artist);
    ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Album);
    ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Song.Base);
    
    SwitchSelection(&DisplayInfo->Playlists, &Playlist->Playlists, 0);
    UpdateDisplayColumnColor(&DisplayInfo->Playlists, &Playlist->Playlists);
    
    // Song panel stuff **************************************
    r32 PlayPauseAndGap = PlayPauseP.x + PlayPauseS + Gap;
    r32 VolumeHeight    = PlayPauseS*2 - (LargeRectS*2 + Gap);
    r32 VolumeUpperY    = SNPBtnY - (LargeRectS+Gap);
    r32 VolumeXEnd      = PlayPauseAndGap+LargeRectS*2*3+Gap*2;
    DisplayInfo->Volume.Background = 
        CreateRenderRect(Renderer, {{PlayPauseAndGap, VolumeUpperY - VolumeHeight}, {VolumeXEnd, VolumeUpperY}}, 
                         BtnDepth, 0, &Palette->SliderBackground);
    DisplayInfo->Volume.GrabThing  = 
        CreateRenderRect(Renderer, {{PlayPauseAndGap, VolumeUpperY - VolumeHeight}, {PlayPauseAndGap+Layout->VolumeGrabThingWidth, VolumeUpperY}}, BtnDepth - 0.000001f, 0, &Palette->SliderGrabThing);
    DisplayInfo->Volume.MaxSlidePix =
        GetExtends(DisplayInfo->Volume.Background).x - GetExtends(DisplayInfo->Volume.GrabThing).x;
    OnVolumeDrag(Renderer, GetLocalPosition(DisplayInfo->Volume.Background), DisplayInfo->Volume.Background, GameState);
    
    playing_song_panel *Panel = &DisplayInfo->PlayingSongPanel;
    Panel->MP3Info = MP3Info;
    Panel->CurrentTimeString = NewStringCompound(&GameState->FixArena, 10);
    
    r32 TimelineX   = VolumeXEnd  + Layout->TimelineXGap;
    r32 TimelineGTY = (PlayPauseP.y - PlayPauseS);
    r32 TimelineY   = TimelineGTY + Layout->TimelineGrapThingHeight*0.5f;
    
    rect TimelineBG = {
        {TimelineX,                       TimelineY - Layout->TimelineHeight*0.5f}, 
        {TimelineX+Layout->TimelineWidth, TimelineY + Layout->TimelineHeight*0.5f}
    };
    rect TimelineGrab = {
        {TimelineX, TimelineGTY}, 
        {TimelineX+Layout->TimelineGrapThingWidth, TimelineGTY+Layout->TimelineGrapThingHeight}
    };
    CreateSlider(GameState, &Panel->Timeline, sliderAxis_X, TimelineBG, TimelineGrab, BtnDepth, false);
    
    SetNewPlayingSong(Renderer, Panel, Layout, &GameState->MusicInfo);
    
    DisplayInfo->SearchIsActive = -1;
    
    // Shortcut-Help button
    r32 BtnY = Layout->TopLeftButtonGroupY;
    DisplayInfo->Help = NewButton(Renderer, SmallBtnRect, BtnDepth, true, Renderer->ButtonBaseID, 
                                  ShortcutID, Renderer->ButtonColors, 0, ShortcutPressedID);
    SetLocalPosition(DisplayInfo->Help, V2(ButtonUpperLeftX, Renderer->Window.CurrentDim.Height-BtnY));
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->Help->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->Help->Icon, fixedTo_TopLeft);
    
    shortcut_popups *Popups = &DisplayInfo->Popups;
    DisplayInfo->Help->OnPressed          = {OnShortcutHelpOn, Popups};
    DisplayInfo->Help->OnPressedToggleOff = {OnShortcutHelpOff, Popups};
    
    // Palette swap button
    DisplayInfo->PaletteSwap = NewButton(Renderer, SmallBtnRect, BtnDepth, false, 
                                         Renderer->ButtonBaseID, PaletteID, Renderer->ButtonColors, 0);
    SetLocalPosition(DisplayInfo->PaletteSwap, V2(ButtonUpperLeftX + StepX*1, Renderer->Window.CurrentDim.Height-BtnY));
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->PaletteSwap->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->PaletteSwap->Icon, fixedTo_TopLeft);
    DisplayInfo->PaletteSwap->OnPressed = {OnPaletteSwap, &DisplayInfo->MusicBtnInfo};
    
    // Color picker button
    DisplayInfo->StyleSettings = NewButton(Renderer, SmallBtnRect, BtnDepth, false, 
                                           Renderer->ButtonBaseID, StyleSettingsID, Renderer->ButtonColors, 0);
    SetLocalPosition(DisplayInfo->StyleSettings, V2(ButtonUpperLeftX + StepX*2, Renderer->Window.CurrentDim.Height-BtnY));
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->StyleSettings->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->StyleSettings->Icon, fixedTo_TopLeft);
    DisplayInfo->StyleSettings->OnPressed = {OnStyleSettings, &GameState->StyleSettings};
    
    // Music path stuff *******************************
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    
    MusicPath->Button = NewButton(Renderer, SmallBtnRect, BtnDepth, false, 
                                  Renderer->ButtonBaseID, MusicPathID, Renderer->ButtonColors, 0);
    SetLocalPosition(MusicPath->Button, V2(ButtonUpperLeftX + StepX*3, Renderer->Window.CurrentDim.Height-BtnY));
    TranslateWithScreen(&Renderer->TransformList, MusicPath->Button->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, MusicPath->Button->Icon, fixedTo_TopLeft);
    MusicPath->Button->OnPressed = {OnMusicPathPressed, &DisplayInfo->MusicBtnInfo};
    
    entry_id *PathParent = CreateRenderRect(Renderer, V2(0), 0, 0, 0);
    PathParent->ID->Render = false;
    Translate(PathParent, V2(WWidth/2.0f, WHeight - Layout->MusicPathHeightOffset));
    TranslateWithScreen(&Renderer->TransformList, PathParent, fixedTo_FixYToGiven_XLeft, Layout->MusicPathHeightScaler);
    
    r32 TextFieldHeight = Layout->MusicPathTextFieldHeight;
    MusicPath->TextField = CreateTextField(Renderer, &GameState->FixArena, 
                                           V2(WWidth-WWidth*0.1f, TextFieldHeight), BtnDepth-0.001f,
                                           (u8 *)"New path to music folder...", 0, &Palette->Text, 
                                           &Palette->ButtonActive, font_Medium);
    Translate(&MusicPath->TextField, V2(WWidth/2.0f, WHeight - Layout->MusicPathHeightOffset));
    MusicPath->TextField.DoMouseHover = false;
    TransformWithScreen(&Renderer->TransformList, MusicPath->TextField.Background, fixedTo_FixYToGiven_XCenter, 
                        scaleAxis_X, Layout->MusicPathHeightScaler);
    MusicPath->TextField.LeftAlign->ID->Parent = PathParent;
    
    MusicPath->Background = CreateRenderRect(Renderer, V2(WWidth, WHeight), BtnDepth-0.0009f, 
                                             &Palette->Foreground, 0);
    Translate(MusicPath->Background, V2(WWidth/2.0f, WHeight/2.0f));
    TransformWithScreen(&Renderer->TransformList, MusicPath->Background, fixedTo_MiddleCenter, scaleAxis_XY);
    SetTransparency(MusicPath->Background, Layout->MusicPathBGTransparency);
    MusicPath->Background->ID->Render = false;
    
    MusicPath->Save = NewButton(Renderer, MediumBtnRect, BtnDepth-0.001f, false, 
                                Renderer->ButtonBaseID, ConfirmID, Renderer->ButtonColors, MusicPath->TextField.LeftAlign);
    Translate(MusicPath->Save, V2(MediumRectS, -MediumRectS - TextFieldHeight/2.0f - Layout->MusicPathButtonYOffset));
    MusicPath->Save->OnPressed = {OnMusicPathSavePressed, &DisplayInfo->MusicBtnInfo};
    SetActive(MusicPath->Save, MusicPath->TextField.IsActive);
    
    MusicPath->Quit = NewButton(Renderer, MediumBtnRect, BtnDepth-0.001f, false, 
                                Renderer->ButtonBaseID, CancelID, Renderer->ButtonColors, MusicPath->TextField.LeftAlign);
    Translate(MusicPath->Quit, V2(MediumRectS*3+Layout->MusicPathButtonGap, 
                                  -MediumRectS - TextFieldHeight/2.0f - Layout->MusicPathButtonYOffset));
    MusicPath->Quit->OnPressed = {OnMusicPathQuitPressed, &DisplayInfo->MusicBtnInfo};
    SetActive(MusicPath->Quit, MusicPath->TextField.IsActive);
    
    MusicPath->OutputString = NewStringCompound(&GameState->FixArena, 500);
    
    v2 LoadingBarSize = V2(WWidth-WWidth*0.1f, Layout->MusicPathLoadingBarHeight);
    MusicPath->LoadingBar = CreateLoadingBar(LoadingBarSize, BtnDepth-0.0011f, MusicPath->TextField.Background);
    SetLocalPosition(&MusicPath->LoadingBar, V2(0, -LoadingBarSize.y*0.5f - MediumRectS*2 - TextFieldHeight/2.0f - 
                                                Layout->MusicPathButtonYOffset*2));
    ScaleWithScreen(&Renderer->TransformList, MusicPath->LoadingBar.BG, scaleAxis_X);
    SetActive(&MusicPath->LoadingBar, false);
    
    MusicPath->CrawlThreadStateCount = 0;
    
    MusicPath->Rescan = NewButton(Renderer, MediumBtnRect, BtnDepth-0.001f, false, Renderer->ButtonBaseID, 
                                  RescanID, Renderer->ButtonColors, MusicPath->TextField.LeftAlign);
    Translate(MusicPath->Rescan, V2(Layout->RescanButtonXOffset, Layout->RescanButtonYOffset));
    SetActive(MusicPath->Rescan, false);
    MusicPath->Rescan->OnPressed = {OnRescanPressed, &DisplayInfo->MusicBtnInfo};
    
    // Quit curtain ****************************
    NewEmptyLocalString(LanguageText, 200);
    string_c QuitText = GetRandomExitMessage(GameState, &LanguageText);
    CreateQuitAnimation(&DisplayInfo->Quit, V2(WWidth, WHeight), &QuitText, Layout->QuitCurtainAnimationTime, font_Big, &LanguageText);
    
    
}

internal void
CreateShortcutPopups(music_display_info *DisplayInfo)
{
    shortcut_popups *Popups = &DisplayInfo->Popups;
    
    Popups->SearchGenre     = NewStaticStringCompound("Search genres by name. [F1]");
    Popups->SearchArtist    = NewStaticStringCompound("Search artists by name. [F2]");
    Popups->SearchAlbum     = NewStaticStringCompound("Search albums by name. [F3]");
    Popups->SearchSong      = NewStaticStringCompound("Search songs by name. [F4]");
    Popups->Help            = NewStaticStringCompound("If active, shows tooltip-popups for various UI-Elements. [F5]");
    Popups->PaletteSwap     = NewStaticStringCompound("Swap color palette. [F6]");
    Popups->ColorPicker     = NewStaticStringCompound("Modify and create new color palette. [F7]");
    Popups->MusicPath       = NewStaticStringCompound("Change path to music folder or rescan metadata. [F8]");
    Popups->SongPlay        = NewStaticStringCompound("Start playing this song.");
    Popups->SongAddToNextUp = NewStaticStringCompound("Add this song to the \"Play Next\" list.");
    Popups->Loop            = NewStaticStringCompound("If active, loops current song list.");
    Popups->Shuffle         = NewStaticStringCompound("If active, shuffles current song list.");
    Popups->PlayPause       = NewStaticStringCompound("Start and pause the currently playing song. [Space]");
    Popups->Stop            = NewStaticStringCompound("Stop the currently playing song (jumps to start of song).");
    Popups->Previous        = NewStaticStringCompound("Jumps to start of the song, or to the previous song if already at the beginning.");
    Popups->Next            = NewStaticStringCompound("Jumps to the next song.");
    Popups->Volume          = NewStaticStringCompound("Regulate the volume of the music. [+/-]");
    Popups->Timeline        = NewStaticStringCompound("Shows the progress of the song.");
    Popups->Quit            = NewStaticStringCompound("Hold or double tap [Escape] to quit application.");
    
    Popups->SaveMusicPath   = NewStaticStringCompound("Starts using the given path and searches for .mp3 files.");
    Popups->CancelMusicPath = NewStaticStringCompound("Quits the Music Path menu. [Escape].");
    Popups->RescanMetadata  = NewStaticStringCompound("Rescans the music folder and recollects all metadata.");
    
    Popups->SavePalette     = NewStaticStringCompound("Saves the changes of the current palette (only for custom palettes).");
    Popups->CopyPalette     = NewStaticStringCompound("Copies the current palette and immidiately switches to it.");
    Popups->DeletePalette   = NewStaticStringCompound("Deletes the current palette (only for custom palettes).");
    Popups->CancelPicker    = NewStaticStringCompound("Quits the color-picker.");
    
    Popups->AddPlaylist     = NewStaticStringCompound("Create a new, empty playlist. Fill it, by Drag&Dropping 'Slots' onto it.");
    Popups->AddSelectionPlaylist = NewStaticStringCompound("Create a playlist with the current selection of songs.");
    Popups->RenamePlaylist  = NewStaticStringCompound("Rename the active playlist ('All' excluded).");
    Popups->RemovePlaylist  = NewStaticStringCompound("Remove the active playlist ('All' excluded).");
    Popups->SearchPlaylists = NewStaticStringCompound("Search playlist by name.");
    
    CreatePopup(&GlobalGameState.Renderer, &GlobalGameState.FixArena, &Popups->Popup, Popups->Help, 
                GetFontSize(&GlobalGameState.Renderer, font_Medium), -0.99f, 0.05f);
    Popups->ActiveText = 19;
}

internal void
ProcessShortcutPopup(shortcut_popups *Popups, r32 dTime, v2 MouseP)
{
    if(Popups->IsActive)
    {
        game_state *GS = &GlobalGameState;
        music_display_info *DisplayInfo = &GS->MusicInfo.DisplayInfo;
        
        b32 PrevIsHovering = Popups->IsHovering;
        Popups->IsHovering = false;
        
        font_size FontSize = GetFontSize(&GS->Renderer, font_Medium);
        if(!DisplayInfo->MusicPath.TextField.IsActive)
        {
            if(IsOnButton(GS->StyleSettings.ColorPicker.Save, MouseP))
            {
                if(Popups->ActiveText != 23) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SavePalette, FontSize);
                Popups->ActiveText = 23;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(GS->StyleSettings.ColorPicker.New, MouseP))
            {
                if(Popups->ActiveText != 24) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->CopyPalette, FontSize);
                Popups->ActiveText = 24;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(GS->StyleSettings.ColorPicker.Remove, MouseP))
            {
                if(Popups->ActiveText != 25) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->DeletePalette, FontSize);
                Popups->ActiveText = 25;
                Popups->IsHovering = true;
            }
            else 
                if(IsOnButton(GS->StyleSettings.Cancel, MouseP))
            {
                if(Popups->ActiveText != 26) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->CancelPicker, FontSize);
                Popups->ActiveText = 26;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->PaletteSwap))
            {
                if(Popups->ActiveText != 1) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->PaletteSwap, FontSize);
                Popups->ActiveText = 1;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->StyleSettings))
            {
                if(Popups->ActiveText != 2) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->ColorPicker, FontSize);
                Popups->ActiveText = 2;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->MusicPath.Button))
            {
                if(Popups->ActiveText != 3) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->MusicPath, FontSize);
                Popups->ActiveText = 3;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Genre.Search.Button))
            {
                if(Popups->ActiveText != 6) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SearchGenre, FontSize);
                Popups->ActiveText = 6;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Artist.Search.Button))
            {
                if(Popups->ActiveText != 7) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SearchArtist, FontSize);
                Popups->ActiveText = 7;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Album.Search.Button))
            {
                if(Popups->ActiveText != 8) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SearchAlbum, FontSize);
                Popups->ActiveText = 8;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Song.Base.Search.Button))
            {
                if(Popups->ActiveText != 9) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SearchSong, FontSize);
                Popups->ActiveText = 9;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->LoopPlaylist, MouseP))
            {
                if(Popups->ActiveText != 10) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Loop, FontSize);
                Popups->ActiveText = 10;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->ShufflePlaylist, MouseP))
            {
                if(Popups->ActiveText != 11) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Shuffle, FontSize);
                Popups->ActiveText = 11;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlayPause, MouseP))
            {
                if(Popups->ActiveText != 12) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->PlayPause, FontSize);
                Popups->ActiveText = 12;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Stop))
            {
                if(Popups->ActiveText != 13) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Stop, FontSize);
                Popups->ActiveText = 13;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Previous))
            {
                if(Popups->ActiveText != 14) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Previous, FontSize);
                Popups->ActiveText = 14;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Next))
            {
                if(Popups->ActiveText != 15) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Next, FontSize);
                Popups->ActiveText = 15;
                Popups->IsHovering = true;
            }
            else if(IsInRect(DisplayInfo->Volume.GrabThing, MouseP) || IsInRect(DisplayInfo->Volume.Background, MouseP))
            {
                if(Popups->ActiveText != 16) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Volume, FontSize);
                Popups->ActiveText = 16;
                Popups->IsHovering = true;
            }
            else if(IsInRect(DisplayInfo->PlayingSongPanel.Timeline.GrabThing, MouseP) ||
                    IsInRect(DisplayInfo->PlayingSongPanel.Timeline.Background, MouseP))
            {
                if(Popups->ActiveText != 17) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Timeline, FontSize);
                Popups->ActiveText = 17;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->Help, MouseP))
            {
                if(Popups->ActiveText != 18) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->Help, FontSize);
                Popups->ActiveText = 18;
                Popups->IsHovering = true;
            }
            else if(IsInRect(DisplayInfo->Song.Base.Background, MouseP))
            {
                For(DisplayInfo->Song.Base.Count)
                {
                    if(IsButtonHovering(DisplayInfo->Song.Play[It]))
                    {
                        if(Popups->ActiveText != 4) 
                            ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SongPlay, FontSize);
                        Popups->ActiveText = 4;
                        Popups->IsHovering = true;
                        break;
                    }
                    else if(IsButtonHovering(DisplayInfo->Song.Add[It]))
                    {
                        if(Popups->ActiveText != 5) 
                            ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SongAddToNextUp, FontSize);
                        Popups->ActiveText = 5;
                        Popups->IsHovering = true;
                        break;
                    }
                }
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.Add, MouseP))
            {
                if(Popups->ActiveText != 27) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->AddPlaylist, FontSize);
                Popups->ActiveText = 27;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.AddSelection, MouseP))
            {
                if(Popups->ActiveText != 28) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->AddSelectionPlaylist, FontSize);
                Popups->ActiveText = 28;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.Rename, MouseP))
            {
                if(Popups->ActiveText != 29) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->RenamePlaylist, FontSize);
                Popups->ActiveText = 29;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.Remove, MouseP))
            {
                if(Popups->ActiveText != 30) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->RemovePlaylist, FontSize);
                Popups->ActiveText = 30;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->Playlists.Search.Button, MouseP))
            {
                if(Popups->ActiveText != 31) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SearchPlaylists, FontSize);
                Popups->ActiveText = 31;
                Popups->IsHovering = true;
            }
        }
        else
        {
            if(IsOnButton(DisplayInfo->MusicPath.Save, MouseP))
            {
                if(Popups->ActiveText != 20) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->SaveMusicPath, FontSize);
                Popups->ActiveText = 20;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->MusicPath.Quit, MouseP))
            {
                if(Popups->ActiveText != 21) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->CancelMusicPath, FontSize);
                Popups->ActiveText = 21;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->MusicPath.Rescan, MouseP))
            {
                if(Popups->ActiveText != 22) 
                    ChangeText(&GS->Renderer, &GS->FixArena, &Popups->Popup, Popups->RescanMetadata, FontSize);
                Popups->ActiveText = 22;
                Popups->IsHovering = true;
            }
        }
        
        if(Popups->IsHovering != PrevIsHovering) SetActive(&Popups->Popup, Popups->IsHovering);
        if(Popups->IsHovering || Popups->Popup.AnimDir != 0)
        {
            SetPosition(&Popups->Popup, MouseP);
        }
        
        rect Screen = Rect(V2(0), V2(GS->Renderer.Window.CurrentDim.Dim));
        if(!IsInRect(Screen, Popups->Popup.BG))
        {
            rect PopupRect = ExtractScreenRect(Popups->Popup.BG);
            v2 MaxDiff = PopupRect.Max - Screen.Max;
            MaxDiff.x = Max(MaxDiff.x, 0.0f);
            MaxDiff.y = Max(MaxDiff.y, 0.0f);
            
            v2 MinDiff = Screen.Min - PopupRect.Min;
            MinDiff.x = Max(MinDiff.x, 0.0f);
            MinDiff.y = Max(MinDiff.y, 0.0f);
            
            v2 Diff = -MaxDiff+MinDiff;
            
            Translate(Popups->Popup.Anchor, Diff);
        }
    }
    DoAnimation(&Popups->Popup, dTime);
}

internal void
SetNewPlayingSong(renderer *Renderer, playing_song_panel *Panel, layout_definition *Layout, music_info *MusicInfo)
{
    game_state *GS = &GlobalGameState;
    
    RemoveRenderText(Renderer, &Panel->DurationText);
    RemoveRenderText(Renderer, &Panel->CurrentTimeText);
    RemoveRenderText(Renderer, &Panel->Title);
    RemoveRenderText(Renderer, &Panel->Artist);
    RemoveRenderText(Renderer, &Panel->Genre);
    RemoveRenderText(Renderer, &Panel->Album);
    RemoveRenderText(Renderer, &Panel->Year);
    RemoveRenderText(Renderer, &Panel->Track);
    
    string_c Empty  = {};
    string_c NoTime = NewStaticStringCompound("00:00");
    
    string_c DurationString = NewStringCompound(&GS->ScratchArena, 14);
    string_c *TitleString  = 0;
    string_c *TrackString  = 0;
    string_c *ArtistString = 0;
    string_c *AlbumString  = 0;
    string_c *YearString   = 0;
    string_c *GenreString  = 0;
    
    if(MusicInfo->PlayingSong.DecodeID >= 0 && 
       (MusicInfo->PlayingSong.DisplayableID >= 0 || MusicInfo->PlayingSong.PlayUpNext))
    {
        playlist_id PlaylistID = GetNextSong(MusicInfo);
        mp3_metadata *Metadata = GetMetadata(&MusicInfo->Playlist_->Song, &Panel->MP3Info->FileInfo, PlaylistID);
        Panel->SongDuration    = (r32)Metadata->Duration;
        
        MillisecondsToMinutes(Metadata->Duration, &DurationString);
        TrackString = &Metadata->TrackString;
        if(Metadata->Title.Pos == 0)  
            TitleString = GetSongFileName(&MusicInfo->Playlist_->Song, &Panel->MP3Info->FileInfo, PlaylistID);
        else                          TitleString  = &Metadata->Title;
        if(Metadata->Artist.Pos == 0) ArtistString = &Empty;
        else                          ArtistString = &Metadata->Artist;
        if(Metadata->Album.Pos == 0)  AlbumString  = &Empty;
        else                          AlbumString  = &Metadata->Album;
        if(Metadata->YearString.Pos == 0 || Metadata->YearString.Pos > 4) // Larger 4 means garbage data.
            YearString = &Empty;
        else                              YearString = &Metadata->YearString;
        if(Metadata->Genre.Pos == 0)  GenreString  = &Empty;
        else                          GenreString  = &Metadata->Genre;
    }
    else
    {
        TitleString = TrackString = ArtistString = AlbumString = YearString = GenreString = &Empty;
        CopyIntoCompound(&DurationString, &NoTime);
        Panel->SongDuration = 0;
    }
    
    font_metrics SmallMetrics  = GetFontMetrics(GS, font_Small, *TitleString);
    font_metrics MediumMetrics = GetFontMetrics(GS, font_Medium, *TitleString);
    
    v3 *TextColor = &Panel->MP3Info->MusicInfo->DisplayInfo.ColorPalette.ForegroundText;
    Panel->CurrentTime = 0;
    
    r32 Depth = Layout->PanelTextDepth;
    r32 BaseTimeX = Layout->PlayPauseButtonX + Layout->PlayPauseButtonExtents + 
        Layout->LargeButtonExtents*2*3 + Layout->ButtonGap*4;
    r32 BaseX = BaseTimeX + Layout->PanelXOffset + Layout->PanelBaseX;
    
    r32 TimeY = Layout->PanelBaseY + Layout->PlayPauseButtonY + Layout->DurationTimeTextYOffset - SmallMetrics.Descent;
    string_c CurrentTime = NewStaticStringCompound("00:00 |");
    RenderText(GS, font_Small, &CurrentTime, TextColor, &Panel->CurrentTimeText, Depth+0.01f, 0);
    SetPosition(&Panel->CurrentTimeText, V2(BaseTimeX+Layout->CurrentTimeTextXOffset, TimeY));
    
    r32 TimeX = GetPosition(Panel->CurrentTimeText.Base).x + Panel->CurrentTimeText.Extends.x + 
        Layout->DurationTimeTextXOffset;
    
    RenderText(GS, font_Small, &DurationString, TextColor, &Panel->DurationText, Depth+0.01f, 0);
    SetPosition(&Panel->DurationText, V2(TimeX, TimeY));
    
    r32 RowY = Layout->PanelBaseY - SmallMetrics.Descent;
    
    RenderText(GS, font_Small, GenreString, TextColor, &Panel->Genre, Depth, 0);
    SetPosition(&Panel->Genre, V2(BaseX, RowY));
    
    RowY += SmallMetrics.RowGap + Layout->PanelTextYOffset;
    
    // #OffsetNullify To offset Layout->PanelTextAlbumXOffset on AlbumX if Panel->Year does not get set.
    Panel->Year.Extends.x = Layout->PanelTextXOffset*-1;
    RenderText(GS, font_Small, YearString, TextColor, &Panel->Year, Depth, 0);
    SetPosition(&Panel->Year, V2(BaseX, RowY));
    
    r32 AlbumX = GetPosition(Panel->Year.Base).x + Panel->Year.Extends.x + Layout->PanelTextXOffset;
    
    RenderText(GS, font_Small, AlbumString, TextColor, &Panel->Album, Depth, 0);
    SetPosition(&Panel->Album, V2(AlbumX, RowY));
    
    RowY += SmallMetrics.RowGap + Layout->PanelTextYOffset;
    
    RenderText(GS, font_Small, ArtistString, TextColor, &Panel->Artist, Depth, 0);
    SetPosition(&Panel->Artist, V2(BaseX, RowY));
    
    r32 RowSongY = RowY + SmallMetrics.Ascent + MediumMetrics.Descent*-1;
    RowY += SmallMetrics.Ascent + MediumMetrics.Descent*-1;
    
    // See #OffsetNullify.
    Panel->Track.Extends.x = Layout->PanelTextXOffset*-1;
    RenderText(GS, font_Small, TrackString, TextColor, &Panel->Track, Depth, 0);
    SetPosition(&Panel->Track, V2(BaseX, RowY));
    
    r32 TitleX = GetPosition(Panel->Track.Base).x + Panel->Track.Extends.x + Layout->PanelTextXOffset;
    
    RenderText(GS, font_Medium, TitleString, TextColor, &Panel->Title, Depth-0.01f, 0);
    SetPosition(&Panel->Title, V2(TitleX, RowSongY));
}

internal void
UpdatePanelTime(game_state *GS, playing_song_panel *Panel, layout_definition *Layout, r32 CurrentTime)
{
    Panel->CurrentTime = (u32)(CurrentTime*1000);
    WipeStringCompound(&Panel->CurrentTimeString);
    MillisecondsToMinutes(Panel->CurrentTime, &Panel->CurrentTimeString);
    AppendStringToCompound(&Panel->CurrentTimeString, (u8 *)" |");
    
    RemoveRenderText(&GS->Renderer, &Panel->CurrentTimeText);
    RenderText(GS, font_Small, &Panel->CurrentTimeString, &Panel->MP3Info->MusicInfo->DisplayInfo.ColorPalette.ForegroundText, &Panel->CurrentTimeText, Layout->PanelTextDepth+0.01f, 0);
    
    r32 BaseTimeX = Layout->PlayPauseButtonX + Layout->PlayPauseButtonExtents + 
        Layout->LargeButtonExtents*2*3 + Layout->ButtonGap*4;
    font_metrics SmallMetrics  = GetFontMetrics(GS, font_Small, Panel->CurrentTimeString);
    r32 TimeY = Layout->PanelBaseY + Layout->PlayPauseButtonY + Layout->DurationTimeTextYOffset - SmallMetrics.Descent;
    SetPosition(&Panel->CurrentTimeText, V2(BaseTimeX+Layout->CurrentTimeTextXOffset, TimeY));
}

internal void
UpdatePanelTimeline(playing_song_panel *Panel, r32 CurrentTime)
{
    slider *Slider = &Panel->Timeline;
    r32 TimePercentage = Min(1.0f, SafeDiv((CurrentTime*1000), Panel->SongDuration));
    r32 NewXPos = (Slider->MaxSlidePix*2)*TimePercentage;
    Slider->GrabThing->ID->Transform.Translation.x = GetPosition(Slider->Background).x-Slider->MaxSlidePix+NewXPos;
}

internal column_type
CheckColumnsForSelectionChange(v2 MouseBtnDownLocation)
{
    column_type Result = columnType_None;
    
    renderer *Renderer = &GlobalGameState.Renderer;
    input_info *Input = &GlobalGameState.Input;
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    playlist_info      *Playlist    = GlobalGameState.MusicInfo.Playlist_;
    
    if(IsInRect(DisplayInfo->Song.Base.Background, Input->MouseP))
    {
        if(GlobalGameState.Time.GameTime - DisplayInfo->Song.Base.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Song.Base, &Playlist->Song);
        else 
            Result = UpdateSelectionArray(&Playlist->Song, &DisplayInfo->Song.Base, MouseBtnDownLocation);
        
        if(Result != columnType_None) UpdateColumnColor(&DisplayInfo->Song.Base, &Playlist->Song);
        DisplayInfo->Song.Base.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Genre.Background, Input->MouseP))
    {
        
        if(GlobalGameState.Time.GameTime - DisplayInfo->Genre.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Genre, &Playlist->Genre);
        else Result = UpdateSelectionArray(&Playlist->Genre, &DisplayInfo->Genre, MouseBtnDownLocation);
        
        if(Result != columnType_None)
        {
            PropagateSelectionChange(&GlobalGameState.MusicInfo);
            DisplayInfo->Artist.DisplayCursor = 0;
            DisplayInfo->Album.DisplayCursor  = 0;
        }
        
        DisplayInfo->Genre.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Artist.Background, Input->MouseP))
    {
        if(GlobalGameState.Time.GameTime - DisplayInfo->Artist.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Artist, &Playlist->Artist);
        else Result = UpdateSelectionArray(&Playlist->Artist, &DisplayInfo->Artist, MouseBtnDownLocation);
        
        if(Result != columnType_None)
        {
            PropagateSelectionChange(&GlobalGameState.MusicInfo);
            DisplayInfo->Album.DisplayCursor  = 0;
        }
        
        DisplayInfo->Artist.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Album.Background, Input->MouseP))
    {
        if(GlobalGameState.Time.GameTime - DisplayInfo->Album.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Album, &Playlist->Album);
        else Result = UpdateSelectionArray(&Playlist->Album, &DisplayInfo->Album, MouseBtnDownLocation);
        
        DisplayInfo->Album.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Playlists.Background, Input->MouseP))
    {
        Result = UpdateSelectionArray(&Playlist->Playlists, &DisplayInfo->Playlists, MouseBtnDownLocation);
    }
    
    return Result;
}

inline string_c 
GetRandomExitMessage(game_state *GS, string_c *Language)
{
    string_c Result = {};
    local_persist const string_compound ExitMessagesLanguage[] = 
    {
        NewStaticStringCompound("Afrikaans"), //  0. 
        NewStaticStringCompound("Arabic"),    //  1.
        NewStaticStringCompound("Bengali"),   //  2.
        NewStaticStringCompound("Bosnian"),   //  3.
        NewStaticStringCompound("Cantonese"), //  4.
        NewStaticStringCompound("Cherokee"),  //  5.
        NewStaticStringCompound("Croatian"),  //  6.
        NewStaticStringCompound("Czech"),     //  7.
        NewStaticStringCompound("Danish"),    //  8.
        NewStaticStringCompound("Dutch"),     //  9.
        NewStaticStringCompound("English"),   // 10.
        NewStaticStringCompound("Estonian"),  // 11.
        NewStaticStringCompound("Finnish"),   // 12.
        NewStaticStringCompound("French"),    // 13.
        NewStaticStringCompound("German"),    // 14.
        NewStaticStringCompound("Greek"),     // 15.
        NewStaticStringCompound("Hawaiian"),  // 16.
        NewStaticStringCompound("Hebrew"),    // 17.
        NewStaticStringCompound("Hindi"),     // 18.
        NewStaticStringCompound("Hungarian"), // 19.
        NewStaticStringCompound("Icelandic"), // 20.
        NewStaticStringCompound("Indonesian"),// 21.
        NewStaticStringCompound("Irish"),     // 22.
        NewStaticStringCompound("Italian"),   // 23.
        NewStaticStringCompound("Japanese"),  // 24.
        NewStaticStringCompound("Korean"),    // 25.
        NewStaticStringCompound("Latin"),     // 26.
        NewStaticStringCompound("Latvian"),   // 27.
        NewStaticStringCompound("Lithuanian"),// 28.
        NewStaticStringCompound("Mandarin"),  // 29.
        NewStaticStringCompound("Nepalese"),  // 30.
        NewStaticStringCompound("Norwegian"), // 31.
        NewStaticStringCompound("Persian"),   // 32.
        NewStaticStringCompound("Polish"),    // 33.
        NewStaticStringCompound("Portuguese"),// 34.
        NewStaticStringCompound("Punjabi"),   // 35.
        NewStaticStringCompound("Romanian"),  // 36.
        NewStaticStringCompound("Russian"),   // 37.
        NewStaticStringCompound("Serbian"),   // 38.
        NewStaticStringCompound("Slovak"),    // 39.
        NewStaticStringCompound("Slovene"),   // 40.
        NewStaticStringCompound("Spanish"),   // 41.
        NewStaticStringCompound("Swedish"),   // 42.
        NewStaticStringCompound("Tamil"),     // 43.
        NewStaticStringCompound("Thai"),      // 44.
        NewStaticStringCompound("Turkish"),   // 45.
        NewStaticStringCompound("Ukrainian"), // 46.
        NewStaticStringCompound("Urdu"),      // 47.
        NewStaticStringCompound("Vietnamese"),// 48.
        NewStaticStringCompound("Welsh"),     // 49.
        NewStaticStringCompound("Zulu"),      // 50.
    };
    
    local_persist const string_compound ExitMessages[] = 
    {
        NewStaticStringCompound("Totsiens!"), // 0. Afrikaans 
        NewStaticStringCompound("       مع السلامة \n(Ma'a as-salaama!)"), // 1. Arabic
        NewStaticStringCompound("   বিদায় \n(Bidāẏa!)"), // 2. Bengali
        NewStaticStringCompound("Zdravo!"), // 3. Bosnian
        NewStaticStringCompound("Joigin!"), // 4. Cantonese
        NewStaticStringCompound("Donadagohvi!"), // 5. Cherokee
        NewStaticStringCompound("Doviđenja!"), // 6. Croatian
        NewStaticStringCompound("Sbohem!"), // 7. Czech
        NewStaticStringCompound("Farvel!"), // 8. Danish
        NewStaticStringCompound("Tot ziens!"), // 9. Dutch
        NewStaticStringCompound("Goodbye!"), // 10. English
        NewStaticStringCompound("Nägemist!"), // 11. Estonian
        NewStaticStringCompound("Näkemiin!"), // 12. Finnish
        NewStaticStringCompound("Au Revoir!"), // 13. French
        NewStaticStringCompound("Auf Wiedersehen!"), // 14. German
        NewStaticStringCompound("Αντίο!"), // 15. Greek
        NewStaticStringCompound("Aloha!"), // 16. Hawaiian
        NewStaticStringCompound(" הֱיה שלום\n(L'hitraot!)"), // 17. Hebrew
        NewStaticStringCompound("   अलविदा\n(alavida!)"), // 18. Hindi
        NewStaticStringCompound("Viszontlátásra!"), // 19. Hungarian
        NewStaticStringCompound("Vertu sæll!"), // 20. Icelandic
        NewStaticStringCompound("Selamat tinggal!"), // 21. Indonesian
        NewStaticStringCompound("Slán!"), // 22. Irish
        NewStaticStringCompound("Arrivederci!"), // 23. Italian
        NewStaticStringCompound(" さようなら \n(Sayōnara!)"), // 24. Japanese
        NewStaticStringCompound("      안녕히 가세요\n(Annyeonghi Gaseyo!)"), // 25. Korean
        NewStaticStringCompound("Vale!"), // 26. Latin
        NewStaticStringCompound("Uz redzēšanos!"), // 27. Latvian
        NewStaticStringCompound("Sudie!"), // 28. Lithuanian
        NewStaticStringCompound("   再见 \n(Zài jiàn)"), // 29. Mandarin
        NewStaticStringCompound("   अलविदा  \n(Alavidā!)"), // 30. Nepalese
        NewStaticStringCompound("Ha det bra!"), // 31. Norwegian
        NewStaticStringCompound("     خداحافظی\n(Khodaa haafez!)"), // 32. Persian
        NewStaticStringCompound("Żegnaj!"), // 33. Polish
        NewStaticStringCompound("Adeus!"), // 34. Portuguese
        NewStaticStringCompound("  ਅਲਵਿਦਾ \n(Alweda!)"), // 35. Punjabi
        NewStaticStringCompound("La revedere!"), // 36. Romanian
        NewStaticStringCompound("До свидания \n(Do svidaniya!)"), // 37. Russian
        NewStaticStringCompound("збогом!\n(zbogom!)"), // 38. Serbian
        NewStaticStringCompound("Dovidenia!"), // 39. Slovak
        NewStaticStringCompound("Nasvidenje!"), // 40. Slovene
        NewStaticStringCompound("Adios!"), // 41. Spanish
        NewStaticStringCompound("Adjö!"), // 42. Swedish
        NewStaticStringCompound("பிரியாவிடை \n(Poitu varein!)"), // 43. Tamil
        NewStaticStringCompound("   ลาก่อน  \n(Laa Gòn!)"), // 44. Thai
        NewStaticStringCompound("Görüşürüz!"), // 45. Turkish
        NewStaticStringCompound(" До побачення \n(Do pobachennia!)"), // 46. Ukrainian
        NewStaticStringCompound("    خدا حافظ \n(Khuda hafiz!)"), // 47. Urdu
        NewStaticStringCompound("Tạm biệt!"), // 48. Vietnamese
        NewStaticStringCompound("Hwyl fawr!"), // 49. Welsh
        NewStaticStringCompound("Hamba kahle!"), // 50. Zulu
    };
    
    u32 RandValue = Floor(Random01()*ArrayCount(ExitMessages));
    Assert(RandValue < ArrayCount(ExitMessages));
    
    Result = NewStringCompound(&GS->ScratchArena, ExitMessages[RandValue].Pos);
    AppendStringCompoundToCompound(&Result, (string_c *)ExitMessages+RandValue);
    
    if(Language)
    {
        AppendStringCompoundToCompound(Language, (string_c *)ExitMessagesLanguage+RandValue);
    }
    
    return Result;
}

internal void
HandlePlaylistsColumnButtonAnimation(game_state *GS, button *Btn, quit_animation *Anim, playlist_btn_type BtnType)
{
    if(!Anim->Activated)
    {
        display_column *DColumn = &GS->MusicInfo.DisplayInfo.Playlists;
        playlist_ui *PlaylistUI = &GS->MusicInfo.DisplayInfo.PlaylistUI;
        
        if(Btn->State == buttonState_Pressed)
        {
            v2 Position = GetPosition(DColumn->Background)+V2(0, GetSize(DColumn->Background).y*0.5f);
            v2 Size     = GetSize(DColumn->Background);
            if(QuitAnimation(Anim, 1, Position, Size))
            {
                if     (BtnType == playlistBtnType_Add)          OnNewPlaylistClick(GS);
                else if(BtnType == playlistBtnType_AddSelection) OnNewPlaylistWithSelectionClick(GS);
                else if(BtnType == playlistBtnType_Remove)       OnRemovePlaylistClick(GS);
                
                SetActive(Anim, false);
            }
        }
        else if(Anim->dAnim != 0)
        {
            v2 Position = GetPosition(DColumn->Background)+V2(0, GetSize(DColumn->Background).y*0.5f);
            v2 Size     = GetSize(DColumn->Background);
            if(QuitAnimation(Anim, -1, Position, Size))
            {
                SetActive(Anim, false);
            }
        }
    }
    if(Btn->State == buttonState_Unpressed) 
    {
        Anim->Activated = false;
    }
}
