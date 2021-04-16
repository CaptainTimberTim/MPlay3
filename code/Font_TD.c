#include "Font_TD.h"

inline font_atlas
NewFontAtlas(settings *Settings, r32 *FontSizes, u32 SizesCount)
{
    PrepareUnicodeGroupList();
    font_atlas Result = {FontSizes, SizesCount, Settings->FontHeightOffset, 5, 0, NULL, Settings->CachedFontNames};
    return Result;
}

internal read_file_result
GetUsedFontData(game_state *GameState)
{
    read_file_result Result = {carmini_DataCount, (u8 *)carmini_Data};
    
    if(GameState->Settings.FontPath.Pos > 0)
    {
        read_file_result FontFile = {};
        if(ReadEntireFile(&GameState->ScratchArena, &FontFile, GameState->Settings.FontPath.S))
        {
            Result = FontFile;
        }
    }
    
    return Result;
}

internal unicode_group *
GetUnicodeGroup(u32 SampleCodePoint)
{
    unicode_group *Result = 0;
    For(UNICODE_GROUP_COUNT)
    {
        if(SampleCodePoint >= UnicodeGroupList.G[It].CodepointRange.First &&
           SampleCodePoint <  UnicodeGroupList.G[It].CodepointRange.Last) 
        {
            Result = UnicodeGroupList.G + It;
            break;
        }
    }
    return Result;
}

inline void
ExpandFontGroupArrayIfNeeded(arena_allocator *Arena, font_atlas *Atlas)
{
    Assert(Atlas->MaxCount > 0);
    if(Atlas->Count == 0)
    {
        Atlas->FontGroups = AllocateArray(Arena, Atlas->MaxCount, font_group);
    }
    else if(Atlas->Count >= Atlas->MaxCount)
    {
        Atlas->FontGroups = ReallocateArray(Arena, Atlas->FontGroups, Atlas->MaxCount, Atlas->MaxCount*2, font_group);
        Atlas->MaxCount *= 2;
    }
}

internal void
LoadFonts(arena_allocator *FixArena, arena_allocator *ScratchArena, font_atlas *Atlas, u8 *RawFontData, u32 *CodepointsFromGroup, u32 CodepointCount)
{
    For(CodepointCount, Point)
    {
        ExpandFontGroupArrayIfNeeded(FixArena, Atlas);
        
        u32 SampleCodePoint  = CodepointsFromGroup[PointIt];
        unicode_group *Group = GetUnicodeGroup(SampleCodePoint);
        b32 Found            = false;
        For(Atlas->Count, Exists)
        {
            if(Atlas->FontGroups[ExistsIt].UnicodeGroup == Group)
            {
                // We can load a group only partially, thats why we check
                // if we are in that specific loaded range.
                if(SampleCodePoint >= Atlas->FontGroups[ExistsIt].CodepointRange.First &&
                   SampleCodePoint <  Atlas->FontGroups[ExistsIt].CodepointRange.Last)
                {
                    Found = true;
                    break;
                }
            }
        }
        if(Found) continue; // This font group is already loaded. Skip to next one.
        
        font_group *NewFontGroup   = Atlas->FontGroups + Atlas->Count++;
        NewFontGroup->UnicodeGroup = Group;
        
        if(Group->CodepointRange.Count > MAX_CODEPOINT_BLOCK)
        {
            // We align the block to the existing group, and load based on our sampleCodepoint.
            // This subdivides the group neatly into smaller ones
            i32 PositionInGroup = SampleCodePoint - Group->CodepointRange.First;
            Assert(PositionInGroup >= 0);
            u32 Block = PositionInGroup/USED_CODEPOINT_RANGE; // Truncates to full integer.
            NewFontGroup->CodepointRange.First = Group->CodepointRange.First + USED_CODEPOINT_RANGE*Block;
            NewFontGroup->CodepointRange.Last  = Group->CodepointRange.First  + USED_CODEPOINT_RANGE*(Block+1);
            NewFontGroup->CodepointRange.Last  = Min(NewFontGroup->CodepointRange.Last, Group->CodepointRange.Last);
            NewFontGroup->CodepointRange.Count = NewFontGroup->CodepointRange.Last - NewFontGroup->CodepointRange.First;
        }
        else NewFontGroup->CodepointRange = Group->CodepointRange;
        
        u32 TotalArea = 0;
        For(Atlas->SizesCount, Size)
        {
            // Summing all areas of each codepoint we want to load. 
            // For this we assume that the codepoint is a square,
            // which seems to work. The only exception is for very
            // few codepoints (like 1) or some special glyphs like 
            // emoticons, thats why we multiply by 2.5. 
            // TODO:: Think of something better that *2.5?
            TotalArea += Square((i32)(Atlas->FontSizes[SizeIt]*2.5f))*(NewFontGroup->CodepointRange.Count);
        }
        u32 Width  = (u32)Sqrt(TotalArea); // First calc the side for square with correct total area,
        Width      = (Width/8)*8;          // then fit the width to a properly byte aligned length.
        u32 Height = TotalArea/Width;      // Finally, calc the length of the height.
        NewFontGroup->BitmapWidth  = Width;
        NewFontGroup->BitmapHeight = Height;
        //DebugLog(250, "The claculated bitmap area for is %i, the sides are %i/%i\n", TotalArea, Width, Height);
        
        u8 *AlphaMap         = AllocateMemory(ScratchArena, Width*Height);
        loaded_bitmap Bitmap = {true, Width, Height, (u32 *)AlphaMap, colorFormat_Alpha, Width};
        
        stbtt_pack_context PackContext;
        stbtt_PackBegin(&PackContext, AlphaMap, Width, Height, 0, 1, NULL);
        
        NewFontGroup->FontSizes = AllocateArray(FixArena, Atlas->SizesCount, font_data);
        NewFontGroup->Count     = Atlas->SizesCount;
        For(Atlas->SizesCount)
        {
            NewFontGroup->FontSizes[It].Size     = Atlas->FontSizes[It];
            NewFontGroup->FontSizes[It].CharData = AllocateArray(FixArena, NewFontGroup->CodepointRange.Count, 
                                                                 stbtt_packedchar);
            
            stbtt_PackSetOversampling(&PackContext, 3, 1);
            i32 R=stbtt_PackFontRange(&PackContext, RawFontData, 0, Atlas->FontSizes[It], NewFontGroup->CodepointRange.First, NewFontGroup->CodepointRange.Count, NewFontGroup->FontSizes[It].CharData);
            Assert(R != 0); // Most likely the given bitmap is too small.
        }
        stbtt_PackEnd(&PackContext);
        
        NewFontGroup->GLID = CreateGLTexture(Bitmap, true);
        
        FreeMemory(ScratchArena, AlphaMap);
    }
}

internal b32
IsCodepointInFont(u8 *Data, u32 Codepoint)
{
    b32 Result = false;
    
    stbtt_fontinfo FontInfo;
    if(stbtt_InitFont(&FontInfo, Data, 0))
    {
        if(stbtt_FindGlyphIndex(&FontInfo, Codepoint))
        {
            Result = true;
        }
    }
    
    return Result;
}

internal b32
AddMissingFontGroup(game_state *GS, font_atlas *Atlas, u32 Codepoint)
{
    b32 Result = false;
    local_persist timer T = StartTimer();
    
    u32 GroupCodepoints[] = { Codepoint };
    unicode_group *Group = GetUnicodeGroup(Codepoint);
    
    read_file_result DefaultFont = GetUsedFontData(GS);
    if(IsCodepointInFont(DefaultFont.Data, Codepoint))
    {
        LoadFonts(&GS->FixArena, &GS->ScratchArena, Atlas, DefaultFont.Data, GroupCodepoints, ArrayCount(GroupCodepoints));
        Result = true;
    }
    else if(Group->BackupFont.Pos > 0)
    {
        NewEmptyLocalString(FontPath, 260);
        NewLocalString(Slash, 2, "\\");
        ConcatStringCompounds(4, &FontPath, &GS->FontPath, &Slash, &Group->BackupFont);
        
        read_file_result Font = {};
        if(ReadEntireFile(&GS->ScratchArena, &Font, FontPath.S))
        {
            if(IsCodepointInFont(Font.Data, Codepoint))
            {
                LoadFonts(&GS->FixArena, &GS->ScratchArena, Atlas, Font.Data, GroupCodepoints, ArrayCount(GroupCodepoints));
                Result = true;
            }
            FreeFileMemory(&GS->ScratchArena, Font);
        }
    }
    if(!Result && Atlas->UsedFontNames->Count > 0)
    {
        For(Atlas->UsedFontNames->Count)
        {
            NewEmptyLocalString(FontPath, 260);
            NewLocalString(Slash, 2, "\\");
            ConcatStringCompounds(4, &FontPath, &GS->FontPath, &Slash, Atlas->UsedFontNames->Names+It);
            
            read_file_result Font = {};
            if(ReadEntireFile(&GS->ScratchArena, &Font, FontPath.S))
            {
                if(IsCodepointInFont(Font.Data, Codepoint))
                {
                    LoadFonts(&GS->FixArena, &GS->ScratchArena, Atlas, Font.Data, GroupCodepoints, ArrayCount(GroupCodepoints));
                    Result = true;
                }
                FreeFileMemory(&GS->ScratchArena, Font);
            }
        }
    }
    
#ifdef SEARCH_FOR_MISSING_FONT
    // If we still don't have our font, we start
    // searching the users machine for a matching font.
    if(!Result)
    {
        DebugLog(50, "Search far and wide for %i\n", Codepoint);
        raw_font RawFont = {Codepoint, &GS->FontPath, /*Will be filled in procedure*/};
        if(FindAndLoadFontWithUnicodeCodepoint(&GS->ScratchArena, &RawFont))
        {
            LoadFonts(&GS->FixArena, &GS->ScratchArena, Atlas, RawFont.Data.Data, 
                      GroupCodepoints, ArrayCount(GroupCodepoints));
            
            if(Atlas->UsedFontNames->Count >= Atlas->UsedFontNames->MaxCount)
            {
                Atlas->UsedFontNames->MaxCount += 10;
                Atlas->UsedFontNames->Names = ReallocateArray(&GS->FixArena, Atlas->UsedFontNames->Names, 
                                                              Atlas->UsedFontNames->Count,
                                                              Atlas->UsedFontNames->MaxCount, string_c);
            }
            Atlas->UsedFontNames->Names[Atlas->UsedFontNames->Count] = NewStringCompound(&GS->FixArena, RawFont.Name.Pos);
            AppendStringCompoundToCompound(Atlas->UsedFontNames->Names+Atlas->UsedFontNames->Count, &RawFont.Name);
            ++Atlas->UsedFontNames->Count;
            
            Result = true;
        }
        else
        {
            // If we do not find anything for this codepoint. We just render the [] glyph.
            // In order to cache this information, and not retry the next time we get this
            // codepoint, we create a 'fake' font_group with the 'Basic Latin' texture.
            ExpandFontGroupArrayIfNeeded(&GS->FixArena, Atlas);
            font_group *FakeGroup = Atlas->FontGroups + Atlas->Count++;
            
            *FakeGroup = *GetFontGroup(GS, Atlas, (u32)'o');
            FakeGroup->CodepointRange.First = Codepoint;
            FakeGroup->CodepointRange.Last  = Codepoint+1;
            FakeGroup->CodepointRange.Count = 1;
            Result = true;
        }
    }
#endif
    string_c TT = NewStaticStringCompound("GetFontGroup");
    SnapTimer(&T, TT);
    
    return Result;
}

internal font_group *
GetFontGroup(game_state *GS, font_atlas *Atlas, u32 Codepoint)
{
    font_group *Result = 0;
    
    For(Atlas->Count)
    {
        if(Codepoint >= Atlas->FontGroups[It].CodepointRange.First &&
           Codepoint <  Atlas->FontGroups[It].CodepointRange.Last)
        {
            Result = Atlas->FontGroups + It;
            break;
        }
    }
    if(!Result) 
    {
        if(AddMissingFontGroup(GS, Atlas, Codepoint))
        {
            Result = Atlas->FontGroups + (Atlas->Count-1);
        }
    }
    
    return Result;
}

internal render_entry
CreateFontEntry(v2 Extends, r32 Depth, u32 BitmapID, v3 *Color, entry_id *Parent = 0)
{
    render_entry Result = {};
    
    // These are not inserted into the renderer list. They only exist as
    // children of the render_text.Base, which is in that list.
    Result.Type = renderType_Text;
    Result.Render = true;
    Result.Transform = {};
    Result.Transform.Scale = {1,1};
    Result.Vertice[0] = {-Extends.x, -Extends.y, Depth};
    Result.Vertice[1] = {-Extends.x,  Extends.y, Depth};
    Result.Vertice[2] = { Extends.x,  Extends.y, Depth};
    Result.Vertice[3] = { Extends.x, -Extends.y, Depth};
    Result.Parent = Parent;
    Result.Transparency = 1.0f;
    Result.Color = Color;
    Result.TexID = BitmapID;
    
    return Result;
}

internal void
RenderText(game_state *GS, arena_allocator *Arena, font_size FontSize, string_c *Text,
           v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent, v2 StartP)
{
    // Create memory if this render_text was not used before
    // or is not big enough for the given text.
    if(ResultText->MaxCount == 0)
    {
        ResultText->MaxCount         = Max(CHARACTERS_PER_TEXT_INFO, Text->Pos+1);
        ResultText->RenderEntries    = AllocateArray(Arena, ResultText->MaxCount, render_entry);
    }
    else if (ResultText->MaxCount < Text->Pos)
    {
        ResultText->RenderEntries = ReallocateArray(Arena, ResultText->RenderEntries, ResultText->MaxCount, 
                                                    Text->Pos, render_entry);
        ResultText->MaxCount = Text->Pos;
    }
    Assert(Text->Pos < ResultText->MaxCount); 
    
    
    renderer *Renderer = &GS->Renderer;
    font_atlas *Atlas = &Renderer->FontAtlas;
    
    ResultText->Count = 0;
    if(Parent) StartP += GetPosition(Parent);
    ResultText->CurrentP = {};//StartP;
    r32 DepthOffset = ZValue;
    
    ResultText->Base = CreateRenderBitmap(Renderer, V2(0), DepthOffset, Parent, 0); // GLID can be 0, never used!
    SetPosition(ResultText->Base, StartP);
    ResultText->Base->ID->Type = renderType_Text;
    ResultText->Base->ID->Text = ResultText;
    Parent = ResultText->Base;
    
    // Calculate baseline offset positions
    font_group *FontGroup    = GetFontGroup(GS, Atlas, (u32)'o');
    u32 FontGroupStart       = FontGroup->CodepointRange.First;
    u32 FontGroupEnd         = FontGroup->CodepointRange.Last;
    
    v2 TP = StartP;
    stbtt_aligned_quad TestQ;
    stbtt_GetPackedQuad(FontGroup->FontSizes[FontSize].CharData, FontGroup->BitmapWidth, FontGroup->BitmapHeight, 0, &TP.x, &TP.y, &TestQ, 0);
    r32 NewBaseline = TestQ.y0;
    r32 OldBaseline = TestQ.y1;
    
    
    v2 BaseP = {};
    v2 TextHeight = {MAX_REAL32, MIN_REAL32};
    For(Text->Pos)
    {
        u32 NextChar = 0;
        u32 Advance  = GetUTF8Decimal(Text->S+It, &NextChar)-1;
        
        // If the last codepoint was not in the same group we
        // get the correct one.
        if(NextChar < FontGroupStart || NextChar >= FontGroupEnd)
        {
            FontGroup = GetFontGroup(GS, Atlas, NextChar);
            if(!FontGroup) 
            {
                // If we did not get a font group, we cannot render the codepoint.
                // To 'fail gracefully' we load Basic Latin and render the []/<?>.
                FontGroup = GetFontGroup(GS, Atlas, (u32)'o');
                NextChar = 0; // This is the <?> glyph
            }
            FontGroupStart = FontGroup->CodepointRange.First;
            FontGroupEnd   = FontGroup->CodepointRange.Last;
            //DebugLog(255, "Switch to %s - %i\n", FontGroup->UnicodeGroup->Name.S, FontGroup->GLID);
        }
        
        if(NextChar == 10) // On '\n' we insert a newline
        {
            ResultText->CurrentP = V2(0/*StartP.x*/, ResultText->CurrentP.y + (OldBaseline-NewBaseline)*2);
            continue;
        }
        u32 CharDataID = NextChar - FontGroupStart; // Map codepoint into array range.
        
        stbtt_aligned_quad A;
        stbtt_GetPackedQuad(FontGroup->FontSizes[FontSize].CharData, FontGroup->BitmapWidth, FontGroup->BitmapHeight, 
                            CharDataID, &ResultText->CurrentP.x, &ResultText->CurrentP.y, &A, 0);
        
        rect Rect = {{A.x0, A.y0}, {A.x1, A.y1}};
        rect_pe RectPE = RectToRectPE(Rect);
        ResultText->RenderEntries[ResultText->Count] = CreateFontEntry(RectPE.Extends, DepthOffset, 
                                                                       FontGroup->GLID, Color, Parent);
        render_entry *Entry  = ResultText->RenderEntries+ResultText->Count++;
        entry_id EntryID = {Entry};
        // Also adding user controlled height offset
        SetLocalPosition(&EntryID, GetCenter(Rect) - V2(0, (r32)Atlas->HeightOffset*(FontSize+1)));
        Entry->TextCodepoint = NextChar;
        
        r32 DistToBaseline = GetRect(&EntryID).Max.y - OldBaseline;
        r32 BaselineOffset = GetRect(&EntryID).Min.y - NewBaseline;
        Translate(&EntryID, V2(0, -(BaselineOffset + DistToBaseline)));
        
        Entry->TexCoords[0] = {A.s0, A.t1};
        Entry->TexCoords[1] = {A.s0, A.t0};
        Entry->TexCoords[2] = {A.s1, A.t0};
        Entry->TexCoords[3] = {A.s1, A.t1};
        
        if(TextHeight.E[0] > A.y0) TextHeight.E[0] = A.y0;
        if(TextHeight.E[1] < A.y1) TextHeight.E[1] = A.y1;
        
        DepthOffset -= 0.000001f;
        
        It += Advance;
    }
    ResultText->Height = TextHeight;
}

inline r32 
FontSizeToPixel(font_size FontSize)
{
    switch(FontSize)
    {
        case font_Small:  return 24.0f;
        case font_Medium: return 50.0f;
        case font_Big:    return 75.0f;
        InvalidDefaultCase;
    }
    return 0.0f;
}

inline b32
IsPlaneOne(u8 Char)
{
    u8 Mask = 1<<7; // Check for 0b0xxxxxxx
    b32 Result = ((Char & Mask) == 0);
    return Result;
}

inline b32
IsPlaneTwo(u8 Char)
{
    u8 Mask = 1<<7 | 1<<6; // Check for 0b110xxxxx
    b32 Result = ((Char & Mask) == Mask) && ((Char & (1<<5)) == 0);
    return Result;
}

inline b32
IsPlaneThree(u8 Char)
{
    u8 Mask = 1<<7 | 1<<6 | 1<<5; // Check for 0b1110xxxx
    b32 Result = ((Char & Mask) == Mask) && ((Char & (1<<4)) == 0);
    return Result;
}

inline b32
IsPlaneFour(u8 Char)
{
    u8 Mask = 1<<7 | 1<<6 | 1<<5 | 1<<4; // Check for 0b11110xxx
    b32 Result = ((Char & Mask) == Mask) && ((Char & (1<<3)) == 0);
    return Result;
}

inline b32
IsValidUTF8SecondOrderByte(u8 Char)
{
    u8 Mask = 1<<7; // Check for 0b10xxxxxx
    b32 Result = ((Char & Mask) == Mask) && ((Char & (1<<6)) == 0);
    return Result;
}

internal u8
GetUTF8Decimal(u8 *S, u32 *Utf8Value)
{
    u8 Result = 0;
    
    if(IsPlaneOne(*S))
    {
        *Utf8Value = *S;
        Result = 1;
    }
    else if(IsPlaneTwo(*S))
    {
        if(IsValidUTF8SecondOrderByte(S[1]))
        {
            *Utf8Value = ((S[0] & 0b00011111)<<6) | (S[1] & 0b00111111);
            Result = 2;
        }
        else *Utf8Value = 0;
    }
    else if(IsPlaneThree(*S))
    {
        if(IsValidUTF8SecondOrderByte(S[1]) && 
           IsValidUTF8SecondOrderByte(S[2]))
        {
            *Utf8Value = ((S[0] & 0b00001111)<<12) | ((S[1] & 0b00111111)<<6) | (S[2] & 0b00111111);
            Result = 3;
        }
        else *Utf8Value = 0;
    }
    else if(IsPlaneFour(*S))
    {
        if(IsValidUTF8SecondOrderByte(S[1]) && 
           IsValidUTF8SecondOrderByte(S[2]) && 
           IsValidUTF8SecondOrderByte(S[3]))
        {
            u8 Mask = 0b00111111;
            *Utf8Value = ((S[0] & 0b00000111)<<18) | ((S[1] & Mask)<<12) | ((S[2] & Mask)<<6) | (S[3] & Mask);
            Result = 4;
        }
        else *Utf8Value = 0;
    }
    
    Assert(Result !=0);
    return Result;
}

internal b32
FindAndLoadFontWithUnicodeCodepoint(arena_allocator *ScratchArena, raw_font *SearchFont)
{
    string_c FolderPathStar = NewStringCompound(ScratchArena, 255);
    AppendStringCompoundToCompound(&FolderPathStar, SearchFont->FolderPath);
    AppendStringToCompound(&FolderPathStar, (u8 *)"\\*");
    
    string_w WideFolderPath = {};
    ConvertString8To16(ScratchArena, &FolderPathStar, &WideFolderPath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WideFolderPath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    
    b32 HasNextFile = true;
    string_c FileType = NewStringCompound(ScratchArena, 16);
    b32 FoundMatchingFont = false;
    while(HasNextFile && !FoundMatchingFont)
    {
        string_c FileName = {};
        ConvertString16To8(ScratchArena, FileData.cFileName, &FileName);
        
        if     (CompareStringAndCompound(&FileName, (u8 *)".") ) {} // Do nothing
        else if(CompareStringAndCompound(&FileName, (u8 *)"..")) {} // Do nothing
        else
        {
            if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {} // Do nothing
            else
            {
                i32 LastDot = FindLastOccurrenceOfCharInStringCompound(&FileName, '.');
                if(LastDot > 0)
                {
                    PasteStringCompoundIntoCompound(&FileType, 0, &FileName, LastDot+1, FileName.Pos-(LastDot+1));
                    if(CompareStringAndCompound(&FileType, (u8 *)"ttf"))
                    {
                        string_c FontPath = NewStringCompound(ScratchArena, FileName.Pos + SearchFont->FolderPath->Pos + 1);
                        string_c Slash    = NewStaticStringCompound("\\");
                        ConcatStringCompounds(4, &FontPath, SearchFont->FolderPath, &Slash, &FileName);
                        
                        read_file_result FontData = {};
                        if(ReadEntireFile(ScratchArena, &FontData, FontPath.S))
                        {
                            if(IsCodepointInFont(FontData.Data, SearchFont->Codepoint))
                            {
                                FoundMatchingFont = true;
                                
                                if(SearchFont->Name.Length == 0) 
                                    SearchFont->Name = NewStringCompound(ScratchArena, FileName.Pos);
                                Assert(SearchFont->Name.Length >= FileName.Pos);
                                AppendStringCompoundToCompound(&SearchFont->Name, &FileName);
                                
                                SearchFont->Data = FontData;
                            }
                            else
                            {
                                FreeFileMemory(ScratchArena, FontData);
                            }
                        }
                        DeleteStringCompound(ScratchArena, &FontPath);
                    }
                    ResetStringCompound(FileType);
                }
            }
        }
        DeleteStringCompound(ScratchArena, &FileName);
        HasNextFile = FindNextFileW(FileHandle, &FileData);
    } 
    
    return FoundMatchingFont;
}

// *****************************
// Not used during runtime: ****
// *****************************

internal void
CreateUnicodeGroupInfo()
{
    For(UNICODE_GROUP_COUNT)
    {
        UnicodeGroupList.G[It].CodepointRange.Count = UnicodeGroupList.G[It].CodepointRange.Last - UnicodeGroupList.G[It].CodepointRange.First + 1;
        //DebugLog(255, "%i\t\t - %s\n", UnicodeGroupList.G[It].CodepointCount, UnicodeGroupList.G[It].Name.S);
    }
}

internal b32
FindFontWithUnicodeCodepoint(arena_allocator *ScratchArena, string_c *FolderPath, u32 Codepoint, string_c *FoundFont)
{
    string_c FolderPathStar = NewStringCompound(ScratchArena, 255);
    AppendStringCompoundToCompound(&FolderPathStar, FolderPath);
    AppendStringToCompound(&FolderPathStar, (u8 *)"\\*");
    
    string_w WideFolderPath = {};
    ConvertString8To16(ScratchArena, &FolderPathStar, &WideFolderPath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WideFolderPath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    
    b32 HasNextFile = true;
    string_c FileType = NewStringCompound(ScratchArena, 16);
    b32 FoundMatchingFont = false;
    while(HasNextFile && !FoundMatchingFont)
    {
        string_c FileName = {};
        ConvertString16To8(ScratchArena, FileData.cFileName, &FileName);
        
        if     (CompareStringAndCompound(&FileName, (u8 *)".") ) {} // Do nothing
        else if(CompareStringAndCompound(&FileName, (u8 *)"..")) {} // Do nothing
        else
        {
            if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {} // Do nothing
            else
            {
                i32 LastDot = FindLastOccurrenceOfCharInStringCompound(&FileName, '.');
                if(LastDot > 0)
                {
                    PasteStringCompoundIntoCompound(&FileType, 0, &FileName, LastDot+1, FileName.Pos-(LastDot+1));
                    u8 *MP3Extension = (u8 *)"ttf";
                    if(CompareStringAndCompound(&FileType, MP3Extension))
                    {
                        string_c FontPath = NewStringCompound(ScratchArena, FileName.Pos + FolderPath->Pos + 1);
                        string_c Slash    = NewStaticStringCompound("\\");
                        ConcatStringCompounds(4, &FontPath, FolderPath, &Slash, &FileName);
                        
                        read_file_result FontData = {};
                        if(ReadEntireFile(ScratchArena, &FontData, FontPath.S))
                        {
                            stbtt_fontinfo FontInfo;
                            if(stbtt_InitFont(&FontInfo, FontData.Data, 0))
                            {
                                if(stbtt_FindGlyphIndex(&FontInfo, Codepoint))
                                {
                                    FoundMatchingFont = true;
                                    
                                    if(FoundFont->Length == 0) *FoundFont = NewStringCompound(ScratchArena, 100);
                                    Assert(FoundFont->Length >= FileName.Pos);
                                    AppendStringCompoundToCompound(FoundFont, &FileName);
                                    //AppendStringToCompound(FoundFont, (u8 *)"\n");
                                }
                            }
                            FreeFileMemory(ScratchArena, FontData);
                        }
                        DeleteStringCompound(ScratchArena, &FontPath);
                    }
                    ResetStringCompound(FileType);
                }
            }
        }
        DeleteStringCompound(ScratchArena, &FileName);
        HasNextFile = FindNextFileW(FileHandle, &FileData);
    } 
    
    return FoundMatchingFont;
}

internal void
PrintAsList2(arena_allocator *Arena, unicode_group *Group, string_c FontName)
{
    string_c Out = NewStringCompound(Arena, 500);
    AppendStringToCompound(&Out, (u8 *)"{ \"");
    AppendStringCompoundToCompound(&Out, &Group->Name);
    AppendStringToCompound(&Out, (u8 *)"\", %i, %i, ");
    char A[500];
    sprintf_s(A, (const char *)Out.S, Group->Name.Pos, Group->Name.Pos);
    CopyStringToCompound(&Out, (u8 *)A, 0u);
    
    
    while(Out.Pos < 52) AppendCharToCompound(&Out, ' ');
    AppendStringToCompound(&Out, (u8 *)"0x%X, 0x%X, %i, ");
    
    char B[500];
    sprintf_s(B, (const char *)Out.S, Group->CodepointRange.First, Group->CodepointRange.Last, Group->CodepointRange.Count);
    CopyStringToCompound(&Out, (u8 *)B, 0u);
    
    while(Out.Pos < (52+27)) AppendCharToCompound(&Out, ' ');
    AppendStringToCompound(&Out, (u8 *)"\"");
    AppendStringCompoundToCompound(&Out, &FontName);
    AppendStringToCompound(&Out, (u8 *)"\", %i, %i },\n");
    
    char C[500];
    sprintf_s(C, (const char *)Out.S, FontName.Pos, FontName.Pos);
    CopyStringToCompound(&Out, (u8 *)C, 0u);
    
    DebugLog(500, "%s", Out.S);
    if(!AppendToFile(Arena, (u8 *)"UnicodeGroupList.font", Out.Pos, Out.S))
    {
        if(!WriteEntireFile(Arena, (u8 *)"UnicodeGroupList.font", Out.Pos, Out.S))
        {
            Assert(false);
        }
    }
    
    DeleteStringCompound(Arena, &Out);
}

internal void
PrintAsList(arena_allocator *Arena, unicode_group *Group, string_c FontName)
{
    string_c Out = NewStringCompound(Arena, 500);
    AppendStringToCompound(&Out, (u8 *)"{ {(u8 *)\"");
    AppendStringCompoundToCompound(&Out, &Group->Name);
    AppendStringToCompound(&Out, (u8 *)"\", %i, %i}, ");
    char A[500];
    sprintf_s(A, (const char *)Out.S, Group->Name.Pos, Group->Name.Pos);
    CopyStringToCompound(&Out, (u8 *)A, 0u);
    
    
    while(Out.Pos < 57) AppendCharToCompound(&Out, ' ');
    AppendStringToCompound(&Out, (u8 *)"{0x%X, 0x%X, %i}, ");
    
    char B[500];
    sprintf_s(B, (const char *)Out.S, Group->CodepointRange.First, Group->CodepointRange.Last, Group->CodepointRange.Count);
    CopyStringToCompound(&Out, (u8 *)B, 0u);
    
    while(Out.Pos < (57+27)) AppendCharToCompound(&Out, ' ');
    AppendStringToCompound(&Out, (u8 *)"{(u8 *)\"");
    AppendStringCompoundToCompound(&Out, &FontName);
    AppendStringToCompound(&Out, (u8 *)"\", %i, %i} },\n");
    
    char C[500];
    sprintf_s(C, (const char *)Out.S, FontName.Pos, FontName.Pos);
    CopyStringToCompound(&Out, (u8 *)C, 0u);
    
    DebugLog(500, "%s", Out.S);
    if(!AppendToFile(Arena, (u8 *)"UnicodeGroupList.font", Out.Pos, Out.S))
    {
        if(!WriteEntireFile(Arena, (u8 *)"UnicodeGroupList.font", Out.Pos, Out.S))
        {
            Assert(false);
        }
    }
    
    DeleteStringCompound(Arena, &Out);
}

internal void
WriteInFile(arena_allocator *Arena, unicode_group *Group, string_c FontName)
{
    string_c S = NewStringCompound(Arena, FontName.Pos+Group->Name.Pos+50);
    AppendStringToCompound(&S, (u8 *)"************ ");
    AppendStringCompoundToCompound(&S, &Group->Name);
    AppendStringToCompound(&S, (u8 *)"\n");
    AppendStringCompoundToCompound(&S, &FontName);
    AppendStringToCompound(&S, (u8 *)"\n");
    AppendStringToCompound(&S, (u8 *)"---------------------------");
    if(!AppendToFile(Arena, (u8 *)"FontsForEachBlock.font", S.Pos, S.S))
    {
        if(!WriteEntireFile(Arena, (u8 *)"FontsForEachBlock.font", S.Pos, S.S))
        {
            Assert(false);
        }
    }
}

internal void
FindAndPrintFontNameForEveryUnicodeGroup(arena_allocator *Arena, string_c Path)
{
    For(UNICODE_GROUP_COUNT)
    {
        unicode_group *Group = UnicodeGroupList.G + It;
        u32 Codepoint = Group->CodepointRange.First + (Group->CodepointRange.Count/2);
        
        string_c FontName = {};
        if(!FindFontWithUnicodeCodepoint(Arena, &Path, Codepoint, &FontName))
        {
            // If we did not find a font with the first codepoint, 
            // we try with another to be sure that we did not hit
            // a unused codepoint (which exist sometimes). We only
            // try one more time, with the first as testing suggested
            // anything else does not result in more found fonts.
            Codepoint = Group->CodepointRange.First;
            FindFontWithUnicodeCodepoint(Arena, &Path, Codepoint, &FontName); 
        }
        //PrintAsList(Arena, Group, FontName);
        //WriteInFile(Arena, Group, FontName);
        PrintAsList2(Arena, Group, FontName);
        ResetMemoryArena(Arena);
    }
}
