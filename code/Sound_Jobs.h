/* date = October 19th 2020 9:31 am */
#ifndef _SOUND__JOBS_H
#define _SOUND__JOBS_H

#define DO_METADATA_SUB_CRAWL

struct job_load_decode_mp3
{
    mp3_info *MP3Info;
    file_id FileID;
    i32 DecodeID;
    i32 PreloadSeconds;
};

struct crawl_thread_out
{
    b32 ThreadIsRunning;
    b32 DoneFolderSearch;
    b32 DoneCrawling;
    
    u32 TestCount;
    u32 CurrentCount;
};

struct crawl_thread
{
    mp3_info *MP3Info;
    string_c TestPath;
    
    crawl_thread_out *Out;
};

struct sub_crawl_thread
{
    crawl_thread CrawlThread;
    u32 StartIt;
    u32 EndIt;
    u32 *CurrentCount;
};

struct check_music_path
{
    thread_state State;
    mp3_info *MP3Info;
    mp3_file_info TestInfo;
    array_u32 RemoveIDs;
    array_u32 AddTestInfoIDs;
};

#define MAX_THREAD_ERRORS 20
struct error_item
{
    load_error_codes Code;
    file_id ID;
};

struct thread_error_list
{
    error_item Errors[MAX_THREAD_ERRORS];
    u32 Count;
    
    b32 RemoveDecode;
    HANDLE Mutex;
};

internal b32  AddJob_NextUndecodedInPlaylist();
internal void AddJob_CheckMusicPathChanged(check_music_path *CheckMusicPath);
internal i32  AddJob_LoadMP3(circular_job_queue *JobQueue, playlist_id PlaylistID, array_u32 *IgnoreDecodeIDs = 0, 
                             i32 PreloadSeconds = DECODE_PRELOAD_SECONDS);
internal void PushErrorMessageFromThread(error_item Error);









#endif //_SOUND__JOBS_H
