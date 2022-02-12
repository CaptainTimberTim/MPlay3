#include "Sound_Thread_TD.h"

internal b32
InitDSound(dsound_instance *SoundInstance, HWND Window)
{
    b32 Result = false;
    
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED( DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat    = {0};
            WaveFormat.wFormatTag      = WAVE_FORMAT_PCM;
            WaveFormat.nChannels       = SoundInstance->Channels;
            WaveFormat.nSamplesPerSec  = SoundInstance->SamplesPerSecond;
            WaveFormat.wBitsPerSample  = 16;
            WaveFormat.nBlockAlign     = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize          = 0;
            
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                // NOTE:: Creating a PrimaryBuffer for legacy reasons to set the Wave Format.
                // The SecondaryBuffer is the one we will write in.
                DSBUFFERDESC BufferDescription = {}; 
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {
                        //SUCCESS!
                        DSBUFFERDESC BufferDescription = {};
                        BufferDescription.dwSize = sizeof(BufferDescription);
                        BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS;
                        BufferDescription.dwBufferBytes = SoundInstance->BufferSize;
                        BufferDescription.lpwfxFormat = &WaveFormat;
                        HRESULT ErrorCodeCreateSecondSoundBuffer = DirectSound->CreateSoundBuffer(&BufferDescription,
                                                                                                  &SoundInstance->Buffer,
                                                                                                  0);
                        if(SUCCEEDED(ErrorCodeCreateSecondSoundBuffer))
                        {
                            OutputDebugStringA("Sound buffer created successfully!\n");
                            Result = true;
                        }
                    }
                    else Assert(false); //ERROR::FAILED
                }
                else Assert(false); //ERROR::FAILED
            }
            else Assert(false); //ERROR::FAILED!
        }
        else Assert(false); // ERROR:: FAILED!
    }
    else Assert(false); //ERROR::DSOUNDLIBRARY not found
    return Result;
}

internal void
FillSoundBuffer(sound_info *SoundInfo, dsound_instance *SoundInstance, 
                DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(SoundInstance->Buffer->Lock(ByteToLock, BytesToWrite,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        DWORD Region1SampleCount = Region1Size / SoundInstance->BytesPerSample;
        i16 *DestSample = (i16 *)Region1;
        i16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            For((u16)(SoundInstance->BytesPerSample/sizeof(u16))) *DestSample++ = *SourceSample++;
            ++SoundInfo->RunningSampleIndex;
        }
        
        // Second region is only > 0 if the ByteToLock + BytesToWrite is bigger than the buffersize.
        // Then the overhang is the second region, because the buffer is circular in conception.
        DWORD Region2SampleCount = Region2Size / SoundInstance->BytesPerSample;
        DestSample = (i16 *)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            For((u16)(SoundInstance->BytesPerSample/sizeof(u16))) *DestSample++ = *SourceSample++;
            ++SoundInfo->RunningSampleIndex;
        }
        
        SoundInstance->Buffer->Unlock(Region1, Region1Size, 
                                      Region2, Region2Size);
    }
}

internal void
ClearSoundBuffer(dsound_instance *SoundInstance)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(SoundInstance->Buffer->Lock(0, SoundInstance->BufferSize,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        u8 *DestSample = (u8 *)Region1;
        for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (u8 *)Region2;
        for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        SoundInstance->Buffer->Unlock(Region1, Region1Size, 
                                      Region2, Region2Size);
    }
}

inline void
NewSoundInstance(dsound_instance *SoundInstance, HWND Window, u32 SongHz, u32 Channels, u32 AppHz)
{
    if(SoundInstance->Buffer) SoundInstance->Buffer->Stop();
    SoundInstance->SamplesPerSecond = SongHz;
    SoundInstance->Channels         = (WORD)Channels;
    SoundInstance->BytesPerSample   = sizeof(i16)*SoundInstance->Channels;
    SoundInstance->BufferSize       = SoundInstance->SamplesPerSecond * SoundInstance->BytesPerSample;
    SoundInstance->SafetyBytes      = (i32)((((r32)SoundInstance->SamplesPerSecond*(r32)SoundInstance->BytesPerSample) / AppHz) / 1.0f);
    
    if(!InitDSound(SoundInstance, Window))
    {
        Assert(!"Failed to create new soundbuffer");
    }
    ClearSoundBuffer(SoundInstance);
    SoundInstance->Buffer->Play(0, 0, DSBPLAY_LOOPING);
}

internal void
FillSamplesFromPlayingFile(time_management *Time, sound_info *SoundInfo, game_sound_output_buffer *SoundBuffer,
                           mp3dec_file_info_t *MP3Decoded, DWORD ByteToLock)
{
    Assert(MP3Decoded);
    Assert(MP3Decoded->buffer);
    Assert(MP3Decoded->channels > 0);
    
    SoundInfo->PlayedTime += Time->dTime;
    
    i16 *SampleOut = SoundBuffer->Samples;
    i16 *SampleIn  = MP3Decoded->buffer + SoundInfo->PlayedSampleCount*MP3Decoded->channels;
    i32 LastSample = -1;
    for(i32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        For((u32)MP3Decoded->channels)
        {
            if(SoundInfo->PlayedSampleCount+SampleIndex+It < MP3Decoded->samples/MP3Decoded->channels)
                *SampleOut++ = (i16)(*SampleIn++ * SoundInfo->ToneVolume);
            else // When file ends, fill the rest of the frame with silence
            {
                if(LastSample < 0) LastSample = SampleIndex;
                *SampleOut++ = 0;
            }
        }
    }
    
    if(SoundInfo->PlayedSampleCount >= MP3Decoded->samples/MP3Decoded->channels)
    {
        SoundInfo->PlayedTime = 0;
        SoundInfo->PlayedSampleCount = (i32)MP3Decoded->samples/MP3Decoded->channels;
        
        if(SoundInfo->SongEndByte < 0)
        {
            dsound_instance *SoundInstance = &SoundInfo->SoundInstance;
            SoundInfo->SongEndByte = (ByteToLock+LastSample*SoundInstance->BytesPerSample)%SoundInstance->BufferSize;
        }
    }
    else SoundInfo->PlayedSampleCount += SoundBuffer->SampleCount;
}


inline i32
CircleDistance(i32 From, i32 To, i32 CircleBufferSize)
{
    i32 Result = 0;
    
    if(To < From) // Handle circular buffer wrap
    {
        Result = (CircleBufferSize - From) + To;
    }
    else Result = To-From;
    
    return Result;
}

inline b32
IsGivenBetweenForCircularBuffer(i32 Given, i32 Left, i32 Right)
{
    b32 IsLeftSmaller = Left < Right;
    
    // Normal case, not at the edge of the circular buffer
    b32              Case_InMiddle =  IsLeftSmaller && (Given > Left) && (Given <= Right);
    // Case where both Given and Right are over the edge.
    b32         Case_LeftPrevCycle = !IsLeftSmaller && (Given < Left) && (Given <= Right);
    // Case where only Right is over the edge.
    b32 Case_LeftAndGivenPrevCycle = !IsLeftSmaller && (Given > Left) && (Given >  Right);
    
    return Case_InMiddle || Case_LeftPrevCycle || Case_LeftAndGivenPrevCycle;
}

inline LONGLONG GetWallClock();
inline r32 GetSecondsElapsed(i64 PerfCountFrequency, LONGLONG Start, LONGLONG End);

internal play_state
SimpleCalculateAndPlaySound(time_management *Time, sound_thread *SoundThread, mp3dec_file_info_t *MP3Decoded,
                            b32 VolumeChanged)
{
    sound_info *SoundInfo          = &SoundThread->SoundInfo;
    dsound_instance *SoundInstance = &SoundInfo->SoundInstance;
    i16 *Samples                   = SoundThread->SoundSamples;
    
    play_state Result = playState_Error;
    
    r32 MaxDTime = 1/2.0f;
    if(Time->dTime < MaxDTime) 
    {
        DWORD PlayCursor;
        DWORD WriteCursor;
        if(SoundInstance->Buffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
        {
            // NOTE::
            // To calculate the amount of new samples to write in the buffer I 
            // use the distance of the write cursor to the ByteToLock (which points
            // to the byte in the buffer where I stopped writing at last frame) to find out
            // how many samples I still need to write to reach the previously decided 
            // amount of samples. It is hardcoded to be enough samples to run at 2 FPS 
            // as minimum framerate. If lower, we pause playing music.
            
            if(!SoundInfo->SoundIsValid)
            {
				Assert(SoundInstance->BytesPerSample > 0);
                SoundInfo->RunningSampleIndex = WriteCursor / SoundInstance->BytesPerSample;
                SoundInfo->SoundIsValid = true;
                SoundInfo->SongEndByte = -1;
            }
            
            DWORD ByteToLock = (SoundInfo->RunningSampleIndex*SoundInstance->BytesPerSample)%SoundInstance->BufferSize;
            i32 WriteSampleAmount = (i32)(SoundInstance->SamplesPerSecond*MaxDTime);
            
            Assert(!(WriteCursor > ByteToLock && WriteCursor < SoundInstance->BufferSize-(SoundInstance->BufferSize*MaxDTime)));
            i32 SampleDistanceWriteToWritten = CircleDistance(WriteCursor, ByteToLock, SoundInstance->BufferSize)
                /SoundInstance->BytesPerSample;
            
            if(VolumeChanged) 
            {
                // If volume changed, we overwrite everything that was already in the buffer with 
                // the samples with the new volume. This makes the change as fast as the actual framerate.
                SoundInfo->RunningSampleIndex -= SampleDistanceWriteToWritten;
                SoundInfo->PlayedSampleCount  -= SampleDistanceWriteToWritten;
                ByteToLock = WriteCursor;
            }
            else WriteSampleAmount = Max(0, WriteSampleAmount - SampleDistanceWriteToWritten);
            
            game_sound_output_buffer SoundBuffer = {};
            SoundBuffer.SamplesPerSecond = SoundInstance->SamplesPerSecond;
            SoundBuffer.SampleCount = WriteSampleAmount;
            SoundBuffer.Samples = Samples;
            
            FillSamplesFromPlayingFile(Time, SoundInfo, &SoundBuffer, MP3Decoded, ByteToLock);
            FillSoundBuffer(SoundInfo, SoundInstance, ByteToLock, WriteSampleAmount*SoundInstance->BytesPerSample,
                            &SoundBuffer);
            
            
            // This handles that the song is finished, but the PlayCursor needs to have played
            // the last samples of the song, before I can give the playState_Done response.
            if(SoundInfo->SongEndByte >= 0) 
            {
                b32 IsDone = IsGivenBetweenForCircularBuffer(SoundInfo->SongEndByte, SoundInfo->PrevPlayCursor, PlayCursor);
                if(IsDone)
                {
                    Result = playState_Done;
                    DebugLog(12, "Done.\n");
                }
            }
            else Result = playState_Playing;
            SoundInfo->PrevPlayCursor = PlayCursor;
        }
    }
    else DebugLog(255, "ERROR: Framerate too low (last frame took %.2fs). Music temporarily paused.\n", Time->dTime);
    return Result;
}

internal b32
GetIsFinishedPlaying(sound_thread_interface *Interface)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    b32 Result = Interface->IsFinishedPlayingToggle;
    Interface->IsFinishedPlayingToggle = false;
    ReleaseMutex(Interface->SoundMutex);
    return Result;
}

internal b32
GetIsPlayingPreload(sound_thread_interface *Interface)
{
    b32 Result = Interface->PreloadIsActive;
    return Result;
}

internal r32 
GetPlayedTime(sound_thread_interface *Interface)
{
    r32 Result = Interface->CurrentPlaytime;
    return Result;
}

internal r32 
GetToneVolume(sound_thread_interface *Interface)
{
    r32 Result = Interface->ToneVolume;
    return Result;
}

internal void
PushNewPlayedTime(sound_thread_interface *Interface, r32 NewTime)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->CurrentPlaytime = NewTime;
    Interface->ChangedTimeToggle = true;
    ReleaseMutex(Interface->SoundMutex);
}

internal void
PushNewPlayedTimeAfterLoading(sound_thread_interface *Interface, r32 NewTime)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->CurrentPlaytime   = NewTime;
    Interface->ChangedTimeToggle = true;
    Interface->ChangedTimeAfterLoadingToggle = true;
    ReleaseMutex(Interface->SoundMutex);
}

internal void
PushToneVolume(sound_thread_interface *Interface, r32 VolumeChange)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->ToneVolume = Clamp01(Interface->ToneVolume+VolumeChange);
    ReleaseMutex(Interface->SoundMutex);
}

internal void
SetToneVolume(sound_thread_interface *Interface, r32 NewVolume)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->ToneVolume = NewVolume;
    ReleaseMutex(Interface->SoundMutex);
}

internal void
PushIsPlaying(sound_thread_interface *Interface, b32 IsPlaying)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->IsPlaying = IsPlaying;
    ReleaseMutex(Interface->SoundMutex);
}

internal void
PushSoundBufferClear(sound_thread_interface *Interface)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->ClearSoundBufferToggle = true;
    ReleaseMutex(Interface->SoundMutex);
}

internal void
PushSongChanged(sound_thread_interface *Interface, mp3dec_file_info_t *SongData)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->SongChangedToggle = true;
    Interface->PlayingSongData = *SongData;
    ReleaseMutex(Interface->SoundMutex);
}

internal void
PushSongLoadingComplete(sound_thread_interface *Interface, mp3dec_file_info_t *SongData)
{
    WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false);
    Interface->SongFinishedLoadingToggle = true;
    Interface->PlayingSongData           = *SongData;
    // TODO:: Shouldn't we copy the data here instead of in the sound thread? I am not sure...
    // The safe solution would be to have this as "passover" data, that is copied here and 
    // _only_ used in mutax-land. The sound thread would need to, maybe, copy small portions
    // of it every frame as needed. Or the whole thing. 
    ReleaseMutex(Interface->SoundMutex);
}

internal SOUND_THREAD_CALLBACK(ProcessSound)
{
    sound_thread_interface *Interface = &ThreadInfo->Interface;
    sound_info *SoundInfo = &ThreadInfo->SoundInfo;
    arena_allocator *Arena = &GlobalGameState.SoundThreadArena;
    Arena->EmptyArenaCount = 1;
    
    // Initializing clock
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    i64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    LONGLONG PrevCycleCount = GetWallClock();
    LONGLONG FlipWallClock  = GetWallClock();
    
    time_management Time = {60, 1.0f/60.0f, 0, 0, 1};
    play_state PlayState = playState_Startup;
    mp3dec_file_info_t MP3Decoded = {};
    i16 *Buffer = 0;
    
    b32 IsPlaying = false;
    b32 DoSoundBufferClear = false;
    b32 WaitForMainThread  = false;
    
    r32 PrevVolume = Interface->ToneVolume;
    r32 CurrentSongPlaytime = 0;
    while(true)
    {
        LONGLONG CurrentCycleCount = GetWallClock();
        Time.dTime = GetSecondsElapsed(PerfCountFrequency, PrevCycleCount, CurrentCycleCount);
        PrevCycleCount = CurrentCycleCount;
        Time.GameTime += Time.dTime;
        
        WaitForSingleObjectEx(Interface->SoundMutex, INFINITE, false); // ************************
        
        if(PlayState == playState_Playing)
        {
            if(Interface->ChangedTimeToggle)
            {
                Interface->ChangedTimeToggle = false;
                Interface->ChangedTimeAfterLoadingToggle = false;
                SoundInfo->PlayedSampleCount = (u32)(Interface->CurrentPlaytime*
                                                     SoundInfo->SoundInstance.SamplesPerSecond);
                CurrentSongPlaytime = Interface->CurrentPlaytime;
                WaitForMainThread   = false;
            }
            
            u32 SongDuration   = (u32)MP3Decoded.samples/MP3Decoded.channels/MP3Decoded.hz*1000;
            r32 PlayPercentage = SoundInfo->PlayedSampleCount / (r32)(MP3Decoded.samples/MP3Decoded.channels);
            r32 Playtime       = PlayPercentage*(r32)SongDuration;
            
            Interface->CurrentPlaytime = Playtime/1000.0f;
        }
        else if(!Interface->ChangedTimeAfterLoadingToggle)
            Interface->CurrentPlaytime = 0;
        
        if(PlayState == playState_Done) 
        {
            Interface->IsFinishedPlayingToggle = true;
            PlayState = playState_Waiting;
        }
        
        if(Interface->SongChangedToggle)
        {
            Interface->SongChangedToggle = false;
            if(PlayState == playState_Startup) PlayState = playState_Waiting;
            else
            {
                // If we change the song and are not in startup state anymore, we don't want to 
                // play from where the song was left on last time the app was closed.
                Interface->ChangedTimeAfterLoadingToggle = false;
                Interface->CurrentPlaytime = 0;
            }
            
            if(MP3Decoded.buffer) FreeMemory(Arena, MP3Decoded.buffer);
            MP3Decoded        = Interface->PlayingSongData;
            MP3Decoded.buffer = AllocateArray(Arena, (u32)MP3Decoded.samples, i16);
            For(MP3Decoded.samples) MP3Decoded.buffer[It] = Interface->PlayingSongData.buffer[It];
            
            if(MP3Decoded.hz != SoundInfo->SoundInstance.SamplesPerSecond ||
               MP3Decoded.channels != SoundInfo->SoundInstance.Channels)
            {
                NewSoundInstance(&SoundInfo->SoundInstance, GlobalGameState.Renderer.Window.WindowHandle,
                                 MP3Decoded.hz, MP3Decoded.channels, 60);
            }
            
            SoundInfo->PlayedTime        = 0;
            CurrentSongPlaytime          = 0;
            SoundInfo->PlayedSampleCount = 0;
            
            WaitForMainThread          = false;
            Interface->PreloadIsActive = true;
        }
        if(Interface->SongFinishedLoadingToggle) // When the song is finished fully loading.
        {
            Interface->SongFinishedLoadingToggle = false;
            
            Assert(Interface->PlayingSongData.hz == MP3Decoded.hz);
            Assert(Interface->PlayingSongData.channels == MP3Decoded.channels);
            
            if(MP3Decoded.buffer) FreeMemory(Arena, MP3Decoded.buffer);
            MP3Decoded        = Interface->PlayingSongData;
            MP3Decoded.buffer = AllocateArray(Arena, (u32)MP3Decoded.samples, i16);
            For(MP3Decoded.samples) MP3Decoded.buffer[It] = Interface->PlayingSongData.buffer[It];
            
            WaitForMainThread = false;
            Interface->PreloadIsActive = false;
        }
        
        DoSoundBufferClear = Interface->ClearSoundBufferToggle;
        Interface->ClearSoundBufferToggle = false;
        
        if(!IsPlaying && Interface->IsPlaying) WaitForMainThread = false;
        IsPlaying             = Interface->IsPlaying;
        SoundInfo->ToneVolume = Interface->ToneVolume;
        
        // Check if playing song preload is close to the preload amount. If so
        // just use the currently decoded stuff of the full song. 
        // NOTE:: This should only ever happen if the decoded file is really big.
        playing_decoded *PlayingDecoding = &GlobalGameState.MP3Info->DecodeInfo.PlayingDecoded;
        u32 QuarterSecondSamples = (MP3Decoded.hz*MP3Decoded.channels)/4;
        b32 PreloadAlmostPlayed = (MP3Decoded.samples-SoundInfo->PlayedSampleCount*MP3Decoded.channels) < QuarterSecondSamples;
        if(PlayState == playState_Playing && PreloadAlmostPlayed && PlayingDecoding->CurrentlyDecoding)
        {
            DebugLog(255, "NOTE:: Preload ended. Using half decoded file.\n");
            if(MP3Decoded.buffer) FreeMemory(Arena, MP3Decoded.buffer);
            MP3Decoded        = PlayingDecoding->Data;
            MP3Decoded.buffer = AllocateArray(Arena, (u32)MP3Decoded.samples, i16);
            For(MP3Decoded.samples) MP3Decoded.buffer[It] = PlayingDecoding->Data.buffer[It];
            
            WaitForMainThread = false;
        }
        
        ReleaseMutex(Interface->SoundMutex); // *************************************************
        
        if(MP3Decoded.buffer)
        {
            if(DoSoundBufferClear) 
            {
                ClearSoundBuffer(&SoundInfo->SoundInstance);
                SoundInfo->SoundIsValid = false;
            }
            
            if(IsPlaying && !WaitForMainThread)
            {
                PlayState = SimpleCalculateAndPlaySound(&Time, ThreadInfo, &MP3Decoded, PrevVolume != SoundInfo->ToneVolume);
                
                if(PlayState == playState_Done)
                {
                    ClearSoundBuffer(&SoundInfo->SoundInstance);
                    SoundInfo->SoundIsValid = false;
                    WaitForMainThread = true;
                }
                else if(PlayState == playState_Error) IsPlaying = false; // TODO:: This does not make sense. Something else should happen...
            }
        }
        PrevVolume = SoundInfo->ToneVolume;
        
        r32 dFrameWorkTime = GetSecondsElapsed(PerfCountFrequency, CurrentCycleCount, GetWallClock());
        if(dFrameWorkTime < Time.GoalFrameRate)
        {
            DWORD SleepMS = (DWORD)(1000.0f * (Time.GoalFrameRate - dFrameWorkTime));
            if(SleepMS > 0)
            {
                Sleep(SleepMS);
            }
        }
        
        FlipWallClock = GetWallClock();
    }
}
