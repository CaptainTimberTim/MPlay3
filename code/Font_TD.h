/* date = March 20th 2021 7:44 am */
#ifndef _FONT__T_D_H
#define _FONT__T_D_H

enum font_size
{
    font_Small  = 0,
    font_Medium = 1,
    font_Big    = 2,
};

#define ATLAS_LETTER_COUNT 255

struct font_size_array
{
    r32 *Sizes;
    u32 Count;
};

struct font_atlas
{
    r32 Size;
    stbtt_packedchar *CharData;
};

struct fonts
{
    u32 GLID;
    u32 BitmapWidth;
    u32 BitmapHeight;
    i32 HeightOffset;
    
    u32 Count;
    font_atlas *Font;
};


internal fonts LoadFonts(arena_allocator *Arena, font_size_array FontSizes, u8 *RawFontData);
internal void RenderText(struct renderer *Renderer, arena_allocator *Arena, font_size Size, string_c *Text, 
                         v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent = 0, v2 StartP = {});
internal void RenderText(struct renderer *Renderer, arena_allocator *Arena, font_atlas Font, string_c *Text, 
                         v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent = 0, v2 StartP = {});
inline r32 FontSizeToPixel(font_size FontSize);


#endif //_FONT__T_D_H
