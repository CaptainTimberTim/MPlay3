#pragma once
#include <dsound.h>


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcDuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

enum load_error_codes
{
    loadErrorCode_NoError        =  0,
    loadErrorCode_FileLoadFailed = -1,
    loadErrorCode_DecodingFailed = -2,
    loadErrorCode_EmptyFile      = -3,
};

enum play_state
{
    playState_Error,
    playState_Done,
    playState_Playing,
    playState_Waiting
};

struct dsound_instance
{
    i32 SamplesPerSecond;
    WORD Channels;
    i32 BytesPerSample;
    DWORD BufferSize;
    DWORD SafetyBytes;
    
    LPDIRECTSOUNDBUFFER Buffer;
};

struct sound_info
{
    dsound_instance SoundInstance;
    u32 RunningSampleIndex;
    
    b32 SoundIsValid;
    
    r32 ToneVolume;
    i32 PlayedSampleCount;
    r32 PlayedTime;
    
    DWORD PrevPlayCursor;
    i32 SongEndByte;
};

struct game_sound_output_buffer
{
    i32 SamplesPerSecond;
    i32 SampleCount;
    i16 *Samples;
};

struct sound_thread_interface
{
    HANDLE SoundMutex; // To change anything, you need to have this mutex!
    
    // In
    b32 IsPlaying;
    r32 ToneVolume;
    b32 ClearSoundBufferToggle;
    mp3dec_file_info_t PlayingSongData;
    b32 ChangedTimeToggle;
    b32 SongChangedToggle;
    b32 SongFinishedLoadingToggle;
    
    // Out
    b32 IsFinishedPlayingToggle;
    
    // In-Out
    r32 CurrentPlaytime;
};

struct sound_thread
{
    arena_allocator *Arena;
    sound_thread_interface Interface;
    
    sound_info SoundInfo;
    i16 *SoundSamples;
};

