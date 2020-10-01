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