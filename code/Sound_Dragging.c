#include "Sound_Dragging.h"

internal void
CreateColumnDragEdges(game_state *GS, 
                      column_edge_drag *FirstEdge, 
                      column_edge_drag *SecondEdge, 
                      column_edge_drag *ThirdEdge, 
                      column_edge_drag *FourthEdge)
{
    music_info *MusicInfo = &GS->MusicInfo;
    music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
    drag_list *DragableList = &GS->DragableList;
    r32 BLeft   = GetExtends(DisplayInfo->EdgeLeft).x  + GS->Layout.DragEdgeWidth/2 + GS->Layout.VerticalSliderWidth;
    r32 BRight  = GetExtends(DisplayInfo->EdgeRight).x + GS->Layout.DragEdgeWidth/2 + GS->Layout.VerticalSliderWidth;
    r32 BMiddle = GS->Layout.DragEdgeWidth + GS->Layout.VerticalSliderWidth;
    
    {
        FirstEdge->LeftEdgeChain  = {
            {DisplayInfo->EdgeLeft}, 
            {BLeft}, 
            {}, 
            1
        };
        FirstEdge->RightEdgeChain = {
            {DisplayInfo->GenreArtist.Edge, DisplayInfo->ArtistAlbum.Edge, DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight}, 
            {BMiddle, BMiddle, BMiddle, BRight}, 
            {&DisplayInfo->GenreArtist.XPercent, &DisplayInfo->ArtistAlbum.XPercent, &DisplayInfo->AlbumSong.XPercent}, 
            4 
        };
        FirstEdge->XPercent     = &DisplayInfo->PlaylistsGenre.XPercent;
        FirstEdge->MusicInfo    = MusicInfo;
        AddDragable(DragableList, DisplayInfo->PlaylistsGenre.Edge, {}, {OnDisplayColumnEdgeDrag, FirstEdge}, {});
    }
    
    {
        SecondEdge->LeftEdgeChain  = {
            {DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->EdgeLeft}, 
            {BMiddle, BLeft}, 
            {&DisplayInfo->PlaylistsGenre.XPercent}, 
            2
        };
        SecondEdge->RightEdgeChain = {
            {DisplayInfo->ArtistAlbum.Edge, DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight}, 
            {BMiddle, BMiddle, BRight}, 
            {&DisplayInfo->ArtistAlbum.XPercent, &DisplayInfo->AlbumSong.XPercent}, 
            3
        };
        SecondEdge->XPercent     = &DisplayInfo->GenreArtist.XPercent;
        SecondEdge->MusicInfo    = MusicInfo;
        AddDragable(DragableList, DisplayInfo->GenreArtist.Edge, {}, {OnDisplayColumnEdgeDrag, SecondEdge}, {});
    }
    
    {
        ThirdEdge->LeftEdgeChain  = {
            {DisplayInfo->GenreArtist.Edge, DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->EdgeLeft}, 
            {BMiddle, BMiddle, BLeft}, 
            {&DisplayInfo->GenreArtist.XPercent, &DisplayInfo->PlaylistsGenre.XPercent}, 3
        };
        ThirdEdge->RightEdgeChain = {
            {DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight}, 
            {BMiddle, BRight},
            {&DisplayInfo->AlbumSong.XPercent}, 
            2
        };
        ThirdEdge->XPercent     = &DisplayInfo->ArtistAlbum.XPercent;
        ThirdEdge->MusicInfo    = MusicInfo;
        AddDragable(DragableList, DisplayInfo->ArtistAlbum.Edge, {}, {OnDisplayColumnEdgeDrag, ThirdEdge}, {});
    }
    
    {
        FourthEdge->LeftEdgeChain  = {
            {DisplayInfo->ArtistAlbum.Edge, DisplayInfo->GenreArtist.Edge, DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->EdgeLeft}, 
            {BMiddle, BMiddle, BMiddle, BLeft}, 
            {&DisplayInfo->ArtistAlbum.XPercent, &DisplayInfo->GenreArtist.XPercent, &DisplayInfo->PlaylistsGenre.XPercent}, 
            4
        };
        FourthEdge->RightEdgeChain = {
            {DisplayInfo->EdgeRight}, 
            {BRight}, 
            {}, 
            1
        };
        FourthEdge->XPercent       = &DisplayInfo->AlbumSong.XPercent;
        FourthEdge->MusicInfo      = MusicInfo;
        AddDragable(DragableList, DisplayInfo->AlbumSong.Edge, {}, {OnDisplayColumnEdgeDrag, FourthEdge}, {});
    }
    
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

internal void
ProcessEdgeDragOnResize(renderer *Renderer, music_display_info *DisplayInfo)
{
    r32 CWidth    = (r32)Renderer->Window.CurrentDim.Width;
    r32 NewY      = CenterYBetweenRects(DisplayInfo->EdgeBottom, DisplayInfo->EdgeTop);
    r32 NewHeight = HeightBetweenRects(DisplayInfo->EdgeTop, DisplayInfo->EdgeBottom);
    
    r32 NewXGenreArtist = CWidth*DisplayInfo->GenreArtist.XPercent;
    SetPosition(DisplayInfo->GenreArtist.Edge, V2(NewXGenreArtist, NewY));
    SetSize(DisplayInfo->GenreArtist.Edge, V2(GetSize(DisplayInfo->GenreArtist.Edge).x, NewHeight));
    
    r32 NewXArtistAlbum = CWidth*DisplayInfo->ArtistAlbum.XPercent;
    SetPosition(DisplayInfo->ArtistAlbum.Edge, V2(NewXArtistAlbum, NewY));
    SetSize(DisplayInfo->ArtistAlbum.Edge, V2(GetSize(DisplayInfo->ArtistAlbum.Edge).x, NewHeight));
    
    r32 NewXAlbumSong = CWidth*DisplayInfo->AlbumSong.XPercent;
    SetPosition(DisplayInfo->AlbumSong.Edge, V2(NewXAlbumSong, NewY));
    SetSize(DisplayInfo->AlbumSong.Edge, V2(GetSize(DisplayInfo->AlbumSong.Edge).x, NewHeight));
    
    r32 NewXPlaylistGenre = CWidth*DisplayInfo->PlaylistsGenre.XPercent;
    SetPosition(DisplayInfo->PlaylistsGenre.Edge, V2(NewXPlaylistGenre, NewY));
    SetSize(DisplayInfo->PlaylistsGenre.Edge, V2(GetSize(DisplayInfo->PlaylistsGenre.Edge).x, NewHeight));
}

// ****************************************************************************************
// Slider/Scrollbars drag stuff ***********************************************************
// ****************************************************************************************

internal void
CreateColumnSliders(game_state *GS, 
                    drag_slider_data *GenreDrag,
                    drag_slider_data *ArtistDrag,
                    drag_slider_data *AlbumDrag,
                    drag_slider_data *SongDrag,
                    drag_slider_data *PlaylistsDrag)
{
    music_info           *MusicInfo = &GS->MusicInfo;
    music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
    drag_list         *DragableList = &GS->DragableList;
    
    *GenreDrag     = {MusicInfo, &DisplayInfo->Genre};
    *ArtistDrag    = {MusicInfo, &DisplayInfo->Artist};
    *AlbumDrag     = {MusicInfo, &DisplayInfo->Album};
    *SongDrag      = {MusicInfo, &DisplayInfo->Song.Base};
    *PlaylistsDrag = {MusicInfo, &DisplayInfo->Playlists};
    
    AddDragable(DragableList, 
                DisplayInfo->Genre.SliderHorizontal.Background,  
                {OnSliderDragStart, &DisplayInfo->Genre.SliderHorizontal}, 
                {OnHorizontalSliderDrag, GenreDrag}, 
                {});
    AddDragable(DragableList, 
                DisplayInfo->Artist.SliderHorizontal.Background, 
                {OnSliderDragStart, &DisplayInfo->Artist.SliderHorizontal}, 
                {OnHorizontalSliderDrag, ArtistDrag}, 
                {});
    AddDragable(DragableList, 
                DisplayInfo->Album.SliderHorizontal.Background,  
                {OnSliderDragStart, &DisplayInfo->Album.SliderHorizontal}, 
                {OnHorizontalSliderDrag, AlbumDrag}, 
                {});
    AddDragable(DragableList, 
                DisplayInfo->Song.Base.SliderHorizontal.Background,
                {OnSliderDragStart, &DisplayInfo->Song.Base.SliderHorizontal}, 
                {OnHorizontalSliderDrag, SongDrag},
                {});
    AddDragable(DragableList, 
                DisplayInfo->Playlists.SliderHorizontal.Background,
                {OnSliderDragStart, &DisplayInfo->Playlists.SliderHorizontal}, 
                {OnHorizontalSliderDrag, PlaylistsDrag},
                {});
    
    AddDragable(DragableList, 
                DisplayInfo->Genre.SliderVertical.Background,
                {OnSliderDragStart, &DisplayInfo->Genre.SliderVertical}, 
                {OnVerticalSliderDrag, GenreDrag}, 
                {});
    AddDragable(DragableList, 
                DisplayInfo->Artist.SliderVertical.Background,
                {OnSliderDragStart, &DisplayInfo->Artist.SliderVertical}, 
                {OnVerticalSliderDrag, ArtistDrag}, 
                {});
    AddDragable(DragableList, 
                DisplayInfo->Album.SliderVertical.Background,
                {OnSliderDragStart, &DisplayInfo->Album.SliderVertical}, 
                {OnVerticalSliderDrag, AlbumDrag}, 
                {});
    AddDragable(DragableList, 
                DisplayInfo->Song.Base.SliderVertical.Background,
                {OnSliderDragStart, &DisplayInfo->Song.Base.SliderVertical}, 
                {OnVerticalSliderDrag, SongDrag},
                {OnSongDragEnd, SongDrag});
    AddDragable(DragableList, 
                DisplayInfo->Playlists.SliderVertical.Background,
                {OnSliderDragStart, &DisplayInfo->Playlists.SliderVertical},
                {OnVerticalSliderDrag, PlaylistsDrag}, 
                {});
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

internal void
UpdateColumnHorizontalSlider(display_column *DisplayColumn, u32 DisplayableCount)
{
    slider *Slider = &DisplayColumn->SliderHorizontal;
    
    ResetColumnText(DisplayColumn, DisplayableCount);
    
    if(DisplayColumn->Type == columnType_Song) 
    {
        Slider->OverhangP = 1 + CalcSongTextOverhangPercentage(DisplayColumn, DisplayableCount);
    }
    else Slider->OverhangP = 1 + CalcTextOverhangPercentage(DisplayColumn, DisplayColumn->SlotText, DisplayableCount);
    
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
}

internal void
SongHorizontalSliderDrag(renderer *Renderer, display_column *DisplayColumn, r32 TranslationX, u32 It)
{
    display_column_song *Song = ColumnExt(DisplayColumn);
    
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongTitle);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongArtist);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongAlbum);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongGenre);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongTrack);
    UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationX, It, Song->SongYear);
    
    // Update song buttons
    Translate(Song->Play[It], V2(TranslationX, 0));
}

internal void
OnHorizontalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    drag_slider_data        *Info = (drag_slider_data *)Data;
    display_column *DisplayColumn = Info->DisplayColumn;
    slider                *Slider = &DisplayColumn->SliderHorizontal;
    u32          DisplayableCount = Info->MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable.A.Count;
    
    r32 GrabThingHalfWidth  = GetSize(Slider->GrabThing).x/2.0f;
    if(Slider->MouseOffset.x < GrabThingHalfWidth && Slider->MouseOffset.x > -GrabThingHalfWidth) 
        AdjustedMouseP.x -= Slider->MouseOffset.x;
    
    r32 BGXPos = GetLocalPosition(Slider->Background).x;
    r32 NewX   = Clamp(AdjustedMouseP.x, BGXPos - Slider->MaxSlidePix, BGXPos + Slider->MaxSlidePix);
    r32 OldX   = GetLocalPosition(Slider->GrabThing).x;
    SetLocalPosition(Slider->GrabThing, V2(NewX, GetLocalPosition(Slider->GrabThing).y));
    
    r32 TranslationPercent = SafeDiv(1.0f, Slider->MaxSlidePix*2)*(OldX-NewX);
    r32 TranslationPix = TranslationPercent*DisplayColumn->SlotWidth*(Slider->OverhangP-1);
    for(u32 It = 0;
        It < DisplayColumn->Count && 
        It < DisplayableCount;
        It++)
    {
        if(DisplayColumn->Type == columnType_Song) SongHorizontalSliderDrag(Renderer, DisplayColumn, TranslationPix, It);
        else UpdateOnHorizontalSliderDrag(Renderer, DisplayColumn, TranslationPix, It, DisplayColumn->SlotText);
    }
}

inline void
UpdateColumnVerticalSliderPosition(display_column *DisplayColumn, u32 DisplayableCount)
{
    slider *Slider       = &DisplayColumn->SliderVertical;
    r32 TotalSliderScale = GetScale(Slider->Background).y;
    r32 TotalHeight      = GetDisplayableHeight(DisplayableCount, SlotHeight(DisplayColumn));
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
    
    r32 TotalDisplayableSize = GetDisplayableHeight(DisplayableCount, SlotHeight(DisplayColumn));
    v2 TotalScale            = GetSize(Slider->Background);
    Slider->OverhangP        = Clamp(TotalScale.y/TotalDisplayableSize, 0.01f, 1.0f);
    
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
    drag_slider_data    *DragData = (drag_slider_data *)Data;
    display_column *DisplayColumn = DragData->DisplayColumn;
    slider                *Slider = &DisplayColumn->SliderVertical;
    u32          DisplayableCount = DragData->MusicInfo->Playlist_->Columns[DisplayColumn->Type].Displayable.A.Count;
    
    r32 GrabThingHalfHeight  = GetSize(Slider->GrabThing).y/2.0f;
    if(Slider->MouseOffset.y < GrabThingHalfHeight && Slider->MouseOffset.y > -GrabThingHalfHeight) 
        AdjustedMouseP.y -= Slider->MouseOffset.y;
    
    r32 ParentY = GetLocalPosition(Slider->GrabThing->ID->Parent).y;
    r32 BGYPos  = GetLocalPosition(Slider->Background).y + ParentY;
    r32 NewY    = Clamp(AdjustedMouseP.y, BGYPos - Slider->MaxSlidePix, BGYPos + Slider->MaxSlidePix);
    
    SetLocalPosition(Slider->GrabThing, V2(GetLocalPosition(Slider->GrabThing).x, NewY-ParentY));
    
    r32 TotalSliderScale = GetSize(Slider->Background).y;
    r32 GrabThingSize    = GetSize(Slider->GrabThing).y;
    r32 RemainingScale   = TotalSliderScale-GrabThingSize;
    r32 TopPositionY     = BGYPos+Slider->MaxSlidePix;
    
    r32 TranslationPercentage = SafeDiv(TopPositionY-NewY,Slider->MaxSlidePix*2);
    if(TranslationPercentage > 0.999f) TranslationPercentage = 1;
    r32 TotalHeight = GetDisplayableHeight(DisplayableCount, SlotHeight(DisplayColumn));
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
    DestroyArray(&GlobalGameState.ScratchArena, &IgnoreDecodeIDs);
}

// ****************************************************************************************
// Timeline drag stuff ********************************************************************
// ****************************************************************************************

internal void
CreateTimelineSliders(game_state *GS, timeline_slider_drag *TimelineDrag)
{
    music_info           *MusicInfo = &GS->MusicInfo;
    music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
    drag_list         *DragableList = &GS->DragableList;
    
    *TimelineDrag = {GS, false};
    
    AddDragable(DragableList, 
                DisplayInfo->Volume.Background, 
                {},
                {OnVolumeDrag, GS},
                {});
    // Adding this twice, once for grabbing the small nubs and for the thin bar.
    AddDragable(DragableList, 
                DisplayInfo->PlayingSongPanel.Timeline.GrabThing,
                {OnTimelineDragStart, TimelineDrag},
                {OnTimelineDrag, TimelineDrag},
                {OnTimelineDragEnd, TimelineDrag});
    AddDragable(DragableList, 
                DisplayInfo->PlayingSongPanel.Timeline.Background,
                {OnTimelineDragStart, TimelineDrag},
                {OnTimelineDrag, TimelineDrag},
                {OnTimelineDragEnd, TimelineDrag});
    
}

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

// ****************************************************************************************
// Drag&Drop Slots ************************************************************************
// ****************************************************************************************

inline b32
CompareSelectPair(i32 T1, i32 T2, void *Data)
{
    select_pair *Array = (select_pair *)Data;
    b32 Result = (Array[T2].PID > Array[T1].PID);
    return Result;
}

inline void
SwitchSelectPair(void *Array, i32 IndexA, i32 IndexB)
{
    select_pair *A = (select_pair *)Array;
    select_pair TMP = A[IndexB];
    A[IndexB] = A[IndexA];
    A[IndexA] = TMP;
}

inline b32
CompareSelectItem(i32 T1, i32 T2, void *Data)
{
    select_array *Array = (select_array *)Data;
    b32 Result = (Array->DID[T2] > Array->DID[T1]);
    return Result;
}

inline void
SwitchSelectItem(void *Array, i32 IndexA, i32 IndexB)
{
    select_array *A = (select_array *)Array;
    i32 TMP = A->SID[IndexB].ID;
    A->SID[IndexB] = A->SID[IndexA];
    A->SID[IndexA].ID = TMP;
    
    TMP = A->DID[IndexB].ID;
    A->DID[IndexB] = A->DID[IndexA];
    A->DID[IndexA].ID = TMP;
    
    TMP = A->PID[IndexB].ID;
    A->PID[IndexB] = A->PID[IndexA];
    A->PID[IndexA].ID = TMP;
}

internal select_array
CreateOrderedSelectArray(arena_allocator *Arena, array_playlist_id *Selected, array_playlist_id *Displayable)
{
    // Because just creating the SelectIDOrder array takes a really long time, 
    // if both Selected and Displayable arrays are large -as we have to search
    // for each entry in Selected through Displayable and find it there- we 
    // made a small algorithm which is much quicker. 
    // (1.) It essentially copies both arrays, sorts them both, and then 
    // (2.) goes through the selected array in reverse order always checking if the
    // last entry in displayable is larger or the same and (3.) shrinks the displayable
    // array by one everytime until it is the searched-for value (4.). The sorting ensures
    // that everything that does not match (in displayable) is not in the selected 
    // array. This makes it essentially linear and goes at maximum through each array 
    // once. 
    // Additional note: (5.) We need to know the initial location in Displayable, 
    // thats why we copy a pair into the new copy-displayable-array.
    select_array Result = {};
    Result.SID = AllocateArray(Arena, Selected->A.Count, select_id);
    Result.DID = AllocateArray(Arena, Selected->A.Count, displayable_id);
    Result.PID = AllocateArray(Arena, Selected->A.Count, playlist_id);
    
    // 1. ---
    array_playlist_id SelectedCpy;
    SelectedCpy.A = Copy(Arena, Selected->A);
    QuickSort3(SelectedCpy.A);
    
    u32 DisplayableCount         = Displayable->A.Count;
    select_pair *DisplayablePair = AllocateArray(Arena, DisplayableCount, select_pair);
    For(DisplayableCount, Fill) // 5. ---
        DisplayablePair[FillIt] = {NewDisplayableID(FillIt), Get(Displayable, NewDisplayableID(FillIt))};
    For(DisplayableCount, Shuffle) 
        SwitchSelectPair(DisplayablePair, ShuffleIt, (i32)(Random01()*DisplayableCount));
    QuickSort(0, DisplayableCount-1, DisplayablePair, {CompareSelectPair, DisplayablePair, SwitchSelectPair});
    
    // 2. ---
    i32 ReverseCount = Selected->A.Count;
    for(i32 SelIt = Selected->A.Count-1; SelIt >= 0; --SelIt)
    {
        select_id SelectID      = NewSelectID(SelIt);
        playlist_id SelectPLID  = Get(&SelectedCpy, SelectID);
        
        b32 DisplayableEmpty = false;
        while(!DisplayableEmpty && 
              DisplayablePair[DisplayableCount-1].PID >= SelectPLID)
        {
            // 4. ---
            if(DisplayablePair[DisplayableCount-1].PID == SelectPLID)
            {
                // 6. ---
                Result.SID[Result.Count]   = SelectID;
                Result.DID[Result.Count]   = DisplayablePair[DisplayableCount-1].DID;
                Result.PID[Result.Count++] = SelectPLID;
            }
            // 3. ---
            if(--DisplayableCount == 0) DisplayableEmpty = true;
        }
        if(DisplayableEmpty) break;
    }
    
    return Result;
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
            if(!IsInRect(DisplayColumn->SlotBGs[It], Input->MouseP)) continue;
            
            // Caching all needed drag/drop information
            DragDrop->Dragging       = true;
            DragDrop->DragsSelected  = false;
            DragDrop->Type           = DisplayColumn->Type;
            DragDrop->StartMouseP    = Input->MouseP;
            DragDrop->ShakeThreshold = GetDistance(DisplayColumn->SlotBGs[It], DisplayInfo->Playlists.SlotBGs[0])/DragDrop->ShakeThresholdDistanceDivisor;
            
            drag_drop_slot *Slot = DragDrop->Slots+DragDrop->SlotCount++;
            Slot->DistToBaseSlot = 0;
            Slot->SlotID         = DisplayColumn->SlotIDs[It];
            Slot->SlotStartP     = GetPosition(DisplayColumn->SlotBGs[It]);
            
            render_text *ToCopy = 0;
            if(DisplayColumn->Type == columnType_Song) ToCopy = ColumnExt(DisplayColumn)->SongTitle+It;
            else ToCopy = DisplayColumn->SlotText+It;
            CopyRenderText(GS, ToCopy, &Slot->SlotText, -0.9999f);
            SetTransparency(&Slot->SlotText, 0.0f);
            
            Slot->DragSlot     = Copy(&GS->Renderer, DisplayColumn->SlotBGs[It]);
            Slot->TextOverhang = CalcTextOverhang(DisplayColumn->SlotBGs[It], *ToCopy)+20;
            SetSize(Slot->DragSlot, GetSize(Slot->DragSlot)+V2(Slot->TextOverhang, 0));
            SetDepth(Slot->DragSlot, GetDepth(Slot->SlotText.Base) + 0.0001f);
            SetTransparency(Slot->DragSlot, 0.0f);
            SetParent(Slot->DragSlot, NULL);
            
            Translate(&Slot->SlotText, V2(-Slot->TextOverhang*0.5f, 0));
            SetParent(Slot->SlotText.Base, Slot->DragSlot);
            Slot->SlotStartP.x += Slot->TextOverhang*0.5f;
            Slot->GrabOffset = (Slot->SlotStartP - DragDrop->StartMouseP);
            
            // If the grabbed slot is a selected slot, then we pull all other selected slots as well.
            // But we visualize max 7, to keep it reasonable.
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
                
                RestartTimer("SortDragDrop");
                select_array SelectIDOrder = CreateOrderedSelectArray(&GS->ScratchArena, 
                                                                      &PlaylistColumn->Selected, 
                                                                      &PlaylistColumn->Displayable);
                
                For(SelectIDOrder.Count, Ran) SwitchSelectItem(&SelectIDOrder, RanIt, (i32)(Random01()*SelectIDOrder.Count));
                QuickSort(0, SelectIDOrder.Count-1, &SelectIDOrder, {CompareSelectItem, &SelectIDOrder, SwitchSelectItem});
                
                
                SnapTimer("SortDragDrop");
                i32 StartIt = -1;
                i32 PickSelectIt = -1;
                r32 PositionOffsetY = GetSize(Slot->DragSlot).y + GS->Layout.SlotGap;
                i32 HalfDragSlot = Floor(MAX_DRAG_SLOT_VISUALS/2.0f);
                For(SelectIDOrder.Count, S) // Get the one that was mouse picked
                {
                    if(PLID == SelectIDOrder.PID[SIt]) 
                    {
                        PickSelectIt = SIt;
                        if(SIt+HalfDragSlot > SelectIDOrder.Count) 
                            StartIt = Max(0, (i32)SelectIDOrder.Count-(i32)MAX_DRAG_SLOT_VISUALS);
                        else StartIt = Max(0, (i32)SIt-HalfDragSlot);
                        break;
                    }
                }
                Assert(PickSelectIt >= 0);
                Assert(StartIt      >= 0);
                
                for(u32 SelectIt = StartIt; 
                    SelectIt < Min(SelectIDOrder.Count, (u32)StartIt+MAX_DRAG_SLOT_VISUALS); 
                    ++SelectIt)
                {
                    if(SelectIDOrder.PID[SelectIt] != PLID)
                    {
                        drag_drop_slot *SubSlot = DragDrop->Slots+DragDrop->SlotCount++;
                        SubSlot->DistToBaseSlot = Abs(PickSelectIt-(i32)SelectIt);
                        SubSlot->SlotID         = SelectIDOrder.DID[SelectIt];
                        SubSlot->SlotStartP     = V2(0, PositionOffsetY*(PickSelectIt-(i32)SelectIt));
                        
                        i32 OnScreenID = GetSlotID(DisplayColumn, SubSlot->SlotID);
                        r32 TextYOffset = 0;
                        if(OnScreenID >= 0)
                        {
                            render_text *ToCopy = 0;
                            if(DisplayColumn->Type == columnType_Song) 
                                ToCopy  = ColumnExt(DisplayColumn)->SongTitle+OnScreenID;
                            else ToCopy = DisplayColumn->SlotText+OnScreenID;
                            
                            CopyRenderText(GS, ToCopy, &SubSlot->SlotText, -0.9999f);
                        }
                        else
                        {
                            font_size_id FontSize = font_Small;
                            string_c *Text        = 0;
                            if(DisplayColumn->Type != columnType_Song)
                            {
                                Text = GetName(PlaylistColumn, SubSlot->SlotID);
                                TextYOffset = GS->Layout.SlotTextYOffPercent*GetFontSize(&GS->Renderer, font_Small).Size;
                            }
                            else 
                            {
                                FontSize = font_Medium;
                                Text = &GetMetadata(PlaylistColumn, &GS->MP3Info->FileInfo, SubSlot->SlotID)->Title;
                                TextYOffset = GetSongRow1YOffset(&GS->Layout, *Text);
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
        
        i32 OnScreenID             = GetSlotID(GS, MusicInfo->Playlists.List+0);
        DragDrop->OriginalAllColor = GetColorPtr(DInfo->Playlists.SlotBGs[OnScreenID]);
        DragDrop->AllRenderText    = DInfo->Playlists.SlotText + OnScreenID;
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
        SetColor(DInfo->Playlists.SlotBGs[OnScreenID], &DragDrop->RemoveColor);
        RemoveRenderText(&GS->Renderer, DragDrop->AllRenderText);
        NewLocalString(RemoveText, 7, "REMOVE");
        RenderText(GS, font_Small, &RemoveText, DInfo->Playlists.Colors.Text, DragDrop->AllRenderText, DInfo->Playlists.ZValue-0.01f, DInfo->Playlists.SlotBGs[OnScreenID]);
        
        r32 Descent = GetFontDescent(GS, font_Small, RemoveText);
        Translate(DragDrop->AllRenderText, V2(0, Descent));
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
            if(IsIntersectingRect(Slot, DisplayColumn->SlotBGs[It])) 
                Push(&IntersectingSlots, It);
        }
        
        i32 ClosestID = -1;
        r32   MinDist = MAX_REAL32;
        v2      SlotP = GetPosition(Slot);
        For(IntersectingSlots.Count)
        {
            u32 ID = Get(&IntersectingSlots, It);
            r32 Dist = Distance(SlotP, GetPosition(DisplayColumn->SlotBGs[ID]));
            if(Dist < MinDist)
            {
                MinDist   = Dist;
                ClosestID = ID;
            }
        }
        if(ClosestID >= 0)
        {
            if(DragDrop->CurHoverID >= 0) 
                SetColor(DisplayColumn->SlotBGs[DragDrop->CurHoverID], DragDrop->CurOriginalColor); 
            
            DragDrop->CurHoverID = ClosestID;
            DragDrop->CurOriginalColor = GetColorPtr(DisplayColumn->SlotBGs[ClosestID]);
            SetColor(DisplayColumn->SlotBGs[ClosestID], &DisplayInfo->ColorPalette.Text);
            
            InSlot = true;
        }
    }
    if(!InSlot && DragDrop->CurHoverID >= 0)
    {
        SetColor(DisplayColumn->SlotBGs[DragDrop->CurHoverID], DragDrop->CurOriginalColor); 
        DragDrop->CurHoverID = -1;
    }
}

internal void
StopDragDrop(game_state *GS, drag_drop *DragDrop)
{
    // Evaluate drop
    if(DragDrop->CurHoverID >= 0)
    {
        SetColor(GS->MusicInfo.DisplayInfo.Playlists.SlotBGs[DragDrop->CurHoverID], DragDrop->CurOriginalColor); 
        
        playlist_id PlaylistID = Get(&GS->MusicInfo.Playlist_->Playlists.Displayable,
                                     GS->MusicInfo.DisplayInfo.Playlists.SlotIDs[DragDrop->CurHoverID]);
        playlist_info *Playlist = GS->MusicInfo.Playlists.List + PlaylistID.ID;
        
        array_u32 DisplayableIDs = {};
        if(DragDrop->DragsSelected)
        {
            // If we have a selection-drop, we need to insert
            // everything that was previously selected in this
            // column into the playlist.
            
            array_playlist_id *Selected    = &GS->MusicInfo.Playlist_->Columns[DragDrop->Type].Selected;
            array_playlist_id *Displayable = &GS->MusicInfo.Playlist_->Columns[DragDrop->Type].Displayable;
            select_array SelectIDOrder = CreateOrderedSelectArray(&GS->ScratchArena, Selected, Displayable);
            DisplayableIDs.Slot   = (u32 *)SelectIDOrder.DID;
            DisplayableIDs.Count  = SelectIDOrder.Count;
            DisplayableIDs.Length = SelectIDOrder.Count;
        }
        
        if(DragDrop->CurHoverID == 0) // 'All' playlist removes slots from playlists...
        {
            if(Playlist != GS->MusicInfo.Playlist_) // Except when we're in itself, then nothing happens.
            {
                if(!DragDrop->DragsSelected) RemoveSlotFromPlaylist (GS, DragDrop->Type, DragDrop->Slots[0].SlotID);
                else                         RemoveSlotsFromPlaylist(GS, DragDrop->Type, DisplayableIDs);
            }
        }
        else if(!DragDrop->DragsSelected) InsertSlotIntoPlaylist (GS, Playlist, DragDrop->Type, DragDrop->Slots[0].SlotID);
        else                              InsertSlotsIntoPlaylist(GS, Playlist, DragDrop->Type, DisplayableIDs);
        
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
        i32 OnScreenID = GetSlotID(GS, GS->MusicInfo.Playlists.List+0);
        
        RemoveRenderText(&GS->Renderer, DragDrop->AllRenderText);
        RenderText(GS, font_Small, GS->MusicInfo.Playlist_->Playlists.Batch.Names+0, DColumn->Colors.Text, DragDrop->AllRenderText, DColumn->ZValue-0.01f, DColumn->SlotBGs[OnScreenID]);
        r32 Descent = GetFontDescent(GS, font_Small, GS->MusicInfo.Playlist_->Playlists.Batch.Names[0]);
        Translate(DragDrop->AllRenderText, V2(0, Descent));
        ResetColumnText(DColumn, GS->MusicInfo.Playlist_->Playlists.Displayable.A.Count);
        
        SetColor(DColumn->SlotBGs[OnScreenID], DragDrop->OriginalAllColor);
        
        DragDrop->OriginalAllColor = NULL;
        DragDrop->AllRenderText    = NULL;
    }
    DragDrop->Dragging = false;
}






































