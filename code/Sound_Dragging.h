/* date = November 14th 2021 9:48 am */
#ifndef _SOUND__DRAGGING_H
#define _SOUND__DRAGGING_H

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
    entry_id *Edge;
    edge_chain LeftEdgeChain;
    edge_chain RightEdgeChain;
    r32 XPercent;
    
    struct music_info *MusicInfo;
};

internal void CreateColumnDragEdges(game_state *GS, r32 MinY, r32 MaxY, 
                                    r32 PlaylistGenreX, 
                                    r32 GenreArtistX, 
                                    r32 ArtistAlbumX, 
                                    r32 AlbumSongX);
internal void OnDisplayColumnEdgeDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);


struct drag_slider_data
{
    struct music_info     *MusicInfo;
    struct display_column *DisplayColumn;
};

internal void OnVerticalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);
internal void OnHorizontalSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);
internal void OnSongDragEnd(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);
inline   void UpdateVerticalSliders(music_info *MusicInfo);
inline   void UpdateHorizontalSliders(music_info *MusicInfo);
inline   void UpdateColumnVerticalSliderPosition(display_column *DisplayColumn, u32 DisplayableCount);


struct timeline_slider_drag
{
    game_state *GameState;
    b32 PausedForDragging;
};

internal void OnTimelineDragStart(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);
internal void OnTimelineDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);
internal void OnTimelineDragEnd(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);

internal void ChangeVolume(game_state *GameState, r32 NewVolume);
internal void OnVolumeDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);

// Drag&Drop ******************

struct drag_drop_slot
{
    displayable_id SlotID;
    v2 SlotStartP;
    entry_id *DragSlot;
    entry_id *Border;
    render_text SlotText;
    r32 TextOverhang;
    v2 GrabOffset;
    u32 DistToBaseSlot;
};

#define MAX_DRAG_SLOT_VISUALS 7u
struct drag_drop
{
    b32 Dragging;
    enum column_type Type;
    v2 StartMouseP;
    drag_drop_slot Slots[MAX_DRAG_SLOT_VISUALS];
    u32 SlotCount = 0;
    r32 TransparencyFalloff = 0.3f;
    b32 DragsSelected = false;
    
    i32 AnimDir = 1;
    r32 dAnim   = 0;
    v2  ShakeDir;
    
    r32 ShakeThresholdDistanceDivisor = 5.0;
    r32 ShakeThreshold; // Depends on the distance to playlist column.
    r32 ShakeMaxRadius = 3.0f;
    r32 ShakeMaxAnimTime = 0.01f;
    r32 MaxTransparency  = 0.95f;
    
    b32 ShakeTransition = false;
    v2 Velocity;
    
    // Drop Info
    i32 CurHoverID = -1;
    v3 *CurOriginalColor;
    
    // Drop delete info
    v3 RemoveColor;
    v3 *OriginalAllColor       = NULL;
    render_text *AllRenderText = NULL;
};

struct select_array 
{ 
    select_id      *SID; 
    displayable_id *DID; 
    playlist_id    *PID;
    u32 Count; 
};

struct select_pair 
{ 
    displayable_id DID; 
    playlist_id PID; 
};


#endif //_SOUND__DRAGGING_H
