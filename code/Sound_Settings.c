#include "Sound_Settings.h"

internal v3 CalculateColorFromSpectrum(r32 SpectrumPercentage);
inline u32 ColorToColor32(v3 Color, u8 Alpha);
internal void UpdateColorPickerTexture(color_picker *ColorPicker, v3 NewColor);
internal void ColorToPickerPosition(color_picker *ColorPicker, v3 Color);

internal void
CreateStyleSettings(game_state *GS, style_settings_window *StyleSettings)
{
    renderer *Renderer = &GS->Renderer;
    layout_definition *Layout = &GS->Layout;
    color_palette *Palette = &GS->MusicInfo.DisplayInfo.ColorPalette;
    r32 MediumFontHeight   = GetFontSize(Renderer, font_Medium).Size;
    
    v2 ColorPickerFieldSize = V2(Layout->ColorPickerColorFieldSide);
    r32 Depth = Layout->SettingsWindowDepth;
    v2 MainPos = V2(Renderer->Window.CurrentDim.Dim)*0.5f;
    
    // Create Move Nob
    r32   BorderTopHeight = Layout->SettingsWindowBorderTop + Layout->SettingsWindowBorder + Layout->SettingsWindowHeight;
    v2             BGSize = V2(Layout->SettingsWindowWidth, Layout->SettingsWindowHeight);
    StyleSettings->Border = CreateRenderRect(Renderer, V2(BGSize.x + Layout->SettingsWindowBorder, BorderTopHeight), 
                                             Depth+0.0011f, &Palette->SliderGrabThing, 0);
    SetPosition(StyleSettings->Border, MainPos+V2(0, Layout->SettingsWindowBorderTop));
    TranslateWithScreen(&Renderer->TransformList, StyleSettings->Border, fixedTo_TopLeft);
    
    // Create Background
    StyleSettings->Background = CreateRenderRect(Renderer, BGSize, Depth+0.001f,&Palette->Foreground, StyleSettings->Border);
    SetPosition(StyleSettings->Background, MainPos+V2(0, 12.5f));
    
    // Initialize FontSettings
    StyleSettings->FontParent = CreateRenderRect(Renderer, {}, Depth, StyleSettings->Background, &Palette->Text);
    CreateFontSettings(GS, &StyleSettings->FontSettings, StyleSettings->FontParent);
    
    // Initialize ColorPicker
    StyleSettings->ColorPickerParent = CreateRenderRect(Renderer, {}, Depth, StyleSettings->Background, &Palette->Text);
    CreateColorPicker(GS, &StyleSettings->ColorPicker, ColorPickerFieldSize, Layout->ColorPickerSpectrumWidth, StyleSettings->ColorPickerParent);
    
    r32 FontSettingsWidth = GetSize(StyleSettings->FontSettings.Background).x;
    r32 ColorPickerWidth  = GetSize(StyleSettings->ColorPicker.Background).x;
    r32 BackgroundWidth   = FontSettingsWidth+ColorPickerWidth;
    SetSize(StyleSettings->Background, V2(BackgroundWidth, Layout->SettingsWindowHeight));
    SetSize(StyleSettings->Border, V2(BackgroundWidth + Layout->SettingsWindowBorder, BorderTopHeight));
    
    r32 FontSettingsX = BackgroundWidth*0.5f - FontSettingsWidth*0.5f;
    SetLocalPosition(StyleSettings->FontParent, V2(FontSettingsX, 0));
    r32 ColorPickerParentX = -BackgroundWidth*0.5f + ColorPickerWidth*0.5f;
    SetLocalPosition(StyleSettings->ColorPickerParent, V2(ColorPickerParentX, 0));
    
    // Create cancel button.
#if RESOURCE_PNG
    u32 CancelID = DecodeAndCreateGLTexture(Cancel_Icon_DataCount, (u8 *)Cancel_Icon_Data);
#else
    u32 CancelID = DecodeAndCreateGLTexture(&GlobalGameState.ScratchArena, NewBitmapData(Cancel_Icon, colorFormat_RGBA));
#endif
    
    r32 BtnExtends = Layout->MediumButtonExtents;
    rect BtnRect = {{-BtnExtends,-BtnExtends},{BtnExtends,BtnExtends}};
    StyleSettings->Cancel = NewButton(Renderer, BtnRect, Depth-0.003f, false, Renderer->ButtonBaseID, CancelID, 
                                      Renderer->ButtonColors, StyleSettings->Background);
    v2 AddOffset = V2(Layout->ColorPickerButtonGapY, -Layout->ColorPickerButtonGapY);
    Translate(StyleSettings->Cancel, V2(BackgroundWidth*0.5f, -BGSize.y*0.5f) - V2(BtnExtends, -BtnExtends) - AddOffset);
    StyleSettings->Cancel->OnPressed = {OnStyleSettings, StyleSettings};
    
    StyleSettings->IsActive = true;
}

inline void
SetActive(style_settings_window *StyleWindow, b32 Activate)
{
    StyleWindow->IsActive = Activate;
    SetActive(StyleWindow->Background, Activate);
    SetActive(StyleWindow->Border, Activate);
    SetActive(StyleWindow->Cancel, Activate);
    SetActive(&StyleWindow->ColorPicker, Activate);
    SetActive(&StyleWindow->FontSettings, Activate); // Needs to come _after_ SetActive(ColorPicker)
    if(Activate) GlobalGameState.CursorState = cursorState_Arrow;
}

internal void
HandleActiveStyleSettings(game_state *GS, style_settings_window *StyleSettings, input_info *Input)
{
    if(Input->KeyChange[KEY_LMB] == KeyDown)
    {
        if(IsInRect(StyleSettings->Border, Input->MouseP) && 
           !IsInRect(StyleSettings->Background, Input->MouseP))
        {
            StyleSettings->IsMoving = true;
            StyleSettings->MoveOffset = GetPosition(StyleSettings->Border)-Input->MouseP;
        }
    }
    else if(Input->KeyChange[KEY_LMB] == KeyUp) 
    {
        StyleSettings->IsMoving = false;
    }
    else if(Input->Pressed[KEY_LMB])
    {
        if(StyleSettings->IsMoving)
        {
            SetPosition(StyleSettings->Border, StyleSettings->MoveOffset+Input->MouseP);
            UpdateOriginalPosition(&GS->Renderer.TransformList, StyleSettings->Border);
        }
    }
    
    ButtonTestMouseInteraction(&GS->Renderer, Input, StyleSettings->Cancel);
    HandleActiveColorPicker(GS,  &StyleSettings->ColorPicker);
    HandleActiveFontSettings(GS, &StyleSettings->FontSettings);
}

inline void
OnStyleSettings(void *Data)
{
    style_settings_window * StyleSettings = (style_settings_window *)Data;
    SetActive(StyleSettings, !StyleSettings->IsActive);
}

internal void
UpdateSettings(game_state *GS)
{
    style_settings_window *StyleSettings = &GS->StyleSettings;
    color_picker *ColorPicker = &StyleSettings->ColorPicker;
    
    r32 TextWidth     = GetColorPickerTextWidth(GS);
    r32 SpectrumWidth = GetSize(&ColorPicker->ColorSpectrum).x;
    r32 FieldWidth    = GetSize(ColorPicker->TextureEntry).x;
    r32 ContentOffX   = SpectrumWidth - TextWidth*0.5f;
    v2  ContentOffset = V2(ContentOffX, GS->Layout.ColorPickerContentOffsetY);
    
    r32 PrevRightX = GetPosition(ColorPicker->ActiveColorBG).x + GetSize(ColorPicker->ActiveColorBG).x*0.5f;
    CreateColorPickerPaletteList(GS, ColorPicker, V2(ContentOffX, ContentOffset.y));
    
    v2 BGSize = V2(FieldWidth + SpectrumWidth*2 + TextWidth, GS->Layout.SettingsWindowHeight);
    SetSize(ColorPicker->Background, BGSize);
    
    r32 RightX = GetPosition(ColorPicker->ActiveColorBG).x + GetSize(ColorPicker->ActiveColorBG).x*0.5f;
    Translate(ColorPicker->FixedSizeHolder, V2(PrevRightX - RightX, 0));
    
    // Style Settings update
    r32 FontSettingsWidth = GetSize(StyleSettings->FontSettings.Background).x;
    r32 BackgroundWidth   = FontSettingsWidth+BGSize.x;
    r32    PrevStyleWidth = GetSize(StyleSettings->Background).x;
    SetSize(StyleSettings->Background, V2(BackgroundWidth, GS->Layout.SettingsWindowHeight));
    SetSize(StyleSettings->Border, V2(BackgroundWidth + GS->Layout.SettingsWindowBorder, GetSize(StyleSettings->Border).y));
    r32 MoveXDist  = -(BackgroundWidth - PrevStyleWidth)*0.5f;
    Translate(StyleSettings->Border, V2(MoveXDist, 0));
    UpdateOriginalPosition(&GS->Renderer.TransformList, StyleSettings->Border);
    
    r32 FontSettingsX = BackgroundWidth*0.5f - FontSettingsWidth*0.5f;
    SetLocalPosition(StyleSettings->FontParent, V2(FontSettingsX, 0));
    r32 ColorPickerParentX = -BackgroundWidth*0.5f + BGSize.x*0.5f;
    SetLocalPosition(StyleSettings->ColorPickerParent, V2(ColorPickerParentX, 0));
    
    r32 BtnExtends = GS->Layout.MediumButtonExtents;
    v2   AddOffset = V2(GS->Layout.ColorPickerButtonGapY, -GS->Layout.ColorPickerButtonGapY);
    SetLocalPosition(StyleSettings->Cancel, V2(BackgroundWidth*0.5f, -BGSize.y*0.5f) - V2(BtnExtends, -BtnExtends) - AddOffset);
    
    // Font Settings update
    NewLocalString(Text, 27, "abcdefghijklmnopqrstuvwxyz");
    font_metrics SmallFontMetrics = GetFontMetrics(GS, font_Small, Text);
    UpdateTextField(&GS->Renderer, &GS->StyleSettings.FontSettings.ActiveFont);
    v2 NewSize = V2(GS->Layout.FontSliderExtendsX*2 - GS->Layout.SearchButtonExtents*2, 
                    SmallFontMetrics.Ascent - SmallFontMetrics.Descent);
    SetSize(&StyleSettings->FontSettings.ActiveFont, NewSize);
    SetActive(&StyleSettings->FontSettings.ActiveFont, GS->StyleSettings.IsActive);
    
    font_metrics MediumFontMetrics = GetFontMetrics(GS, font_Medium, Text);
    r32 PaletteNameWidth = GetSize(&ColorPicker->PaletteName).x;
    SetSize(&ColorPicker->PaletteName, V2(PaletteNameWidth, MediumFontMetrics.Ascent - MediumFontMetrics.Descent));
}

// Font settings ********************************************

internal b32
FinalizeFontChange(game_state *GS, string_c Path)
{
    b32 PathIsValid = true;
    if(Path.Pos == 0)
    {
        ResetStringCompound(GS->Settings.FontPath);
        ChangeFontSizes(GS, GS->Renderer.FontSizes);
    }
    else
    {
        PathIsValid = CheckFileExists(&GS->ScratchArena, Path);
        if(PathIsValid)
        {
            NewLocalString(TtfString, 5, ".ttf");
            NewLocalString(OtfString, 5, ".otf");
            NewLocalString(TTFString, 5, ".TTF");
            NewLocalString(OTFString, 5, ".OTF");
            PathIsValid = StringEndsWith(Path, TtfString) || StringEndsWith(Path, OtfString)
                || StringEndsWith(Path, TTFString) || StringEndsWith(Path, OTFString);
            if(PathIsValid)
            {
                if(GS->Settings.FontPath.Length < 260)
                    GS->Settings.FontPath = NewStringCompound(&GS->FixArena, 260);
                CopyIntoCompound(&GS->Settings.FontPath, &Path);
                ChangeFontSizes(GS, GS->Renderer.FontSizes);
            }
        }
    }
    return PathIsValid;
}

internal void
OnToggleEditFont(void *Data)
{
    game_state *GS = &GlobalGameState;
    font_settings *FontSettings = (font_settings *)Data;
    FontSettings->EditingFont = !FontSettings->EditingFont;
    
    if(!FontSettings->EditingFont)
    {
        b32 Succeeded = FinalizeFontChange(GS, FontSettings->ActiveFont.TextString);
        if(!Succeeded)
        {
            ResetStringCompound(FontSettings->ActiveFont.TextString);
            UpdateTextField(&GS->Renderer, &FontSettings->ActiveFont);
        }
    }
    
    SetActive(FontSettings->ActiveFont.Cursor, FontSettings->EditingFont);
}

internal void
CreateFontSettings(game_state *GS, font_settings *FontSettings, entry_id *Parent)
{
    layout_definition *Layout = &GS->Layout;
    color_palette *Palette = &GS->MusicInfo.DisplayInfo.ColorPalette;
    NewLocalString(ChangeSmallText,  10, "Small");
    NewLocalString(ChangeMediumText, 10, "Medium");
    
    font_metrics SmallFontMetrics  = GetFontMetrics(GS, font_Small, ChangeSmallText);
    font_metrics MediumFontMetrics = GetFontMetrics(GS, font_Medium, ChangeMediumText);
    
    FontSettings->Parent = Parent;
    r32 Depth = GetDepth(Parent) - 0.000001f;
    
    v2 BaseOffset = {};//{Layout->FontSettingsOffsetX, Layout->FontSettingsOffsetY};
    v2 BGSize     = V2(Layout->FontSettingsBGWidth, Layout->SettingsWindowHeight);
    FontSettings->Background = CreateRenderRect(&GS->Renderer, BGSize, Depth+0.0008f, &Palette->Slot, Parent);
    SetLocalPosition(FontSettings->Background, BaseOffset);
    
    Parent = FontSettings->Background;
    BaseOffset = V2(Layout->FontSettingsContentOffsetX, Layout->FontSettingsContentOffsetY + Layout->ColorPickerOffsetY);
    v2 SliderExtends = {Layout->FontSliderExtendsX, Layout->FontSliderExtendsY};
    v2    FontOffset = BaseOffset + V2(-SliderExtends.x, 0);
    v2  SliderOffset = V2(BaseOffset.x, FontOffset.y + SmallFontMetrics.Descent - Layout->FontSliderGrabY);
    rect_pe   BGRect = {SliderOffset, SliderExtends};
    rect_pe GrabRect = {SliderOffset, {Layout->FontSliderGrabX, Layout->FontSliderGrabY}};
    
    RenderText(GS, font_Small, &ChangeSmallText, &GS->MusicInfo.DisplayInfo.ColorPalette.Text, &FontSettings->SmallFontText, Depth + 0.00001f, Parent, FontOffset);
    SetScissor(&FontSettings->SmallFontText, FontSettings->Background);
    
    CreateSlider(GS, &FontSettings->SmallFontSlider, sliderAxis_X, Rect(BGRect), Rect(GrabRect), Depth, true, Parent);
    r32 CurSmallSize = GS->Renderer.FontSizes.Sizes[font_Small].Size;
    r32 SmallPercentage = (CurSmallSize-GS->Layout.SmallFontMin)/GS->Layout.SmallFontMax;
    SetSliderPosition(&FontSettings->SmallFontSlider, SmallPercentage);
    
    FontOffset.y += -GrabRect.Extends.y*2 + SmallFontMetrics.Descent - MediumFontMetrics.Ascent - Layout->FontSizeSliderGap;
    BGRect.Pos.y  = FontOffset.y + SmallFontMetrics.Descent - GrabRect.Extends.y;
    
    RenderText(GS, font_Medium, &ChangeMediumText, &GS->MusicInfo.DisplayInfo.ColorPalette.Text, &FontSettings->MediumFontText, Depth + 0.00001f, Parent, FontOffset);
    SetScissor(&FontSettings->MediumFontText, FontSettings->Background);
    
    CreateSlider(GS, &FontSettings->MediumFontSlider, sliderAxis_X, Rect(BGRect), Rect(GrabRect), Depth, true, Parent);
    r32 CurMediumSize = GS->Renderer.FontSizes.Sizes[font_Medium].Size;
    r32 MediumPercentage = (CurMediumSize-GS->Layout.MediumFontMin)/GS->Layout.MediumFontMax;
    SetSliderPosition(&FontSettings->MediumFontSlider, MediumPercentage);
    
    
    // Create Textfield for active font ****************
#if RESOURCE_PNG
    u32 EditID = DecodeAndCreateGLTexture(Playlist_Rename_Icon_DataCount, (u8 *)Playlist_Rename_Icon_Data);
#else
    u32 EditID = DecodeAndCreateGLTexture(&GS->ScratchArena, NewBitmapData(Playlist_Rename_Icon, colorFormat_RGBA));
#endif
    
    r32      BtnExt = Layout->SearchButtonExtents;
    rect    BtnRect = {{-BtnExt, -BtnExt}, {BtnExt, BtnExt}};
    v2 ButtonOffset = BGRect.Pos - V2(BGRect.Extends.x - BtnExt, BtnExt*2 + Layout->FontSizeSliderGap + MediumFontMetrics.Ascent);
    
    // TODO:: Make this one a toggle button?
    FontSettings->EditFont = NewButton(&GS->Renderer, BtnRect, Depth, false, GS->Renderer.ButtonBaseID, EditID, 
                                       GS->Renderer.ButtonColors, Parent);
    Translate(FontSettings->EditFont, ButtonOffset);
    FontSettings->EditFont->OnPressed = {OnToggleEditFont, FontSettings};
    
    v2 TextFieldSize = V2(Layout->FontSliderExtendsX*2 - BtnExt*2, SmallFontMetrics.Ascent - SmallFontMetrics.Descent);
    FontSettings->ActiveFont = CreateTextField(&GS->Renderer, &GS->FixArena, TextFieldSize, Depth, 
                                               (u8 *)"Custom Font Path", Parent, 
                                               &Palette->ForegroundText, &Palette->SliderBackground, font_Small);
    Translate(&FontSettings->ActiveFont, ButtonOffset + V2(Layout->FontSliderExtendsX, 0));
    SetActive(&FontSettings->ActiveFont, true);
    
    AppendStringCompoundToCompound(&FontSettings->ActiveFont.TextString, &GS->Settings.FontPath);
    UpdateTextField(&GS->Renderer, &FontSettings->ActiveFont);
}

inline void
SetActive(font_settings *FontSettings, b32 Activate)
{
    SetActive(&GlobalGameState.DragableList, FontSettings->SmallFontSlider.Background, Activate); 
    SetActive(&GlobalGameState.DragableList, FontSettings->MediumFontSlider.Background, Activate);
    SetActive(&FontSettings->SmallFontSlider, Activate);
    SetActive(&FontSettings->MediumFontSlider, Activate);
    SetActive(&FontSettings->SmallFontText, Activate);
    SetActive(&FontSettings->MediumFontText, Activate);
    SetActive(FontSettings->EditFont, Activate);
    SetActive(&FontSettings->ActiveFont, Activate);
    SetActive(FontSettings->Background, Activate);
}


internal void
HandleActiveFontSettings(game_state *GS, font_settings *FontSettings)
{
    if(FontSettings->SmallFontSlider.SliderIsDragged)
    {
        FontSettings->SmallFontSlider.SliderIsDragged = false;
        
        r32 Percentage       = FontSettings->SmallFontSlider.SlidePercentage;
        r32 SmallFontMin     = GS->Layout.SmallFontMin;
        r32 SmallFontMax     = GS->Layout.SmallFontMax;
        r32 NewSmallFontSize = SmallFontMin + (SmallFontMax - SmallFontMin)*Percentage;
        
        GS->Renderer.FontSizes.Sizes[font_Small].Size = NewSmallFontSize;
        ChangeFontSizes(GS, GS->Renderer.FontSizes);
    }
    if(FontSettings->MediumFontSlider.SliderIsDragged)
    {
        FontSettings->MediumFontSlider.SliderIsDragged = false;
        
        r32 Percentage       = FontSettings->MediumFontSlider.SlidePercentage;
        r32 MediumFontMin     = GS->Layout.MediumFontMin;
        r32 MediumFontMax     = GS->Layout.MediumFontMax;
        r32 NewMediumFontSize = MediumFontMin + (MediumFontMax - MediumFontMin)*Percentage;
        
        GS->Renderer.FontSizes.Sizes[font_Medium].Size = NewMediumFontSize;
        ChangeFontSizes(GS, GS->Renderer.FontSizes);
    }
    
    ButtonTestMouseInteraction(&GS->Renderer, &GS->Input, FontSettings->EditFont);
    
    // Process text field
    if(FontSettings->EditingFont)
    {
        text_field_flag_result TFResult = ProcessTextField(&GS->Renderer, GS->Time.dTime, &GS->Input, &FontSettings->ActiveFont);
        if(TFResult.Flag & processTextField_TextChanged)
        {
            i32 FailedPos = 0;
            b32 PathIsValid = CheckPathValidity(&GS->ScratchArena, FontSettings->ActiveFont.TextString, &FailedPos);
            
            color_palette *Palette = &GS->MusicInfo.DisplayInfo.ColorPalette;
            if(PathIsValid || FontSettings->ActiveFont.TextString.Pos == 0) 
                SetColor(&FontSettings->ActiveFont.Text, &Palette->ForegroundText);
            else            
            {
                SetColor(&FontSettings->ActiveFont.Text, &Palette->ErrorText, FailedPos);
            }
        }
        if(TFResult.Flag & processTextField_Confirmed)
        {
            OnToggleEditFont(FontSettings);
        }
    }
}


// Color picker *********************************************

internal void
CreateColorPickerPaletteList(game_state *GS, color_picker *ColorPicker, v2 Offset)
{
    color_palette *Palette = &GS->MusicInfo.DisplayInfo.ColorPalette;
    
    r32         FontSize = GetFontSize(&GS->Renderer, font_Small).Size;
    font_metrics Metrics = GetFontMetrics(GS, font_Small, GlobalPaletteColorNames[0]);
    r32         ColorGap = GS->Layout.ColorPickerPaletteColorsGap;
    r32        YDownMove = Metrics.RowGap + ColorGap;
    
    v2 ColorSquareSize = V2(FontSize);
    v2 ColorSquareOutlineSize = ColorSquareSize + V2(GS->Layout.ColorPickerColorOutline)*2;
    v2 StartP = V2(GS->Layout.ColorPickerColorFieldSide)*0.5f; 
    StartP += V2(ColorSquareOutlineSize.x + ColorGap, -ColorSquareOutlineSize.y)*0.5f;
    StartP += Offset + V2(0, GS->Layout.ColorPickerOffsetY);
    
    r32 YDown = 0;
    r32 DepthAdvance = 0.00001f;
    r32 Depth = GS->Layout.SettingsWindowDepth - DepthAdvance;
    r32 MaxTextWidth = 0;
    For(PALETTE_COLOR_AMOUNT)
    {
        palette_color *C = ColorPicker->PaletteColors+It; 
        
        if(C->Outline == NULL)
        {
            C->Outline = CreateRenderRect(&GS->Renderer, ColorSquareOutlineSize, Depth, 
                                          &Palette->ForegroundText, ColorPicker->Background);
            SetScissor(C->Outline, ColorPicker->Background);
        }
        else SetSize(C->Outline, ColorSquareOutlineSize);
        SetLocalPosition(C->Outline, StartP + V2(0, YDown));
        
        if(C->Preview == NULL)
        {
            C->Preview = CreateRenderRect(&GS->Renderer, ColorSquareSize, Depth-DepthAdvance, Palette->Colors+It, C->Outline);
            SetScissor(C->Preview, ColorPicker->Background);
        }
        else SetSize(C->Preview, ColorSquareSize);
        
        b32 Render = true;
        if(C->Name.Base != NULL) Render = C->Name.Base->ID->Render;
        RemoveRenderText(&GS->Renderer, &C->Name);
        RenderText(GS, font_Small, GlobalPaletteColorNames+It, &Palette->ForegroundText, &C->Name, GS->Layout.SettingsWindowDepth-0.001f, C->Preview, V2(ColorSquareOutlineSize.x*0.5f + ColorGap, ColorSquareSize.y*0.5f-Metrics.Ascent));
        SetScissor(&C->Name, ColorPicker->Background);
        SetActive(&C->Name, Render);
        
        if(MaxTextWidth < C->Name.Extends.x) MaxTextWidth = C->Name.Extends.x;
        
        YDown -= YDownMove;
        Depth -= DepthAdvance*2;
    }
    
    // Create active color bg
    v2 ActiveColorSize = V2(MaxTextWidth + ColorSquareOutlineSize.x + ColorGap*3, ColorSquareOutlineSize.y);
    if(ColorPicker->ActiveColorBG == NULL)
    {
        ColorPicker->ActiveColorBG = CreateRenderRect(&GS->Renderer, ActiveColorSize, Depth+0.001f, 
                                                      &Palette->SliderGrabThing, ColorPicker->PaletteColors[0].Outline);
        SetScissor(ColorPicker->ActiveColorBG, ColorPicker->Background);
    }
    else SetSize(ColorPicker->ActiveColorBG, ActiveColorSize);
    SetLocalPosition(ColorPicker->ActiveColorBG, V2(ActiveColorSize.x*0.5f - ColorSquareOutlineSize.x*0.5f - ColorGap, 0));
    
}

inline r32
GetColorPickerTextWidth(game_state *GS)
{
    string_c *LongestName = GlobalPaletteColorNames+5; // 'Slider Background'
    render_text Text = {};
    RenderText(GS, font_Small, LongestName, NULL, &Text, 0, NULL, {});
    RemoveRenderText(&GS->Renderer, &Text);
    
    r32 FontSize = GetFontSize(&GS->Renderer, font_Small).Size;
    r32 ColorSquareWidth = FontSize + GS->Layout.ColorPickerColorOutline*2;
    r32 Result = Text.Extends.x + ColorSquareWidth + GS->Layout.ColorPickerPaletteColorsGap*3;
    return Result;
}

internal void
CreateColorPicker(game_state *GS, color_picker *Result, v2 ColorFieldSize, r32 SpectrumWidth, entry_id *Parent)
{
    renderer *Renderer = &GS->Renderer;
    color_palette *Palette = &GS->MusicInfo.DisplayInfo.ColorPalette;
    layout_definition *Layout = &GS->Layout;
    r32 Depth = Layout->SettingsWindowDepth - 0.000001f;
    r32 SmallFontHeight  = GetFontSize(Renderer, font_Small).Size;
    r32 MediumFontHeight = GetFontSize(Renderer, font_Medium).Size;
    
    Result->Parent = Parent;
    Result->Bitmap.ColorFormat = colorFormat_RGBA;
    Result->Bitmap.Width  = (u32)ColorFieldSize.x;
    Result->Bitmap.Height = (u32)ColorFieldSize.y;
    Result->Bitmap.WasLoaded = true;
    
    r32 TextWidth         = GetColorPickerTextWidth(GS);
    v2  BGSize            = V2(ColorFieldSize.x + SpectrumWidth*2 + TextWidth, Layout->SettingsWindowHeight);
    r32 ContentOffX       = SpectrumWidth - TextWidth*0.5f;
    v2  ContentOffset     = V2(ContentOffX, Layout->ColorPickerContentOffsetY);
    v2i ColorSpectrumSize = V2i((i32)SpectrumWidth, (i32)ColorFieldSize.y);
    
    Result->Background = CreateRenderRect(&GS->Renderer, BGSize, Depth+0.001f, &Palette->Foreground, Parent);
    Parent = Result->Background;
    
    Result->FixedSizeHolder = CreateRenderRect(&GS->Renderer, {}, Depth+0.001f, &Palette->ErrorText, Parent);
    SetLocalPosition(Result->FixedSizeHolder, V2(0, GS->Layout.ColorPickerOffsetY));
    Parent = Result->FixedSizeHolder;
    
    // Create palette colors
    CreateColorPickerPaletteList(&GlobalGameState, Result, ContentOffset);
    
    // Create brightness slider
    loaded_bitmap ColorSpectrumBitmap = {};
    ColorSpectrumBitmap.Pixels = AllocateArray(&GS->FixArena, ColorSpectrumSize.x*ColorSpectrumSize.y, u32);
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
    
    CreateSlider(GS, &Result->ColorSpectrum, sliderAxis_Y, V2(ColorSpectrumSize), 
                 V2((r32)ColorSpectrumSize.x+8.0f, 8.0f), Depth, ColorSpectrumBitmap, &Palette->SliderGrabThing, Parent);
    SetLocalPosition(&Result->ColorSpectrum, ContentOffset + -V2(ColorFieldSize.x*0.5f + ColorSpectrumSize.x, 0));
    
    // Create color texture
    v3 NewColor = CalculateColorFromSpectrum(Result->ColorSpectrum.SlidePercentage);
    UpdateColorPickerTexture(Result, NewColor);
    Result->TextureEntry = CreateRenderBitmap(Renderer, ColorFieldSize, Depth, Parent, Result->GLID);
    SetLocalPosition(Result->TextureEntry, ContentOffset);
    
    // Create Pick dot
    Result->PickDot = CreateRenderRect(Renderer, V2(8,8), Depth-0.001f, &Palette->Text, Parent);
    Result->InnerInnerDot = CreateRenderRect(Renderer, V2(6,6), Depth-0.0011f, &Palette->Slot, Result->PickDot);
    Result->InnerDot = CreateRenderRect(Renderer, V2(4,4), Depth-0.0012f, &Palette->Text, Result->PickDot);
    
    // Create Buttons
#if RESOURCE_PNG
    u32 NewID    = DecodeAndCreateGLTexture(Plus_Icon_DataCount, (u8 *)Plus_Icon_Data);
    u32 RemoveID = DecodeAndCreateGLTexture(Minus_Icon_DataCount, (u8 *)Minus_Icon_Data);
    u32 SaveID   = DecodeAndCreateGLTexture(Save_Icon_DataCount, (u8 *)Save_Icon_Data);
#else
    u32 NewID    = DecodeAndCreateGLTexture(&GS->ScratchArena, NewBitmapData(Add_Medium_Icon, colorFormat_RGBA));
    u32 RemoveID = DecodeAndCreateGLTexture(&GS->ScratchArena, NewBitmapData(Minus_Icon, colorFormat_RGBA));
    u32 SaveID   = DecodeAndCreateGLTexture(&GS->ScratchArena, NewBitmapData(Save_Icon, colorFormat_RGBA));
#endif
    
    r32 BtnExtends = Layout->SmallButtonExtents;
    rect BtnRect = {{-BtnExtends,-BtnExtends},{BtnExtends,BtnExtends}};
    Result->New    = NewButton(Renderer, BtnRect, Depth-0.003f, false, Renderer->ButtonBaseID, NewID, 
                               Renderer->ButtonColors, Parent);
    Result->Remove = NewButton(Renderer, BtnRect, Depth-0.003f, false, Renderer->ButtonBaseID, RemoveID, 
                               Renderer->ButtonColors, Parent);
    Result->Save   = NewButton(Renderer, BtnRect, Depth-0.003f, false, Renderer->ButtonBaseID, SaveID, 
                               Renderer->ButtonColors, Parent);
    
    // Extends*2 = Size, Size*4 = Btn width total, Div 5 = gap amount for 4 btns.
    r32 ButtonGap = Layout->ColorPickerButtonGapY; 
    v2  BtnBaseOffset = ContentOffset -ColorFieldSize*0.5f + V2(BtnExtends, -BtnExtends-ButtonGap);
    Translate(Result->New,    BtnBaseOffset + V2(0, -(BtnExtends*2+ButtonGap)*0));
    Translate(Result->Remove, BtnBaseOffset + V2(0, -(BtnExtends*2+ButtonGap)*1));
    Translate(Result->Save,   BtnBaseOffset + V2(0, -(BtnExtends*2+ButtonGap)*2));
    
    Result->New->OnPressed    = {OnAnimationDone, &Result->NewAnim.Activated};
    Result->Remove->OnPressed = {OnAnimationDone, &Result->RemoveAnim.Activated};
    Result->Save->OnPressed   = {OnAnimationDone, &Result->SaveAnim.Activated};
    // Cancel will be set in CreateStyleSettings.
    
    string_c NewText    = NewStaticStringCompound("New Color Palette.");
    string_c SaveText   = NewStaticStringCompound("Saving current Color Palette.");
    string_c RemoveText = NewStaticStringCompound("Remove this Color Palette?");
    
    CreateQuitAnimation(&Result->NewAnim, ColorFieldSize, &NewText, 0.5f);
    CreateQuitAnimation(&Result->SaveAnim, ColorFieldSize, &SaveText, 0.75f);
    CreateQuitAnimation(&Result->RemoveAnim, ColorFieldSize, &RemoveText, 1.2f);
    
    // Create Textfield for palette names ****************
    Result->PaletteName = CreateTextField(Renderer, &GS->FixArena, V2(Layout->PaletteNameFieldX, MediumFontHeight), 
                                          Depth+0.0010001f, (u8 *)"Custom Palette", Parent, 
                                          &Palette->ForegroundText, &Palette->Foreground, font_Medium);
    r32 FieldX = Layout->PaletteNameFieldX*0.5f - ColorFieldSize.x*0.5f - Layout->PaletteNameFieldIndent;
    Translate(&Result->PaletteName, ContentOffset + V2(FieldX, ColorFieldSize.y*0.5f + MediumFontHeight*0.5f));
    SetActive(&Result->PaletteName, true);
    AppendStringCompoundToCompound(&Result->PaletteName.TextString, GetCurrentPaletteName());
    UpdateTextField(Renderer, &Result->PaletteName);
    SetScissor(&Result->PaletteName, Result->Background);
    
    Result->CurrentColorPaletteID = MAX_UINT32;
    ColorToPickerPosition(Result, GetColor(Result->PaletteColors[0].Preview));
    SetActive(Result, true);
}

inline void
SetActive(color_picker *ColorPicker, b32 Activate)
{
    SetActive(ColorPicker->TextureEntry, Activate);
    SetActive(&ColorPicker->ColorSpectrum, Activate);
    SetActive(&ColorPicker->RText, Activate);
    SetActive(&ColorPicker->GText, Activate);
    SetActive(&ColorPicker->BText, Activate);
    SetActive(&ColorPicker->HText, Activate);
    SetActive(&ColorPicker->SText, Activate);
    SetActive(&ColorPicker->VText, Activate);
    SetActive(&ColorPicker->HexText, Activate);
    SetActive(ColorPicker->PickDot, Activate);
    SetActiveAllButGiven(&GlobalGameState.DragableList, ColorPicker->ColorSpectrum.Background, !Activate);
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
    SetActive(&ColorPicker->PaletteName, Activate);
    if(Activate) SetActive(ColorPicker->PaletteName.Cursor, false);
    SetActive(ColorPicker->Background, Activate);
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
CreateRenderTextFromColor(color_picker *ColorPicker, v3 NewColor, v3 NewHSVColor)
{
    renderer *Renderer = &GlobalGameState.Renderer;
    color_palette *Palette = &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette;
    layout_definition *Layout = &GlobalGameState.Layout;
    r32 DepthOff = 0.00001f;
    r32 Depth = Layout->SettingsWindowDepth;
    r32 Descent = 0;
    i32 Red, Green, Blue;
    
    { // Set the RBG colors text 
        Red   = (i32)(255*NewColor.r);
        Green = (i32)(255*NewColor.g);
        Blue  = (i32)(255*NewColor.b);
        
        NewLocalString(RString, 10, "R: ");
        NewLocalString(GString, 10, "G: ");
        NewLocalString(BString, 10, "B: ");
        I32ToString(&RString, Red);
        I32ToString(&GString, Green);
        I32ToString(&BString, Blue);
        
        RemoveRenderText(Renderer, &ColorPicker->RText);
        RemoveRenderText(Renderer, &ColorPicker->GText);
        RemoveRenderText(Renderer, &ColorPicker->BText);
        
        RenderText(&GlobalGameState, font_Small, &RString, &Palette->ForegroundText, &ColorPicker->RText, Depth + DepthOff, ColorPicker->New->Entry);
        RenderText(&GlobalGameState, font_Small, &GString, &Palette->ForegroundText, &ColorPicker->GText, Depth + DepthOff*2, ColorPicker->Remove->Entry);
        RenderText(&GlobalGameState, font_Small, &BString, &Palette->ForegroundText, &ColorPicker->BText, Depth + DepthOff*3, ColorPicker->Save->Entry);
        
        Descent = GetFontDescent(&GlobalGameState, font_Small, RString);
        v2 Offset = V2(Layout->SmallButtonExtents + Layout->ColorPickerButtonGapY, -Layout->SmallButtonExtents - Descent);
        SetLocalPosition(&ColorPicker->RText, Offset);
        SetLocalPosition(&ColorPicker->GText, Offset);
        SetLocalPosition(&ColorPicker->BText, Offset);
    }
    
    { // Now set the HSV colors text
        i32 Hue        = (i32)NewHSVColor.r;
        i32 Saturation = (i32)(100*NewHSVColor.g);
        i32 Value      = (i32)(100*NewHSVColor.b);
        
        NewLocalString(HString, 10, "H: ");
        NewLocalString(SString, 10, "S: ");
        NewLocalString(VString, 10, "V: ");
        I32ToString(&HString, Hue);
        I32ToString(&SString, Saturation);
        I32ToString(&VString, Value);
        
        RemoveRenderText(Renderer, &ColorPicker->HText);
        RemoveRenderText(Renderer, &ColorPicker->SText);
        RemoveRenderText(Renderer, &ColorPicker->VText);
        
        RenderText(&GlobalGameState, font_Small, &HString, &Palette->ForegroundText, &ColorPicker->HText, Depth + DepthOff*4, ColorPicker->New->Entry);
        RenderText(&GlobalGameState, font_Small, &SString, &Palette->ForegroundText, &ColorPicker->SText, Depth + DepthOff*5, ColorPicker->Remove->Entry);
        RenderText(&GlobalGameState, font_Small, &VString, &Palette->ForegroundText, &ColorPicker->VText, Depth + DepthOff*6, ColorPicker->Save->Entry);
        
        v2 Offset = V2(Layout->SmallButtonExtents + Layout->ColorPickerHSVTextOffset, -Layout->SmallButtonExtents - Descent);
        SetLocalPosition(&ColorPicker->HText, Offset);
        SetLocalPosition(&ColorPicker->SText, Offset);
        SetLocalPosition(&ColorPicker->VText, Offset);
    }
    
    { // New set the Hex color text
        u32 Hex = Red << 16 | Green << 8 | Blue;
        NewLocalString(HexString, 15, "#: ");
        DecimalToHexString(Hex, &HexString);
        
        RemoveRenderText(Renderer, &ColorPicker->HexText);
        RenderText(&GlobalGameState, font_Small, &HexString, &Palette->ForegroundText, &ColorPicker->HexText, Depth + DepthOff*7, ColorPicker->Save->Entry);
        
        v2 Offset = V2(Layout->SmallButtonExtents + Layout->ColorPickerButtonGapY - 5, 
                       -(Layout->SmallButtonExtents - Descent)*2);
        SetLocalPosition(&ColorPicker->HexText, Offset);
    }
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
    
    CreateRenderTextFromColor(ColorPicker, Color, ColorHSV);
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
    
    CreateRenderTextFromColor(ColorPicker, NewColor, RGBToHSV(NewColor));
}

inline void
OnNewPalette(color_picker *ColorPicker)
{
    music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
    
    if(ColorPicker->NewAnim.Activated)
    {
        AddCustomColorPalette(&DisplayInfo->ColorPalette, &ColorPicker->PaletteName.TextString);
        
        // Move to the now saved palette!
        DisplayInfo->ColorPaletteID = DEFAULT_COLOR_PALETTE_COUNT+GlobalGameState.Settings.PaletteCount-1;
        UpdateColorPalette(DisplayInfo, false);
    }
}

inline void
OnRemovePalette(color_picker *ColorPicker)
{
    quit_animation *Anim = &ColorPicker->RemoveAnim;
    if(Anim->Activated)
    {
        music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
        RemoveCustomColorPalette(DisplayInfo->ColorPaletteID);
        UpdateColorPalette(DisplayInfo, false);
    }
}

inline void
OnSavePalette(color_picker *ColorPicker)
{
    if(ColorPicker->SaveAnim.Activated)
    {
        serialization_settings *Settings = &GlobalGameState.Settings;
        music_display_info *DisplayInfo = &GlobalGameState.MusicInfo.DisplayInfo;
        u32 CustomID = DisplayInfo->ColorPaletteID-DEFAULT_COLOR_PALETTE_COUNT;
        Assert(CustomID < Settings->PaletteCount);
        
        if(ColorPicker->PaletteName.TextString.Pos >= COLOR_PALETTE_MAX_NAME_LENGTH) 
            ColorPicker->PaletteName.TextString.Pos = COLOR_PALETTE_MAX_NAME_LENGTH;
        ResetStringCompound(Settings->PaletteNames[CustomID]);
        AppendStringCompoundToCompound(Settings->PaletteNames+CustomID, &ColorPicker->PaletteName.TextString);
        For(PALETTE_COLOR_AMOUNT)
        {
            Settings->Palettes[CustomID].Colors[It] = DisplayInfo->ColorPalette.Colors[It]*255.0f;
        }
    }
}

inline void
OnAnimationDone(void *Data)
{
    b32 *Activated = (b32 *)Data;
    *Activated = false;
}

internal void
HandleColorPickerButtonAnimation(color_picker *ColorPicker, button *Btn, color_picker_anim_btn AnimBtn, quit_animation *Anim)
{
    if(!Anim->Activated)
    {
        r32 Pushdown = 25;
        if(Btn->State == buttonState_Pressed)
        {
            v2 Position = GetPosition(ColorPicker->Parent)+V2(0, GetExtends(ColorPicker->Parent).y-Pushdown);
            v2 Size = GetSize(ColorPicker->Parent)-V2(0, Pushdown);
            if(QuitAnimation(Anim, 1, Position, Size))
            {
                if(AnimBtn == colorPickerAnimBtn_New)         OnNewPalette(ColorPicker);
                else if(AnimBtn == colorPickerAnimBtn_Save)   OnSavePalette(ColorPicker);
                else if(AnimBtn == colorPickerAnimBtn_Remove) OnRemovePalette(ColorPicker);
                
                SetActive(Anim, false);
            }
        }
        else if(Anim->dAnim != 0)
        {
            v2 Position = GetPosition(ColorPicker->Parent)+V2(0, GetExtends(ColorPicker->Parent).y-Pushdown);
            v2 Size = GetSize(ColorPicker->Parent)-V2(0, Pushdown);
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

internal void
HandleActiveColorPicker(game_state *GS, color_picker *ColorPicker)
{
    input_info *Input = &GS->Input;
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
                    SetColor(ColorPicker->InnerDot, &GS->MusicInfo.DisplayInfo.ColorPalette.Colors[It]);
                    break;
                }
            }
        }
        ColorPicker->IsPickingColor = false;
    }
    else if(Input->Pressed[KEY_LMB])
    {
        if(ColorPicker->IsPickingColor)
        {
            v2 MouseP = ClampToRect(Input->MouseP, ColorPicker->TextureEntry);
            SetPosition(ColorPicker->PickDot, MouseP);
            UpdateColorPickerSelectedColor(ColorPicker);
        }
    }
    
    // Process button presses
    ButtonTestMouseInteraction(&GS->Renderer, Input, ColorPicker->New);
    ButtonTestMouseInteraction(&GS->Renderer, Input, ColorPicker->Remove);
    ButtonTestMouseInteraction(&GS->Renderer, Input, ColorPicker->Save);
    
    // Update stuff on palette change
    if(ColorPicker->CurrentColorPaletteID != GS->MusicInfo.DisplayInfo.ColorPaletteID)
    {
        ColorPicker->CurrentColorPaletteID = GS->MusicInfo.DisplayInfo.ColorPaletteID;
        
        // Update Textfield text
        ResetStringCompound(ColorPicker->PaletteName.TextString);
        AppendStringCompoundToCompound(&ColorPicker->PaletteName.TextString, GetCurrentPaletteName());
        UpdateTextField(&GS->Renderer, &ColorPicker->PaletteName);
        
        // Change buttons colors if deactivated
        if(IsColorPaletteDefault()) 
        {
            SetDisabled(ColorPicker->Save,   true, &GS->MusicInfo.DisplayInfo.ColorPalette.ErrorText);
            SetDisabled(ColorPicker->Remove, true, &GS->MusicInfo.DisplayInfo.ColorPalette.ErrorText);
            
            SetActive(ColorPicker->PaletteName.Cursor, false);
        }
        else 
        {
            SetDisabled(ColorPicker->Save,   false, GS->Renderer.ButtonColors.IconColor);
            SetDisabled(ColorPicker->Remove, false, GS->Renderer.ButtonColors.IconColor);
        }
        ColorToPickerPosition(ColorPicker, GetColor(ColorPicker->PaletteColors[0].Preview));
        SetColor(ColorPicker->InnerDot, &GS->MusicInfo.DisplayInfo.ColorPalette.Colors[0]);
    }
    
    // process text field
    if(!IsColorPaletteDefault() && !GS->StyleSettings.FontSettings.EditingFont)
    {
        text_field_flag_result TFResult = ProcessTextField(&GS->Renderer, GS->Time.dTime, 
                                                           Input, &ColorPicker->PaletteName);
        if(TFResult.Flag & processTextField_TextChanged)
        {
        }
        if(TFResult.Flag & processTextField_Confirmed)
        {
        }
    }
    
    // Handle Remove button animation
    HandleColorPickerButtonAnimation(ColorPicker, ColorPicker->New, colorPickerAnimBtn_New, &ColorPicker->NewAnim);
    HandleColorPickerButtonAnimation(ColorPicker, ColorPicker->Save, colorPickerAnimBtn_Save, &ColorPicker->SaveAnim);
    HandleColorPickerButtonAnimation(ColorPicker, ColorPicker->Remove, colorPickerAnimBtn_Remove, &ColorPicker->RemoveAnim);
}

inline color_palette *
GetColorPalette(game_state *GS)
{
    return &GS->MusicInfo.DisplayInfo.ColorPalette;
}

inline void
CreateLushGreenColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(11,  15,  11);
    Palette->Text             = Color(144, 174, 130);
    Palette->Selected         = Color(18,  45,  18);
    Palette->Foreground       = Color(14,  18,  12);
    Palette->ForegroundText   = Color(144, 174, 130);
    Palette->SliderBackground = Color(17,  24,  17);
    Palette->SliderGrabThing  = Color(38,  52,  34);
    Palette->ButtonActive     = Color(22,  51,  16);
    Palette->PlayingSong      = Color(24,  40,  24);
    Palette->ErrorText        = Color(170, 11,  22);
}

inline void
CreateEvilColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(71,  25,  25);
    Palette->Text             = Color(217, 199, 197);
    Palette->Selected         = Color(106, 34,  34);
    Palette->Foreground       = Color(106, 34,  34);
    Palette->ForegroundText   = Color(217, 199, 197);
    Palette->SliderBackground = Color(56,  23,  23);
    Palette->SliderGrabThing  = Color(86,  26,  23);
    Palette->ButtonActive     = Color(58,  12,  12);
    Palette->PlayingSong      = Color(56,  23,  23);
    Palette->ErrorText        = Color(170, 11,  22);
}

inline void
CreateAquaColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(17,  26,  44);
    Palette->Text             = Color(150, 187, 197);
    Palette->Selected         = Color(27,  47,  89);
    Palette->Foreground       = Color(23,  53,  102);
    Palette->ForegroundText   = Color(150, 187, 197);
    Palette->SliderBackground = Color(20,  36,  79);
    Palette->SliderGrabThing  = Color(11,  29,  58);
    Palette->ButtonActive     = Color(19,  52,  102);
    Palette->PlayingSong      = Color(20,  37,  79);
    Palette->ErrorText        = Color(170, 11,  22);
}

inline void
CreateSunriseColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(201, 103, 29);
    Palette->Text             = Color(255, 239, 149);
    Palette->Selected         = Color(171, 83,  32);
    Palette->Foreground       = Color(220, 163, 45);
    Palette->ForegroundText   = Color(135, 39,  4);
    Palette->SliderBackground = Color(249, 212, 94);
    Palette->SliderGrabThing  = Color(218, 136, 42);
    Palette->ButtonActive     = Color(253, 128, 41);
    Palette->PlayingSong      = Color(206, 122, 34);
    Palette->ErrorText        = Color(170, 11,  22);
}

inline void
CreateMonochromeColorPalette(color_palette *Palette)
{
    Palette->Slot             = Color(44,  44,  44);
    Palette->Text             = Color(173, 173, 173);
    Palette->Selected         = Color(79,  79,  79);
    Palette->Foreground       = Color(27,  27,  27);
    Palette->ForegroundText   = Color(173, 173, 173);
    Palette->SliderBackground = Color(88,  88,  88);
    Palette->SliderGrabThing  = Color(39,  39,  39);
    Palette->ButtonActive     = Color(88,  88,  88);
    Palette->PlayingSong      = Color(22,  22,  22);
    Palette->ErrorText        = Color(170, 11,  22);
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
        case 0: CreateLushGreenColorPalette(&DisplayInfo->ColorPalette); break;
        case 1: CreateEvilColorPalette(&DisplayInfo->ColorPalette);  break;
        case 2: CreateAquaColorPalette(&DisplayInfo->ColorPalette); break;
        case 3: CreateSunriseColorPalette(&DisplayInfo->ColorPalette); break;
        case 4: CreateMonochromeColorPalette(&DisplayInfo->ColorPalette); break;
        case 5: CreateMonoInvertedColorPalette(&DisplayInfo->ColorPalette); break;
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
    serialization_settings *Settings = &GlobalGameState.Settings;
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
    serialization_settings *Settings = &GlobalGameState.Settings;
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



