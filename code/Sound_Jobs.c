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
    CrawlInfo->Out->TestCount = Test.Count;
    CrawlInfo->Out->DoneFolderSearch = true;
    
    if(Test.Count > 0) 
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
#define DO_SUB_CRAWL
        timer T = StartTimer();
#ifdef  DO_SUB_CRAWL
        
        // Preparing crawl jobs.
        // Each crawl job will get SUB_CRAWLER_COUNT
        // files that it will process. The amount of
        // crawlers is calculated based on that and
        // the total files to crawl.
#define SUB_CRAWLER_COUNT 400.0f
        r32 SubCrawlerCount = SUB_CRAWLER_COUNT;
        u32 JobCount= Ceiling(Test.Count/SubCrawlerCount);
        if(JobCount > MAX_ACTIVE_JOBS - 2) // Just for the rare case that we have more Jobs than possible
        {
            JobCount = MAX_ACTIVE_JOBS - 2;
            SubCrawlerCount = CeilingR32(Test.Count/(r32)JobCount);
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
            CrawlThreadInfos[It].EndIt        = Min((i32)(SubCrawlerCount + It*SubCrawlerCount), (i32)Test.Count);
            
            // I can only call AddJobToQueue here, inside a thread, because I can guarantee that
            // the main thread is not going to add any jobs, while this one is active.
            AddJobToQueue(&GlobalGameState.JobQueue, JobCrawlPartialMetadata, CrawlThreadInfos[It]);
        }
        
        // Looping until all crawler threads have finished.
        while(CrawlInfo->Out->CurrentCount != Test.Count)
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
                    Assert(CrawlInfo->Out->CurrentCount <= Test.Count);
                }
            }
            Sleep(16);
        }
        Assert(CrawlInfo->Out->CurrentCount == Test.Count);
#else
        CrawlFilesForMetadata(ScratchArena, &MP3Info->FileInfo, 0, MP3Info->FileInfo.Count, &MP3Info->FolderPath, &CrawlInfo->Out->CurrentCount);
#endif
        string_c TText = NewStaticStringCompound("MetadataCrawl");
        SnapTimer(&T, TText);
        
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
        CrawlFileForMetadata(&GlobalGameState.ScratchArena, FileInfo->Metadata+FileInfo->Count, &FilePath,
                             FileInfo->FileName[FileInfo->Count]);
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
        RenderText(&GlobalGameState.Renderer, &GlobalGameState.FixArena, font_Medium, &MusicPath->OutputString,
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

internal error_item
LoadAndDecodeMP3StartFrames(arena_allocator *ScratchArena, i32 SecondsToDecode, file_id FileID, 
                            i32 DecodeID, mp3dec_file_info_t *DecodeResult)
{
    // 1. Build the complete filepath
    // 2. Extract the size of the metadata
    // 3. Decode first frame to get information of the file
    // 4. Calculate maximum size to load for wanted amount
    // 5. Claculate initial buffer size and create it if needed.
    // 6. Load and decode the stuff
    error_item Result = {(load_error_codes)DecodeID, FileID};
    
    mp3_info *MP3Info = GlobalGameState.MP3Info;
    i16 *ExistingBuffer = DecodeResult->buffer;
    u32 PrevSampleCount = (u32)DecodeResult->samples;
    *DecodeResult = {};
    
    // 1.
    NewEmptyLocalString(FilePath, 255);
    ConcatStringCompounds(4, &FilePath, &MP3Info->FolderPath, MP3Info->FileInfo.SubPath + FileID.ID, MP3Info->FileInfo.FileName + FileID.ID);
    
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
            FreeFileMemory(ScratchArena, File.Data);
        }
        else
        {
            Result.Code = loadErrorCode_FileLoadFailed;
            break;
        }
        
        ReadAmount *= 10;
    }
    FreeMemory(ScratchArena, DecodeResult->buffer);
    
    if(Result.Code != loadErrorCode_FileLoadFailed)
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
            
            if(MP3Info->DecodeInfo.CancelDecoding) Result.Code = loadErrorCode_DecodingCanceled;
            else if(DecodeResult->samples == 0)    Result.Code = loadErrorCode_DecodingFailed;
            else
            {
                Put(&MP3Info->DecodeInfo.FileID, NewDecodeID(DecodeID), FileID);
                TouchDecoded(&MP3Info->DecodeInfo, DecodeID);
                DebugLog(255, "NOTE:: Loaded %s.\n", MP3Info->FileInfo.Metadata[FileID.ID].Title.S);
                
                if(SecondsToDecode != DECODE_PRELOAD_SECONDS)
                    CreateSongDurationForMetadata(MP3Info, FileID, DecodeID);
            }
            
            FreeFileMemory(ScratchArena, File.Data);
        }
        else Result.Code = loadErrorCode_FileLoadFailed;
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
        else if((i32)Get(&MP3Info->DecodeInfo.FileID.A, DecodeID) != FileID.ID)
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
AddJob_LoadNewPlayingSong(circular_job_queue *JobQueue, file_id FileID)
{
    i32 DecodeID = AddJob_LoadMP3(JobQueue, FileID, 0);
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
