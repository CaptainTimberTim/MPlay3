#include "Font_TD.h"

internal fonts 
LoadFonts(arena_allocator *Arena, font_size_array FontSizes, u8 *RawFontData)
{
    fonts Result = {};
    Result.Count = FontSizes.Count;
    Result.Font  = AllocateArray(Arena, FontSizes.Count, font_atlas);
    
    const u32 StartChar  = 32;
    const u32 CharAmount = ATLAS_LETTER_COUNT;
    
    const u32 Width  = 2048;
    const u32 Height = 1024;
    Result.BitmapWidth  = Width;
    Result.BitmapHeight = Height;
    u8 AlphaMap[Width][Height];
    loaded_bitmap Bitmap = {true, Width, Height, (u32 *)AlphaMap[0], colorFormat_Alpha, Width*sizeof(u32)};
    
    stbtt_pack_context PackContext;
    stbtt_PackBegin(&PackContext, AlphaMap[0], Width, Height, 0, 1, NULL);
    For(FontSizes.Count)
    {
        Result.Font[It].CharData = AllocateArray(Arena, CharAmount, stbtt_packedchar);
        
        stbtt_PackSetOversampling(&PackContext, 3, 1);
        stbtt_PackFontRange(&PackContext, RawFontData, 0, FontSizes.Sizes[It], StartChar, CharAmount, Result.Font[It].CharData);
    }
    stbtt_PackEnd(&PackContext);
    
    Result.GLID = CreateGLTexture(Bitmap, true);
    
    return Result;
}


internal render_entry
CreateFontEntry(v2 Extends, r32 Depth, u32 BitmapID, v3 *Color, entry_id *Parent = 0)
{
    render_entry Result = {};
    
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
RenderText(renderer *Renderer, arena_allocator *Arena, font_atlas Font, string_c *Text,
           v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent, v2 StartP)
{
    ResultText->Count = 0;
    if(Parent) StartP += GetPosition(Parent);
    ResultText->CurrentP = {};//StartP;
    r32 DepthOffset = ZValue;
    
    ResultText->Base = CreateRenderBitmap(Renderer, V2(0), DepthOffset, Parent, Renderer->FontInfo.GLID);
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
    v2 TP = StartP;
    
    stbtt_aligned_quad TestQ;
    stbtt_GetPackedQuad(Font.CharData, Renderer->FontInfo.BitmapWidth, Renderer->FontInfo.BitmapHeight, 'o'-32, &TP.x, &TP.y, &TestQ, 0);
    
    r32 NewBaseline = TestQ.y0;
    r32 OldBaseline = TestQ.y1;
    
    v2 BaseP = {};
    v2 TextHeight = {MAX_REAL32, MIN_REAL32};
    For(Text->Pos)
    {
        u8 NextSymbol = Text->S[It];
        if(Text->S[It] >= 128)
        {
            for(i32 SymbolID = 0; SymbolID < ArrayCount(BasicSymbolsGer); SymbolID++)
            {
                if(CompareStringAndCompound(&BasicSymbolsGer[SymbolID].UTF8, Text->S+It))
                {
                    NextSymbol = BasicSymbolsGer[SymbolID].ANSI;
                    It++;
                }
            }
        }
        if(NextSymbol >= 32 && NextSymbol < ATLAS_LETTER_COUNT)
        {
            stbtt_aligned_quad A;
            stbtt_GetPackedQuad(Font.CharData, Renderer->FontInfo.BitmapWidth, Renderer->FontInfo.BitmapHeight, 
                                NextSymbol - 32, &ResultText->CurrentP.x, &ResultText->CurrentP.y, &A, 0);
            
            rect Rect = {{A.x0, A.y0}, {A.x1, A.y1}};
            rect_pe RectPE = RectToRectPE(Rect);
            ResultText->RenderEntries[ResultText->Count] = CreateFontEntry(RectPE.Extends, DepthOffset, 
                                                                           Renderer->FontInfo.GLID, Color, Parent);
            render_entry *Entry  = ResultText->RenderEntries+ResultText->Count++;
            entry_id EntryID = {Entry};
            // Also adding user controlled height offset
            SetLocalPosition(&EntryID, GetCenter(Rect) - V2(0, (r32)Renderer->FontInfo.HeightOffset));
            
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
        if(NextSymbol == 10)
        {
            ResultText->CurrentP = V2(0/*StartP.x*/, ResultText->CurrentP.y + (OldBaseline-NewBaseline)*2);
        }
    }
    ResultText->Height = TextHeight;
}

internal void
RenderText(renderer *Renderer, arena_allocator *Arena, font_size Size, string_c *Text,
           v3 *Color, render_text *ResultText, r32 ZValue, entry_id *Parent, v2 StartP)
{
    Assert((i32)Renderer->FontInfo.Count > Size);
    RenderText(Renderer, Arena, Renderer->FontInfo.Font[Size], Text, Color, ResultText, ZValue, Parent, StartP);
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