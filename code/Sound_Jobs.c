#include "Sound_Jobs.h"

// ***************************************
// Job: Load/Reload metadata *************
// ***************************************

internal void // Only called from Main thread
HandleActiveMusicPath(music_display_info *DisplayInfo, input_info *Input, crawl_thread_out *CrawlInfoOut)
{
    music_path_ui *MusicPath = &DisplayInfo->MusicPath;
    // This procedure is tightly coupled with LoadNewMetadata_Thread.
    
    if(CrawlInfoOut->DoneFolderSearch && MusicPath->CrawlThreadStateCount == 2) 
    {
        // If the crawling thread is done with searching the folder
        if(CrawlInfoOut->TestCount > 0) MusicPath->CrawlThreadStateCount = 5;
        else MusicPath->CrawlThreadStateCount = 4;
    }
    if(CrawlInfoOut->DoneCrawling && MusicPath->CrawlThreadStateCount == 6) 
        MusicPath->CrawlThreadStateCount = 7;
    
    switch(MusicPath->CrawlThreadStateCount)
    {
        case 0: // #Many: Just opened, no thread running yet
        {
            if(Input->KeyChange[KEY_ENTER] == KeyDown) OnMusicPathSavePressed(&DisplayInfo->MusicBtnInfo);
            ProcessMusicPath(&GlobalGameState, GlobalGameState.Time.dTime, Input, &MusicPath->TextField);
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


internal JOB_LIST_CALLBACK(JobCrawlPartialMetadata)
{
    sub_crawl_thread *JobInfo = (sub_crawl_thread *)Data;
    
    CrawlFilesForMetadata(&ThreadInfo->ScratchArena, &JobInfo->CrawlThread.MP3Info->FileInfo, 
                          JobInfo->StartIt, JobInfo->EndIt, &JobInfo->CrawlThread.MP3Info->FolderPath, 
                          JobInfo->CurrentCount);
}


internal void
LoadNewMetadata_Thread(arena_allocator *ScratchArena, crawl_thread *CrawlInfo)
{
    mp3_info *MP3Info = CrawlInfo->MP3Info;
    
    string_c SubPath = {};
    mp3_file_info Test = {};
    CreateFileInfoStruct(&Test, MAX_MP3_INFO_COUNT);
    FindAllMP3FilesInFolder(ScratchArena, &CrawlInfo->TestPath, &SubPath, &Test);
    CrawlInfo->Out->TestCount = Test.Count_;
    CrawlInfo->Out->DoneFolderSearch = true;
    
    if(Test.Count_ > 0) 
    {
        ReplaceFolderPath(MP3Info, &CrawlInfo->TestPath);
        
        DeleteFileInfoStruct(&MP3Info->FileInfo);
        CreateFileInfoStruct(&MP3Info->FileInfo, MAX_MP3_INFO_COUNT);
        SubPath = {};
        FindAllMP3FilesInFolder(ScratchArena, &MP3Info->FolderPath, &SubPath, &MP3Info->FileInfo);
        
        // We split the big job of crawling all the mp3 
        // files into multiple smaller ones. This thread
        // is then only monitoring the sub-threads for 
        // updating the current crawl count, for the loading
        // bar.
        RestartTimer("MetadataCrawl");
#ifdef  DO_METADATA_SUB_CRAWL
        
        // Preparing crawl jobs.
        // Each crawl job will get SUB_CRAWLER_COUNT
        // files that it will process. The amount of
        // crawlers is calculated based on that and
        // the total files to crawl.
#define SUB_CRAWLER_COUNT 400.0f
        r32 SubCrawlerCount = SUB_CRAWLER_COUNT;
        u32 JobCount= Ceiling(Test.Count_/SubCrawlerCount);
        if(JobCount > MAX_ACTIVE_JOBS - 2) // Just for the rare case that we have more Jobs than possible
        {
            JobCount = MAX_ACTIVE_JOBS - 2;
            SubCrawlerCount = CeilingR32(Test.Count_/(r32)JobCount);
        }
        
        DebugLog(250, "Starting %i sub crawler.\n", JobCount);
        sub_crawl_thread *CrawlThreadInfos = AllocateArray(ScratchArena, JobCount, sub_crawl_thread);
        u32 *JobCrawlCount = AllocateArray(ScratchArena, JobCount, u32);
        u32 *JobCountCount = AllocateArray(ScratchArena, JobCount, u32);
        For(JobCount)
        {
            JobCountCount[It] = JobCrawlCount[It] = 0;
            
            CrawlThreadInfos[It].CrawlThread.MP3Info  = MP3Info;
            CrawlThreadInfos[It].CrawlThread.TestPath = MP3Info->FolderPath;
            CrawlThreadInfos[It].CrawlThread.Out      = NULL;
            CrawlThreadInfos[It].CurrentCount = JobCrawlCount+It;
            CrawlThreadInfos[It].StartIt      = (u32)(It*SubCrawlerCount);
            CrawlThreadInfos[It].EndIt        = Min((i32)(SubCrawlerCount + It*SubCrawlerCount), (i32)Test.Count_);
            
            // I can only call AddJobToQueue here, inside a thread, because I can guarantee that
            // the main thread is not going to add any jobs, while this one is active.
            AddJobToQueue(&GlobalGameState.JobQueue, JobCrawlPartialMetadata, CrawlThreadInfos[It]);
        }
        
        // Looping until all crawler threads have finished.
        while(CrawlInfo->Out->CurrentCount != Test.Count_)
        {
            // Count all the crawler thread progresses to know
            // when they are all done _and_ to update the progress
            // bar.
            For(JobCount)
            {
                if(JobCrawlCount[It] > JobCountCount[It])
                {
                    CrawlInfo->Out->CurrentCount += JobCrawlCount[It] - JobCountCount[It];
                    JobCountCount[It] = JobCrawlCount[It];
                    Assert(CrawlInfo->Out->CurrentCount <= Test.Count_);
                }
            }
            Sleep(16);
        }
        Assert(CrawlInfo->Out->CurrentCount == Test.Count_);
#else
        CrawlFilesForMetadata(ScratchArena, &MP3Info->FileInfo, 0, MP3Info->FileInfo.Count, &MP3Info->FolderPath, &CrawlInfo->Out->CurrentCount);
#endif
        SnapTimer("MetadataCrawl");
        
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
RemoveFileFromInfo(music_info *MusicInfo, mp3_file_info *FileInfo, u32 RemoveID)
{
    Assert(FileInfo->Count_ > 0 && RemoveID >= 0);
    Assert(RemoveID < FileInfo->Count_);
    playlist_id PLID = GetPlaylistID(&MusicInfo->Playlist_->Song, NewFileID(RemoveID));
    RemoveItem(FileInfo->FileNames_, FileInfo->Count_, RemoveID, string_c);
    RemoveItem(FileInfo->SubPath, FileInfo->Count_, RemoveID, string_c);
    RemoveItem(FileInfo->Metadata, FileInfo->Count_, RemoveID, mp3_metadata);
    FileInfo->Count_--;
    
    // Reducing the fileIDs for all playlists.
    For(MusicInfo->Playlists.Count)
    {
        playlist_column *PlaylistCol = &MusicInfo->Playlists.List[It].Song;
        For(PlaylistCol->FileIDs.A.Count, File)
        {
            // If it points to a fileId after the removed id, we
            // need to decrease it by one, so that it still points to
            // the same file.
            if(PlaylistCol->FileIDs.A.Slot[FileIt] > RemoveID)
            {
                PlaylistCol->FileIDs.A.Slot[FileIt] -= 1;
            }
        }
    }
    
    // Reducing the playlistIDs for upNext.
    StackFindAndTake(&MusicInfo->UpNextList, PLID);
    For(MusicInfo->UpNextList.A.Count)
    {
        if(MusicInfo->UpNextList.A.Slot[It] > (u32)PLID.ID)
        {
            MusicInfo->UpNextList.A.Slot[It] -= 1;
        }
    }
}

internal b32 
AddFileToInfo(mp3_file_info *FileInfo, string_c *SubPath, string_c *FileName)
{
    b32 Result = false;
    if(FileInfo->Count_ < FileInfo->MaxCount_)
    {
        Result = true;
        FileInfo->SubPath[FileInfo->Count_] = NewStringCompound(&GlobalGameState.FixArena, SubPath->Pos);
        AppendStringCompoundToCompound(FileInfo->SubPath+FileInfo->Count_, SubPath);
        FileInfo->FileNames_[FileInfo->Count_] = NewStringCompound(&GlobalGameState.FixArena, FileName->Pos);
        AppendStringCompoundToCompound(FileInfo->FileNames_+FileInfo->Count_, FileName);
        
        string_c FilePath = NewStringCompound(&GlobalGameState.ScratchArena, 255);
        ConcatStringCompounds(4, &FilePath, &GlobalGameState.MP3Info->FolderPath, FileInfo->SubPath+FileInfo->Count_, 
                              FileInfo->FileNames_+FileInfo->Count_);
        CrawlFileForMetadata(&GlobalGameState.ScratchArena, FileInfo->Metadata+FileInfo->Count_, &FilePath,
                             FileInfo->FileNames_[FileInfo->Count_]);
        DeleteStringCompound(&GlobalGameState.ScratchArena, &FilePath);
        FileInfo->Count_++;
    }
    return Result;
}

internal void // Only callded from Main thread
CheckForMusicPathMP3sChanged_End(check_music_path *CheckMusicPath, music_path_ui *MusicPath)
{
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    mp3_file_info *FileInfo = &GlobalGameState.MP3Info->FileInfo;
    mp3_file_info *TestInfo = &CheckMusicPath->TestInfo;
    
    For(CheckMusicPath->RemoveIDs.Count)
    {
        u32 RemoveID = Get(&CheckMusicPath->RemoveIDs, It);
        RemoveFileFromInfo(MusicInfo, FileInfo, RemoveID);
    }
    
    if(CheckMusicPath->AddTestInfoIDs.Count > 0)
    {
        ResizeFileInfo(FileInfo, FileInfo->MaxCount_+CheckMusicPath->AddTestInfoIDs.Count);
    }
    For(CheckMusicPath->AddTestInfoIDs.Count)
    {
        u32 NewIt = Get(&CheckMusicPath->AddTestInfoIDs, It);
        b32 R = AddFileToInfo(FileInfo, TestInfo->SubPath+NewIt, TestInfo->FileNames_+NewIt);
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
        RenderText(&GlobalGameState, font_Medium, &MusicPath->OutputString,
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
    array_u32 *AddArray  = &CheckMusicPath->AddTestInfoIDs;
    array_u32 *RemoveArray  = &CheckMusicPath->RemoveIDs;
    For(FileInfo->Count_, MD)
    {
        b32 FileFound = false;
        For(TestInfo->Count_)
        {
            if(CompareStringCompounds(FileInfo->SubPath+MDIt, TestInfo->SubPath+It))
            {
                if(CompareStringCompounds(FileInfo->FileNames_+MDIt, TestInfo->FileNames_+It))
                {
                    FileFound = true;
                    break;
                }
            }
        }
        
        if(!FileFound) 
        {
            if(RemoveArray->Count >= RemoveArray->Length)
            {
                u32 NewCount      = RemoveArray->Length*2;
                RemoveArray->Slot = ReallocateArray(&GlobalGameState.JobThreadsArena, RemoveArray->Slot, 
                                                    RemoveArray->Length, NewCount, u32);
                RemoveArray->Length = NewCount;
            }
            Push(RemoveArray, MDIt);
        }
    }
    QuickSort(0, CheckMusicPath->RemoveIDs.Count-1, &CheckMusicPath->RemoveIDs, {IsHigher, &CheckMusicPath->RemoveIDs, Switch});
    
    // Find which files are new and add them to library
    For(TestInfo->Count_, New)
    {
        b32 FileFound = false;
        For(FileInfo->Count_)
        {
            if(CompareStringCompounds(FileInfo->SubPath+It, TestInfo->SubPath+NewIt))
            {
                if(CompareStringCompounds(FileInfo->FileNames_+It, TestInfo->FileNames_+NewIt))
                {
                    FileFound = true;
                    break;
                }
            }
        }
        
        if(!FileFound) 
        {
            if(AddArray->Count >= AddArray->Length)
            {
                u32 NewCount   = AddArray->Length*2;
                AddArray->Slot = ReallocateArray(&GlobalGameState.JobThreadsArena, AddArray->Slot, 
                                                 AddArray->Length, NewCount, u32);
                AddArray->Length = NewCount;
            }
            Push(AddArray, NewIt);
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
    if(CheckMusicPath->State == threadState_Inactive)
    {
        CheckMusicPath->State = threadState_Running;
        
        if(CheckMusicPath->MP3Info == NULL)
        {
            CreateFileInfoStruct(&CheckMusicPath->TestInfo, 150);
            CheckMusicPath->MP3Info        = GlobalGameState.MP3Info;
            CheckMusicPath->RemoveIDs      = CreateArray(&GlobalGameState.JobThreadsArena, 150);
            CheckMusicPath->AddTestInfoIDs = CreateArray(&GlobalGameState.JobThreadsArena, 150);
        }
        
        AddJobToQueue(&GlobalGameState.JobQueue, JobCheckMusicPathChanged, CheckMusicPath);
    }
}

// ***************************************
// Job: Load and decode files ************
// ***************************************

internal void
DecodeMP3StartFrames(arena_allocator *Arena, mp3dec_t *MP3Decoder, read_file_result File, mp3dec_file_info_t *DecodeResult,
                     u32 BufferSize, u32 DecodeFrameAmount, b32 *BreakOnTrue, 
                     b32 NoFinalResize = false, i32 *ConsumedBytes = 0)
{
    /*typedef struct
   {
       int frame_bytes;
       int channels;
       int hz;
       int layer;
       int bitrate_kbps;
   } mp3dec_frame_info_t;*/
    mp3dec_frame_info_t FrameInfo;
    
    u32 FrameCount = 0;
    For(DecodeFrameAmount)
    {
        // Allocate more memory if necessary.
        if((BufferSize - DecodeResult->samples) < MINIMP3_MAX_SAMPLES_PER_FRAME)
        {
            DecodeResult->buffer = ReallocateArray(Arena, DecodeResult->buffer, BufferSize, BufferSize*2, i16, Private);
            BufferSize *= 2;
            DebugLog(255, "Resized Buffer, new sample count: %i\n", BufferSize);
        }
        
        // mp3dec_decode_frame result:
        // 0:    No MP3 data was found in the input buffer
        // 384:  Layer 1
        // 576:  MPEG 2 Layer 3
        // 1152: Otherwise
        u32 SampleCount = mp3dec_decode_frame(MP3Decoder, File.Data, File.Size, 
                                              DecodeResult->buffer+DecodeResult->samples, &FrameInfo);
        
        if(SampleCount > 0 && FrameInfo.frame_bytes > 0) // Succesfull decode.
        {
            File.Data += FrameInfo.frame_bytes;
            File.Size -= FrameInfo.frame_bytes;
            DecodeResult->samples += SampleCount*FrameInfo.channels;
            DecodeResult->avg_bitrate_kbps += FrameInfo.bitrate_kbps;
            FrameCount++;
            
            if(It == 0)
            {
                DecodeResult->channels = FrameInfo.channels;
                DecodeResult->hz = FrameInfo.hz;
                DecodeResult->layer = FrameInfo.layer;
            }
            Assert((MINIMP3_MAX_SAMPLES_PER_FRAME*DecodeFrameAmount) >= DecodeResult->samples);
        }
        else if(SampleCount == 0 && FrameInfo.frame_bytes > 0) // Decoder skipped ID3/invalid data (no samples were decoded).
        {
            File.Data += FrameInfo.frame_bytes;
            File.Size -= FrameInfo.frame_bytes;
            It--; // Try again to decode frame.
        }
        else if(SampleCount == 0 && FrameInfo.frame_bytes == 0) break; // Nothing was there to be decoded or skipped. EOF
        else Assert(false); // Double Assert just to know if there can be another case I did not catch.
        
        if(ConsumedBytes) *ConsumedBytes += FrameInfo.frame_bytes;
        if(*BreakOnTrue) break;
    }
    DecodeResult->avg_bitrate_kbps = SafeDiv(DecodeResult->avg_bitrate_kbps, FrameCount);
    
    // Fit buffer to actual size
    if(!NoFinalResize && BufferSize != DecodeResult->samples)
    {
        DecodeResult->buffer = ReallocateArray(Arena, DecodeResult->buffer, BufferSize, 
                                               DecodeResult->samples, i16, Private);
    }
}

internal error_item // #ThreadedUse
LoadAndDecodeMP3StartFrames(arena_allocator *ScratchArena, i32 SecondsToDecode, file_id FileID, 
                            i32 DecodeID, mp3dec_file_info_t *DecodeResult)
{
    // 1. Build the complete filepath
    // 2. Extract the size of the metadata
    // 3. Decode first frame to get information of the file
    // 4. Calculate maximum size to load for wanted amount
    // 5. Calculate initial buffer size and create it if needed.
    // 6. Load and decode the stuff
    error_item Result = {(error_codes)DecodeID, FileID.ID};
    
    mp3_info *MP3Info = GlobalGameState.MP3Info;
    i16 *ExistingBuffer = DecodeResult->buffer;
    u32 PrevSampleCount = (u32)DecodeResult->samples;
    *DecodeResult = {};
    
    // 1.
    NewEmptyLocalString(FilePath, 255);
    ConcatStringCompounds(4, &FilePath, &MP3Info->FolderPath, MP3Info->FileInfo.SubPath + FileID.ID, MP3Info->FileInfo.FileNames_ + FileID.ID);
    
    mp3dec_t MP3Decoder = {};
    mp3dec_init(&MP3Decoder);
    
    // 2.
    DecodeResult->buffer = AllocateArray(ScratchArena, MINIMP3_MAX_SAMPLES_PER_FRAME, i16); // Temp buffer for one frame
    u32 MetadataSize    = ExtractMetadataSize(ScratchArena, &FilePath);
    i32 ConsumedBytes   = 0; // Is taken for the other load to alleviate MetadataSize being wrong sometimes.
    u32 ReadAmount      = MetadataSize + MINIMP3_MAX_SAMPLES_PER_FRAME + 512;
    b32 StubBreakOn     = false;
    
    read_file_result File = {};
    // 3. 
    // Because ExtractMetadataSize is sometimes not correct (must be because of wrong size information
    // in the ID3 header), we loop until we get our first frame and increase the amount we read from the
    // file every time.
    while(!DecodeResult->channels)
    {
        if(ReadBeginningOfFile(ScratchArena, &File, FilePath.S, ReadAmount))
        {
            DecodeMP3StartFrames(ScratchArena, &MP3Decoder, File, DecodeResult, 
                                 MINIMP3_MAX_SAMPLES_PER_FRAME, 1, &StubBreakOn, true, &ConsumedBytes);
            FreeFileMemory(ScratchArena, File);
        }
        else
        {
            Result.Code = errorCode_FileLoadFailed;
            break;
        }
        
        ReadAmount *= 10;
    }
    FreeMemory(ScratchArena, DecodeResult->buffer);
    DecodeResult->buffer = NULL;
    
    if(Result.Code != errorCode_FileLoadFailed)
    {
        Assert(DecodeResult->channels);
        Assert(DecodeResult->hz);
        
        u32 DecodeSampleAmount = DecodeResult->hz*SecondsToDecode; 
        // samples has channels already in it. But I need the frame amount which is channel indipendent.
        u32 DecodeFrameAmount  = DecodeSampleAmount/(u32)(DecodeResult->samples/DecodeResult->channels);
        // 4. This should always be enough, as the mp3 file should be smaller, as it is compressed...
        ReadAmount = DecodeSampleAmount*DecodeResult->channels*sizeof(i16) + DecodeFrameAmount*sizeof(u32) + ConsumedBytes; 
        if(ReadBeginningOfFile(ScratchArena, &File, FilePath.S, ReadAmount))
        {
            // 5.
            u32 InitialSampleCount = PrevSampleCount;
            Assert((ExistingBuffer && PrevSampleCount) || (!ExistingBuffer && !PrevSampleCount));
            
            b32 *CancelDecode = &StubBreakOn;
            if(SecondsToDecode != DECODE_PRELOAD_SECONDS) 
            {
                // For full songs we use the last buffersize or, if none exists, start with five minutes worth 
                // of space for the song. Seems like a good middle, for most songs to not need to reallocate.
                // Some need to do it once or max twice (like TOOL songs).
                if(!InitialSampleCount) InitialSampleCount = DecodeResult->hz*DecodeResult->channels*300;
                if(!ExistingBuffer)
                    ExistingBuffer = AllocateArray(&GlobalGameState.JobThreadsArena, InitialSampleCount, i16, Private);
                CancelDecode = (b32 *)&GlobalGameState.MP3Info->DecodeInfo.CancelDecoding;
            }
            else 
            {
                if(!InitialSampleCount) InitialSampleCount = DecodeFrameAmount*(u32)DecodeResult->samples;
                if(!ExistingBuffer) ExistingBuffer = AllocateArray(&GlobalGameState.JobThreadsArena, InitialSampleCount, i16);
            }
            Assert(ExistingBuffer);
            
            // 6.
            DecodeResult->buffer = ExistingBuffer;
            DecodeResult->samples = 0;
            DecodeMP3StartFrames(&GlobalGameState.JobThreadsArena, &MP3Decoder, File, DecodeResult, 
                                 InitialSampleCount, DecodeFrameAmount, CancelDecode);
            
            if(MP3Info->DecodeInfo.CancelDecoding) Result.Code = errorCode_DecodingCanceled;
            else if(DecodeResult->samples == 0)    Result.Code = errorCode_DecodingFailed;
            else
            {
                Put(&MP3Info->DecodeInfo.FileIDs, NewDecodeID(DecodeID), FileID);
                TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
                DebugLog(255, "Loaded %s.\n", MP3Info->FileInfo.Metadata[FileID.ID].Title.S);
                
                if(SecondsToDecode != DECODE_PRELOAD_SECONDS)
                    CreateSongDurationForMetadata(MP3Info, FileID.ID, DecodeID);
            }
            
            FreeFileMemory(ScratchArena, File);
        }
        else Result.Code = errorCode_FileLoadFailed;
    }
    
    if(DecodeID >= 0) 
    {
        if(SecondsToDecode == DECODE_PRELOAD_SECONDS) MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = false;
        else MP3Info->DecodeInfo.PlayingDecoded.CurrentlyDecoding = false;
    }
    return Result;
}


internal JOB_LIST_CALLBACK(JobLoadAndDecodeMP3File)
{
    job_load_decode_mp3 *JobInfo = (job_load_decode_mp3 *)Data;
    
    mp3dec_file_info_t *DecodeResult = JobInfo->MP3Info->DecodeInfo.DecodedData + JobInfo->DecodeID;
    Assert(JobInfo->PreloadSeconds == DECODE_PRELOAD_SECONDS);
    
    error_item Result = LoadAndDecodeMP3StartFrames(&ThreadInfo->ScratchArena, JobInfo->PreloadSeconds, JobInfo->FileID, 
                                                    JobInfo->DecodeID, DecodeResult);
    
    if(Result.Code < 0) PushErrorMessage(&GlobalGameState, Result);
}

internal JOB_LIST_CALLBACK(JobLoadAndDecodeEntireMP3File)
{
    job_load_decode_mp3 *JobInfo = (job_load_decode_mp3 *)Data;
    
    mp3dec_file_info_t *DecodeResult = &JobInfo->MP3Info->DecodeInfo.PlayingDecoded.Data;
    error_item Result = LoadAndDecodeMP3StartFrames(&ThreadInfo->ScratchArena, JobInfo->PreloadSeconds, JobInfo->FileID, 
                                                    JobInfo->DecodeID, DecodeResult);
    
    if(Result.Code < 0) PushErrorMessage(&GlobalGameState, Result);
}

internal i32
AddJob_LoadMP3(circular_job_queue *JobQueue, playlist_id PlaylistID, 
               array_u32 *IgnoreDecodeIDs, i32 PreloadSeconds)
{
    mp3_info *MP3Info = GlobalGameState.MP3Info;
    if(PlaylistID < 0) return -1;
    
    i32 DecodeID = -1;
    // PlaylistID needs to be mapped before the multithreaded code to avoid
    // accessing the playlist at that stage. @PlaylistChange
    file_id FileID = GetFileID(&MP3Info->MusicInfo->Playlist_->Song, PlaylistID);
    if(!IsSongDecoded(MP3Info, FileID, &DecodeID))
    {
        DecodeID = GetNextDecodeIDToEvict(&MP3Info->DecodeInfo, IgnoreDecodeIDs);
        Assert(DecodeID >= 0);
        if(!MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID])
        {
            // If this gets called twice, the second time IsSongDecoded will 
            // definately be true, even if job is not done. 
            Put(&MP3Info->DecodeInfo.FileIDs, NewDecodeID(DecodeID), FileID);
            MP3Info->DecodeInfo.CurrentlyDecoding[DecodeID] = true;
            
            job_load_decode_mp3 Data = {MP3Info, FileID, DecodeID, PreloadSeconds};
            AddJobToQueue(JobQueue, JobLoadAndDecodeMP3File, Data);
        }
        else if((i32)Get(&MP3Info->DecodeInfo.FileIDs.A, DecodeID) != FileID.ID)
        {
            DecodeID = -1;
            DebugLog(255, "Tried to add new song, while all slots were still decoding.\n");
        }
        else Assert(false); 
    }
    
    if(DecodeID >= 0) TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
    return DecodeID;
}

internal i32
AddJob_LoadNewPlayingSong(circular_job_queue *JobQueue, playlist_id PlaylistID)
{
    i32 DecodeID = AddJob_LoadMP3(JobQueue, PlaylistID, 0);
    Assert(DecodeID >= 0);
    
    // The strategy for starting to load another song while the old one is still
    // decoding is to have the job check every decode-frame if CancelDecoding is
    // set. If so, stop immidiately. The main thread loops until that happened, 
    // which is very quick, and then proceed as normal.
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    if(DecodeInfo->PlayingDecoded.CurrentlyDecoding)
    {
        DebugLog(235, "NOTE:: Canceled previous loading of song to load the new playing song!\n");
        
        DecodeInfo->CancelDecoding = true;
        while(DecodeInfo->PlayingDecoded.CurrentlyDecoding) Sleep(1); // Loop 1 ms between checks. Seems fine for now.
        DecodeInfo->CancelDecoding = false;
    }
    
    DecodeInfo->PlayingDecoded.DecodeID = DecodeID;
    DecodeInfo->PlayingDecoded.CurrentlyDecoding = true;
    
    // PlaylistID needs to be mapped before the multithreaded code to avoid
    // accessing the playlist at that stage. @PlaylistChange
    file_id FileID = GetFileID(&GlobalGameState.MusicInfo.Playlist_->Song, PlaylistID);
    job_load_decode_mp3 Data = {GlobalGameState.MP3Info, FileID, DecodeID, 1000000};
    AddJobToQueue(JobQueue, JobLoadAndDecodeEntireMP3File, Data);
    
    return DecodeID;
}

internal void
AddJobs_LoadMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs)
{
    mp3_info *MP3Info = GameState->MP3Info;
    playing_song Song = GameState->MusicInfo.PlayingSong;
    if(Song.DisplayableID < 0) return;
    
    if(Song.DisplayableID >= 0)
    {
        u32 DisplayableCount  = GameState->MusicInfo.Playlist_->Song.Displayable.A.Count;
        u32 PlaylistSize      = DisplayableCount + GameState->MusicInfo.UpNextList.A.Count;
        b32 DoNext = true;
        displayable_id CurrentNext = GetPreviousSong(DisplayableCount, Song.DisplayableID);
        displayable_id CurrentPrev = Song.DisplayableID;
        for(u32 It = 0; 
            It < PlaylistSize && It < MAX_MP3_DECODE_COUNT;
            ++It)
        {
            if(DoNext)
            {
                CurrentNext.ID = (CurrentNext.ID+1)%DisplayableCount;
                playlist_id PlaylistID = GetPlaylistID(MP3Info->MusicInfo, CurrentNext);
                AddJob_LoadMP3(JobQueue, PlaylistID, IgnoreDecodeIDs);
            }
            else
            {
                CurrentPrev    = GetPreviousSong(DisplayableCount, CurrentPrev);
                playlist_id PlaylistID = GetPlaylistID(MP3Info->MusicInfo, CurrentPrev);
                AddJob_LoadMP3(JobQueue, PlaylistID, IgnoreDecodeIDs);
            }
            DoNext = !DoNext;
        }
    }
}

internal void
AddJobs_LoadOnScreenMP3s(game_state *GameState, circular_job_queue *JobQueue, array_u32 *IgnoreDecodeIDs)
{
    display_column *DisplayColumn = &GameState->MusicInfo.DisplayInfo.Song.Base;
    
    u32 IgnoreCount = 0;
    if(IgnoreDecodeIDs) IgnoreCount = Min((i32)IgnoreDecodeIDs->Count, MAX_MP3_DECODE_COUNT);
    for(u32 It = 0; 
        It < GameState->MusicInfo.Playlist_->Song.Displayable.A.Count &&
        It < DisplayColumn->Count && 
        It < MAX_MP3_DECODE_COUNT - IgnoreCount;
        It++)
    {
        playlist_id PlaylistID = GetPlaylistID(&GameState->MusicInfo, DisplayColumn->SlotIDs[It]);
        AddJob_LoadMP3(JobQueue, PlaylistID, IgnoreDecodeIDs);
    }
}

internal b32
AddJob_NextUndecodedInPlaylist()
{
    b32 Result = false;
    music_info *MusicInfo = &GlobalGameState.MusicInfo;
    mp3_decode_info *DecodeInfo = &GlobalGameState.MP3Info->DecodeInfo;
    if(MusicInfo->PlayingSong.DisplayableID < 0) return Result;
    
    displayable_id StartDisplayableID = NewDisplayableID(0);
    if(MusicInfo->PlayingSong.DisplayableID >= 0) StartDisplayableID = MusicInfo->PlayingSong.DisplayableID;
    
    array_playlist_id *Displayable = &MusicInfo->Playlist_->Song.Displayable;
    for(u32 It = StartDisplayableID.ID + 1; 
        It < Displayable->A.Count; 
        It++)
    {
        u32 DecodeID = 0;
        displayable_id DisplayableID = NewDisplayableID(It);
        file_id FileID = GetFileID(MusicInfo, DisplayableID);
        if(!Find(&DecodeInfo->FileIDs, FileID, &DecodeID))
        {
            Result = true;
            AddJob_LoadMP3(&GlobalGameState.JobQueue, Get(Displayable, DisplayableID));
            break;
        }
        else TouchDecoded(DecodeInfo, DecodeID);
    }
    
    // If we did not find anything, meaning nothing needs to be decoded, then we check if
    // we want to loop and try to find something to decode at the beginning of the playlist.
    if(!Result && MusicInfo->Playlist_->Looping == playLoop_Loop)
    {
        Assert(StartDisplayableID.ID >= 0);
        For((u32)StartDisplayableID.ID)
        {
            u32 DecodeID = 0;
            displayable_id PID = NewDisplayableID(It);
            file_id FileID    = GetFileID(MusicInfo, PID);
            if(!Find(&DecodeInfo->FileIDs, FileID, &DecodeID))
            {
                Result = true;
                AddJob_LoadMP3(&GlobalGameState.JobQueue, Get(Displayable, PID));
                break;
            }
            else TouchDecoded(DecodeInfo, DecodeID);
        }
    }
    
    return Result;
}


