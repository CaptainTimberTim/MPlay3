#include "Sound_Jobs.h"

// ***************************************
// Job: Load/Reload metadata *************
// ***************************************

internal void // Only called from Main thread
HandleActiveMusicPath(music_display_info *DisplayInfo, input_info *Input, crawl_thread_out *CrawlInfoOut)
{
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    
    if(CrawlInfoOut->DoneFolderSearch && MusicPath->CrawlThreadStateCount == 2) 
    {
        if(CrawlInfoOut->ThreadIsRunning) MusicPath->CrawlThreadStateCount = 5;
        else MusicPath->CrawlThreadStateCount = 4;
    }
    if(CrawlInfoOut->DoneCrawling && MusicPath->CrawlThreadStateCount == 6) MusicPath->CrawlThreadStateCount = 7;
    
    switch(MusicPath->CrawlThreadStateCount)
    {
        case 0: // #Many: Just opened, no thread running yet
        {
            if(Input->KeyChange[KEY_ENTER] == KeyDown) OnMusicPathSavePressed(&DisplayInfo->MusicBtnInfo);
            ProcessMusicPath(&GlobalGameState.Renderer, GlobalGameState.Time.dTime, Input, &MusicPath->TextField);
            ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, MusicPath->Save);
            ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, MusicPath->Quit);
            ButtonTestMouseInteraction(&GlobalGameState.Renderer, Input, MusicPath->Rescan);
        } break;
        
        case 1: // #Once: Started search
        {
            SetActive(&MusicPath->LoadingBar, true);
            UpdateLoadingBar(&MusicPath->LoadingBar, 0.1f);
            MusicPath->CrawlThreadStateCount++;
        } // #Through
        
        case 2: // #Many: Is searching
        {
            // Animate indeterminite loading bar
            if(MusicPath->dWaitThenCancelTime >= 1.0f)
            {
                MusicPath->dWaitThenCancelTime = 0.0f;
                UpdateIndeterminiteLoadingBar(&MusicPath->LoadingBar, 1);
            }
            else 
            {
                MusicPath->dWaitThenCancelTime += GlobalGameState.Time.dTime/1.0f;
                UpdateIndeterminiteLoadingBar(&MusicPath->LoadingBar, MusicPath->dWaitThenCancelTime);
            }
        } break;
        
        case 4: // #Once: Searching done. Nothing found. Done.
        {
            TestFolderSearchDone(&DisplayInfo->MusicPath, CrawlInfoOut->TestCount);
            MusicPath->dWaitThenCancelTime = 0.0f;
            SetActive(&MusicPath->LoadingBar, false);
            
            CrawlInfoOut->DoneFolderSearch = false;
            SetActive(&MusicPath->LoadingBar, false);
            MusicPath->CrawlThreadStateCount = 0;
        } break;
        
        case 5: // #Once: Searching done. Started crawling
        {
            TestFolderSearchDone(&DisplayInfo->MusicPath, CrawlInfoOut->TestCount);
            MusicPath->dWaitThenCancelTime = 0.0f;
            SetActive(&MusicPath->LoadingBar, false);
            
            SetActive(&MusicPath->LoadingBar, true);
            MusicPath->CrawlThreadStateCount++;
        } // #Through
        
        case 6: // #Many: Is crawling
        {
            r32 CurrentPercentage = CrawlInfoOut->CurrentCount/(r32)CrawlInfoOut->TestCount;
            UpdateLoadingBar(&MusicPath->LoadingBar, CurrentPercentage);
        } break;
        
        case 7: // #Once: Done crawling
        {
            ApplyNewMetadata(&GlobalGameState, &GlobalGameState.MusicInfo);
            FinishedSettingUpMusicPath(&GlobalGameState, &DisplayInfo->MusicPath);
            UpdateLoadingBar(&MusicPath->LoadingBar, 1);
            MusicPath->CrawlThreadStateCount++;
        } // #Through
        
        case 8: // #Many: Wait and display
        {
            // Wait for user to maybe read/see the last info.
            if(MusicPath->dWaitThenCancelTime >= 1.0f) // #Once: Done with _wait and display_
            {
                MusicPath->dWaitThenCancelTime = 0.0f;
                OnMusicPathQuitPressed(&DisplayInfo->MusicBtnInfo);
                SetActive(&MusicPath->LoadingBar, false);
                MusicPath->CrawlThreadStateCount = 0;
            }
            else MusicPath->dWaitThenCancelTime += GlobalGameState.Time.dTime/1.5f;
        } break;
        
        InvalidDefaultCase;
    }
    
}

inline void 
ReplaceFolderPath(mp3_info *MP3Info, string_c *NewPath)
{
    ResetStringCompound(MP3Info->FolderPath);
    AppendStringCompoundToCompound(&MP3Info->FolderPath, NewPath);
}

internal void
LoadNewMetadata_Thread(arena_allocator *ScratchArena, crawl_thread *CrawlInfo)
{
    mp3_info *MP3Info = CrawlInfo->MP3Info;
    
    string_c SubPath = {};
    mp3_file_info Test = {};
    CreateFileInfoStruct(&Test, MAX_MP3_INFO_COUNT);
    FindAllMP3FilesInFolder(ScratchArena, &CrawlInfo->TestPath, &SubPath, &Test);
    CrawlInfo->Out->TestCount = Test.Count;
    CrawlInfo->Out->DoneFolderSearch = true;
    
    if(Test.Count > 0) 
    {
        ReplaceFolderPath(MP3Info, &CrawlInfo->TestPath);
        
        DeleteFileInfoStruct(&MP3Info->FileInfo);
        CreateFileInfoStruct(&MP3Info->FileInfo, MAX_MP3_INFO_COUNT);
        SubPath = {};
        FindAllMP3FilesInFolder(ScratchArena, &MP3Info->FolderPath, &SubPath, &MP3Info->FileInfo);
        
        CrawlFilesForMetadata(ScratchArena, &MP3Info->FileInfo, &MP3Info->FolderPath, &CrawlInfo->Out->CurrentCount);
        CrawlInfo->Out->DoneCrawling = true;
    }
    
    DeleteFileInfoStruct(&Test);
    CrawlInfo->Out->ThreadIsRunning = false;
}


internal JOB_LIST_CALLBACK(JobLoadNewMetadata)
{
    crawl_thread *JobInfo = (crawl_thread *)Data;
    LoadNewMetadata_Thread(&ThreadInfo->ScratchArena, JobInfo);
}

internal b32
AddJob_LoadMetadata(game_state *GameState)
{
    b32 Result = false;
    
    if(!GameState->CrawlInfo.Out->ThreadIsRunning)
    {
        Result = true;
        
        GameState->MusicInfo.IsPlaying = false;
        PushSoundBufferClear(GameState->SoundThreadInterface);
        
        GameState->CrawlInfo.Out->DoneFolderSearch = false;
        GameState->CrawlInfo.Out->DoneCrawling     = false;
        GameState->CrawlInfo.Out->ThreadIsRunning  = true;
        GameState->CrawlInfo.Out->CurrentCount     = 0;
        
        AddJobToQueue(&GameState->JobQueue, JobLoadNewMetadata, GameState->CrawlInfo);
    }
    
    return Result;
}


// *******************************************************
// Job: Update/Check for music library path changed ******
// *******************************************************

internal void
RemoveFileFromInfo(mp3_file_info *FileInfo, u32 RemoveID)
{
    Assert(FileInfo->Count > 0 && RemoveID >= 0);
    Assert(RemoveID < FileInfo->Count);
    RemoveItem(FileInfo->FileName, FileInfo->Count, RemoveID, string_c);
    RemoveItem(FileInfo->SubPath, FileInfo->Count, RemoveID, string_c);
    RemoveItem(FileInfo->Metadata, FileInfo->Count, RemoveID, mp3_metadata);
    FileInfo->Count--;
}

internal b32 
AddFileToInfo(mp3_file_info *FileInfo, string_c *SubPath, string_c *FileName)
{
    b32 Result = false;
    if(FileInfo->Count < FileInfo->MaxCount)
    {
        Result = true;
        FileInfo->SubPath[FileInfo->Count] = NewStringCompound(&GlobalGameState.FixArena, SubPath->Pos);
        AppendStringCompoundToCompound(FileInfo->SubPath+FileInfo->Count, SubPath);
        FileInfo->FileName[FileInfo->Count] = NewStringCompound(&GlobalGameState.FixArena, FileName->Pos);
        AppendStringCompoundToCompound(FileInfo->FileName+FileInfo->Count, FileName);
        
        string_c FilePath = NewStringCompound(&GlobalGameState.ScratchArena, 255);
        ConcatStringCompounds(4, &FilePath, &GlobalGameState.MP3Info->FolderPath, FileInfo->SubPath+FileInfo->Count, 
                              FileInfo->FileName+FileInfo->Count);
        CrawlFileForMetadata(&GlobalGameState.ScratchArena, FileInfo->Metadata+FileInfo->Count, &FilePath);
        DeleteStringCompound(&GlobalGameState.ScratchArena, &FilePath);
        FileInfo->Count++;
    }
    return Result;
}

internal void // Only callded from Main thread
CheckForMusicPathMP3sChanged_End(check_music_path *CheckMusicPath, music_path_ui *MusicPath)
{
    mp3_file_info *FileInfo = &GlobalGameState.MP3Info->FileInfo;
    mp3_file_info *TestInfo = &CheckMusicPath->TestInfo;
    
    For(CheckMusicPath->RemoveIDs.Count)
    {
        RemoveFileFromInfo(FileInfo, Get(&CheckMusicPath->RemoveIDs, It));
    }
    
    if(CheckMusicPath->AddTestInfoIDs.Count > 0)
    {
        ResizeFileInfo(&GlobalGameState.FixArena, FileInfo, FileInfo->MaxCount+CheckMusicPath->AddTestInfoIDs.Count);
    }
    For(CheckMusicPath->AddTestInfoIDs.Count)
    {
        u32 NewIt = Get(&CheckMusicPath->AddTestInfoIDs, It);
        b32 R = AddFileToInfo(FileInfo, TestInfo->SubPath+NewIt, TestInfo->FileName+NewIt);
        Assert(R);
    }
    
    if(CheckMusicPath->RemoveIDs.Count > 0 || CheckMusicPath->AddTestInfoIDs.Count > 0) 
    {
        ApplyNewMetadata(&GlobalGameState, &GlobalGameState.MusicInfo);
        DebugLog(255, "NOTE:: Removed %i songs, added %i songs.\n", CheckMusicPath->RemoveIDs.Count,
                 CheckMusicPath->AddTestInfoIDs.Count)
    }
    else DebugLog(50, "NOTE:: No songs were added or removed!\n");
    
    if(MusicPath->TextField.IsActive)
    {
        ResetStringCompound(MusicPath->OutputString);
        AppendStringToCompound(&MusicPath->OutputString, (u8 *)"Removed ");
        I32ToString(&MusicPath->OutputString, CheckMusicPath->RemoveIDs.Count);
        AppendStringToCompound(&MusicPath->OutputString, (u8 *)" songs and added ");
        I32ToString(&MusicPath->OutputString, CheckMusicPath->AddTestInfoIDs.Count);
        AppendStringToCompound(&MusicPath->OutputString, (u8 *)" songs from old path.");
        
        RemoveRenderText(&GlobalGameState.Renderer, &MusicPath->Output);
        CreateRenderText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, font_Medium, &MusicPath->OutputString,
                         &GlobalGameState.MusicInfo.DisplayInfo.ColorPalette.ForegroundText, &MusicPath->Output, -0.6f-0.001f, MusicPath->TextField.LeftAlign, V2(0, -175));
    }
    
    
    Clear(&CheckMusicPath->AddTestInfoIDs);
    Clear(&CheckMusicPath->RemoveIDs);
    ClearFileInfoStruct(TestInfo);
    
    CheckMusicPath->State = threadState_Inactive;
}

inline b32
IsHigher(i32 T1, i32 T2, void *Data)
{
    array_u32 *Array = (array_u32 *)Data;
    
    return Get(Array, T1) > Get(Array, T2);
}

internal void
CheckForMusicPathMP3sChanged_Thread(arena_allocator *ScratchArena, check_music_path *CheckMusicPath)
{
    string_c SubPath = {};
    mp3_file_info *TestInfo = &CheckMusicPath->TestInfo;
    FindAllMP3FilesInFolder(ScratchArena, &CheckMusicPath->MP3Info->FolderPath, &SubPath, TestInfo);
    
    mp3_file_info *FileInfo = &CheckMusicPath->MP3Info->FileInfo;
    
    // Find which files are missing and remove them from library
    For(FileInfo->Count, MD)
    {
        b32 FileFound = false;
        For(TestInfo->Count)
        {
            if(CompareStringCompounds(FileInfo->SubPath+MDIt, TestInfo->SubPath+It))
            {
                if(CompareStringCompounds(FileInfo->FileName+MDIt, TestInfo->FileName+It))
                {
                    FileFound = true;
                    break;
                }
            }
        }
        
        if(!FileFound) Push(&CheckMusicPath->RemoveIDs, MDIt);
    }
    QuickSort(0, CheckMusicPath->RemoveIDs.Count-1, &CheckMusicPath->RemoveIDs, {IsHigher, &CheckMusicPath->RemoveIDs});
    
    if(TestInfo->Count != FileInfo->Count)
    {
        // Find which files are new and add them to library
        For(TestInfo->Count, New)
        {
            b32 FileFound = false;
            For(FileInfo->Count)
            {
                if(CompareStringCompounds(FileInfo->SubPath+It, TestInfo->SubPath+NewIt))
                {
                    if(CompareStringCompounds(FileInfo->FileName+It, TestInfo->FileName+NewIt))
                    {
                        FileFound = true;
                        break;
                    }
                }
            }
            
            if(!FileFound) Push(&CheckMusicPath->AddTestInfoIDs, NewIt);
        }
    }
    CheckMusicPath->State = threadState_Finished;
}

internal JOB_LIST_CALLBACK(JobCheckMusicPathChanged)
{
    check_music_path *JobInfo = *((check_music_path **)Data);
    CheckForMusicPathMP3sChanged_Thread(&ThreadInfo->ScratchArena, JobInfo);
}

internal void
AddJob_CheckMusicPathChanged(check_music_path *CheckMusicPath)
{
    CheckMusicPath->MP3Info        = GlobalGameState.MP3Info;
    CheckMusicPath->State          = threadState_Running;
    
    AddJobToQueue(&GlobalGameState.JobQueue, JobCheckMusicPathChanged, CheckMusicPath);
}

// ***************************************
// Job: Load and decode files ************
// ***************************************



internal JOB_LIST_CALLBACK(JobLoadAndDecodeMP3File)
{
    job_load_decode_mp3 *JobInfo = (job_load_decode_mp3 *)Data;
    
    
#ifdef DECODE_STREAMING_TMP
    mp3dec_file_info_t *DecodeResult = JobInfo->MP3Info->DecodeInfo.DecodedData + JobInfo->DecodeID;
    Assert(JobInfo->PreloadSeconds == DECODE_PRELOAD_SECONDS);
    
    error_item Result = LoadAndDecodeMP3StartFrames(&ThreadInfo->ScratchArena, JobInfo->PreloadSeconds, JobInfo->FileID, 
                                                    JobInfo->DecodeID, DecodeResult);
#else 
    error_item Result = LoadAndDecodeMP3Data(&ThreadInfo->ScratchArena, JobInfo->MP3Info, 
                                             JobInfo->FileID, JobInfo->DecodeID);
#endif
    
    if(Result.Code < 0) PushErrorMessageFromThread(Result);
}

internal JOB_LIST_CALLBACK(JobLoadAndDecodeEntireMP3File)
{
    job_load_decode_mp3 *JobInfo = (job_load_decode_mp3 *)Data;
    
    mp3dec_file_info_t *DecodeResult = &JobInfo->MP3Info->DecodeInfo.PlayingDecoded.Data;
    error_item Result = LoadAndDecodeMP3StartFrames(&ThreadInfo->ScratchArena, JobInfo->PreloadSeconds, JobInfo->FileID, 
                                                    JobInfo->DecodeID, DecodeResult);
    
    if(Result.Code < 0) PushErrorMessageFromThread(Result);
}

internal i32
AddJob_LoadMP3(circular_job_queue *JobQueue, file_id FileID, 
               array_u32 *IgnoreDecodeIDs, i32 PreloadSeconds)
{
    mp3_info *MP3Info = GlobalGameState.MP3Info;
    if(FileID < 0) return -1;
    
    i32 DecodeID = -1;
    if(!IsSongDecoded(MP3Info, FileID, &DecodeID))
    {
        DecodeID = GetNextDecodeIDToEvict(&MP3Info->DecodeInfo, IgnoreDecodeIDs);
        Assert(DecodeID >= 0);
        if(!MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID])
        {
            // If this gets called twice, the second time IsSongDecoded will 
            // definately be true, even if job is not done. 
            Put(&MP3Info->DecodeInfo.FileID, NewDecodeID(DecodeID), FileID);
            MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = true;
            
            job_load_decode_mp3 Data = {MP3Info, FileID, DecodeID, PreloadSeconds};
            AddJobToQueue(JobQueue, JobLoadAndDecodeMP3File, Data);
        }
        //else Assert(false); // TODO:: Think about killing/interrupting the worker thread to start new job.
    }
    
    Assert(DecodeID >= 0);
    TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
    return DecodeID;
}

internal i32
AddJob_LoadNewPlayingSong(circular_job_queue *JobQueue, file_id FileID)
{
    i32 DecodeID = AddJob_LoadMP3(JobQueue, FileID, 0);
    
    // TODO:: Problem is, that playingDecoded could also still decode the previous song...
    // 1. We could make a job for just waiting until that other decode job is done and then 
    //    start the new one. 
    // 2. Or we can try to interrupt the decode job, but I do not think that
    //    is easily doable as there are only 1-2 procedure calls that take 99% of the time 
    //    (LoadFile and decodeData) and those cant just be stopped. 
    // 3. Another idea would be to kill the worker and spawn a new one. I have no idea if that is 
    //    a viable thing to do, though. We would need to know which is the correct worker.
    // 4. We could also just create another instance of playing_decoded and decode into that.
    //    And after the already running thread is finished decoding, then switch and delete the other.
    // NOTE:: I did Nr. 3. for now. lets see if it works...
    
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    if(DecodeInfo->PlayingDecoded.CurrentlyDecoding)
    {
        DebugLog(235, "NOTE:: Did a Job Kill to load the new playing song!\n");
        FindJobThreadStopAndRestartIt(GlobalGameState.JobHandles, GlobalGameState.JobInfos, JobLoadAndDecodeEntireMP3File);
    }
    
    DecodeInfo->PlayingDecoded.DecodeID = DecodeID;
    DecodeInfo->PlayingDecoded.CurrentlyDecoding = true;
    job_load_decode_mp3 Data = {GlobalGameState.MP3Info, FileID, DecodeID, 1000000};
    AddJobToQueue(JobQueue, JobLoadAndDecodeEntireMP3File, Data);
    
    return DecodeID;
}

internal void
AddJobs_LoadMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs)
{
    mp3_info *MP3Info = GameState->MP3Info;
    play_list *Playlist = &GameState->MusicInfo.Playlist;
    playing_song Song = GameState->MusicInfo.PlayingSong;
    if(Song.PlaylistID < 0) return;
    
    if(Song.PlaylistID >= 0)
    {
        u32 PlaylistSize = Playlist->Songs.A.Count + Playlist->UpNext.A.Count;
        b32 DoNext = true;
        playlist_id CurrentNext = GetPreviousSong(Playlist, Song.PlaylistID);
        playlist_id CurrentPrev = Song.PlaylistID;
        for(u32 It = 0; 
            It < PlaylistSize && It < MAX_MP3_DECODE_COUNT;
            It++)
        {
            if(DoNext)
            {
                CurrentNext.ID = (CurrentNext.ID+1)%Playlist->Songs.A.Count;
                file_id FileID = PlaylistIDToFileID(Playlist, CurrentNext);
                AddJob_LoadMP3(JobQueue, FileID, IgnoreDecodeIDs);
            }
            else
            {
                CurrentPrev = GetPreviousSong(Playlist, CurrentPrev);
                file_id FileID = PlaylistIDToFileID(Playlist, CurrentPrev);
                AddJob_LoadMP3(JobQueue, FileID, IgnoreDecodeIDs);
            }
            DoNext = !DoNext;
        }
    }
}

internal void
AddJobs_LoadOnScreenMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs)
{
    music_display_column *DisplayColumn = &GameState->MusicInfo.DisplayInfo.Song.Base;
    play_list *Playlist = &GameState->MusicInfo.Playlist;
    
    u32 IgnoreCount = 0;
    if(IgnoreDecodeIDs) IgnoreCount = Min(IgnoreDecodeIDs->Count, MAX_MP3_DECODE_COUNT);
    for(u32 It = 0; 
        It < Playlist->Songs.A.Count &&
        It < DisplayColumn->Count && 
        It < MAX_MP3_DECODE_COUNT - IgnoreCount;
        It++)
    {
        file_id FileID = PlaylistIDToFileID(Playlist, NewPlaylistID(DisplayColumn->OnScreenIDs[It]));
        AddJob_LoadMP3(JobQueue, FileID, IgnoreDecodeIDs);
    }
}

internal b32
AddJob_NextUndecodedInPlaylist()
{
    b32 Result = false;
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    if(MusicInfo->PlayingSong.PlaylistID < 0) return Result;
    
    playlist_id StartPlaylistID = NewPlaylistID(0);
    if(MusicInfo->PlayingSong.PlaylistID >= 0) StartPlaylistID = MusicInfo->PlayingSong.PlaylistID;
    
    for(u32 It = StartPlaylistID.ID + 1; 
        It < MusicInfo->Playlist.Songs.A.Count; 
        It++)
    {
        u32 DecodeID = 0;
        playlist_id PID = NewPlaylistID(It);
        if(!Find(&DecodeInfo->FileID, Get(&MusicInfo->Playlist.Songs, PID), &DecodeID))
        {
            Result = true;
            AddJob_LoadMP3(&GlobalGameState.JobQueue, Get(&MusicInfo->Playlist.Songs, PID));
            break;
        }
        else TouchDecoded(DecodeInfo, DecodeID);
    }
    
    // If we did not find anything, meaning nothing needs to be decoded, then we check if
    // we want to loop and try to find something to decode at the beginning of the playlist.
    if(!Result && MusicInfo->Looping == playLoop_Loop)
    {
        Assert(StartPlaylistID.ID >= 0);
        For((u32)StartPlaylistID.ID)
        {
            u32 DecodeID = 0;
            playlist_id PID = NewPlaylistID(It);
            if(!Find(&DecodeInfo->FileID, Get(&MusicInfo->Playlist.Songs, PID), &DecodeID))
            {
                Result = true;
                AddJob_LoadMP3(&GlobalGameState.JobQueue, Get(&MusicInfo->Playlist.Songs, PID));
                break;
            }
            else TouchDecoded(DecodeInfo, DecodeID);
        }
    }
    
    return Result;
}



// ***************************************
// Job error messaging *******************
// ***************************************

internal void
PushErrorMessageFromThread(error_item Error)
{
    WaitForSingleObjectEx(GlobalGameState.ThreadErrorList.Mutex, INFINITE, false);
    if(GlobalGameState.ThreadErrorList.Count < MAX_THREAD_ERRORS)
    {
        GlobalGameState.ThreadErrorList.Errors[GlobalGameState.ThreadErrorList.Count++] = Error;
        GlobalGameState.ThreadErrorList.RemoveDecode = true;
    }
    ReleaseMutex(GlobalGameState.ThreadErrorList.Mutex);
}

internal error_item
PopErrorMessageFromThread()
{
    error_item Result = {loadErrorCode_NoError, {-1}};
    WaitForSingleObjectEx(GlobalGameState.ThreadErrorList.Mutex, INFINITE, false);
    if(GlobalGameState.ThreadErrorList.Count > 0) 
    {
        Result = GlobalGameState.ThreadErrorList.Errors[--GlobalGameState.ThreadErrorList.Count];
    }
    ReleaseMutex(GlobalGameState.ThreadErrorList.Mutex);
    return Result;
}

inline void
RemoveDecodeFails()
{
    WaitForSingleObjectEx(GlobalGameState.ThreadErrorList.Mutex, INFINITE, false);
    
    u32 DecodeID = 0;
    For(GlobalGameState.ThreadErrorList.Count)
    {
        if(Find(&GlobalGameState.MP3Info->DecodeInfo.FileID, GlobalGameState.ThreadErrorList.Errors[It].ID, &DecodeID))
        {
            Put(&GlobalGameState.MP3Info->DecodeInfo.FileID.A, DecodeID, MAX_UINT32);
            Put(&GlobalGameState.MP3Info->DecodeInfo.LastTouched, DecodeID, 0);
            if(GlobalGameState.MusicInfo.PlayingSong.DecodeID == (i32)DecodeID) 
            {
                GlobalGameState.MusicInfo.PlayingSong.PlaylistID.ID = -1;
                GlobalGameState.MusicInfo.PlayingSong.FileID.ID = -1;
                GlobalGameState.MusicInfo.PlayingSong.DecodeID = -1;
            }
        }
    }
    
    GlobalGameState.ThreadErrorList.RemoveDecode = false;
    ReleaseMutex(GlobalGameState.ThreadErrorList.Mutex);
}

internal void
ProcessThreadErrors()
{
    if(GlobalGameState.ThreadErrorList.Count)
    {
        if(GlobalGameState.ThreadErrorList.RemoveDecode) RemoveDecodeFails();
        
        if(!GlobalGameState.MusicInfo.DisplayInfo.UserErrorText.IsAnimating)
        {
            error_item NextError = PopErrorMessageFromThread();
            switch(NextError.Code)
            {
                case loadErrorCode_DecodingFailed:
                {
                    string_c ErrorMsg = NewStringCompound(&GlobalGameState.ScratchArena, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not decode song. Is file corrupted? (");
                    AppendStringCompoundToCompound(&ErrorMsg, GlobalGameState.MP3Info->FileInfo.FileName + NextError.ID.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    PushUserErrorMessage(&ErrorMsg);
                    DeleteStringCompound(&GlobalGameState.ScratchArena, &ErrorMsg);
                } break;
                
                case loadErrorCode_FileLoadFailed:
                {
                    string_c ErrorMsg = NewStringCompound(&GlobalGameState.ScratchArena, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not load song from disk. If files were moved, do a retrace. (");
                    AppendStringCompoundToCompound(&ErrorMsg, GlobalGameState.MP3Info->FileInfo.FileName + NextError.ID.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    PushUserErrorMessage(&ErrorMsg);
                    DeleteStringCompound(&GlobalGameState.ScratchArena, &ErrorMsg);
                } break;
                
                case loadErrorCode_EmptyFile:
                {
                    string_c ErrorMsg = NewStringCompound(&GlobalGameState.ScratchArena, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not load song. File was empty. (");
                    AppendStringCompoundToCompound(&ErrorMsg, GlobalGameState.MP3Info->FileInfo.FileName + NextError.ID.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    PushUserErrorMessage(&ErrorMsg);
                    DeleteStringCompound(&GlobalGameState.ScratchArena, &ErrorMsg);
                } break;
            }
        }
    }
}
