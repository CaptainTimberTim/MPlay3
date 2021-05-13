#pragma once

struct rect_2D
{
    v2 Min; 
    v2 Max; 
};
typedef rect_2D rect;

struct rect_pe_2D
{
    v2 Pos;
    v2 Extends;
};
typedef rect_pe_2D rect_pe;

enum render_type
{
    renderType_NONE,
    renderType_2DBitmap,
    renderType_2DRectangle,
    renderType_Text,
};

struct transform_2D
{
    v2 Scale;
    v2 Translation;
    
    v2 RotationP; // TODO:: DELETE
    r32 Angle;
};

struct entry_id
{
    struct render_entry *ID;
    u32 UID;
};

struct render_entry
{
    entry_id *ID;
    render_type Type;
    b32 Render;
    
    // NOTE:: Vertice[0].z will be set to < -1 when marked for deletion.
    // It will then be deleted on the next render!
    v3 Vertice[4]; // Start bottom-left and goes clockwise
    v2 TexCoords[4];
    transform_2D Transform;
    
    enum fixed_to FixedTo;
    
    entry_id *Parent; 
    
    v3 *Color;
    r32 Transparency;
    
    // render type specific stuff
    u32 TexID;
    struct render_text *Text; // NOTE:: Having the text not in the list is propably not the best for the cache...
    u32 TextCodepoint; // TEMP:: Only for debugging. This is render_text specific
    
    // TODO:: Cache the GLSpace data and us it in (isIn.. functions)
};

#define CHARACTERS_PER_TEXT_INFO 256u
struct render_text
{
    v2 CurrentP;
    entry_id *Base; // StartP is the position of base parent
    render_entry *RenderEntries;
    u32 Count;
    u32 MaxCount;
    v2 Extends; // TODO:: Width might not be correct!
};

enum axis
{
    XAxis, 
    YAxis, 
    ZAxis
};

union dimensions
{
    struct
    {
        i32 Width;
        i32 Height;
    };
    v2i Dim;
    i32 D[2];
};

struct window_info
{
    HWND WindowHandle;
    
    dimensions CurrentDim;
    r32 CurrentAspect;  // width/height
    r32 CurrentReverseAspect; // height/width
    
    dimensions FixedDim;
    r32 FixedAspect;
    
    b32 GotResized;
};

enum scale_axis
{
    scaleAxis_None,
    scaleAxis_XY,
    scaleAxis_X,
    scaleAxis_Y,
};

enum fixed_to
{
    fixedTo_None,
    
    fixedTo_BottomLeft,
    fixedTo_BottomRight,
    fixedTo_BottomCenter,
    
    fixedTo_TopLeft,
    fixedTo_TopRight,
    fixedTo_TopCenter,
    
    fixedTo_MiddleLeft,
    fixedTo_MiddleRight,
    fixedTo_MiddleCenter,
    
    // NOTE:: If the render_entry is fixed_to the screen _and_ has a parent, 
    // it is always the parent translation used, instead of the fixed_to translation.
    // With the exception of the fixedTo_Original tags, where the corresponding 
    // _original_ is the parent translation and the other axis is the fixed_to translation. TODO::Outdaten?
    fixedTo_FixXToGiven_YBottom,
    fixedTo_FixXToGiven_YCenter,
    fixedTo_FixXToGiven_YTop,
    
    fixedTo_FixYToGiven_XLeft,
    fixedTo_FixYToGiven_XCenter,
    fixedTo_FixYToGiven_XRight,
    
    fixedTo_FixXYToGiven,
    
    // These will only change the the given axis
    fixedTo_Bottom,
    fixedTo_MiddleY,
    fixedTo_Top,
    
    fixedTo_Right,
    fixedTo_CenterX,
    fixedTo_Left,
};

struct screen_transform_list
{
    entry_id   **Entries;
    v2         *FixToPosition; // Between 0-1
    v2         *OriginalPosition;
    fixed_to   *DoTranslation;
    scale_axis *DoScale;
    u32 MaxCount;
    u32 Count;
    
    u32 OpenSlotCount;
    b32 *OpenSlots;
};

#define START_RENDER_ENTRIES 12000
struct render_entry_list
{
    render_entry *Entries; // This array will be sorted, and changes everytime a new entry is added.
    u32 EntryCount;
    entry_id *IDs;          // This has a fixed list. IDs will not be moved around.
    u32 IDCount;
    
    u32 MaxCount;
    
    u32 OpenSlotCount;
    b32 *OpenSlots;
    
    b32 _SortingNeeded;
};

#include "UI_TD.h"

struct renderer
{
    b32 Rerender;
    b32 Minimized;
    
    u32 UIDCounter;
    render_entry_list RenderEntryList;
    screen_transform_list TransformList;
    
    window_info Window;
    
    font_atlas FontAtlas;
    button_group ButtonGroup;
    
    union color_palette *ColorPalette;
    v4 BackgroundColor;
    v3 DefaultEntryColor;
    
    loaded_bitmap ButtonBase;
    u32 ButtonBaseID;
    button_colors ButtonColors;
    u32 ButtonIconCount;
};

inline v3 Color(u8 R, u8 G, u8 B);

inline rect Rect(v2 Min, v2 Max);
inline v2 GetCenter(rect Rect);
inline v2 GetExtends(rect Rect);
inline rect_pe RectToRectPE(rect Rect);

inline void   UpdateEntryList(render_entry_list *EntryList);
internal void FixUpEntries(render_entry_list *EntryList);
inline void   Quicksort3(render_entry *Entries, i32 Count);

internal entry_id *CreateRenderRect(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, v3 *Color);
internal entry_id *CreateRenderRect(renderer *Renderer, v2 Size, r32 Depth, v3 *Color, entry_id *Parent = 0);
internal entry_id *CreateRenderBitmap(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, loaded_bitmap Bitmap);
internal entry_id *CreateRenderBitmap(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, u32 BitmapID);
internal entry_id *CreateRenderBitmap(renderer *Renderer, v2 Size, r32 Depth, entry_id *Parent, u32 BitmapID);
internal entry_id *CreateRenderBitmap(renderer *Renderer, rect Rect, r32 Depth, entry_id *Parent, string_c *Path);
internal void RemoveRenderEntry(renderer *Renderer, entry_id *EntryID);

internal entry_id *Copy(renderer *Renderer, entry_id *Entry);

inline void SetTransparency(entry_id *Entry, r32 T);
inline void ApplyTransform(render_entry *Entry, v3 *Result);

inline v2   GetSize(entry_id *Entry); // Get Raw + Transform
inline void SetSize(entry_id *Entry, v2 SizeInPixel); // Set Transform
inline v2   GetScale(entry_id *Entry); // Get Transform
inline void SetScale(entry_id *Entry, v2 Scale); // Set Transform
inline v2   GetExtends(v3 *RenderRectVertice); // Get Raw
inline v2   GetExtends(entry_id *Entry); // Get Raw
inline rect GetRect(entry_id *Entry);

inline v2   GetPosition(entry_id *Entry); // Get Raw + global Transform
inline void SetPosition(entry_id *Entry, v2 Position); // Set Global Transform
inline void SetPositionX(entry_id *Entry, r32 X);
inline void SetPositionY(entry_id *Entry, r32 Y);
inline v2   GetLocalPosition(entry_id *Entry); // Get Raw + Transform
inline void SetLocalPosition(entry_id *Entry, v2 Position); // Set Transform
inline void SetLocalPositionY(entry_id *Entry, r32 NewY);
inline void SetLocalPositionX(entry_id *Entry, r32 NewX);
inline void Translate(entry_id *Entry, v2 TranslationOffset); // Move Transform

inline void SetActive(entry_id *Entry, b32 Activate);
inline b32  IsActive(entry_id *Entry);
inline void SetColor(entry_id *Entry, v3 *Color);
inline v3   GetColor(entry_id *Entry);
inline v3  *GetColorPtr(entry_id *Entry);
inline void SetParent(entry_id *Entry, entry_id *Parent);
inline entry_id *GetParent(entry_id *Entry);
inline r32  GetDepth(entry_id *Entry);
inline void SetDepth(entry_id *Entry, r32 Depth);

inline r32  GetDistance(entry_id *E1, entry_id *E2);

// Helper for entry relations
inline rect ExtractScreenRect(entry_id *Entry);
inline r32 DistanceToRectEdge(entry_id *Entry, v2 Point);
inline b32 IsInRect(rect_2D Rect, v2 P);
inline b32 IsInRect(entry_id *Entry, v2 P);
inline b32 IsInRect(rect Rect1,  rect Rect2);
inline b32 IsInRect(rect Rect1,  entry_id *Entry);

inline b32 IsLowerThanRect(entry_id *Entry, entry_id *Rect);
inline b32 IsHigherThanRect(entry_id *Entry, entry_id *Rect);
inline b32 IsTopShowing(entry_id *Entry, entry_id *Rect);
inline b32 IsBottomShowing(entry_id *Entry, entry_id *Rect);
inline b32 IsIntersectingRectButTopShowing(entry_id *Entry, entry_id *Rect);
inline b32 IsIntersectingRectButBottomShowing(entry_id *Entry, entry_id *Rect);

inline r32 HeightBetweenRects(entry_id *RectA, entry_id *RectB);
inline r32 WidthBetweenRects(entry_id *RectA, entry_id *RectB);
inline r32 CenterXBetweenRects(entry_id *LeftRect, entry_id *RightRect);
inline r32 CenterYBetweenRects(entry_id *BottomRect, entry_id *TopRect);

inline v2 ClampToRect(v2 Pos, entry_id *Entry);
inline v2 ClampToRect(v2 Pos, rect_2D Rect);

// Auto screen transform stuff
inline screen_transform_list CreateScreenTransformList(arena_allocator *Arena, u32 Size = 128);
inline void RemoveFromTransformList(screen_transform_list *List, entry_id *Entry);
inline u32 TranslateWithScreen(screen_transform_list *List, entry_id *Entry, fixed_to FixedTo, r32 FixToPosition = 0);
inline u32 TranslateWithScreen(screen_transform_list *List, entry_id *Entry, fixed_to FixedTo, v2  FixToPosition);
inline u32 ScaleWithScreen(screen_transform_list *List, entry_id *Entry, scale_axis ScaleAxis);
inline u32 TransformWithScreen(screen_transform_list *List, entry_id *Entry, 
                               fixed_to FixedTo, scale_axis ScaleAxis, r32 FixToPosition = 0);
inline u32 TransformWithScreen(screen_transform_list *List, entry_id *Entry, 
                               fixed_to FixedTo, scale_axis ScaleAxis, v2 FixToPosition);
inline void ChangeFixToPosition(screen_transform_list *List, u32 ID, r32 NewFixToPosition);
inline void ChangeFixToPosition(screen_transform_list *List, u32 ID, v2  NewFixToPosition);
internal void PerformScreenTransform(renderer *Renderer);

// Render Text stuff
inline void SetPosition(render_text *Info, v2 P);
inline void SetPositionX(render_text *Info, r32 X);
inline void SetPositionY(render_text *Info, r32 Y);
inline void SetLocalPosition(render_text *Text, v2 P);
inline void SetActive(render_text *Text, b32 Render);
inline void SetColor(render_text *Text, v3 *Color);
inline void Translate(render_text *Info, v2 Translation);
inline v2   GetPosition(render_text *Info, u32 LetterID);
inline void RemoveRenderText(renderer *Renderer, render_text *Text);
inline void SetTransparency(render_text *Text, r32 T);
inline void CenterText(render_text *Text);
inline v2 GetSize(render_text *Text);
inline v2 GetExtends(render_text *Text);