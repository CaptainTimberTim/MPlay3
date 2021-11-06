#include "ErrorHandling_TD.h"
internal void UpdatePlayingSong(music_info *MusicInfo);
internal void ResetAllDecodedIDs(game_state *GS);

internal void
_PushUserErrorMessage(struct game_state *GS, string_c *String, v2 MessagePosition = {MIN_REAL32, MIN_REAL32})
{
    renderer *Renderer = &GS->Renderer;
    user_error_text *ErrorInfo = &GS->UserErrorText;
    
    // For each output character we extend the visibility time of the message.
    ErrorInfo->AnimTime = 1.0f + String->Pos*0.1f; 
    
    font_size_id FontSize = font_Medium;
    if(String->Pos > 60) FontSize = font_Small;
    
    ErrorInfo->dAnim = 0;
    ErrorInfo->IsAnimating = true;
    if(MessagePosition.x == MIN_REAL32)
    {
        MessagePosition.x = GS->Layout.ErrorMessageX;
        if(FontSize == font_Medium)     MessagePosition.y = -GS->Layout.ErrorMessageMediumTextY;
        else if(FontSize == font_Small) MessagePosition.y = -GS->Layout.ErrorMessageSmallTextY;
    }
    
    RenderText(GS, FontSize, String, &GS->MusicInfo.DisplayInfo.ColorPalette.ErrorText, &ErrorInfo->Message, -0.8f, 0, MessagePosition + V2(0, (r32)Renderer->Window.FixedDim.Height));
    TranslateWithScreen(&Renderer->TransformList, ErrorInfo->Message.Base, fixedTo_TopLeft);
    
    // This needs to happen, because TranslateWithScreen already takes currentDim change into accuont, 
    // thats why we put the absolute position for the text. But that places us in the wrong position,
    // when the screen dim has changed, thats why we set the actual position right here with currentDim.
    SetPosition(&ErrorInfo->Message, MessagePosition + V2(0, (r32)Renderer->Window.CurrentDim.Height));
    SetTransparency(&ErrorInfo->Message, 0);
}

inline void
AnimateErrorMessage(renderer *Renderer, user_error_text *ErrorInfo, r32 dTime)
{
    if(ErrorInfo->IsAnimating)
    {
        if(ErrorInfo->dAnim >= 1.0f)
        {
            SetActive(&ErrorInfo->Message, false);
            RemoveFromTransformList(&Renderer->TransformList, ErrorInfo->Message.Base);
            RemoveRenderText(Renderer, &ErrorInfo->Message);
            ErrorInfo->IsAnimating = false;
        }
        else 
        {
            r32 Alpha = 1-Pow(ErrorInfo->dAnim, 10.0f);
            SetTransparency(&ErrorInfo->Message, Alpha);
            
            ErrorInfo->dAnim += dTime/ErrorInfo->AnimTime;
        }
    }
}

// ***************************************
// Job error messaging *******************
// ***************************************

internal void
PushErrorMessage(struct game_state *GS, error_item Error)
{
    WaitForSingleObjectEx(GS->ThreadErrorList.Mutex, INFINITE, false);
    if(GS->ThreadErrorList.Count < MAX_THREAD_ERRORS)
    {
        GS->ThreadErrorList.Errors[GS->ThreadErrorList.Count++] = Error;
        GS->ThreadErrorList.RemoveDecode = true;
    }
    ReleaseMutex(GS->ThreadErrorList.Mutex);
}

internal void
PushErrorMessage(struct game_state *GS, string_c ErrorText)
{
    WaitForSingleObjectEx(GS->ThreadErrorList.Mutex, INFINITE, false);
    if(GS->ThreadErrorList.Count < MAX_THREAD_ERRORS)
    {
        error_item Error = {errorCode_Text, -1, };
        Error.ErrorText = NewStringCompound(&GS->JobThreadsArena, ErrorText.Pos);
        CopyIntoCompound(&Error.ErrorText, &ErrorText);
        GS->ThreadErrorList.Errors[GS->ThreadErrorList.Count++] = Error;
    }
    ReleaseMutex(GS->ThreadErrorList.Mutex);
}


internal error_item
PopErrorMessageFromThread(struct game_state *GS)
{
    error_item Result = {errorCode_NoError, {-1}};
    WaitForSingleObjectEx(GS->ThreadErrorList.Mutex, INFINITE, false);
    if(GS->ThreadErrorList.Count > 0) 
    {
        Result = GS->ThreadErrorList.Errors[--GS->ThreadErrorList.Count];
    }
    ReleaseMutex(GS->ThreadErrorList.Mutex);
    return Result;
}

inline void
RemoveDecodeFails(struct game_state *GS)
{
    WaitForSingleObjectEx(GS->ThreadErrorList.Mutex, INFINITE, false);
    
    u32 DecodeID = 0;
    For(GS->ThreadErrorList.Count)
    {
        if(GS->ThreadErrorList.Errors[It].Code == errorCode_Text) continue;
        if(Find(&GS->MP3Info->DecodeInfo.FileIDs.A, GS->ThreadErrorList.Errors[It].ID, &DecodeID))
        {
            Put(&GS->MP3Info->DecodeInfo.FileIDs.A, DecodeID, MAX_UINT32);
            Put(&GS->MP3Info->DecodeInfo.LastTouched, DecodeID, 0);
            if(GS->MusicInfo.PlayingSong.DecodeID == (i32)DecodeID) 
            {
                GS->MusicInfo.PlayingSong.DisplayableID.ID = -1;
                GS->MusicInfo.PlayingSong.PlaylistID.ID = -1;
                GS->MusicInfo.PlayingSong.DecodeID = -1;
            }
        }
    }
    
    GS->ThreadErrorList.RemoveDecode = false;
    ReleaseMutex(GS->ThreadErrorList.Mutex);
}

internal void
ProcessThreadErrors(struct game_state *GS)
{
    if(GS->ThreadErrorList.Count)
    {
        // To remove decode fails savely from the main thread (which this is only called from), 
        // we set RemoveDecode on the errorList when a new one occurs. RemoveDecodeFails then
        // goes through the _whole list_ and removes all which need to be decode-removed. 
        if(GS->ThreadErrorList.RemoveDecode) RemoveDecodeFails(GS);
        
        if(!GS->UserErrorText.IsAnimating)
        {
            error_item NextError = PopErrorMessageFromThread(GS);
            
            switch(NextError.Code)
            {
                case errorCode_Text:
                {
                    _PushUserErrorMessage(GS, &NextError.ErrorText);
                    DeleteStringCompound(&GS->JobThreadsArena, &NextError.ErrorText);
                } break;
                
                case errorCode_DecodingFailed:
                {
                    string_c ErrorMsg = NewStringCompound(&GS->ScratchArena, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not decode song. Is file corrupted? (");
                    AppendStringCompoundToCompound(&ErrorMsg, GS->MP3Info->FileInfo.FileNames_ + NextError.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    _PushUserErrorMessage(GS, &ErrorMsg);
                    DeleteStringCompound(&GS->ScratchArena, &ErrorMsg);
                    
                    UpdatePlayingSong(&GS->MusicInfo);
                    ResetAllDecodedIDs(GS);
                    ChangeSong(GS, &GS->MusicInfo.PlayingSong); 
                } break;
                
                case errorCode_FileLoadFailed:
                {
                    string_c ErrorMsg = NewStringCompound(&GS->ScratchArena, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not load song from disk (");
                    AppendStringCompoundToCompound(&ErrorMsg, GS->MP3Info->FileInfo.FileNames_ + NextError.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)").\nFiles were moved, doing a retrace.");
                    _PushUserErrorMessage(GS, &ErrorMsg);
                    DeleteStringCompound(&GS->ScratchArena, &ErrorMsg);
                    
                    UpdatePlayingSong(&GS->MusicInfo);
                    ResetAllDecodedIDs(GS);
                    ChangeSong(GS, &GS->MusicInfo.PlayingSong); 
                    
                    AddJob_CheckMusicPathChanged(&GS->CheckMusicPath);
                } break;
                
                case errorCode_EmptyFile:
                {
                    string_c ErrorMsg = NewStringCompound(&GS->ScratchArena, 555);
                    AppendStringToCompound(&ErrorMsg, (u8 *)"ERROR:: Could not load song. File was empty. (");
                    AppendStringCompoundToCompound(&ErrorMsg, GS->MP3Info->FileInfo.FileNames_ + NextError.ID);
                    AppendStringToCompound(&ErrorMsg, (u8 *)")");
                    _PushUserErrorMessage(GS, &ErrorMsg);
                    DeleteStringCompound(&GS->ScratchArena, &ErrorMsg);
                    
                    UpdatePlayingSong(&GS->MusicInfo);
                    ResetAllDecodedIDs(GS);
                    ChangeSong(GS, &GS->MusicInfo.PlayingSong); 
                } break;
            }
            
        }
    }
}


// *************************************************************************
// Crash handling **********************************************************
// *************************************************************************

#if 0
#include <dbghelp.h>

internal void 
PrintStack()
{
    BOOL              Result;
    HANDLE            Process;
    HANDLE            Thread;
    CONTEXT           Context;
    STACKFRAME64      Stack = {};
    ULONG             Frame;
    IMAGEHLP_SYMBOL64 Symbol;
    DWORD64           Displacement;
    NewEmptyLocalString(Name, MAX_PATH);
    
    RtlCaptureContext(&Context);
    //MemorySet(&Stack, 0, sizeof(STACKFRAME64));
    
    Process                = GetCurrentProcess();
    Thread                 = GetCurrentThread();
    Displacement           = 0;
    Stack.AddrPC.Offset    = Context.Eip;
    Stack.AddrPC.Mode      = AddrModeFlat;
    Stack.AddrStack.Offset = Context.Esp;
    Stack.AddrStack.Mode   = AddrModeFlat;
    Stack.AddrFrame.Offset = Context.Ebp;
    Stack.AddrFrame.Mode   = AddrModeFlat;
    
    for(FrameIt = 0; ; ++FrameIt)
    {
        Result = StackWalk64
        (
         IMAGE_FILE_MACHINE_I386,
         process,
         thread,
         &stack,
         &context,
         NULL,
         SymFunctionTableAccess64,
         SymGetModuleBase64,
         NULL
         );
        
        symbol.SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL64 );
        symbol.MaxNameLength = 255;
        
        SymGetSymFromAddr64( process, ( ULONG64 )stack.AddrPC.Offset, &displacement, &symbol );
        UnDecorateSymbolName( symbol.Name, ( PSTR )name, 256, UNDNAME_COMPLETE );
        
        printf
        (
         "Frame %lu:\n"
         "    Symbol name:    %s\n"
         "    PC address:     0x%08LX\n"
         "    Stack address:  0x%08LX\n"
         "    Frame address:  0x%08LX\n"
         "\n",
         frame,
         symbol.Name,
         ( ULONG64 )stack.AddrPC.Offset,
         ( ULONG64 )stack.AddrStack.Offset,
         ( ULONG64 )stack.AddrFrame.Offset
         );
        
        if( !result )
        {
            break;
        }
    }
}

#endif