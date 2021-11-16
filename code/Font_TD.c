#include "Font_TD.h"
inline r32 SlotHeight(display_column *DisplayColumn);
inline r32 GetSongButtonYOffset(layout_definition *Layout);
inline r32 GetBottomPanelHeight(layout_definition *Layout);
internal void ProcessWindowResizeForDisplayColumn(renderer *Renderer, music_info *MusicInfo, display_column *DisplayColumn);
inline b32 GetDefaultFontDir(arena_allocator *Arena, string_c *Path);

inline font_atlas
NewFontAtlas(serialization_settings *Settings, font_sizes FontSizes)
{
    PrepareUnicodeGroupList();
    font_atlas Result = {FontSizes, Settings->FontHeightOffset, 5, 0, NULL, Settings->CachedFontNames};
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

internal font_atlas
InitializeFont(game_state *GS)
{
    GetDefaultFontDir(&GS->FixArena, &GS->FontPath);
    font_sizes FontSizes = {{
            {font_Small,  GS->Settings.SmallFontSize}, 
            {font_Medium, GS->Settings.MediumFontSize}, 
            {font_Big,    GS->Layout.FontSizeBig}/*Non changeable for now.*/}, 3
    };
    GS->Renderer.FontSizes = FontSizes;
    
    u32 GroupCodepoints[] = {0x00};//, 0x80};
    font_atlas FontAtlas = NewFontAtlas(&GS->Settings, FontSizes);
    //FindAndPrintFontNameForEveryUnicodeGroup(&GS->ScratchArena, GS->FontPath);
    LoadFonts(&GS->FixArena, &GS->ScratchArena, &FontAtlas,
              (u8 *)GetUsedFontData(GS).Data, GroupCodepoints, ArrayCount(GroupCodepoints));
    
    return FontAtlas;
}

internal void
CreateAndWriteFontGroupTexture(arena_allocator *FixArena, arena_allocator *ScratchArena, 
                               font_group *FontGroup, font_sizes FontSizes, u8 *RawFontData)
{
    // First allocate the charData we will need.
    // then try to pack it. If that fails, try packing with a
    // larger bitmap, until it fits.
    For(FontSizes.Count)
    {
        FontGroup->FontDataForEachSize[It].Size     = FontSizes.Sizes[It].Size;
        FontGroup->FontDataForEachSize[It].CharData = AllocateArray(FixArena, FontGroup->CodepointRange.Count, 
                                                                    stbtt_packedchar);
    }
    
    r32 SizeMultiplier = 2.5f;
    loaded_bitmap Bitmap = {};
    while(true)
    {
        u32 TotalArea = 0;
        For(FontSizes.Count, Size)
        {
            // Summing all areas of each codepoint we want to load. 
            // For this we assume that the codepoint is a square,
            // which seems to work. The only exception is for very
            // few codepoints (like 1) or some special glyphs like 
            // emoticons, thats why we multiply by SizeMultiplier. 
            TotalArea += Square((i32)(FontSizes.Sizes[SizeIt].Size*SizeMultiplier))*(FontGroup->CodepointRange.Count);
        }
        u32 Width  = (u32)Sqrt(TotalArea); // First calc the side for square with correct total area,
        Width      = (Width/8)*8;          // then fit the width to a properly byte aligned length.
        u32 Height = TotalArea/Width;      // Finally, calc the length of the height.
        //DebugLog(250, "The claculated bitmap area for is %i, the sides are %i/%i\n", TotalArea, Width, Height);
        
        FontGroup->BitmapWidth  = Width;
        FontGroup->BitmapHeight = Height;
        
        u8 *AlphaMap = AllocateMemory(ScratchArena, Width*Height);
        Bitmap = {true, Width, Height, (u32 *)AlphaMap, colorFormat_Alpha, Width};
        
        stbtt_pack_context PackContext;
        stbtt_PackBegin(&PackContext, AlphaMap, Width, Height, 0, 1, NULL);
        
        i32 PackResult = 0;
        For(FontSizes.Count)
        {
            stbtt_PackSetOversampling(&PackContext, 3, 1);
            PackResult = stbtt_PackFontRange(&PackContext, RawFontData, 0, FontSizes.Sizes[It].Size, 
                                             FontGroup->CodepointRange.First, FontGroup->CodepointRange.Count, FontGroup->FontDataForEachSize[It].CharData);
            if(PackResult == 0) break; // Most likely the given bitmap is too small.
        }
        stbtt_PackEnd(&PackContext);
        if(PackResult == 0) 
        {
            FreeMemory(ScratchArena, AlphaMap);
            SizeMultiplier += 0.5f;
        }
        else break; // If we have a PackResult, everything worked out and we can stop.
    }
    
    FontGroup->GLID = CreateGLTexture(Bitmap, true); // After stbtt_PackEnd as it is written before that.
    FreeMemory(ScratchArena, Bitmap.Pixels);
}

internal void
ChangeFontSizes(game_state *GS, font_sizes NewSizes)
{
    font_atlas *Atlas = &GS->Renderer.FontAtlas;
    
    // First change the existing font groups and their data.
    Atlas->FontSizes = NewSizes;
    u32 GroupCount   = Atlas->Count;
    For(GroupCount, Group)
    {
        font_group *FontGroup = Atlas->FontGroups + GroupIt;
        
        DeleteGLTexture(FontGroup->GLID);
        For(ArrayCount(FontGroup->FontDataForEachSize))
        {
            FreeMemory(&GS->FixArena, FontGroup->FontDataForEachSize[It].CharData);
            FontGroup->FontDataForEachSize[It].CharData = NULL;
            FontGroup->FontDataForEachSize[It].Size = 0;
        }
        
#if 1
        FontGroup->GLID           = 0;
        FontGroup->BitmapWidth    = 0;
        FontGroup->BitmapHeight   = 0;
        FontGroup-> UnicodeGroup  = NULL;
        FontGroup->CodepointRange = {};
        Atlas->Count              = 0;
#else
        // This recreates all font groups that already existed. The big
        // disadvantage is that when there are many existing font groups
        // already, it has to do a lot of work. Even though these groups
        // may not be used at this moment. Much cleaner to just delete
        // all groups and let the 'normal' path recreate them only when 
        // needed. Which is the code right above this.
        read_file_result Font = {};
        b32 FontLoaded = false;
        if(FontGroup->UsedFontPath.Pos == 0) Font = GetUsedFontData(GS);
        else
        {
            if(!ReadEntireFile(&GS->ScratchArena, &Font, FontGroup->UsedFontPath.S))
            {
                // TODO:: What should happen if previously used font is not accessible anymore?
                Assert(false);
            }
            FontLoaded = true;
        }
        CreateAndWriteFontGroupTexture(&GS->FixArena, &GS->ScratchArena, FontGroup, NewSizes, Font.Data);
        
        if(FontLoaded) FreeFileMemory(&GS->ScratchArena, Font);
#endif
        
    }
    
    // Secondly, recreate all existing render entries of type renderType_Text.
    render_entry_list *RenderList = &GS->Renderer.RenderEntryList;
#if 1
    u32 EntryCount = RenderList->EntryCount;
    For(EntryCount, Entry)
    {
        render_entry *RenderEntry = RenderList->Entries + EntryIt;
        if(RenderEntry->Type == renderType_Text && RenderEntry->Vertice[0].z > -2.0f)
        {
            render_text *TextEntry = RenderEntry->Text;
            
            string_c FontText = NewStringCompound(&GS->ScratchArena, TextEntry->Text.Pos);
            CopyIntoCompound(&FontText, &TextEntry->Text);
            
            font_size_id FontSize = TextEntry->FontSize;
            r32 Depth             = RenderEntry->Vertice[0].z;
            v3 *Color             = TextEntry->RenderEntries[0].Color;
            entry_id *Parent      = RenderEntry->Parent;
            v2 StartP             = GetLocalPosition(RenderEntry->ID);
            b32 Rendering         = RenderEntry->Render;
            entry_id *Scissor     = RenderEntry->Scissor;
            r32 Transparency      = TextEntry->RenderEntries[0].Transparency;
            
            RemoveRenderText(&GS->Renderer, TextEntry);
            RenderText(GS, FontSize, &FontText, Color, TextEntry, Depth, Parent, StartP);
            SetActive(TextEntry, Rendering);
            SetScissor(TextEntry, Scissor);
            SetTransparency(TextEntry, Transparency);
        }
    }
#endif
    music_display_info *DisplayInfo = &GS->MusicInfo.DisplayInfo;
    
    // Thirdly, the bottom panel and everything that depends on it.
    r32 BottomPanelHeight = GetBottomPanelHeight(&GS->Layout);
    
    SetSize(DisplayInfo->EdgeBottom, V2(GetSize(DisplayInfo->EdgeBottom).x, BottomPanelHeight));
    SetPosition(DisplayInfo->EdgeBottom, V2(GetPosition(DisplayInfo->EdgeBottom).x, BottomPanelHeight*0.5f));
    UpdateOriginalTransform(&GS->Renderer.TransformList, DisplayInfo->EdgeBottom);
    
    r32 SidePanelHeight = HeightBetweenRects(DisplayInfo->EdgeTop, DisplayInfo->EdgeBottom);
    r32 SidePanelY      = CenterYBetweenRects(DisplayInfo->EdgeBottom, DisplayInfo->EdgeTop);
    
    SetSize(DisplayInfo->EdgeLeft, V2(GetSize(DisplayInfo->EdgeLeft).x, SidePanelHeight));
    SetPosition(DisplayInfo->EdgeLeft, V2(GetPosition(DisplayInfo->EdgeLeft).x, SidePanelY));
    UpdateOriginalTransform(&GS->Renderer.TransformList, DisplayInfo->EdgeLeft);
    
    SetSize(DisplayInfo->EdgeRight, V2(GetSize(DisplayInfo->EdgeRight).x, SidePanelHeight));
    SetPosition(DisplayInfo->EdgeRight, V2(GetPosition(DisplayInfo->EdgeRight).x, SidePanelY));
    UpdateOriginalTransform(&GS->Renderer.TransformList, DisplayInfo->EdgeRight);
    
    ProcessEdgeDragOnResize(&GS->Renderer, DisplayInfo);
    
    // Fourth, change the heights of all slots.
    For(5, Column)
    {
        display_column *Column = DisplayInfo->Columns[ColumnIt];
        
        r32 AnchorY = GetPosition(Column->TopBorder).y - 
            GetSize(Column->TopBorder).y/2.0f - SlotHeight(Column)/2.0f;
        SetPosition(Column->SlotBGAnchor, V2(0, AnchorY));
        UpdateOriginalPosition(&GS->Renderer.TransformList, Column->SlotBGAnchor);
        UpdateSlots(GS, Column);
        
        r32 HoriSliderY = BottomPanelHeight + GetSize(Column->SliderHorizontal.Background).y*0.5f;
        SetLocalPosition(&Column->SliderHorizontal, V2(GetLocalPosition(&Column->SliderHorizontal).x, HoriSliderY));
        
        entry_id *SearchRect = Column->BetweenSliderRect;
        r32 SearchRectY = BottomPanelHeight + GetSize(SearchRect).y*0.5f;
        SetPosition(SearchRect, V2(GetPosition(SearchRect).x, SearchRectY));
        
        ProcessWindowResizeForDisplayColumn(&GS->Renderer, &GS->MusicInfo, Column);
    }
    
    PerformScreenTransform(&GS->Renderer);
    
    SetNewPlayingSong(&GS->Renderer, &DisplayInfo->PlayingSongPanel, &GS->Layout, &GS->MusicInfo);
    
    UpdateSettings(GS);
    
    // Update song column buttons
    /*
    r32 FontHeight                  = GetFontSize(&GS->Renderer, font_Small).Size;
    display_column_song *SongColumn = &GS->MusicInfo.DisplayInfo.Song;
    v2 Size = V2(FontHeight*2);
    For(SongColumn->Base.Count)
    {
        SetSize(SongColumn->Play[It], Size);
        SetSize(SongColumn->Add[It], Size);
    }*/
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
LoadFonts(arena_allocator *FixArena, arena_allocator *ScratchArena, font_atlas *Atlas, u8 *RawFontData, /*string_c FontPath,*/ u32 *CodepointsFromGroup, u32 CodepointCount)
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
        
        //NewFontGroup->UsedFontPath = NewStringCompound(FixArena, 260);
        //CopyIntoCompound(&NewFontGroup->UsedFontPath, &FontPath);
        CreateAndWriteFontGroupTexture(FixArena, ScratchArena, NewFontGroup, Atlas->FontSizes, RawFontData);
        
        stbtt_fontinfo Font;
        stbtt_InitFont(&Font, RawFontData, 0);
        i32 Ascent, Descent, LineGap, RowGap;
        stbtt_GetFontVMetrics(&Font, &Ascent, &Descent, &LineGap);
        RowGap = Ascent + (Descent*-1) + LineGap;
        
        NewFontGroup->FontMetrics = {(r32)Ascent, (r32)Descent, (r32)LineGap, (r32)RowGap};
        
    }
}

internal b32
IsCodepointInFont(u8 *Data, u32 Codepoint)
{
    b32 Result = false;
    
    u32 FCount = stbtt_GetNumberOfFonts(Data);
    For(FCount)
    {
        i32 Offset = stbtt_GetFontOffsetForIndex(Data, It);
        
        stbtt_fontinfo FontInfo;
        if(stbtt_InitFont(&FontInfo, Data, Offset))
        {
            if(stbtt_FindGlyphIndex(&FontInfo, Codepoint))
            {
                Result = true;
                break;
            }
        }
    }
    
    return Result;
}

internal b32
AddMissingFontGroup(game_state *GS, font_atlas *Atlas, u32 Codepoint)
{
    b32 Result = false;
    
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
    if(!Result && Atlas->CachedFontNames->Count > 0)
    {
        For(Atlas->CachedFontNames->Count)
        {
            NewEmptyLocalString(FontPath, 260);
            NewLocalString(Slash, 2, "\\");
            ConcatStringCompounds(4, &FontPath, &GS->FontPath, &Slash, Atlas->CachedFontNames->Names+It);
            
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
        NewEmptyLocalString(UsedFontPath, 260);
        if(FindAndLoadFontWithUnicodeCodepoint(&GS->ScratchArena, &RawFont, &UsedFontPath))
        {
            LoadFonts(&GS->FixArena, &GS->ScratchArena, Atlas, RawFont.Data.Data, GroupCodepoints, ArrayCount(GroupCodepoints));
            
            if(Atlas->CachedFontNames->Count >= Atlas->CachedFontNames->MaxCount)
            {
                Atlas->CachedFontNames->MaxCount += 10;
                Atlas->CachedFontNames->Names = ReallocateArray(&GS->FixArena, Atlas->CachedFontNames->Names, 
                                                                Atlas->CachedFontNames->Count,
                                                                Atlas->CachedFontNames->MaxCount, string_c);
            }
            Atlas->CachedFontNames->Names[Atlas->CachedFontNames->Count] = NewStringCompound(&GS->FixArena, RawFont.Name.Pos);
            AppendStringCompoundToCompound(Atlas->CachedFontNames->Names+Atlas->CachedFontNames->Count, &RawFont.Name);
            ++Atlas->CachedFontNames->Count;
            
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
RenderText(game_state *GS, font_size_id FontSize, string_c *Text,
           v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent, v2 StartP)
{
    
    if(Parent)
    {
        v2 p = GetPosition(Parent) + StartP;
        if(p.x < 0 || p.y < 0)
            i32 g = 20;
    }
    
    // Create memory if this render_text was not used before
    // or is not big enough for the given text.
    if(ResultText->MaxCount == 0)
    {
        ResultText->MaxCount         = Max(CHARACTERS_PER_TEXT_INFO, Text->Pos+1);
        ResultText->RenderEntries    = AllocateArray(&GS->FixArena, ResultText->MaxCount, render_entry);
        ResultText->Text             = NewStringCompound(&GS->FixArena, ResultText->MaxCount);
    }
    else if (ResultText->MaxCount < Text->Pos)
    {
        DeleteStringCompound(&GS->FixArena, &ResultText->Text);
        ResultText->RenderEntries = ReallocateArray(&GS->FixArena, ResultText->RenderEntries, ResultText->MaxCount, 
                                                    Text->Pos, render_entry);
        ResultText->MaxCount      = Text->Pos;
        ResultText->Text          = NewStringCompound(&GS->FixArena, ResultText->MaxCount);
    }
    Assert(Text->Pos < ResultText->MaxCount); 
    
    CopyIntoCompound(&ResultText->Text, Text);
    ResultText->FontSize = FontSize;
    
    renderer *Renderer = &GS->Renderer;
    font_atlas *Atlas = &Renderer->FontAtlas;
    
    ResultText->Count = 0;
    if(Parent) StartP += GetPosition(Parent);
    ResultText->CurrentP = {};
    r32 DepthOffset = ZValue;
    
    //entry_id *TestBase = CreateRenderRect(Renderer, V2(5,5), DepthOffset-0.00001f, Color, Parent);
    //SetPosition(TestBase, StartP);
    
    ResultText->Base = CreateRenderBitmap(Renderer, V2(0), DepthOffset, Parent, 0); // GLID can be 0, never used!
    SetPosition(ResultText->Base, StartP);
    ResultText->Base->ID->Type = renderType_Text;
    ResultText->Base->ID->Text = ResultText;
    Parent = ResultText->Base;
    
    // Calculate baseline offset positions
    font_group *FontGroup    = GetFontGroup(GS, Atlas, (u32)'o');
    u32 FontGroupStart       = FontGroup->CodepointRange.First;
    u32 FontGroupEnd         = FontGroup->CodepointRange.Last;
    
    
    r32 FontHeight = GetFontSize(Renderer, FontSize).Size;
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
        
        r32 FontScaleFactor = FontHeight/(FontGroup->FontMetrics.Ascent + FontGroup->FontMetrics.Descent*-1);
        if(NextChar == 10) // On '\n' we insert a newline
        {
            ResultText->CurrentP = V2(0, ResultText->CurrentP.y + FontGroup->FontMetrics.RowGap*FontScaleFactor);
            continue;
        }
        u32 CharDataID = NextChar - FontGroupStart; // Map codepoint into array range.
        
        stbtt_aligned_quad A;
        stbtt_GetPackedQuad(FontGroup->FontDataForEachSize[FontSize].CharData, FontGroup->BitmapWidth, FontGroup->BitmapHeight, 
                            CharDataID, &ResultText->CurrentP.x, &ResultText->CurrentP.y, &A, 0);
        
        rect Rect = {{A.x0, A.y1*-1}, {A.x1, A.y0*-1}}; // Switch Ascent/Descent as we draw inverse to stbtt.
        rect_pe RectPE = RectToRectPE(Rect);
        ResultText->RenderEntries[ResultText->Count] = CreateFontEntry(RectPE.Extends, DepthOffset, 
                                                                       FontGroup->GLID, Color, Parent);
        render_entry *Entry  = ResultText->RenderEntries+ResultText->Count++;
        entry_id EntryID = {Entry};
        SetLocalPosition(&EntryID, GetCenter(Rect));
        
        Entry->TexCoords[0] = {A.s0, A.t1};
        Entry->TexCoords[1] = {A.s0, A.t0};
        Entry->TexCoords[2] = {A.s1, A.t0};
        Entry->TexCoords[3] = {A.s1, A.t1};
        
        if(TextHeight.E[0] > A.y0) TextHeight.E[0] = A.y0;
        if(TextHeight.E[1] < A.y1) TextHeight.E[1] = A.y1;
        
        DepthOffset -= 0.000001f;
        
        It += Advance;
    }
    if(ResultText->Count > 0)
    {
        ResultText->Extends.x  = ResultText->CurrentP.x;
        ResultText->Extends.y  = Abs(TextHeight.E[0]) + Abs(TextHeight.E[1]);
        // entry_id *TestBase2 = CreateRenderRect(Renderer, V2(5,5), DepthOffset-0.00001f, Color, Parent);
        // SetPosition(TestBase2, StartP + V2(ResultText->Extends.x, 0));
    }
}

internal void
CopyRenderText(game_state *GS, render_text *In, render_text *Out, r32 Depth = -2.0f)
{
    if(Out->MaxCount == 0)
    {
        Out->MaxCount         = Max(CHARACTERS_PER_TEXT_INFO, In->Count);
        Out->RenderEntries    = AllocateArray(&GS->FixArena, Out->MaxCount, render_entry);
    }
    else if (Out->MaxCount < In->Count)
    {
        Out->RenderEntries = ReallocateArray(&GS->FixArena, Out->RenderEntries, Out->MaxCount, In->Count, render_entry);
        Out->MaxCount = In->Count;
    }
    r32 DepthOffset = (Depth > -2.0f) ? Depth : GetDepth(In->Base);
    Out->CurrentP = In->CurrentP;
    Out->Count    = In->Count;
    Out->Extends  = In->Extends;
    Out->Base     = CreateRenderBitmap(&GS->Renderer, V2(0), DepthOffset, GetParent(In->Base), 0); // GLID can be 0, never used!
    SetPosition(Out->Base, GetPosition(In->Base));
    Out->Base->ID->Type = renderType_Text;
    Out->Base->ID->Text = Out;
    
    For(Out->Count)
    {
        Out->RenderEntries[It] = In->RenderEntries[It];
        Out->RenderEntries[It].Parent = Out->Base;
        Out->RenderEntries[It].Vertice[0].z = DepthOffset;
        Out->RenderEntries[It].Vertice[1].z = DepthOffset;
        Out->RenderEntries[It].Vertice[2].z = DepthOffset;
        Out->RenderEntries[It].Vertice[3].z = DepthOffset;
        DepthOffset -= 0.000001f;
    }
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
    if(S == NULL) return Result;
    
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
FindAndLoadFontWithUnicodeCodepoint(arena_allocator *ScratchArena, raw_font *SearchFont, string_c *FoundFontPath_out)
{
    string_c FolderPathStar = NewStringCompound(ScratchArena, 255);
    AppendStringCompoundToCompound(&FolderPathStar, SearchFont->FolderPath);
    AppendStringToCompound(&FolderPathStar, (u8 *)"\\*");
    
    string_w WideFolderPath = {};
    ConvertString8To16(ScratchArena, &FolderPathStar, &WideFolderPath);
    
    FoundFontPath_out->S = NULL;
    FoundFontPath_out->Length = 0;
    FoundFontPath_out->Pos = 0;
    
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
                                CopyIntoCompound(FoundFontPath_out, &FontPath);
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

inline font_size 
GetFontSize(struct renderer *Renderer, font_size_id ID)
{
    font_size Result = Renderer->FontAtlas.FontSizes.Sizes[ID];
    return Result;
}

// These are pre-scaled for the given size.
inline font_metrics
GetFontMetrics(game_state *GS, font_size_id ID, string_c Text)
{
    u32 Codepoint = 0;
    GetUTF8Decimal(Text.S, &Codepoint);
    font_metrics Metrics =  GetFontGroup(GS, &GS->Renderer.FontAtlas, Codepoint)->FontMetrics;
    r32 FontHeight   = GetFontSize(&GS->Renderer, ID).Size;
    r32 ScaleFactor  = FontHeight/(Metrics.Ascent + Metrics.Descent*-1);
    Metrics.Ascent  *= ScaleFactor;
    Metrics.Descent *= ScaleFactor;
    Metrics.LineGap *= ScaleFactor;
    Metrics.RowGap  *= ScaleFactor;
    return Metrics;
}

// These are pre-scaled for the given size.
inline r32
GetFontDescent(game_state *GS, font_size_id ID, string_c Text)
{
    return GetFontMetrics(GS, ID, Text).Descent;
}

// These are pre-scaled for the given size.
inline r32
GetFontAscent(game_state *GS, font_size_id ID, string_c Text)
{
    return GetFontMetrics(GS, ID, Text).Ascent;
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
