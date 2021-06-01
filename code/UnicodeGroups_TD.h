/* date = April 11th 2021 8:01 pm */
#ifndef _UNICODE_GROUPS__T_D_H
#define _UNICODE_GROUPS__T_D_H

// TODO:: Maybe make it even friendlier for the compiler. 
// Right now it takes ~1.8s to compile this thing, which
// is still the bottleneck for compiling by a long shot.

// This is done in such a way because having the large list
// as a global variable destroys the CodeGeneration stage
// of the compiler in optimized build.
struct unicode_group
{
    string_c Name;
    codepoint_range CodepointRange;
    string_c BackupFont;
};

#define UNICODE_GROUP_COUNT 209
struct unicode_group_list
{
    unicode_group G[UNICODE_GROUP_COUNT];
};
global_variable unicode_group_list UnicodeGroupList;

internal void
PrepareUnicodeGroupList()
{
#define NewSSC NewStaticStringCompound
    const unicode_group UnicodeGroups[] = {
        /*Name,                                                    First, Last, Length,       Font  */
        { {(u8 *)"Basic Latin", 11, 11},                           {0x0, 0x7F, 128},          {(u8 *)"Segoesc.ttf", 11, 11} },
        { {(u8 *)"Latin-1 Supplement", 18, 18},                    {0x80, 0xFF, 128},         {(u8 *)"Segoesc.ttf", 11, 11} },
        { {(u8 *)"Latin Extended-A", 16, 16},                      {0x100, 0x17F, 128},       {(u8 *)"Segoesc.ttf", 11, 11} },
        { {(u8 *)"Latin Extended-B", 16, 16},                      {0x180, 0x24F, 208},       {(u8 *)"Segoesc.ttf", 11, 11} },
        { {(u8 *)"IPA Extensions", 14, 14},                        {0x250, 0x2AF, 96},        {(u8 *)"arial.", 9, 9} },
        { {(u8 *)"Spacing Modifier Letters", 24, 24},              {0x2B0, 0x2FF, 80},        {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Combining Diacritical Marks", 27, 27},           {0x300, 0x36F, 112},       {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Greek and Coptic", 16, 16},                      {0x370, 0x3FF, 144},       {(u8 *)"Segoesc.ttf", 11, 11} },
        { {(u8 *)"Cyrillic", 8, 8},                                {0x400, 0x4FF, 256},       {(u8 *)"Consola.ttf", 11, 11} },
        { {(u8 *)"Cyrillic Supplement", 19, 19},                   {0x500, 0x527, 40},        {(u8 *)"Consola.ttf", 11, 11} },
        { {(u8 *)"Armenian", 8, 8},                                {0x531, 0x58A, 90},        {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Hebrew", 6, 6},                                  {0x591, 0x5F4, 100},       {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Arabic", 6, 6},                                  {0x600, 0x6FF, 256},       {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Syriac", 6, 6},                                  {0x700, 0x74F, 80},        {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Arabic Supplement", 17, 17},                     {0x750, 0x77F, 48},        {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Thaana", 6, 6},                                  {0x780, 0x7B1, 50},        {(u8 *)"mvboli.ttf", 10, 10} },
        { {(u8 *)"NKo", 3, 3},                                     {0x7C0, 0x7FA, 59},        {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Samaritan", 9, 9},                               {0x800, 0x83E, 63},        {(u8 *)"", 0, 0} },
        { {(u8 *)"Mandaic", 7, 7},                                 {0x840, 0x85E, 31},        {(u8 *)"", 0, 0} },
        { {(u8 *)"Devanagari", 10, 10},                            {0x900, 0x97F, 128},       {(u8 *)"Kalam-Bold.ttf", 14, 14} },
        { {(u8 *)"Bengali", 7, 7},                                 {0x981, 0x9FB, 123},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Gurmukhi", 8, 8},                                {0xA01, 0xA75, 117},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Gujarati", 8, 8},                                {0xA81, 0xAF1, 113},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Oriya", 5, 5},                                   {0xB01, 0xB77, 119},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Tamil", 5, 5},                                   {0xB82, 0xBFA, 121},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Telugu", 6, 6},                                  {0xC01, 0xC7F, 127},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Kannada", 7, 7},                                 {0xC82, 0xCF2, 113},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Malayalam", 9, 9},                               {0xD02, 0xD7F, 126},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Sinhala", 7, 7},                                 {0xD82, 0xDF4, 115},       {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Thai", 4, 4},                                    {0xE01, 0xE5B, 91},        {(u8 *)"LeelaUIb.ttf", 12, 12} },
        { {(u8 *)"Lao", 3, 3},                                     {0xE81, 0xEDD, 93},        {(u8 *)"LeelaUIb.ttf", 12, 12} },
        { {(u8 *)"Tibetan", 7, 7},                                 {0xF00, 0xFDA, 219},       {(u8 *)"himalaya.ttf", 12, 12} },
        { {(u8 *)"Myanmar", 7, 7},                                 {0x1000, 0x109F, 160},     {(u8 *)"mmrtext.ttf", 11, 11} },
        { {(u8 *)"Georgian", 8, 8},                                {0x10A0, 0x10FC, 93},      {(u8 *)"calibri.ttf", 11, 11} },
        { {(u8 *)"Hangul Jamo", 11, 11},                           {0x1100, 0x11FF, 256},     {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Ethiopic", 8, 8},                                {0x1200, 0x137C, 381},     {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Ethiopic Supplement", 19, 19},                   {0x1380, 0x1399, 26},      {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Cherokee", 8, 8},                                {0x13A0, 0x13F4, 85},      {(u8 *)"gadugi.ttf", 10, 10} },
        { {(u8 *)"Unified Canadian Aboriginal Syllabics", 37, 37}, {0x1400, 0x167F, 640},     {(u8 *)"gadugi.ttf", 10, 10} },
        { {(u8 *)"Ogham", 5, 5},                                   {0x1680, 0x169C, 29},      {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Runic", 5, 5},                                   {0x16A0, 0x16F0, 81},      {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Tagalog", 7, 7},                                 {0x1700, 0x1714, 21},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Hanunoo", 7, 7},                                 {0x1720, 0x1736, 23},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Buhid", 5, 5},                                   {0x1740, 0x1753, 20},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Tagbanwa", 8, 8},                                {0x1760, 0x1773, 20},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Khmer", 5, 5},                                   {0x1780, 0x17F9, 122},     {(u8 *)"LeelaUIb.ttf", 12, 12} },
        { {(u8 *)"Mongolian", 9, 9},                               {0x1800, 0x18AA, 171},     {(u8 *)"monbaiti.ttf", 12, 12} },
        { {(u8 *)"Unified Canadian Aboriginal Syllabics Extended", 46, 46}, {0x18B0, 0x18F5, 70}, {(u8 *)"gadugi.ttf", 10, 10} },
        { {(u8 *)"Limbu", 5, 5},                                   {0x1900, 0x194F, 80},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Tai Le", 6, 6},                                  {0x1950, 0x1974, 37},      {(u8 *)"taile.ttf", 9, 9} },
        { {(u8 *)"New Tai Lue", 11, 11},                           {0x1980, 0x19DF, 96},      {(u8 *)"ntailu.ttf", 10, 10} },
        { {(u8 *)"Khmer Symbols", 13, 13},                         {0x19E0, 0x19FF, 32},      {(u8 *)"LeelaUIb.ttf", 12, 12} },
        { {(u8 *)"Buginese", 8, 8},                                {0x1A00, 0x1A1F, 32},      {(u8 *)"LeelaUIb.ttf", 12, 12} },
        { {(u8 *)"Tai Tham", 8, 8},                                {0x1A20, 0x1AAD, 142},     {(u8 *)"", 0, 0} },
        { {(u8 *)"Balinese", 8, 8},                                {0x1B00, 0x1B7C, 125},     {(u8 *)"", 0, 0} },
        { {(u8 *)"Sundanese", 9, 9},                               {0x1B80, 0x1BB9, 58},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Batak", 5, 5},                                   {0x1BC0, 0x1BFF, 64},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Lepcha", 6, 6},                                  {0x1C00, 0x1C4F, 80},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Ol Chiki", 8, 8},                                {0x1C50, 0x1C7F, 48},      {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Vedic Extensions", 16, 16},                      {0x1CD0, 0x1CF2, 35},      {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Phonetic Extensions", 19, 19},                   {0x1D00, 0x1D7F, 128},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Phonetic Extensions Supplement", 30, 30},        {0x1D80, 0x1DBF, 64},      {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Combining Diacritical Marks Supplement", 38, 38}, {0x1DC0, 0x1DFF, 64},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Latin Extended Additional", 25, 25},             {0x1E00, 0x1EFF, 256},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Greek Extended", 14, 14},                        {0x1F00, 0x1FFE, 255},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"General Punctuation", 19, 19},                   {0x2000, 0x206F, 112},     {(u8 *)"calibri.ttf", 11, 11} },
        { {(u8 *)"Superscripts and Subscripts", 27, 27},           {0x2070, 0x209C, 45},      {(u8 *)"BungeeInline-Regular.ttf", 24, 24} },
        { {(u8 *)"Currency Symbols", 16, 16},                      {0x20A0, 0x20B9, 26},      {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Combining Diacritical Marks for Symbols", 39, 39}, {0x20D0, 0x20F0, 33},    {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Letterlike Symbols", 18, 18},                    {0x2100, 0x214F, 80},      {(u8 *)"l_10646.ttf", 11, 11} },
        { {(u8 *)"Number Forms", 12, 12},                          {0x2150, 0x2189, 58},      {(u8 *)"micross.ttf", 11, 11} },
        { {(u8 *)"Arrows", 6, 6},                                  {0x2190, 0x21FF, 112},     {(u8 *)"l_10646.ttf", 11, 11} },
        { {(u8 *)"Mathematical Operators", 22, 22},                {0x2200, 0x22FF, 256},     {(u8 *)"l_10646.ttf", 11, 11} },
        { {(u8 *)"Miscellaneous Technical", 23, 23},               {0x2300, 0x23F3, 244},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Control Pictures", 16, 16},                      {0x2400, 0x2426, 39},      {(u8 *)"l_10646.ttf", 11, 11} },
        { {(u8 *)"Optical Character Recognition", 29, 29},         {0x2440, 0x244A, 11},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Enclosed Alphanumerics", 22, 22},                {0x2460, 0x24FF, 160},     {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Box Drawing", 11, 11},                           {0x2500, 0x257F, 128},     {(u8 *)"consola.ttf", 11, 11} },
        { {(u8 *)"Block Elements", 14, 14},                        {0x2580, 0x259F, 32},      {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Geometric Shapes", 16, 16},                      {0x25A0, 0x25FF, 96},      {(u8 *)"l_10646.ttf", 11, 11} },
        { {(u8 *)"Miscellaneous Symbols", 21, 21},                 {0x2600, 0x26FF, 256},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Dingbats", 8, 8},                                {0x2701, 0x27BF, 191},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Miscellaneous Mathematical Symbols-A", 36, 36},  {0x27C0, 0x27EF, 48},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Supplemental Arrows-A", 21, 21},                 {0x27F0, 0x27FF, 16},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Braille Patterns", 16, 16},                      {0x2800, 0x28FF, 256},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Supplemental Arrows-B", 21, 21},                 {0x2900, 0x297F, 128},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Miscellaneous Mathematical Symbols-B", 36, 36},  {0x2980, 0x29FF, 128},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Supplemental Mathematical Operators", 35, 35},   {0x2A00, 0x2AFF, 256},     {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Miscellaneous Symbols and Arrows", 32, 32},      {0x2B00, 0x2B59, 90},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Glagolitic", 10, 10},                            {0x2C00, 0x2C5E, 95},      {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Latin Extended-C", 16, 16},                      {0x2C60, 0x2C7F, 32},      {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Coptic", 6, 6},                                  {0x2C80, 0x2CFF, 128},     {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Georgian Supplement", 19, 19},                   {0x2D00, 0x2D25, 38},      {(u8 *)"calibri.ttf", 11, 11} },
        { {(u8 *)"Tifinagh", 8, 8},                                {0x2D30, 0x2D7F, 80},      {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Ethiopic Extended", 17, 17},                     {0x2D80, 0x2DDE, 95},      {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Cyrillic Extended-A", 19, 19},                   {0x2DE0, 0x2DFF, 32},      {(u8 *)"micross.ttf", 11, 11} },
        { {(u8 *)"Supplemental Punctuation", 24, 24},              {0x2E00, 0x2E31, 50},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"CJK Radicals Supplement", 23, 23},               {0x2E80, 0x2EF3, 116},     {(u8 *)"", 0, 0} },
        { {(u8 *)"Kangxi Radicals", 15, 15},                       {0x2F00, 0x2FD5, 214},     {(u8 *)"", 0, 0} },
        { {(u8 *)"Ideographic Description Characters", 34, 34},    {0x2FF0, 0x2FFB, 12},      {(u8 *)"", 0, 0} },
        { {(u8 *)"CJK Symbols and Punctuation", 27, 27},           {0x3000, 0x303F, 64},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Hiragana", 8, 8},                                {0x3041, 0x309F, 95},      {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Katakana", 8, 8},                                {0x30A0, 0x30FF, 96},      {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Bopomofo", 8, 8},                                {0x3105, 0x312D, 41},      {(u8 *)"kaiu.ttf", 8, 8} },
        { {(u8 *)"Hangul Compatibility Jamo", 25, 25},             {0x3131, 0x318E, 94},      {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Kanbun", 6, 6},                                  {0x3190, 0x319F, 16},      {(u8 *)"kaiu.ttf", 8, 8} },
        { {(u8 *)"Bopomofo Extended", 17, 17},                     {0x31A0, 0x31BA, 27},      {(u8 *)"", 0, 0} },
        { {(u8 *)"CJK Strokes", 11, 11},                           {0x31C0, 0x31E3, 36},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Katakana Phonetic Extensions", 28, 28},          {0x31F0, 0x31FF, 16},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Enclosed CJK Letters and Months", 31, 31},       {0x3200, 0x32FE, 255},     {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"CJK Compatibility", 17, 17},                     {0x3300, 0x33FF, 256},     {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"CJK Unified Ideographs Extension A", 34, 34},    {0x3400, 0x4DB5, 6582},    {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Yijing Hexagram Symbols", 23, 23},               {0x4DC0, 0x4DFF, 64},      {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"CJK Unified Ideographs", 22, 22},                {0x4E00, 0x9FCB, 20940},   {(u8 *)"mingliu.ttc", 11, 11} },
        { {(u8 *)"Yi Syllables", 12, 12},                          {0xA000, 0xA48C, 1165},    {(u8 *)"msyi.ttf", 8, 8} },
        { {(u8 *)"Yi Radicals", 11, 11},                           {0xA490, 0xA4C6, 55},      {(u8 *)"msyi.ttf", 8, 8} },
        { {(u8 *)"Lisu", 4, 4},                                    {0xA4D0, 0xA4FF, 48},      {(u8 *)"calibri.ttf", 11, 11} },
        { {(u8 *)"Vai", 3, 3},                                     {0xA500, 0xA62B, 300},     {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Cyrillic Extended-B", 19, 19},                   {0xA640, 0xA697, 88},      {(u8 *)"calibri.ttf", 11, 11} },
        { {(u8 *)"Bamum", 5, 5},                                   {0xA6A0, 0xA6F7, 88},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Modifier Tone Letters", 21, 21},                 {0xA700, 0xA71F, 32},      {(u8 *)"segoeui.ttf", 11, 11} },
        { {(u8 *)"Latin Extended-D", 16, 16},                      {0xA720, 0xA7FF, 224},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Syloti Nagri", 12, 12},                          {0xA800, 0xA82B, 44},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Common Indic Number Forms", 25, 25},             {0xA830, 0xA839, 10},      {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Phags-pa", 8, 8},                                {0xA840, 0xA877, 56},      {(u8 *)"phagspa.ttf", 11, 11} },
        { {(u8 *)"Saurashtra", 10, 10},                            {0xA880, 0xA8D9, 90},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Devanagari Extended", 19, 19},                   {0xA8E0, 0xA8FB, 28},      {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Kayah Li", 8, 8},                                {0xA900, 0xA92F, 48},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Rejang", 6, 6},                                  {0xA930, 0xA95F, 48},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Hangul Jamo Extended-A", 22, 22},                {0xA960, 0xA97C, 29},      {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Javanese", 8, 8},                                {0xA980, 0xA9DF, 96},      {(u8 *)"javatext.ttf", 12, 12} },
        { {(u8 *)"Cham", 4, 4},                                    {0xAA00, 0xAA5F, 96},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Myanmar Extended-A", 18, 18},                    {0xAA60, 0xAA7B, 28},      {(u8 *)"mmrtext.ttf", 11, 11} },
        { {(u8 *)"Tai Viet", 8, 8},                                {0xAA80, 0xAADF, 96},      {(u8 *)"", 0, 0} },
        { {(u8 *)"Ethiopic Extended-A", 19, 19},                   {0xAB01, 0xAB2E, 46},      {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Meetei Mayek", 12, 12},                          {0xABC0, 0xABF9, 58},      {(u8 *)"Nirmala.ttf", 11, 11} },
        { {(u8 *)"Hangul Syllables", 16, 16},                      {0xAC00, 0xD7A3, 11172},   {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Hangul Jamo Extended-B", 22, 22},                {0xD7B0, 0xD7FB, 76},      {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"High Surrogates", 15, 15},                       {0xD800, 0xDB7F, 896},     {(u8 *)"", 0, 0} },
        { {(u8 *)"High Private Use Surrogates", 27, 27},           {0xDB80, 0xDBFF, 128},     {(u8 *)"", 0, 0} },
        { {(u8 *)"Low Surrogates", 14, 14},                        {0xDC00, 0xDFFF, 1024},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Private Use Area", 16, 16},                      {0xE000, 0xF8FF, 6400},    {(u8 *)"segmdl2.ttf", 11, 11} },
        { {(u8 *)"CJK Compatibility Ideographs", 28, 28},          {0xF900, 0xFAD9, 474},     {(u8 *)"malgun.ttf", 10, 10} },
        { {(u8 *)"Alphabetic Presentation Forms", 29, 29},         {0xFB00, 0xFB4F, 80},      {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Arabic Presentation Forms-A", 27, 27},           {0xFB50, 0xFDFD, 686},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Variation Selectors", 19, 19},                   {0xFE00, 0xFE0F, 16},      {(u8 *)"phagspa.ttf", 11, 11} },
        { {(u8 *)"Vertical Forms", 14, 14},                        {0xFE10, 0xFE19, 10},      {(u8 *)"monbaiti.ttf", 12, 12} },
        { {(u8 *)"Combining Half Marks", 20, 20},                  {0xFE20, 0xFE26, 7},       {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"CJK Compatibility Forms", 23, 23},               {0xFE30, 0xFE4F, 32},      {(u8 *)"himalaya.ttf", 12, 12} },
        { {(u8 *)"Small Form Variants", 19, 19},                   {0xFE50, 0xFE6B, 28},      {(u8 *)"kaiu.ttf", 8, 8} },
        { {(u8 *)"Arabic Presentation Forms-B", 27, 27},           {0xFE70, 0xFEFF, 144},     {(u8 *)"arial.ttf", 9, 9} },
        { {(u8 *)"Halfwidth and Fullwidth Forms", 29, 29},         {0xFF01, 0xFFEE, 238},     {(u8 *)"BungeeInline-Regular.ttf", 24, 24} },
        { {(u8 *)"Specials", 8, 8},                                {0xFFF9, 0xFFFD, 5},       {(u8 *)"", 0, 0} },
        { {(u8 *)"Linear B Syllabary", 18, 18},                    {0x10000, 0x1005D, 94},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Linear B Ideograms", 18, 18},                    {0x10080, 0x100FA, 123},   {(u8 *)"", 0, 0} },
        { {(u8 *)"Aegean Numbers", 14, 14},                        {0x10100, 0x1013F, 64},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Ancient Greek Numbers", 21, 21},                 {0x10140, 0x1018A, 75},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Ancient Symbols", 15, 15},                       {0x10190, 0x1019B, 12},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Phaistos Disc", 13, 13},                         {0x101D0, 0x101FD, 46},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Lycian", 6, 6},                                  {0x10280, 0x1029C, 29},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Carian", 6, 6},                                  {0x102A0, 0x102D0, 49},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Old Italic", 10, 10},                            {0x10300, 0x10323, 36},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Gothic", 6, 6},                                  {0x10330, 0x1034A, 27},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Ugaritic", 8, 8},                                {0x10380, 0x1039F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Old Persian", 11, 11},                           {0x103A0, 0x103D5, 54},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Deseret", 7, 7},                                 {0x10400, 0x1044F, 80},    {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Shavian", 7, 7},                                 {0x10450, 0x1047F, 48},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Osmanya", 7, 7},                                 {0x10480, 0x104A9, 42},    {(u8 *)"ebrima.ttf", 10, 10} },
        { {(u8 *)"Cypriot Syllabary", 17, 17},                     {0x10800, 0x1083F, 64},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Imperial Aramaic", 16, 16},                      {0x10840, 0x1085F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Phoenician", 10, 10},                            {0x10900, 0x1091F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Lydian", 6, 6},                                  {0x10920, 0x1093F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Kharoshthi", 10, 10},                            {0x10A00, 0x10A58, 89},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Old South Arabian", 17, 17},                     {0x10A60, 0x10A7F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Avestan", 7, 7},                                 {0x10B00, 0x10B3F, 64},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Inscriptional Parthian", 22, 22},                {0x10B40, 0x10B5F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Inscriptional Pahlavi", 21, 21},                 {0x10B60, 0x10B7F, 32},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Old Turkic", 10, 10},                            {0x10C00, 0x10C48, 73},    {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Rumi Numeral Symbols", 20, 20},                  {0x10E60, 0x10E7E, 31},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Brahmi", 6, 6},                                  {0x11000, 0x1106F, 112},   {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Kaithi", 6, 6},                                  {0x11080, 0x110C1, 66},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Cuneiform", 9, 9},                               {0x12000, 0x1236E, 879},   {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Cuneiform Numbers and Punctuation", 33, 33},     {0x12400, 0x12473, 116},   {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Egyptian Hieroglyphs", 20, 20},                  {0x13000, 0x1342E, 1071},  {(u8 *)"seguihis.ttf", 12, 12} },
        { {(u8 *)"Bamum Supplement", 16, 16},                      {0x16800, 0x16A38, 569},   {(u8 *)"", 0, 0} },
        { {(u8 *)"Kana Supplement", 15, 15},                       {0x1B000, 0x1B001, 2},     {(u8 *)"", 0, 0} },
        { {(u8 *)"Byzantine Musical Symbols", 25, 25},             {0x1D000, 0x1D0F5, 246},   {(u8 *)"", 0, 0} },
        { {(u8 *)"Musical Symbols", 15, 15},                       {0x1D100, 0x1D1DD, 222},   {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Ancient Greek Musical Notation", 30, 30},        {0x1D200, 0x1D245, 70},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Tai Xuan Jing Symbols", 21, 21},                 {0x1D300, 0x1D356, 87},    {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Counting Rod Numerals", 21, 21},                 {0x1D360, 0x1D371, 18},    {(u8 *)"", 0, 0} },
        { {(u8 *)"Mathematical Alphanumeric Symbols", 33, 33},     {0x1D400, 0x1D7FF, 1024},  {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Mahjong Tiles", 13, 13},                         {0x1F000, 0x1F02B, 44},    {(u8 *)"seguiemj.ttf", 12, 12} },
        { {(u8 *)"Domino Tiles", 12, 12},                          {0x1F030, 0x1F093, 100},   {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Playing Cards", 13, 13},                         {0x1F0A0, 0x1F0DF, 64},    {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"Enclosed Alphanumeric Supplement", 32, 32},      {0x1F100, 0x1F1FF, 256},   {(u8 *)"seguiemj.ttf", 12, 12} },
        { {(u8 *)"Enclosed Ideographic Supplement", 31, 31},       {0x1F200, 0x1F251, 82},    {(u8 *)"seguiemj.ttf", 12, 12} },
        { {(u8 *)"Miscellaneous Symbols And Pictographs", 37, 37}, {0x1F300, 0x1F5FF, 768},   {(u8 *)"seguiemj.ttf", 12, 12} },
        { {(u8 *)"Emoticons", 9, 9},                               {0x1F601, 0x1F64F, 79},    {(u8 *)"seguiemj.ttf", 12, 12} },
        { {(u8 *)"Transport And Map Symbols", 25, 25},             {0x1F680, 0x1F6C5, 70},    {(u8 *)"seguiemj.ttf", 12, 12} },
        { {(u8 *)"Alchemical Symbols", 18, 18},                    {0x1F700, 0x1F773, 116},   {(u8 *)"seguisym.ttf", 12, 12} },
        { {(u8 *)"CJK Unified Ideographs Extension B", 34, 34},    {0x20000, 0x2A6D6, 42711}, {(u8 *)"simsunb.ttf", 11, 11} },
        { {(u8 *)"CJK Unified Ideographs Extension C", 34, 34},    {0x2A700, 0x2B734, 4149},  {(u8 *)"simsunb.ttf", 11, 11} },
        { {(u8 *)"CJK Unified Ideographs Extension D", 34, 34},    {0x2B740, 0x2B81D, 222},   {(u8 *)"simsunb.ttf", 11, 11} },
        { {(u8 *)"CJK Compatibility Ideographs Supplement", 39, 39}, {0x2F800, 0x2FA1D, 542}, {(u8 *)"", 0, 0} },
        { {(u8 *)"Tags", 4, 4},                                    {0xE0001, 0xE007F, 127},   {(u8 *)"", 0, 0} },
        { {(u8 *)"Variation Selectors Supplement", 30, 30},        {0xE0100, 0xE01EF, 240},   {(u8 *)"", 0, 0} },
        { {(u8 *)"Supplementary Private Use Area-A", 32, 32},      {0xF0000, 0xFFFFD, 65534}, {(u8 *)"", 0, 0} },
        { {(u8 *)"Supplementary Private Use Area-B", 32, 32},      {0x100000, 0x10FFFD, 65534}, {(u8 *)"", 0, 0} },
    };
#undef NewSSC
    
    For(ArrayCount(UnicodeGroups))
    {
        UnicodeGroupList.G[It] = UnicodeGroups[It];
    }
}




#endif //_UNICODE_GROUPS__T_D_H
