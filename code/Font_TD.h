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
    
    font_data *FontSizes; // Array
    u32 Count;
};

struct font_atlas
{
    font_sizes FontSizes;
    
    i32 HeightOffset;
    
    u32 MaxCount;
    u32 Count;
    font_group *FontGroups; // Sparse array of required codepoints
    
    font_name_list *UsedFontNames;
};

inline font_atlas NewFontAtlas(struct settings *Settings, font_sizes FontSizes);
internal void LoadFonts(arena_allocator *FixArena, arena_allocator *ScratchArena, font_atlas *Atlas, 
                        u8 *RawFontData, u32 *CodepointsFromGroup, u32 CodepointCount);
internal void RenderText(struct game_state *GS, arena_allocator *Arena, font_size_id FontSize, string_c *Text, 
                         v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent = 0, v2 StartP = {});
internal u8 GetUTF8Decimal(u8 *S, u32 *Utf8Value);
internal font_group *GetFontGroup(game_state *GS, font_atlas *Atlas, u32 Codepoint);
inline font_size GetFontSize(struct renderer *Renderer, font_size_id ID);

struct raw_font
{
    // In
    u32 Codepoint;
    string_c *FolderPath;
    
    // Out
    string_c Name;
    read_file_result Data; // In Scratch Memory!
};
internal b32 FindAndLoadFontWithUnicodeCodepoint(arena_allocator *ScratchArena, raw_font *SearchFont);


#include "UnicodeGroups_TD.h"

#if 0
#define NewSSC NewStaticStringCompound
global_variable unicode_group UnicodeGroups[] = {
    /*Name,                                              First, Last, Length,       Font  */
    { NewSSC("Basic Latin"),                            { 0x0,   0x7F, 128 },        NewSSC("arial.ttf") },
    { NewSSC("Latin-1 Supplement"),                     { 0x80,  0xFF, 128 },        NewSSC("arial.ttf") },
    { NewSSC("Latin Extended-A"),                       { 0x100, 0x17F, 128 },       NewSSC("arial.ttf") },
    { NewSSC("Latin Extended-B"),                       { 0x180, 0x24F, 208 },       NewSSC("arial.ttf") },
    { NewSSC("IPA Extensions"),                         { 0x250, 0x2AF, 96 },        NewSSC("arial.ttf") },
    { NewSSC("Spacing Modifier Letters"),               { 0x2B0, 0x2FF, 80 },        NewSSC("arial.ttf") },
    { NewSSC("Combining Diacritical Marks"),            { 0x300, 0x36F, 112 },       NewSSC("arial.ttf") },
    { NewSSC("Greek and Coptic"),                       { 0x370, 0x3FF, 144 },       NewSSC("arial.ttf") },
    { NewSSC("Cyrillic"),                               { 0x400, 0x4FF, 256 },       NewSSC("arial.ttf") },
    { NewSSC("Cyrillic Supplement"),                    { 0x500, 0x527, 40 },        NewSSC("arial.ttf") },
    { NewSSC("Armenian"),                               { 0x531, 0x58A, 90 },        NewSSC("arial.ttf") },
    { NewSSC("Hebrew"),                                 { 0x591, 0x5F4, 100 },       NewSSC("arial.ttf") },
    { NewSSC("Arabic"),                                 { 0x600, 0x6FF, 256 },       NewSSC("arial.ttf") },
    { NewSSC("Syriac"),                                 { 0x700, 0x74F, 80 },        NewSSC("seguihis.ttf") },
    { NewSSC("Arabic Supplement"),                      { 0x750, 0x77F, 48 },        NewSSC("arial.ttf") },
    { NewSSC("Thaana"),                                 { 0x780, 0x7B1, 50 },        NewSSC("mvboli.ttf") },
    { NewSSC("NKo"),                                    { 0x7C0, 0x7FA, 59 },        NewSSC("ebrima.ttf") },
    { NewSSC("Samaritan"),                              { 0x800, 0x83E, 63 },        NewSSC("") },
    { NewSSC("Mandaic"),                                { 0x840, 0x85E, 31 },        NewSSC("") },
    { NewSSC("Devanagari"),                             { 0x900, 0x97F, 128 },       NewSSC("Kalam-Bold.ttf") },
    { NewSSC("Bengali"),                                { 0x981, 0x9FB, 123 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Gurmukhi"),                               { 0xA01, 0xA75, 117 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Gujarati"),                               { 0xA81, 0xAF1, 113 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Oriya"),                                  { 0xB01, 0xB77, 119 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Tamil"),                                  { 0xB82, 0xBFA, 121 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Telugu"),                                 { 0xC01, 0xC7F, 127 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Kannada"),                                { 0xC82, 0xCF2, 113 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Malayalam"),                              { 0xD02, 0xD7F, 126 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Sinhala"),                                { 0xD82, 0xDF4, 115 },       NewSSC("Nirmala.ttf") },
    { NewSSC("Thai"),                                   { 0xE01, 0xE5B, 91 },        NewSSC("LeelaUIb.ttf") },
    { NewSSC("Lao"),                                    { 0xE81, 0xEDD, 93 },        NewSSC("LeelaUIb.ttf") },
    { NewSSC("Tibetan"),                                { 0xF00, 0xFDA, 219 },       NewSSC("himalaya.ttf") },
    { NewSSC("Myanmar"),                                { 0x1000, 0x109F, 160 },     NewSSC("mmrtext.ttf") },
    { NewSSC("Georgian"),                               { 0x10A0, 0x10FC, 93 },      NewSSC("calibri.ttf") },
    { NewSSC("Hangul Jamo"),                            { 0x1100, 0x11FF, 256 },     NewSSC("malgun.ttf") },
    { NewSSC("Ethiopic"),                               { 0x1200, 0x137C, 381 },     NewSSC("ebrima.ttf") },
    { NewSSC("Ethiopic Supplement"),                    { 0x1380, 0x1399, 26 },      NewSSC("ebrima.ttf") },
    { NewSSC("Cherokee"),                               { 0x13A0, 0x13F4, 85 },      NewSSC("gadugi.ttf") },
    { NewSSC("Unified Canadian Aboriginal Syllabics"),  { 0x1400, 0x167F, 640 },     NewSSC("gadugi.ttf") },
    { NewSSC("Ogham"),                                  { 0x1680, 0x169C, 29 },      NewSSC("seguihis.ttf") },
    { NewSSC("Runic"),                                  { 0x16A0, 0x16F0, 81 },      NewSSC("seguihis.ttf") },
    { NewSSC("Tagalog"),                                { 0x1700, 0x1714, 21 },      NewSSC("") },
    { NewSSC("Hanunoo"),                                { 0x1720, 0x1736, 23 },      NewSSC("") },
    { NewSSC("Buhid"),                                  { 0x1740, 0x1753, 20 },      NewSSC("") },
    { NewSSC("Tagbanwa"),                               { 0x1760, 0x1773, 20 },      NewSSC("") },
    { NewSSC("Khmer"),                                  { 0x1780, 0x17F9, 122 },     NewSSC("LeelaUIb.ttf") },
    { NewSSC("Mongolian"),                              { 0x1800, 0x18AA, 171 },     NewSSC("monbaiti.ttf") },
    { NewSSC("Unified Canadian Aboriginal Syllabics Extended"),{ 0x18B0, 0x18F5, 70 }, NewSSC("gadugi.ttf") },
    { NewSSC("Limbu"),                                  { 0x1900, 0x194F, 80 },      NewSSC("") },
    { NewSSC("Tai Le"),                                 { 0x1950, 0x1974, 37 },      NewSSC("taile.ttf") },
    { NewSSC("New Tai Lue"),                            { 0x1980, 0x19DF, 96 },      NewSSC("ntailu.ttf") },
    { NewSSC("Khmer Symbols"),                          { 0x19E0, 0x19FF, 32 },      NewSSC("LeelaUIb.ttf") },
    { NewSSC("Buginese"),                               { 0x1A00, 0x1A1F, 32 },      NewSSC("LeelaUIb.ttf") },
    { NewSSC("Tai Tham"),                               { 0x1A20, 0x1AAD, 142 },     NewSSC("") },
    { NewSSC("Balinese"),                               { 0x1B00, 0x1B7C, 125 },     NewSSC("") },
    { NewSSC("Sundanese"),                              { 0x1B80, 0x1BB9, 58 },      NewSSC("") },
    { NewSSC("Batak"),                                  { 0x1BC0, 0x1BFF, 64 },      NewSSC("") },
    { NewSSC("Lepcha"),                                 { 0x1C00, 0x1C4F, 80 },      NewSSC("") },
    { NewSSC("Ol Chiki"),                               { 0x1C50, 0x1C7F, 48 },      NewSSC("Nirmala.ttf") },
    { NewSSC("Vedic Extensions"),                       { 0x1CD0, 0x1CF2, 35 },      NewSSC("Nirmala.ttf") },
    { NewSSC("Phonetic Extensions"),                    { 0x1D00, 0x1D7F, 128 },     NewSSC("arial.ttf") },
    { NewSSC("Phonetic Extensions Supplement"),         { 0x1D80, 0x1DBF, 64 },      NewSSC("arial.ttf") },
    { NewSSC("Combining Diacritical Marks Supplement"), { 0x1DC0, 0x1DFF, 64 },      NewSSC("arial.ttf") },
    { NewSSC("Latin Extended Additional"),              { 0x1E00, 0x1EFF, 256 },     NewSSC("arial.ttf") },
    { NewSSC("Greek Extended"),                         { 0x1F00, 0x1FFE, 255 },     NewSSC("arial.ttf") },
    { NewSSC("General Punctuation"),                    { 0x2000, 0x206F, 112 },     NewSSC("calibri.ttf") },
    { NewSSC("Superscripts and Subscripts"),            { 0x2070, 0x209C, 45 },      NewSSC("BungeeInline-Regular.ttf") },
    { NewSSC("Currency Symbols"),                       { 0x20A0, 0x20B9, 26 },      NewSSC("arial.ttf") },
    { NewSSC("Combining Diacritical Marks for Symbols"),{ 0x20D0, 0x20F0, 33 },      NewSSC("seguisym.ttf") },
    { NewSSC("Letterlike Symbols"),                     { 0x2100, 0x214F, 80 },      NewSSC("l_10646.ttf") },
    { NewSSC("Number Forms"),                           { 0x2150, 0x2189, 58 },      NewSSC("micross.ttf") },
    { NewSSC("Arrows"),                                 { 0x2190, 0x21FF, 112 },     NewSSC("l_10646.ttf") },
    { NewSSC("Mathematical Operators"),                 { 0x2200, 0x22FF, 256 },     NewSSC("l_10646.ttf") },
    { NewSSC("Miscellaneous Technical"),                { 0x2300, 0x23F3, 244 },     NewSSC("seguisym.ttf") },
    { NewSSC("Control Pictures"),                       { 0x2400, 0x2426, 39 },      NewSSC("l_10646.ttf") },
    { NewSSC("Optical Character Recognition"),          { 0x2440, 0x244A, 11 },      NewSSC("seguisym.ttf") },
    { NewSSC("Enclosed Alphanumerics"),                 { 0x2460, 0x24FF, 160 },     NewSSC("malgun.ttf") },
    { NewSSC("Box Drawing"),                            { 0x2500, 0x257F, 128 },     NewSSC("consola.ttf") },
    { NewSSC("Block Elements"),                         { 0x2580, 0x259F, 32 },      NewSSC("arial.ttf") },
    { NewSSC("Geometric Shapes"),                       { 0x25A0, 0x25FF, 96 },      NewSSC("l_10646.ttf") },
    { NewSSC("Miscellaneous Symbols"),                  { 0x2600, 0x26FF, 256 },     NewSSC("seguisym.ttf") },
    { NewSSC("Dingbats"),                               { 0x2701, 0x27BF, 191 },     NewSSC("seguisym.ttf") },
    { NewSSC("Miscellaneous Mathematical Symbols-A"),   { 0x27C0, 0x27EF, 48 },      NewSSC("seguisym.ttf") },
    { NewSSC("Supplemental Arrows-A"),                  { 0x27F0, 0x27FF, 16 },      NewSSC("seguisym.ttf") },
    { NewSSC("Braille Patterns"),                       { 0x2800, 0x28FF, 256 },     NewSSC("seguisym.ttf") },
    { NewSSC("Supplemental Arrows-B"),                  { 0x2900, 0x297F, 128 },     NewSSC("seguisym.ttf") },
    { NewSSC("Miscellaneous Mathematical Symbols-B"),   { 0x2980, 0x29FF, 128 },     NewSSC("seguisym.ttf") },
    { NewSSC("Supplemental Mathematical Operators"),    { 0x2A00, 0x2AFF, 256 },     NewSSC("seguisym.ttf") },
    { NewSSC("Miscellaneous Symbols and Arrows"),       { 0x2B00, 0x2B59, 90 },      NewSSC("seguisym.ttf") },
    { NewSSC("Glagolitic"),                             { 0x2C00, 0x2C5E, 95 },      NewSSC("seguihis.ttf") },
    { NewSSC("Latin Extended-C"),                       { 0x2C60, 0x2C7F, 32 },      NewSSC("arial.ttf") },
    { NewSSC("Coptic"),                                 { 0x2C80, 0x2CFF, 128 },     NewSSC("seguihis.ttf") },
    { NewSSC("Georgian Supplement"),                    { 0x2D00, 0x2D25, 38 },      NewSSC("calibri.ttf") },
    { NewSSC("Tifinagh"),                               { 0x2D30, 0x2D7F, 80 },      NewSSC("ebrima.ttf") },
    { NewSSC("Ethiopic Extended"),                      { 0x2D80, 0x2DDE, 95 },      NewSSC("ebrima.ttf") },
    { NewSSC("Cyrillic Extended-A"),                    { 0x2DE0, 0x2DFF, 32 },      NewSSC("micross.ttf") },
    { NewSSC("Supplemental Punctuation"),               { 0x2E00, 0x2E31, 50 },      NewSSC("seguisym.ttf") },
    { NewSSC("CJK Radicals Supplement"),                { 0x2E80, 0x2EF3, 116 },     NewSSC("") },
    { NewSSC("Kangxi Radicals"),                        { 0x2F00, 0x2FD5, 214 },     NewSSC("") },
    { NewSSC("Ideographic Description Characters"),     { 0x2FF0, 0x2FFB, 12 },      NewSSC("") },
    { NewSSC("CJK Symbols and Punctuation"),            { 0x3000, 0x303F, 64 },      NewSSC("seguisym.ttf") },
    { NewSSC("Hiragana"),                               { 0x3041, 0x309F, 95 },      NewSSC("malgun.ttf") },
    { NewSSC("Katakana"),                               { 0x30A0, 0x30FF, 96 },      NewSSC("malgun.ttf") },
    { NewSSC("Bopomofo"),                               { 0x3105, 0x312D, 41 },      NewSSC("") },
    { NewSSC("Hangul Compatibility Jamo"),              { 0x3131, 0x318E, 94 },      NewSSC("malgun.ttf") },
    { NewSSC("Kanbun"),                                 { 0x3190, 0x319F, 16 },      NewSSC("") },
    { NewSSC("Bopomofo Extended"),                      { 0x31A0, 0x31BA, 27 },      NewSSC("") },
    { NewSSC("CJK Strokes"),                            { 0x31C0, 0x31E3, 36 },      NewSSC("") },
    { NewSSC("Katakana Phonetic Extensions"),           { 0x31F0, 0x31FF, 16 },      NewSSC("") },
    { NewSSC("Enclosed CJK Letters and Months"),        { 0x3200, 0x32FE, 255 },     NewSSC("malgun.ttf") },
    { NewSSC("CJK Compatibility"),                      { 0x3300, 0x33FF, 256 },     NewSSC("malgun.ttf") },
    { NewSSC("CJK Unified Ideographs Extension A"),     { 0x3400, 0x4DB5, 6582 },    NewSSC("malgun.ttf") },
    { NewSSC("Yijing Hexagram Symbols"),                { 0x4DC0, 0x4DFF, 64 },      NewSSC("seguisym.ttf") },
    { NewSSC("CJK Unified Ideographs"),                 { 0x4E00, 0x9FCB, 20940 },   NewSSC("malgun.ttf") },
    { NewSSC("Yi Syllables"),                           { 0xA000, 0xA48C, 1165 },    NewSSC("msyi.ttf") },
    { NewSSC("Yi Radicals"),                            { 0xA490, 0xA4C6, 55 },      NewSSC("msyi.ttf") },
    { NewSSC("Lisu"),                                   { 0xA4D0, 0xA4FF, 48 },      NewSSC("calibri.ttf") },
    { NewSSC("Vai"),                                    { 0xA500, 0xA62B, 300 },     NewSSC("ebrima.ttf") },
    { NewSSC("Cyrillic Extended-B"),                    { 0xA640, 0xA697, 88 },      NewSSC("calibri.ttf") },
    { NewSSC("Bamum"),                                  { 0xA6A0, 0xA6F7, 88 },      NewSSC("") },
    { NewSSC("Modifier Tone Letters"),                  { 0xA700, 0xA71F, 32 },      NewSSC("segoeui.ttf") },
    { NewSSC("Latin Extended-D"),                       { 0xA720, 0xA7FF, 224 },     NewSSC("arial.ttf") },
    { NewSSC("Syloti Nagri"),                           { 0xA800, 0xA82B, 44 },      NewSSC("") },
    { NewSSC("Common Indic Number Forms"),              { 0xA830, 0xA839, 10 },      NewSSC("Nirmala.ttf") },
    { NewSSC("Phags-pa"),                               { 0xA840, 0xA877, 56 },      NewSSC("phagspa.ttf") },
    { NewSSC("Saurashtra"),                             { 0xA880, 0xA8D9, 90 },      NewSSC("") },
    { NewSSC("Devanagari Extended"),                    { 0xA8E0, 0xA8FB, 28 },      NewSSC("Nirmala.ttf") },
    { NewSSC("Kayah Li"),                               { 0xA900, 0xA92F, 48 },      NewSSC("") },
    { NewSSC("Rejang"),                                 { 0xA930, 0xA95F, 48 },      NewSSC("") },
    { NewSSC("Hangul Jamo Extended-A"),                 { 0xA960, 0xA97C, 29 },      NewSSC("malgun.ttf") },
    { NewSSC("Javanese"),                               { 0xA980, 0xA9DF, 96 },      NewSSC("javatext.ttf") },
    { NewSSC("Cham"),                                   { 0xAA00, 0xAA5F, 96 },      NewSSC("") },
    { NewSSC("Myanmar Extended-A"),                     { 0xAA60, 0xAA7B, 28 },      NewSSC("mmrtext.ttf") },
    { NewSSC("Tai Viet"),                               { 0xAA80, 0xAADF, 96 },      NewSSC("") },
    { NewSSC("Ethiopic Extended-A"),                    { 0xAB01, 0xAB2E, 46 },      NewSSC("ebrima.ttf") },
    { NewSSC("Meetei Mayek"),                           { 0xABC0, 0xABF9, 58 },      NewSSC("Nirmala.ttf") },
    { NewSSC("Hangul Syllables"),                       { 0xAC00, 0xD7A3, 11172 },   NewSSC("malgun.ttf") },
    { NewSSC("Hangul Jamo Extended-B"),                 { 0xD7B0, 0xD7FB, 76 },      NewSSC("malgun.ttf") },
    { NewSSC("High Surrogates"),                        { 0xD800, 0xDB7F, 896 },     NewSSC("") },
    { NewSSC("High Private Use Surrogates"),            { 0xDB80, 0xDBFF, 128 },     NewSSC("") },
    { NewSSC("Low Surrogates"),                         { 0xDC00, 0xDFFF, 1024 },    NewSSC("") },
    { NewSSC("Private Use Area"),                       { 0xE000, 0xF8FF, 6400 },    NewSSC("segmdl2.ttf") },
    { NewSSC("CJK Compatibility Ideographs"),           { 0xF900, 0xFAD9, 474 },     NewSSC("malgun.ttf") },
    { NewSSC("Alphabetic Presentation Forms"),          { 0xFB00, 0xFB4F, 80 },      NewSSC("arial.ttf") },
    { NewSSC("Arabic Presentation Forms-A"),            { 0xFB50, 0xFDFD, 686 },     NewSSC("arial.ttf") },
    { NewSSC("Variation Selectors"),                    { 0xFE00, 0xFE0F, 16 },      NewSSC("phagspa.ttf") },
    { NewSSC("Vertical Forms"),                         { 0xFE10, 0xFE19, 10 },      NewSSC("monbaiti.ttf") },
    { NewSSC("Combining Half Marks"),                   { 0xFE20, 0xFE26, 7 },       NewSSC("arial.ttf") },
    { NewSSC("CJK Compatibility Forms"),                { 0xFE30, 0xFE4F, 32 },      NewSSC("himalaya.ttf") },
    { NewSSC("Small Form Variants"),                    { 0xFE50, 0xFE6B, 28 },      NewSSC("") },
    { NewSSC("Arabic Presentation Forms-B"),            { 0xFE70, 0xFEFF, 144 },     NewSSC("arial.ttf") },
    { NewSSC("Halfwidth and Fullwidth Forms"),          { 0xFF01, 0xFFEE, 238 },     NewSSC("BungeeInline-Regular.ttf") },
    { NewSSC("Specials"),                               { 0xFFF9, 0xFFFD, 5 },       NewSSC("") },
    { NewSSC("Linear B Syllabary"),                     { 0x10000, 0x1005D, 94 },    NewSSC("") },
    { NewSSC("Linear B Ideograms"),                     { 0x10080, 0x100FA, 123 },   NewSSC("") },
    { NewSSC("Aegean Numbers"),                         { 0x10100, 0x1013F, 64 },    NewSSC("") },
    { NewSSC("Ancient Greek Numbers"),                  { 0x10140, 0x1018A, 75 },    NewSSC("") },
    { NewSSC("Ancient Symbols"),                        { 0x10190, 0x1019B, 12 },    NewSSC("") },
    { NewSSC("Phaistos Disc"),                          { 0x101D0, 0x101FD, 46 },    NewSSC("") },
    { NewSSC("Lycian"),                                 { 0x10280, 0x1029C, 29 },    NewSSC("seguihis.ttf") },
    { NewSSC("Carian"),                                 { 0x102A0, 0x102D0, 49 },    NewSSC("seguihis.ttf") },
    { NewSSC("Old Italic"),                             { 0x10300, 0x10323, 36 },    NewSSC("seguihis.ttf") },
    { NewSSC("Gothic"),                                 { 0x10330, 0x1034A, 27 },    NewSSC("seguihis.ttf") },
    { NewSSC("Ugaritic"),                               { 0x10380, 0x1039F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Old Persian"),                            { 0x103A0, 0x103D5, 54 },    NewSSC("seguihis.ttf") },
    { NewSSC("Deseret"),                                { 0x10400, 0x1044F, 80 },    NewSSC("seguisym.ttf") },
    { NewSSC("Shavian"),                                { 0x10450, 0x1047F, 48 },    NewSSC("seguihis.ttf") },
    { NewSSC("Osmanya"),                                { 0x10480, 0x104A9, 42 },    NewSSC("ebrima.ttf") },
    { NewSSC("Cypriot Syllabary"),                      { 0x10800, 0x1083F, 64 },    NewSSC("seguihis.ttf") },
    { NewSSC("Imperial Aramaic"),                       { 0x10840, 0x1085F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Phoenician"),                             { 0x10900, 0x1091F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Lydian"),                                 { 0x10920, 0x1093F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Kharoshthi"),                             { 0x10A00, 0x10A58, 89 },    NewSSC("seguihis.ttf") },
    { NewSSC("Old South Arabian"),                      { 0x10A60, 0x10A7F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Avestan"),                                { 0x10B00, 0x10B3F, 64 },    NewSSC("") },
    { NewSSC("Inscriptional Parthian"),                 { 0x10B40, 0x10B5F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Inscriptional Pahlavi"),                  { 0x10B60, 0x10B7F, 32 },    NewSSC("seguihis.ttf") },
    { NewSSC("Old Turkic"),                             { 0x10C00, 0x10C48, 73 },    NewSSC("seguihis.ttf") },
    { NewSSC("Rumi Numeral Symbols"),                   { 0x10E60, 0x10E7E, 31 },    NewSSC("") },
    { NewSSC("Brahmi"),                                 { 0x11000, 0x1106F, 112 },   NewSSC("seguihis.ttf") },
    { NewSSC("Kaithi"),                                 { 0x11080, 0x110C1, 66 },    NewSSC("") },
    { NewSSC("Cuneiform"),                              { 0x12000, 0x1236E, 879 },   NewSSC("seguihis.ttf") },
    { NewSSC("Cuneiform Numbers and Punctuation"),      { 0x12400, 0x12473, 116 },   NewSSC("seguihis.ttf") },
    { NewSSC("Egyptian Hieroglyphs"),                   { 0x13000, 0x1342E, 1071 },  NewSSC("seguihis.ttf") },
    { NewSSC("Bamum Supplement"),                       { 0x16800, 0x16A38, 569 },   NewSSC("") },
    { NewSSC("Kana Supplement"),                        { 0x1B000, 0x1B001, 2 },     NewSSC("") },
    { NewSSC("Byzantine Musical Symbols"),              { 0x1D000, 0x1D0F5, 246 },   NewSSC("") },
    { NewSSC("Musical Symbols"),                        { 0x1D100, 0x1D1DD, 222 },   NewSSC("seguisym.ttf") },
    { NewSSC("Ancient Greek Musical Notation"),         { 0x1D200, 0x1D245, 70 },    NewSSC("") },
    { NewSSC("Tai Xuan Jing Symbols"),                  { 0x1D300, 0x1D356, 87 },    NewSSC("seguisym.ttf") },
    { NewSSC("Counting Rod Numerals"),                  { 0x1D360, 0x1D371, 18 },    NewSSC("") },
    { NewSSC("Mathematical Alphanumeric Symbols"),      { 0x1D400, 0x1D7FF, 1024 },  NewSSC("seguisym.ttf") },
    { NewSSC("Mahjong Tiles"),                          { 0x1F000, 0x1F02B, 44 },    NewSSC("seguiemj.ttf") },
    { NewSSC("Domino Tiles"),                           { 0x1F030, 0x1F093, 100 },   NewSSC("seguisym.ttf") },
    { NewSSC("Playing Cards"),                          { 0x1F0A0, 0x1F0DF, 64 },    NewSSC("seguisym.ttf") },
    { NewSSC("Enclosed Alphanumeric Supplement"),       { 0x1F100, 0x1F1FF, 256 },   NewSSC("seguiemj.ttf") },
    { NewSSC("Enclosed Ideographic Supplement"),        { 0x1F200, 0x1F251, 82 },    NewSSC("seguiemj.ttf") },
    { NewSSC("Miscellaneous Symbols And Pictographs"),  { 0x1F300, 0x1F5FF, 768 },   NewSSC("seguiemj.ttf") },
    { NewSSC("Emoticons"),                              { 0x1F601, 0x1F64F, 79 },    NewSSC("seguiemj.ttf") },
    { NewSSC("Transport And Map Symbols"),              { 0x1F680, 0x1F6C5, 70 },    NewSSC("seguiemj.ttf") },
    { NewSSC("Alchemical Symbols"),                     { 0x1F700, 0x1F773, 116 },   NewSSC("seguisym.ttf") },
    { NewSSC("CJK Unified Ideographs Extension B"),     { 0x20000, 0x2A6D6, 42711 }, NewSSC("simsunb.ttf") },
    { NewSSC("CJK Unified Ideographs Extension C"),     { 0x2A700, 0x2B734, 4149 },  NewSSC("simsunb.ttf") },
    { NewSSC("CJK Unified Ideographs Extension D"),     { 0x2B740, 0x2B81D, 222 },   NewSSC("simsunb.ttf") },
    { NewSSC("CJK Compatibility Ideographs Supplement"),{ 0x2F800, 0x2FA1D, 542 },   NewSSC("") },
    { NewSSC("Tags"),                                   { 0xE0001, 0xE007F, 127 },   NewSSC("") },
    { NewSSC("Variation Selectors Supplement"),         { 0xE0100, 0xE01EF, 240 },   NewSSC("") },
    { NewSSC("Supplementary Private Use Area-A"),       { 0xF0000, 0xFFFFD, 65534 }, NewSSC("") },
    { NewSSC("Supplementary Private Use Area-B"),       { 0x100000, 0x10FFFD, 65534 }, NewSSC("") },
};
#undef NewSSC
#endif

#endif //_FONT__T_D_H
