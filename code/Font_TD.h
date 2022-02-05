/* date = March 20th 2021 7:44 am */
#ifndef _FONT__T_D_H
#define _FONT__T_D_H

enum font_size_id // #Helper
{
    font_Small  = 0,
    font_Medium = 1,
    font_Big    = 2,
    font_size_id_Size // Needs to be last _always_!
};
struct font_size
{
    font_size_id ID;
    r32 Size;
};
struct font_sizes
{
    font_size Sizes[font_size_id_Size]; // font_size_id should be able to access the font_size with its own ID!
    u32 Count;
};

struct font_name_list
{
    u32 MaxCount;
    u32 Count;
    string_c *Names;
};

struct font_data
{
    r32 Size;
    stbtt_packedchar *CharData;
};

struct font_metrics
{
    // Everything in unscaled pixels. 
    // Multiply by font scale factor = (FontHeight/(Ascent + Descent*-1).
    r32 Ascent;  // From baseline.
    r32 Descent; // From baseline. Negative.
    r32 LineGap; // Spacing between lines (from descent to ascent).
    r32 RowGap;  // Spacing between lines (from baseline to baseline.
};

struct codepoint_range
{
    u32 First;
    u32 Last;
    u32 Count;
};

#define MAX_CODEPOINT_BLOCK 500
// The amount of codepoints loaded, when the containing 
// Unicode Group it is bigger than MAX_CODEPOINT_BLOCK.
#define USED_CODEPOINT_RANGE 1

// TODO:: Maybe cache the fonts, which where found in the 
// Settings file. This will reduce the large overhead of
// searching _all_ font, every time. 
#define SEARCH_FOR_MISSING_FONT
struct font_group
{
    u32 GLID;
    u32 BitmapWidth;
    u32 BitmapHeight;
    struct unicode_group *UnicodeGroup;
    codepoint_range CodepointRange;
    font_metrics FontMetrics;
    
    font_data FontDataForEachSize[font_size_id_Size];
};

struct font_atlas
{
    font_sizes FontSizes;
    
    u32 MaxCount;
    u32 Count;
    font_group *FontGroups; // Sparse array of required codepoints
    
    // These are the fonts, which are not in the unicode-group-list.
    font_name_list *CachedFontNames; // Points to Settings->CachedFontNames.
};

inline font_atlas NewFontAtlas(struct serialization_settings *Settings, font_sizes FontSizes);
internal void LoadFonts(arena_allocator *FixArena, arena_allocator *ScratchArena, font_atlas *Atlas, 
                        u8 *RawFontData, /*string_c FontPath, */u32 *CodepointsFromGroup, u32 CodepointCount);
internal void RenderText(struct game_state *GS, font_size_id FontSizeID, string_c *Text, 
                         v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent = 0, v2 StartP = {});
internal u8 GetUTF8Decimal(u8 *S, u32 *Utf8Value);
internal font_group *GetFontGroup(game_state *GS, font_atlas *Atlas, u32 Codepoint);
inline font_size GetFontSize(struct renderer *Renderer, font_size_id ID);

// These are pre-scaled for the given size.
inline font_metrics GetFontMetrics(game_state *GS, font_size_id ID, string_c Text);
inline r32          GetFontDescent(game_state *GS, font_size_id ID, string_c Text);
inline r32          GetFontAscent (game_state *GS, font_size_id ID, string_c Text);

struct raw_font
{
    // In
    u32 Codepoint;
    string_c *FolderPath;
    
    // Out
    string_c Name;
    read_file_result Data; // In Scratch Memory!
};
internal b32 FindAndLoadFontWithUnicodeCodepoint(arena_allocator *ScratchArena, raw_font *SearchFont,  string_c *FoundFontPath_out);


#include "UnicodeGroups_TD.h"

#endif //_FONT__T_D_H
