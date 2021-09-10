#include "GameBasics_TD.h"

inline b32
IsActive(game_state *GS, mode_flags Mode)
{
    b32 Result = (GS->ModeFlags&Mode) == Mode;
    return Result;
}

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

internal loaded_bitmap 
LoadImage_STB(read_file_result Memory)
{
    loaded_bitmap Result = {};
    
    read_file_result ImageTest = {};
    i32 BG_X, BG_Y, BG_N;
    Result.Pixels = (u32 *)stbi_load_from_memory(Memory.Data, Memory.Size, &BG_X, &BG_Y, &BG_N, 0);
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

inline void
InitTimers()
{
    _debugTimerTable = HashTable(&GlobalGameState.FixArena, TIMER_MAX_COUNT);
}

inline void
_StartTimer(u8 *Name)
{
    timer *Timer = 0;
    if(AddToHashTable(&_debugTimerTable, Name, _debugTimerCount)) 
    {
        Assert(_debugTimerCount < TIMER_MAX_COUNT);
        
        Timer = _debugTimers + _debugTimerCount++;
        Timer->LastSnap = GetWallClock();
        Timer->Total    = 0;
    }
    else 
    {
        u32 ID;
        if(!GetFromHashTable(&_debugTimerTable, Name, ID)) Assert(false);
        Timer = _debugTimers + ID;
        
        if(Timer->Paused) 
        {
            Timer->LastSnap = GetWallClock();
        }
    }
    Timer->Paused = false;
}

inline void
_RestartTimer(u8 *Name)
{
    Assert(_debugTimerCount < TIMER_MAX_COUNT);
    
    timer *Timer = 0;
    if(AddToHashTable(&_debugTimerTable, Name, _debugTimerCount))
    {
        Timer = _debugTimers + _debugTimerCount++;
    }
    else 
    {
        u32 ID;
        if(!GetFromHashTable(&_debugTimerTable, Name, ID)) Assert(false);
        Timer = _debugTimers + ID;
    }
    
    Timer->LastSnap = GetWallClock();
    Timer->Total    = 0;
    Timer->Paused   = false;
    Timer->Count    = 0;
}

inline void
_PauseTimer(u8 *Name)
{
    i64 NewSnap = GetWallClock();
    
    u32 ID;
    if(!GetFromHashTable(&_debugTimerTable, Name, ID)) Assert(false);
    timer *Timer = _debugTimers + ID;
    
    Timer->Paused = true;
    Timer->Total += NewSnap - Timer->LastSnap;
}

inline void
_SnapTimer(u8 *Name)
{
    i64 NewSnap = GetWallClock();
    
    u32 ID;
    if(!GetFromHashTable(&_debugTimerTable, Name, ID)) Assert(false);
    timer *Timer = _debugTimers + ID;
    
    if(!Timer->Paused) Timer->Total += NewSnap - Timer->LastSnap;
    else NewSnap = Timer->LastSnap;
    r32 CurrentSnap = GetSecondsElapsed(GlobalGameState.Time.PerfCountFrequency, Timer->LastSnap, NewSnap);
    r32 Total = ((r32)Timer->Total / (r32)GlobalGameState.Time.PerfCountFrequency);
    
    DebugLog(255, "%s Timer snap %i: %.8f, total: %.8f\n", Name, ++Timer->Count, CurrentSnap, Total);
    Timer->LastSnap = NewSnap;
}

#endif











