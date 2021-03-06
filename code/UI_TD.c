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
    
    Result->Icon = CreateRenderBitmap(Renderer, Rect, Depth-0.0000001f, Result->Entry, IconPath);
    Result->Icon->ID->Color = IconColor;
    
    if(IsToggle)
    {
        Assert(ToggleIconPath);
        Result->ToggleIcon = CreateRenderBitmap(Renderer, Rect, Depth-0.0000001f, Result->Entry, ToggleIconPath);
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
    
    Result->Icon = CreateRenderBitmap(Renderer, Rect, Depth-0.0000001f, Result->Entry, IconBitmapID);
    Result->Icon->ID->Color = IconColor;
    
    if(IsToggle)
    {
        Assert(ToggleIconBitmapID >= 0);
        Result->ToggleIcon = CreateRenderBitmap(Renderer, Rect, Depth-0.0000001f, Result->Entry, ToggleIconBitmapID);
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
        if(Renderer->ButtonGroup.Buttons[It].Entry->ID->Render) 
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
SetActive(button *Button, b32 SetActive)
{
    Button->Entry->ID->Render = SetActive;
    Button->Icon->ID->Render = SetActive;
    if(Button->ToggleIcon) Button->ToggleIcon->ID->Render = SetActive;
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

// Dragable stuff

internal void
AddDragable(drag_list *DragList, entry_id *Entry, 
            drag_func_pointer OnDragStart, drag_func_pointer OnDragging, drag_func_pointer OnDragEnd)
{
    Assert(DragList->Count < DRAGABLE_MAX_COUNT);
    DragList->IsActive[DragList->Count] = true;
    DragList->Dragables[DragList->Count] = Entry;
    DragList->OnDragStart[DragList->Count] = OnDragStart;
    DragList->OnDragging[DragList->Count] = OnDragging;
    DragList->OnDragEnd[DragList->Count++] = OnDragEnd;
}

inline void
OnDraggingStart(drag_list *DragableList, renderer *Renderer, v2 MouseP)
{
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
        }
    }
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

// Textfield stuff

internal text_field
CreateTextField(renderer *Renderer, arena_allocator *Arena, v2 Size, r32 ZValue, u8 *EmptyFieldString, entry_id *Parent, v3 *TextColor, v3 *BGColor)
{
    text_field Result = {};
    
    Result.TextColor = TextColor;
    Result.Transparency = 0.25f;
    Result.ZValue    = ZValue;
    Result.NoText    = NewStaticStringCompound(EmptyFieldString);
    
    Result.Background = CreateRenderRect(Renderer, Size, Result.ZValue, BGColor, Parent);
    Result.Background->ID->Render = false;
    Result.LeftAlign = CreateRenderRect(Renderer, V2(0), Result.ZValue-0.000001f, 
                                        TextColor, Result.Background);
    SetLocalPosition(Result.LeftAlign, V2(-(Size.x-4)/2.0f, 0));
    
    Result.Cursor = CreateRenderRect(Renderer, V2(2, 35), 
                                     Result.ZValue-0.001f, Result.TextColor,
                                     Result.LeftAlign);
    SetLocalPosition(Result.Cursor, V2(4, 0));
    Result.Cursor->ID->Render = false;
    
    Result.TextString = NewStringCompound(Arena, 255);
    Result.DoMouseHover = true;
    
    return Result;
}

inline void
Translate(text_field *TextField, v2 Translation)
{
    Translate(TextField->Background, Translation);
}

inline void
SetActive(text_field *TextField, b32 MakeActive)
{
    TextField->IsActive = MakeActive;
    TextField->Background->ID->Render = MakeActive;
    TextField->Cursor->ID->Render = MakeActive;
    SetActive(&TextField->Text, MakeActive);
}

inline void
UpdateTextField(renderer *Renderer, text_field *TextField)
{
    TextField->dBlink = 0.0f;
    TextField->Cursor->ID->Render = true;
    
    RemoveRenderText(Renderer, &TextField->Text);
    
    CreateRenderText(Renderer, &GlobalGameState.FixArena, font_Medium, &TextField->TextString, 
                     TextField->TextColor, &TextField->Text, TextField->ZValue-0.000001f, TextField->LeftAlign);
    Translate(&TextField->Text, V2(12, 10));
    SetLocalPosition(TextField->Cursor, V2(TextField->Text.CurrentP.x + 10, 0));
    
    if(TextField->TextString.Pos == 0)
    {
        RemoveRenderText(Renderer, &TextField->Text);
        CreateRenderText(Renderer, &GlobalGameState.FixArena, font_Medium, &TextField->NoText, 
                         TextField->TextColor, &TextField->Text, TextField->ZValue-0.000001f, TextField->LeftAlign);
        SetTransparency(&TextField->Text, TextField->Transparency);
        Translate(&TextField->Text, V2(12, 10));
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
                    // If Input->KeyChange[KEY_BackSPACE] == KeyDown then fall through
                    if(TextField->dBackspacePress >= 1 ||
                       Input->KeyChange[KEY_BACKSPACE] == KeyDown) // If pressed longer, start after a while to delete more letters
                    {
                        if(TextField->dBackspaceSpeed >= 1 || 
                           Input->KeyChange[KEY_BACKSPACE] == KeyDown) // Letter deletion interval
                        {
                            TextField->dBackspaceSpeed = 0.0f;
                            
                            TextField->TextString.Pos -= 1;
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
    SetLocalPosition(Slider->Background, T);
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
CreateSlider(slider *Result, renderer *Renderer, v2 BGSize, v2 GrabSize, r32 Depth, loaded_bitmap BGBitmap, 
             v3 *GrabColor, slider_axis Axis, entry_id *Parent = 0)
{
    Result->Axis = Axis;
    Result->Background = CreateRenderBitmap(Renderer, BGSize, Depth, Parent, CreateGLTexture(BGBitmap));
    Result->GrabThing  = CreateRenderRect(Renderer, GrabSize, Depth-0.00001f, GrabColor, Result->Background);
    Result->SlidePercentage = 0.5f;
    
    if(Axis == sliderAxis_Y) Result->MaxSlidePix = (BGSize.y-GrabSize.y)*0.5f;
    else Result->MaxSlidePix = (BGSize.x-GrabSize.x)*0.5f;
    
    AddDragable(&GlobalGameState.DragableList, Result->Background, {OnSliderDragStart, Result}, {OnSliderDrag, Result}, {});
}


// Quit curtain ************************

internal void
CreateQuitAnimation(quit_animation *Result, v2 Size, string_c *ClosingText, r32 AnimationTime)
{
    renderer *Renderer = &GlobalGameState.Renderer;
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    
    Result->Curtain = CreateRenderRect(Renderer, Size, -0.99f, &DisplayInfo->ColorPalette.SliderGrabThing);
    Translate(Result->Curtain, Size/2.0f);
    SetActive(Result->Curtain, false);
    Result->dAnim = 0;
    Result->Time = AnimationTime;
    
    CreateRenderText(Renderer, &GlobalGameState.FixArena, font_Big, ClosingText, &DisplayInfo->ColorPalette.Text, 
                     &Result->Text, -0.999f, 0);
    SetPosition(&Result->Text, Size/2.0f);
    SetActive(&Result->Text, false);
    
    Result->LastEscapePressTime = -10;
}

inline void 
SetActive(quit_animation *Quit, b32 Activate)
{
    Quit->AnimationStart = Activate;
    SetActive(Quit->Curtain, Activate);
    SetActive(&Quit->Text, Activate);
}

internal b32
QuitAnimation(quit_animation *Quit, r32 Dir, v2 Position, v2 Size)
{
    b32 Result = false;
    
    if(!Quit->AnimationStart) SetActive(Quit, true);
    
    time_management *Time = &GlobalGameState.Time;
    if(Quit->dAnim >= 1.0f || Quit->dAnim < 0.0f)
    {
        SetScale(Quit->Curtain, V2(1, 1));
        SetLocalPositionY(Quit->Curtain, Position.y);
        Result = true;
        Quit->Activated = Quit->dAnim >= 1.0f;
        Quit->dAnim = 0.0f;
    }
    else 
    {
        Quit->dAnim += Time->dTime/Quit->Time * Dir;
        
        r32 NewYScale = GraphFirstQuickThenSlow(Quit->dAnim);
        SetSize(Quit->Curtain, V2(Size.x, Size.y*NewYScale));
        
        r32 CurrentHeight = GetSize(Quit->Curtain).y/2.0f;
        SetLocalPosition(Quit->Curtain, V2(Position.x, Position.y - CurrentHeight));
        SetPosition(&Quit->Text, V2(Position.x - Quit->Text.CurrentP.x/2.0f, Position.y - CurrentHeight));
        
        SetTransparency(Quit->Curtain, Quit->dAnim/4.0f + 0.75f);
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
    
    CreateRenderText(Renderer, Arena, FontSize, &Text, &Renderer->ColorPalette->Text, &Result->Text, Depth-0.00001f, 0);
    
    v2 Size = {Result->Text.CurrentP.x - GetPosition(Result->Text.Base).x, (r32)FontSizeToPixel(FontSize)};
    Result->BG = CreateRenderRect(Renderer, Size+V2(40,0), Depth, &Renderer->ColorPalette->SliderGrabThing, Result->Anchor);
    SetLocalPosition(Result->BG, V2(Size.x + 40, -Size.y)/2.0f);
    
    SetParent(Result->Text.Base, Result->BG);
    SetLocalPosition(&Result->Text, V2(-Size.x/2.0f, Size.y*0.2f));
    
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
    CreateRenderText(Renderer, Arena, FontSize, &NewText, &Renderer->ColorPalette->Text, &Popup->Text, Depth, 0);
    
    v2 Size = {Popup->Text.CurrentP.x - GetPosition(Popup->Text.Base).x, (r32)FontSizeToPixel(FontSize)};
    RemoveRenderEntry(Renderer, Popup->BG);
    Popup->BG = CreateRenderRect(Renderer, Size+V2(40,0), Depth+0.00001f, 
                                 &Renderer->ColorPalette->SliderGrabThing, Popup->Anchor);
    SetLocalPosition(Popup->BG, V2(Size.x + 40, -Size.y)/2.0f);
    
    SetParent(Popup->Text.Base, Popup->BG);
    SetLocalPosition(&Popup->Text, V2(-Size.x/2.0f, Size.y*0.2f));
    
    SetActive(Popup->BG, BGActive);
    SetActive(&Popup->Text, TextActive);
}












