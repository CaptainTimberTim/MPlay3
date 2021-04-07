#include "Font_TD.h"

inline font_atlas
NewFontAtlas(r32 *FontSizes, u32 SizesCount, i32 HeightOffset)
{
    font_atlas Result = {FontSizes, SizesCount, HeightOffset, 5, 0, NULL};
    return Result;
}

internal unicode_group *
GetUnicodeGroup(u32 SampleCodePoint)
{
    unicode_group *Result = 0;
    For(ArrayCount(UnicodeGroups))
    {
        if(SampleCodePoint >= UnicodeGroups[It].FirstCodepoint &&
           SampleCodePoint <  UnicodeGroups[It].LastCodepoint) 
        {
            Result = UnicodeGroups + It;
            break;
        }
    }
    return Result;
}

internal void
LoadFonts(arena_allocator *FixArena, arena_allocator *ScratchArena, font_atlas *Atlas, u8 *RawFontData, u32 *CodepointsFromGroup, u32 CodepointCount)
{
    Assert(Atlas->MaxCount > 0);
    if(Atlas->Count == 0)
    {
        Atlas->FontGroups = AllocateArray(FixArena, Atlas->MaxCount, font_group);
    }
    else if(Atlas->Count >= Atlas->MaxCount) // TODO:: Test this
    {
        Atlas->FontGroups = ReallocateArray(FixArena, Atlas->FontGroups, Atlas->MaxCount, Atlas->MaxCount*2, font_group);
        Atlas->MaxCount *= 2;
    }
    
    For(CodepointCount, Point)
    {
        u32 SampleCodePoint  = CodepointsFromGroup[PointIt];
        unicode_group *Group = GetUnicodeGroup(SampleCodePoint);
        b32 Found            = false;
        For(Atlas->Count, Exists)
        {
            if(Atlas->FontGroups[ExistsIt].UnicodeGroup == Group)
            {
                Found = true;
                break;
            }
        }
        if(Found) continue; // This font group is already loaded. Skip to next one.
        
        if(Group->CodepointCount > 500)
        {
            // TODO:: If we get here, we only load a range of them?
            // I have to think about it...
            Assert(false);
        }
        
        
        font_group *NewFontGroup = Atlas->FontGroups + Atlas->Count++;
        NewFontGroup->UnicodeGroup = Group;
        
        u32 Width  = Group->CodepointCount*10;
        u32 Height = Group->CodepointCount*5;
        NewFontGroup->BitmapWidth  = Width;
        NewFontGroup->BitmapHeight = Height;
        
        u8 *AlphaMap         = AllocateMemory(ScratchArena, Width*Height);
        loaded_bitmap Bitmap = {true, Width, Height, (u32 *)AlphaMap, colorFormat_Alpha, Width*sizeof(u32)};
        
        stbtt_pack_context PackContext;
        stbtt_PackBegin(&PackContext, AlphaMap, Width, Height, 0, 1, NULL);
        
        NewFontGroup->FontSizes = AllocateArray(FixArena, Atlas->SizesCount, font_data);
        NewFontGroup->Count     = Atlas->SizesCount;
        For(Atlas->SizesCount)
        {
            NewFontGroup->FontSizes[It].Size     = Atlas->FontSizes[It];
            NewFontGroup->FontSizes[It].CharData = AllocateArray(FixArena, Group->CodepointCount, stbtt_packedchar);
            
            stbtt_PackSetOversampling(&PackContext, 3, 1);
            stbtt_PackFontRange(&PackContext, RawFontData, 0, Atlas->FontSizes[It], Group->FirstCodepoint, Group->CodepointCount, NewFontGroup->FontSizes[It].CharData);
        }
        
        stbtt_PackEnd(&PackContext);
        
        NewFontGroup->GLID = CreateGLTexture(Bitmap, true);
        
        FreeMemory(ScratchArena, AlphaMap);
    }
}

internal font_group *
GetFontGroup(font_atlas *Atlas, u32 Codepoint)
{
    font_group *Result = 0;
    
    For(Atlas->Count)
    {
        if(Codepoint >= Atlas->FontGroups[It].UnicodeGroup->FirstCodepoint &&
           Codepoint <  Atlas->FontGroups[It].UnicodeGroup->LastCodepoint)
        {
            Result = Atlas->FontGroups + It;
            break;
        }
    }
    Assert(Result);
    
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
RenderText(renderer *Renderer, arena_allocator *Arena, font_size FontSize, string_c *Text,
           v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent, v2 StartP)
{
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
    
    if(ResultText->MaxCount == 0)
    {
        ResultText->MaxCount         = Max(CHARACTERS_PER_TEXT_INFO, Text->Pos+1);
        ResultText->RenderEntries    = AllocateArray(Arena, ResultText->MaxCount, render_entry);
    }
    // TODO:: Maybe when this happens increase the size? With current allocater system
    // this would not work very well. But once it changed... maybe.
    Assert(Text->Pos < ResultText->MaxCount); 
    
    // Calculate baseline offset positions
    u32 FirstCodepoint = 0;
    if(Text->Pos > 0) GetUTF8Decimal(Text->S, &FirstCodepoint);
    else              GetUTF8Decimal((u8 *)"o", &FirstCodepoint);
    font_group *FontGroup = GetFontGroup(Atlas, FirstCodepoint);
    u32 PrevGroupStart    = FontGroup->UnicodeGroup->FirstCodepoint;
    u32 PrevGroupEnd      = FontGroup->UnicodeGroup->LastCodepoint;
    
    v2 TP = StartP;
    stbtt_aligned_quad TestQ;
    stbtt_GetPackedQuad(FontGroup->FontSizes[FontSize].CharData, FontGroup->BitmapWidth, FontGroup->BitmapHeight, 
                        0, &TP.x, &TP.y, &TestQ, 0);
    
    r32 NewBaseline = TestQ.y0;
    r32 OldBaseline = TestQ.y1;
    
    v2 BaseP = {};
    v2 TextHeight = {MAX_REAL32, MIN_REAL32};
    For(Text->Pos)
    {
        u32 NextChar = 0;
        It          += GetUTF8Decimal(Text->S+It, &NextChar)-1;
        
        // If the last codepoint was not in the same group we
        // get the correct one.
        if(NextChar < PrevGroupStart || NextChar >= PrevGroupEnd)
        {
            FontGroup      = GetFontGroup(Atlas, NextChar);
            PrevGroupStart = FontGroup->UnicodeGroup->FirstCodepoint;
            PrevGroupEnd   = FontGroup->UnicodeGroup->LastCodepoint;
            //DebugLog(255, "Switch to %s - %i\n", FontGroup->UnicodeGroup->Name.S, FontGroup->GLID);
        }
        
        if(NextChar == 10) // On '\n' we insert a newline
        {
            ResultText->CurrentP = V2(0/*StartP.x*/, ResultText->CurrentP.y + (OldBaseline-NewBaseline)*2);
            continue;
        }
        u32 CharDataID = NextChar - PrevGroupStart; // Map codepoint into array range.
        
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
        SetLocalPosition(&EntryID, GetCenter(Rect) - V2(0, (r32)Atlas->HeightOffset));
        
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


// *****************************
// Not used during runtime: ****
// *****************************

internal void
CreateUnicodeGroupInfo()
{
    For(ArrayCount(UnicodeGroups))
    {
        UnicodeGroups[It].CodepointCount = UnicodeGroups[It].LastCodepoint - UnicodeGroups[It].FirstCodepoint + 1;
        //DebugLog(255, "%i\t\t - %s\n", UnicodeGroups[It].CodepointCount, UnicodeGroups[It].Name.S);
    }
}

internal b32
FindFontWithUnicodeCodepoint(arena_allocator *ScratchArena, string_c *FolderPath, u32 Codepoint, string_c *FoundFont)
{
    b32 Result = false;
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
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        b32 HasNextFile = true;
        string_c FileType = NewStringCompound(ScratchArena, 16);
        
        b32 FoundMatchingFont = false;
        while(HasNextFile && !FoundMatchingFont)
        {
            string_c FileName = {};
            ConvertString16To8(ScratchArena, FileData.cFileName, &FileName);
            
            if     (CompareStringAndCompound(&FileName, (u8 *)".") ) {}
            else if(CompareStringAndCompound(&FileName, (u8 *)"..")) {}
            else
            {
                if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {}
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
                                        
                                        if(FoundFont->Length == 0)
                                            *FoundFont = NewStringCompound(ScratchArena, FileName.Pos);
                                        Assert(FoundFont->Length >= FileName.Pos);
                                        FoundFont->Pos = 0;
                                        AppendStringCompoundToCompound(FoundFont, &FileName);
                                    }
                                }
                                FreeFileMemory(ScratchArena, FontData.Data);
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
        Result = FoundMatchingFont;
    }
    return Result;
}

internal void
FindAndPrintFontNameForEveryUnicodeGroup(arena_allocator *Arena, string_c Path)
{
    For(ArrayCount(UnicodeGroups))
    {
        unicode_group *Group = UnicodeGroups + It;
        u32 Codepoint = Group->FirstCodepoint + (Group->CodepointCount/2);
        
        string_c FontName = {};
        if(!FindFontWithUnicodeCodepoint(Arena, &Path, Codepoint, &FontName))
        {
            // If we did not find a font with the first codepoint, 
            // we try with another to be sure that we did not hit
            // a unused codepoint (which exist sometimes). We only
            // try one more time, with the first as testing suggested
            // anything else does not result in more found fonts.
            Codepoint = Group->FirstCodepoint;
            FindFontWithUnicodeCodepoint(Arena, &Path, Codepoint, &FontName); 
        }
        
        string_c Out = NewStringCompound(Arena, 500);
        AppendStringToCompound(&Out, (u8 *)"{ NewSSC(\"");
        AppendStringCompoundToCompound(&Out, &Group->Name);
        AppendStringToCompound(&Out, (u8 *)"\"), ");
        while(Out.Pos < 53) AppendCharToCompound(&Out, ' ');
        AppendStringToCompound(&Out, (u8 *)"0x%X, 0x%X, %i, ");
        
        char B[500];
        sprintf_s(B, (const char *)Out.S, Group->FirstCodepoint, Group->LastCodepoint, Group->CodepointCount);
        CopyStringToCompound(&Out, (u8 *)B, 0u);
        
        while(Out.Pos < (53+27)) AppendCharToCompound(&Out, ' ');
        AppendStringToCompound(&Out, (u8 *)"NewSSC(\"");
        AppendStringCompoundToCompound(&Out, &FontName);
        AppendStringToCompound(&Out, (u8 *)"\") },\n");
        
        DebugLog(500, "%s", Out.S);
        
        DeleteStringCompound(Arena, &Out);
    }
}