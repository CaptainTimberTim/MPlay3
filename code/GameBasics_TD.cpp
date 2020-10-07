#include "GameBasics_TD.h"

internal loaded_bitmap 
LoadImage_STB(u8 *Path)
{
    loaded_bitmap Result = {};
    
    // TODO:: Small image is strange: https://github.com/nothings/stb/issues/161
    i32 BG_X, BG_Y, BG_N;
    Result.Pixels = (u32 *)stbi_load((const char *)Path, &BG_X, &BG_Y, &BG_N, 0);
    Result.Width = BG_X;
    Result.Height = BG_Y;
    if(Result.Pixels) 
    {
        Result.WasLoaded = true;
        switch(BG_N)
        {
            case 3:
            {
                Result.ColorFormat = colorFormat_RGB;
            } break;
            case 4:
            {
                Result.ColorFormat = colorFormat_RGBA;
            } break;
            
            InvalidDefaultCase
        }
    }
    
    return Result;
}

inline void
FreeImage_STB(loaded_bitmap Bitmap)
{
    stbi_image_free(Bitmap.Pixels);
}


#if DEBUG_TD
inline timer
StartTimer()
{
    timer Timer = {};
    Timer.Start = Timer.LastSnap = GetWallClock();
    return Timer;
}

inline void
SnapTimer(timer *Timer)
{
    i64 NewSnap = GetWallClock();
    r32 CurrentSnap = GetSecondsElapsed(GlobalGameState.Time.PerfCountFrequency, Timer->LastSnap, NewSnap);
    r32 Total = GetSecondsElapsed(GlobalGameState.Time.PerfCountFrequency, Timer->Start, NewSnap);
    DebugLog(255, "Timer snap %i: %.8f, total: %.8f\n", ++Timer->Count, CurrentSnap, Total);
    Timer->LastSnap = NewSnap;
}
#endif

internal void
UpdateColorPickerTexture(color_picker *ColorPicker)
{
    loaded_bitmap *Bitmap = &ColorPicker->Bitmap;
    if(!Bitmap->Pixels) 
        Bitmap->Pixels = PushArrayOnBucket(&GlobalGameState.Bucket.Fixed, Bitmap->Width*Bitmap->Height, u32);
    
    For(Bitmap->Height, Y_)
    {
        r32 YPercent = Y_It/(r32)Bitmap->Height;
        For(Bitmap->Width, X_)
        {
            i32 Pos = Y_It*Bitmap->Width + X_It;
            
            r32 XPercent = X_It/(r32)Bitmap->Width;
            
            r32 Red32   = (1-XPercent)*(1-YPercent)-ColorPicker->Blackness;
            r32 Blue32  =    XPercent *(1-YPercent)-ColorPicker->Blackness;
            r32 Green32 = (1-XPercent)*   YPercent -ColorPicker->Blackness;
            
            u8 Red   = (u8)Min(255.0f, Max(0.0f, Red32*255));
            u8 Blue  = (u8)Min(255.0f, Max(0.0f, Blue32*255));
            u8 Green = (u8)Min(255.0f, Max(0.0f, Green32*255));
            u8 Alpha = (XPercent+YPercent > 1) ? 0 : 255;
            
            Bitmap->Pixels[Pos] = (Red << 0) | (Green << 8) | (Blue << 16) | (Alpha << 24);
        }
    }
    
    if(!ColorPicker->GLID) ColorPicker->GLID = CreateGLTexture(*Bitmap);
    else UpdateGLTexture(*Bitmap, ColorPicker->GLID);
}

internal color_picker
CreateColorPicker(v2i BitmapSize)
{
    color_picker Result = {};
    
    Result.Bitmap.ColorFormat = colorFormat_RGBA;
    Result.Bitmap.Width  = BitmapSize.x;
    Result.Bitmap.Height = BitmapSize.y;
    Result.Bitmap.WasLoaded = true;
    
    Result.Blackness = -0.00f;
    
    UpdateColorPickerTexture(&Result);
    
    Result.TextureEntry = CreateRenderBitmap(&GlobalGameState.Renderer, V2(BitmapSize), -0.9f, 0, Result.GLID);
    SetPosition(Result.TextureEntry, V2(GlobalGameState.Renderer.Window.CurrentDim.Dim)*0.5f);
    
    return Result;
}











