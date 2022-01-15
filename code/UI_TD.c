#include "UI_TD.h"


inline button *
NewButton(renderer *Renderer, rect Rect, r32 Depth, b32 IsToggle,
          string_c *BtnPath, v3 *BaseColor, v3 *DownColor, v3 *HoverColor, string_c *IconPath, v3 *IconColor,
          entry_id *Parent, string_c *ToggleIconPath)
{
    Assert(Renderer->ButtonGroup.Count < MAX_BUTTONS);
    button *Result = Renderer->ButtonGroup.Buttons + Renderer->ButtonGroup.Count++;
    Result->Entry = CreateRenderBitmap(Renderer, Rect, Depth, Parent, BtnPath);
    Result->Entry->ID->Color = BaseColor;
    Result->BaseColor = BaseColor;
    Result->DownColor = DownColor;
    Result->HoverColor = HoverColor;
    Result->IsToggle = IsToggle;
    
    Result->Icon = CreateRenderBitmap(Renderer, Rect, Depth-0.000001f, Result->Entry, IconPath);
    Result->Icon->ID->Color = IconColor;
    
    if(IsToggle)
    {
        Assert(ToggleIconPath);
        Result->ToggleIcon = CreateRenderBitmap(Renderer, Rect, Depth-0.000001f, Result->Entry, ToggleIconPath);
        Result->ToggleIcon->ID->Render = false;
        Result->ToggleIcon->ID->Color = IconColor;
    }
    
    return Result;
}


inline button *
NewButton(renderer *Renderer, rect Rect, r32 Depth, b32 IsToggle,
          u32 ButtonBitmapID, v3 *BaseColor, v3 *DownColor, v3 *HoverColor, 
          u32 IconBitmapID, v3 *IconColor, entry_id *Parent, i32 ToggleIconBitmapID)
{
    Assert(Renderer->ButtonGroup.Count < MAX_BUTTONS);
    button *Result = Renderer->ButtonGroup.Buttons + Renderer->ButtonGroup.Count++;
    Result->Entry = CreateRenderBitmap(Renderer, Rect, Depth, Parent, ButtonBitmapID);
    Result->Entry->ID->Color = BaseColor;
    Result->BaseColor = BaseColor;
    Result->DownColor = DownColor;
    Result->HoverColor = HoverColor;
    Result->IsToggle = IsToggle;
    
    Result->Icon = CreateRenderBitmap(Renderer, Rect, Depth-0.000001f, Result->Entry, IconBitmapID);
    Result->Icon->ID->Color = IconColor;
    
    if(IsToggle)
    {
        Assert(ToggleIconBitmapID >= 0);
        Result->ToggleIcon = CreateRenderBitmap(Renderer, Rect, Depth-0.000001f, Result->Entry, ToggleIconBitmapID);
        Result->ToggleIcon->ID->Render = false;
        Result->ToggleIcon->ID->Color = IconColor;
    }
    
    return Result;
}

inline button *
NewButton(renderer *Renderer, rect Rect, r32 Depth, b32 IsToggle, u32 ButtonBitmapID, u32 IconBitmapID,
          button_colors Colors, entry_id *Parent, i32 ToggleIconBitmapID)
{
    return NewButton(Renderer, Rect, Depth, IsToggle, ButtonBitmapID, Colors.BaseColor, Colors.DownColor, Colors.HoverColor, IconBitmapID, Colors.IconColor, Parent, ToggleIconBitmapID);
}

inline void
ToggleButtonVisuals(button *Button, b32 ToggleOn)
{
    Assert(Button->IsToggle);
    Button->State = ToggleOn ? buttonState_Pressed : buttonState_Unpressed;
    Button->Entry->ID->Color = ToggleOn ? Button->DownColor : Button->BaseColor;
    Button->Icon->ID->Render = !ToggleOn;
    Button->ToggleIcon->ID->Render = ToggleOn;
    
}

internal void
ButtonTestMouseInteraction(renderer *Renderer, input_info *Input, button *Button)
{
    if(Button->IsDisabled) return;
    
    b32 MouseInsideButton = false;
    MouseInsideButton = IsInRect(Button->Entry, Input->MouseP);
    
    if(Button->IsToggle)
    {
        if(MouseInsideButton)
        {
            if(Input->KeyChange[KEY_LMB] == KeyDown) // Btn starts being pressed down
            {
                Button->ClickedInBtn = true;
                if(Button->State != buttonState_Pressed)
                {
                    Button->State = buttonState_Pressed;
                    Button->Entry->ID->Color = Button->DownColor;
                    Button->Icon->ID->Render = false;
                    Button->ToggleIcon->ID->Render = true;
                }
                else
                {
                    Button->State = buttonState_Hover;
                    Button->Entry->ID->Color = Button->HoverColor;
                    Button->Icon->ID->Render = true;
                    Button->ToggleIcon->ID->Render = false;
                    if(Button->OnHoverEnter.Func) Button->OnHoverEnter.Func(Button->OnHoverEnter.Data);
                }
            }
            else if(Button->State == buttonState_Unpressed) // Btn starts being hovered
            {
                Button->State = buttonState_Hover;
                Button->Entry->ID->Color = Button->HoverColor;
                if(Button->OnHoverEnter.Func) Button->OnHoverEnter.Func(Button->OnHoverEnter.Data);
            }
            else if(Input->KeyChange[KEY_LMB] == KeyUp && 
                    Button->ClickedInBtn) // Btn press ended
            {
                Button->ClickedInBtn = false;
                if(!Button->Icon->ID->Render && Button->OnPressed.Func) Button->OnPressed.Func(Button->OnPressed.Data);
                else if(Button->OnPressedToggleOff.Func) Button->OnPressedToggleOff.Func(Button->OnPressed.Data);
            }
        }
        else
        {
            if(Button->State == buttonState_Hover) // Btn being exited
            {
                Button->State = buttonState_Unpressed;
                Button->Entry->ID->Color = Button->BaseColor;
                if(Button->OnHoverExit.Func) Button->OnHoverExit.Func(Button->OnHoverExit.Data);
            }
            else if(Input->Pressed[KEY_LMB] && Button->State == buttonState_Pressed &&
                    Button->ClickedInBtn)
            {
                Button->State = buttonState_Unpressed;
                Button->Icon->ID->Render = true;
                Button->ToggleIcon->ID->Render = false;
            }
            if(Input->KeyChange[KEY_LMB] == KeyUp && Button->ClickedInBtn) Button->ClickedInBtn = false;
        }
    }
    else
    {
        if(MouseInsideButton)
        {
            if(Input->KeyChange[KEY_LMB] == KeyDown) // Btn starts being pressed down
            {
                Button->State = buttonState_Pressed;
                Button->Entry->ID->Color = Button->DownColor;
            }
            else if(Button->State == buttonState_Unpressed) // Btn starts being hovered
            {
                Button->State = buttonState_Hover;
                Button->Entry->ID->Color = Button->HoverColor;
                if(Button->OnHoverEnter.Func) Button->OnHoverEnter.Func(Button->OnHoverEnter.Data);
            }
            else if(Input->KeyChange[KEY_LMB] == KeyUp && 
                    Button->State == buttonState_Pressed) // Btn press ended
            {
                Button->State = buttonState_Hover;
                Button->Entry->ID->Color = Button->HoverColor;
                if(Button->OnPressed.Func) Button->OnPressed.Func(Button->OnPressed.Data);
            }
        }
        else
        {
            if(Button->State == buttonState_Hover ||
               Button->State == buttonState_Pressed) // Btn being exited
            {
                Button->State = buttonState_Unpressed;
                Button->Entry->ID->Color = Button->BaseColor;
                if(Button->OnHoverExit.Func) Button->OnHoverExit.Func(Button->OnHoverExit.Data);
            }
        }
    }
}

internal void
UpdateButtons(renderer *Renderer, input_info *Input)
{
    For(Renderer->ButtonGroup.Count)
    {
        if(Renderer->ButtonGroup.Buttons[It].Entry->ID->Render &&
           !Renderer->ButtonGroup.Buttons[It].IsDisabled) 
            ButtonTestMouseInteraction(Renderer, Input,
                                       Renderer->ButtonGroup.Buttons+It);
    }
}

inline b32 
IsButtonHovering(button *Button)
{
    return Button->State == buttonState_Hover;
}

inline b32 
IsOnButton(button *Button, v2 Position)
{
    b32 Result = IsInRect(Button->Entry, Position);
    return Result;
}

inline void
SetDisabled(button *Button, b32 Disable, v3 *Color)
{
    SetColor(Button->Icon, Color);
    Button->IsDisabled = Disable;
}

inline void
SetActive(button *Button, b32 SetActive)
{
    Button->Entry->ID->Render = SetActive;
    Button->Icon->ID->Render = SetActive;
    if(Button->ToggleIcon) Button->ToggleIcon->ID->Render = SetActive;
}

inline v2
GetSize(button *Button)
{
    return GetSize(Button->Entry);
}

inline void
SetSize(button *Button, v2 Size)
{
    SetSize(Button->Entry, Size);
    SetSize(Button->Icon, Size);
    if(Button->IsToggle)
        SetSize(Button->ToggleIcon, Size);
}

inline void
ResetBtnState(button *Button)
{
    Button->State            = buttonState_Unpressed;
    Button->Entry->ID->Color = Button->BaseColor;
}

inline void
Translate(button *Button, v2 Translation)
{
    Translate(Button->Entry, Translation);
}

inline void
SetLocalPosition(button *Button, v2 T)
{
    SetLocalPosition(Button->Entry, T);
}

inline void
SetLocalPositionX(button *Button, r32 X)
{
    SetLocalPositionX(Button->Entry, X);
}

inline void
SetPosition(button *Button, v2 T)
{
    SetPosition(Button->Entry, T);
}

inline v2
GetPosition(button *Button)
{
    return GetPosition(Button->Entry);
}

inline v2
GetLocalPosition(button *Button)
{
    return GetLocalPosition(Button->Entry);
}

inline void 
SetScissor(button *Button, entry_id *ScissorID)
{
    Button->Entry->ID->Scissor = ScissorID;
    Button->Icon->ID->Scissor = ScissorID;
    if(Button->IsToggle) 
        Button->ToggleIcon->ID->Scissor = ScissorID;
}

// Dragable stuff

internal void
AddDragable(drag_list *DragList, entry_id *Entry, 
            drag_func_pointer OnDragStart, drag_func_pointer OnDragging, drag_func_pointer OnDragEnd)
{
    Assert(DragList->Count < DRAGABLE_MAX_COUNT);
    DragList->IsActive[DragList->Count]    = true;
    DragList->Dragables[DragList->Count]   = Entry;
    DragList->OnDragStart[DragList->Count] = OnDragStart;
    DragList->OnDragging[DragList->Count]  = OnDragging;
    DragList->OnDragEnd[DragList->Count++] = OnDragEnd;
}

inline b32
OnDraggingStart(drag_list *DragableList, renderer *Renderer, v2 MouseP)
{
    b32 Result = false;
    For(DragableList->Count)
    {
        if(!DragableList->Dragables[It]->ID->Render) continue;
        if(!DragableList->IsActive[It])              continue;
        if(IsInRect(DragableList->Dragables[It], MouseP))
        {
            v2 AdjustedMouseP = MouseP;
            DragableList->DraggingID = It;
            if(DragableList->OnDragStart[It].Func)
            {
                DragableList->OnDragStart[It].Func(Renderer, AdjustedMouseP, DragableList->Dragables[It], 
                                                   DragableList->OnDragStart[It].Data);
            }
            Result = true;
            break;
        }
    }
    return Result;
}

inline void
OnDragging(drag_list *DragableList, renderer *Renderer, v2 MouseP)
{
    if(DragableList->OnDragging[DragableList->DraggingID].Func)
    {
        v2 AdjustedMouseP = MouseP;
        DragableList->OnDragging[DragableList->DraggingID].Func(Renderer, AdjustedMouseP, 
                                                                DragableList->Dragables[DragableList->DraggingID],
                                                                DragableList->OnDragging[DragableList->DraggingID].Data);
    }
}

inline void
OnDraggingEnd(drag_list *DragableList, renderer *Renderer, v2 MouseP)
{
    if(DragableList->OnDragEnd[DragableList->DraggingID].Func)
    {
        v2 AdjustedMouseP = MouseP;
        DragableList->OnDragEnd[DragableList->DraggingID].Func(Renderer, AdjustedMouseP, 
                                                               DragableList->Dragables[DragableList->DraggingID],
                                                               DragableList->OnDragEnd[DragableList->DraggingID].Data);
    }
    DragableList->DraggingID = -1;
}

inline void
SetActiveAll(drag_list *DragList, b32 Activate)
{
    For(DragList->Count) DragList->IsActive[It] = Activate;
}

inline void
SetActiveAllButGiven(drag_list *DragList, entry_id *ID, b32 Activate)
{
    For(DragList->Count) 
    {
        if(ID->ID == DragList->Dragables[It]->ID) DragList->IsActive[It] = !Activate;
        else DragList->IsActive[It] = Activate;
    }
}

inline void
SetActive(drag_list *DragList, entry_id *ID, b32 Activate)
{
    For(DragList->Count) 
    {
        if(ID->ID == DragList->Dragables[It]->ID) DragList->IsActive[It] = Activate;
    }
}

// Textfield stuff

internal text_field
CreateTextField(renderer *Renderer, arena_allocator *Arena, v2 Size, r32 ZValue, u8 *EmptyFieldString, entry_id *Parent, v3 *TextColor, v3 *BGColor, font_size_id FontSize, u32 MaxStringLength)
{
    text_field Result = {};
    
    Result.TextColor = TextColor;
    Result.Transparency = 0.25f;
    Result.ZValue    = ZValue;
    Result.NoText    = NewStaticStringCompound(EmptyFieldString);
    Result.FontSize  = FontSize;
    
    Result.Background = CreateRenderRect(Renderer, Size, Result.ZValue, BGColor, Parent);
    Result.Background->ID->Render = false;
    Result.LeftAlign = CreateRenderRect(Renderer, V2(0), Result.ZValue-0.000001f, 
                                        TextColor, Result.Background);
    SetLocalPosition(Result.LeftAlign, V2(-Size.x*0.5f - 2, 0));
    
    // @Layout
    font_metrics Metrics = GetFontMetrics(&GlobalGameState, FontSize, Result.NoText);
    r32 CursorHeight = Metrics.Ascent - Metrics.Descent;  
    switch(FontSize) {
        case font_Small: {
            Result._FontOffset = V2(10, -Size.y*0.5f - Metrics.Descent);
        } break;
        case font_Medium: {
            Result._FontOffset = V2(12, -Size.y*0.5f - Metrics.Descent);
        } break;
        case font_Big: {
            Result._FontOffset = V2(14, -Size.y*0.5f - Metrics.Descent);
        } break;
    }
    Result.Cursor = CreateRenderRect(Renderer, V2(2, CursorHeight), 
                                     Result.ZValue-0.001f, Result.TextColor,
                                     Result.LeftAlign);
    SetLocalPosition(Result.Cursor, V2(4, 0));
    Result.Cursor->ID->Render = false;
    
    Result.TextString = NewStringCompound(Arena, MaxStringLength);
    Result.DoMouseHover = true;
    
    return Result;
}

inline void
Translate(text_field *TextField, v2 Translation)
{
    Translate(TextField->Background, Translation);
}

inline void
SetLocalPosition(text_field *TextField, v2 Position)
{
    SetLocalPosition(TextField->Background, Position);
}

inline void
SetActive(text_field *TextField, b32 MakeActive)
{
    TextField->IsActive = MakeActive;
    SetActive(TextField->Background, MakeActive);
    SetActive(&TextField->Text, MakeActive);
    // This only ever needs to be deactivated as the
    // active textfield will toggle it on/off itself.
    // We just need to hide it.
    SetActive(TextField->Cursor, false);
}

inline void 
SetParent(text_field *TextField, entry_id *Parent)
{
    SetParent(TextField->Background, Parent);
}

inline v2
GetSize(text_field *TextField)
{
    v2 Result = GetSize(TextField->Background);
    return Result;
}

inline void
SetSize(text_field *TextField, v2 Size)
{
    SetSize(TextField->Background, Size);
    
    font_metrics Metrics = GetFontMetrics(&GlobalGameState, TextField->FontSize, TextField->NoText);
    switch(TextField->FontSize) {
        case font_Small: {
            TextField->_FontOffset = V2(10, -Size.y*0.5f - Metrics.Descent);
        } break;
        case font_Medium: {
            TextField->_FontOffset = V2(12, -Size.y*0.5f - Metrics.Descent);
        } break;
        case font_Big: {
            TextField->_FontOffset = V2(14, -Size.y*0.5f - Metrics.Descent);
        } break;
    }
    r32 CursorHeight = Metrics.Ascent - Metrics.Descent;  
    SetSize(TextField->Cursor, V2(2, CursorHeight));
    
    SetLocalPosition(TextField->LeftAlign, V2(-Size.x*0.5f - 2, 0));
}

inline void 
SetScissor(text_field *TextField, entry_id *Rect)
{
    SetScissor(TextField->Background, Rect);
    SetScissor(TextField->Cursor, Rect);
    SetScissor(&TextField->Text, Rect);
}

inline void
UpdateTextField(renderer *Renderer, text_field *TextField)
{
    TextField->dBlink = 0.0f;
    TextField->Cursor->ID->Render = true;
    
    RemoveRenderText(Renderer, &TextField->Text);
    
    RenderText(&GlobalGameState, TextField->FontSize, &TextField->TextString, 
               TextField->TextColor, &TextField->Text, TextField->ZValue-0.000001f, TextField->LeftAlign);
    Translate(&TextField->Text, TextField->_FontOffset);
    SetScissor(&TextField->Text, TextField->Background);
    SetLocalPosition(TextField->Cursor, V2(TextField->Text.CurrentP.x + 10, 0)); // @Layout
    
    // Keep text _right_ aligned when it would move of the right side as you want to see the newest
    // text you write.
    v2 AlignPos = GetPosition(TextField->LeftAlign);
    if(TextField->TextString.Pos > 0)
    {
        r32 FieldWidth = GetSize(TextField->Background).x;
        r32 Overhang = TextField->Text.Extends.x - FieldWidth + TextField->_FontOffset.x;
        if(Overhang > 0)
        {
            SetPositionX(&TextField->Text, AlignPos.x + TextField->_FontOffset.x - Overhang);
            SetLocalPosition(TextField->Cursor, V2(FieldWidth, 0)); // @Layout
        }
    }
    
    if(TextField->TextString.Pos == 0)
    {
        RemoveRenderText(Renderer, &TextField->Text);
        RenderText(&GlobalGameState, TextField->FontSize, &TextField->NoText, 
                   TextField->TextColor, &TextField->Text, TextField->ZValue-0.000001f, TextField->LeftAlign);
        SetTransparency(&TextField->Text, TextField->Transparency);
        Translate(&TextField->Text, TextField->_FontOffset);
        SetScissor(&TextField->Text, TextField->Background);
    }
    
}

internal text_field_flag_result
ProcessTextField(renderer *Renderer, r32 dTime, input_info *Input, text_field *TextField)
{
    text_field_flag_result Result = {};
    if(TextField->IsActive)
    {
        // Do cursor blinking
        if(TextField->dBlink >= 1.0f)
        {
            TextField->Cursor->ID->Render = !TextField->Cursor->ID->Render;
            TextField->dBlink = 0.0f;
        }
        else TextField->dBlink += dTime/BLINK_TIME;
        
        if(IsInRect(TextField->Background, Input->MouseP) && TextField->DoMouseHover)
        {
            SetTransparency(TextField->Background, TextField->Transparency);
            SetTransparency(TextField->Cursor, TextField->Transparency);
            if(TextField->TextString.Pos != 0) SetTransparency(&TextField->Text, TextField->Transparency);
        }
        else 
        {
            SetTransparency(TextField->Background, 1.0f);
            SetTransparency(TextField->Cursor, 1.0f);
            if(TextField->TextString.Pos != 0) SetTransparency(&TextField->Text, 1.0f);
        }
        
        b32 StringIsPasted = false;
        if((Input->Pressed[KEY_CONTROL_LEFT] || Input->Pressed[KEY_CONTROL_RIGHT]) && Input->KeyChange[KEY_V] == KeyDown)
            StringIsPasted = true;
        
        // Only process input when no modifier is pressed
        if(((!Input->Pressed[KEY_CONTROL_LEFT] || Input->Pressed[KEY_ALT_RIGHT]) && !Input->Pressed[KEY_CONTROL_RIGHT] &&
            !Input->Pressed[KEY_ALT_LEFT]) || StringIsPasted)
        {
            b32 TextChanged = false;
            
            if(StringIsPasted) // Process pasted symbols
            {
                string_c PastedString = NewStringCompound(&GlobalGameState.ScratchArena, 1500);
                if(TryGetClipboardText(&PastedString))
                {
                    if(PastedString.Pos > TextField->TextString.Length-TextField->TextString.Pos)
                    {
                        PastedString.Pos = TextField->TextString.Length-TextField->TextString.Pos;
                    }
                    AppendStringCompoundToCompound(&TextField->TextString, &PastedString);
                    TextChanged = true;
                }
                DeleteStringCompound(&GlobalGameState.ScratchArena, &PastedString);
            }
            else if(Input->CharCount > 0) // Process all symbols
            {
                For(Input->CharCount)
                {
                    if(TextField->TextString.Pos < TextField->TextString.Length)
                    {
                        AppendCharToCompound(&TextField->TextString, Input->Chars[It]);
                    }
                    else DebugLog(255, "Reached searchbar character limit.\n");
                }
                TextChanged = true;
            }
            
            if(Input->Pressed[KEY_BACKSPACE]) // Delete a symbol
            {
                if(TextField->TextString.Pos > 0)
                {
                    if(TextField->dBackspacePress >= 1 ||
                       Input->KeyChange[KEY_BACKSPACE] == KeyDown) // If pressed longer, start after a while to delete more letters
                    {
                        if(TextField->dBackspaceSpeed >= 1 || 
                           Input->KeyChange[KEY_BACKSPACE] == KeyDown) // Letter deletion interval
                        {
                            TextField->dBackspaceSpeed = 0.0f;
                            
                            RemoveLastUTF8Char(&TextField->TextString);
                            TextChanged = true;
                        }
                        else TextField->dBackspaceSpeed += dTime/BACKSPACE_CONTIUOUS_SPEED;
                    }
                    else TextField->dBackspacePress += dTime/BACKSPACE_CONTIUOUS_TIME;
                }
            }
            else 
            {
                TextField->dBackspacePress = 0.0f;
                TextField->dBackspaceSpeed = 0.0f;
            }
            
            if(TextChanged)
            {
                UpdateTextField(Renderer, TextField);
                Result.Flag |= processTextField_TextChanged;
            }
            if(Input->KeyChange[KEY_ENTER] == KeyDown)
            {
                Result.Flag |= processTextField_Confirmed;
            }
        }
    }
    return Result;
}

// LOAD BAR ****************************************


inline loading_bar
CreateLoadingBar(v2 Size, r32 Depth, entry_id *Parent)
{
    loading_bar Result;
    
    Result.BG = CreateRenderRect(&GlobalGameState.Renderer, Size, Depth, 
                                 &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.SliderGrabThing, Parent);
    Result.ProgressBar = CreateRenderRect(&GlobalGameState.Renderer, V2(Size.x, Max(0.0f, Size.y-4)), Depth-0.00001f, 
                                          &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.SliderBackground, Result.BG);
    
    return Result;
}

inline void 
SetPosition(loading_bar *LoadingBar, v2 NewP)
{
    SetPosition(LoadingBar->BG, NewP);
}

inline void 
SetLocalPosition(loading_bar *LoadingBar, v2 NewP)
{
    SetLocalPosition(LoadingBar->BG, NewP);
}

inline void
SetActive(loading_bar *LoadingBar, b32 IsActive)
{
    LoadingBar->BG->ID->Render = IsActive;
    LoadingBar->ProgressBar->ID->Render = IsActive;
}

inline void
UpdateLoadingBar(loading_bar *LoadingBar, r32 ProgressPercentage)
{
    r32 Width = Max(0.0f, GetSize(LoadingBar->BG).x - 6);
    
    v2 NewSize = V2(Width*ProgressPercentage, GetSize(LoadingBar->ProgressBar).y);
    SetSize(LoadingBar->ProgressBar, NewSize);
    SetLocalPosition(LoadingBar->ProgressBar, V2(-Width/2 + NewSize.x/2, 0));
}

inline void
UpdateIndeterminiteLoadingBar(loading_bar *LoadingBar, r32 RoundtripPercentage)
{
    r32 Width = Max(0.0f, GetSize(LoadingBar->BG).x - 6);
    r32 Size  = GetSize(LoadingBar->ProgressBar).x;
    v2 NewP = V2((-Width/2+Size/2) + (Width-Size)*RoundtripPercentage, 0);
    SetLocalPosition(LoadingBar->ProgressBar, NewP);
}

// Slider *********************

inline void
Translate(slider *Slider, v2 T)
{
    Translate(Slider->Background, T);
}

inline void
SetLocalPosition(slider *Slider, v2 T)
{
    v2 OldPBackground = GetLocalPosition(Slider->Background);
    v2 OldPGrabThing  = GetLocalPosition(Slider->GrabThing);
    SetLocalPosition(Slider->Background, T);
    SetLocalPosition(Slider->GrabThing, T + (OldPBackground - OldPGrabThing));
}

inline void
SetLocalPositionX(slider *Slider, r32 X)
{
    r32 OldPBackground = GetLocalPosition(Slider->Background).x;
    r32 OldPGrabThing  = GetLocalPosition(Slider->GrabThing).x;
    SetLocalPositionX(Slider->Background, X);
    SetLocalPositionX(Slider->GrabThing, X + (OldPBackground - OldPGrabThing));
}

inline v2
GetLocalPosition(slider *Slider)
{
    return GetLocalPosition(Slider->Background);
}

inline v2 
GetSize(slider *Slider)
{
    v2 Result = GetSize(Slider->Background);
    return Result;
}

inline void
SetSize(slider *Slider, v2 Size)
{
    SetSize(Slider->Background, Size);
}

inline void
SetSize(slider *Slider, v2 Size, v2 GrabSize)
{
    SetSize(Slider->Background, Size);
    SetSize(Slider->GrabThing, GrabSize);
}

inline void
SetActive(slider *Slider, b32 Activate)
{
    SetActive(Slider->Background, Activate);
    SetActive(Slider->GrabThing, Activate);
}

inline void 
SetTransparency(slider *Slider, r32 Alpha)
{
    SetTransparency(Slider->Background, Alpha);
    SetTransparency(Slider->GrabThing, Alpha);
}

inline void
OnSliderDragStart(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    slider *Slider = (slider *)Data;
    Slider->MouseOffset = AdjustedMouseP - GetPosition(Slider->GrabThing);
}

internal void
OnSliderDrag(renderer *Renderer, v2 AdjustedMouseP, entry_id *Dragable, void *Data)
{
    slider *Slider = (slider *)Data;
    Slider->SliderIsDragged = true;
    
    v2 GrabThingExtends  = GetExtends(Slider->GrabThing);
    if(Slider->Axis == sliderAxis_Y)
    {
        if(Slider->MouseOffset.y < GrabThingExtends.y && Slider->MouseOffset.y > -GrabThingExtends.y) 
            AdjustedMouseP.y -= Slider->MouseOffset.y;
        
        r32 BGY  = GetPosition(Slider->Background).y;
        r32 NewY = Clamp(AdjustedMouseP.y, BGY - Slider->MaxSlidePix, BGY + Slider->MaxSlidePix);
        SetLocalPosition(Slider->GrabThing, V2(0, NewY-BGY));
        
        Slider->SlidePercentage = SafeDiv(1.0f,(Slider->MaxSlidePix*2))*(NewY-BGY) + 0.5f;
    }
    else
    {
        if(Slider->MouseOffset.x < GrabThingExtends.x && Slider->MouseOffset.x > -GrabThingExtends.x) 
            AdjustedMouseP.x -= Slider->MouseOffset.x;
        
        r32 BGX  = GetPosition(Slider->Background).x;
        r32 NewX = Clamp(AdjustedMouseP.x, BGX - Slider->MaxSlidePix, BGX + Slider->MaxSlidePix);
        SetLocalPosition(Slider->GrabThing, V2(NewX-BGX, 0));
        
        Slider->SlidePercentage = SafeDiv(1.0f,(Slider->MaxSlidePix*2))*(NewX-BGX) + 0.5f;
    }
}

internal void
SetSliderPosition(slider *Slider, r32 Percentage)
{
    Slider->SlidePercentage = Clamp01(Percentage);
    r32 NewPos = (Slider->MaxSlidePix*2)*(Slider->SlidePercentage-0.5f);
    
    if(Slider->Axis == sliderAxis_Y) SetLocalPosition(Slider->GrabThing, V2(0, NewPos));
    else SetLocalPosition(Slider->GrabThing, V2(NewPos, 0));
}

internal void
CreateSlider(game_state *GS, slider *Result, slider_axis Axis, v2 BGSize, v2 GrabSize, r32 Depth, loaded_bitmap BGBitmap, 
             v3 *GrabColor, entry_id *Parent)
{
    Result->Axis = Axis;
    Result->Background = CreateRenderBitmap(&GS->Renderer, BGSize, Depth, Parent, CreateGLTexture(BGBitmap));
    Result->GrabThing  = CreateRenderRect(&GS->Renderer, GrabSize, Depth-0.00001f, GrabColor, Result->Background);
    Result->SlidePercentage = 0.5f;
    
    if(Axis == sliderAxis_Y) Result->MaxSlidePix = (BGSize.y-GrabSize.y)*0.5f;
    else Result->MaxSlidePix = (BGSize.x-GrabSize.x)*0.5f;
    
    AddDragable(&GS->DragableList, Result->Background, {OnSliderDragStart, Result}, {OnSliderDrag, Result}, {});
}

internal void
CreateSlider(game_state *GS, slider *Result, slider_axis Axis, rect BGRect, rect GrabRect, r32 Depth, b32 ShouldAutoDrag, entry_id *Parent)
{
    color_palette *Palette = &GS->MusicInfo.DisplayInfo.ColorPalette;
    
    Result->Axis = Axis;
    Result->Background  = CreateRenderRect(&GS->Renderer, BGRect, Depth, Parent, &Palette->SliderBackground);
    Result->GrabThing   = CreateRenderRect(&GS->Renderer, GrabRect, Depth - 0.0000001f, 
                                           Parent ? Result->Background : NULL, // If we got parent, BG is GrabThingParent.
                                           &Palette->SliderGrabThing);
    
    if(Axis == sliderAxis_Y) 
        Result->MaxSlidePix = GetExtends(Result->Background).y - GetExtends(Result->GrabThing).y;
    else                     
        Result->MaxSlidePix = GetExtends(Result->Background).x - GetExtends(Result->GrabThing).x;
    
    if(ShouldAutoDrag) 
        AddDragable(&GS->DragableList, Result->Background, {OnSliderDragStart, Result}, {OnSliderDrag, Result}, {});
}


// Quit curtain ************************
internal void
CreateQuitAnimation(quit_animation *Result, v2 Size, string_c *ClosingText, r32 AnimationTime, font_size_id FontSize, string_c *BonusText, r32 Depth)
{
    renderer *Renderer = &GlobalGameState.Renderer;
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    
    Result->Curtain = CreateRenderRect(Renderer, Size, Depth, &DisplayInfo->ColorPalette.SliderGrabThing);
    Translate(Result->Curtain, Size/2.0f);
    SetActive(Result->Curtain, false);
    Result->dAnim = 0;
    Result->Time = AnimationTime;
    
    RenderText(&GlobalGameState, FontSize, ClosingText, &DisplayInfo->ColorPalette.Text, 
               &Result->Text, Depth-0.0001f, 0);
    SetPosition(&Result->Text, Size/2.0f);
    SetScissor(&Result->Text, Result->Curtain);
    SetActive(&Result->Text, false);
    
    if(BonusText)
    {
        r32 FontHeight = GetFontSize(Renderer, font_Small).Size;
        RenderText(&GlobalGameState, font_Small, BonusText, &DisplayInfo->ColorPalette.Text, 
                   &Result->BonusText, Depth-0.0001f, 0);
        SetPosition(&Result->BonusText, V2(0, Size.y - FontHeight));
        SetActive(&Result->BonusText, false);
    }
    
    Result->LastEscapePressTime = -10;
}

inline void 
SetActive(quit_animation *Quit, b32 Activate)
{
    Quit->AnimationStart = Activate;
    SetActive(Quit->Curtain, Activate);
    SetActive(&Quit->Text, Activate);
    if(Quit->BonusText.MaxCount) SetActive(&Quit->BonusText, Activate);
}

internal b32
QuitAnimation(quit_animation *Quit, r32 Dir, v2 Position, v2 Size)
{
    b32 Result = false;
    
    if(!Quit->AnimationStart) SetActive(Quit, true);
    
    time_management *Time = &GlobalGameState.Time;
    // We have to check the direction as well, because it can happen
    // that, for instance, dAnim was decreased and is < 0, therefore
    // the next frame it would be set back in the 'if'. But before 
    // that can happen the user start pressing the btn again and the 
    // direction is changed. When we then trigger the 'if' we wrongfully 
    // declare a finished animation for the new direction.
    if((Quit->dAnim >= 1.0f && Dir > 0) || 
       (Quit->dAnim <  0.0f && Dir < 0))
    {
        SetScale(Quit->Curtain, V2(1, 1));
        SetLocalPositionY(Quit->Curtain, Position.y);
        Quit->Activated = Quit->dAnim >= 1.0f;
        Quit->dAnim = 0.0f;
        
        Result = true;
    }
    else 
    {
        Quit->dAnim += Time->dTime/Quit->Time * Dir;
        if(Quit->dAnim < 1.0f && Quit->dAnim >= 0.0f)
        {
            r32 NewYScale = GraphFirstQuickThenSlow(Quit->dAnim);
            SetSize(Quit->Curtain, V2(Size.x, Size.y*NewYScale));
            
            r32 CurrentHeight = GetSize(Quit->Curtain).y/2.0f;
            SetLocalPosition(Quit->Curtain, V2(Position.x, Position.y - CurrentHeight));
            SetPosition(&Quit->Text, V2(Position.x - Quit->Text.CurrentP.x/2.0f, Position.y - CurrentHeight));
            if(Quit->BonusText.MaxCount) 
            {
                r32 FontHeight = GetFontSize(&GlobalGameState.Renderer, font_Small).Size;
                SetPosition(&Quit->BonusText, V2(10, Position.y - CurrentHeight*2 + FontHeight));
            }
            SetTransparency(Quit->Curtain, Quit->dAnim/4.0f + 0.75f);
        }
    }
    
    return Result;
}


// Popup text

inline void 
SetActive(popup *Popup, b32 Activate)
{
    SetActive(Popup->BG, true);
    SetActive(&Popup->Text, false);
    Popup->AnimDir = Activate ? 1 : -1;
    
    SetScale(Popup->BG, V2(1-(r32)Activate));
}

inline b32
IsActive(popup *Popup)
{
    return Popup->BG->ID->Render;
}

inline void
SetPosition(popup *Popup, v2 P)
{
    SetPosition(Popup->Anchor, P);
}

inline void
SetLocalPosition(popup *Popup, v2 P)
{
    SetLocalPosition(Popup->Anchor, P);
}

inline void
SetParent(popup *Popup, entry_id *Parent)
{
    SetParent(Popup->Anchor, Parent);
}

internal void
DoAnimation(popup *Popup, r32 dTime)
{
    if(Popup->AnimDir != 0)
    {
        if(Popup->dAnim > 1.0f || Popup->dAnim < 0.0f)
        {
            Popup->dAnim = Clamp01(Popup->dAnim);
            if(Popup->AnimDir == 1) SetActive(&Popup->Text, true);
            else SetActive(Popup->BG, false);
            
            SetScale(Popup->BG, V2(1));
            Popup->AnimDir = 0;
        }
        else
        {
            SetScale(Popup->BG, V2(Popup->dAnim));
            Popup->dAnim += (dTime/Popup->AnimTime)*Popup->AnimDir;
        }
    }
}

internal void
CreatePopup(renderer *Renderer, arena_allocator *Arena, popup *Result, string_c Text, font_size FontSize, r32 Depth, r32 AnimTime)
{
    Result->Anchor = CreateRenderRect(Renderer, V2(0,0), Depth, 0);
    SetActive(Result->Anchor, false);
    
    RenderText(&GlobalGameState, FontSize.ID, &Text, &Renderer->ColorPalette->Text, &Result->Text, Depth-0.00001f, 0);
    
    v2 Size = {Result->Text.CurrentP.x - GetPosition(Result->Text.Base).x, FontSize.Size};
    Result->BG = CreateRenderRect(Renderer, Size+V2(40,0), Depth, &Renderer->ColorPalette->SliderGrabThing, Result->Anchor);
    SetLocalPosition(Result->BG, V2(Size.x + 40, -Size.y)/2.0f);
    
    r32 Descent = GetFontDescent(&GlobalGameState, FontSize.ID, Text);
    SetParent(Result->Text.Base, Result->BG);
    SetLocalPosition(&Result->Text, V2(-Size.x/2.0f, -Size.y*0.5f - Descent));
    
    SetActive(Result->BG, false);
    SetActive(&Result->Text, false);
    Result->AnimTime = AnimTime;
    Result->dAnim = 0.0f;
    Result->AnimDir = 0;
}

internal void 
ChangeText(renderer *Renderer, arena_allocator *Arena, popup *Popup, string_c NewText, font_size FontSize)
{
    b32 BGActive = IsActive(Popup->BG);
    b32 TextActive = IsActive(Popup->Text.Base);
    r32 Depth = Popup->Text.Base->ID->Vertice[0].z;
    RemoveRenderText(Renderer, &Popup->Text);
    RenderText(&GlobalGameState, FontSize.ID, &NewText, &Renderer->ColorPalette->Text, &Popup->Text, Depth, 0);
    
    v2 Size = {Popup->Text.CurrentP.x - GetPosition(Popup->Text.Base).x, FontSize.Size};
    RemoveRenderEntry(Renderer, Popup->BG);
    // @Layout
    Popup->BG = CreateRenderRect(Renderer, Size+V2(40,0), Depth+0.00001f, 
                                 &Renderer->ColorPalette->SliderGrabThing, Popup->Anchor);
    SetLocalPosition(Popup->BG, V2(Size.x + 40, -Size.y)/2.0f);
    
    r32 Descent = GetFontDescent(&GlobalGameState, FontSize.ID, NewText);
    SetParent(Popup->Text.Base, Popup->BG);
    SetLocalPosition(&Popup->Text, V2(-Size.x/2.0f, -Size.y*0.5f - Descent));
    
    SetActive(Popup->BG, BGActive);
    SetActive(&Popup->Text, TextActive);
}












