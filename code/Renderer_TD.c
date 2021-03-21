#include "Renderer_TD.h"
#include "GameBasics_TD.h"

inline void GetWindowDimensions(HWND Window, v2i *Dim);
inline renderer
InitializeRenderer(game_state *GameState, HWND WindowHandle)
{
    renderer Result = {};
    
    GetWindowDimensions(WindowHandle, &Result.Window.FixedDim.Dim);
    Result.Window.FixedAspect = (r32)Result.Window.FixedDim.Width/(r32)Result.Window.FixedDim.Height;
    Result.Window.CurrentDim = Result.Window.FixedDim;
    
    Result.Window.CurrentAspect        = (r32)Result.Window.CurrentDim.Width/(r32)Result.Window.CurrentDim.Height;
    Result.Window.CurrentReverseAspect = (r32)Result.Window.CurrentDim.Height/(r32)Result.Window.CurrentDim.Width;
    Result.Window.WindowHandle = WindowHandle;
    
    Result.ColorPalette = &GameState->MusicInfo.DisplayInfo.ColorPalette;
    Result.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};//{3.0f/255, 17.0f/255, 3.0f/255, 1};
    Result.DefaultEntryColor = {1, 1, 1};
    
    Result.TransformList  = CreateScreenTransformList(&GameState->FixArena);
    
    Result.RenderEntryList.Entries = AllocateArray(&GameState->FixArena, START_RENDER_ENTRIES, render_entry);
    Result.RenderEntryList.IDs     = AllocateArray(&GameState->FixArena, START_RENDER_ENTRIES, entry_id);
    Result.RenderEntryList.OpenSlots = AllocateArray(&GameState->FixArena, START_RENDER_ENTRIES, b32);
    Result.RenderEntryList.MaxCount = START_RENDER_ENTRIES;
    
    return Result;
}

inline rect_2D
Rect(v2 Min, v2 Max)
{
    rect_2D Result = {Min, Max};
    return Result;
}

inline v2
GetCenter(rect_2D Rect)
{
    v2 Result = Rect.Min + (Rect.Max-Rect.Min)*0.5f;
    return Result;
}

inline v2
GetExtends(rect_2D Rect)
{
    v2 Result = Rect.Max-Rect.Min;
    return Result;
}

inline rect_pe_2D
RectToRectPE(rect Rect)
{
    rect_pe_2D Result = {};
    
    Result.Extends = (Rect.Max - Rect.Min)*0.5f;
    Result.Pos = Rect.Min + Result.Extends;
    
    return Result;
}

inline void
ApplyTransform(render_entry *Entry, v3 *Result)
{
    transform_2D *T = &Entry->Transform;
    v2 Dim    = Entry->Vertice[2].xy - Entry->Vertice[0].xy;
    v2 Center = Entry->Vertice[0].xy + Dim*0.5f;
    
    v2 ParentTranslation = {};
    if(Entry->Parent)
    {
        ParentTranslation = GetPosition(Entry->Parent);
        switch(Entry->FixedTo)
        {
            case fixedTo_None: break;
            case fixedTo_BottomLeft:    // #Through
            case fixedTo_BottomRight:   // #Through
            case fixedTo_BottomCenter:  // #Through
            case fixedTo_TopLeft:       // #Through
            case fixedTo_TopRight:      // #Through
            case fixedTo_TopCenter:     // #Through
            case fixedTo_MiddleLeft:    // #Through 
            case fixedTo_MiddleRight:   // #Through
            case fixedTo_Bottom:        // #Through
            case fixedTo_MiddleY:       // #Through
            case fixedTo_Top:           // #Through
            case fixedTo_Right:         // #Through
            case fixedTo_CenterX:       // #Through
            case fixedTo_Left:          // #Through
            case fixedTo_MiddleCenter: ParentTranslation = {}; break;
            case fixedTo_FixXToGiven_YBottom: // #Through
            case fixedTo_FixXToGiven_YCenter: // #Through
            case fixedTo_FixXToGiven_YTop: 
            {
                T->Translation.x = 0;
                ParentTranslation.y = 0; 
            } break;
            case fixedTo_FixYToGiven_XLeft:   // #Through
            case fixedTo_FixYToGiven_XCenter: // #Through
            case fixedTo_FixYToGiven_XRight:  
            {
                T->Translation.y = 0;
                ParentTranslation.x = 0;
            } break;
            case fixedTo_FixXYToGiven: break;
        }
    }
    
    For(4)
    {
        Result[It] = Entry->Vertice[It];
        
        // Scaling
        Result[It].xy -= Center;
        Result[It].xy  = HadamardProduct(T->Scale, Result[It].xy);
        Result[It].xy += Center;
        // Translation
        Result[It].xy += T->Translation;
        // Rotation
        Result[It].xy  = Rotate(Result[It].xy, T->RotationP, T->Angle);
        
        Result[It].xy += ParentTranslation;
    }
}

inline v3
Color(u8 R, u8 G, u8 B)
{
    v3 Result = {};
    
    Result.r = (r32)R/255.0f;
    Result.g = (r32)G/255.0f;
    Result.b = (r32)B/255.0f;
    
    return Result;
}

internal void
FixUpEntries(render_entry_list *EntryList)
{
    for(i32 It = EntryList->EntryCount-1; It >= 0; It--)
    {
        render_entry *Entry = EntryList->Entries+It;
        if(Entry->Vertice[0].z < -1.0f) 
        {
            *Entry = {};
            EntryList->EntryCount--;
        }
        else 
        {
            Entry->ID->ID = Entry;
        }
    }
}

inline void
UpdateEntryList(render_entry_list *EntryList)
{
    if(EntryList->_SortingNeeded) 
    {
        EntryList->_SortingNeeded = false;
        Quicksort3(EntryList->Entries, EntryList->EntryCount);
        FixUpEntries(EntryList);
    }
}

inline i32 
GetEntryID_ID(renderer *Renderer, entry_id *EntryID)
{
    i32 Result = -1;
    render_entry_list *EntryList = &Renderer->RenderEntryList;
    
    For(EntryList->MaxCount)
    {
        if(EntryID->ID == EntryList->IDs[It].ID)
        {
            Result = It;
            break;
        }
    }
    Assert(Result >= 0);
    
    return Result;
}

internal void
RemoveRenderEntry(renderer *Renderer, entry_id *EntryID)
{
    render_entry_list *EntryList = &Renderer->RenderEntryList;
    Assert(EntryID->ID >= 0);
    i32 EntryID_ID = GetEntryID_ID(Renderer, EntryID);
    if(!EntryList->OpenSlots[EntryID_ID])
    {
        EntryList->OpenSlots[EntryID_ID] = true;
        EntryList->OpenSlotCount++;
        
        EntryID->ID->Vertice[0].z = -2; // If this is set, it will be deleted on next sort
        EntryID->ID = 0;
        
        EntryList->_SortingNeeded = true;
    }
}

internal entry_id *
CreateRenderEntry(renderer *Renderer, v2 Size, r32 Depth, v2 Position = {}, entry_id *Parent = 0)
{
    entry_id *Result = 0;
    
    render_entry_list *EntryList = &Renderer->RenderEntryList;
    if(EntryList->OpenSlotCount > 0)
    {
        For(EntryList->MaxCount)
        {
            if(EntryList->OpenSlots[It])
            {
                Result = EntryList->IDs + It; 
                EntryList->OpenSlots[It] = false;
                EntryList->OpenSlotCount--;
                Assert(Result->ID == 0);
                break;
            }
        }
    }
    else
    {
        Assert(EntryList->IDCount < EntryList->MaxCount);
        Result = EntryList->IDs + EntryList->IDCount++;
    }
    Assert(Result);
    Assert(Depth <= 1.0f && Depth >= -1.0f);
    Assert(EntryList->EntryCount < EntryList->MaxCount);
    
    render_entry *Entry = EntryList->Entries + EntryList->EntryCount;
    Entry->ID = Result;
    Result->ID = Entry;
    EntryList->EntryCount++;
    EntryList->_SortingNeeded = true;
    
    Assert(Result->ID >= 0);
    Assert(Entry);
    
    
    Entry->Type = renderType_NONE;
    Entry->Render = true;
    Entry->Transform = {};
    Entry->Transform.Scale = {1,1};
    Entry->Transform.Translation = Position;
    Size *= 0.5f;
    Entry->Vertice[0] = {-Size.x, -Size.y, Depth};
    Entry->Vertice[1] = {-Size.x,  Size.y, Depth};
    Entry->Vertice[2] = { Size.x,  Size.y, Depth};
    Entry->Vertice[3] = { Size.x, -Size.y, Depth};
    Entry->TexCoords[0] = {0, 0};
    Entry->TexCoords[1] = {0, 1};
    Entry->TexCoords[2] = {1, 1};
    Entry->TexCoords[3] = {1, 0};
    Entry->Parent = Parent;
    Entry->Transparency = 1.0f;
    
    return Result;
}

inline entry_id *
CreateRenderRect(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, v3 *Color)
{
    entry_id *Result = 0;
    
    rect_pe_2D RectPE = RectToRectPE(Rect);
    Result = CreateRenderEntry(Renderer, RectPE.Extends*2, Depth, RectPE.Pos, Parent);
    
    Result->ID->Type = renderType_2DRectangle;
    Result->ID->Color = Color;
    
    return Result;
}

internal entry_id *
CreateRenderRect(renderer *Renderer, v2 Size, r32 Depth, v3 *Color, entry_id *Parent)
{
    entry_id *Result = CreateRenderEntry(Renderer, Size, Depth, {}, Parent);
    
    Result->ID->Type = renderType_2DRectangle;
    Result->ID->Color = Color;
    
    return Result;
}

inline entry_id *
CreateRenderBitmap(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, loaded_bitmap Bitmap)
{
    entry_id *Result = 0;
    
    rect_pe_2D RectPE = RectToRectPE(Rect);
    Result = CreateRenderEntry(Renderer, RectPE.Extends*2, Depth, RectPE.Pos, Parent);
    
    Result->ID->Type = renderType_2DBitmap;
    Result->ID->TexID = CreateGLTexture(Bitmap);
    Result->ID->Color = &Renderer->DefaultEntryColor;
    
    return Result;
}

inline entry_id *
CreateRenderBitmap(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, u32 BitmapID)
{
    entry_id *Result = 0;
    
    rect_pe_2D RectPE = RectToRectPE(Rect);
    Result = CreateRenderEntry(Renderer, RectPE.Extends*2, Depth, RectPE.Pos, Parent);
    
    Result->ID->Type = renderType_2DBitmap;
    Result->ID->TexID = BitmapID;
    Result->ID->Color = &Renderer->DefaultEntryColor;
    
    return Result;
}

internal entry_id *
CreateRenderBitmap(renderer *Renderer, v2 Size, r32 Depth, entry_id *Parent, u32 BitmapID)
{
    entry_id *Result = 0;
    
    Result = CreateRenderEntry(Renderer, Size, Depth, {}, Parent);
    
    Result->ID->Type = renderType_2DBitmap;
    Result->ID->TexID = BitmapID;
    Result->ID->Color = &Renderer->DefaultEntryColor;
    
    return Result;
}

inline entry_id *
CreateRenderBitmap(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, string_c *Path)
{
    entry_id *Result = 0;
    
    rect_pe_2D RectPE = RectToRectPE(Rect);
    Result = CreateRenderEntry(Renderer, RectPE.Extends*2, Depth, RectPE.Pos, Parent);
    
    loaded_bitmap Bitmap = LoadImage_STB(Path->S); 
    Assert(Bitmap.Pixels);
    
    Result->ID->Type = renderType_2DBitmap;
    Result->ID->TexID = CreateGLTexture(Bitmap);
    FreeImage_STB(Bitmap);
    Result->ID->Color = &Renderer->DefaultEntryColor;
    return Result;
}

inline void
SetTransparency(entry_id *Entry, r32 T)
{
    Entry->ID->Transparency = Clamp01(T);
}

// Transform helper *****************************************************************

inline v2
GetSize(entry_id *Entry)
{
    v2 Result = HadamardProduct(GetExtends(Entry->ID->Vertice)*2, Entry->ID->Transform.Scale);
    return Result;
}

inline void
SetSize(entry_id *Entry, v2 SizeInPixel)
{
    Entry->ID->Transform.Scale = HadamardDivision(SizeInPixel, GetExtends(Entry->ID->Vertice)*2);
}

inline v2
GetScale(entry_id *Entry)
{
    v2 Result = Entry->ID->Transform.Scale;
    return Result;
}

inline void
SetScale(entry_id *Entry, v2 Scale)
{
    Entry->ID->Transform.Scale = Scale;
}

inline v2
GetExtends(v3 *RenderRectVertice)
{
    v2 Result = RenderRectVertice[2].xy;
    return Result;
}

inline v2
GetExtends(entry_id *Entry)
{
    return GetExtends(Entry->ID->Vertice);
}

inline rect 
GetRect(entry_id *Entry)
{
    rect Result = {};
    
    v2 P = GetPosition(Entry);
    v2 E = GetExtends(Entry);
    Result.Min = P - E;
    Result.Max = P + E;
    
    return Result;
}

inline v2
GetPosition(entry_id *Entry)
{
    v2 Result = {};
    while(Entry)
    {
        Result += Entry->ID->Transform.Translation;
        Entry = Entry->ID->Parent;
    }
    return Result;
}

inline void
SetPosition(entry_id *Entry, v2 NewTranslation)
{
    v2 ParentTranslation = (Entry->ID->Parent) ? GetPosition(Entry->ID->Parent) : V2(0);
    
    Entry->ID->Transform.Translation = NewTranslation - ParentTranslation;
}

inline void
SetPositionX(entry_id *Entry, r32 X)
{
    SetPosition(Entry, V2(X, GetPosition(Entry).y));
}

inline void
SetPositionY(entry_id *Entry, r32 Y)
{
    SetPosition(Entry, V2(GetPosition(Entry).x, Y));
}

inline v2
GetLocalPosition(entry_id *Entry)
{
    v2 Result = Entry->ID->Transform.Translation;
    return Result;
}

inline void
SetLocalPosition(entry_id *Entry, v2 NewTranslation)
{
    Entry->ID->Transform.Translation = NewTranslation;
}

inline void
SetLocalPositionX(entry_id *Entry, r32 NewX)
{
    Entry->ID->Transform.Translation.x = NewX;
}

inline void
SetLocalPositionY(entry_id *Entry, r32 NewY)
{
    Entry->ID->Transform.Translation.y = NewY;
}

inline void
Translate(entry_id *Entry, v2 TranslationOffset)
{
    Entry->ID->Transform.Translation += TranslationOffset;
}

inline void 
SetActive(entry_id *Entry, b32 Activate)
{
    Entry->ID->Render = Activate;
}

inline b32
IsActive(entry_id *Entry)
{
    return Entry->ID->Render;
}

inline void 
SetColor(entry_id *Entry, v3 *Color)
{
    Entry->ID->Color = Color;
}

inline v3
GetColor(entry_id *Entry)
{
    v3 Result = *Entry->ID->Color;
    return Result;
}

inline void 
SetParent(entry_id *Entry, entry_id *Parent)
{
    Entry->ID->Parent = Parent;
}

// Entry rect relation helper *********************************************************

inline r32
HeightBetweenRects(entry_id *RectA, entry_id *RectB)
{
    r32 Result = 0;
    v3 VA[4];
    ApplyTransform(RectA->ID, VA);
    v3 VB[4];
    ApplyTransform(RectB->ID, VB);
    
    if(VA[0].y > VB[2].y)
    {
        Result = Abs(VB[2].y - VA[0].y);
    }
    else if(VA[2].y <= VB[0].y)
    {
        Result = Abs(VA[2].y - VB[0].y);
    }
    
    return Result;
}

inline r32
WidthBetweenRects(entry_id *RectA, entry_id *RectB)
{
    r32 Result = 0;
    v3 VA[4];
    ApplyTransform(RectA->ID, VA);
    v3 VB[4];
    ApplyTransform(RectB->ID, VB);
    
    if(VA[0].x > VB[2].x)
    {
        Result = Abs(VB[2].x - VA[0].x);
    }
    else if(VA[2].x <= VB[0].x)
    {
        Result = Abs(VA[2].x - VB[0].x);
    }
    
    return Result;
}

inline r32
CenterXBetweenRects(entry_id *LeftRect, entry_id *RightRect)
{
    r32 Result = {};
    v3 VA[4];
    ApplyTransform(LeftRect->ID, VA);
    v3 VB[4];
    ApplyTransform(RightRect->ID, VB);
    
    Result = VA[2].x + (VB[0].x - VA[2].x)*0.5f;
    
    return Result;
}

inline r32
CenterYBetweenRects(entry_id *BottomRect, entry_id *TopRect)
{
    r32 Result = {};
    v3 VA[4];
    ApplyTransform(BottomRect->ID, VA);
    v3 VB[4];
    ApplyTransform(TopRect->ID, VB);
    
    Result = VA[2].y + (VB[0].y - VA[2].y)*0.5f;
    
    return Result;
}

inline b32
IsLowerThanRect(entry_id *Entry, entry_id *Rect)
{
    b32 Result = false;
    v3 VE[4];
    ApplyTransform(Entry->ID, VE);
    v3 VR[4];
    ApplyTransform(Rect->ID, VR);
    
    if(VE[2].y < VR[0].y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32
IsHigherThanRect(entry_id *Entry, entry_id *Rect)
{
    b32 Result = false;
    v3 VE[4];
    ApplyTransform(Entry->ID, VE);
    v3 VR[4];
    ApplyTransform(Rect->ID, VR);
    
    if(VE[0].y > VR[2].y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32
IsTopShowing(entry_id *Entry, entry_id *Rect)
{
    b32 Result = false;
    v3 VE[4];
    ApplyTransform(Entry->ID, VE);
    v3 VR[4];
    ApplyTransform(Rect->ID, VR);
    
    if(VE[2].y > VR[2].y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32
IsBottomShowing(entry_id *Entry, entry_id *Rect)
{
    b32 Result = false;
    v3 VE[4];
    ApplyTransform(Entry->ID, VE);
    v3 VR[4];
    ApplyTransform(Rect->ID, VR);
    
    if(VE[0].y < VR[0].y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32
IsIntersectingRectButTopShowing(entry_id *Entry, entry_id *Rect)
{
    b32 Result = false;
    v3 VE[4];
    ApplyTransform(Entry->ID, VE);
    v3 VR[4];
    ApplyTransform(Rect->ID, VR);
    
    if(VE[0].y < VR[2].y && 
       VE[2].y > VR[2].y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32
IsIntersectingRectButBottomShowing(entry_id *Entry, entry_id *Rect)
{
    b32 Result = false;
    v3 VE[4];
    ApplyTransform(Entry->ID, VE);
    v3 VR[4];
    ApplyTransform(Rect->ID, VR);
    
    if(VE[2].y > VR[0].y && 
       VE[0].y < VR[0].y)
    {
        Result = true;
    }
    
    return Result;
}

inline v2 
ClampToRect(v2 Pos, rect_2D Rect)
{
    v2 Result = {};
    
    Result.x = Clamp(Pos.x, Rect.Min.x, Rect.Max.x);
    Result.y = Clamp(Pos.y, Rect.Min.y, Rect.Max.y);
    
    return Result;
}

inline v2 
ClampToRect(v2 Pos, entry_id *Entry)
{
    rect_2D Rect = ExtractScreenRect(Entry);
    return ClampToRect(Pos, Rect);
}

inline b32
IsInRect(rect_2D Rect, v2 P)
{
    b32 Result = false;
    
    if(P.x < Rect.Max.x && P.x > Rect.Min.x &&
       P.y < Rect.Max.y && P.y > Rect.Min.y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32
IsInRect(entry_id *Entry, v2 P)
{
    b32 Result = false;
    
    rect_2D Rect = ExtractScreenRect(Entry);
    Result = IsInRect(Rect, P);
    
    return Result;
}

inline b32 
IsInRect(rect Rect1,  rect Rect2)
{
    b32 Result = false;
    
    if(Rect1.Min.x <= Rect2.Min.x && Rect1.Min.y <= Rect2.Min.y &&
       Rect1.Max.x >= Rect2.Max.x && Rect1.Max.y >= Rect2.Max.y)
    {
        Result = true;
    }
    
    return Result;
}

inline b32 
IsInRect(rect Rect1,  entry_id *Entry)
{
    b32 Result = false;
    
    rect Rect2 = ExtractScreenRect(Entry);
    Result = IsInRect(Rect1, Rect2);
    
    return Result;
}

inline rect
ExtractScreenRect(entry_id *Entry)
{
    rect Result = {};
    
    v3 AdjustedCorners[4];
    ApplyTransform(Entry->ID, AdjustedCorners);
    
    Result.Min = AdjustedCorners[0].xy;
    Result.Max = AdjustedCorners[2].xy;
    
    return Result;
}

inline r32
DistanceToRectEdge(entry_id *Entry, v2 Point)
{
    r32 Result = MAX_REAL32;
    
    rect Rect = ExtractScreenRect(Entry);
    
    v2 Smaller = Rect.Min - Point;
    v2 Bigger = Point - Rect.Max;
    
    if(Smaller.x > 0) Result = Smaller.x;
    if(Smaller.y > 0 && Smaller.y < Result) Result = Smaller.y;
    if(Bigger.x  > 0 && Bigger.x  < Result)  Result = Bigger.x;
    if(Bigger.y  > 0 && Bigger.y  < Result)  Result = Bigger.y;
    if(Result == MAX_REAL32) Result = 0;
    
    return Result;
}


// Auto Screen transform *************************************************

inline screen_transform_list
CreateScreenTransformList(arena_allocator *Arena, u32 Size)
{
    screen_transform_list Result = {};
    Result.Entries       = AllocateArray(Arena, Size, entry_id *);
    Result.FixToPosition = AllocateArray(Arena, Size, v2);
    Result.OriginalPosition = AllocateArray(Arena, Size, v2);
    Result.DoTranslation = AllocateArray(Arena, Size, fixed_to);
    Result.DoScale       = AllocateArray(Arena, Size, scale_axis);
    Result.OpenSlots     = AllocateArray(Arena, Size, b32);
    Result.MaxCount      = Size;
    
    return Result;
}

inline void
RemoveFromTransformList(screen_transform_list *List, entry_id *Entry)
{
    For(List->Count)
    {
        if(List->Entries[It] == Entry)
        {
            List->OpenSlots[It] = true;
            List->OpenSlotCount++;
            break;
        }
    }
}

inline u32
TransformWithScreen(screen_transform_list *List, entry_id *Entry, fixed_to FixedTo, scale_axis ScaleAxis, v2 FixToPosition)
{
    i32 ID = -1;
    if(List->OpenSlotCount > 0)
    {
        For(List->Count)
        {
            if(List->OpenSlots[It])
            {
                ID = It;
                List->OpenSlots[It] = false;
                List->OpenSlotCount--;
                break;
            }
        }
    }
    else 
    {
        Assert(List->Count < List->MaxCount);
        ID = List->Count++;
    }
    
    List->Entries[ID]          = Entry;
    List->FixToPosition[ID]    = FixToPosition;
    List->OriginalPosition[ID] = GetPosition(Entry);
    List->DoTranslation[ID]    = FixedTo;
    List->DoScale[ID]          = ScaleAxis;
    return ID;
}

inline u32 
TransformWithScreen(screen_transform_list *List, entry_id *Entry, fixed_to FixedTo, scale_axis ScaleAxis, r32 FixToPosition)
{
    return TransformWithScreen(List, Entry, FixedTo, ScaleAxis, {FixToPosition, FixToPosition});
}

inline u32 
TranslateWithScreen(screen_transform_list *List, entry_id *Entry, fixed_to FixedTo, r32 FixToPosition)
{
    return TransformWithScreen(List, Entry, FixedTo, scaleAxis_None, {FixToPosition, FixToPosition});
}

inline u32 
TranslateWithScreen(screen_transform_list *List, entry_id *Entry, fixed_to FixedTo, v2  FixToPosition)
{
    return TransformWithScreen(List, Entry, FixedTo, scaleAxis_None, FixToPosition);
}

inline u32
ScaleWithScreen(screen_transform_list *List, entry_id *Entry, scale_axis ScaleAxis)
{
    return TransformWithScreen(List, Entry, fixedTo_None, ScaleAxis);
}

inline void 
ChangeFixToPosition(screen_transform_list *List, u32 ID, r32 NewFixToPosition)
{
    Assert(ID < List->Count);
    List->FixToPosition[ID].x = NewFixToPosition;
    List->FixToPosition[ID].y = NewFixToPosition;
}

inline void 
ChangeFixToPosition(screen_transform_list *List, u32 ID, v2 NewFixToPosition)
{
    Assert(ID < List->Count);
    List->FixToPosition[ID] = NewFixToPosition;
}

internal void
PerformScreenTransform(renderer *Renderer)
{
    v2 FixedDim   = V2(Renderer->Window.FixedDim.Dim);
    v2 CurrentDim = V2(Renderer->Window.CurrentDim.Dim);
    
    screen_transform_list *List = &Renderer->TransformList;
    
    For(List->Count)
    {
        if(List->OpenSlots[It]) continue;
        Assert(List->DoScale[It] != scaleAxis_None || List->DoTranslation[It] != fixedTo_None);
        entry_id *Entry = List->Entries[It];
        if(List->DoTranslation[It] != fixedTo_None)
        {
            Entry->ID->FixedTo = List->DoTranslation[It];
            v2 OriginalT = Entry->ID->Transform.Translation;
            v2 Center = List->OriginalPosition[It];
            v2 AnchorP = {0.5f, 0.5f};
            switch(List->DoTranslation[It])
            {
                case fixedTo_MiddleY: // Through
                case fixedTo_CenterX: // Through
                case fixedTo_MiddleCenter: break;
                case fixedTo_Bottom: // Through
                case fixedTo_BottomLeft:      AnchorP = {0.0f, 0.0f}; break;
                case fixedTo_BottomRight:     AnchorP = {1.0f, 0.0f}; break;
                case fixedTo_BottomCenter:    AnchorP = {0.5f, 0.0f}; break;
                case fixedTo_Top: // Through
                case fixedTo_TopLeft:         AnchorP = {0.0f, 1.0f}; break;
                case fixedTo_TopRight:        AnchorP = {1.0f, 1.0f}; break;
                case fixedTo_TopCenter:       AnchorP = {0.5f, 1.0f}; break;
                case fixedTo_Left: // Through
                case fixedTo_MiddleLeft:      AnchorP = {0.0f, 0.5f}; break;
                case fixedTo_Right: // Through
                case fixedTo_MiddleRight:     AnchorP = {1.0f, 0.5f}; break;
                case fixedTo_FixXToGiven_YBottom: AnchorP = {List->FixToPosition[It].x, 0.0f}; break;   
                case fixedTo_FixXToGiven_YCenter: AnchorP = {List->FixToPosition[It].x, 0.5f}; break;
                case fixedTo_FixXToGiven_YTop:    AnchorP = {List->FixToPosition[It].x, 1.0f}; break;
                case fixedTo_FixYToGiven_XLeft:   AnchorP = {0.0f, List->FixToPosition[It].y}; break;
                case fixedTo_FixYToGiven_XCenter: AnchorP = {0.5f, List->FixToPosition[It].y}; break;
                case fixedTo_FixYToGiven_XRight:  AnchorP = {1.0f, List->FixToPosition[It].y}; break;
                case fixedTo_FixXYToGiven:        AnchorP = List->FixToPosition[It]; break;
                InvalidDefaultCase;
            }
            
            v2 CurrentCenter = HadamardProduct(AnchorP, CurrentDim);
            v2 FixedCenter   = HadamardProduct(AnchorP, FixedDim);
            v2 Result        = CurrentCenter - (FixedCenter - Center);
            
            Entry->ID->Transform.Translation = Result;
            switch(List->DoTranslation[It])
            {
                case fixedTo_Bottom: 
                case fixedTo_MiddleY: 
                case fixedTo_Top: Entry->ID->Transform.Translation.x = OriginalT.x; break;
                case fixedTo_Left: 
                case fixedTo_CenterX: 
                case fixedTo_Right: Entry->ID->Transform.Translation.y = OriginalT.y; break;
            }
        }
        
        if(List->DoScale[It] != scaleAxis_None)
        {
            v2 NewSize = GetExtends(Entry)*2;
            if(List->DoScale[It] == scaleAxis_X ||
               List->DoScale[It] == scaleAxis_XY)
            {
                r32 LeftDist  = Entry->ID->Vertice[0].x;
                r32 RightDist = FixedDim.x - Entry->ID->Vertice[2].x;
                NewSize.x = Max(0.0f, CurrentDim.x - (LeftDist + RightDist));
            }
            if(List->DoScale[It] == scaleAxis_Y ||
               List->DoScale[It] == scaleAxis_XY)
            {
                r32 DownDist  = Entry->ID->Vertice[0].y;
                r32 UpDist    = FixedDim.y - Entry->ID->Vertice[2].y;
                NewSize.y = Max(0.0f, CurrentDim.y - (DownDist + UpDist));
            }
            SetSize(Entry, NewSize);
        }
    }
}

// Render Text********************************************************

inline void
SetPosition(render_text *Text, v2 P)
{
    if(Text->Base == 0) return;
    SetPosition(Text->Base, P);
}

inline void
SetPositionX(render_text *Text, r32 X)
{
    if(Text->Base == 0) return;
    SetPosition(Text->Base, V2(X, GetPosition(Text->Base).y));
}

inline void
SetPositionY(render_text *Text, r32 Y)
{
    if(Text->Base == 0) return;
    SetPosition(Text->Base, V2(GetPosition(Text->Base).x, Y));
}

inline void
SetLocalPosition(render_text *Text, v2 P)
{
    if(Text->Base == 0) return;
    SetLocalPosition(Text->Base, P);
}

inline void
Translate(render_text *Text, v2 Translation)
{
    if(Text->Base == 0) return;
    Translate(Text->Base, Translation);
}

inline v2
GetPosition(render_text *Text, u32 LetterID)
{
    Assert(LetterID < Text->Count);
    v2 Result = {};
    entry_id EntryID = {Text->RenderEntries+LetterID};
    Result = GetPosition(&EntryID);
    return Result;
}

inline void
SetActive(render_text *Text, b32 Render)
{
    if(Text->Base == 0) return;
    Text->Base->ID->Render = Render;
    For(Text->Count)
    {
        Text->RenderEntries[It].Render = Render;
    }
}

inline void 
RemoveRenderText(renderer *Renderer, render_text *Text)
{
    Text->Count = 0;
    if(Text->Base) 
    {
        RemoveRenderEntry(Renderer, Text->Base);
        Text->Base = 0;
    }
}

inline void
SetTransparency(render_text *Text, r32 T)
{
    For(Text->Count)
    {
        Text->RenderEntries[It].Transparency = Clamp01(T);
    }
}

inline void 
SetColor(render_text *Text, v3 *Color)
{
    For(Text->Count)
    {
        Text->RenderEntries[It].Color = Color;
    }
}

inline void
CenterText(render_text *Text)
{
    r32 NewY = Abs(Text->Height.E[0]) + Abs(Text->Height.E[1]);
    NewY /= 2;
    
    Translate(Text, V2(0, CeilingR32(NewY)-7));
}

// *******************  Sorting algorithms ***********************
// 3-Way Quicksort ***********************************************

#define GetDepth3(Entry) (Entry).Vertice[0].z

inline void
Swap(render_entry *A, render_entry *B)
{
    render_entry T = *A;
    *A = *B;
    *B = T;
}

// This function partitions a[] in three parts 
// a) a[l..i] contains all elements smaller than pivot 
// b) a[i+1..j-1] contains all occurrences of pivot 
// c) a[j..r] contains all elements greater than pivot
internal void
Partition(render_entry *Entries, i32 l, i32 r, i32 *i, i32 *j) 
{ 
    *i = l-1;
    *j = r; 
    i32 p = l-1;
    i32 q = r; 
    r32 v = GetDepth3(Entries[r]); 
    
    while (true) 
    { 
        // From left, find the first element smaller than 
        // or equal to v. This loop will definitely terminate 
        // as v is last element 
        while (GetDepth3(Entries[++(*i)]) > v) ; 
        
        // From right, find the first element greater than or 
        // equal to v 
        while (v > GetDepth3(Entries[--(*j)])) 
            if (*j == l) 
            break; 
        
        // If i and j cross, then we are done 
        if (*i >= *j) break; 
        
        // Swap, so that smaller goes on left greater goes on right 
        Swap(Entries+ *i, Entries+ *j); 
        
        // Move all same left occurrence of pivot to beginning of 
        // array and keep count using p 
        if (GetDepth3(Entries[*i]) == v) 
        { 
            p++; 
            Swap(Entries + p, Entries + *i); 
        } 
        
        // Move all same right occurrence of pivot to end of array 
        // and keep count using q 
        if (GetDepth3(Entries[*j]) == v) 
        { 
            q--; 
            Swap(Entries + *j, Entries + q); 
        } 
    } 
    
    // Move pivot element to its correct index 
    Swap(Entries+*i, Entries+r); 
    
    // Move all left same occurrences from beginning 
    // to adjacent to arr[i] 
    *j = (*i)-1; 
    for (i32 k = l; k < p; k++, (*j)--) 
        Swap(Entries+k, Entries+*j); 
    
    // Move all right same occurrences from end 
    // to adjacent to arr[i] 
    *i = (*i)+1; 
    for (i32 k = r-1; k > q; k--, (*i)++) 
        Swap(Entries+*i, Entries+k); 
} 

// 3-way partition based quick sort 
inline void 
Quicksort3Recurse(render_entry *Entries, i32 l, i32 r) 
{ 
    if (r <= l) return; 
    
    i32 i, j; 
    
    // Note that i and j are passed as reference 
    Partition(Entries, l, r, &i, &j); 
    
    // Recur 
    Quicksort3Recurse(Entries, l, j); 
    Quicksort3Recurse(Entries, i, r); 
}

inline void 
Quicksort3(render_entry *Entries, i32 Count) 
{
    Quicksort3Recurse(Entries, 0, Count-1);
}








