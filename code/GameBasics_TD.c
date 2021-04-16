#include "GameBasics_TD.h"

internal loaded_bitmap 
LoadImage_STB(u8 *Path)
{
    loaded_bitmap Result = {};
    
    // TODO:: Small image is strange: https://github.com/nothings/stb/issues/161
    i32 BG_X, BG_Y, BG_N;
    Result.Pixels = (u32 *)stbi_load((const char *)Path, &BG_X, &BG_Y, &BG_N, 0);
    Result.Width = BG_X;
    Result.Height = BG_Y;
    if(Result.Pixels) 
    {
        Result.WasLoaded = true;
        switch(BG_N)
        {
            case 3:
            {
                Result.ColorFormat = colorFormat_RGB;
            } break;
            case 4:
            {
                Result.ColorFormat = colorFormat_RGBA;
            } break;
            
            InvalidDefaultCase
        }
    }
    
    return Result;
}

internal loaded_bitmap 
LoadImage_STB(read_file_result Memory)
{
    loaded_bitmap Result = {};
    
    read_file_result ImageTest = {};
    i32 BG_X, BG_Y, BG_N;
    Result.Pixels = (u32 *)stbi_load_from_memory(Memory.Data, Memory.Size, &BG_X, &BG_Y, &BG_N, 0);
    Result.Width = BG_X;
    Result.Height = BG_Y;
    if(Result.Pixels) 
    {
        Result.WasLoaded = true;
        switch(BG_N)
        {
            case 3:
            {
                Result.ColorFormat = colorFormat_RGB;
            } break;
            case 4:
            {
                Result.ColorFormat = colorFormat_RGBA;
            } break;
            
            InvalidDefaultCase
        }
    }
    
    return Result;
}

inline void
FreeImage_STB(loaded_bitmap Bitmap)
{
    stbi_image_free(Bitmap.Pixels);
}


#if DEBUG_TD
inline timer
StartTimer()
{
    timer Timer = {};
    Timer.Start = Timer.LastSnap = GetWallClock();
    return Timer;
}

inline void
SnapTimer(timer *Timer, string_c Identification)
{
    i64 NewSnap = GetWallClock();
    r32 CurrentSnap = GetSecondsElapsed(GlobalGameState.Time.PerfCountFrequency, Timer->LastSnap, NewSnap);
    r32 Total = GetSecondsElapsed(GlobalGameState.Time.PerfCountFrequency, Timer->Start, NewSnap);
    u8 *Addon = Identification.Pos ? Identification.S : (u8 *)"";
    DebugLog(255, "%s Timer snap %i: %.8f, total: %.8f\n", Addon, ++Timer->Count, CurrentSnap, Total);
    Timer->LastSnap = NewSnap;
}
#endif

// Color picker *********************************************

inline void
SetActive(color_picker *ColorPicker, b32 Activate)
{
    ColorPicker->IsActive = Activate;
    SetActive(ColorPicker->TextureEntry, Activate);
    SetActive(&ColorPicker->ColorSpectrum, Activate);
    SetActive(ColorPicker->Background, Activate);
    SetActive(&ColorPicker->RGBText, Activate);
    SetActive(ColorPicker->PickDot, Activate);
    SetActiveAllButGiven(&GlobalGameState.DragableList, ColorPicker->ColorSpectrum.Background, !Activate);
    SetActive(ColorPicker->MoveNob, Activate);
    SetActive(ColorPicker->InnerDot, Activate);
    SetActive(ColorPicker->InnerInnerDot, Activate);
    SetActive(ColorPicker->ActiveColorBG, Activate);
    For(PALETTE_COLOR_AMOUNT)
    {
        SetActive(ColorPicker->PaletteColors[It].Outline, Activate);
        SetActive(ColorPicker->PaletteColors[It].Preview, Activate);
        SetActive(&ColorPicker->PaletteColors[It].Name, Activate);
    }
    SetActive(ColorPicker->New, Activate);
    SetActive(ColorPicker->Remove, Activate);
    SetActive(ColorPicker->Save, Activate);
    SetActive(ColorPicker->Cancel, Activate);
    SetActive(&ColorPicker->PaletteName, Activate);
    if(Activate) SetActive(ColorPicker->PaletteName.Cursor, false);
}

inline u32
ColorToColor32(v3 Color, u8 Alpha)
{
    u32 Result = 0;
    
    u8 Red   = (u8)(255*Color.r);
    u8 Blue  = (u8)(255*Color.g);
    u8 Green = (u8)(255*Color.b);
    
    Result = (Red << 0) | (Blue << 8) | (Green << 16) | (Alpha << 24);
    return Result;
}

internal v3
CalculateColorFromSpectrum(r32 SpectrumPercentage)
{
    v3 Result = {};
    r32 P = 1-SpectrumPercentage;
    r32 OneSixth = 1.0f/6.0f;
    
    r32 Red = 1;
    if(P >= OneSixth) Red = Clamp01(1-(P-OneSixth)*6);
    if(P >= OneSixth*4) Red = Clamp01((P-OneSixth*4)*6);
    
    r32 Green = 1;
    if(P <= OneSixth) Green = Clamp01(P*6);
    else if(P >= OneSixth*3) Green = Clamp01(1-(P-OneSixth*3)*6);
    
    r32 Blue = 0;
    if(P >= OneSixth*2 && P < OneSixth*5) Blue = Clamp01((P-OneSixth*2)*6);
    else if(P >= OneSixth*5) Blue = Clamp01(1-(P-OneSixth*5)*6);
    
    Result.r = Red;
    Result.g = Green;
    Result.b = Blue;
    
    return Result;
}

internal void
UpdateColorPickerTexture(color_picker *ColorPicker, v3 NewColor)
{
    loaded_bitmap *Bitmap = &ColorPicker->Bitmap;
    if(!Bitmap->Pixels) 
        Bitmap->Pixels = AllocateArray(&GlobalGameState.FixArena, Bitmap->Width*Bitmap->Height, u32);
    
    v3 NewColorInvert = V3(1)-NewColor;
    For(Bitmap->Height, Y_)
    {
        r32 YPercent = Y_It/(r32)Bitmap->Height;
        For(Bitmap->Width, X_)
        {
            i32 Pos = Y_It*Bitmap->Width + X_It;
            r32 XPercent = X_It/(r32)Bitmap->Width;
            
            v3 NextColor = Clamp01(NewColor+NewColorInvert*(1-XPercent))*YPercent;
#if 0
            if(Y_It == 0)
            {
                DebugLog(255, "%f %f %f\n", NextColor.x, NextColor.y, NextColor.z);
            }
#endif
            u8 Red   = (u8)(255*NextColor.r);
            u8 Blue  = (u8)(255*NextColor.g);
            u8 Green = (u8)(255*NextColor.b);
            Bitmap->Pixels[Pos] = (Red << 0) | (Blue << 8) | (Green << 16) | (255 << 24);
        }
    }
    
    if(!ColorPicker->GLID) ColorPicker->GLID = CreateGLTexture(*Bitmap);
    else UpdateGLTexture(*Bitmap, ColorPicker->GLID);
}

inline v3
RGBToHSV(v3 Color)
{
    v3 Result = {}; // x == Hue, y == Saturation, z == Value
    
    r32 CMax = Max(Color.r, Max(Color.g, Color.b));
    r32 CMin = Min(Color.r, Min(Color.g, Color.b));
    
    r32 CDelta = CMax - CMin;
    
    // Calculate Hue
    if(CDelta < 0.000001f)   Result.x = 0.0f;
    else if(CMax == Color.r) Result.x = 60*fmodf((Color.g-Color.b)/CDelta, 6.0f);
    else if(CMax == Color.g) Result.x = 60*((Color.b-Color.r)/CDelta + 2);
    else if(CMax == Color.b) Result.x = 60*((Color.r-Color.g)/CDelta + 4);
    if(Result.x < 0) Result.x += 360;
    
    // Calculate Saturation
    Result.y = SafeDiv(CDelta, CMax);
    
    // Calculate Value
    Result.z = CMax;
    
    return Result;
}

inline void
CreateRenderTextFromColor(color_picker *ColorPicker, v3 NewColor)
{
    renderer *Renderer = &GlobalGameState.Renderer;
    color_palette *Palette = &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette;
    
    i32 Red   = (i32)(255*NewColor.r);
    i32 Green = (i32)(255*NewColor.g);
    i32 Blue  = (i32)(255*NewColor.b);
    
    NewLocalString(ColorString, 22, "R: ");
    I32ToString(&ColorString, Red);
    AppendStringToCompound(&ColorString, (u8 *)"  G: ");
    I32ToString(&ColorString, Green);
    AppendStringToCompound(&ColorString, (u8 *)"  B: ");
    I32ToString(&ColorString, Blue);
    
    RemoveRenderText(Renderer, &ColorPicker->RGBText);
    RenderText(&GlobalGameState, &GlobalGameState.FixArena, font_Medium, &ColorString, &Palette->ForegroundText, 
               &ColorPicker->RGBText, -0.9f, ColorPicker->Background);
    SetLocalPosition(&ColorPicker->RGBText, ColorPicker->_BGOffset+V2(ColorPicker->RGBText.CurrentP.x*-0.5f, 
                                                                      ColorPicker->Bitmap.Height*-0.5f - 20.0f));
}

internal void
ColorToPickerPosition(color_picker *ColorPicker, v3 Color)
{
    v3 ColorHSV = RGBToHSV(Color);
    
    SetSliderPosition(&ColorPicker->ColorSpectrum, 1-(ColorHSV.x/360.0f));
    
    rect_2D TexRect = GetRect(ColorPicker->TextureEntry);
    
    v2 NewPickDotP = V2(TexRect.Min.x+ColorPicker->Bitmap.Width*ColorHSV.y, 
                        TexRect.Min.y+ColorPicker->Bitmap.Height*ColorHSV.z);
    SetPosition(ColorPicker->PickDot, NewPickDotP);
    
    v3 NewColor = CalculateColorFromSpectrum(ColorPicker->ColorSpectrum.SlidePercentage);
    UpdateColorPickerTexture(ColorPicker, NewColor);
    
    CreateRenderTextFromColor(ColorPicker, Color);
}

inline v3 
CalculatePickDotColor(color_picker *ColorPicker)
{
    v3 Result = {};
    
    v2 PickDotP = GetPosition(ColorPicker->PickDot);
    rect_2D TexRect = GetRect(ColorPicker->TextureEntry);
    PickDotP -= TexRect.Min;
    
    v3 NewColor = CalculateColorFromSpectrum(ColorPicker->ColorSpectrum.SlidePercentage);
    v3 NewColorInvert = V3(1)-NewColor;
    
    r32 YPercent = PickDotP.y/(r32)ColorPicker->Bitmap.Height;
    r32 XPercent = PickDotP.x/(r32)ColorPicker->Bitmap.Width;
    Result = Clamp01(NewColor+NewColorInvert*(1-XPercent))*YPercent;
    
    return Result;
}

internal void
UpdateColorPickerSelectedColor(color_picker *ColorPicker)
{
    color_palette *Palette = &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette;
    
    v3 NewColor = CalculatePickDotColor(ColorPicker);
    ColorPicker->SelectedColor = NewColor;
    Palette->Colors[ColorPicker->ActiveColor] = NewColor;
    SetColor(ColorPicker->PaletteColors[ColorPicker->ActiveColor].Preview, &Palette->Colors[ColorPicker->ActiveColor]);
    
    CreateRenderTextFromColor(ColorPicker, NewColor);
}

internal void
HandleColorPickerButtonAnimation(color_picker *ColorPicker, button *Btn, quit_animation *Anim)
{
    if(!Anim->Activated)
    {
        r32 Pushdown = 25;
        if(Btn->State == buttonState_Pressed)
        {
            v2 Position = GetPosition(ColorPicker->Background)+V2(0, GetExtends(ColorPicker->Background).y-Pushdown);
            v2 Size = GetSize(ColorPicker->Background)-V2(0, Pushdown);
            if(QuitAnimation(Anim, 1, Position, Size))
            {
                Btn->OnPressed.Func(Btn->OnPressed.Data);
                SetActive(Anim, false);
            }
        }
        else if(Anim->dAnim != 0)
        {
            v2 Position = GetPosition(ColorPicker->Background)+V2(0, GetExtends(ColorPicker->Background).y-Pushdown);
            v2 Size = GetSize(ColorPicker->Background)-V2(0, Pushdown);
            if(QuitAnimation(Anim, -1, Position, Size))
            {
                SetActive(Anim, false);
            }
        }
    }
    if(Btn->State == buttonState_Unpressed) Anim->Activated = false;
}

internal void
HandleActiveColorPicker(color_picker *ColorPicker)
{
    input_info *Input = &GlobalGameState.Input;
    if(ColorPicker->ColorSpectrum.SliderIsDragged)
    {
        ColorPicker->ColorSpectrum.SliderIsDragged = false;
        
        v3 NewColor = CalculateColorFromSpectrum(ColorPicker->ColorSpectrum.SlidePercentage);
        UpdateColorPickerTexture(ColorPicker, NewColor);
        
        UpdateColorPickerSelectedColor(ColorPicker);
    }
    else if(Input->KeyChange[KEY_LMB] == KeyDown)
    {
        if(IsInRect(ColorPicker->TextureEntry, Input->MouseP))
        {
            ColorPicker->IsPickingColor = true;
        }
        if(IsInRect(ColorPicker->MoveNob, Input->MouseP))
        {
            ColorPicker->IsMoving = true;
            ColorPicker->MoveOffset = GetPosition(ColorPicker->MoveNob)-Input->MouseP;
        }
    }
    else if(Input->KeyChange[KEY_LMB] == KeyUp) 
    {
        if(!ColorPicker->IsPickingColor)
        {
            For(PALETTE_COLOR_AMOUNT)
            {
                if(IsInRect(ColorPicker->PaletteColors[It].Outline, Input->MouseP))
                {
                    ColorPicker->ActiveColor = It;
                    SetParent(ColorPicker->ActiveColorBG, ColorPicker->PaletteColors[It].Outline);
                    ColorToPickerPosition(ColorPicker, GetColor(ColorPicker->PaletteColors[It].Preview));
                    SetColor(ColorPicker->InnerDot, &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.Colors[It]);
                    break;
                }
            }
        }
        ColorPicker->IsPickingColor = false;
        ColorPicker->IsMoving = false;
    }
    else if(Input->Pressed[KEY_LMB])
    {
        if(ColorPicker->IsPickingColor)
        {
            v2 MouseP = ClampToRect(Input->MouseP, ColorPicker->TextureEntry);
            SetPosition(ColorPicker->PickDot, MouseP);
            UpdateColorPickerSelectedColor(ColorPicker);
        }
        else if(ColorPicker->IsMoving)
        {
            SetPosition(ColorPicker->MoveNob, ColorPicker->MoveOffset+Input->MouseP);
        }
    }
    
    // Process button presses
    ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, ColorPicker->New);
    if(!IsColorPaletteDefault()) 
    {
        ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, ColorPicker->Remove);
        ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, ColorPicker->Save);
    }
    ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, ColorPicker->Cancel);
    
    // Update stuff on palette change
    if(ColorPicker->CurrentColorPaletteID != GlobalGameState.MusicInfo.DisplayInfo.ColorPaletteID)
    {
        ColorPicker->CurrentColorPaletteID = GlobalGameState.MusicInfo.DisplayInfo.ColorPaletteID;
        
        // Update Textfield text
        ResetStringCompound(ColorPicker->PaletteName.TextString);
        AppendStringCompoundToCompound(&ColorPicker->PaletteName.TextString, GetCurrentPaletteName());
        UpdateTextField(&GlobalGameState.Renderer, &ColorPicker->PaletteName);
        
        // Change buttons colors if deactivated
        if(IsColorPaletteDefault()) 
        {
            SetColor(ColorPicker->Save->Icon, &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.ErrorText);
            SetColor(ColorPicker->Remove->Icon, &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.ErrorText);
            
            SetActive(ColorPicker->PaletteName.Cursor, false);
        }
        else 
        {
            SetColor(ColorPicker->Save->Icon, GlobalGameState.Renderer.ButtonColors.IconColor);
            SetColor(ColorPicker->Remove->Icon, GlobalGameState.Renderer.ButtonColors.IconColor);
        }
        ColorToPickerPosition(ColorPicker, GetColor(ColorPicker->PaletteColors[0].Preview));
        SetColor(ColorPicker->InnerDot, &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.Colors[0]);
    }
    
    // process text field
    if(!IsColorPaletteDefault()) 
    {
        text_field_flag_result TFResult = ProcessTextField(&GlobalGameState.Renderer, GlobalGameState.Time.dTime, 
                                                           Input, &ColorPicker->PaletteName);
        if(TFResult.Flag & processTextField_TextChanged)
        {
        }
        if(TFResult.Flag & processTextField_Confirmed)
        {
        }
    }
    
    // Handle Remove button animation
    HandleColorPickerButtonAnimation(ColorPicker, ColorPicker->New, &ColorPicker->NewAnim);
    HandleColorPickerButtonAnimation(ColorPicker, ColorPicker->Save, &ColorPicker->SaveAnim);
    HandleColorPickerButtonAnimation(ColorPicker, ColorPicker->Remove, &ColorPicker->RemoveAnim);
}

inline void
OnNewPalette(void *Data)
{
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    color_picker *ColorPicker = (color_picker *)Data;
    
    if(ColorPicker->NewAnim.Activated)
    {
        AddCustomColorPalette(&DisplayInfo->ColorPalette, &ColorPicker->PaletteName.TextString);
        
        // Move to the now saved palette!
        DisplayInfo->ColorPaletteID = DEFAULT_COLOR_PALETTE_COUNT+GlobalGameState.Settings.PaletteCount-1;
        UpdateColorPalette(DisplayInfo, false);
    }
}

inline void
OnRemovePalette(void *Data)
{
    quit_animation *Anim = (quit_animation *)Data;
    if(Anim->Activated)
    {
        music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
        RemoveCustomColorPalette(DisplayInfo->ColorPaletteID);
        UpdateColorPalette(DisplayInfo, false);
    }
}

inline void
OnSavePalette(void *Data)
{
    color_picker *ColorPicker = (color_picker *)Data;
    
    if(ColorPicker->SaveAnim.Activated)
    {
        settings *Settings = &GlobalGameState.Settings;
        music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
        u32 CustomID = DisplayInfo->ColorPaletteID-DEFAULT_COLOR_PALETTE_COUNT;
        Assert(CustomID < Settings->PaletteCount);
        
        if(ColorPicker->PaletteName.TextString.Pos >= 100) ColorPicker->PaletteName.TextString.Pos = 100;
        ResetStringCompound(Settings->PaletteNames[CustomID]);
        AppendStringCompoundToCompound(Settings->PaletteNames+CustomID, &ColorPicker->PaletteName.TextString);
        For(PALETTE_COLOR_AMOUNT)
        {
            Settings->Palettes[CustomID].Colors[It] = DisplayInfo->ColorPalette.Colors[It]*255.0f;
        }
    }
}

inline void
OnCancelColorPicker(void *Data)
{
    SetActive((color_picker *)Data, false);
}

internal void
CreateColorPicker(color_picker *Result, v2i BitmapSize)
{
    renderer *Renderer = &GlobalGameState.Renderer;
    color_palette *Palette = &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette;
    r32 Depth = -0.9f;
    
    Result->IsActive = true;
    Result->Bitmap.ColorFormat = colorFormat_RGBA;
    Result->Bitmap.Width  = BitmapSize.x;
    Result->Bitmap.Height = BitmapSize.y;
    Result->Bitmap.WasLoaded = true;
    
    v2 MainPos = V2(GlobalGameState.Renderer.Window.CurrentDim.Dim)*0.5f;
    Result->_BGOffset = V2(-70, 20-12.5f-13);
    v2 BGOffset = Result->_BGOffset;
    v2i ColorSpectrumSize = V2i(50, BitmapSize.y);
    
    // Create Move Nob
    v2 BGSize = V2(BitmapSize)+V2(ColorSpectrumSize.x + 260.0f, 80+26);
    Result->MoveNob = CreateRenderRect(Renderer, V2(BGSize.x, 25.0f), 
                                       Depth+0.001f, &Palette->SliderGrabThing, 0);
    SetPosition(Result->MoveNob, MainPos+V2(0, BGSize.y*0.5f+12.5f));
    TranslateWithScreen(&Renderer->TransformList, Result->MoveNob, fixedTo_TopLeft);
    
    // Create Background
    Result->Background = CreateRenderRect(Renderer, BGSize+V2(0, 25), Depth+0.0011f, 
                                          &Palette->Foreground, Result->MoveNob);
    //SetTransparency(Result->Background, 0.95f);
    SetPosition(Result->Background, MainPos+V2(0, 12.5f));
    
    // Create brightness slider
    loaded_bitmap ColorSpectrumBitmap = {};
    ColorSpectrumBitmap.Pixels = AllocateArray(&GlobalGameState.FixArena, ColorSpectrumSize.x*ColorSpectrumSize.y, u32);
    ColorSpectrumBitmap.ColorFormat = colorFormat_RGBA;
    ColorSpectrumBitmap.Width  = ColorSpectrumSize.x;
    ColorSpectrumBitmap.Height = ColorSpectrumSize.y;
    ColorSpectrumBitmap.WasLoaded = true;
    
    // Generate color spectrum texture
    For((u32)ColorSpectrumSize.y, Y_)
    {
        r32 YPercent = Y_It/(r32)ColorSpectrumSize.y;
        For((u32)ColorSpectrumSize.x, X_)
        {
            i32 Pos = Y_It*ColorSpectrumSize.x + X_It;
            
            v3 NewColor = CalculateColorFromSpectrum(YPercent);
            
            ColorSpectrumBitmap.Pixels[Pos] = ColorToColor32(NewColor, 255);
        }
    }
    
    CreateSlider(&Result->ColorSpectrum, Renderer, V2(ColorSpectrumSize), 
                 V2((r32)ColorSpectrumSize.x+8.0f, 8.0f), Depth, ColorSpectrumBitmap, &Palette->SliderGrabThing, 
                 sliderAxis_Y, Result->Background);
    SetLocalPosition(&Result->ColorSpectrum, BGOffset-V2(BitmapSize.x*0.5f + ColorSpectrumSize.x*0.5f + 15, 0));
    
    // Create color texture
    v3 NewColor = CalculateColorFromSpectrum(Result->ColorSpectrum.SlidePercentage);
    UpdateColorPickerTexture(Result, NewColor);
    Result->TextureEntry = CreateRenderBitmap(Renderer, V2(BitmapSize), Depth, 
                                              Result->Background, Result->GLID);
    SetLocalPosition(Result->TextureEntry, BGOffset);
    
    // Create Pick dot
    Result->PickDot = CreateRenderRect(Renderer, V2(8,8), Depth-0.001f, &Palette->Text, Result->Background);
    SetLocalPosition(Result->PickDot, BGOffset);
    Result->InnerInnerDot = CreateRenderRect(Renderer, V2(6,6), Depth-0.0011f, &Palette->Slot, Result->PickDot);
    Result->InnerDot = CreateRenderRect(Renderer, V2(4,4), Depth-0.0012f, &Palette->Text, Result->PickDot);
    
    // Create palette colors
    v2 StartP = BGOffset+V2(BitmapSize)*0.5f+V2(25, -20);
    r32 YDown = 0;
    For(PALETTE_COLOR_AMOUNT)
    {
        palette_color *C = Result->PaletteColors+It; 
        C->Outline = CreateRenderRect(Renderer, V2(29,29), Depth-0.001f, 
                                      &Palette->ForegroundText, Result->Background);
        SetLocalPosition(C->Outline, StartP + V2(0, YDown));
        C->Preview = CreateRenderRect(Renderer, V2(25,25), Depth-0.002f, 
                                      Palette->Colors+It, C->Outline);
        RenderText(&GlobalGameState, &GlobalGameState.FixArena, font_Small, GlobalPaletteColorNames+It,
                   &Palette->ForegroundText, &C->Name, Depth-0.001f, C->Preview, V2(20, 0));
        
        YDown -= 51;
    }
    
    // Create active color bg
    Result->ActiveColorBG = CreateRenderRect(Renderer, V2(210, 40), Depth-0.00011f, 
                                             &Palette->SliderGrabThing, Result->PaletteColors[0].Outline);
    SetLocalPosition(Result->ActiveColorBG, V2(85, 0));
    
    
    // Create Buttons
#if RESOURCE_PNG
    u32 NewID    = DecodeAndCreateGLTexture(Plus_Icon_DataCount, (u8 *)Plus_Icon_Data);
    u32 RemoveID = DecodeAndCreateGLTexture(Minus_Icon_DataCount, (u8 *)Minus_Icon_Data);
    u32 SaveID   = DecodeAndCreateGLTexture(Save_Icon_DataCount, (u8 *)Save_Icon_Data);
    u32 CancelID = DecodeAndCreateGLTexture(Cancel_Icon_DataCount, (u8 *)Cancel_Icon_Data);
#else
    loaded_bitmap Bitmap = {1, Plus_Icon_Width, Plus_Icon_Height, (u32 *)Plus_Icon_Data, colorFormat_RGBA, ArrayCount(Plus_Icon_Data)};
    u32 NewID    = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
    
    Bitmap = {1, Minus_Icon_Width, Minus_Icon_Height, (u32 *)Minus_Icon_Data, colorFormat_RGBA, ArrayCount(Minus_Icon_Data)};
    u32 RemoveID = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
    
    Bitmap = {1, Save_Icon_Width, Save_Icon_Height, (u32 *)Save_Icon_Data, colorFormat_RGBA, ArrayCount(Save_Icon_Data)};
    u32 SaveID   = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
    
    Bitmap = {1, Cancel_Icon_Width, Cancel_Icon_Height, (u32 *)Cancel_Icon_Data, colorFormat_RGBA, ArrayCount(Cancel_Icon_Data)};
    u32 CancelID = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, Bitmap);
#endif
    
    rect BtnRect = {{-21,-21},{21,21}};
    Result->New    = NewButton(Renderer, BtnRect, Depth-0.001f, false, Renderer->ButtonBaseID, NewID, 
                               Renderer->ButtonColors, Result->Background);
    Result->Remove = NewButton(Renderer, BtnRect, Depth-0.001f, false, Renderer->ButtonBaseID, RemoveID, 
                               Renderer->ButtonColors, Result->Background);
    Result->Save   = NewButton(Renderer, BtnRect, Depth-0.001f, false, Renderer->ButtonBaseID, SaveID, 
                               Renderer->ButtonColors, Result->Background);
    Result->Cancel = NewButton(Renderer, BtnRect, Depth-0.001f, false, Renderer->ButtonBaseID, CancelID, 
                               Renderer->ButtonColors, Result->Background);
    
    r32 ButtonGap = 10;
    v2  BtnBaseOffset = BGOffset+V2((r32)BitmapSize.x, -(r32)BitmapSize.y)*0.5f;
    Translate(Result->New,    BtnBaseOffset + V2(21+10 + (42+ButtonGap)*0, -21-8));
    Translate(Result->Remove, BtnBaseOffset + V2(21+10 + (42+ButtonGap)*1, -21-8));
    Translate(Result->Save,   BtnBaseOffset + V2(21+10 + (42+ButtonGap)*2, -21-8));
    Translate(Result->Cancel, BtnBaseOffset + V2(21+10 + (42+ButtonGap)*3, -21-8));
    
    Result->New->OnPressed    = {OnNewPalette, Result};
    Result->Remove->OnPressed = {OnRemovePalette, &Result->RemoveAnim};
    Result->Save->OnPressed   = {OnSavePalette, Result};
    Result->Cancel->OnPressed = {OnCancelColorPicker, Result};
    
    string_c NewText    = NewStaticStringCompound("New Color Palette.");
    string_c SaveText   = NewStaticStringCompound("Saving current Color Palette.");
    string_c RemoveText = NewStaticStringCompound("Remove this Color Palette?");
    
    CreateQuitAnimation(&Result->NewAnim, V2(BitmapSize), &NewText, 0.5f);
    CreateQuitAnimation(&Result->SaveAnim, V2(BitmapSize), &SaveText, 0.75f);
    CreateQuitAnimation(&Result->RemoveAnim, V2(BitmapSize), &RemoveText, 1.2f);
    
    // Create Textfield for palette names ****************
    Result->PaletteName = CreateTextField(Renderer, &GlobalGameState.FixArena, V2((r32)BitmapSize.x, 20.0f), 
                                          Depth-0.01f, (u8 *)"Custom Palette", Result->Background, 
                                          &Palette->ForegroundText, &Palette->Foreground);
    Translate(&Result->PaletteName, BGOffset + V2(-8.0f, BitmapSize.y*0.5f + 10 + 11));
    SetActive(&Result->PaletteName, true);
    AppendStringCompoundToCompound(&Result->PaletteName.TextString, GetCurrentPaletteName());
    UpdateTextField(Renderer, &Result->PaletteName);
    
    Result->CurrentColorPaletteID = MAX_UINT32;
    ColorToPickerPosition(Result, GetColor(Result->PaletteColors[0].Preview));
    SetActive(Result, true);
}











