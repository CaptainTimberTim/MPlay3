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
internal void UpdatePlayingSongForSelectionChange(music_info *MusicInfo);
internal void InsertSlotIntoPlaylist(game_state *GS, playlist_info *Playlist, column_type Type, displayable_id DisplayableID);
internal void InsertSlotsIntoPlaylist(game_state *GS, playlist_info *IntoPlaylist, column_type Type, array_u32 DisplayableIDs);
internal void RemoveSlotFromPlaylist(game_state *GS, column_type Type, displayable_id DisplayableID);
internal void OnNewPlaylistClick(void *Data);
internal void OnNewPlaylistWithSelectionClick(void *Data);
internal void OnRemovePlaylistClick(void *Data);
internal void OnRenamePlaylistClick(void *Data);
inline r32 GetXTextPosition(entry_id *Background, r32 XOffset);

inline file_id        GetFileID(music_info *MusicInfo, displayable_id DisplayableID);
inline playlist_id    GetPlaylistID(music_info *MusicInfo, displayable_id DisplayableID);
inline playlist_id    GetPlaylistID(playlist_column *SongColumn, file_id FileID);
inline displayable_id GetDisplayableID(music_info *MusicInfo, u32 OnScreenID, playlist_id *PlaylistID = 0);
inline displayable_id GetDisplayableID(array_playlist_id *Displayable, playlist_id PlaylistID);
inline   i32          GetOnScreenID(display_column *DisplayColumn, displayable_id DisplayableID);
internal i32          GetOnScreenID(game_state *GS, column_type Type, displayable_id DisplayableID);
internal i32          GetOnScreenID(game_state *GS, playlist_info *Playlist);
inline string_c *     GetName(playlist_column *PlaylistColumn, displayable_id DID);
inline void OnAnimationDone(void *Data);

inline u32 
CountPossibleDisplayedSlots(renderer *Renderer, display_column *DisplayColumn)
{
    u32 Result = 0;
    r32 Height = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    Result = Ceiling(Height/DisplayColumn->SlotHeight)+1;
    return Result;
}

inline void
CreateDisplayBackgroundRect(renderer *Renderer, display_column *DisplayColumn, u32 ID, r32 ZValue, entry_id *Parent)
{
    r32 SlotGap = GlobalGameState.Layout.SlotGap;
    v2 HalfDim = V2(DisplayColumn->SlotWidth, DisplayColumn->SlotHeight-SlotGap)/2;
    DisplayColumn->BGRects[ID] = CreateRenderRect(Renderer, {-HalfDim, HalfDim}, ZValue, Parent, DisplayColumn->Colors.Slot);
    r32 YDown = (ID > 0) ? -DisplayColumn->SlotHeight : 0;
    SetLocalPosition(DisplayColumn->BGRects[ID], V2(0, YDown));
}

inline r32
GetInitialYForDisplayColumnSlot(display_column *DisplayColumn)
{
    r32 Result = 0;
    Result  = GetLocalPosition(DisplayColumn->TopBorder).y;
    Result -= GetSize(DisplayColumn->TopBorder).y/2;
    Result -= DisplayColumn->SlotHeight/2;
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
    
    UpdatePlayingSongForSelectionChange(MusicInfo);
    
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
    
    playlist_id PlaylistID = Get(&MusicInfo->Playlist_->Song.Displayable, SongColumn->OnScreenIDs[PlayInfo->DisplayID]);
    Push(&MusicInfo->UpNextList, PlaylistID);
    if(MusicInfo->UpNextList.A.Count < MAX_MP3_DECODE_COUNT-1)
    {
        AddJob_LoadMP3(&PlayInfo->GameState->JobQueue, PlaylistID);
    }
}

inline void
CreateSongButtons(renderer *Renderer, display_column_song_extension *SongColumn, u32 ID)
{
    u32 ButtonID = SongColumn->Base.Base->PlayPause->Entry->ID->TexID;
    
    color_palette *Palette = &SongColumn->Base.Base->ColorPalette;
    
    r32 Z = SongColumn->Base.ZValue - 0.01f;
    
    r32 HalfSize = GlobalGameState.Layout.AddButtonExtents;
    rect Rect = {{-HalfSize, -HalfSize}, {HalfSize, HalfSize}};
    SongColumn->Play[ID] = NewButton(Renderer, Rect, Z, false, ButtonID, SongColumn->PlayPauseGLID, Renderer->ButtonColors, 
                                     SongColumn->Base.BGRects[ID]);
    Translate(SongColumn->Play[ID], V2(0, GlobalGameState.Layout.SongPlayButtonYOffset));
    SongColumn->PlayBtnData[ID] = {ID, &GlobalGameState};
    SongColumn->Play[ID]->OnPressed = {OnSongPlayPressed, &SongColumn->PlayBtnData[ID]};
    
    SongColumn->Add[ID] = NewButton(Renderer, Rect, Z, false, ButtonID, SongColumn->AddGLID, 
                                    Renderer->ButtonColors, SongColumn->Play[ID]->Entry);
    Translate(SongColumn->Add[ID], V2(HalfSize*2+GlobalGameState.Layout.TopLeftButtonGroupGap, 0));
    
    SongColumn->Add[ID]->OnPressed = {OnSongAddPressed, &SongColumn->PlayBtnData[ID]};
}

inline void
SetSongButtonsActive(display_column_song_extension *SongColumn, u32 ID, b32 IsActive)
{
    if(SongColumn->Play[ID]) SetActive(SongColumn->Play[ID], IsActive);
    if(SongColumn->Add[ID]) SetActive(SongColumn->Add[ID], IsActive);
}

inline b32
IsSearchOpen(music_display_info *DisplayInfo)
{
    b32 Result = false;
    
    Result = DisplayInfo->Genre.Search.TextField.IsActive || DisplayInfo->Artist.Search.TextField.IsActive || 
        DisplayInfo->Album.Search.TextField.IsActive || DisplayInfo->Song.Base.Search.TextField.IsActive;
    
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
        UpdatePlayingSongForSelectionChange(&GlobalGameState.MusicInfo);
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
    renderer *Renderer                   = ColumnInfo.Renderer;
    display_column *DisplayColumn  = ColumnInfo.DisplayColumn;
    playlist_column      *PlaylistColumn = ColumnInfo.PlaylistColumn;
    text_field_flag_result FieldResult   = ProcessTextField(Renderer, dTime, Input, &DisplayColumn->Search.TextField);
    
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
            UpdateColumnColor(DisplayColumn, PlaylistColumn);
        }
        ResetSearchList(Renderer, DisplayColumn);
        UpdateSortingInfoChanged(Renderer, &GlobalGameState.MusicInfo, GlobalGameState.MP3Info, DisplayColumn->Type);
        
        playlist_id SelectedID = Get(&PlaylistColumn->Selected, NewSelectID(0));
        if(DisplayColumn->Type == columnType_Song) BringDisplayableEntryOnScreen(ColumnInfo.MusicInfo, DisplayColumn, SelectedID);
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
    
    if(Type == columnType_Song) 
    {
        DisplayColumn->SlotHeight = Layout->SongSlotHeight;
        DisplayColumn->TextX      = Layout->BigTextLeftBorder;
    }
    else
    {
        DisplayColumn->SlotHeight  = Layout->SlotHeight;
        DisplayColumn->TextX       = Layout->SmallTextLeftBorder;
    }
    
    // Creating horizontal slider 
    r32 SliderHoriHeight = Layout->HorizontalSliderHeight;
    r32 SliderVertWidth = Layout->VerticalSliderWidth;
    r32 InsetHori = Layout->HorizontalSliderGrabThingBorder;
    DisplayColumn->SlotWidth = (GetRect(RightBorder).Min.x - SliderVertWidth) - GetRect(LeftBorder).Max.x;
    
    v2 BottomLeft = V2(GetRect(LeftBorder).Max.x, GetRect(DisplayInfo->EdgeBottom).Max.y);
    DisplayColumn->SliderHorizontal.Background = 
        CreateRenderRect(Renderer, {BottomLeft, BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight)},
                         DisplayColumn->ZValue-0.03f, 0, ColumnColors.SliderBG);
    
    DisplayColumn->SliderHorizontal.GrabThing  = 
        CreateRenderRect(Renderer, { BottomLeft+V2(0, InsetHori), 
                             BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight-InsetHori)
                         }, DisplayColumn->ZValue-0.031f, 0, ColumnColors.SliderGrabThing);
    
    
    DisplayColumn->BottomBorder = DisplayColumn->SliderHorizontal.Background;
    r32 ColumnHeight = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
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
    
    DisplayColumn->BGRectAnchor = CreateRenderRect(Renderer, V2(0), 0, ColumnColors.Background, 0);
    r32 AnchorY = GetPosition(DisplayColumn->TopBorder).y-GetSize(DisplayColumn->TopBorder).y/2.0f-
        DisplayColumn->SlotHeight/2.0f;
    SetPosition(DisplayColumn->BGRectAnchor, V2(0, AnchorY));
    TranslateWithScreen(&Renderer->TransformList, DisplayColumn->BGRectAnchor, fixedTo_Top);
    
    entry_id *Parent = DisplayColumn->BGRectAnchor;
    For(DisplayColumn->Count)
    {
        CreateDisplayBackgroundRect(Renderer, DisplayColumn, It, ZValue, Parent);
        Parent = DisplayColumn->BGRects[It];
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
            r32 Overhang = DistanceToRectEdge(DisplayColumn->BGRects[It], TextP);
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
    display_column_song_extension *Song = ColumnExt(DisplayColumn);
    
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

internal void
ResetColumnText(display_column *DisplayColumn, u32 DisplayableCount)
{
    layout_definition *Layout = &GlobalGameState.Layout;
    for(u32 It = 0;
        It < DisplayColumn->Count &&
        It < DisplayableCount; 
        ++It)
    {
        
        //r32 TextX = GetPosition(DisplayColumn->LeftBorder).x + GetSize(DisplayColumn->LeftBorder).x/2 + DisplayColumn->TextX;
        r32 TextX = GetXTextPosition(DisplayColumn->BGRects[It], DisplayColumn->TextX);
        if(DisplayColumn->Type == columnType_Song)
        {
            SetPositionX(ColumnExt(DisplayColumn)->SongTitle+It,  TextX+Layout->SongXOffset);
            SetPositionX(ColumnExt(DisplayColumn)->SongArtist+It, TextX+Layout->SongXOffset);
            SetPositionX(ColumnExt(DisplayColumn)->SongAlbum+It,  TextX+Layout->SongAlbumXOffset);
            SetPositionX(ColumnExt(DisplayColumn)->SongTrack+It,  TextX+Layout->SongTrackXOffset);
            SetPositionX(ColumnExt(DisplayColumn)->SongYear+It,   TextX+Layout->SongXOffset);
            
            SetActive(ColumnExt(DisplayColumn)->SongTitle+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongArtist+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongAlbum+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongGenre+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongTrack+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongYear+It, true);
            
            r32 NewBtnX = GetPosition(DisplayColumn->LeftBorder).x;
            SetPositionX(ColumnExt(DisplayColumn)->Play[It]->Entry, NewBtnX+Layout->SongPlayButtonXOffset);
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, true);
        }
        else
        {
            SetPositionX(DisplayColumn->Text+It, TextX);
            SetActive(DisplayColumn->Text+It, true);
        }
    }
}

internal void
UpdateColumnHorizontalSlider(display_column *DisplayColumn, u32 DisplayableCount)
{
    slider *Slider = &DisplayColumn->SliderHorizontal;
    
    ResetColumnText(DisplayColumn, DisplayableCount);
    
    if(DisplayColumn->Type == columnType_Song) 
    {
        Slider->OverhangP = 1 + CalcSongTextOverhangPercentage(DisplayColumn, DisplayableCount);
    }
    else Slider->OverhangP = 1 + CalcTextOverhangPercentage(DisplayColumn, DisplayColumn->Text, DisplayableCount);
    
    r32 Scale      = GetSize(Slider->Background).x;
    r32 NewScale   = Max(Scale/Slider->OverhangP, 5.0f);
    r32 PixelScale = Scale-NewScale;
    
    SetSize(Slider->GrabThing, V2(NewScale, GetSize(Slider->GrabThing).y));
    Slider->GrabThing->ID->Transform.Translation.x = Slider->Background->ID->Transform.Translation.x - PixelScale/2;
    Slider->MaxSlidePix = PixelScale/2;
}

inline void
UpdateHorizontalSliders(music_info *MusicInfo)
{
    UpdateColumnHorizontalSlider(&MusicInfo->DisplayInfo.Playlists, MusicInfo->Playlist_->Playlists.Displayable.A.Count);
    UpdateColumnHorizontalSlider(&MusicInfo->DisplayInfo.Genre,     MusicInfo->Playlist_->Genre.Displayable.A.Count);
    UpdateColumnHorizontalSlider(&MusicInfo->DisplayInfo.Artist,    MusicInfo->Playlist_->Artist.Displayable.A.Count);
    UpdateColumnHorizontalSlider(&MusicInfo->DisplayInfo.Album,     MusicInfo->Playlist_->Album.Displayable.A.Count);
    UpdateColumnHorizontalSlider(&MusicInfo->DisplayInfo.Song.Base, MusicInfo->Playlist_->Song.Displayable.A.Count);
}

inline void
UpdateOnHorizontalSliderDrag(renderer *Renderer, display_column *DisplayColumn, r32 TranslationX, u32 It, 
                             render_text *Text)
{
    Translate(&Text[It], {TranslationX, 0});
    if(Text[It].Count > 0) 
    {
        For(Text[It].Count, Letter)
        {
            v2 TextT = GetPosition(&Text[It], LetterIt);
            v2 TextP = TextT-V2(3,0);
            r32 Overhang = DistanceToRectEdge(DisplayColumn->BGRects[It], TextP);
            
            render_entry *Letter = Text[It].RenderEntries+LetterIt;
            if(Overhang > 0)         Letter->Render = false;
            else if(!Letter->Render) Letter->Render = true;
            else                     break;
        }
    }
}

internal void
SongHorizontalSliderDrag(renderer *Renderer, display_column *DisplayColumn, r32 TranslationX, u32 It)
{
    display_column_song_extension *Song = ColumnExt(DisplayColumn);
    
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongTitle);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongArtist);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongAlbum);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongGenre);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongTrack);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongYear);
    
    // Update song buttons
    Translate(Song->Play[It], V2(TranslationX, 0));
    
    r32 Overhang = WidthBetweenRects(DisplayColumn->LeftBorder, Song->Play[It]->Entry);
    b32 RenderButton = Overhang == 0 || (GetPosition(Song->Play[It]->Entry).x < GetPosition(DisplayColumn->LeftBorder).x);
    SetActive(Song->Play[It], !RenderButton);
    
    Overhang = WidthBetweenRects(DisplayColumn->LeftBorder, Song->Add[It]->Entry);
    RenderButton = Overhang == 0 || (GetPosition(Song->Add[It]->Entry).x < GetPosition(DisplayColumn->LeftBorder).x);
    SetActive(Song->Add[It], !RenderButton);
}

internal void
OnHorizontalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    drag_slider_data *Info = (drag_slider_data *)Data;
    display_column *DisplayColumn = Info->DisplayColumn;
    slider *Slider = &DisplayColumn->SliderHorizontal;
    u32 DisplayableCount = Info->MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable.A.Count;
    
    r32 GrabThingHalfWidth  = GetSize(Slider->GrabThing).x/2.0f;
    if(Slider->MouseOffset.x < GrabThingHalfWidth && Slider->MouseOffset.x > -GrabThingHalfWidth) 
        AdjustedMouseP.x -= Slider->MouseOffset.x;
    
    r32 BGXPos = GetLocalPosition(Slider->Background).x;
    r32 NewX   = Clamp(AdjustedMouseP.x, BGXPos - Slider->MaxSlidePix, BGXPos + Slider->MaxSlidePix);
    r32 OldX   = GetLocalPosition(Slider->GrabThing).x;
    SetLocalPosition(Slider->GrabThing, V2(NewX, GetLocalPosition(Slider->GrabThing).y));
    
    r32 TranslationPercent = SafeDiv(1.0f, Slider->MaxSlidePix*2)*(OldX-NewX);
    r32 TranslationPix = TranslationPercent*DisplayColumn->SlotWidth*(Slider->OverhangP-1);
    // TODO:: The first visible letter needs to be squashed according to its overhang!
    for(u32 It = 0;
        It < DisplayColumn->Count && 
        It < DisplayableCount;
        It++)
    {
        if(DisplayColumn->Type == columnType_Song) SongHorizontalSliderDrag(Renderer, DisplayColumn, TranslationPix, It);
        else UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationPix, It, DisplayColumn->Text);
    }
}


inline r32
GetDisplayableHeight(u32 DisplayableCount, r32 SlotHeight)
{
    return DisplayableCount*SlotHeight;
}

inline void
UpdateColumnVerticalSliderPosition(display_column *DisplayColumn, u32 DisplayableCount)
{
    slider *Slider       = &DisplayColumn->SliderVertical;
    r32 TotalSliderScale = GetScale(Slider->Background).y;
    r32 TotalHeight      = GetDisplayableHeight(DisplayableCount, DisplayColumn->SlotHeight);
    TotalHeight          = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    r32 CursorHeightPercentage = SafeDiv(DisplayColumn->DisplayCursor,(r32)TotalHeight);
    v2 P = V2(GetLocalPosition(Slider->GrabThing).x, GetLocalPosition(Slider->Background).y+Slider->MaxSlidePix);
    P.y -= CursorHeightPercentage*(GetSize(Slider->Background).y-GetSize(Slider->GrabThing).y);
    SetLocalPosition(Slider->GrabThing, P);
    
    UpdateColumnHorizontalSlider(DisplayColumn, DisplayableCount);
}

internal void
UpdateColumnVerticalSlider(display_column *DisplayColumn, u32 DisplayableCount)
{
    slider *Slider = &DisplayColumn->SliderVertical;
    
    r32 TotalDisplayableSize = GetDisplayableHeight(DisplayableCount, DisplayColumn->SlotHeight);
    v2 TotalScale            = GetSize(Slider->Background);
    Slider->OverhangP = Clamp(TotalScale.y/TotalDisplayableSize, 0.01f, 1.0f);
    
    v2 NewScale = {GetSize(Slider->GrabThing).x, Max(TotalScale.y*Slider->OverhangP, 5.0f)};
    SetSize(Slider->GrabThing, NewScale);
    Slider->MaxSlidePix = Max(TotalScale.y - NewScale.y, 0.0f)/2.0f;
    UpdateColumnVerticalSliderPosition(DisplayColumn, DisplayableCount);
}

inline void
UpdateVerticalSliders(music_info *MusicInfo)
{
    UpdateColumnVerticalSlider(&MusicInfo->DisplayInfo.Playlists, MusicInfo->Playlist_->Playlists.Displayable.A.Count);
    UpdateColumnVerticalSlider(&MusicInfo->DisplayInfo.Genre,     MusicInfo->Playlist_->Genre.Displayable.A.Count);
    UpdateColumnVerticalSlider(&MusicInfo->DisplayInfo.Artist,    MusicInfo->Playlist_->Artist.Displayable.A.Count);
    UpdateColumnVerticalSlider(&MusicInfo->DisplayInfo.Album,     MusicInfo->Playlist_->Album.Displayable.A.Count);
    UpdateColumnVerticalSlider(&MusicInfo->DisplayInfo.Song.Base, MusicInfo->Playlist_->Song.Displayable.A.Count);
}

internal void
OnVerticalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    drag_slider_data *DragData = (drag_slider_data *)Data;
    display_column *DisplayColumn = DragData->DisplayColumn;
    slider *Slider = &DisplayColumn->SliderVertical;
    u32 DisplayableCount = DragData->MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable.A.Count;
    
    r32 GrabThingHalfHeight  = GetSize(Slider->GrabThing).y/2.0f;
    if(Slider->MouseOffset.y < GrabThingHalfHeight && Slider->MouseOffset.y > -GrabThingHalfHeight) 
        AdjustedMouseP.y -= Slider->MouseOffset.y;
    
    r32 ParentY = GetLocalPosition(Slider->GrabThing->ID->Parent).y;
    r32 BGYPos = GetLocalPosition(Slider->Background).y + ParentY;
    r32 NewY   = Clamp(AdjustedMouseP.y, BGYPos - Slider->MaxSlidePix, BGYPos + Slider->MaxSlidePix);
    
    SetLocalPosition(Slider->GrabThing, V2(GetLocalPosition(Slider->GrabThing).x, NewY-ParentY));
    
    r32 TotalSliderScale = GetSize(Slider->Background).y;
    r32 GrabThingSize    = GetSize(Slider->GrabThing).y;
    r32 RemainingScale   = TotalSliderScale-GrabThingSize;
    r32 TopPositionY     = BGYPos+Slider->MaxSlidePix;
    
    r32 TranslationPercentage = SafeDiv(TopPositionY-NewY,Slider->MaxSlidePix*2);
    if(TranslationPercentage > 0.999f) TranslationPercentage = 1;
    r32 TotalHeight = GetDisplayableHeight(DisplayableCount, DisplayColumn->SlotHeight);
    TotalHeight = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    i32 ScrollAmount = (i32)(DisplayColumn->DisplayCursor - TotalHeight*TranslationPercentage);
    ScrollDisplayColumn(Renderer, DragData->MusicInfo, DisplayColumn, (r32)-ScrollAmount);
    
    UpdateColumnHorizontalSlider(DisplayColumn, DisplayableCount);
}

internal void
OnSongDragEnd(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    playlist_column *SongColumn = &MusicInfo->Playlist_->Song;
    
    array_u32 IgnoreDecodeIDs = CreateArray(&GlobalGameState.ScratchArena, 2);
    
    // This finds the next and prev DecodeIDs in order to not evict them on the upcoming load
    if(MusicInfo->PlayingSong.DisplayableID.ID > -1)
    {
        file_id PrevFileID = GetFileID(MusicInfo, 
                                       GetPreviousSong(SongColumn->Displayable.A.Count, MusicInfo->PlayingSong.DisplayableID));
        Assert(PrevFileID > -1);
        u32 PrevDecodeID = 0;
        if(!Find(&DecodeInfo->FileIDs, PrevFileID, &PrevDecodeID)) 
        {
            AddJob_LoadMP3(&GlobalGameState.JobQueue, GetPlaylistID(&MusicInfo->Playlist_->Song, PrevFileID));
            if(!Find(&DecodeInfo->FileIDs, PrevFileID, &PrevDecodeID)) Assert(false);
        }
        Push(&IgnoreDecodeIDs, PrevDecodeID);
        
        file_id NextFileID = GetFileID(MusicInfo, 
                                       GetSongAfterCurrent(SongColumn->Displayable.A.Count, MusicInfo->PlayingSong.DisplayableID));
        Assert(NextFileID > -1);
        u32 NextDecodeID = 0;
        if(!Find(&DecodeInfo->FileIDs, NextFileID, &NextDecodeID)) 
        {
            AddJob_LoadMP3(&GlobalGameState.JobQueue, GetPlaylistID(&MusicInfo->Playlist_->Song, NextFileID));
            if(!Find(&DecodeInfo->FileIDs, NextFileID, &NextDecodeID)) Assert(false);
        }
        Push(&IgnoreDecodeIDs, NextDecodeID);
    }
    
    AddJobs_LoadOnScreenMP3s(&GlobalGameState, &GlobalGameState.JobQueue, &IgnoreDecodeIDs);
    DestroyArray(&GlobalGameState.ScratchArena, IgnoreDecodeIDs);
}

internal void
ChangeVolume(game_state *GameState, r32 NewVolume)
{
    NewVolume = Clamp(NewVolume, 0.0f, 1.0f);
    music_display_info *DisplayInfo = &GameState->MusicInfo.DisplayInfo;
    slider *Slider = &DisplayInfo->Volume;
    
    r32 NewX = GetPosition(Slider->Background).x - Slider->MaxSlidePix + Slider->MaxSlidePix*NewVolume*2;
    
    SetPosition(Slider->GrabThing, V2(NewX, GetPosition(Slider->GrabThing).y));
    
    SetToneVolume(GameState->SoundThreadInterface, NewVolume);
}

internal void
OnVolumeDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    game_state *GameState = (game_state *)Data;
    music_display_info *DisplayInfo = &GameState->MusicInfo.DisplayInfo;
    slider *VolumeSlider = &DisplayInfo->Volume;
    
    r32 BGX = GetPosition(VolumeSlider->Background).x;
    r32 NewX = Clamp(AdjustedMouseP.x, BGX-VolumeSlider->MaxSlidePix, BGX+VolumeSlider->MaxSlidePix);
    
    VolumeSlider->GrabThing->ID->Transform.Translation.x = NewX;
    r32 TranslationPercent = SafeDiv(1.0f,(VolumeSlider->MaxSlidePix*2))*(NewX-BGX) + 0.5f;
    
    SetToneVolume(GameState->SoundThreadInterface, TranslationPercent);
}

inline b32 
IsInOnScreenList(display_column *DisplayColumn, displayable_id PlaylistID, u32 *OnScreenID = 0)
{
    b32 Result = false;
    
    For(DisplayColumn->Count)
    {
        if(PlaylistID == DisplayColumn->OnScreenIDs[It].ID) // TODO::PLAYLIST_DISPLAYABLE
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
Select(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 OnScreenIDsID)
{
    Assert(DisplayColumn->Count > OnScreenIDsID && PlaylistColumn->Displayable.A.Count > OnScreenIDsID);
    
    playlist_id PlaylistID = Get(&PlaylistColumn->Displayable, DisplayColumn->OnScreenIDs[OnScreenIDsID]);
    Push(&PlaylistColumn->Selected, PlaylistID);
}

inline void
Deselect(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 OnScreenIDsID)
{
    Assert(DisplayColumn->Count > OnScreenIDsID && PlaylistColumn->Displayable.A.Count > OnScreenIDsID);
    
    playlist_id PlaylistID  = Get(&PlaylistColumn->Displayable, DisplayColumn->OnScreenIDs[OnScreenIDsID]);
    StackFindAndTake(&PlaylistColumn->Selected, PlaylistID);
}

inline b32
IsSelected(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 OnScreenIDsID)
{
    Assert(DisplayColumn->Count > OnScreenIDsID && PlaylistColumn->Displayable.A.Count > OnScreenIDsID);
    
    playlist_id PlaylistID  = Get(&PlaylistColumn->Displayable, DisplayColumn->OnScreenIDs[OnScreenIDsID]);
    return StackContains(&PlaylistColumn->Selected, PlaylistID);
}

inline void
ToggleSelection(display_column *DisplayColumn, playlist_column *PlaylistColumn, u32 OnScreenIDsID)
{
    if(IsSelected(DisplayColumn, PlaylistColumn, OnScreenIDsID)) Deselect(DisplayColumn, PlaylistColumn, OnScreenIDsID);
    else Select(DisplayColumn, PlaylistColumn, OnScreenIDsID);
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
            SetColor(DisplayColumn->BGRects[It], DisplayColumn->Colors.Selected);
        }
        else SetColor(DisplayColumn->BGRects[It], DisplayColumn->Colors.Slot); 
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
        playlist_id ActualID  = Get(&PlaylistColumn->Displayable, DisplayColumn->OnScreenIDs[It]);
        if(PlaylistID == ActualID)
        {
            SetColor(DisplayColumn->BGRects[It], Color);
        }
        else if(StackContains(&PlaylistColumn->Selected, ActualID))
        {
            SetColor(DisplayColumn->BGRects[It], DisplayColumn->Colors.Selected);
        }
        else SetColor(DisplayColumn->BGRects[It], DisplayColumn->Colors.Slot); 
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
        if(!DisplayColumn->BGRects[It]) 
        {
            Assert(It > 0);
            CreateDisplayBackgroundRect(Renderer, DisplayColumn, It, DisplayColumn->ZValue, DisplayColumn->BGRects[It-1]);
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
        else SetActive(DisplayColumn->Text+ID, false);
        if(DisplayColumn->BGRects[ID]) DisplayColumn->BGRects[ID]->ID->Render = false;
    }
    
    DisplayColumn->Count = NewCount;
}

inline void
RemoveSongText(display_column *DisplayColumn, u32 ID)
{
    display_column_song_extension *DisplaySong = ColumnExt(DisplayColumn);
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
    display_column_song_extension *DisplaySong = ColumnExt(DisplayColumn);
    mp3_metadata *MD   = GetMetadata(PlaylistColumn, DisplaySong->FileInfo, NextID);
    
    v2 SongP = {Layout->SongXOffset, Layout->SongFirstRowYOffset};
    RenderText(&GlobalGameState, font_Medium, &MD->Title, DisplayColumn->Colors.Text,
               DisplaySong->SongTitle+ID, -0.12001f, DisplayColumn->BGRects[ID], SongP);
    
    v2 AlbumP = {Layout->SongAlbumXOffset, Layout->SongSecondRowYOffset}; 
    RenderText(&GlobalGameState, font_Small, &MD->Album, DisplayColumn->Colors.Text,
               DisplaySong->SongAlbum+ID, -0.12f, DisplayColumn->BGRects[ID], AlbumP);
    
    v2 ArtistP = {Layout->SongXOffset, Layout->SongThirdRowYOffset};
    RenderText(&GlobalGameState, font_Small, &MD->Artist, DisplayColumn->Colors.Text,
               DisplaySong->SongArtist+ID, -0.12f, DisplayColumn->BGRects[ID], ArtistP);
    
    v2 TrackP = {Layout->SongTrackXOffset, Layout->SongFirstRowYOffset};
    RenderText(&GlobalGameState, font_Medium, &MD->TrackString, DisplayColumn->Colors.Text, DisplaySong->SongTrack+ID, -0.12f, 
               DisplayColumn->BGRects[ID], TrackP);
    
    string_c YearAddon = NewStringCompound(&GlobalGameState.ScratchArena, 10);
    string_c Addon = NewStaticStringCompound(" |");
    if(MD->YearString.Pos < 4) 
    {
        string_c FakeYear = NewStaticStringCompound("   --  ");
        ConcatStringCompounds(3, &YearAddon, &FakeYear, &Addon);
    }
    else if(MD->YearString.Pos > 4);
    else ConcatStringCompounds(3, &YearAddon, &MD->YearString, &Addon);
    
    v2 YearP = {Layout->SongXOffset, Layout->SongSecondRowYOffset};
    RenderText(&GlobalGameState, font_Small, &YearAddon, DisplayColumn->Colors.Text,
               DisplaySong->SongYear+ID, -0.12f, DisplayColumn->BGRects[ID], YearP);
    
    DeleteStringCompound(&GlobalGameState.ScratchArena, &YearAddon);
}

internal void
MoveDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn,
                  displayable_id DisplayableStartID, r32 StartY)
{ 
    playlist_column *PlaylistColumn = MusicInfo->Playlist_->Columns + DisplayColumn->Type;
    sort_batch      *SortBatch      = &PlaylistColumn->Batch;
    // FileOrDisplayableStartID is either a fileID when the colum is the song column, 
    // or a displayID when it is another.
    DisplayColumn->DisplayCursor = DisplayableStartID.ID*DisplayColumn->SlotHeight + StartY;
    
    For(DisplayColumn->Count)
    {
        if(DisplayColumn->Type == columnType_Song)
        {
            RemoveSongText(DisplayColumn, It);
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, false);
        }
        else RemoveRenderText(Renderer, DisplayColumn->Text+It);
        DisplayColumn->BGRects[It]->ID->Render = false;
    }
    
    displayable_id NextID = DisplayableStartID;
    SetLocalPosition(DisplayColumn->BGRects[0], V2(0, StartY));
    
    for(u32 It = 0; 
        It < PlaylistColumn->Displayable.A.Count && 
        It < DisplayColumn->Count;
        It++)
    {
        DisplayColumn->BGRects[It]->ID->Render = true;
        
        if(DisplayColumn->Type == columnType_Song) 
        {
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, true);
            UpdateSongText(PlaylistColumn, DisplayColumn, It, NextID);
        }
        else
        {
            string_c *Name  = GetName(PlaylistColumn, NextID);
            RenderText(&GlobalGameState, font_Small, Name, DisplayColumn->Colors.Text,
                       DisplayColumn->Text+It,  DisplayColumn->ZValue-0.01f, DisplayColumn->BGRects[It]);
            
            Translate(DisplayColumn->Text+It, V2(0, GlobalGameState.Layout.SlotTextYOffset));
            //CenterText(DisplayColumn->Text+It); // NOTE:: Use this to align ontext height
        }
        DisplayColumn->OnScreenIDs[It] = NextID;
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
    r32 SlotHeight = DisplayColumn->SlotHeight;
    r32 TotalHeight   = GetDisplayableHeight(PlaylistColumn->Displayable.A.Count, SlotHeight);
    // DisplayCursor is the very top position. Therefore MaxHeight needs to be reduced by VisibleHeight
    TotalHeight = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    i32 PrevCursorID = Floor(DisplayColumn->DisplayCursor/SlotHeight);
    DisplayColumn->DisplayCursor = Clamp(DisplayColumn->DisplayCursor+ScrollAmount, 0.0f, TotalHeight);
    
    r32 NewY = Mod(DisplayColumn->DisplayCursor, SlotHeight);
    i32 NewCursorID = Floor(DisplayColumn->DisplayCursor/SlotHeight);
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
        SetLocalPosition(DisplayColumn->BGRects[0], V2(0, NewY));
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
        if(IsIntersectingRectButTopShowing(DisplayColumn->BGRects[OnScreenID], DisplayColumn->BottomBorder))
        {
            ScrollDisplayColumn(Renderer, MusicInfo, DisplayColumn, DisplayColumn->SlotHeight);
        }
        else if(IsIntersectingRectButBottomShowing(DisplayColumn->BGRects[OnScreenID], DisplayColumn->TopBorder))
        {
            ScrollDisplayColumn(Renderer, MusicInfo, DisplayColumn, -DisplayColumn->SlotHeight);
        }
    }
}

internal void
FitDisplayColumnIntoSlot(renderer *Renderer, display_column *DisplayColumn, u32 DisplayableCount)
{
    // Calc new column scale
    DisplayColumn->ColumnHeight = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    DisplayColumn->SlotWidth    = /*4+*/WidthBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->RightBorder);
    
    // Calc new column translation
    r32 CenterX = CenterXBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->RightBorder);
    r32 CenterY = CenterYBetweenRects(DisplayColumn->BottomBorder, DisplayColumn->TopBorder);
    
    SetSize(DisplayColumn->Background, V2(DisplayColumn->SlotWidth, DisplayColumn->ColumnHeight));
    SetLocalPosition(DisplayColumn->Background, V2(CenterX, CenterY));
    
    // Calc slot and text positions
    SetPosition(DisplayColumn->BGRectAnchor, V2(CenterX, GetPosition(DisplayColumn->BGRectAnchor).y));
    For(DisplayColumn->Count)
    {
        SetSize(DisplayColumn->BGRects[It], V2(DisplayColumn->SlotWidth, GetSize(DisplayColumn->BGRects[It]).y));
    }
    ResetColumnText(DisplayColumn, DisplayableCount);
    
    // Calc slider fit
    r32 BGSizeY = GetSize(DisplayColumn->SliderHorizontal.Background).y;
    r32 BGPosY  = GetLocalPosition(DisplayColumn->SliderHorizontal.Background).y;
    SetSize(DisplayColumn->SliderHorizontal.Background, V2(DisplayColumn->SlotWidth, BGSizeY));
    SetPosition(DisplayColumn->SliderHorizontal.Background, V2(CenterX, BGPosY));
    
    slider *VSlider = &DisplayColumn->SliderVertical;
    r32 BGSizeX = GetSize(VSlider->Background).x;
    SetSize(VSlider->Background, V2(BGSizeX, DisplayColumn->ColumnHeight));
    UpdateColumnVerticalSlider(DisplayColumn, DisplayableCount);
    
    // Search bar
    SetSize(DisplayColumn->Search.TextField.Background,
            V2(DisplayColumn->SlotWidth, GetSize(DisplayColumn->Search.TextField.Background).y)); // @Layout
    SetLocalPosition(DisplayColumn->Search.TextField.LeftAlign, V2(DisplayColumn->SlotWidth*-0.5f, 0));
    
    // Fitting the playlist panel at this point, as nothing depends on it being set directly.
    // This is a good place to set it, as it is technically part of the column itself.
    if(DisplayColumn->Type == columnType_Playlists)
    {
        playlist_ui *PlaylistUI = &DisplayColumn->Base->PlaylistUI;
        r32 PLCenterX = CenterXBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->Base->PlaylistsGenre.Edge);
        r32 PLWidth   = WidthBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->Base->PlaylistsGenre.Edge);
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
    MoveDisplayColumn(Renderer, MusicInfo, DisplayColumn, DisplayColumn->OnScreenIDs[0],GetLocalPosition(DisplayColumn->BGRects[0]).y);
    
    // I do this to fix that if the column is at the bottom and the window gets bigger, the
    // slots will be stopped at the edge. Without this, the new visible slots will be the same
    // ID. See: #LastSlotOverflow in MoveDisplayColumn
    drag_slider_data Data = { MusicInfo, DisplayColumn};
    OnVerticalSliderDrag(Renderer, GetPosition(DisplayColumn->SliderVertical.GrabThing), 0, &Data);
}

internal void
OnDisplayColumnEdgeDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    column_edge_drag *Info = (column_edge_drag *)Data;
    
    edge_chain *LeftChain  = &Info->LeftEdgeChain;
    edge_chain *RightChain = &Info->RightEdgeChain;
    Assert(LeftChain->Count  > 0); // A 'border' edge is required for 
    Assert(RightChain->Count > 0); // both sides.
    
    // These are the max distances the Dragable edge 
    // can go in both directions.
    r32 TotalLeftOffset  = 0;
    r32 TotalRightOffset = 0;
    For(LeftChain->Count)  TotalLeftOffset  += LeftChain->Offsets[It];
    For(RightChain->Count) TotalRightOffset += RightChain->Offsets[It];
    
    // Constraining the dragged edge to the max and min, 
    // then setting its position and current percentage, 
    // which is needed when resizing the window.
    r32 NewDragX = Clamp(AdjustedMouseP.x, 
                         GetLocalPosition(LeftChain->Edges[ LeftChain->Count -1]).x + TotalLeftOffset, 
                         GetLocalPosition(RightChain->Edges[RightChain->Count-1]).x - TotalRightOffset);
    SetLocalPosition(Dragable, V2(NewDragX, GetLocalPosition(Dragable).y));
    *Info->XPercent = GetPosition(Dragable).x/(r32)Renderer->Window.CurrentDim.Width;
    
    // Constrain all following left side edges that
    // might need to be constrained.
    r32 RightSideP = NewDragX;
    For(LeftChain->Count-1)
    {
        v2  OldP = GetLocalPosition(LeftChain->Edges[It]);
        
        TotalLeftOffset -= LeftChain->Offsets[It];
        r32 NewX = Clamp(OldP.x, GetLocalPosition(LeftChain->Edges[LeftChain->Count - 1]).x + TotalLeftOffset, 
                         RightSideP - LeftChain->Offsets[It]);
        SetLocalPosition(LeftChain->Edges[It], V2(NewX, OldP.y));
        *LeftChain->XPercent[It] = GetPosition(LeftChain->Edges[It]).x/(r32)Renderer->Window.CurrentDim.Width;
        
        RightSideP = NewX;
    }
    
    // Constrain all following right side edges that
    // might need to be constrained.
    r32 LeftSideP = NewDragX;
    For(RightChain->Count-1)
    {
        v2  OldP      = GetLocalPosition(RightChain->Edges[It]);
        
        TotalRightOffset -= RightChain->Offsets[It];
        r32 NewX = Clamp(OldP.x, LeftSideP + RightChain->Offsets[It], 
                         GetLocalPosition(RightChain->Edges[RightChain->Count - 1]).x - TotalRightOffset);
        SetLocalPosition(RightChain->Edges[It], V2(NewX, OldP.y));
        *RightChain->XPercent[It] = GetPosition(RightChain->Edges[It]).x/(r32)Renderer->Window.CurrentDim.Width;
        
        LeftSideP = NewX;
    }
    
    
    playlist_info *Playlist = Info->MusicInfo->Playlist_;
    FitDisplayColumnIntoSlot(Renderer, &Info->MusicInfo->DisplayInfo.Playlists, Playlist->Playlists.Displayable.A.Count);
    FitDisplayColumnIntoSlot(Renderer, &Info->MusicInfo->DisplayInfo.Genre,     Playlist->Genre.Displayable.A.Count);
    FitDisplayColumnIntoSlot(Renderer, &Info->MusicInfo->DisplayInfo.Artist,    Playlist->Artist.Displayable.A.Count);
    FitDisplayColumnIntoSlot(Renderer, &Info->MusicInfo->DisplayInfo.Album,     Playlist->Album.Displayable.A.Count);
    FitDisplayColumnIntoSlot(Renderer, &Info->MusicInfo->DisplayInfo.Song.Base, Playlist->Song.Displayable.A.Count);
    UpdateHorizontalSliders(Info->MusicInfo);
}

inline void
SetBetweenSliderRectPosition(entry_id *BetweenRect, entry_id *EdgeDragEntry)
{
    v2 Pos0 = GetPosition(EdgeDragEntry) - GetSize(EdgeDragEntry)*0.5f;
    Pos0 += HadamardProduct(GetSize(BetweenRect)*0.5f, V2(-1, 1));
    SetPosition(BetweenRect, Pos0);
}

internal void
ProcessEdgeDragOnResize(renderer *Renderer, music_display_info *DisplayInfo)
{
    r32 CWidth   = (r32)Renderer->Window.CurrentDim.Width;
    r32 CurrentY = (r32)Renderer->Window.CurrentDim.Height*0.5f;
    r32 FixedY   = (r32)Renderer->Window.FixedDim.Height*0.5f;
    r32 NewYTranslation = CurrentY - FixedY;
    
    r32 NewXGenreArtist = CWidth*DisplayInfo->GenreArtist.XPercent;
    SetPosition(DisplayInfo->GenreArtist.Edge, V2(NewXGenreArtist, DisplayInfo->GenreArtist.OriginalYHeight+NewYTranslation));
    
    r32 NewXArtistAlbum = CWidth*DisplayInfo->ArtistAlbum.XPercent;
    SetPosition(DisplayInfo->ArtistAlbum.Edge, V2(NewXArtistAlbum, DisplayInfo->ArtistAlbum.OriginalYHeight+NewYTranslation));
    
    r32 NewXAlbumSong = CWidth*DisplayInfo->AlbumSong.XPercent;
    SetPosition(DisplayInfo->AlbumSong.Edge, V2(NewXAlbumSong, DisplayInfo->AlbumSong.OriginalYHeight+NewYTranslation));
    
    r32 NewXPlaylistGenre = CWidth*DisplayInfo->PlaylistsGenre.XPercent;
    SetPosition(DisplayInfo->PlaylistsGenre.Edge, V2(NewXPlaylistGenre, 
                                                     DisplayInfo->PlaylistsGenre.OriginalYHeight+NewYTranslation));
    
    SetBetweenSliderRectPosition(DisplayInfo->Genre.BetweenSliderRect,     DisplayInfo->GenreArtist.Edge);
    SetBetweenSliderRectPosition(DisplayInfo->Artist.BetweenSliderRect,    DisplayInfo->ArtistAlbum.Edge);
    SetBetweenSliderRectPosition(DisplayInfo->Album.BetweenSliderRect,     DisplayInfo->AlbumSong.Edge);
    SetBetweenSliderRectPosition(DisplayInfo->Song.Base.BetweenSliderRect, DisplayInfo->EdgeRight);
    SetBetweenSliderRectPosition(DisplayInfo->Playlists.BetweenSliderRect, DisplayInfo->PlaylistsGenre.Edge);
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
    
    GameState->MusicInfo.Looping = playLoop_Loop;
}

inline void
OnLoopPlaylistToggleOff(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    
    GameState->MusicInfo.Looping = playLoop_NoLoop;
}

inline void
OnShufflePlaylistToggleOn(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->IsShuffled = true;
    
    ShuffleStack(&MusicInfo->Playlist_->Song.Displayable);
    UpdatePlayingSongForSelectionChange(MusicInfo);
    MoveDisplayColumn(&GameState->Renderer, MusicInfo, &MusicInfo->DisplayInfo.Song.Base);
}

inline void
OnShufflePlaylistToggleOff(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->IsShuffled = false;
    
    SortDisplayables(MusicInfo, &GameState->MP3Info->FileInfo);
    UpdatePlayingSongForSelectionChange(MusicInfo);
    MoveDisplayColumn(&GameState->Renderer, MusicInfo, &MusicInfo->DisplayInfo.Song.Base);
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
    settings *Settings = &GlobalGameState.Settings;
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
    settings *Settings = &GlobalGameState.Settings;
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
        RenderText(&GlobalGameState, font_Medium, &PathText, &DisplayInfo->ColorPalette.ForegroundText, &MusicPath->CurrentPath, -0.6f-0.001f, MusicPath->TextField.LeftAlign, 
                   V2(0, 62)); // @Layout
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
OnColorPicker(void *Data)
{
    SetActive((color_picker *)Data, true);
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
    r32 BottomBorder = Layout->BottomBorder;
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
    bitmap_color_format cF = colorFormat_RGBA;
    loaded_bitmap Bitmap = {1, Background_Icon_Width, Background_Icon_Height, 
        (u32 *)Background_Icon_Data, cF, ArrayCount(Background_Icon_Data)};
    Renderer->ButtonBaseID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Loop_Icon_Width, Loop_Icon_Height, 
        (u32 *)Loop_Icon_Data, cF, ArrayCount(Loop_Icon_Data)};
    u32 LoopID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Loop_Pressed_Icon_Width, Loop_Pressed_Icon_Height, 
        (u32 *)Loop_Pressed_Icon_Data, cF, ArrayCount(Loop_Pressed_Icon_Data)};
    u32 LoopPressedID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Randomize_Icon_Width, Randomize_Icon_Height, 
        (u32 *)Randomize_Icon_Data, cF, ArrayCount(Randomize_Icon_Data)};
    u32 RandomizeID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Randomize_Pressed_Icon_Width, Randomize_Pressed_Icon_Height, 
        (u32 *)Randomize_Pressed_Icon_Data, cF, ArrayCount(Randomize_Pressed_Icon_Data)};
    u32 RandomizePressedID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Play_Icon_Width, Play_Icon_Height, 
        (u32 *)Play_Icon_Data, cF, ArrayCount(Play_Icon_Data)};
    u32 PlayID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Pause_Icon_Width, Pause_Icon_Height, 
        (u32 *)Pause_Icon_Data, cF, ArrayCount(Pause_Icon_Data)};
    u32 PauseID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Stop_Icon_Width, Stop_Icon_Height, 
        (u32 *)Stop_Icon_Data, cF, ArrayCount(Stop_Icon_Data)};
    u32 StopID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Next_Icon_Width, Next_Icon_Height, 
        (u32 *)Next_Icon_Data, cF, ArrayCount(Next_Icon_Data)};
    u32 NextID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Previous_Icon_Width, Previous_Icon_Height, 
        (u32 *)Previous_Icon_Data, cF, ArrayCount(Previous_Icon_Data)};
    u32 PreviousID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, MusicPath_Icon_Width, MusicPath_Icon_Height, 
        (u32 *)MusicPath_Icon_Data, cF, ArrayCount(MusicPath_Icon_Data)};
    u32 MusicPathID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Confirm_Icon_Width, Confirm_Icon_Height, 
        (u32 *)Confirm_Icon_Data, cF, ArrayCount(Confirm_Icon_Data)};
    u32 ConfirmID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Cancel_Icon_Width, Cancel_Icon_Height, 
        (u32 *)Cancel_Icon_Data, cF, ArrayCount(Cancel_Icon_Data)};
    u32 CancelID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, PaletteSwap_Icon_Width, PaletteSwap_Icon_Height, 
        (u32 *)PaletteSwap_Icon_Data, cF, ArrayCount(PaletteSwap_Icon_Data)};
    u32 PaletteID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Rescan_Icon_Width, Rescan_Icon_Height, 
        (u32 *)Rescan_Icon_Data, cF, ArrayCount(Rescan_Icon_Data)};
    u32 RescanID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, ColorPicker_Icon_Width, ColorPicker_Icon_Height, 
        (u32 *)ColorPicker_Icon_Data, cF, ArrayCount(ColorPicker_Icon_Data)};
    u32 ColorPickerID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Help_Icon_Width, Help_Icon_Height, 
        (u32 *)Help_Icon_Data, cF, ArrayCount(Help_Icon_Data)};
    u32 ShortcutID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Help_Pressed_Icon_Width, Help_Pressed_Icon_Height, 
        (u32 *)Help_Pressed_Icon_Data, cF, ArrayCount(Help_Pressed_Icon_Data)};
    u32 ShortcutPressedID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Playlist_Add_Icon_Width, Playlist_Add_Icon_Height, 
        (u32 *)Playlist_Add_Icon_Data, cF, ArrayCount(Playlist_Add_Icon_Data)};
    u32 PLAddID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Playlist_AddSelection_Icon_Width, Playlist_AddSelection_Icon_Height, 
        (u32 *)Playlist_AddSelection_Icon_Data, cF, ArrayCount(Playlist_AddSelection_Icon_Data)};
    u32 PLAddSelectionID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Playlist_Remove_Icon_Width, Playlist_Remove_Icon_Height, 
        (u32 *)Playlist_Remove_Icon_Data, cF, ArrayCount(Playlist_Remove_Icon_Data)};
    u32 PLRemoveID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
    Bitmap = {1, Playlist_Rename_Icon_Width, Playlist_Rename_Icon_Height, 
        (u32 *)Playlist_Rename_Icon_Data, cF, ArrayCount(Playlist_Rename_Icon_Data)};
    u32 PLRenameID = DecodeAndCreateGLTexture(&GameState->ScratchArena, Bitmap);
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
    
    r32 EdgeWidth = Layout->DragEdgeWidth;
    r32 ColumnWidth = (WWidth/2.0f)/4.0f + 30;
    
    
    r32 PlaylistsGenreX = ColumnWidth*1+LeftBorder*0;
    DisplayInfo->PlaylistsGenre.Edge = 
        CreateRenderRect(Renderer, {{PlaylistsGenreX,BottomBorder},{PlaylistsGenreX+EdgeWidth,WHeight-TopBorder}},
                         -0.5f, 0, &Palette->Foreground);
    DisplayInfo->PlaylistsGenre.XPercent        = GetPosition(DisplayInfo->PlaylistsGenre.Edge).x/WWidth;
    DisplayInfo->PlaylistsGenre.OriginalYHeight = GetPosition(DisplayInfo->PlaylistsGenre.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->PlaylistsGenre.Edge, scaleAxis_Y);
    
    r32 GenreArtistX = ColumnWidth*2+LeftBorder*1;
    DisplayInfo->GenreArtist.Edge = CreateRenderRect(Renderer, 
                                                     {{GenreArtistX,BottomBorder},{GenreArtistX+EdgeWidth,WHeight-TopBorder}},
                                                     -0.5f, 0, &Palette->Foreground);
    DisplayInfo->GenreArtist.XPercent        = GetPosition(DisplayInfo->GenreArtist.Edge).x/WWidth;
    DisplayInfo->GenreArtist.OriginalYHeight = GetPosition(DisplayInfo->GenreArtist.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->GenreArtist.Edge, scaleAxis_Y);
    
    
    
    r32 ArtistAlbumX = ColumnWidth*3+LeftBorder+EdgeWidth*2;
    DisplayInfo->ArtistAlbum.Edge = CreateRenderRect(Renderer, 
                                                     {{ArtistAlbumX,BottomBorder},{ArtistAlbumX+EdgeWidth,WHeight-TopBorder}}, 
                                                     -0.5f, 0, &Palette->Foreground);
    DisplayInfo->ArtistAlbum.XPercent        = GetPosition(DisplayInfo->ArtistAlbum.Edge).x/WWidth;
    DisplayInfo->ArtistAlbum.OriginalYHeight = GetPosition(DisplayInfo->ArtistAlbum.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->ArtistAlbum.Edge, scaleAxis_Y);
    
    
    
    r32 AlbumSongX  = ColumnWidth*4+LeftBorder+EdgeWidth*3;
    DisplayInfo->AlbumSong.Edge   = CreateRenderRect(Renderer, 
                                                     {{AlbumSongX,BottomBorder},{AlbumSongX+EdgeWidth,WHeight-TopBorder}},
                                                     -0.5f, 0, &Palette->Foreground);
    DisplayInfo->AlbumSong.XPercent        = GetPosition(DisplayInfo->AlbumSong.Edge).x/WWidth;
    DisplayInfo->AlbumSong.OriginalYHeight = GetPosition(DisplayInfo->AlbumSong.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->AlbumSong.Edge, scaleAxis_Y);
    
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
    TranslateWithScreen(&Renderer->TransformList, PlaylistUI->Panel, fixedTo_TopLeft);
    
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
    
    PlaylistUI->RenameField = CreateTextField(Renderer, &GameState->FixArena, V2(200.0f, Layout->SlotHeight-Layout->SlotGap), 
                                              PanelDepth-0.0019f, (u8 *)"New name...", 0, 
                                              PLColumnColors.Text, PLColumnColors.Slot, font_Small, PLAYLIST_MAX_NAME_LENGTH);
    SetActive(&PlaylistUI->RenameField, false);
    UpdateTextField(Renderer, &PlaylistUI->RenameField);
    
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
                        DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->GenreArtist.Edge, Layout);
    CreateDisplayColumn(ArtistColumn, &GameState->FixArena, columnType_Artist, ColumnColors, -0.05f, 
                        DisplayInfo->GenreArtist.Edge, DisplayInfo->ArtistAlbum.Edge, Layout);
    CreateDisplayColumn(AlbumColumn, &GameState->FixArena, columnType_Album, ColumnColors, -0.075f, 
                        DisplayInfo->ArtistAlbum.Edge, DisplayInfo->AlbumSong.Edge, Layout);
    CreateDisplayColumn(SongColumn, &GameState->FixArena, columnType_Song, ColumnColors, -0.1f, 
                        DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight, Layout);
    CreateDisplayColumn(PlaylistsColumn, &GameState->FixArena, columnType_Playlists, PLColumnColors, PlaylistColumnDepth, 
                        DisplayInfo->EdgeLeft, DisplayInfo->PlaylistsGenre.Edge, Layout);
    
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
    
    r32 TimelineX   = VolumeXEnd + Layout->TimelineXGap;
    r32 TimelineGTY = (PlayPauseP.y - PlayPauseS);
    Panel->Timeline.GrabThing  = 
        CreateRenderRect(Renderer, {{TimelineX, TimelineGTY}, {TimelineX+Layout->TimelineGrapThingWidth, TimelineGTY+Layout->TimelineGrapThingHeight}}, BtnDepth - 0.0000001f, 0, &Palette->SliderGrabThing);
    
    r32 TimelineY = TimelineGTY + Layout->TimelineGrapThingHeight*0.5f;
    Panel->Timeline.Background = 
        CreateRenderRect(Renderer, {{TimelineX, TimelineY - Layout->TimelineHeight*0.5f}, 
                             {TimelineX+Layout->TimelineWidth, TimelineY+Layout->TimelineHeight*0.5f}
                         }, BtnDepth, 0, &Palette->SliderBackground);
    
    Panel->Timeline.MaxSlidePix = GetExtends(Panel->Timeline.Background).x - GetExtends(Panel->Timeline.GrabThing).x;
    SetTheNewPlayingSong(Renderer, Panel, Layout, &GameState->MusicInfo);
    
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
    DisplayInfo->ColorPicker = NewButton(Renderer, SmallBtnRect, BtnDepth, false, 
                                         Renderer->ButtonBaseID, ColorPickerID, Renderer->ButtonColors, 0);
    SetLocalPosition(DisplayInfo->ColorPicker, V2(ButtonUpperLeftX + StepX*2, Renderer->Window.CurrentDim.Height-BtnY));
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->ColorPicker->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->ColorPicker->Icon, fixedTo_TopLeft);
    DisplayInfo->ColorPicker->OnPressed = {OnColorPicker, &GameState->ColorPicker};
    
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
                                           (u8 *)"New Path...", 0, &Palette->Text, 
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
        music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
        
        b32 PrevIsHovering = Popups->IsHovering;
        Popups->IsHovering = false;
        
        font_size FontSize = GetFontSize(&GlobalGameState.Renderer, font_Medium);
        if(!DisplayInfo->MusicPath.TextField.IsActive)
        {
            if(IsOnButton(GlobalGameState.ColorPicker.Save, MouseP))
            {
                if(Popups->ActiveText != 23) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->SavePalette, FontSize);
                Popups->ActiveText = 23;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(GlobalGameState.ColorPicker.New, MouseP))
            {
                if(Popups->ActiveText != 24) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->CopyPalette, FontSize);
                Popups->ActiveText = 24;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(GlobalGameState.ColorPicker.Remove, MouseP))
            {
                if(Popups->ActiveText != 25) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->DeletePalette, FontSize);
                Popups->ActiveText = 25;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(GlobalGameState.ColorPicker.Cancel, MouseP))
            {
                if(Popups->ActiveText != 26) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->CancelPicker, FontSize);
                Popups->ActiveText = 26;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->PaletteSwap))
            {
                if(Popups->ActiveText != 1) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->PaletteSwap, FontSize);
                Popups->ActiveText = 1;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->ColorPicker))
            {
                if(Popups->ActiveText != 2) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->ColorPicker, FontSize);
                Popups->ActiveText = 2;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->MusicPath.Button))
            {
                if(Popups->ActiveText != 3) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->MusicPath, FontSize);
                Popups->ActiveText = 3;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Genre.Search.Button))
            {
                if(Popups->ActiveText != 6) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->SearchGenre, FontSize);
                Popups->ActiveText = 6;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Artist.Search.Button))
            {
                if(Popups->ActiveText != 7) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->SearchArtist, FontSize);
                Popups->ActiveText = 7;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Album.Search.Button))
            {
                if(Popups->ActiveText != 8) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->SearchAlbum, FontSize);
                Popups->ActiveText = 8;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Song.Base.Search.Button))
            {
                if(Popups->ActiveText != 9) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                       &Popups->Popup, Popups->SearchSong, FontSize);
                Popups->ActiveText = 9;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->LoopPlaylist, MouseP))
            {
                if(Popups->ActiveText != 10) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Loop, FontSize);
                Popups->ActiveText = 10;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->ShufflePlaylist, MouseP))
            {
                if(Popups->ActiveText != 11) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Shuffle, FontSize);
                Popups->ActiveText = 11;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlayPause, MouseP))
            {
                if(Popups->ActiveText != 12) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->PlayPause, FontSize);
                Popups->ActiveText = 12;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Stop))
            {
                if(Popups->ActiveText != 13) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Stop, FontSize);
                Popups->ActiveText = 13;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Previous))
            {
                if(Popups->ActiveText != 14) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Previous, FontSize);
                Popups->ActiveText = 14;
                Popups->IsHovering = true;
            }
            else if(IsButtonHovering(DisplayInfo->Next))
            {
                if(Popups->ActiveText != 15) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Next, FontSize);
                Popups->ActiveText = 15;
                Popups->IsHovering = true;
            }
            else if(IsInRect(DisplayInfo->Volume.GrabThing, MouseP) || IsInRect(DisplayInfo->Volume.Background, MouseP))
            {
                if(Popups->ActiveText != 16) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Volume, FontSize);
                Popups->ActiveText = 16;
                Popups->IsHovering = true;
            }
            else if(IsInRect(DisplayInfo->PlayingSongPanel.Timeline.GrabThing, MouseP) ||
                    IsInRect(DisplayInfo->PlayingSongPanel.Timeline.Background, MouseP))
            {
                if(Popups->ActiveText != 17) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Timeline, FontSize);
                Popups->ActiveText = 17;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->Help, MouseP))
            {
                if(Popups->ActiveText != 18) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->Help, FontSize);
                Popups->ActiveText = 18;
                Popups->IsHovering = true;
            }
            else if(IsInRect(DisplayInfo->Song.Base.Background, MouseP))
            {
                For(DisplayInfo->Song.Base.Count)
                {
                    if(IsButtonHovering(DisplayInfo->Song.Play[It]))
                    {
                        if(Popups->ActiveText != 4) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                               &Popups->Popup, Popups->SongPlay, FontSize);
                        Popups->ActiveText = 4;
                        Popups->IsHovering = true;
                        break;
                    }
                    else if(IsButtonHovering(DisplayInfo->Song.Add[It]))
                    {
                        if(Popups->ActiveText != 5) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                               &Popups->Popup, Popups->SongAddToNextUp, FontSize);
                        Popups->ActiveText = 5;
                        Popups->IsHovering = true;
                        break;
                    }
                }
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.Add, MouseP))
            {
                if(Popups->ActiveText != 27) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->AddPlaylist, FontSize);
                Popups->ActiveText = 27;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.AddSelection, MouseP))
            {
                if(Popups->ActiveText != 28) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->AddSelectionPlaylist, FontSize);
                Popups->ActiveText = 28;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.Rename, MouseP))
            {
                if(Popups->ActiveText != 29) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->RenamePlaylist, FontSize);
                Popups->ActiveText = 29;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->PlaylistUI.Remove, MouseP))
            {
                if(Popups->ActiveText != 30) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->RemovePlaylist, FontSize);
                Popups->ActiveText = 30;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->Playlists.Search.Button, MouseP))
            {
                if(Popups->ActiveText != 31) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->SearchPlaylists, FontSize);
                Popups->ActiveText = 31;
                Popups->IsHovering = true;
            }
        }
        else
        {
            if(IsOnButton(DisplayInfo->MusicPath.Save, MouseP))
            {
                if(Popups->ActiveText != 20) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->SaveMusicPath, FontSize);
                Popups->ActiveText = 20;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->MusicPath.Quit, MouseP))
            {
                if(Popups->ActiveText != 21) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->CancelMusicPath, FontSize);
                Popups->ActiveText = 21;
                Popups->IsHovering = true;
            }
            else if(IsOnButton(DisplayInfo->MusicPath.Rescan, MouseP))
            {
                if(Popups->ActiveText != 22) ChangeText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, 
                                                        &Popups->Popup, Popups->RescanMetadata, FontSize);
                Popups->ActiveText = 22;
                Popups->IsHovering = true;
            }
        }
        
        if(Popups->IsHovering != PrevIsHovering) SetActive(&Popups->Popup, Popups->IsHovering);
        if(Popups->IsHovering || Popups->Popup.AnimDir != 0)
        {
            SetPosition(&Popups->Popup, MouseP);
        }
        
        rect Screen = Rect(V2(0), V2(GlobalGameState.Renderer.Window.CurrentDim.Dim));
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
SetTheNewPlayingSong(renderer *Renderer, playing_song_panel *Panel, layout_definition *Layout, music_info *MusicInfo)
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
    
    string_c MissingData = NewStaticStringCompound(" -- ");
    string_c NoTime      = NewStaticStringCompound("00:00");
    
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
        if(Metadata->Artist.Pos == 0) ArtistString = &MissingData;
        else                          ArtistString = &Metadata->Artist;
        if(Metadata->Album.Pos == 0)  AlbumString  = &MissingData;
        else                          AlbumString  = &Metadata->Album;
        if(Metadata->YearString.Pos == 0) YearString = &MissingData;
        else                              YearString = &Metadata->YearString;
        if(Metadata->Genre.Pos == 0)  GenreString  = &MissingData;
        else                          GenreString  = &Metadata->Genre;
    }
    else
    {
        TitleString = TrackString = ArtistString = AlbumString = YearString = GenreString = &MissingData;
        CopyIntoCompound(&DurationString, &NoTime);
        Panel->SongDuration = 0;
    }
    
    v3 *TextColor = &Panel->MP3Info->MusicInfo->DisplayInfo.ColorPalette.ForegroundText;
    Panel->CurrentTime = 0;
    
    r32 BaseTimeX = Layout->PlayPauseButtonX + Layout->PlayPauseButtonExtents + 
        Layout->LargeButtonExtents*2*3 + Layout->ButtonGap*4;
    r32 BaseX = BaseTimeX + Layout->PlayingSongPanelXOffset;
    r32 BaseY = Layout->PlayPauseButtonY + Layout->PlayingSongPanelBaseY;
    
    string_c CurrentTime = NewStaticStringCompound("00:00 |");
    RenderText(GS, font_Small, &CurrentTime, TextColor, &Panel->CurrentTimeText, -0.6f, 0);
    SetPosition(&Panel->CurrentTimeText, V2(BaseTimeX+Layout->CurrentTimeTextXOffset, 
                                            BaseY+Layout->CurrentTimeTextYOffset));
    
    RenderText(GS, font_Small, &DurationString, TextColor, &Panel->DurationText, -0.6f, 0);
    SetPosition(&Panel->DurationText, V2(BaseTimeX + Layout->CurrentTimeTextXOffset + Layout->DurationTimeTextYOffset, 
                                         BaseY + Layout->CurrentTimeTextYOffset));
    
    RenderText(GS, font_Medium, TitleString, TextColor, &Panel->Title, -0.61f, 0);
    SetPosition(&Panel->Title, V2(BaseX + Layout->PlayingSongTextXOffset + Layout->PlayingSongTextTitleXOffset, 
                                  BaseY + Layout->PlayingSongTextTitleYOffset));
    
    RenderText(GS, font_Small, TrackString, TextColor, &Panel->Track, -0.6f, 0);
    SetPosition(&Panel->Track, V2(BaseX + Layout->PlayingSongTextXOffset, 
                                  BaseY + Layout->PlayingSongTextTrackYOffset));
    
    RenderText(GS, font_Small, ArtistString, TextColor, &Panel->Artist, -0.6f, 0);
    SetPosition(&Panel->Artist, V2(BaseX + Layout->PlayingSongTextXOffset, 
                                   BaseY + Layout->PlayingSongTextArtistYOffset));
    
    RenderText(GS, font_Small, AlbumString, TextColor, &Panel->Album, -0.6f, 0);
    SetPosition(&Panel->Album, V2(BaseX + Layout->PlayingSongTextXOffset + Layout->PlayingSongTextAlbumXOffset, 
                                  BaseY + Layout->PlayingSongTextAlbumYOffset));
    
    RenderText(GS, font_Small, YearString, TextColor, &Panel->Year, -0.6f, 0);
    SetPosition(&Panel->Year, V2(BaseX + Layout->PlayingSongTextXOffset, 
                                 BaseY + Layout->PlayingSongTextAlbumYOffset));
    
    RenderText(GS, font_Small, GenreString, TextColor, &Panel->Genre, -0.6f, 0);
    SetPosition(&Panel->Genre, V2(BaseX + Layout->PlayingSongTextXOffset, 
                                  BaseY + Layout->PlayingSongTextGenreYOffset));
}

internal void
UpdatePanelTime(renderer *Renderer, playing_song_panel *Panel, layout_definition *Layout, r32 CurrentTime)
{
    Panel->CurrentTime = (u32)(CurrentTime*1000);
    WipeStringCompound(&Panel->CurrentTimeString);
    MillisecondsToMinutes(Panel->CurrentTime, &Panel->CurrentTimeString);
    AppendStringToCompound(&Panel->CurrentTimeString, (u8 *)" |");
    
    RemoveRenderText(Renderer, &Panel->CurrentTimeText);
    RenderText(&GlobalGameState, font_Small, &Panel->CurrentTimeString, &Panel->MP3Info->MusicInfo->DisplayInfo.ColorPalette.ForegroundText, &Panel->CurrentTimeText, -0.6f, 0);
    
    r32 BaseX = Layout->PlayPauseButtonX+Layout->PlayPauseButtonExtents+Layout->LargeButtonExtents*2*3+Layout->ButtonGap*4;
    r32 BaseY = Layout->PlayPauseButtonY + Layout->PlayingSongPanelBaseY;
    SetPosition(&Panel->CurrentTimeText, V2(BaseX + Layout->CurrentTimeTextXOffset, 
                                            BaseY + Layout->CurrentTimeTextYOffset));
}

internal void
UpdatePanelTimeline(playing_song_panel *Panel, r32 CurrentTime)
{
    slider *Slider = &Panel->Timeline;
    r32 TimePercentage = Min(1.0f, SafeDiv((CurrentTime*1000), Panel->SongDuration));
    r32 NewXPos = (Slider->MaxSlidePix*2)*TimePercentage;
    Slider->GrabThing->ID->Transform.Translation.x = GetPosition(Slider->Background).x-Slider->MaxSlidePix+NewXPos;
}

struct timeline_slider_drag
{
    game_state *GameState;
    b32 PausedForDragging;
};

internal void
OnTimelineDragStart(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    if(GetIsPlayingPreload(GlobalGameState.SoundThreadInterface)) return;
    timeline_slider_drag *Info = (timeline_slider_drag *)Data;
    
    if(Info->GameState->MusicInfo.IsPlaying)
    {
        Info->GameState->MusicInfo.IsPlaying = false;
        PushSoundBufferClear(Info->GameState->SoundThreadInterface);
        Info->PausedForDragging = true;
    }
}

internal void
OnTimelineDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    if(GetIsPlayingPreload(GlobalGameState.SoundThreadInterface)) return;
    
    timeline_slider_drag *Info = (timeline_slider_drag *)Data;
    music_display_info *DisplayInfo = &Info->GameState->MusicInfo.DisplayInfo;
    slider *Slider = &DisplayInfo->PlayingSongPanel.Timeline;
    
    r32 BGX = GetPosition(Slider->Background).x;
    r32 NewX = Clamp(AdjustedMouseP.x, BGX-Slider->MaxSlidePix, BGX+Slider->MaxSlidePix);
    
    Slider->GrabThing->ID->Transform.Translation.x = NewX;
    r32 TranslationPercentage = SafeDiv(1.0f,(Slider->MaxSlidePix*2))*(NewX-BGX) + 0.5f;
    
    r32 NewPlaytime = (DisplayInfo->PlayingSongPanel.SongDuration/1000.0f)*TranslationPercentage;
    
    PushNewPlayedTime(Info->GameState->SoundThreadInterface, NewPlaytime);
}

internal void
OnTimelineDragEnd(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    if(GetIsPlayingPreload(GlobalGameState.SoundThreadInterface)) return;
    
    timeline_slider_drag *Info = (timeline_slider_drag *)Data;
    
    if(Info->PausedForDragging)
    {
        Info->GameState->MusicInfo.IsPlaying = true;
        Info->PausedForDragging = false;
    }
}

internal void
TestHoveringEdgeDrags(game_state *GameState, v2 MouseP, music_display_info *DisplayInfo)
{
    if(IsInRect(DisplayInfo->GenreArtist.Edge, MouseP) ||
       IsInRect(DisplayInfo->ArtistAlbum.Edge, MouseP) ||
       IsInRect(DisplayInfo->AlbumSong.Edge, MouseP)   ||
       IsInRect(DisplayInfo->PlaylistsGenre.Edge, MouseP))
    {
        GameState->CursorState = cursorState_Drag;
    }
    else GameState->CursorState = cursorState_Arrow;
    
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
        {
            if(Playlist->Song.Selected.A.Count > 0)
            {
                playlist_id PlaylistID = Get(&Playlist->Song.Selected, NewSelectID(0));
                i32 OnScreenID = -1;
                if(StackFind(&Playlist->Song.Displayable, PlaylistID, &OnScreenID))
                {
                    For(DisplayInfo->Song.Base.Count) 
                    {
                        if(DisplayInfo->Song.Base.OnScreenIDs[It].ID == OnScreenID)
                        {
                            OnScreenID = It;
                            break;
                        }
                    }
                    
                    music_btn BtnInfo = {&GlobalGameState, &GlobalGameState.MusicInfo.PlayingSong};
                    OnStopSong(&BtnInfo);
                    song_play_btn BtnInfo2 = {(u32)OnScreenID, &GlobalGameState};
                    OnSongPlayPressed(&BtnInfo2);
                }
                else Assert(false);
            }
            else
            {
                music_btn BtnInfo = {&GlobalGameState, &GlobalGameState.MusicInfo.PlayingSong};
                OnPlayPauseSongToggleOff(&BtnInfo);
            }
        }
        else 
        {
            if(UpdateSelectionArray(&Playlist->Song, &DisplayInfo->Song.Base, MouseBtnDownLocation))
            {
                UpdateColumnColor(&DisplayInfo->Song.Base, &Playlist->Song);
            }
        }
        
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


// **************************************
// Drag and Drop ************************
// **************************************
struct select_item { select_id SID; displayable_id DID; playlist_id PID; };

inline b32
CompareDragDrop(i32 T1, i32 T2, void *Data)
{
    select_item *Array = (select_item *)Data;
    b32 Result = (Array[T2].DID > Array[T1].DID);
    return Result;
}

inline void
SwitchDragDrop(void *Array, i32 IndexA, i32 IndexB)
{
    select_item *A = (select_item *)Array;
    select_item TMP = A[IndexB];
    A[IndexB] = A[IndexA];
    A[IndexA] = TMP;
}

internal b32
CheckSlotDragDrop(input_info *Input, game_state *GS, drag_drop *DragDrop)
{
    b32 IsInColumn = false;
    music_info *MusicInfo               = &GS->MusicInfo;
    music_display_info     *DisplayInfo = &MusicInfo->DisplayInfo;
    display_column *DisplayColumn = NULL;
    playlist_column     *PlaylistColumn = NULL;
    
    if(IsInRect(DisplayInfo->Song.Base.Background, Input->MouseP))
    {
        DisplayColumn  = &MusicInfo->DisplayInfo.Song.Base;
        PlaylistColumn = &MusicInfo->Playlist_->Song;
        IsInColumn = true;
    }
    else if(IsInRect(DisplayInfo->Album.Background, Input->MouseP))
    {
        DisplayColumn  = &DisplayInfo->Album;
        PlaylistColumn = &MusicInfo->Playlist_->Album;
        IsInColumn = true;
    }
    else if(IsInRect(DisplayInfo->Artist.Background, Input->MouseP))
    {
        DisplayColumn  = &DisplayInfo->Artist;
        PlaylistColumn = &MusicInfo->Playlist_->Artist;
        IsInColumn = true;
    }
    else if(IsInRect(DisplayInfo->Genre.Background, Input->MouseP))
    {
        DisplayColumn  = &DisplayInfo->Genre;
        PlaylistColumn = &MusicInfo->Playlist_->Genre;
        IsInColumn = true;
    }
    
    if(IsInColumn)
    {
        for(u32 It = 0; 
            It < PlaylistColumn->Displayable.A.Count && 
            It < DisplayColumn->Count;
            ++It)
        {
            if(!IsInRect(DisplayColumn->BGRects[It], Input->MouseP)) continue;
            
            // Caching all needed drag/drop information
            DragDrop->Dragging       = true;
            DragDrop->DragsSelected  = false;
            DragDrop->Type           = DisplayColumn->Type;
            DragDrop->StartMouseP    = Input->MouseP;
            DragDrop->ShakeThreshold = GetDistance(DisplayColumn->BGRects[It], DisplayInfo->Playlists.BGRects[0])/DragDrop->ShakeThresholdDistanceDivisor;
            
            drag_drop_slot *Slot = DragDrop->Slots+DragDrop->SlotCount++;
            Slot->DistToBaseSlot = 0;
            Slot->SlotID         = DisplayColumn->OnScreenIDs[It];
            Slot->SlotStartP     = GetPosition(DisplayColumn->BGRects[It]);
            
            render_text *ToCopy = 0;
            if(DisplayColumn->Type == columnType_Song) ToCopy = ColumnExt(DisplayColumn)->SongTitle+It;
            else ToCopy = DisplayColumn->Text+It;
            CopyRenderText(GS, ToCopy, &Slot->SlotText, -0.9999f);
            SetTransparency(&Slot->SlotText, 0.0f);
            
            Slot->DragSlot     = Copy(&GS->Renderer, DisplayColumn->BGRects[It]);
            Slot->TextOverhang = CalcTextOverhang(DisplayColumn->BGRects[It], *ToCopy)+20;
            SetSize(Slot->DragSlot, GetSize(Slot->DragSlot)+V2(Slot->TextOverhang, 0));
            SetDepth(Slot->DragSlot, GetDepth(Slot->SlotText.Base) + 0.0001f);
            SetTransparency(Slot->DragSlot, 0.0f);
            SetParent(Slot->DragSlot, NULL);
            
            Translate(&Slot->SlotText, V2(-Slot->TextOverhang*0.5f, 0));
            SetParent(Slot->SlotText.Base, Slot->DragSlot);
            Slot->SlotStartP.x += Slot->TextOverhang*0.5f;
            Slot->GrabOffset = (Slot->SlotStartP - DragDrop->StartMouseP);
            
            // If the grabbed slot is a selected slot, then we pull all other selected slots as well.
            // But we visualize max 5, to keep it reasonable.
            i32 ArrayP = 0;
            playlist_id PLID = Get(&PlaylistColumn->Displayable, Slot->SlotID);
            if(StackFind(&PlaylistColumn->Selected, PLID, &ArrayP))
            {
                DragDrop->DragsSelected = true;
                // Ceate array which will contain, by displayable_id, sorted 
                // pairs. With this, we then can easily go through and create
                // the grab information for the slots that are closes to the 
                // one picked with the mouse.
                u32 SelectCount = PlaylistColumn->Selected.A.Count;
                select_item *SelectIDOrder = AllocateArray(&GS->ScratchArena, PlaylistColumn->Selected.A.Count, select_item);
                RestartTimer("SortDragDrop");
                For(SelectCount, Select)
                {
                    select_id SelectID      = NewSelectID(SelectIt);
                    playlist_id SelectPLID  = Get(&PlaylistColumn->Selected, SelectID);
                    // TODO:: @SLOW:: This GetDisplayableID goes through most of diplayable every time
                    // this costs .25s in deubg mode... What can we do to reduce this?
                    displayable_id NewDID   = GetDisplayableID(&PlaylistColumn->Displayable, SelectPLID);
                    SelectIDOrder[SelectIt] = {SelectID, NewDID, SelectPLID};
                }
                
                SnapTimer("SortDragDrop");
#if 0
                QuickSort(0, SelectCount-1, SelectIDOrder, {CompareDragDrop, SelectIDOrder, SwitchDragDrop});
#else
                For(SelectCount-1, Outer) // @SLOW:: At least don't use Bubble sort...
                {
                    for(u32 InnerIt = OuterIt+1; InnerIt < SelectCount; ++InnerIt)
                    {
                        if(SelectIDOrder[OuterIt].DID > SelectIDOrder[InnerIt].DID)
                        {
                            select_item TMP = SelectIDOrder[OuterIt];
                            SelectIDOrder[OuterIt] = SelectIDOrder[InnerIt];
                            SelectIDOrder[InnerIt] = TMP;
                        }
                    }
                }
#endif
                SnapTimer("SortDragDrop");
                i32 StartIt = -1;
                i32 PickSelectIt = -1;
                r32 PositionOffsetY = GetSize(Slot->DragSlot).y + GS->Layout.SlotGap;
                i32 HalfDragSlot = Floor(MAX_DRAG_SLOT_VISUALS/2.0f);
                For(SelectCount, S) // Get the one that was mouse picked
                {
                    if(PLID == SelectIDOrder[SIt].PID) 
                    {
                        PickSelectIt = SIt;
                        if(SIt+HalfDragSlot > SelectCount) StartIt = Max(0, (i32)SelectCount-(i32)MAX_DRAG_SLOT_VISUALS);
                        else StartIt = Max(0, (i32)SIt-HalfDragSlot);
                        break;
                    }
                }
                Assert(PickSelectIt >= 0);
                Assert(StartIt      >= 0);
                
                for(u32 SelectIt = StartIt; SelectIt < Min(SelectCount, (u32)StartIt+MAX_DRAG_SLOT_VISUALS); ++SelectIt)
                {
                    if(SelectIDOrder[SelectIt].PID != PLID)
                    {
                        drag_drop_slot *SubSlot = DragDrop->Slots+DragDrop->SlotCount++;
                        SubSlot->DistToBaseSlot = Abs(PickSelectIt-(i32)SelectIt);
                        SubSlot->SlotID         = SelectIDOrder[SelectIt].DID;
                        SubSlot->SlotStartP     = V2(0, PositionOffsetY*(PickSelectIt-(i32)SelectIt));
                        
                        i32 OnScreenID = GetOnScreenID(DisplayColumn, SubSlot->SlotID);
                        r32 TextYOffset = 0;
                        if(OnScreenID >= 0)
                        {
                            render_text *ToCopy = 0;
                            if(DisplayColumn->Type == columnType_Song) 
                                ToCopy  = ColumnExt(DisplayColumn)->SongTitle+OnScreenID;
                            else ToCopy = DisplayColumn->Text+OnScreenID;
                            
                            CopyRenderText(GS, ToCopy, &SubSlot->SlotText, -0.9999f);
                        }
                        else
                        {
                            font_size_id FontSize = font_Small;
                            string_c *Text        = 0;
                            if(DisplayColumn->Type != columnType_Song)
                            {
                                Text = GetName(PlaylistColumn, SubSlot->SlotID);
                                TextYOffset = GS->Layout.SlotTextYOffset;
                            }
                            else 
                            {
                                FontSize = font_Medium;
                                Text = &GetMetadata(PlaylistColumn, &GS->MP3Info->FileInfo, SubSlot->SlotID)->Title;
                                TextYOffset = GS->Layout.SongFirstRowYOffset;
                            }
                            RenderText(GS, FontSize, Text, DisplayColumn->Colors.Text, &SubSlot->SlotText, -0.9999f);
                            
                            SetPositionX(&SubSlot->SlotText, GetPosition(&Slot->SlotText).x);
                        }
                        
                        SubSlot->DragSlot = Copy(&GS->Renderer, Slot->DragSlot);
                        SetParent(SubSlot->SlotText.Base, SubSlot->DragSlot);
                        SubSlot->TextOverhang = CalcTextOverhang(SubSlot->DragSlot, SubSlot->SlotText);
                        if(SubSlot->TextOverhang > 0) SubSlot->TextOverhang += 20;
                        SetSize(SubSlot->DragSlot,  GetSize(SubSlot->DragSlot)+V2(SubSlot->TextOverhang, 0));
                        SetDepth(SubSlot->DragSlot, GetDepth(SubSlot->SlotText.Base) + 0.0001f);
                        
                        SetTransparency(SubSlot->DragSlot, 0.0f);
                        SetTransparency(&SubSlot->SlotText, 0.0f);
                        Translate(&SubSlot->SlotText, V2(-SubSlot->TextOverhang*0.5f, TextYOffset));
                        Translate(&SubSlot->SlotText, V2(-Slot->TextOverhang*0.5f, 0));
                        
                        SubSlot->GrabOffset = (SubSlot->SlotStartP - DragDrop->StartMouseP);
                        SubSlot->SlotStartP.x += SubSlot->TextOverhang*0.5f;
                        SetParent(SubSlot->DragSlot, Slot->DragSlot);
                        SetLocalPosition(SubSlot->DragSlot, SubSlot->SlotStartP);
                    }
                }
            }
            
            // TODO:: Think about doing this always, even when we are in All playlist, as that
            // would make the behaviour similar everywhere and dropping the thing to delete
            // would do nothing.
            
            break;
        }
    }
    
    return DragDrop->Dragging;
}

internal void
CreateREMOVEVisuals(game_state *GS, drag_drop *DragDrop)
{
    music_info *MusicInfo = &GS->MusicInfo;
    // If we are in a playlist (not All), then create "Delete slot" on top of 'All'.
    if(DragDrop->OriginalAllColor == NULL &&                // To only activate it once.
       MusicInfo->Playlist_ != MusicInfo->Playlists.List+0) // Pointer compare
    {
        music_display_info *DInfo = &MusicInfo->DisplayInfo;
        
        i32 OnScreenID             = GetOnScreenID(GS, MusicInfo->Playlists.List+0);
        DragDrop->OriginalAllColor = GetColorPtr(DInfo->Playlists.BGRects[OnScreenID]);
        DragDrop->AllRenderText    = DInfo->Playlists.Text + OnScreenID;
        DragDrop->RemoveColor      = *DragDrop->OriginalAllColor;
        if(DragDrop->RemoveColor.r < 0.9f) // TODO:: Maybe rather use the original r and invert-substract it?
        {
            DragDrop->RemoveColor   += V3(0.2f, 0, 0);
            DragDrop->RemoveColor.r *= 1.5f;
        }
        else
        {
            DragDrop->RemoveColor   += V3(0, 0.2f, 0.2f);
            DragDrop->RemoveColor.g *= 1.5f;
            DragDrop->RemoveColor.b *= 1.5f;
        }
        DragDrop->RemoveColor = Clamp01(DragDrop->RemoveColor);
        SetColor(DInfo->Playlists.BGRects[OnScreenID], &DragDrop->RemoveColor);
        RemoveRenderText(&GS->Renderer, DragDrop->AllRenderText);
        NewLocalString(RemoveText, 7, "REMOVE");
        RenderText(GS, font_Small, &RemoveText, DInfo->Playlists.Colors.Text, DragDrop->AllRenderText, DInfo->Playlists.ZValue-0.01f, DInfo->Playlists.BGRects[OnScreenID]);
        Translate(DragDrop->AllRenderText, V2(0, 3));
        ResetColumnText(&DInfo->Playlists, MusicInfo->Playlist_->Playlists.Displayable.A.Count);
    }
}

internal void 
CalculateRubberDrag(drag_drop *DragDrop, r32 dTime, v2 Force)
{
    const r32 Mass    = 3;
    const r32 Tension = 1600;
    const r32 Drag    = 23;
    
    Force *= Tension;
    v2 Acceleration = (Force/Mass) - Drag*DragDrop->Velocity;
    DragDrop->Velocity = Acceleration*dTime + DragDrop->Velocity;
    // Remove small jitter when having small velocity.
    if(LengthSquared(DragDrop->Velocity) < 4) DragDrop->Velocity = V2(0);
    
    v2 P = DragDrop->Velocity*dTime + GetPosition(DragDrop->Slots[0].DragSlot);
    SetPosition(DragDrop->Slots[0].DragSlot, P);
}

internal void
DoDragDropAnim(game_state *GS, drag_drop *DragDrop)
{
    input_info *Input = &GS->Input;
    drag_drop_slot *Slot = DragDrop->Slots+0;
    r32 Dist = Distance(DragDrop->StartMouseP, Input->MouseP);
    
    if(Dist > 1) CreateREMOVEVisuals(GS, DragDrop);
    if(Dist < DragDrop->ShakeThreshold)
    {
        if(DragDrop->ShakeTransition)
        {
            DragDrop->ShakeTransition = false;
            if(Slot->Border) 
            {
                RemoveRenderEntry(&GS->Renderer, Slot->Border);
                Slot->Border = 0;
            }
        }
        
        if(DragDrop->dAnim > 1.0f || DragDrop->dAnim < -1.0f)
        {
            DragDrop->AnimDir *= -1;
            Clamp(&DragDrop->dAnim, -1.0f, 1.0f);
            DragDrop->ShakeDir = Normalize(V2(Random01()*2-1, Random01()*2-1));
        }
        else
        {
            r32 NDist = Dist/DragDrop->ShakeThreshold;
            r32 RDist = Root2_8(NDist);
            
            r32 ShakeSpeed = SafeRatio1(DragDrop->ShakeMaxAnimTime, RDist);
            DragDrop->dAnim += GS->Time.dTime/ShakeSpeed * DragDrop->AnimDir;
            
            r32 dMove = RDist*DragDrop->dAnim*DragDrop->ShakeMaxRadius;
            
            // Calculates position NDist percentage between the original and mouse position, with regards to the grab offset.
            v2 OriP = Input->MouseP*NDist - (Slot->SlotStartP-Slot->GrabOffset)*NDist + Slot->SlotStartP;
            //v2 OriP = ((Slot->SlotStartP-Slot->GrabOffset) + (Input->MouseP - (Slot->SlotStartP-Slot->GrabOffset))*NDist)+Slot->GrabOffset; // Old, more verbose
            SetPosition(Slot->DragSlot, V2(OriP.x + (dMove*DragDrop->ShakeDir.x), OriP.y + (dMove*DragDrop->ShakeDir.y)));
            
            For(DragDrop->SlotCount)
            {
                drag_drop_slot *Slot = DragDrop->Slots + It;
                r32 Transparency = RDist*DragDrop->MaxTransparency - (Slot->DistToBaseSlot*DragDrop->TransparencyFalloff);
                
                SetTransparency(Slot->DragSlot, Transparency);
                SetTransparency(&Slot->SlotText, Transparency);
            }
        }
    }
    else
    {
        v2 DragDir = Input->MouseP - GetPosition(Slot->DragSlot) + Slot->GrabOffset;
        
        if(!DragDrop->ShakeTransition)
        {
            DragDrop->ShakeTransition = true;
            SetTransparency(Slot->DragSlot, DragDrop->MaxTransparency*0.9f);
            SetTransparency(&Slot->SlotText, DragDrop->MaxTransparency*0.9f);
            Slot->Border = Copy(&GS->Renderer, Slot->DragSlot);
            SetDepth(Slot->Border, GetDepth(Slot->DragSlot) + 0.0001f);
            SetColor(Slot->Border, &GS->MusicInfo.DisplayInfo.ColorPalette.Text);
            SetLocalPosition(Slot->Border, {});
            SetSize(Slot->Border, GetSize(Slot->Border)+V2(6, 6));
            SetTransparency(Slot->Border, DragDrop->MaxTransparency*0.5f);
            SetParent(Slot->Border, Slot->DragSlot);
            
            // Add strong bounce at beginning of rubber banding.
            DragDir = Input->MouseP - Slot->SlotStartP + Slot->GrabOffset;
            DragDir *= 5;
        }
        CalculateRubberDrag(DragDrop, GS->Time.dTime, DragDir);
    }
}


internal void
EvalDragDropPosition(game_state *GS, drag_drop *DragDrop)
{
    music_display_info *DisplayInfo = &GS->MusicInfo.DisplayInfo;
    display_column   *DisplayColumn = &DisplayInfo->Playlists;
    entry_id                  *Slot = DragDrop->Slots[0].DragSlot;
    
    b32 InSlot = false;
    if(IsIntersectingRect(Slot, DisplayColumn->Background))
    {
        playlist_column *PlaylistColumn = &GS->MusicInfo.Playlist_->Playlists;
        
        array_u32 IntersectingSlots = CreateArray(&GS->ScratchArena, Min(PlaylistColumn->Displayable.A.Count, DisplayColumn->Count));
        For(IntersectingSlots.Length)
        {
            if(IsIntersectingRect(Slot, DisplayColumn->BGRects[It])) 
                Push(&IntersectingSlots, It);
        }
        
        i32 ClosestID = -1;
        r32   MinDist = MAX_REAL32;
        v2      SlotP = GetPosition(Slot);
        For(IntersectingSlots.Count)
        {
            u32 ID = Get(&IntersectingSlots, It);
            r32 Dist = Distance(SlotP, GetPosition(DisplayColumn->BGRects[ID]));
            if(Dist < MinDist)
            {
                MinDist   = Dist;
                ClosestID = ID;
            }
        }
        if(ClosestID >= 0)
        {
            if(DragDrop->CurHoverID >= 0) 
                SetColor(DisplayColumn->BGRects[DragDrop->CurHoverID], DragDrop->CurOriginalColor); 
            
            DragDrop->CurHoverID = ClosestID;
            DragDrop->CurOriginalColor = GetColorPtr(DisplayColumn->BGRects[ClosestID]);
            SetColor(DisplayColumn->BGRects[ClosestID], &DisplayInfo->ColorPalette.Text);
            
            InSlot = true;
        }
    }
    if(!InSlot && DragDrop->CurHoverID >= 0)
    {
        SetColor(DisplayColumn->BGRects[DragDrop->CurHoverID], DragDrop->CurOriginalColor); 
        DragDrop->CurHoverID = -1;
    }
}

internal void
StopDragDrop(game_state *GS, drag_drop *DragDrop)
{
    // Evaluate drop
    if(DragDrop->CurHoverID >= 0)
    {
        SetColor(GS->MusicInfo.DisplayInfo.Playlists.BGRects[DragDrop->CurHoverID], DragDrop->CurOriginalColor); 
        
        playlist_id PlaylistID = Get(&GS->MusicInfo.Playlist_->Playlists.Displayable,
                                     GS->MusicInfo.DisplayInfo.Playlists.OnScreenIDs[DragDrop->CurHoverID]);
        playlist_info *Playlist = GS->MusicInfo.Playlists.List + PlaylistID.ID;
        
        if(DragDrop->CurHoverID == 0) // 'All' playlist removes slots from playlists...
        {
            if(Playlist != GS->MusicInfo.Playlist_) // Except when were in itself, then nothing happens.
                RemoveSlotFromPlaylist(GS, DragDrop->Type, DragDrop->Slots[0].SlotID);
        }
        else if(!DragDrop->DragsSelected) InsertSlotIntoPlaylist(GS, Playlist, DragDrop->Type, DragDrop->Slots[0].SlotID);
        else
        {
            // If we have a selection drop, we need to insert
            // everything that was previously selected in this
            // column into the playlist.
            array_playlist_id *Selected = &GS->MusicInfo.Playlist_->Columns[DragDrop->Type].Selected;
            array_u32 DisplayableIDs = CreateArray(&GS->ScratchArena, Selected->A.Count);
            
            For(Selected->A.Count)
            {
                displayable_id DID = GetDisplayableID(&GS->MusicInfo.Playlist_->Columns[DragDrop->Type].Displayable, 
                                                      Get(Selected, NewSelectID(It)));
                Push(&DisplayableIDs, DID.ID);
            }
            InsertSlotsIntoPlaylist(GS, Playlist, DragDrop->Type, DisplayableIDs);
        }
        
        DragDrop->CurHoverID = -1;
    }
    
    // Remove all the entries and stuff.
    if(DragDrop->Slots[0].Border) 
    {
        RemoveRenderEntry(&GS->Renderer, DragDrop->Slots[0].Border);
        DragDrop->Slots[0].Border = 0;
    }
    For(DragDrop->SlotCount)
    {
        RemoveRenderEntry(&GS->Renderer, DragDrop->Slots[It].DragSlot);
        RemoveRenderText(&GS->Renderer, &DragDrop->Slots[It].SlotText);
    }
    DragDrop->SlotCount = 0;
    
    if(DragDrop->OriginalAllColor)
    {
        display_column *DColumn = &GS->MusicInfo.DisplayInfo.Playlists;
        i32 OnScreenID = GetOnScreenID(GS, GS->MusicInfo.Playlists.List+0);
        
        RemoveRenderText(&GS->Renderer, DragDrop->AllRenderText);
        RenderText(GS, font_Small, GS->MusicInfo.Playlist_->Playlists.Batch.Names+0, DColumn->Colors.Text, DragDrop->AllRenderText, DColumn->ZValue-0.01f, DColumn->BGRects[OnScreenID]);
        Translate(DragDrop->AllRenderText, V2(0, 3));
        ResetColumnText(DColumn, GS->MusicInfo.Playlist_->Playlists.Displayable.A.Count);
        
        SetColor(DColumn->BGRects[OnScreenID], DragDrop->OriginalAllColor);
        
        DragDrop->OriginalAllColor = NULL;
        DragDrop->AllRenderText    = NULL;
    }
    DragDrop->Dragging = false;
}

internal void
HandlePlaylistButtonAnimation(game_state *GS, button *Btn, quit_animation *Anim, playlist_btn_type BtnType)
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



