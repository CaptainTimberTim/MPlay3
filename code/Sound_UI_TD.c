#include "Sound_UI_TD.h"
inline file_id GetNextSong(play_list *Playlist, playing_song *PlayingSong);
inline playlist_id GetSongAfterCurrent(play_list *Playlist, playlist_id PlaylistID);
inline playlist_id GetPreviousSong(play_list *Playlist, playlist_id PlaylistID);
inline void SetPreviousSong(play_list *Playlist, playing_song *PlayingSong, play_loop Looping);
inline file_id PlaylistIDToFileID(play_list *Playlist, playlist_id PlaylistID);
internal void MillisecondsToMinutes(memory_bucket_container *Bucket, u32 Millis, string_c *Out);
internal void MillisecondsToMinutes(u32 Millis, string_c *Out);
internal void ChangeDisplayColumn_(music_display_column *DisplayColumn, renderer *Renderer, column_sorting_info *SortingColumn,
                                   i32 FileStartID, r32 StartY);
internal void MoveDisplayColumn(renderer *Renderer, music_display_column *DisplayColumn, 
                                displayable_id DisplayableStartID = {0}, r32 StartY = 0);
internal void SortDisplayables(music_sorting_info *SortingInfo, mp3_file_info *MP3FileInfo);
internal b32 FindAllMP3FilesInFolder(memory_bucket_container *FixedBucket, memory_bucket_container *TransientBucket,
                                     string_compound *FolderPath, string_compound *SubPath, mp3_file_info *ResultingFileInfo);
inline mp3_info *CreateMP3InfoStruct(memory_bucket_container *Bucket, u32 FileInfoCount);
inline mp3_file_info CreateFileInfoStruct(memory_bucket_container *Bucket, u32 FileInfoCount);
inline void DeleteFileInfoStruct(memory_bucket_container *Bucket, mp3_file_info *FileInfo);
inline void ReplaceFolderPath(mp3_info *MP3Info, string_c *NewPath);
inline void SetSelectionArray(music_display_column *DisplayColumn, column_sorting_info *SortingColumn, u32 ColumnDisplayID);
internal void UpdateSelectionChanged(renderer *Renderer, music_info *MusicInfo, mp3_info *MP3Info, column_type Type);
inline displayable_id FileIDToSongDisplayableID(music_display_column *DisplayColumn, file_id FileID);
inline displayable_id SortingIDToColumnDisplayID(music_display_column *DisplayColumn, batch_id BatchID);
inline playlist_id OnScreenIDToPlaylistID(music_info *MusicInfo, u32 OnScreenID, file_id *FileID = 0);
inline void HandleChangeToNextSong(game_state *GameState);
internal void AddJobs_LoadOnScreenMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs = 0);
internal void AddJobs_LoadMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs = 0);
internal b32 AddJob_LoadMetadata(game_state *GameState);
internal void UseDisplayableAsPlaylist(music_info *MusicInfo);
internal column_type UpdateSelectionArray(column_sorting_info *SortingColumn, music_display_column *DisplayColumn);
internal column_type SelectAllOrNothing(music_display_column *DisplayColumn, column_sorting_info *SortingColumn);
internal void PropagateSelectionChange(music_sorting_info *SortingInfo);

inline u32 
CountPossibleDisplayedSlots(renderer *Renderer, music_display_column *DisplayColumn)
{
    u32 Result = 0;
    r32 Height = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    Result = Ceiling(Height/DisplayColumn->SlotHeight)+1;
    return Result;
}

inline void
CreateDisplayBackgroundRect(renderer *Renderer, music_display_column *DisplayColumn, u32 ID, r32 ZValue, entry_id *Parent)
{
    v2 HalfDim = V2(DisplayColumn->SlotWidth, DisplayColumn->SlotHeight-SLOT_DISTANCE_PIXEL)/2;
    DisplayColumn->BGRects[ID] = CreateRenderRect(Renderer, {-HalfDim, HalfDim}, ZValue, Parent,
                                                  &DisplayColumn->Base->ColorPalette.Slot);
    r32 YDown = (ID > 0) ? -DisplayColumn->SlotHeight : 0;
    SetLocalPosition(DisplayColumn->BGRects[ID], V2(0, YDown));
}

inline r32
GetInitialYForDisplayColumnSlot(music_display_column *DisplayColumn)
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
    music_display_column *SongColumn = &MusicInfo->DisplayInfo.Song.Base;
    b32 *IsPlaying = &MusicInfo->IsPlaying;
    
    if(!IsInRect(SongColumn->Background, GlobalGameState.Input.MouseP)) return;
    
    UseDisplayableAsPlaylist(MusicInfo);
    
    file_id FileID = {};
    playlist_id PlaylistID = OnScreenIDToPlaylistID(MusicInfo, PlayInfo->DisplayID, &FileID);
    //playlist_id PlaylistID = {SongColumn->OnScreenIDs[PlayInfo->DisplayID].ID}; // TODO::PLAYLIST_DISPLAYABLE
    
    if(MusicInfo->PlayingSong.PlaylistID != PlaylistID)
    {
        *IsPlaying = true;
        MusicInfo->PlayingSong.PlaylistID = PlaylistID;
        MusicInfo->PlayingSong.FileID = FileID; //PlaylistIDToFileID(&MusicInfo->Playlist, PlaylistID);
        if(MusicInfo->PlayingSong.PlayUpNext)
        {
            MusicInfo->PlayingSong.PlayUpNext = false; 
            Take(&MusicInfo->Playlist.UpNext, NewPlaylistID(0));
        }
        ChangeSong(PlayInfo->GameState, &MusicInfo->PlayingSong);
        
        AddJob_LoadMP3(PlayInfo->GameState, &PlayInfo->GameState->JobQueue,
                       PlaylistIDToFileID(&MusicInfo->Playlist, PlaylistID));
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
    music_display_column *SongColumn = &MusicInfo->DisplayInfo.Song.Base;
    
    if(!IsInRect(SongColumn->Background, GlobalGameState.Input.MouseP)) return;
    
    file_id FileID = Get(&MusicInfo->Playlist.Songs, NewPlaylistID(SongColumn->OnScreenIDs[PlayInfo->DisplayID]));
    Push(&MusicInfo->Playlist.UpNext, FileID);
    if(MusicInfo->Playlist.UpNext.A.Count < MAX_MP3_DECODE_COUNT-1)
    {
        AddJob_LoadMP3(PlayInfo->GameState, &PlayInfo->GameState->JobQueue, FileID);
    }
}

inline void
CreateSongButtons(renderer *Renderer, display_column_song_extension *SongColumn, u32 ID)
{
    u32 ButtonID = SongColumn->Base.Base->PlayPause->Entry->ID->TexID;
    u32 PlayID = SongColumn->Base.Base->PlayPause->Icon->ID->TexID;
    
    color_palette *Palette = &SongColumn->Base.Base->ColorPalette;
    
    r32 Z = SongColumn->Base.ZValue - 0.01f;
    
    r32 HalfSize = 22;
    rect Rect = {{-HalfSize, -HalfSize}, {HalfSize, HalfSize}};
    SongColumn->Play[ID] = NewButton(Renderer, Rect, Z, false, 
                                     ButtonID, &Palette->SliderGrabThing, 
                                     &Palette->ButtonActive, &Palette->SliderBackground, 
                                     PlayID, &Palette->Text, SongColumn->Base.BGRects[ID]);
    Translate(SongColumn->Play[ID], V2(0, SONG_PLAY_BUTTON_Y_OFFSET));
    SongColumn->PlayBtnData[ID] = {ID, &GlobalGameState};
    SongColumn->Play[ID]->OnPressed = {OnSongPlayPressed, &SongColumn->PlayBtnData[ID]};
    
    SongColumn->Add[ID] = NewButton(Renderer, Rect, Z, false, 
                                    ButtonID, &Palette->SliderGrabThing, 
                                    &Palette->ButtonActive, &Palette->SliderBackground, 
                                    SongColumn->AddGLID, &Palette->Text, SongColumn->Play[ID]->Entry);
    Translate(SongColumn->Add[ID], V2(52, 0));
    
    SongColumn->Add[ID]->OnPressed = {OnSongAddPressed, &SongColumn->PlayBtnData[ID]};
}

inline void
SetSongButtonsActive(display_column_song_extension *SongColumn, u32 ID, b32 IsActive)
{
    if(SongColumn->Play[ID]) SetActive(SongColumn->Play[ID], IsActive);
    if(SongColumn->Add[ID]) SetActive(SongColumn->Add[ID], IsActive);
}

internal void
CreateDisplayColumnSong(renderer *Renderer, display_column_song_extension *SongColumn)
{
    For(SongColumn->Base.Count)
    {
        CreateSongButtons(Renderer, SongColumn, It);
    }
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
    
    if(DisplayInfo->Genre.Search.TextField.IsActive) Result = columnType_Genre;
    else if(DisplayInfo->Artist.Search.TextField.IsActive) Result = columnType_Artist;
    else if(DisplayInfo->Album.Search.TextField.IsActive) Result = columnType_Album;
    else if(DisplayInfo->Song.Base.Search.TextField.IsActive) Result = columnType_Song;
    
    return Result;
}

inline void
CopyBackDisplayable(music_display_column *DisplayColumn)
{
    Copy(DisplayColumn->SearchInfo.Displayable, &DisplayColumn->Search.InitialDisplayables);
    MoveDisplayColumn(DisplayColumn->SearchInfo.Renderer, DisplayColumn);
}

internal void
OnSearchPressed(void *Data)
{
    search_bar_btn_info *SearchInfo = (search_bar_btn_info *)Data;
    search_bar *Search = SearchInfo->Search;
    
    if(SearchInfo->DisplayInfo->SearchIsActive < 0) 
    {
        SearchInfo->DisplayInfo->SearchIsActive = SearchInfo->DisplayColumn->Type;
        UpdateTextField(SearchInfo->Renderer, &Search->TextField);
        Copy(&Search->InitialDisplayables, SearchInfo->Displayable);
    }
    else
    {
        if(SearchInfo->DisplayInfo->SearchIsActive == SearchInfo->DisplayColumn->Type) 
        {
            CopyBackDisplayable(SearchInfo->DisplayColumn);
            SearchInfo->DisplayInfo->SearchIsActive = -1;
            RemoveRenderText(&Search->TextField.Text);
        }
        else 
        {
            // Deactivating previous search.
            if(SearchInfo->DisplayInfo->SearchIsActive == columnType_Genre)
            {
                CopyBackDisplayable(&SearchInfo->DisplayInfo->Genre);
                OnSearchPressed(&SearchInfo->DisplayInfo->Genre.SearchInfo);
            }
            else if(SearchInfo->DisplayInfo->SearchIsActive == columnType_Artist)
            {
                CopyBackDisplayable(&SearchInfo->DisplayInfo->Artist);
                OnSearchPressed(&SearchInfo->DisplayInfo->Artist.SearchInfo);
            }
            else if(SearchInfo->DisplayInfo->SearchIsActive == columnType_Album)
            {
                CopyBackDisplayable(&SearchInfo->DisplayInfo->Album);
                OnSearchPressed(&SearchInfo->DisplayInfo->Album.SearchInfo);
            }
            else if(SearchInfo->DisplayInfo->SearchIsActive == columnType_Song)
            {
                CopyBackDisplayable(&SearchInfo->DisplayInfo->Song.Base);
                OnSearchPressed(&SearchInfo->DisplayInfo->Song.Base.SearchInfo);
            }
            
            SearchInfo->DisplayInfo->SearchIsActive = SearchInfo->DisplayColumn->Type;
            UpdateTextField(SearchInfo->Renderer, &Search->TextField);
            Copy(&Search->InitialDisplayables, SearchInfo->Displayable);
        }
        UseDisplayableAsPlaylist(&GlobalGameState.MusicInfo);
        UpdateColumnVerticalSlider(SearchInfo->Renderer, SearchInfo->DisplayColumn, SearchInfo->SortingInfo);
    }
    
    SetTextFieldActive(&Search->TextField, !Search->TextField.IsActive);
    
    ResetStringCompound(Search->TextField.TextString);
    Search->TextField.dBackspacePress = 0;
}

inline void
ResetSearchList(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *ColumnSortInfo)
{
    Copy(DisplayColumn->SearchInfo.Displayable, &DisplayColumn->Search.InitialDisplayables);
    MoveDisplayColumn(Renderer, DisplayColumn);
    OnSearchPressed(&DisplayColumn->SearchInfo);
}

internal void
InterruptSearch(renderer *Renderer, music_display_info *DisplayInfo, music_sorting_info *SortInfo)
{
    music_display_column *DisplayColumn = 0;
    column_sorting_info  *SortColumn    = 0;
    switch(GetOpenSearch(DisplayInfo))
    {
        case columnType_Genre: 
        {
            DisplayColumn = &DisplayInfo->Genre;
            SortColumn    = &SortInfo->Genre;
        } break;
        case columnType_Artist: 
        {
            DisplayColumn = &DisplayInfo->Artist; 
            SortColumn    = &SortInfo->Artist;
        } break;
        case columnType_Album: 
        {
            DisplayColumn = &DisplayInfo->Album;  
            SortColumn    = &SortInfo->Album;
        } break;
        case columnType_Song: 
        {
            DisplayColumn = &DisplayInfo->Song.Base; 
            SortColumn    = &SortInfo->Song;
        } break;
    }
    if(DisplayColumn)
    {
        ResetSearchList(Renderer, DisplayColumn, SortColumn);
    }
}

inline void
ProcessActiveSearch(renderer *Renderer, music_display_column *DisplayColumn, r32 dTime, input_info *Input,
                    column_sorting_info *ColumnSortInfo, mp3_file_info *FileInfo = 0)
{
    text_field_flag_result FieldResult = ProcessTextField(Renderer, dTime, Input, &DisplayColumn->Search.TextField);
    
    if(FieldResult.Flag & processTextField_TextChanged)
    {
        SearchInDisplayable(ColumnSortInfo, &DisplayColumn->Search, FileInfo);
        MoveDisplayColumn(Renderer, DisplayColumn);
        UpdateColumnVerticalSlider(Renderer, DisplayColumn, ColumnSortInfo);
    }
    if(FieldResult.Flag & processTextField_Confirmed)
    {
        if(DisplayColumn->SearchInfo.Displayable->A.Count == 1) 
        {
            SetSelectionArray(DisplayColumn, ColumnSortInfo, 0);
            UpdateColumnColor(DisplayColumn, ColumnSortInfo);
        }
        ResetSearchList(Renderer, DisplayColumn, ColumnSortInfo);
        UpdateSelectionChanged(Renderer, &GlobalGameState.MusicInfo, GlobalGameState.MP3Info, DisplayColumn->Type);
        
        file_id SelectedID = Get(&ColumnSortInfo->Selected, NewSelectID(0));
        if(DisplayColumn->Type == columnType_Song) BringDisplayableEntryOnScreen(DisplayColumn, SelectedID);
        else BringDisplayableEntryOnScreenWithSortID(DisplayColumn, SelectedID);
    }
}

internal search_bar
CreateSearchBar(renderer *Renderer, memory_bucket_container *Bucket, music_display_info *DisplayInfo, music_display_column *DisplayColumn, entry_id *Parent, column_sorting_info *SortingInfo)
{
    search_bar Result = {};
    r32 BtnDepth        = -0.6f;
    r32 SearchExt       = 12;
    
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Search_Icon.png"); 
    u32 SearchID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    
    Result.Button = NewButton(Renderer, {{-SearchExt, -SearchExt},{SearchExt, SearchExt}}, 
                              BtnDepth, false, Renderer->ButtonBaseID, 
                              &DisplayInfo->ColorPalette.SliderGrabThing, 
                              &DisplayInfo->ColorPalette.ButtonActive, 
                              &DisplayInfo->ColorPalette.SliderBackground, SearchID,
                              &DisplayInfo->ColorPalette.Text, Parent);
    Translate(Result.Button, V2(1, -1));
    DisplayColumn->SearchInfo = 
    {
        Renderer, DisplayInfo, &DisplayColumn->Search, DisplayColumn, 
        &SortingInfo->Displayable, SortingInfo
    };
    Result.Button->OnPressed = {OnSearchPressed, &DisplayColumn->SearchInfo};
    
    Result.InitialDisplayables.A = CreateArray(Bucket, SortingInfo->Displayable.A.Length);
    
    v2 TextFieldSize = V2(DisplayColumn->SlotWidth/2.0f, 50);
    Result.TextField = CreateTextField(Renderer, Bucket, TextFieldSize, 
                                       DisplayColumn->ZValue-0.029f, (u8 *)"Search...", DisplayColumn->SliderHorizontal.Background, 
                                       &DisplayInfo->ColorPalette.Text, &DisplayInfo->ColorPalette.ButtonActive);
    Translate(&Result.TextField, V2(0, 25+GetExtends(DisplayColumn->SliderHorizontal.Background).y));
    
    return Result;
}

internal void
CreateDisplayColumn(memory_bucket_container *Bucket, renderer *Renderer, music_display_info *DisplayInfo,
                    music_display_column *DisplayColumn, column_sorting_info *SortingInfo, column_type Type,
                    r32 SlotHeight, r32 ZValue, 
                    entry_id *LeftBorder, entry_id *RightBorder, r32 TextLeftBorderOffset)
{
    // NOTE:: This should only happen at the very beginning, before a window resize
    DisplayColumn->Base       = DisplayInfo;
    DisplayColumn->Type       = Type;
    DisplayColumn->SlotHeight = SlotHeight;
    //DisplayColumn->SlotWidth  = SlotWidth;
    DisplayColumn->ZValue     = ZValue;
    DisplayColumn->LeftBorder = LeftBorder;
    DisplayColumn->TopBorder  = DisplayInfo->EdgeTop;
    // NOTE:: Right and bottom border is a slider, therefore set later in procedure
    DisplayColumn->TextX      = TextLeftBorderOffset;
    DisplayColumn->SortingInfo= SortingInfo;
    
    color_palette *Palette = &DisplayColumn->Base->ColorPalette;
    
    // Creating horizontal slider 
    r32 SliderHoriHeight = 26;
    r32 SliderVertWidth = SliderHoriHeight;
    DisplayColumn->SlotWidth = (GetRect(RightBorder).Min.x - SliderVertWidth) - GetRect(LeftBorder).Max.x;
    
    v2 BottomLeft = V2(GetRect(LeftBorder).Max.x, GetRect(DisplayInfo->EdgeBottom).Max.y);
    DisplayColumn->SliderHorizontal.Background = 
        CreateRenderRect(Renderer, {BottomLeft, BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight)},
                         DisplayColumn->ZValue-0.03f, 0, &Palette->SliderBackground);
    
    DisplayColumn->SliderHorizontal.GrabThing  = 
        CreateRenderRect(Renderer, {BottomLeft+V2(0, 4), BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight-4)},
                         DisplayColumn->ZValue-0.031f, 0, &Palette->SliderGrabThing);
    
    
    DisplayColumn->BottomBorder = DisplayColumn->SliderHorizontal.Background;
    r32 ColumnHeight = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    DisplayColumn->ColumnHeight = ColumnHeight;
    
    // Creating vertical slider
    v2 BottomLeft2 = BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight);
    v2 VertSliderExtends = V2(SliderVertWidth, ColumnHeight)*0.5f;
    DisplayColumn->SliderVertical.Background = 
        CreateRenderRect(Renderer, {-VertSliderExtends, VertSliderExtends}, DisplayColumn->ZValue-0.0311f,
                         RightBorder, &Palette->SliderBackground);
    SetPosition(DisplayColumn->SliderVertical.Background, BottomLeft2+VertSliderExtends);
    
    DisplayColumn->SliderVertical.GrabThing  = 
        CreateRenderRect(Renderer, {-VertSliderExtends+V2(4, 0), VertSliderExtends-V2(4, 0)},
                         DisplayColumn->ZValue-0.0312f, RightBorder, &Palette->SliderGrabThing);
    SetPosition(DisplayColumn->SliderVertical.GrabThing, BottomLeft2+VertSliderExtends);
    
    DisplayColumn->RightBorder = DisplayColumn->SliderVertical.Background;
    
    // Creating Slot background and slots
    rect BGRect = {BottomLeft+V2(0, SliderHoriHeight), BottomLeft+V2(DisplayColumn->SlotWidth, SliderHoriHeight+ColumnHeight)};
    DisplayColumn->Background = CreateRenderRect(Renderer, BGRect, ZValue+0.01f, 0, 
                                                 &DisplayColumn->Base->ColorPalette.Foreground);
    
    DisplayColumn->Count = CountPossibleDisplayedSlots(Renderer, DisplayColumn);
    
    DisplayColumn->BGRectAnchor = CreateRenderRect(Renderer, V2(0), 0, &Palette->Foreground, 0);
    r32 AnchorY = Renderer->Window.CurrentDim.Height - GetSize(DisplayInfo->EdgeTop).y - SlotHeight/2.0f;
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
        Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Add_Icon.png"); 
        ColumnExt(DisplayColumn)->AddGLID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
        
        CreateDisplayColumnSong(Renderer, ColumnExt(DisplayColumn));
    }
    
    
    // Creating Search bar and miscelanious
    rect BetweenRect = {{-13, -13},{13, 13}};
    DisplayColumn->BetweenSliderRect = CreateRenderRect(Renderer, BetweenRect, -0.5f, RightBorder, 
                                                        &Palette->Foreground);
    
    DisplayColumn->Search = CreateSearchBar(Renderer, Bucket, DisplayInfo, DisplayColumn, DisplayColumn->BetweenSliderRect, 
                                            SortingInfo);
    
    
}

internal r32
CalcTextOverhangPercentage(renderer *Renderer, music_display_column *DisplayColumn, render_text *Text)
{
    r32 Result = 0;
    
    r32 MaxOverhang = 0.0f;
    for(u32 It = 0;
        It < DisplayColumn->Count && 
        It < DisplayColumn->SortingInfo->Displayable.A.Count;
        It++)
    {
        if(Text[It].Count > 0) 
        {
            v2 TextT = GetPosition(&Text[It], Text[It].Count-1);
            v2 TextP = TextT+V2(15,0);
            r32 Overhang = DistanceToRectEdge(DisplayColumn->BGRects[It], TextP);
            if(Overhang > MaxOverhang)
            {
                MaxOverhang = Overhang;
            }
        }
    }
    
    Result = (1.0f/DisplayColumn->SlotWidth) * MaxOverhang;
    
    return Result;
}

internal r32
CalcSongTextOverhangPercentage(renderer *Renderer, music_display_column *DisplayColumn)
{
    r32 Result = 0;
    display_column_song_extension *Song = ColumnExt(DisplayColumn);
    
    r32 TitleOverhang = CalcTextOverhangPercentage(Renderer, DisplayColumn, Song->SongTitle);
    r32 ArtistOverhang = CalcTextOverhangPercentage(Renderer, DisplayColumn, Song->SongArtist);
    r32 AlbumOverhang = CalcTextOverhangPercentage(Renderer, DisplayColumn, Song->SongAlbum);
    
    Result = Max(TitleOverhang, Max(ArtistOverhang, AlbumOverhang));
    
    return Result;
}


internal void
ResetColumnText(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *SortingColumn)
{
    for(u32 It = 0;
        It < DisplayColumn->Count &&
        It < SortingColumn->Displayable.A.Count; 
        It++)
    {
        
        r32 TextX = GetPosition(DisplayColumn->LeftBorder).x + DisplayColumn->TextX;
        if(DisplayColumn->Type == columnType_Song)
        {
            SetPositionX(ColumnExt(DisplayColumn)->SongTitle+It, TextX+SONG_TITLE_X_OFFSET);
            SetPositionX(ColumnExt(DisplayColumn)->SongArtist+It, TextX+SONG_ARTIST_X_OFFSET);
            SetPositionX(ColumnExt(DisplayColumn)->SongAlbum+It, TextX+SONG_ALBUM_X_OFFSET);
            SetPositionX(ColumnExt(DisplayColumn)->SongTrack+It, TextX+SONG_TRACK_X_OFFSET);
            SetPositionX(ColumnExt(DisplayColumn)->SongYear+It, TextX+SONG_ARTIST_X_OFFSET);
            
            SetActive(ColumnExt(DisplayColumn)->SongTitle+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongArtist+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongAlbum+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongGenre+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongTrack+It, true);
            SetActive(ColumnExt(DisplayColumn)->SongYear+It, true);
            
            r32 NewBtnX = GetPosition(DisplayColumn->LeftBorder).x;
            SetPositionX(ColumnExt(DisplayColumn)->Play[It]->Entry, NewBtnX+SONG_PLAY_BUTTON_X_OFFSET);
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
UpdateColumnHorizontalSlider(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *SortingColumn)
{
    slider *Slider = &DisplayColumn->SliderHorizontal;
    
    ResetColumnText(Renderer, DisplayColumn, SortingColumn);
    
    if(DisplayColumn->Type == columnType_Song) 
    {
        Slider->OverhangP = 1 + CalcSongTextOverhangPercentage(Renderer, DisplayColumn);
    }
    else Slider->OverhangP = 1 + CalcTextOverhangPercentage(Renderer, DisplayColumn, DisplayColumn->Text);
    
    r32 Scale      = GetSize(Slider->Background).x;
    r32 NewScale   = Max(Scale/Slider->OverhangP, 5.0f);
    r32 PixelScale = Scale-NewScale;
    
    SetSize(Slider->GrabThing, V2(NewScale, GetSize(Slider->GrabThing).y));
    Slider->GrabThing->ID->Transform.Translation.x = Slider->Background->ID->Transform.Translation.x - PixelScale/2;
    Slider->MaxSlidePix = PixelScale/2;
}

inline void
UpdateHorizontalSliders(renderer *Renderer, music_display_info *DisplayInfo, music_sorting_info *SortingInfo)
{
    UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Genre, &SortingInfo->Genre);
    UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Artist, &SortingInfo->Artist);
    UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Album, &SortingInfo->Album);
    UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Song.Base, &SortingInfo->Song);
}

inline void
UpdateOnHorizontalSliderDrag(renderer *Renderer, music_display_column *DisplayColumn, r32 TranslationX, u32 It, 
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
SongHorizontalSliderDrag(renderer *Renderer, music_display_column *DisplayColumn, r32 TranslationX, u32 It)
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
OnSliderDragStart(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    drag_slider_data *Info = (drag_slider_data *)Data;
    
    Info->MouseOffset.x = AdjustedMouseP.x - GetPosition(Info->DisplayColumn->SliderHorizontal.GrabThing).x;
    Info->MouseOffset.y = AdjustedMouseP.y - GetPosition(Info->DisplayColumn->SliderVertical.GrabThing).y;
}

internal void
OnHorizontalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    drag_slider_data *Info = (drag_slider_data *)Data;
    music_display_column *DisplayColumn = Info->DisplayColumn;
    slider *Slider = &DisplayColumn->SliderHorizontal;
    r32 GrabThingHalfWidth  = GetSize(Slider->GrabThing).x/2.0f;
    if(Info->MouseOffset.x < GrabThingHalfWidth && Info->MouseOffset.x > -GrabThingHalfWidth) 
        AdjustedMouseP.x -= Info->MouseOffset.x;
    
    r32 BGXPos = GetLocalPosition(Slider->Background).x;
    r32 NewX   = Clamp(AdjustedMouseP.x, BGXPos - Slider->MaxSlidePix, BGXPos + Slider->MaxSlidePix);
    r32 OldX   = GetLocalPosition(Slider->GrabThing).x;
    SetLocalPosition(Slider->GrabThing, V2(NewX, GetLocalPosition(Slider->GrabThing).y));
    
    r32 TranslationPercent = SaveDiv(1.0f, Slider->MaxSlidePix*2)*(OldX-NewX);
    r32 TranslationPix = TranslationPercent*DisplayColumn->SlotWidth*(Slider->OverhangP-1);
    // TODO:: The first visible letter needs to be squashed according to its overhang!
    for(u32 It = 0;
        It < DisplayColumn->Count && 
        It < DisplayColumn->SortingInfo->Displayable.A.Count;
        It++)
    {
        if(DisplayColumn->Type == columnType_Song) SongHorizontalSliderDrag(Renderer, DisplayColumn, TranslationPix, It);
        else UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationPix, It, DisplayColumn->Text);
    }
}


inline r32
GetDisplayableHeight(column_sorting_info *ColumnSortingInfo, r32 SlotHeight)
{
    return ColumnSortingInfo->Displayable.A.Count * SlotHeight;
}

inline void
UpdateColumnVerticalSliderPosition(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *ColumnSorting)
{
    slider *Slider       = &DisplayColumn->SliderVertical;
    r32 TotalSliderScale = GetScale(Slider->Background).y;
    r32 TotalHeight      = GetDisplayableHeight(ColumnSorting, DisplayColumn->SlotHeight);
    TotalHeight          = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    r32 CursorHeightPercentage = SaveDiv(DisplayColumn->DisplayCursor,(r32)TotalHeight);
    v2 P = V2(GetLocalPosition(Slider->GrabThing).x, GetLocalPosition(Slider->Background).y+Slider->MaxSlidePix);
    P.y -= CursorHeightPercentage*(GetSize(Slider->Background).y-GetSize(Slider->GrabThing).y);
    SetLocalPosition(Slider->GrabThing, P);
    
    UpdateColumnHorizontalSlider(Renderer, DisplayColumn, ColumnSorting);
}

internal void
UpdateColumnVerticalSlider(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *ColumnSorting)
{
    slider *Slider = &DisplayColumn->SliderVertical;
    
    r32 TotalDisplayableSize = GetDisplayableHeight(ColumnSorting, DisplayColumn->SlotHeight);
    v2 TotalScale            = GetSize(Slider->Background);
    Slider->OverhangP = Clamp(TotalScale.y/TotalDisplayableSize, 0.01f, 1.0f);
    
    v2 NewScale = {GetSize(Slider->GrabThing).x, Max(TotalScale.y*Slider->OverhangP, 5.0f)};
    SetSize(Slider->GrabThing, NewScale);
    Slider->MaxSlidePix = (TotalScale.y - NewScale.y)/2.0f;
    UpdateColumnVerticalSliderPosition(Renderer, DisplayColumn, ColumnSorting);
}

inline void
UpdateVerticalSliders(renderer *Renderer, music_display_info *DisplayInfo, music_sorting_info *SortInfo)
{
    UpdateColumnVerticalSlider(Renderer, &DisplayInfo->Genre, &SortInfo->Genre);
    UpdateColumnVerticalSlider(Renderer, &DisplayInfo->Artist, &SortInfo->Artist);
    UpdateColumnVerticalSlider(Renderer, &DisplayInfo->Album, &SortInfo->Album);
    UpdateColumnVerticalSlider(Renderer, &DisplayInfo->Song.Base, &SortInfo->Song);
}

internal void
OnVerticalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    drag_slider_data *DragData = (drag_slider_data *)Data;
    music_display_column *DisplayColumn = DragData->DisplayColumn;
    column_sorting_info  *SortingColumn = DragData->SortingColumn;
    slider *Slider = &DisplayColumn->SliderVertical;
    
    r32 GrabThingHalfHeight  = GetSize(Slider->GrabThing).y/2.0f;
    if(DragData->MouseOffset.y < GrabThingHalfHeight && DragData->MouseOffset.y > -GrabThingHalfHeight) 
        AdjustedMouseP.y -= DragData->MouseOffset.y;
    
    r32 ParentY = GetLocalPosition(Slider->GrabThing->ID->Parent).y;
    r32 BGYPos = GetLocalPosition(Slider->Background).y + ParentY;
    r32 NewY   = Clamp(AdjustedMouseP.y, BGYPos - Slider->MaxSlidePix, BGYPos + Slider->MaxSlidePix);
    
    SetLocalPosition(Slider->GrabThing, V2(GetLocalPosition(Slider->GrabThing).x, NewY-ParentY));
    
    r32 TotalSliderScale = GetSize(Slider->Background).y;
    r32 GrabThingSize    = GetSize(Slider->GrabThing).y;
    r32 RemainingScale   = TotalSliderScale-GrabThingSize;
    r32 TopPositionY     = BGYPos+Slider->MaxSlidePix;
    
    r32 TranslationPercentage = (TopPositionY-NewY)/(Slider->MaxSlidePix*2);
    if(TranslationPercentage > 0.999f) TranslationPercentage = 1;
    r32 TotalHeight = GetDisplayableHeight(SortingColumn, DisplayColumn->SlotHeight);
    TotalHeight = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    i32 ScrollAmount = (i32)(DisplayColumn->DisplayCursor - TotalHeight*TranslationPercentage);
    ScrollDisplayColumn(Renderer, DisplayColumn, (r32)-ScrollAmount);
    
    UpdateColumnHorizontalSlider(Renderer, DisplayColumn, SortingColumn);
}

internal void
OnSongDragEnd(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    
    array_u32 IgnoreDecodeIDs = CreateArray(&GlobalGameState.Bucket.Transient, 2);
    
    // This finds the next and prev DecodeIDs in order to not evict them on the upcoming load
    if(MusicInfo->PlayingSong.PlaylistID.ID > -1)
    {
        file_id PrevFileID = PlaylistIDToFileID(&MusicInfo->Playlist, 
                                                GetPreviousSong(&MusicInfo->Playlist, MusicInfo->PlayingSong.PlaylistID));
        Assert(PrevFileID < (i32)GlobalGameState.MP3Info->FileInfo.Count);
        Assert(PrevFileID > -1);
        u32 PrevDecodeID = 0;
        if(!Find(&DecodeInfo->FileID, PrevFileID, &PrevDecodeID)) 
        {
            AddJob_LoadMP3(&GlobalGameState, &GlobalGameState.JobQueue, PrevFileID);
            Assert(Find(&DecodeInfo->FileID, PrevFileID, &PrevDecodeID));
        }
        Push(&IgnoreDecodeIDs, PrevDecodeID);
        
        file_id NextFileID = PlaylistIDToFileID(&MusicInfo->Playlist, 
                                                GetSongAfterCurrent(&MusicInfo->Playlist, MusicInfo->PlayingSong.PlaylistID));
        Assert(NextFileID < (i32)GlobalGameState.MP3Info->FileInfo.Count);
        Assert(NextFileID > -1);
        u32 NextDecodeID = 0;
        if(!Find(&DecodeInfo->FileID, NextFileID, &NextDecodeID)) 
        {
            AddJob_LoadMP3(&GlobalGameState, &GlobalGameState.JobQueue, NextFileID);
            if(!Find(&DecodeInfo->FileID, NextFileID, &NextDecodeID)) Assert(false);
        }
        Push(&IgnoreDecodeIDs, NextDecodeID);
    }
    
    AddJobs_LoadOnScreenMP3s(&GlobalGameState, &GlobalGameState.JobQueue, &IgnoreDecodeIDs);
    DestroyArray(&GlobalGameState.Bucket.Transient, IgnoreDecodeIDs);
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
    r32 TranslationPercent = SaveDiv(1.0f,(VolumeSlider->MaxSlidePix*2))*(NewX-BGX) + 0.5f;
    
    SetToneVolume(GameState->SoundThreadInterface, TranslationPercent);
}

inline b32 
IsInOnScreenList(music_display_column *DisplayColumn, column_sorting_info *SortingInfo, 
                 playlist_id PlaylistID, u32 *OnScreenID = 0)
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
SetSelection(music_display_column *DisplayColumn, column_sorting_info *SortColumn, displayable_id ID, b32 Select)
{
    Assert(ID.ID < (i32)SortColumn->Displayable.A.Count);
    
    file_id FileID = Get(&SortColumn->Displayable, ID);
    if(Select) Push(&SortColumn->Selected, FileID);
    else StackFindAndTake(&SortColumn->Selected, FileID);
}

inline void
Select(music_display_column *DisplayColumn, column_sorting_info *SortColumn, u32 OnScreenIDsID)
{
    Assert(DisplayColumn->Count > OnScreenIDsID && SortColumn->Displayable.A.Count > OnScreenIDsID);
    
    file_id FileID = Get(&SortColumn->Displayable, DisplayColumn->OnScreenIDs[OnScreenIDsID]);
    Push(&SortColumn->Selected, FileID);
}

inline void
Deselect(music_display_column *DisplayColumn, column_sorting_info *SortColumn, u32 OnScreenIDsID)
{
    Assert(DisplayColumn->Count > OnScreenIDsID && SortColumn->Displayable.A.Count > OnScreenIDsID);
    
    file_id FileID  = Get(&SortColumn->Displayable, DisplayColumn->OnScreenIDs[OnScreenIDsID]);
    StackFindAndTake(&SortColumn->Selected, FileID);
}

inline b32
IsSelected(music_display_column *DisplayColumn, column_sorting_info *SortColumn, u32 OnScreenIDsID)
{
    Assert(DisplayColumn->Count > OnScreenIDsID && SortColumn->Displayable.A.Count > OnScreenIDsID);
    
    file_id FileID  = Get(&SortColumn->Displayable, DisplayColumn->OnScreenIDs[OnScreenIDsID]);
    return StackContains(&SortColumn->Selected, FileID);
}

inline void
ToggleSelection(music_display_column *DisplayColumn, column_sorting_info *SortColumn, u32 OnScreenIDsID)
{
    if(IsSelected(DisplayColumn, SortColumn, OnScreenIDsID)) Deselect(DisplayColumn, SortColumn, OnScreenIDsID);
    else Select(DisplayColumn, SortColumn, OnScreenIDsID);
}

inline void
ClearSelection(column_sorting_info *SortColumn)
{
    Reset(&SortColumn->Selected);
}

internal void
UpdateDisplayColumnColor(music_display_column *DisplayColumn, column_sorting_info *SortColumnInfo)
{
    Assert(DisplayColumn->Type != columnType_Song);
    for(u32 It = 0; 
        It < DisplayColumn->Count &&
        It < SortColumnInfo->Displayable.A.Count; 
        It++)
    {
        if(IsSelected(DisplayColumn, SortColumnInfo, It))
        {
            DisplayColumn->BGRects[It]->ID->Color = &DisplayColumn->Base->ColorPalette.Selected;
        }
        else DisplayColumn->BGRects[It]->ID->Color = &DisplayColumn->Base->ColorPalette.Slot; 
    }
}

internal void
UpdatePlayingSongColor(music_display_column *DisplayColumn, column_sorting_info *SortColumnInfo, file_id FileID, v3 *Color)
{
    for(u32 It = 0; 
        It < DisplayColumn->Count &&
        It < SortColumnInfo->Displayable.A.Count; 
        It++)
    {
        file_id ActualID  = Get(&SortColumnInfo->Displayable, NewPlaylistID(DisplayColumn->OnScreenIDs[It]));
        if(FileID == ActualID)
        {
            DisplayColumn->BGRects[It]->ID->Color = Color;
        }
        else if(StackContains(&SortColumnInfo->Selected, ActualID))
        {
            DisplayColumn->BGRects[It]->ID->Color = &DisplayColumn->Base->ColorPalette.Selected;
        }
        else DisplayColumn->BGRects[It]->ID->Color = &DisplayColumn->Base->ColorPalette.Slot; 
    }
}

inline void
UpdateColumnColor(music_display_column *DisplayColumn, column_sorting_info *SortingColumn)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    if(DisplayColumn->Type == columnType_Song)
    {
        file_id FileID = {-1};
        if(MusicInfo->PlayingSong.PlaylistID >= 0) FileID = Get(&MusicInfo->Playlist.Songs, MusicInfo->PlayingSong.PlaylistID);
        UpdatePlayingSongColor(DisplayColumn, SortingColumn, FileID, &MusicInfo->DisplayInfo.ColorPalette.PlayingSong);
    }
    else
    {
        UpdateDisplayColumnColor(DisplayColumn, SortingColumn);
    }
}

internal void
UpdateSelectionColors(music_info *MusicInfo)
{
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Genre, &MusicInfo->SortingInfo.Genre);
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Artist, &MusicInfo->SortingInfo.Artist);
    UpdateDisplayColumnColor(&MusicInfo->DisplayInfo.Album, &MusicInfo->SortingInfo.Album);
    
    if(MusicInfo->PlayingSong.PlaylistID >= 0)
    {
        file_id FileID = Get(&MusicInfo->Playlist.Songs, MusicInfo->PlayingSong.PlaylistID);
        UpdatePlayingSongColor(&MusicInfo->DisplayInfo.Song.Base, &MusicInfo->SortingInfo.Song, FileID, &MusicInfo->DisplayInfo.ColorPalette.PlayingSong);
    }
}

inline void
ChangeDisplayColumnCount(renderer *Renderer, music_display_column *DisplayColumn, u32 NewCount)
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
RemoveSongText(music_display_column *DisplayColumn, u32 ID)
{
    display_column_song_extension *DisplaySong = ColumnExt(DisplayColumn);
    RemoveRenderText(DisplaySong->SongTitle+ID);
    RemoveRenderText(DisplaySong->SongArtist+ID);
    RemoveRenderText(DisplaySong->SongAlbum+ID);
    RemoveRenderText(DisplaySong->SongTrack+ID);
    RemoveRenderText(DisplaySong->SongYear+ID);
}

inline void
UpdateSongText(renderer *Renderer, column_sorting_info *SortingColumn, music_display_column *DisplayColumn, u32 ID, displayable_id NextID)
{
    display_column_song_extension *DisplaySong = ColumnExt(DisplayColumn);
    file_id NextSongID = Get(&SortingColumn->Displayable, NextID);
    mp3_metadata *MD = &DisplaySong->FileInfo->Metadata[NextSongID.ID];
    
    string_c *Title = 0;
    if((MD->FoundFlags & metadata_Title) == metadata_Title) Title  = &MD->Title;
    else 
    {
        Title = &DisplaySong->FileInfo->FileName[NextSongID.ID];
        Title->Pos -= 4;
    }
    
    v2 SongP = {SONG_TITLE_X_OFFSET, SONG_FIRST_ROW_Y_OFFSET};
    CreateRenderText(Renderer, Renderer->FontInfo.MediumFont, Title, &DisplayColumn->Base->ColorPalette.Text,
                     DisplaySong->SongTitle+ID, -0.12f, DisplayColumn->BGRects[ID], SongP);
    
    if((MD->FoundFlags & metadata_Title) != metadata_Title) Title->Pos += 4;
    
    v2 AlbumP = {SONG_ALBUM_X_OFFSET, SONG_SECOND_ROW_Y_OFFSET}; 
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, &MD->Album, &DisplayColumn->Base->ColorPalette.Text,
                     DisplaySong->SongAlbum+ID, -0.12f, DisplayColumn->BGRects[ID], AlbumP);
    
    v2 ArtistP = {SONG_ARTIST_X_OFFSET, SONG_THIRD_ROW_Y_OFFSET};
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, &MD->Artist, &DisplayColumn->Base->ColorPalette.Text,
                     DisplaySong->SongArtist+ID, -0.12f, DisplayColumn->BGRects[ID], ArtistP);
    
    v2 TrackP = {SONG_TRACK_X_OFFSET, SONG_FIRST_ROW_Y_OFFSET};
    CreateRenderText(Renderer, Renderer->FontInfo.MediumFont, 
                     &MD->TrackString, &DisplayColumn->Base->ColorPalette.Text, DisplaySong->SongTrack+ID, -0.12f, 
                     DisplayColumn->BGRects[ID], TrackP);
    
    string_c YearAddon = NewStringCompound(&GlobalGameState.Bucket.Transient, 10);
    string_c Addon = NewStaticStringCompound(" |");
    if(MD->YearString.Pos < 4) 
    {
        string_c FakeYear = NewStaticStringCompound("   --  ");
        ConcatStringCompounds(3, &YearAddon, &FakeYear, &Addon);
    }
    else if(MD->YearString.Pos > 4);
    else ConcatStringCompounds(3, &YearAddon, &MD->YearString, &Addon);
    
    v2 YearP = {SONG_ARTIST_X_OFFSET, SONG_SECOND_ROW_Y_OFFSET};
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, &YearAddon, &DisplayColumn->Base->ColorPalette.Text,
                     DisplaySong->SongYear+ID, -0.12f, DisplayColumn->BGRects[ID], YearP);
    
    DeleteStringCompound(&GlobalGameState.Bucket.Transient, &YearAddon);
}

internal void
MoveDisplayColumn(renderer *Renderer, music_display_column *DisplayColumn, displayable_id DisplayableStartID, r32 StartY)
{
    // FileOrDisplayableStartID is either a fileID when the colum is the song column, 
    // or a displayID when it is another.
    column_sorting_info *SortingColumn = DisplayColumn->SortingInfo;
    DisplayColumn->DisplayCursor = DisplayableStartID.ID*DisplayColumn->SlotHeight + StartY;
    
    For(DisplayColumn->Count)
    {
        if(DisplayColumn->Type == columnType_Song) 
        {
            RemoveSongText(DisplayColumn, It);
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, false);
        }
        else RemoveRenderText(DisplayColumn->Text+It);
        DisplayColumn->BGRects[It]->ID->Render = false;
    }
    
    displayable_id NextID = DisplayableStartID;
    SetLocalPosition(DisplayColumn->BGRects[0], V2(0, StartY));
    
    for(u32 It = 0; 
        It < SortingColumn->Displayable.A.Count && 
        It < DisplayColumn->Count;
        It++)
    {
        DisplayColumn->BGRects[It]->ID->Render = true;
        
        if(DisplayColumn->Type == columnType_Song) 
        {
            SetSongButtonsActive(ColumnExt(DisplayColumn), It, true);
            UpdateSongText(Renderer, SortingColumn, DisplayColumn, It, NextID);
        }
        else
        {
            string_c *Name  = SortingColumn->Batch.Names+Get(&SortingColumn->Displayable, NextID).ID;
            CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, Name, &DisplayColumn->Base->ColorPalette.Text,
                             DisplayColumn->Text+It,  DisplayColumn->ZValue-0.01f, DisplayColumn->BGRects[It]);
        }
        DisplayColumn->OnScreenIDs[It] = NextID;
        // #LastSlotOverflow, The last ID is not visible when the column is at the bottom, 
        // but the NextID would be out of displayable range.
        if(NextID.ID+1 < (i32)DisplayColumn->SortingInfo->Displayable.A.Count) NextID.ID++; 
    }
    ResetColumnText(Renderer, DisplayColumn, DisplayColumn->SortingInfo);
    UpdateColumnColor(DisplayColumn, DisplayColumn->SortingInfo);
    
    UpdateColumnVerticalSliderPosition(Renderer, DisplayColumn, DisplayColumn->SortingInfo);
}

internal void
ScrollDisplayColumn(renderer *Renderer, music_display_column *DisplayColumn, r32 ScrollAmount)
{
    column_sorting_info *SortingColumn = DisplayColumn->SortingInfo;
    r32 SlotHeight = DisplayColumn->SlotHeight;
    r32 TotalHeight   = GetDisplayableHeight(SortingColumn, SlotHeight);
    // DisplayCursor is the very top position. Therefore MaxHeight needs to be reduced by VisibleHeight
    TotalHeight = Max(0.0f, TotalHeight-DisplayColumn->ColumnHeight);
    
    i32 PrevCursorID = Floor(DisplayColumn->DisplayCursor/SlotHeight);
    DisplayColumn->DisplayCursor = Clamp(DisplayColumn->DisplayCursor+ScrollAmount, 0.0f, TotalHeight);
    
    r32 NewY = Mod(DisplayColumn->DisplayCursor, SlotHeight);
    i32 NewCursorID = Floor(DisplayColumn->DisplayCursor/SlotHeight);
    NewCursorID     = Min(NewCursorID, SortingColumn->Displayable.A.Count-1);
    i32 CursorDiff  = NewCursorID - PrevCursorID;
    
    if(CursorDiff != 0)
    {
        i32 MaximumID = Max(0, SortingColumn->Displayable.A.Count - DisplayColumn->Count + 1);
        MoveDisplayColumn(Renderer, DisplayColumn, NewDisplayableID(Min(NewCursorID, MaximumID)), NewY);
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
    
    MoveDisplayColumn(Renderer, &DisplayInfo->Song.Base);
    MoveDisplayColumn(Renderer, &DisplayInfo->Genre);
    MoveDisplayColumn(Renderer, &DisplayInfo->Artist);
    MoveDisplayColumn(Renderer, &DisplayInfo->Album);
}

internal void
BringDisplayableEntryOnScreen(music_display_column *DisplayColumn, file_id FileID)
{
    displayable_id DisplayID = FileIDToColumnDisplayID(DisplayColumn, FileID);
    i32 MaximumID = Max(0, DisplayColumn->SortingInfo->Displayable.A.Count-DisplayColumn->Count+1);
    DisplayID.ID = Min(DisplayID.ID, MaximumID);
    
    MoveDisplayColumn(&GlobalGameState.Renderer, DisplayColumn, DisplayID, 0);
}

internal void
BringDisplayableEntryOnScreenWithSortID(music_display_column *DisplayColumn, batch_id BatchID)
{
    displayable_id DisplayID = SortingIDToColumnDisplayID(DisplayColumn, BatchID);
    i32 MaximumID = Max(0, DisplayColumn->SortingInfo->Displayable.A.Count-DisplayColumn->Count+1);
    DisplayID.ID = Min(DisplayID.ID, MaximumID);
    
    MoveDisplayColumn(&GlobalGameState.Renderer, DisplayColumn, DisplayID, 0);
}

internal void
KeepPlayingSongOnScreen(renderer *Renderer, music_info *MusicInfo)
{
    if(MusicInfo->PlayingSong.PlaylistID < 0) return;
    music_display_column *DisplayColumn = &MusicInfo->DisplayInfo.Song.Base;
    column_sorting_info *SortingColumn = &MusicInfo->SortingInfo.Song;
    u32 OnScreenID = 0;
    if(IsInOnScreenList(DisplayColumn, SortingColumn, MusicInfo->PlayingSong.PlaylistID, &OnScreenID))
    {
        if(IsIntersectingRectButTopShowing(DisplayColumn->BGRects[OnScreenID], DisplayColumn->BottomBorder))
        {
            ScrollDisplayColumn(Renderer, DisplayColumn, DisplayColumn->SlotHeight);
        }
        else if(IsIntersectingRectButBottomShowing(DisplayColumn->BGRects[OnScreenID], DisplayColumn->TopBorder))
        {
            ScrollDisplayColumn(Renderer, DisplayColumn, -DisplayColumn->SlotHeight);
        }
    }
}

internal void
FitDisplayColumnIntoSlot(renderer *Renderer, music_display_column *DisplayColumn, column_sorting_info *ColumnSorting)
{
    // Calc new column scale
    DisplayColumn->ColumnHeight = HeightBetweenRects(DisplayColumn->TopBorder, DisplayColumn->BottomBorder);
    DisplayColumn->SlotWidth = 4+WidthBetweenRects(DisplayColumn->LeftBorder, DisplayColumn->RightBorder);
    
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
    ResetColumnText(Renderer, DisplayColumn, ColumnSorting);
    
    // Calc slider fit
    r32 BGSizeY = GetSize(DisplayColumn->SliderHorizontal.Background).y;
    r32 BGPosY  = GetLocalPosition(DisplayColumn->SliderHorizontal.Background).y;
    SetSize(DisplayColumn->SliderHorizontal.Background, V2(DisplayColumn->SlotWidth, BGSizeY));
    SetPosition(DisplayColumn->SliderHorizontal.Background, V2(CenterX, BGPosY));
    
    slider *VSlider = &DisplayColumn->SliderVertical;
    r32 BGSizeX = GetSize(VSlider->Background).x;
    SetSize(VSlider->Background, V2(BGSizeX, DisplayColumn->ColumnHeight));
    UpdateColumnVerticalSlider(Renderer, DisplayColumn, ColumnSorting);
    
    // Search bar
    SetSize(DisplayColumn->Search.TextField.Background,
            V2(DisplayColumn->SlotWidth-4, GetSize(DisplayColumn->Search.TextField.Background).y));
    SetLocalPosition(DisplayColumn->Search.TextField.LeftAlign, V2(-(DisplayColumn->SlotWidth-4)/2.0f, 0));
}

internal void
ProcessWindowResizeForDisplayColumn(renderer *Renderer, music_info *MusicInfo, 
                                    column_sorting_info *SortingColumn, music_display_column *DisplayColumn)
{
    u32 NewDisplayCount = CountPossibleDisplayedSlots(Renderer, DisplayColumn);
    ChangeDisplayColumnCount(Renderer, DisplayColumn, NewDisplayCount);
    
    FitDisplayColumnIntoSlot(Renderer, DisplayColumn, SortingColumn);
    MoveDisplayColumn(Renderer, DisplayColumn, DisplayColumn->OnScreenIDs[0],GetLocalPosition(DisplayColumn->BGRects[0]).y);
    
    // I do this to fix that if the column is at the bottom and the window gets bigger, the
    // slots will be stopped at the edge. Without this, the new visible slots will be the same
    // ID. See: #LastSlotOverflow in MoveDisplayColumn
    drag_slider_data Data = { MusicInfo, DisplayColumn, SortingColumn, 0};
    OnVerticalSliderDrag(Renderer, GetPosition(DisplayColumn->SliderVertical.GrabThing), 0, &Data);
    
    SetActive(DisplayColumn->Search.Button, (Renderer->Window.CurrentDim.Height > (GlobalMinWindowHeight + 15)));
}

struct column_edge_drag
{
    entry_id *LeftEdge;
    r32 LeftOffset;
    entry_id *RightEdge;
    r32 RightOffset;
    
    r32 *XPercent;
    music_display_info *DisplayInfo;
    music_sorting_info *SortingInfo;
};

internal void
OnDisplayColumnEdgeDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    column_edge_drag *Info = (column_edge_drag *)Data;
    AdjustedMouseP.x = Clamp(AdjustedMouseP.x, 
                             GetLocalPosition(Info->LeftEdge).x + Info->LeftOffset, 
                             GetLocalPosition(Info->RightEdge).x - Info->RightOffset);
    
    
    SetLocalPosition(Dragable, V2(AdjustedMouseP.x, GetLocalPosition(Dragable).y));
    *Info->XPercent = GetPosition(Dragable).x/(r32)Renderer->Window.CurrentDim.Width;
    
    FitDisplayColumnIntoSlot(Renderer, &Info->DisplayInfo->Genre, &Info->SortingInfo->Genre);
    FitDisplayColumnIntoSlot(Renderer, &Info->DisplayInfo->Artist, &Info->SortingInfo->Artist);
    FitDisplayColumnIntoSlot(Renderer, &Info->DisplayInfo->Album, &Info->SortingInfo->Album);
    FitDisplayColumnIntoSlot(Renderer, &Info->DisplayInfo->Song.Base, &Info->SortingInfo->Song);
    UpdateHorizontalSliders(Renderer, Info->DisplayInfo, Info->SortingInfo);
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
    
    SetBetweenSliderRectPosition(DisplayInfo->Genre.BetweenSliderRect,     DisplayInfo->GenreArtist.Edge);
    SetBetweenSliderRectPosition(DisplayInfo->Artist.BetweenSliderRect,    DisplayInfo->ArtistAlbum.Edge);
    SetBetweenSliderRectPosition(DisplayInfo->Album.BetweenSliderRect,     DisplayInfo->AlbumSong.Edge);
    SetBetweenSliderRectPosition(DisplayInfo->Song.Base.BetweenSliderRect, DisplayInfo->EdgeRight);
}

inline void
OnNextSong(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    
    HandleChangeToNextSong(GameState);
    if(Info->PlayingSong->PlaylistID == -1) 
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
        SetPreviousSong(&MusicInfo->Playlist, Info->PlayingSong, MusicInfo->Looping);
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
    
    if(Info->GameState->MusicInfo.PlayingSong.PlaylistID < 0) 
    {
        Info->GameState->MusicInfo.PlayingSong.PlaylistID.ID = 0;
        Info->GameState->MusicInfo.PlayingSong.FileID = PlaylistIDToFileID(&Info->GameState->MusicInfo.Playlist, NewPlaylistID(0));
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
    
    //if(Info->PlayingSong->PlaylistID >= 0)
    {
        ChangeSong(GameState, Info->PlayingSong);
        Info->GameState->MusicInfo.IsPlaying = false;
        PushSoundBufferClear(GameState->SoundThreadInterface);
        
        GameState->MusicInfo.DisplayInfo.PlayPause->State = buttonState_Unpressed;
        GameState->MusicInfo.DisplayInfo.PlayPause->Entry->ID->Color = GameState->MusicInfo.DisplayInfo.PlayPause->BaseColor;
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
    
    ShuffleStack(&MusicInfo->SortingInfo.Song.Displayable);
    UseDisplayableAsPlaylist(MusicInfo);
    MoveDisplayColumn(&GameState->Renderer, &MusicInfo->DisplayInfo.Song.Base);
}

inline void
OnShufflePlaylistToggleOff(void *Data)
{
    music_btn *Info = (music_btn *)Data;
    game_state *GameState = Info->GameState;
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->IsShuffled = false;
    
    SortDisplayables(&MusicInfo->SortingInfo, &GameState->MP3Info->FileInfo);
    UseDisplayableAsPlaylist(MusicInfo);
    MoveDisplayColumn(&GameState->Renderer, &MusicInfo->DisplayInfo.Song.Base);
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

inline void
OnPaletteSwap(void *Data)
{
    music_display_info *DisplayInfo = &((music_btn *)Data)->GameState->MusicInfo.DisplayInfo;
    UpdateColorPalette(DisplayInfo, true);
}

inline void
OnMusicPathPressed(void *Data)
{
    music_btn *MusicBtnInfo = (music_btn *)Data;
    music_display_info *DisplayInfo = &MusicBtnInfo->GameState->MusicInfo.DisplayInfo;
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    
    SetTextFieldActive(&MusicPath->TextField, !MusicPath->TextField.IsActive);
    if(MusicPath->TextField.IsActive)
    {
        if(IsSearchOpen(DisplayInfo))
        {
            InterruptSearch(&MusicBtnInfo->GameState->Renderer, DisplayInfo, &MusicBtnInfo->GameState->MusicInfo.SortingInfo);
        }
        UpdateTextField(&MusicBtnInfo->GameState->Renderer, &MusicPath->TextField);
        
        renderer *Renderer = &MusicBtnInfo->GameState->Renderer;
        string_c PathText = NewStringCompound(&MusicBtnInfo->GameState->Bucket.Transient, 255+12);
        AppendStringToCompound(&PathText, (u8 *)"Old Path:     ");
        if(MusicBtnInfo->GameState->MP3Info->FolderPath.Pos == 0)
            AppendStringToCompound(&PathText, (u8 *)" - ");
        else AppendStringCompoundToCompound(&PathText, &MusicBtnInfo->GameState->MP3Info->FolderPath);
        RemoveRenderText(&MusicPath->CurrentPath);
        CreateRenderText(Renderer, Renderer->FontInfo.MediumFont, &PathText, &DisplayInfo->ColorPalette.ForegroundText, &MusicPath->CurrentPath, -0.6f-0.001f, MusicPath->TextField.LeftAlign, 
                         V2(0, 62));
        DeleteStringCompound(&MusicBtnInfo->GameState->Bucket.Transient, &PathText);
    }
    else 
    {
        RemoveRenderText(&MusicPath->TextField.Text);
        SetActive(&MusicPath->LoadingBar, false);
    }
    MusicPath->Background->ID->Render = MusicPath->TextField.IsActive;
    SetActive(&MusicPath->CurrentPath, MusicPath->TextField.IsActive);
    SetActive(MusicPath->Save, MusicPath->TextField.IsActive);
    SetActive(MusicPath->Quit, MusicPath->TextField.IsActive);
    SetActive(MusicPath->Rescan, MusicPath->TextField.IsActive);
    RemoveRenderText(&MusicPath->Output);
    
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
    
    RemoveRenderText(&MusicPath->Output);
    CreateRenderText(&GlobalGameState.Renderer, GlobalGameState.Renderer.FontInfo.MediumFont, &MusicPath->OutputString,
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
    RemoveRenderText(&MusicPath->Output);
    CreateRenderText(&GameState->Renderer, GameState->Renderer.FontInfo.MediumFont, &MusicPath->OutputString,
                     &GameState->MusicInfo.DisplayInfo.ColorPalette.ForegroundText, &MusicPath->Output, -0.6f-0.001f, MusicPath->TextField.LeftAlign, V2(0, -175));
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

internal void
InitializeDisplayInfo(music_display_info *DisplayInfo, game_state *GameState, mp3_info *MP3Info)
{
    renderer *Renderer = &GameState->Renderer;
    r32 WWidth = (r32)Renderer->Window.FixedDim.Width;
    r32 WHeight = (r32)Renderer->Window.FixedDim.Height;
    
    DisplayInfo->Song.FileInfo = &MP3Info->FileInfo;
    CreateBasicColorPalette(&DisplayInfo->ColorPalette);
    
    r32 TopMargin = 50;
    r32 EdgeMargin = 24;
    r32 BottomMargin = EdgeMargin*5;
    r32 LeftX = -688;
    
    
    DisplayInfo->EdgeTop = CreateRenderRect(Renderer, {{0, WHeight-TopMargin},{WWidth, WHeight}}, -0.5f,
                                            0, &DisplayInfo->ColorPalette.Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeTop, fixedTo_TopCenter, scaleAxis_X);
    DisplayInfo->EdgeBottom = CreateRenderRect(Renderer, {{0, 0},{WWidth, BottomMargin}}, -0.5f,
                                               0, &DisplayInfo->ColorPalette.Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeBottom, fixedTo_BottomCenter, scaleAxis_X);
    
    r32 HeightEdge = HeightBetweenRects(DisplayInfo->EdgeTop, DisplayInfo->EdgeBottom);
    
    DisplayInfo->EdgeLeft   = CreateRenderRect(Renderer, {{0,BottomMargin},{EdgeMargin,WHeight-TopMargin}}, -0.5f,
                                               0, &DisplayInfo->ColorPalette.Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeLeft, fixedTo_MiddleLeft, scaleAxis_Y);
    r32 RightX = 688;
    DisplayInfo->EdgeRight  = CreateRenderRect(Renderer, {{WWidth-EdgeMargin,BottomMargin},{WWidth,WHeight-TopMargin}}, 
                                               -0.5f, 0, &DisplayInfo->ColorPalette.Foreground);
    TransformWithScreen(&Renderer->TransformList, DisplayInfo->EdgeRight, fixedTo_MiddleRight, scaleAxis_Y);
    
    r32 EdgeWidth = 6;
    r32 ColumnWidth = (WWidth/2.0f)/3.0f + 30;
    r32 GenreArtistX = ColumnWidth+EdgeMargin ;
    DisplayInfo->GenreArtist.Edge = CreateRenderRect(Renderer, 
                                                     {{GenreArtistX,BottomMargin},{GenreArtistX+EdgeWidth,WHeight-TopMargin}},
                                                     -0.5f, 0, &DisplayInfo->ColorPalette.Foreground);
    DisplayInfo->GenreArtist.XPercent = GetPosition(DisplayInfo->GenreArtist.Edge).x/WWidth;
    DisplayInfo->GenreArtist.OriginalYHeight = GetPosition(DisplayInfo->GenreArtist.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->GenreArtist.Edge, scaleAxis_Y);
    
    r32 ArtistAlbumX = ColumnWidth*2+EdgeMargin+EdgeWidth;
    DisplayInfo->ArtistAlbum.Edge = CreateRenderRect(Renderer, 
                                                     {{ArtistAlbumX,BottomMargin},{ArtistAlbumX+EdgeWidth,WHeight-TopMargin}}, 
                                                     -0.5f, 0, &DisplayInfo->ColorPalette.Foreground);
    DisplayInfo->ArtistAlbum.XPercent = GetPosition(DisplayInfo->ArtistAlbum.Edge).x/WWidth;
    DisplayInfo->ArtistAlbum.OriginalYHeight = GetPosition(DisplayInfo->ArtistAlbum.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->ArtistAlbum.Edge, scaleAxis_Y);
    
    r32 AlbumSongX  = ColumnWidth*3+EdgeMargin+EdgeWidth*2;
    DisplayInfo->AlbumSong.Edge   = CreateRenderRect(Renderer, 
                                                     {{AlbumSongX,BottomMargin},{AlbumSongX+EdgeWidth,WHeight-TopMargin}},
                                                     -0.5f, 0, &DisplayInfo->ColorPalette.Foreground);
    DisplayInfo->AlbumSong.XPercent = GetPosition(DisplayInfo->AlbumSong.Edge).x/WWidth;
    DisplayInfo->AlbumSong.OriginalYHeight = GetPosition(DisplayInfo->AlbumSong.Edge).y;
    ScaleWithScreen(&Renderer->TransformList, DisplayInfo->AlbumSong.Edge, scaleAxis_Y);
    
    Renderer->ButtonBase = LoadImage_STB((u8 *)"..\\data\\Buttons\\PlayPause.png"); 
    Renderer->ButtonBaseID = CreateGLTexture(Renderer->ButtonBase);
    
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Shuffle_Icon.png"); 
    u32 ShuffleID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Shuffle_Pressed_Icon.png"); 
    u32 ShufflePressedID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Loop_Icon.png"); 
    u32 LoopID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Loop_Pressed_Icon.png"); 
    u32 LoopPressedID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Randomize_Icon.png"); 
    u32 RandomizeID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Randomize_Pressed_Icon.png"); 
    u32 RandomizePressedID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Play_Icon.png"); 
    u32 PlayID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Pause_Icon.png"); 
    u32 PauseID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Stop_Icon.png"); 
    u32 StopID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Next_Icon.png"); 
    u32 NextID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Previous_Icon.png"); 
    u32 PreviousID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\MusicPath_Icon.png"); 
    u32 MusicPathID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Confirm_Icon.png"); 
    u32 ConfirmID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Cancel_Icon.png"); 
    u32 CancelID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\PaletteSwap_Icon2.png"); 
    u32 PaletteID = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    Renderer->ButtonIcons[Renderer->ButtonIconCount] = LoadImage_STB((u8 *)"..\\data\\Buttons\\Rescan_Icon.png"); 
    u32 RescanID  = CreateGLTexture(Renderer->ButtonIcons[Renderer->ButtonIconCount++]);
    
    r32 BtnDepth = -0.6f;
    
    r32 PlayPauseX  = 94+22;
    
    r32 SmallRectS  = 16;
    r32 MediumRectS = 21;
    r32 LargeRectS  = 24;
    r32 PlayPauseS  = 45;
    rect PlayPauseRect = {{-PlayPauseS, -PlayPauseS}, {PlayPauseS,PlayPauseS}};
    rect LargeBtnRect  = {{-LargeRectS, -LargeRectS}, {LargeRectS, LargeRectS}};
    rect MediumBtnRect = {{-MediumRectS,-MediumRectS},{MediumRectS,MediumRectS}};
    rect SmallBtnRect  = {{-SmallRectS, -SmallRectS}, {SmallRectS,SmallRectS}};
    DisplayInfo->PlayPause = NewButton(Renderer, PlayPauseRect, BtnDepth, true, 
                                       Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                       &DisplayInfo->ColorPalette.ButtonActive, 
                                       &DisplayInfo->ColorPalette.SliderBackground, 
                                       PlayID, &DisplayInfo->ColorPalette.Text, 0, PauseID);
    SetLocalPosition(DisplayInfo->PlayPause, V2(PlayPauseX, 60));
    DisplayInfo->PlayPause->OnPressed = {OnPlayPauseSongToggleOn, &DisplayInfo->MusicBtnInfo};
    DisplayInfo->PlayPause->OnPressedToggleOff = {OnPlayPauseSongToggleOff, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->Stop      = NewButton(Renderer, LargeBtnRect, BtnDepth, false, 
                                       Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                       &DisplayInfo->ColorPalette.ButtonActive, 
                                       &DisplayInfo->ColorPalette.SliderBackground,
                                       StopID, &DisplayInfo->ColorPalette.Text, 0);
    SetLocalPosition(DisplayInfo->Stop, V2(PlayPauseX + 74, 80.75f));
    DisplayInfo->Stop->OnPressed = {OnStopSong, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->Previous  = NewButton(Renderer, LargeBtnRect, BtnDepth, false, 
                                       Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                       &DisplayInfo->ColorPalette.ButtonActive, 
                                       &DisplayInfo->ColorPalette.SliderBackground,
                                       PreviousID, &DisplayInfo->ColorPalette.Text, 0);
    SetLocalPosition(DisplayInfo->Previous, V2(PlayPauseX+74+53, 80.75f));
    DisplayInfo->Previous->OnPressed = {OnPreviousSong, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->Next      = NewButton(Renderer, LargeBtnRect, BtnDepth, false, 
                                       Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                       &DisplayInfo->ColorPalette.ButtonActive, 
                                       &DisplayInfo->ColorPalette.SliderBackground,
                                       NextID, &DisplayInfo->ColorPalette.Text, 0);
    SetLocalPosition(DisplayInfo->Next, V2(PlayPauseX+74+53*2, 80.75f));
    DisplayInfo->Next->OnPressed = {OnNextSong, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->PaletteSwap = NewButton(Renderer, SmallBtnRect, BtnDepth, false, 
                                         Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                         &DisplayInfo->ColorPalette.ButtonActive, 
                                         &DisplayInfo->ColorPalette.SliderBackground,
                                         PaletteID, &DisplayInfo->ColorPalette.Text, 0);
    //SetButtonTranslation(DisplayInfo->PaletteSwap, V2(PlayPauseX-64, 30));
    SetLocalPosition(DisplayInfo->PaletteSwap, V2(40, Renderer->Window.CurrentDim.Height-25.0f));
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->PaletteSwap->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, DisplayInfo->PaletteSwap->Icon, fixedTo_TopLeft);
    DisplayInfo->PaletteSwap->OnPressed = {OnPaletteSwap, &DisplayInfo->MusicBtnInfo};
    
    // Music path stuff *******************************
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    
    MusicPath->Button = NewButton(Renderer, SmallBtnRect, BtnDepth, false, 
                                  Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                  &DisplayInfo->ColorPalette.ButtonActive, 
                                  &DisplayInfo->ColorPalette.SliderBackground,
                                  MusicPathID, &DisplayInfo->ColorPalette.Text, 0);
    SetLocalPosition(MusicPath->Button, V2(40+SmallRectS*2+5, Renderer->Window.CurrentDim.Height-25.0f));
    TranslateWithScreen(&Renderer->TransformList, MusicPath->Button->Entry, fixedTo_TopLeft);
    TranslateWithScreen(&Renderer->TransformList, MusicPath->Button->Icon, fixedTo_TopLeft);
    MusicPath->Button->OnPressed = {OnMusicPathPressed, &DisplayInfo->MusicBtnInfo};
    
    entry_id *Parent = CreateRenderRect(Renderer, V2(0), 0, 0, 0);
    Parent->ID->Render = false;
    Translate(Parent, V2(WWidth/2.0f, WHeight - 200));
    TranslateWithScreen(&Renderer->TransformList, Parent, fixedTo_FixYToGiven_XLeft, 0.86f);
    
    MusicPath->TextField = CreateTextField(Renderer, &GameState->Bucket.Fixed, V2(WWidth-WWidth*0.1f, 50), BtnDepth-0.001f,
                                           (u8 *)"New Path...", 0, &DisplayInfo->ColorPalette.Text, 
                                           &DisplayInfo->ColorPalette.ButtonActive);
    Translate(&MusicPath->TextField, V2(WWidth/2.0f, WHeight - 200));
    MusicPath->TextField.DoMouseHover = false;
    TransformWithScreen(&Renderer->TransformList, MusicPath->TextField.Background, fixedTo_FixYToGiven_XCenter, scaleAxis_X, 0.86f);
    MusicPath->TextField.LeftAlign->ID->Parent = Parent;
    
    MusicPath->Background = CreateRenderRect(Renderer, V2(WWidth, WHeight), BtnDepth-0.0009f, 
                                             &DisplayInfo->ColorPalette.Foreground, 0);
    Translate(MusicPath->Background, V2(WWidth/2.0f, WHeight/2.0f));
    TransformWithScreen(&Renderer->TransformList, MusicPath->Background, fixedTo_MiddleCenter, scaleAxis_XY);
    SetTransparency(MusicPath->Background, 0.75f);
    MusicPath->Background->ID->Render = false;
    
    MusicPath->Save = NewButton(Renderer, MediumBtnRect, BtnDepth-0.001f, false, 
                                Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                &DisplayInfo->ColorPalette.ButtonActive, 
                                &DisplayInfo->ColorPalette.SliderBackground,
                                ConfirmID, &DisplayInfo->ColorPalette.Text, MusicPath->TextField.LeftAlign);
    Translate(MusicPath->Save, V2(MediumRectS, -MediumRectS -44));
    MusicPath->Save->OnPressed = {OnMusicPathSavePressed, &DisplayInfo->MusicBtnInfo};
    SetActive(MusicPath->Save, MusicPath->TextField.IsActive);
    
    MusicPath->Quit = NewButton(Renderer, MediumBtnRect, BtnDepth-0.001f, false, 
                                Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                &DisplayInfo->ColorPalette.ButtonActive, 
                                &DisplayInfo->ColorPalette.SliderBackground,
                                CancelID, &DisplayInfo->ColorPalette.Text, MusicPath->TextField.LeftAlign);
    Translate(MusicPath->Quit, V2(MediumRectS*3+10, -MediumRectS -25 -19));
    MusicPath->Quit->OnPressed = {OnMusicPathQuitPressed, &DisplayInfo->MusicBtnInfo};
    SetActive(MusicPath->Quit, MusicPath->TextField.IsActive);
    MusicPath->OutputString = NewStringCompound(&GameState->Bucket.Fixed, 500);
    
    v2 LoadingBarSize = V2(WWidth-WWidth*0.1f, 40);
    MusicPath->LoadingBar = CreateLoadingBar(LoadingBarSize, BtnDepth-0.0011f, 
                                             MusicPath->TextField.Background);
    SetLocalPosition(&MusicPath->LoadingBar, V2(0, -LoadingBarSize.y*0.5f - MediumRectS*2 -25 -19*2));
    ScaleWithScreen(&Renderer->TransformList, MusicPath->LoadingBar.BG, scaleAxis_X);
    SetActive(&MusicPath->LoadingBar, false);
    
    MusicPath->CrawlThreadStateCount = 0;
    
    MusicPath->Rescan = NewButton(Renderer, MediumBtnRect, BtnDepth-0.001f, false, Renderer->ButtonBaseID, 
                                  &DisplayInfo->ColorPalette.SliderGrabThing, &DisplayInfo->ColorPalette.ButtonActive, 
                                  &DisplayInfo->ColorPalette.SliderBackground, RescanID, &DisplayInfo->ColorPalette.Text,
                                  MusicPath->TextField.LeftAlign);
    Translate(MusicPath->Rescan, V2(204, 55));
    SetActive(MusicPath->Rescan, false);
    MusicPath->Rescan->OnPressed = {OnRescanPressed, &DisplayInfo->MusicBtnInfo};
    
    
    // Song panel stuff ****************************
    DisplayInfo->Volume.Background = CreateRenderRect(Renderer, {{PlayPauseX+50, 15}, {PlayPauseX+204, 52}}, BtnDepth,
                                                      0, &DisplayInfo->ColorPalette.SliderBackground);
    DisplayInfo->Volume.GrabThing  = CreateRenderRect(Renderer, {{PlayPauseX+50, 15}, {PlayPauseX+50+10, 52}}, 
                                                      BtnDepth - 0.0000001f, 0, &DisplayInfo->ColorPalette.SliderGrabThing);
    DisplayInfo->Volume.MaxSlidePix =
        GetExtends(DisplayInfo->Volume.Background).x - GetExtends(DisplayInfo->Volume.GrabThing).x;
    OnVolumeDrag(Renderer, GetLocalPosition(DisplayInfo->Volume.Background), DisplayInfo->Volume.Background, GameState);
    
    playing_song_panel *Panel = &DisplayInfo->PlayingSongPanel;
    Panel->MP3Info = MP3Info;
    Panel->CurrentTimeString = NewStringCompound(&GameState->Bucket.Fixed, 10);
    
    r32 TimelineX = PlayPauseX+204+10;
    Panel->Timeline.Background = CreateRenderRect(Renderer, {{TimelineX, 15+30}, {TimelineX+400, 15+30+10}}, BtnDepth,
                                                  0, &DisplayInfo->ColorPalette.SliderBackground);
    Panel->Timeline.GrabThing  = CreateRenderRect(Renderer, {{TimelineX, 15}, {TimelineX+10, 15+70}}, BtnDepth - 0.0000001f,
                                                  0, &DisplayInfo->ColorPalette.SliderGrabThing);
    Panel->Timeline.MaxSlidePix = GetExtends(Panel->Timeline.Background).x - GetExtends(Panel->Timeline.GrabThing).x;
    
    SetTheNewPlayingSong(Renderer, Panel, &GameState->MusicInfo);
    
    DisplayInfo->SearchIsActive = -1;
    
    DisplayInfo->ShufflePlaylist = NewButton(Renderer, MediumBtnRect, BtnDepth, true, 
                                             Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                             &DisplayInfo->ColorPalette.ButtonActive, 
                                             &DisplayInfo->ColorPalette.SliderBackground,
                                             RandomizeID, &DisplayInfo->ColorPalette.Text, 0, RandomizePressedID);
    SetLocalPosition(DisplayInfo->ShufflePlaylist, V2(PlayPauseX - (PlayPauseS+MediumRectS+5), MediumRectS+15));
    DisplayInfo->ShufflePlaylist->OnPressed = {OnShufflePlaylistToggleOn, &DisplayInfo->MusicBtnInfo};
    DisplayInfo->ShufflePlaylist->OnPressedToggleOff = {OnShufflePlaylistToggleOff, &DisplayInfo->MusicBtnInfo};
    
    DisplayInfo->LoopPlaylist = NewButton(Renderer, MediumBtnRect, BtnDepth, true, 
                                          Renderer->ButtonBaseID, &DisplayInfo->ColorPalette.SliderGrabThing, 
                                          &DisplayInfo->ColorPalette.ButtonActive, 
                                          &DisplayInfo->ColorPalette.SliderBackground,
                                          LoopID, &DisplayInfo->ColorPalette.Text, 0, LoopPressedID);
    SetLocalPosition(DisplayInfo->LoopPlaylist, V2(PlayPauseX - (PlayPauseS+MediumRectS+5), MediumRectS*3+15+6));
    DisplayInfo->LoopPlaylist->OnPressed = {OnLoopPlaylistToggleOn,  &DisplayInfo->MusicBtnInfo};
    DisplayInfo->LoopPlaylist->OnPressedToggleOff = {OnLoopPlaylistToggleOff, &DisplayInfo->MusicBtnInfo};
    
    // Quit curtain ****************************
    quit_animation *QuitStuff = &DisplayInfo->Quit;
    
    QuitStuff->Curtain = CreateRenderRect(Renderer, V2(WWidth, WHeight), -0.99f,
                                          &DisplayInfo->ColorPalette.SliderGrabThing);
    Translate(QuitStuff->Curtain, V2(WWidth/2, WHeight/2));
    SetActive(QuitStuff->Curtain, false);
    QuitStuff->dAnim = 0;
    QuitStuff->Time = 0.8f;
    
    string_c QuitText = NewStaticStringCompound("Goodbye!");
    CreateRenderText(Renderer, Renderer->FontInfo.BigFont, &QuitText, &DisplayInfo->ColorPalette.Text, 
                     &QuitStuff->Text, -0.999f, 0);
    SetPosition(&QuitStuff->Text, V2(WWidth/2, WHeight/2));
    SetActive(&QuitStuff->Text, false);
    
    QuitStuff->LastEscapePressTime = -10;
    
    // User error text *************************************
    user_error_text *UserErrorText = &DisplayInfo->UserErrorText;
    
    UserErrorText->AnimTime = 2.5f;
    UserErrorText->dAnim = 1.0f;
}

internal void
SetTheNewPlayingSong(renderer *Renderer, playing_song_panel *Panel, music_info *MusicInfo)
{
    RemoveRenderText(&Panel->DurationText);
    RemoveRenderText(&Panel->CurrentTimeText);
    RemoveRenderText(&Panel->Title);
    RemoveRenderText(&Panel->Artist);
    RemoveRenderText(&Panel->Genre);
    RemoveRenderText(&Panel->Album);
    RemoveRenderText(&Panel->Year);
    RemoveRenderText(&Panel->Track);
    
    string_c MissingData = NewStaticStringCompound(" -- ");
    string_c NoTime      = NewStaticStringCompound("00:00");
    
    string_c *DurationString = 0;
    string_c *TitleString  = 0;
    string_c *TrackString  = 0;
    string_c *ArtistString = 0;
    string_c *AlbumString  = 0;
    string_c *YearString   = 0;
    string_c *GenreString  = 0;
    
    if(MusicInfo->PlayingSong.DecodeID >= 0 && 
       (MusicInfo->PlayingSong.PlaylistID >= 0 || MusicInfo->PlayingSong.PlayUpNext))
    {
        file_id FileID = GetNextSong(&MusicInfo->Playlist, &MusicInfo->PlayingSong);
        mp3_metadata *Metadata = &Panel->MP3Info->FileInfo.Metadata[FileID.ID];
        Panel->SongDuration = (r32)Metadata->Duration;
        
        DurationString = &Metadata->DurationString;
        if(Metadata->Title.Pos == 0) TitleString = Panel->MP3Info->FileInfo.FileName+FileID.ID;
        else TitleString = &Metadata->Title;
        TrackString = &Metadata->TrackString;
        if(Metadata->Artist.Pos == 0) ArtistString = &MissingData;
        else ArtistString = &Metadata->Artist;
        if(Metadata->Album.Pos == 0) AlbumString = &MissingData;
        else AlbumString = &Metadata->Album;
        if(Metadata->YearString.Pos == 0) YearString = &MissingData;
        else YearString = &Metadata->YearString;
        if(Metadata->Genre.Pos == 0) GenreString = &MissingData;
        else GenreString = &Metadata->Genre;
    }
    else
    {
        TitleString = TrackString = ArtistString = AlbumString = YearString = GenreString = &MissingData;
        DurationString = &NoTime;
        Panel->SongDuration = 0;
    }
    
    v3 *TextColor = &Panel->MP3Info->MusicInfo->DisplayInfo.ColorPalette.ForegroundText;
    Panel->CurrentTime = 0;
    
    r32 BaseX = 315;
    r32 BaseY = 80;
    
    string_c CurrentTime = NewStaticStringCompound("00:00 |");
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, &CurrentTime, TextColor, &Panel->CurrentTimeText, -0.6f, 0);
    SetPosition(&Panel->CurrentTimeText, V2(BaseX+22, BaseY+19));
    
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, DurationString, TextColor, &Panel->DurationText, -0.6f, 0);
    SetPosition(&Panel->DurationText, V2(BaseX + 75 + 22, BaseY+19));
    
    CreateRenderText(Renderer, Renderer->FontInfo.MediumFont, TitleString, TextColor, &Panel->Title, -0.6f, 0);
    SetPosition(&Panel->Title, V2(BaseX + 480, BaseY+15));
    
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, TrackString, TextColor, &Panel->Track, -0.6f, 0);
    SetPosition(&Panel->Track, V2(BaseX + 440, BaseY + 8));
    
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, ArtistString, TextColor, &Panel->Artist, -0.6f, 0);
    SetPosition(&Panel->Artist, V2(BaseX + 440, BaseY - 20));
    
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, AlbumString, TextColor, &Panel->Album, -0.6f, 0);
    SetPosition(&Panel->Album, V2(BaseX + 500, BaseY - 40));
    
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, YearString, TextColor, &Panel->Year, -0.6f, 0);
    SetPosition(&Panel->Year, V2(BaseX + 440, BaseY - 40));
    
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, GenreString, TextColor, &Panel->Genre, -0.6f, 0);
    SetPosition(&Panel->Genre, V2(BaseX + 440, BaseY - 60));
}

internal void
UpdatePanelTime(renderer *Renderer, playing_song_panel *Panel, r32 CurrentTime)
{
    
    Panel->CurrentTime = (u32)(CurrentTime*1000);
    WipeStringCompound(&Panel->CurrentTimeString);
    MillisecondsToMinutes(Panel->CurrentTime, &Panel->CurrentTimeString);
    AppendStringToCompound(&Panel->CurrentTimeString, (u8 *)" |");
    
    RemoveRenderText(&Panel->CurrentTimeText);
    CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, &Panel->CurrentTimeString, &Panel->MP3Info->MusicInfo->DisplayInfo.ColorPalette.ForegroundText, &Panel->CurrentTimeText, -0.6f, 0);
    SetPosition(&Panel->CurrentTimeText, V2(315+22, 80 + 19));
}

internal void
UpdatePanelTimeline(playing_song_panel *Panel, r32 CurrentTime)
{
    slider *Slider = &Panel->Timeline;
    r32 TimePercentage = Min(1.0f, SaveDiv((CurrentTime*1000), Panel->SongDuration));
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
    timeline_slider_drag *Info = (timeline_slider_drag *)Data;
    music_display_info *DisplayInfo = &Info->GameState->MusicInfo.DisplayInfo;
    slider *Slider = &DisplayInfo->PlayingSongPanel.Timeline;
    
    r32 BGX = GetPosition(Slider->Background).x;
    r32 NewX = Clamp(AdjustedMouseP.x, BGX-Slider->MaxSlidePix, BGX+Slider->MaxSlidePix);
    
    Slider->GrabThing->ID->Transform.Translation.x = NewX;
    r32 TranslationPercentage = SaveDiv(1.0f,(Slider->MaxSlidePix*2))*(NewX-BGX) + 0.5f;
    
    r32 NewPlaytime = (DisplayInfo->PlayingSongPanel.SongDuration/1000.0f)*TranslationPercentage;
    
    PushNewPlayedTime(Info->GameState->SoundThreadInterface, NewPlaytime);
}

internal void
OnTimelineDragEnd(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
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
       IsInRect(DisplayInfo->AlbumSong.Edge, MouseP))
    {
        GameState->CursorState = cursorState_Drag;
    }
    else GameState->CursorState = cursorState_Arrow;
    
}

internal column_type
CheckColumnsForSelectionChange()
{
    column_type Result = columnType_None;
    
    renderer *Renderer = &GlobalGameState.Renderer;
    input_info *Input = &GlobalGameState.Input;
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    music_sorting_info *SortingInfo = &GlobalGameState.MusicInfo.SortingInfo;
    
    if(IsInRect(DisplayInfo->Song.Base.Background, Input->MouseP))
    {
        
        if(GlobalGameState.Time.GameTime - DisplayInfo->Song.Base.LastClickTime < DOUBLE_CLICK_TIME)
        {
            if(SortingInfo->Song.Selected.A.Count > 0)
            {
                file_id FileID = Get(&SortingInfo->Song.Selected, NewSelectID(0));
                i32 OnScreenID = -1;
                if(StackFind(&SortingInfo->Song.Displayable, FileID, &OnScreenID))
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
            if(UpdateSelectionArray(&SortingInfo->Song, &DisplayInfo->Song.Base))
            {
                UpdateColumnColor(&DisplayInfo->Song.Base, &SortingInfo->Song);
            }
        }
        
        DisplayInfo->Song.Base.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Genre.Background, Input->MouseP))
    {
        
        if(GlobalGameState.Time.GameTime - DisplayInfo->Genre.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Genre, &SortingInfo->Genre);
        else Result = UpdateSelectionArray(&SortingInfo->Genre, &DisplayInfo->Genre);
        
        if(Result)
        {
            PropagateSelectionChange(SortingInfo);
            DisplayInfo->Artist.DisplayCursor = 0;
            DisplayInfo->Album.DisplayCursor  = 0;
        }
        
        DisplayInfo->Genre.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Artist.Background, Input->MouseP))
    {
        if(GlobalGameState.Time.GameTime - DisplayInfo->Artist.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Artist, &SortingInfo->Artist);
        else Result = UpdateSelectionArray(&SortingInfo->Artist, &DisplayInfo->Artist);
        
        if(Result)
        {
            PropagateSelectionChange(SortingInfo);
            DisplayInfo->Album.DisplayCursor  = 0;
        }
        
        DisplayInfo->Artist.LastClickTime = GlobalGameState.Time.GameTime;
    }
    else if(IsInRect(DisplayInfo->Album.Background, Input->MouseP))
    {
        if(GlobalGameState.Time.GameTime - DisplayInfo->Album.LastClickTime < DOUBLE_CLICK_TIME)
            Result = SelectAllOrNothing(&DisplayInfo->Album, &SortingInfo->Album);
        else Result = UpdateSelectionArray(&SortingInfo->Album, &DisplayInfo->Album);
        
        DisplayInfo->Album.LastClickTime = GlobalGameState.Time.GameTime;
    }
    
    return Result;
}

inline void
SetQuitAnimation(quit_animation *Quit, b32 Activate)
{
    Quit->AnimationStart = Activate;
    SetActive(Quit->Curtain, Activate);
    SetActive(&Quit->Text, Activate);
}

internal b32
QuitAnimation(r32 Dir)
{
    b32 Result = false;
    
    window_info *Window = &GlobalGameState.Renderer.Window;
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    time_management *Time = &GlobalGameState.Time;
    quit_animation *Quit = &GlobalGameState.MusicInfo.DisplayInfo.Quit;
    if(Quit->dAnim >= 1.0f || Quit->dAnim < 0.0f)
    {
        SetScale(Quit->Curtain, V2(1, 1));
        SetLocalPositionY(Quit->Curtain, Window->CurrentDim.Height/2.0f);
        Result = true;
        Quit->dAnim = 0.0f;
    }
    else 
    {
        Quit->dAnim += Time->dTime/Quit->Time * Dir;
        
        r32 NewYScale = GraphFirstQuickThenSlow(Quit->dAnim);
        SetSize(Quit->Curtain, V2((r32)Window->CurrentDim.Width, Window->CurrentDim.Height*NewYScale));
        SetLocalPosition(Quit->Curtain, V2(Window->CurrentDim.Width/2.0f, 
                                           Window->CurrentDim.Height - GetSize(Quit->Curtain).y/2.0f));
        SetPosition(&Quit->Text, V2((Window->CurrentDim.Width - Quit->Text.CurrentP.x)/2.0f,
                                    Window->CurrentDim.Height - GetSize(Quit->Curtain).y/2.0f));
        
        SetTransparency(Quit->Curtain, Quit->dAnim/4.0f + 0.75f);//Max(0.85f, Quit->dAnim));
    }
    
    return Result;
}

inline void
PushUserErrorMessage(string_c *String)
{
    renderer *Renderer = &GlobalGameState.Renderer;
    user_error_text *ErrorInfo = &GlobalGameState.MusicInfo.DisplayInfo.UserErrorText;
    
    // For each output character we extend the visibility time of the message.
    ErrorInfo->AnimTime = 1.0f + String->Pos*0.1f; 
    
    render_text_atlas *Atlas = Renderer->FontInfo.MediumFont;
    if(String->Pos > 60) Atlas = Renderer->FontInfo.SmallFont;
    
    CreateRenderText(Renderer, Atlas, String,
                     &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.ErrorText,
                     &ErrorInfo->Message, -0.8f, 0);
    SetTransparency(&ErrorInfo->Message, 0);
    ErrorInfo->dAnim = 0;
    ErrorInfo->IsAnimating = true;
}

inline void
AnimateErrorMessage(user_error_text *ErrorInfo, r32 dTime)
{
    if(ErrorInfo->IsAnimating)
    {
        if(ErrorInfo->dAnim >= 1.0f)
        {
            SetActive(&ErrorInfo->Message, false);
            RemoveRenderText(&ErrorInfo->Message);
            ErrorInfo->IsAnimating = false;
        }
        else 
        {
            r32 Alpha = 1-Pow(ErrorInfo->dAnim, 10);
            SetTransparency(&ErrorInfo->Message, Alpha);
            SetPosition(&ErrorInfo->Message, V2(105, GlobalGameState.Renderer.Window.CurrentDim.Height - 16.0f));
            
            ErrorInfo->dAnim += dTime/ErrorInfo->AnimTime;
        }
    }
}









