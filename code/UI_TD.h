/* date = September 6th 2020 8:04 am */
#ifndef _U_I__T_D_H
#define _U_I__T_D_H
#include "Font_TD.h"

// Button stuff
enum button_state
{
    buttonState_Unpressed,
    buttonState_Pressed,
    buttonState_Hover,
};

struct function_pointer
{
    void (*Func)(void *Data);
    void *Data;
};

struct button
{
    entry_id *Entry;
    v3 *BaseColor;
    v3 *DownColor;
    v3 *HoverColor;
    entry_id *Icon;
    entry_id *ToggleIcon;
    v3 *IconColor;
    
    function_pointer OnHoverEnter;
    function_pointer OnHoverExit;
    function_pointer OnPressed;
    function_pointer OnPressedToggleOff; // Only when the button is toggle
    
    // TODO:: Make toggle btn its super form? 
    // TODO:: Make this toggle btn interaction stuff simpler with the new child-structure
    b32 IsToggle;
    button_state State;
    b32 ClickedInBtn;
    
    b32 IsDisabled;
};

#define MAX_BUTTONS 120
struct button_group
{
    button Buttons[MAX_BUTTONS];
    u32 Count;
};

struct button_colors
{
    v3 *BaseColor;
    v3 *DownColor;
    v3 *HoverColor;
    v3 *IconColor;
};

inline button *NewButton(struct renderer *Renderer, rect Rect, r32 Depth, b32 IsToggle,
                         string_c *BtnPath, v3 *BaseColor, v3 *DownColor, v3 *HoverColor, string_c *IconPath, v3 *IconColor,
                         entry_id *Parent, string_c *ToggleIconPath = 0);
inline button *NewButton(struct renderer *Renderer, rect Rect, r32 Depth, b32 IsToggle,
                         u32 ButtonBitmapID, v3 *BaseColor, v3 *DownColor, v3 *HoverColor, 
                         u32 IconBitmapID, v3 *IconColor, entry_id *Parent, i32 ToggleIconBitmapID = -1);
inline button *NewButton(struct renderer *Renderer, rect Rect, r32 Depth, b32 IsToggle, u32 ButtonBitmapID, u32 IconBitmapID,
                         button_colors Colors, entry_id *Parent, i32 ToggleIconBitmapID = -1);
inline void SetDisabled(button *Button, b32 Disable, v3 *Color);
inline void SetActive(button *Button, b32 SetActive);
inline void ResetBtnState(button *Button);
inline void Translate(button *Button, v2 Translation);
inline void SetLocalPosition(button *Button, v2 Translation);
inline void SetLocalPositionX(button *Button, r32 X);
inline void SetPosition(button *Button, v2 Translation);
inline v2   GetPosition(button *Button);
inline v2   GetLocalPosition(button *Button);
inline void ToggleButtonVisuals(button *Button, b32 ToggleOn);
internal void ButtonTestMouseInteraction(renderer *Renderer, input_info *Input, button *Button);
inline b32 IsButtonHovering(button *Button);
inline b32 IsOnButton(button *Button, v2 Position);
inline void SetScissor(button *Button, entry_id *ScissorID);
inline v2  GetSize(button *Button);
inline void SetSize(button *Button, v2 Size);

// Dragable UI stuff
struct drag_func_pointer
{
    void (*Func)(struct renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data);
    void *Data;
};

#define DRAGABLE_MAX_COUNT 20
struct drag_list
{
    i32 DraggingID;
    b32 IsActive[DRAGABLE_MAX_COUNT];
    entry_id *Dragables[DRAGABLE_MAX_COUNT];
    drag_func_pointer OnDragStart[DRAGABLE_MAX_COUNT];
    drag_func_pointer OnDragging [DRAGABLE_MAX_COUNT];
    drag_func_pointer OnDragEnd  [DRAGABLE_MAX_COUNT];
    u32 Count;
};

// Text Fields stuff

#define BLINK_TIME 0.5f
#define BACKSPACE_CONTIUOUS_TIME 0.5f
#define BACKSPACE_CONTIUOUS_SPEED 0.05f

enum process_text_field_flag
{
    processTextField_TextChanged = 1<<0,
    processTextField_Confirmed   = 1<<1,
};
struct text_field_flag_result
{
    i32 Flag;
};

struct text_field
{
    v3 *TextColor;
    r32 Transparency;
    r32 ZValue;
    
    entry_id *Background;
    entry_id *LeftAlign;
    entry_id *Cursor;
    r32 dBlink;
    
    render_text Text;
    string_c TextString;
    string_c NoText;
    font_size_id FontSize;
    v2 _FontOffset; // Internal
    
    r32 dBackspacePress;
    r32 dBackspaceSpeed;
    
    b32 DoMouseHover;
    b32 IsActive;
};

internal text_field CreateTextField(renderer *Renderer, arena_allocator *Arena, v2 Size, r32 ZValue, u8 *EmptyFieldString, entry_id *Parent, v3 *TextColor, v3 *BGColor, font_size_id FontSize, u32 MaxStringLength = 255);
inline void Translate(text_field *TextField, v2 Translation);
inline void SetLocalPosition(text_field *TextField, v2 Position);
inline void SetActive(text_field *TextField, b32 MakeActive);
inline void SetParent(text_field *TextField, entry_id *Parent);
inline v2   GetSize(text_field *TextField);
inline void SetSize(text_field *TextField, v2 Size);
inline void UpdateTextField(renderer *Renderer, text_field *TextField);
internal text_field_flag_result ProcessTextField(renderer *Renderer, r32 dTime, input_info *Input, text_field *TextField);
inline void SetScissor(text_field *TextField, entry_id *Rect);

// LOAD BAR *********************

struct loading_bar
{
    entry_id *BG;
    entry_id *ProgressBar;
};

inline loading_bar CreateLoadingBar(v2 Size, r32 Depth, entry_id *Parent = 0);
inline void UpdateLoadingBar(loading_bar *LoadingBar, r32 ProgressPercentage);
inline void UpdateIndeterminiteLoadingBar(loading_bar *LoadingBar, r32 RoundtripPercentage); 
inline void SetPosition(loading_bar *LoadingBar, v2 NewP);
inline void SetLocalPosition(loading_bar *LoadingBar, v2 NewP);
inline void SetActive(loading_bar *LoadingBar, b32 IsActive);


// slider ***********************

enum slider_axis
{
    sliderAxis_X,
    sliderAxis_Y,
};

struct slider
{
    entry_id *Background;
    entry_id *GrabThing;
    r32 MaxSlidePix;
    
    v2 MouseOffset;
    slider_axis Axis;
    
    r32 SlidePercentage;
    b32 SliderIsDragged;
    
    r32 OverhangP; // CLEANUP:: This is for the column stuff only
};

inline void SetTransparency(slider *Slider, r32 Alpha);
inline void Translate(slider *Slider, v2 T);
inline void SetLocalPosition(slider *Slider, v2 T);
inline void SetLocalPositionX(slider *Slider, r32 X);
inline v2   GetLocalPosition(slider *Slider);
inline v2   GetSize(slider *Slider);
inline void SetSize(slider *Slider, v2 Size);
inline void SetSize(slider *Slider, v2 Size, v2 GrabSize);
inline void SetActive(slider *Slider, b32 Activate);

internal void CreateSlider(game_state *GS, slider *Result, slider_axis Axis, rect BGRect, rect GrabRect, r32 Depth, b32 ShouldAutoDrag, entry_id *Parent = 0);
internal void CreateSlider(game_state *GS, slider *Result, slider_axis Axis, v2 BGSize, v2 GrabSize, r32 Depth, loaded_bitmap BGBitmap, v3 *GrabColor, entry_id *Parent = 0);

// quit curtain

struct quit_animation
{
    b32 AnimationStart;
    b32 WindowExit;
    entry_id *Curtain;
    render_text Text;
    r32 dAnim;
    r32 Time;
    
    r64 LastEscapePressTime;
    b32 Activated;
    
    render_text BonusText;
};

internal void CreateQuitAnimation(quit_animation *Result, v2 Size, string_c *ClosingText, r32 AnimationTime, font_size_id FontSize = font_Big, string_c *BonusText = NULL, r32 Depth = -0.99f);
inline void SetActive(quit_animation *Quit, b32 Activate);
internal b32 QuitAnimation(quit_animation *Quit, r32 Dir, v2 Position, v2 Size);

// Popup text

struct popup
{
    entry_id *Anchor;
    entry_id *BG;
    render_text Text;
    
    r32 AnimTime;
    r32 dAnim;
    i32 AnimDir;
};

inline void SetActive(popup *Popup, b32 Activate);
inline b32 IsActive(popup *Popup);
inline void SetPosition(popup *Popup, v2 P);
inline void SetLocalPosition(popup *Popup, v2 P);
inline void SetParent(popup *Popup, entry_id *Parent);
internal void CreatePopup(renderer *Renderer, arena_allocator *Arena, popup *Result, string_c Text, font_size FontSize, r32 Depth = -0.95f, r32 AnimTime = 0.1f);
internal void ChangeText(renderer *Renderer, arena_allocator *Arena, popup *Popup, string_c NewText, font_size FontSize);
internal void DoAnimation(popup *Popup, r32 dTime);

// Border - Rectangle 

struct border_rectangle
{
    entry_id *Top;
    entry_id *Bottom;
    entry_id *Right;
    entry_id *Left;
    
    r32 Thickness;
};
internal border_rectangle CreateBorderRectangle(renderer *Renderer, v2 Size, r32 Depth, r32 Thickness, v3 *Color, entry_id *Parent = NULL);
inline void SetSize(border_rectangle *BorderRect, v2 Size);
inline  b32 IsActive(border_rectangle *BorderRect);
inline void SetActive(border_rectangle *BorderRect, b32 Activate);
inline void SetParent(border_rectangle *BorderRect, entry_id *Parent);

#endif //_U_I__T_D_H
