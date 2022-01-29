/* date = September 10th 2021 9:06 am */
#ifndef _SOUND_SETTINGS_H
#define _SOUND_SETTINGS_H

struct palette_color
{
    entry_id *Outline;
    entry_id *Preview;
    render_text Name;
};

struct color_picker
{
    entry_id *Parent;
    entry_id *Background;
    entry_id *FixedSizeHolder;
    
    loaded_bitmap Bitmap;
    u32 GLID; 
    entry_id *TextureEntry;
    b32 IsPickingColor;
    
    slider ColorSpectrum;
    render_text RText;
    render_text GText;
    render_text BText;
    
    render_text HText;
    render_text SText;
    render_text VText;
    
    render_text HexText;
    
    entry_id *PickDot;
    entry_id *InnerDot;
    entry_id *InnerInnerDot;
    v3 SelectedColor;
    
    palette_color PaletteColors[PALETTE_COLOR_AMOUNT];
    u32 ActiveColor;
    entry_id *ActiveColorBG;
    u32 CurrentColorPaletteID;
    text_field PaletteName;
    
    button *New;
    button *Save;
    button *Remove;
    quit_animation NewAnim;
    quit_animation SaveAnim;
    quit_animation RemoveAnim;
};

enum color_picker_anim_btn
{
    colorPickerAnimBtn_New,
    colorPickerAnimBtn_Save,
    colorPickerAnimBtn_Remove
};

struct font_settings
{
    entry_id *Parent;
    entry_id *Background;
    
    button *EditFont;
    b32 EditingFont;
    text_field ActiveFont;
    
    render_text SmallFontText;
    slider      SmallFontSlider;
    
    render_text MediumFontText;
    slider      MediumFontSlider;
    
    //slider LargeFontSlider;
};

struct style_settings_window
{
    b32 IsActive;
    
    b32 IsMoving;
    v2 MoveOffset; // For mouse drag.
    entry_id *Border;
    entry_id *Background; 
    v2 _BGOffset;
    
    button *Cancel;
    
    entry_id *ColorPickerParent;
    color_picker ColorPicker;
    
    entry_id *FontParent;
    font_settings FontSettings;
};

internal void CreateStyleSettings(game_state *GS, style_settings_window *StyleSettings);
internal void HandleActiveStyleSettings(game_state *GS, style_settings_window *StyleSettings, input_info *Input);
inline   void SetActive(style_settings_window *StyleWindow, b32 Activate);
inline   void OnStyleSettings(void *Data);
internal void UpdateSettings(game_state *GS);

// Font settings
internal void CreateFontSettings(game_state *GS, font_settings *FontSettings, entry_id *Parent);
inline   void SetActive(font_settings *FontSettings, b32 Activate);
internal void HandleActiveFontSettings(game_state *GS, font_settings *FontSettings);

// Color Picker
internal void CreateColorPicker(game_state *GS, color_picker *Result, v2 BitmapSize, r32 SpectrumWidth, entry_id *Parent);
internal void HandleActiveColorPicker(game_state *GS, color_picker *ColorPicker);
inline   void SetActive(color_picker *ColorPicker, b32 Activate);
internal void CreateColorPickerPaletteList(game_state *GS, color_picker *ColorPicker, v2 Offset);
inline    r32 GetColorPickerTextWidth(game_state *GS);
//inline void CreateBasicColorPalette(color_palette *Palette);
inline void CreateLushGreenColorPalette(color_palette *Palette);
inline void OnPaletteSwap(void *Data);
inline void UpdateColorPalette(music_display_info *DisplayInfo, b32 GoToNextPalette);
inline string_c * GetCurrentPaletteName();
internal void AddCustomColorPalette(color_palette *ColorPalette, string_c *Name);
internal void RemoveCustomColorPalette(u32 PaletteID);
inline b32 IsColorPaletteDefault();

#endif //_SOUND_SETTINGS_H
