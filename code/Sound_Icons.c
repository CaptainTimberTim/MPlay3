

inline u32
ColorToColor32(v3 Color, u8 Alpha)
{
    u32 Result = 0;
    
    u8 Red   = (u8)(255*Color.r);
    u8 Blue  = (u8)(255*Color.g);
    u8 Green = (u8)(255*Color.b);
    
    Result = (Red << 0) | (Blue << 8) | (Green << 16) | (Alpha << 24);
    return Result;
}

internal loaded_bitmap
CreatePlusIcon(game_state *GS, u32 Size)
{
    loaded_bitmap Bitmap = {};
    Bitmap.Pixels = AllocateArray(&GS->FixArena, Size*Size, u32);
    Bitmap.ColorFormat = colorFormat_RGBA;
    Bitmap.Width  = Size;
    Bitmap.Height = Size;
    Bitmap.WasLoaded = true;
    
    r32 Border    = 0.25f;
    r32 PlusWidth = 0.15f;
    
    u32 MinBorder = (u32)(Size*Border);
    u32 MaxBorder = (u32)(Size*(1-Border));
    
    u32  Left = (u32)(Size*(0.5f - (PlusWidth*0.5f)));
    u32 Right = (u32)(Size*(0.5f + (PlusWidth*0.5f)));
    
    u32 NewColor = ColorToColor32(V3(1, 1, 1), 0);
    For(Size, Y_)
    {
        For(Size, X_)
        {
            i32 Pos = Y_It*Size + X_It;
            
            u8 Alpha = 0;
            if(Y_It > MinBorder && Y_It < MaxBorder && 
               X_It > MinBorder && X_It < MaxBorder) // Border around it.
            {
                if     (X_It > Left && X_It < Right) Alpha = 255;
                else if(Y_It > Left && Y_It < Right) Alpha = 255;
            }
            Bitmap.Pixels[Pos] = NewColor | Alpha << 24;
        }
    }
    
    return Bitmap;
}


internal loaded_bitmap
CreatePlayPauseIcon(game_state *GS, u32 Size)
{
    loaded_bitmap Bitmap = {};
    Bitmap.Pixels = AllocateArray(&GS->FixArena, Size*Size, u32);
    Bitmap.ColorFormat = colorFormat_RGBA;
    Bitmap.Width  = Size;
    Bitmap.Height = Size;
    Bitmap.WasLoaded = true;
    
    r32 Border        = 0.2f;
    r32 PauseBarWidth = 0.15f;
    r32 PauseStart    = 0.5f;
    r32 PauseGap      = 0.025f;
    
    u32 TriCount  = 0;
    u32 MinBorder = (u32)(Size*Border);
    u32 MaxBorder = (u32)(Size*(1-Border));
    
    u32 PauseBar1Start = (u32)(Size*PauseStart);
    u32 PauseBar1End   = PauseBar1Start + (u32)(Size*PauseBarWidth);
    u32 PauseBar2Start = PauseBar1End   + (u32)(Size*PauseGap);
    u32 PauseBar2End   = PauseBar2Start + (u32)(Size*PauseBarWidth);
    
    
    u32 NewColor = ColorToColor32(V3(1, 1, 1), 0);
    For(Size, Y_)
    {
        if(Y_It < Size*0.5f) ++TriCount;
        else                 --TriCount;
        
        For(Size, X_)
        {
            i32 Pos = Y_It*Size + X_It;
            
            u8 Alpha = 0;
            if(Y_It > MinBorder && Y_It < MaxBorder && 
               X_It > MinBorder && X_It < MaxBorder) // Border around it.
            {
                if     (X_It < TriCount)                              Alpha = 255;
                else if(X_It > PauseBar1Start && X_It < PauseBar1End) Alpha = 255;
                else if(X_It > PauseBar2Start && X_It < PauseBar2End) Alpha = 255;
            }
            Bitmap.Pixels[Pos] = NewColor | Alpha << 24;
        }
    }
    
    return Bitmap;
}


internal loaded_bitmap
_CreatePlayPauseIcon(game_state *GS, u32 Size)
{
    Size = (u32)(Size*0.5f);
    loaded_bitmap Bitmap = {};
    Bitmap.Pixels = AllocateArray(&GS->FixArena, Size*Size, u32);
    Bitmap.ColorFormat = colorFormat_RGBA;
    Bitmap.Width  = Size;
    Bitmap.Height = Size;
    Bitmap.WasLoaded = true;
    
    r32 Border        = 0.0f;
    r32 PauseBarWidth = 0.15f;
    r32 PauseStart    = 0.5f;
    r32 PauseGap      = 0.025f;
    r32 PlayApex      = 0.6f;
    
    u32 TriCount  = 0;
    u32 MinBorder = (u32)(Size*Border);
    u32 MaxBorder = (u32)(Size*(1-Border));
    
    u32 PauseBar1Start = (u32)(Size*PauseStart);
    u32 PauseBar1End   = PauseBar1Start + (u32)(Size*PauseBarWidth);
    u32 PauseBar2Start = PauseBar1End   + (u32)(Size*PauseGap);
    u32 PauseBar2End   = PauseBar2Start + (u32)(Size*PauseBarWidth);
    
    u32 ApexCount = (u32)(PlayApex*Size);
    r32 TriPart = PlayApex*2;
    
    u32 NewColor = ColorToColor32(V3(1, 1, 1), 0);
    For(Size, Y_)
    {
        if(Y_It < Size*0.5f) ++TriCount;
        else                 --TriCount;
        
        u32 TriBorder = Y_It;
        if(Y_It > Size*0.5f) TriBorder = Size - Y_It;
        
        r32 TriBorderP = (TriBorder*TriPart);
        TriBorder = (u32)TriBorderP;
        
        For(Size, X_)
        {
            i32 Pos = Y_It*Size + X_It;
            
            u8 Alpha = 0;
            if(Y_It > MinBorder && Y_It < MaxBorder && 
               X_It > MinBorder && X_It < MaxBorder) // Border around it.
            {
                if(X_It < TriBorder)
                    Alpha = 255;
                else if(X_It == TriBorder)
                    Alpha = (u8)(255*(TriBorderP-TriBorder));
                
                //else if(X_It > PauseBar1Start && X_It < PauseBar1End) Alpha = 255;
                //else if(X_It > PauseBar2Start && X_It < PauseBar2End) Alpha = 255;
            }
            Bitmap.Pixels[Pos] = NewColor | Alpha << 24;
        }
    }
    
    return Bitmap;
}



