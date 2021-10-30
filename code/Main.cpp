#include <Shlobj.h>
#include <windows.h>

#include "Definitions_TD.h"
#if RESOURCE_FILE==0
#include "..\\data\\resources\\EmbeddedResourcesSTUB.h"
#elif RESOURCE_FILE==1
#include "..\\data\\resources\\EmbeddedResourcesExactSize_png.h"
#else
#include "..\\data\\resources\\EmbeddedResourcesExactSize_huf.h"
#endif

#define MINIMP3_IMPLEMENTATION
#include "Libraries\\MiniMP3.h"

// TODO:: Implement STBI_MALLOC, STBI_REALLOC and STBI_FREE!
#define STB_IMAGE_IMPLEMENTATION
#include "Libraries\\STB_Image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "Libraries\\STB_Rect_Pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "Libraries\\STB_Truetype.h"

#if DEBUG_TD
#define STBI_ASSERT(x) if(!(x)) {*(int *)0 = 0;}
#else
#define STBI_ASSERT(x)
#endif

struct time_management
{
    i32 GameHz;
    r32 GoalFrameRate;
    r32 dTime;
    r64 GameTime;
    r32 CurrentTimeSpeed;
    
    i64 PerfCountFrequency;
    b32 SleepIsGranular;
};

#include "Math_TD.h"
#include "Allocator_TD.c"

internal b32 TryGetClipboardText(struct string_compound *String);
internal void ApplyWindowResize(HWND Window, i32 NewWidth, i32 NewHeight, b32 ForceResize = false);
inline v2i GetWindowSize();

#include "Input_TD.c"
#include "String_TD.h"
#include "StandardUtilities_TD.c"
#include "Threading_TD.c"
#include "Math_TD.c"
#include "FileUtilities_TD.c"
#include "GL_TD.c"
#include "Font_TD.c"
#include "Renderer_TD.c"
#include "ErrorHandling_TD.c"

global_variable game_state GlobalGameState;
global_variable i32 GlobalMinWindowWidth  = 344;
global_variable i32 GlobalMinWindowHeight = 190;

#include "Sound_Thread_TD.c"
#include "Sound_UI_TD.c"
#include "Sound_Backend_TD.c"
#include "Sound_Jobs.c"
#include "Sound_Serialization.c"
#include "UI_TD.c"
#include "GameBasics_TD.c"
#include "Sound_Settings.c"

global_variable b32 IsRunning;
HCURSOR ArrowCursor = 0;
HCURSOR DragCursor = 0;

internal void
WindowGotResized(game_state *GameState)
{
    if(GameState->Renderer.Window.GotResized)
    {
        renderer *Renderer = &GameState->Renderer;
        music_display_info *DisplayInfo = &GameState->MusicInfo.DisplayInfo;
        
        PerformScreenTransform(Renderer);
        ProcessEdgeDragOnResize(Renderer, DisplayInfo);
        
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &DisplayInfo->Playlists);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &DisplayInfo->Genre);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &DisplayInfo->Artist);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &DisplayInfo->Album);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &DisplayInfo->Song.Base);
        
        SetActive(&DisplayInfo->Quit, false);
        DisplayInfo->Quit.dAnim = 0;
        AnimateErrorMessage(Renderer, &GameState->UserErrorText, 0);
        
        Renderer->Window.GotResized = false;
    }
    
}

internal void
ApplyWindowResize(HWND Window, i32 NewWidth, i32 NewHeight, b32 ForceResize)
{
    window_info *WinInfo = &GlobalGameState.Renderer.Window;
    
    RECT WRect = {};
    GetWindowRect(Window, &WRect);
    if(NewWidth < GlobalMinWindowWidth || NewHeight < GlobalMinWindowHeight || ForceResize)
    {
        NewWidth  = (NewWidth < GlobalMinWindowWidth) ? GlobalMinWindowWidth : NewWidth;
        NewHeight = (NewHeight < GlobalMinWindowHeight) ? GlobalMinWindowHeight : NewHeight;
        
        if(SetWindowPos(Window, HWND_TOP, WRect.left, WRect.top, NewWidth, NewHeight, 0))
        {
            // Stopped from making window too small
        }
    }
    
    RECT Rect = {};
    if(GetClientRect(Window, &Rect)) // The client has the window borders included. 
    {
        WinInfo->CurrentDim.Width  = Rect.right - Rect.left;
        WinInfo->CurrentDim.Height = Rect.bottom - Rect.top;
    }
    
    WinInfo->CurrentAspect = (r32)WinInfo->CurrentDim.Width/(r32)WinInfo->CurrentDim.Height;
    WinInfo->CurrentReverseAspect = (r32)WinInfo->CurrentDim.Height/(r32)WinInfo->CurrentDim.Width;
    
    GlobalGameState.Renderer.Window.GotResized = true;
    GlobalGameState.Renderer.Rerender = true;
    ReshapeGLWindow(&GlobalGameState.Renderer);
    WindowGotResized(&GlobalGameState);
    
    HDC DeviceContext = GetDC(Window);
    DisplayBufferInWindow(DeviceContext, &GlobalGameState.Renderer);
    
    ReleaseDC(Window, DeviceContext);
}

inline v2i
GetWindowSize()
{
    v2i Result = {};
    RECT Rect = {};
    GetWindowRect(GlobalGameState.Renderer.Window.WindowHandle, &Rect);
    
    Result.x = Rect.right - Rect.left;
    Result.y = Rect.bottom - Rect.top;
    
    return Result;
}

internal LRESULT CALLBACK
WindowCallback(HWND Window,
               UINT Message,
               WPARAM WParam,
               LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_SIZE: 
        {
            if(IsRunning) 
            {
                switch(WParam)
                {
                    case SIZE_MAXHIDE:
                    {
                        DebugLog(255, "Message is sent to all pop-up windows when some other window is maximized.\n"); 
                    } break;
                    case SIZE_MAXSHOW:
                    {
                        DebugLog(255, "Message is sent to all pop-up windows when some other window has been restored to its former size.\n"); 
                    } break;
                    case SIZE_MINIMIZED:
                    {
                        GlobalGameState.Renderer.Minimized = true;
                    } break;
                    case SIZE_MAXIMIZED:
                    case SIZE_RESTORED:
                    {
                        GlobalGameState.Renderer.Minimized = false;
                        
                        RECT WRect = {};
                        GetWindowRect(Window, &WRect);
                        i32 WWidth  = WRect.right - WRect.left;
                        i32 WHeight = WRect.bottom - WRect.top;
                        
                        ApplyWindowResize(Window, WWidth, WHeight);
                        
                    } break;
                }
            }
            
        } break;
        case WM_DESTROY:
        case WM_CLOSE: 
        { 
            quit_animation *Quit = &GlobalGameState.MusicInfo.DisplayInfo.Quit;
            SetActive(Quit, true);
            Quit->WindowExit = true;
            Quit->Time *= 1.0f/GlobalGameState.Layout.QuitCurtainAnimationMultiplies;
        } break;
        case WM_ACTIVATEAPP: {} break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {} break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            DisplayBufferInWindow(DeviceContext, &GlobalGameState.Renderer);
            SwapBuffers(DeviceContext);
            EndPaint(Window, &Paint);
            ReleaseDC(Window, DeviceContext);
        } break;
        case WM_SETCURSOR: 
        {
            if (GlobalGameState.CursorState == cursorState_Arrow) 
                Result = DefWindowProc(Window, Message, WParam, LParam);
            else if(GlobalGameState.CursorState == cursorState_Drag) 
                SetCursor(DragCursor);
        } break;
        case WM_SETFOCUS:
        {
            HandleFocusRegain(&GlobalGameState.Input);
        } break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

internal void
ProcessPendingMessages(input_info *Input, HWND Window)
{
    b32 PrevMouseMove = Input->_MouseMoved;
    ResetKeys(Input);
    
    MSG Message = {};
    while(PeekMessageW(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT: { IsRunning = false; } break;
            case WM_HOTKEY:
            {
                HandleHotkey(Input, (i32)Message.wParam, Message.lParam);
            } break;
            case WM_CHAR: 
            {
                // Converts the incomming utf-16 to utf-8.
                u8 UTF8[4] = {};
                i32 Count = ArrayCount(UTF8);
                // This should never happening as the docu says, but how do, for instance, 
                // Chinese traditional characters fit?
                Assert(Message.wParam <= MAX_UINT16);
                ConvertChar16To8((wchar_t)Message.wParam, UTF8, &Count);
                UpdateTypedCharacters(Input, UTF8, Count);
            } break;
            case WM_SYSKEYDOWN: 
            case WM_SYSKEYUP: 
            case WM_KEYDOWN: 
            case WM_KEYUP:
            {
                u32 Scancode = (Message.lParam & 0x00ff0000) >> 16;
                i32 Extended = (Message.lParam & 0x01000000) != 0;
                
                switch (Message.wParam) {
                    case VK_SHIFT:
                    {
                        u32 NewKeyCode = MapVirtualKey(Scancode, MAPVK_VSC_TO_VK_EX);
                        UpdateKeyChange(Input, NewKeyCode, Message.lParam);
                    } break;
                    
                    case VK_CONTROL:
                    {
                        u32 NewKeyCode = Extended ? VK_RCONTROL : VK_LCONTROL;
                        UpdateKeyChange(Input, NewKeyCode, Message.lParam);
                    } break;
                    
                    case VK_MENU:
                    {
                        u32 NewKeyCode = Extended ? VK_RMENU : VK_LMENU;
                        UpdateKeyChange(Input, NewKeyCode, Message.lParam);
                    } break;
                    
                    default:
                    {
                        if(!UpdateKeyChange(Input, (u32)Message.wParam, Message.lParam))
                        {
                            // If we don't process this message, try translating it to WM_CHAR
                            TranslateMessage(&Message);
                            DispatchMessageW(&Message);
                        }
                    } break;    
                }
            } break;
            case WM_MOUSEMOVE:
            {
                Input->MouseP.x = (u16) Message.lParam; 
                Input->MouseP.y = (u16)(Message.lParam >> 16);
                Input->MouseP.y = GlobalGameState.Renderer.Window.CurrentDim.Height - Input->MouseP.y;
                
                if(!PrevMouseMove) // Checks if mouse buttons are still pressed after leaving and re-entering the window
                {
                    if(Input->Pressed[KEY_LMB] && GetAsyncKeyState(VK_LBUTTON) == 0)
                        UpdateSingleKey(Input, KEY_LMB, true, false);
                    if(Input->Pressed[KEY_RMB] && GetAsyncKeyState(VK_RBUTTON) == 0) 
                        UpdateSingleKey(Input, KEY_RMB, true, false);
                    if(Input->Pressed[KEY_MMB] && GetAsyncKeyState(VK_MBUTTON) == 0) 
                        UpdateSingleKey(Input, KEY_MMB, true, false);
                }
                Input->_MouseMoved = true;
            } break;
            case WM_MOUSEWHEEL:
            {
                i32 Modifiers = GET_KEYSTATE_WPARAM(Message.wParam)/4;
                i32 ZDelta    = GET_WHEEL_DELTA_WPARAM(Message.wParam);
                
                i32 WheelPlus = (Modifiers == 2) ? 1000 : 1 + Modifiers*3;
                Input->WheelAmount += ((ZDelta/30)*-WheelPlus)*10;
            } break;
            case WM_LBUTTONDOWN:
            {
                UpdateSingleKey(Input, KEY_LMB, false, true); 
            } break;
            case WM_LBUTTONUP:
            {
                UpdateSingleKey(Input, KEY_LMB, true, false); 
            } break;
            case WM_RBUTTONDOWN:
            {
                UpdateSingleKey(Input, KEY_RMB, false, true); 
            } break;
            case WM_RBUTTONUP:
            {
                UpdateSingleKey(Input, KEY_RMB, false, true); 
            } break;
            case WM_MBUTTONDOWN:
            {
                UpdateSingleKey(Input, KEY_MMB, false, true); 
            } break;
            case WM_MBUTTONUP:
            {
                UpdateSingleKey(Input, KEY_MMB, false, true); 
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}

internal b32
TryGetClipboardText(string_c *String)
{
    b32 Result = false;
    Assert(String);
    
    // Try opening the clipboard
    if (OpenClipboard(0))
    {
        // Get handle of clipboard object for ANSI text
        HANDLE Data = GetClipboardData(CF_UNICODETEXT);
        if (Data)
        {
            // Lock the handle to get the actual text pointer
            wchar_t *Text = (wchar_t *)GlobalLock(Data);
            if (Text)
            {
                Result = true;
                
                if(StringLength(Text) >= String->Length) Text[String->Length-1] = 0;
                string_c TextUTF8;
                ConvertString16To8(&GlobalGameState.ScratchArena, Text, &TextUTF8);
                AppendStringCompoundToCompound(String, &TextUTF8);
                DeleteStringCompound(&GlobalGameState.ScratchArena, &TextUTF8);
                
                // Release the lock
                GlobalUnlock(Data);
                
                // Release the clipboard
                CloseClipboard();
            }
        }
    }
    return Result;
}

inline void
GetWindowDimensions(HWND Window, v2i *Dim)
{
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    Dim->x = Paint.rcPaint.right - Paint.rcPaint.left; 
    Dim->y = Paint.rcPaint.bottom - Paint.rcPaint.top;
    EndPaint(Window, &Paint);
    ReleaseDC(Window, DeviceContext);
}

inline LONGLONG
GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result.QuadPart);
}

inline r32
GetSecondsElapsed(i64 PerfCountFrequency, LONGLONG Start, LONGLONG End)
{
    r32 Result = ((r32)(End - Start) / (r32)PerfCountFrequency);
    return(Result);
}

internal void
RetrieveAndSetDataPaths(game_state *GameState)
{
    wchar_t *WAppdataPath;
    if(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, 0, &WAppdataPath) == S_OK) // Get appdata path
    {
        string_c MPlay3Folder = NewStaticStringCompound("\\MPlay3Data\\");
        string_c AppdataPath = {};
        if(ConvertString16To8(&GameState->ScratchArena, WAppdataPath, &AppdataPath))
        {
            string_c DataPath = NewStringCompound(&GameState->ScratchArena, AppdataPath.Pos+MPlay3Folder.Pos);
            ConcatStringCompounds(3, &DataPath, &AppdataPath, &MPlay3Folder);
            
            // Settings path
            NewLocalString(SettingsPath, MAX_PATH, DataPath.S);
            AppendStringCompoundToCompound(&SettingsPath, &SETTINGS_FILE_NAME);
            read_file_result R = {};
            if(ReadBeginningOfFile(&GameState->ScratchArena, &R, SettingsPath.S, 1)) // If we can read 1 byt from it, it exists
            {
                GameState->SettingsPath = NewStringCompound(&GameState->FixArena, SettingsPath.Pos);
                AppendStringCompoundToCompound(&GameState->SettingsPath, &SettingsPath);
            }
            
            // Library path
            NewLocalString(LibraryPath, MAX_PATH, DataPath.S);
            AppendStringCompoundToCompound(&LibraryPath, &LIBRARY_FILE_NAME);
            b32 FoundLibAtAppData = false;
            if(ReadBeginningOfFile(&GameState->ScratchArena, &R, LibraryPath.S, 1))
            {
                GameState->LibraryPath = NewStringCompound(&GameState->FixArena, LibraryPath.Pos);
                AppendStringCompoundToCompound(&GameState->LibraryPath, &LibraryPath);
                FoundLibAtAppData = true;
            }
            
            // PlaylistPath, this will only be to %APPDATA%/MPlay3Data if the library file is also there.
            if(FoundLibAtAppData)
            {
                NewLocalString(PlaylistPath, MAX_PATH, DataPath.S);
                AppendStringCompoundToCompound(&PlaylistPath, &PLAYLIST_FILE_NAME);
                GameState->PlaylistPath = NewStringCompound(&GameState->FixArena, PlaylistPath.Pos);
                AppendStringCompoundToCompound(&GameState->PlaylistPath, &PlaylistPath);
            }
        }
    }
    CoTaskMemFree(WAppdataPath);
    
    // If still 0 length, we put it in the local folder, aka just using the name.
    if(GameState->SettingsPath.Pos == 0) GameState->SettingsPath = SETTINGS_FILE_NAME;
    if(GameState->LibraryPath.Pos == 0)  GameState->LibraryPath  = LIBRARY_FILE_NAME;
    if(GameState->PlaylistPath.Pos == 0) GameState->PlaylistPath = PLAYLIST_FILE_NAME;
}

inline b32
GetDefaultFontDir(arena_allocator *Arena, string_c *Path)
{
    b32 Result = false;
    
    wchar_t *WFontPath;
    if(SHGetKnownFolderPath(FOLDERID_Fonts, 0, 0, &WFontPath) == S_OK)
    {
        if(ConvertString16To8(Arena, WFontPath, Path))
        {
            Result = true;
        }
    }
    CoTaskMemFree(WFontPath);
    
    return Result;
}

i32 CALLBACK 
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CmdLine,
        i32 ShowCmd)
{
    // Hey, baby! Check out the nil-value _I'm_ dereferencing.
    ArrowCursor = LoadCursor(0, IDC_ARROW);
    DragCursor = LoadCursor(0, IDC_SIZEWE);
    
    WNDCLASSW WindowClass     = {};
    WindowClass.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc   = WindowCallback;
    WindowClass.hInstance     = Instance;
    WindowClass.hCursor       = ArrowCursor;
    WindowClass.hIcon         = LoadIcon(Instance, MAKEINTRESOURCE(102));
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WindowClass.lpszClassName = L"MPlay3ClassName";
    
    if(RegisterClassW(&WindowClass))
    {
        // Initializing GameState
        game_state *GameState = &GlobalGameState;
        *GameState = {};
        GameState->Time.GameHz           = 60; // TODO:: Get monitor refresh rate!?
        GameState->Time.GoalFrameRate    = 1.0f/GameState->Time.GameHz;
        GameState->Time.dTime            = 0.0f;
        GameState->Time.GameTime         = 0.0f;
        GameState->Time.CurrentTimeSpeed = 1.0f;
        GameState->Time.SleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR); 
        GameState->Input                 = {};
        input_info *Input = &GameState->Input;
        
        // Initializing clock
        LARGE_INTEGER PerfCountFrequencyResult;
        QueryPerformanceFrequency(&PerfCountFrequencyResult);
        GameState->Time.PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
        LONGLONG PrevCycleCount = GetWallClock();
        LONGLONG FlipWallClock  = GetWallClock();
        
        InitTimers();
        StartTimer("Initialization");
        
        SetRandomSeed(FlipWallClock);
        
        // Initializing Allocator
        GameState->FixArena.MaxEmptyArenaCount = 2;
        GameState->ScratchArena = {arenaFlags_IsTransient, Megabytes(16)};
        GameState->JobThreadsArena.Flags = arenaFlags_IsThreaded;
        
        // NOTE:: Loading Settings file
        layout_definition *Layout = &GameState->Layout;
        RetrieveAndSetDataPaths(GameState);
        GameState->Settings = TryLoadSettingsFile(GameState);
        
        u32 InitialWindowWidth  = GameState->Settings.WindowDimX;
        u32 InitialWindowHeight = GameState->Settings.WindowDimY;
        HWND Window = CreateWindowExW(0, WindowClass.lpszClassName, L"MPlay3", 
                                      WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, 
                                      CW_USEDEFAULT, InitialWindowWidth, -1, 
                                      0, 0, Instance, 0);
        if(Window)
        {
            SetWindowPos(Window, 0, 0, 0, InitialWindowWidth, InitialWindowHeight, SWP_NOMOVE); // NOTE:: AMD Win7 driver bug fix
            HDC DeviceContext = GetDC(Window);
            InitOpenGL(Window);
            
            AddHotKey(Window, Input, KEY_PLAY_PAUSE);
            AddHotKey(Window, Input, KEY_STOP);
            AddHotKey(Window, Input, KEY_NEXT);
            AddHotKey(Window, Input, KEY_PREVIOUS);
            //AddHotKey(Window, Input, KEY_MUTE);
            //AddHotKey(Window, Input, KEY_VOLUME_UP);
            //AddHotKey(Window, Input, KEY_VOLUME_DOWN);
            
            
            // ********************************************
            // Threading***********************************
            // ********************************************
            
            circular_job_queue *JobQueue = &GameState->JobQueue;
            *JobQueue = {};
            job_thread_info *JobInfos = GameState->JobInfos;
            InitializeJobThreads(GameState->JobHandles, JobQueue, JobInfos);
            
            sound_thread_data SoundThreadData = {};
            sound_thread SoundThreadInfo = {};
            GameState->SoundThreadInterface = &SoundThreadInfo.Interface;
            SoundThreadInfo.Interface.SoundMutex = CreateMutexEx(0, 0, 0, MUTEX_ALL_ACCESS);
            SoundThreadInfo.Interface.ToneVolume = SoundThreadInfo.SoundInfo.ToneVolume = 0.5f;
            
            NewSoundInstance(&SoundThreadInfo.SoundInfo.SoundInstance, Window, 48000, 2, GameState->Time.GameHz);
            SoundThreadInfo.SoundSamples = AllocateArray(&GameState->FixArena, 
                                                         SoundThreadInfo.SoundInfo.SoundInstance.BufferSize, i16);
            GameState->ThreadErrorList.Mutex = CreateMutexEx(0, 0, 0, MUTEX_ALL_ACCESS);
            
            InitializeSoundThread(&SoundThreadData, ProcessSound, &SoundThreadInfo);
            
            // Preparing base structs
            PrepareGenreTypesList();
            renderer *Renderer = &GameState->Renderer;
            *Renderer = InitializeRenderer(GameState, Window);
            
            ReshapeGLWindow(&GameState->Renderer);
            b32 *ReRender = &Renderer->Rerender;
            
            music_info *MusicInfo     = &GameState->MusicInfo;
            MusicInfo->Playlists      = CreatePlaylistList(&GameState->FixArena, 20);
            MusicInfo->MuteVolume     = -1;
            playing_song *PlayingSong = &MusicInfo->PlayingSong;
            *PlayingSong = {-1, -1, -1};
            mp3_info *MP3Info = 0;
            
            
            // User error text *************************************
            user_error_text *UserErrorText = &GameState->UserErrorText;
            UserErrorText->AnimTime        = Layout->ErrorTextAnimationTime;
            UserErrorText->dAnim           = 1.0f;
            
            // Preparing arguments for loading files
            b32 LoadedLibraryFile = false;
            u32 FileInfoCount = 0;
            if(ConfirmLibraryWithCorrectVersionExists(GameState, LIBRARY_CURRENT_VERSION, &FileInfoCount) && 
               ConfirmLibraryMusicPathExists(GameState))
            {
                MP3Info = CreateMP3InfoStruct(&GameState->FixArena, FileInfoCount);
                StartTimer("Load library file");
                LoadMP3LibraryFile(GameState, MP3Info);
                SnapTimer("Load library file");
                LoadedLibraryFile = true;
            }
            else
            {
                MP3Info = CreateMP3InfoStruct(&GameState->FixArena, 0);
                CreateOrWipeStringComp(&GameState->FixArena, &MP3Info->FolderPath, 256);
            }
            GameState->MP3Info = MP3Info;
            MP3Info->MusicInfo = &GameState->MusicInfo;
            
            MusicInfo->UpNextList.A = CreateArray(&GameState->FixArena, 200);
            
            
            CreateMusicSortingInfo(MusicInfo->Playlists.List+0, true);
            FillDisplayables(MusicInfo, &MP3Info->FileInfo, &MusicInfo->DisplayInfo);
            if(LoadedLibraryFile) SortDisplayables(MusicInfo, &MP3Info->FileInfo);
            UpdatePlayingSong(MusicInfo);
            
            LoadAllPlaylists(GameState);
            
            // ********************************************
            // FONT stuff *********************************
            // ********************************************
            Renderer->FontAtlas = InitializeFont(GameState);
            
            /*
GetFontGroup(GameState, &Renderer->FontAtlas, 0x1f608);
            GetFontGroup(GameState, &Renderer->FontAtlas, 0x2F00);
            GetFontGroup(GameState, &Renderer->FontAtlas, 0x211c);
            GetFontGroup(GameState, &Renderer->FontAtlas, 0x685c);
            GetFontGroup(GameState, &Renderer->FontAtlas, 0x5ead);
            GetFontGroup(GameState, &Renderer->FontAtlas, 0x7d71);
            
            entry_id *FontMap1 = CreateRenderBitmap(Renderer, {{0,0},{1500,1500}}, -0.99f, 0, Renderer->FontAtlas.FontGroups[1].GLID);
            entry_id *FontMap2 = CreateRenderBitmap(Renderer, {{192,0},{192*2,202}}, -0.99f, 0, Renderer->FontAtlas.FontGroups[3].GLID);
            entry_id *FontMap3 = CreateRenderBitmap(Renderer, {{192*2,0},{192*3,202}}, -0.99f, 0, Renderer->FontAtlas.FontGroups[4].GLID);
            */
            
            // ********************************************
            // UI rendering stuff   ***********************
            // ********************************************
            r32 WWidth = (r32)Renderer->Window.FixedDim.Width;
            r32 WMid    = WWidth*0.5f;
            r32 WHeight = (r32)Renderer->Window.FixedDim.Height;
            
            MusicInfo->DisplayInfo.MusicBtnInfo = {GameState, PlayingSong};
            InitializeDisplayInfo(&MusicInfo->DisplayInfo, GameState, MP3Info, Layout);
            music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
            DisplayInfo->DragDrop = {};
            
            
            // ********************************************
            // Dragging************************************
            // ********************************************
            drag_list *DragableList = &GameState->DragableList;
            DragableList->DraggingID = -1;
            
            // Column edges
            r32 BLeft   = GetExtends(DisplayInfo->EdgeLeft).x  + Layout->DragEdgeWidth/2 + Layout->VerticalSliderWidth;
            r32 BRight  = GetExtends(DisplayInfo->EdgeRight).x + Layout->DragEdgeWidth/2 + Layout->VerticalSliderWidth;
            r32 BMiddle = Layout->DragEdgeWidth + Layout->VerticalSliderWidth;
            
            column_edge_drag EdgePlaylistsGenreDrag = {};
            EdgePlaylistsGenreDrag.LeftEdgeChain  = {{DisplayInfo->EdgeLeft}, {BLeft}, {}, 1};
            EdgePlaylistsGenreDrag.RightEdgeChain = {
                {DisplayInfo->GenreArtist.Edge, DisplayInfo->ArtistAlbum.Edge, DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight}, 
                {BMiddle, BMiddle, BMiddle, BRight}, 
                {&DisplayInfo->GenreArtist.XPercent, &DisplayInfo->ArtistAlbum.XPercent, &DisplayInfo->AlbumSong.XPercent}, 4 
            };
            EdgePlaylistsGenreDrag.XPercent     = &DisplayInfo->PlaylistsGenre.XPercent;
            EdgePlaylistsGenreDrag.MusicInfo    = MusicInfo;
            AddDragable(DragableList, DisplayInfo->PlaylistsGenre.Edge, {}, 
                        {OnDisplayColumnEdgeDrag, &EdgePlaylistsGenreDrag}, {});
            
            column_edge_drag EdgeGenreArtistDrag = {};
            EdgeGenreArtistDrag.LeftEdgeChain  = {
                {DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->EdgeLeft}, 
                {BLeft, BMiddle}, {&DisplayInfo->PlaylistsGenre.XPercent}, 2
            };
            EdgeGenreArtistDrag.RightEdgeChain = {
                {DisplayInfo->ArtistAlbum.Edge, DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight}, 
                {BMiddle, BMiddle, BRight}, {&DisplayInfo->ArtistAlbum.XPercent, &DisplayInfo->AlbumSong.XPercent}, 3
            };
            EdgeGenreArtistDrag.XPercent     = &DisplayInfo->GenreArtist.XPercent;
            EdgeGenreArtistDrag.MusicInfo    = MusicInfo;
            AddDragable(DragableList, DisplayInfo->GenreArtist.Edge, {}, {OnDisplayColumnEdgeDrag, &EdgeGenreArtistDrag}, {});
            
            column_edge_drag EdgeArtistAlbumDrag = {};
            EdgeArtistAlbumDrag.LeftEdgeChain  = {
                {DisplayInfo->GenreArtist.Edge, DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->EdgeLeft}, 
                {BMiddle, BMiddle, BLeft}, 
                {&DisplayInfo->GenreArtist.XPercent, &DisplayInfo->PlaylistsGenre.XPercent}, 3
            };
            EdgeArtistAlbumDrag.RightEdgeChain = {
                {DisplayInfo->AlbumSong.Edge, DisplayInfo->EdgeRight}, {BMiddle, BRight},
                {&DisplayInfo->AlbumSong.XPercent}, 2};
            EdgeArtistAlbumDrag.XPercent     = &DisplayInfo->ArtistAlbum.XPercent;
            EdgeArtistAlbumDrag.MusicInfo    = MusicInfo;
            AddDragable(DragableList, DisplayInfo->ArtistAlbum.Edge, {}, {OnDisplayColumnEdgeDrag, &EdgeArtistAlbumDrag}, {});
            
            column_edge_drag EdgeAlbumSongDrag = {};
            EdgeAlbumSongDrag.LeftEdgeChain  = {
                {DisplayInfo->ArtistAlbum.Edge, DisplayInfo->GenreArtist.Edge, DisplayInfo->PlaylistsGenre.Edge, DisplayInfo->EdgeLeft}, {BMiddle, BMiddle, BMiddle, BLeft}, {&DisplayInfo->ArtistAlbum.XPercent, &DisplayInfo->GenreArtist.XPercent, &DisplayInfo->PlaylistsGenre.XPercent}, 4
            };
            EdgeAlbumSongDrag.RightEdgeChain = {{DisplayInfo->EdgeRight}, {BRight}, {}, 1};
            EdgeAlbumSongDrag.XPercent       = &DisplayInfo->AlbumSong.XPercent;
            EdgeAlbumSongDrag.MusicInfo      = MusicInfo;
            AddDragable(DragableList, DisplayInfo->AlbumSong.Edge, {}, {OnDisplayColumnEdgeDrag, &EdgeAlbumSongDrag}, {});
            
            // Sliders
            drag_slider_data GenreDrag  = {MusicInfo, &DisplayInfo->Genre};
            drag_slider_data ArtistDrag = {MusicInfo, &DisplayInfo->Artist};
            drag_slider_data AlbumDrag  = {MusicInfo, &DisplayInfo->Album};
            drag_slider_data SongDrag   = {MusicInfo, &DisplayInfo->Song.Base};
            drag_slider_data PlaylistsDrag = {MusicInfo, &DisplayInfo->Playlists};
            
            AddDragable(DragableList, DisplayInfo->Genre.SliderHorizontal.Background,  
                        {OnSliderDragStart, &DisplayInfo->Genre.SliderHorizontal}, {OnHorizontalSliderDrag, &GenreDrag}, {});
            AddDragable(DragableList, DisplayInfo->Artist.SliderHorizontal.Background, 
                        {OnSliderDragStart, &DisplayInfo->Artist.SliderHorizontal}, {OnHorizontalSliderDrag, &ArtistDrag}, {});
            AddDragable(DragableList, DisplayInfo->Album.SliderHorizontal.Background,  
                        {OnSliderDragStart, &DisplayInfo->Album.SliderHorizontal}, {OnHorizontalSliderDrag, &AlbumDrag}, {});
            AddDragable(DragableList, DisplayInfo->Song.Base.SliderHorizontal.Background,
                        {OnSliderDragStart, &DisplayInfo->Song.Base.SliderHorizontal}, {OnHorizontalSliderDrag, &SongDrag},{});
            AddDragable(DragableList, DisplayInfo->Playlists.SliderHorizontal.Background,
                        {OnSliderDragStart, &DisplayInfo->Playlists.SliderHorizontal}, {OnHorizontalSliderDrag, &PlaylistsDrag},{});
            
            AddDragable(DragableList, DisplayInfo->Genre.SliderVertical.Background,
                        {OnSliderDragStart, &DisplayInfo->Genre.SliderVertical}, {OnVerticalSliderDrag, &GenreDrag}, {});
            AddDragable(DragableList, DisplayInfo->Artist.SliderVertical.Background,
                        {OnSliderDragStart, &DisplayInfo->Artist.SliderVertical}, {OnVerticalSliderDrag, &ArtistDrag}, {});
            AddDragable(DragableList, DisplayInfo->Album.SliderVertical.Background,
                        {OnSliderDragStart, &DisplayInfo->Album.SliderVertical}, {OnVerticalSliderDrag, &AlbumDrag}, {});
            AddDragable(DragableList, DisplayInfo->Song.Base.SliderVertical.Background,
                        {OnSliderDragStart, &DisplayInfo->Song.Base.SliderVertical}, {OnVerticalSliderDrag, &SongDrag},
                        {OnSongDragEnd, &SongDrag});
            AddDragable(DragableList, DisplayInfo->Playlists.SliderVertical.Background,
                        {OnSliderDragStart, &DisplayInfo->Playlists.SliderVertical}, {OnVerticalSliderDrag, &PlaylistsDrag}, {});
            
            // Timeline stuff
            AddDragable(DragableList, DisplayInfo->Volume.Background,{},{OnVolumeDrag, GameState},{});
            timeline_slider_drag TimelineDragInfo = {GameState, false};
            // Adding this twice, once for grabbing the small nubs and for the thin bar.
            AddDragable(DragableList, DisplayInfo->PlayingSongPanel.Timeline.GrabThing,
                        {OnTimelineDragStart, &TimelineDragInfo},
                        {OnTimelineDrag, &TimelineDragInfo},
                        {OnTimelineDragEnd, &TimelineDragInfo});
            AddDragable(DragableList, DisplayInfo->PlayingSongPanel.Timeline.Background,
                        {OnTimelineDragStart, &TimelineDragInfo},
                        {OnTimelineDrag, &TimelineDragInfo},
                        {OnTimelineDragEnd, &TimelineDragInfo});
            
            if(LoadedLibraryFile) AddJob_CheckMusicPathChanged(&GameState->CheckMusicPath);
            else                  OnMusicPathPressed(&DisplayInfo->MusicBtnInfo);
            ApplySettings(GameState, GameState->Settings);
            
            // Style settings and Color Picker *******************************
            CreateStyleSettings(GameState, &GameState->StyleSettings);
            SetActive(&GameState->StyleSettings, true);
            
            CreateShortcutPopups(DisplayInfo);
            // ********************************************
            // Threading **********************************
            // ********************************************
            AddJobs_LoadOnScreenMP3s(GameState, JobQueue);
            
            
            crawl_thread_out CrawlInfoOut = {false, false, false, 0};
            GameState->CrawlInfo = {MP3Info, NewStringCompound(&GameState->FixArena, 255), &CrawlInfoOut};
            
            // ********************************************
            // Test Area **********************************
            // ********************************************
            
            //NewLocalString(TestError, 100, "Oh no, I have an error... \nOk now I have to type until I reach 105!");
            //NewLocalString(TestError, 100, "Oh no... \nOk now!");
            //PushErrorMessage(GameState, TestError);
            //PushErrorMessage(GameState, {errorCode_EmptyFile, 27});
            /*
            v3 ColorT = {0, 1, 1};
            r32 FontHeightSmall = GetFontSize(&GameState->Renderer, font_Small).Size;
            entry_id *TestBase = CreateRenderRect(Renderer, V2(5,FontHeightSmall), -0.9f, &ColorT, NULL);
            SetPosition(TestBase, V2(500, 500));
            
            r32 FontHeightMedium = GetFontSize(&GameState->Renderer, font_Medium).Size;
            TestBase = CreateRenderRect(Renderer, V2(5,FontHeightMedium), -0.9f, &ColorT, NULL);
            SetPosition(TestBase, V2(510, 500));
            */
            
            // ********************************************
            // FPS ****************************************
            // ********************************************
            
#if DEBUG_TD 
            r32 FPSList[100] = {};
            u32 FPSCount = 0;
            v3 NOP = {};
            entry_id *FPSParent = CreateRenderRect(Renderer, {{},{}}, -0.9f, 0, &NOP);
            SetPosition(FPSParent, V2(Renderer->Window.CurrentDim.Dim) - V2(60, 11));
            TranslateWithScreen(&Renderer->TransformList, FPSParent, fixedTo_TopRight);
            render_text FPSText = {};
            r32 dUpdateRate = 0.0f;
#endif
            b32 SongSlotHeightToggle = false;
            b32 SongChangedIsCurrentlyDecoding      = true;
            b32 SongChangedIsCurrentlyFullyDecoding = true;
            b32 PrevIsPlaying = MusicInfo->IsPlaying;
            scroll_load_info ScrollLoadInfo = {0, 0.5f, true};
            IsRunning = true;
            SnapTimer("Initialization");
            while(IsRunning)
            {
                ResetMemoryArena(&GameState->ScratchArena);
                
                LONGLONG CurrentCycleCount = GetWallClock();
                GameState->Time.dTime = GetSecondsElapsed(GameState->Time.PerfCountFrequency, 
                                                          PrevCycleCount, CurrentCycleCount);
                PrevCycleCount            = CurrentCycleCount;
                GameState->Time.GameTime += GameState->Time.dTime;
                
#if DEBUG_TD
                if(!Renderer->Minimized) {
                    FPSList[FPSCount] = (1.0f/GameState->Time.dTime);
                    r32 CurrentFPS = 0;
                    For(100) CurrentFPS += FPSList[It];
                    CurrentFPS /= 100.0f;
                    
                    if(dUpdateRate >= 1.0f)
                    {
                        dUpdateRate = 0.0f;
                        char FPSString[10];
                        sprintf_s(FPSString, "%.2f", CurrentFPS);
                        string_c FPSComp = NewStringCompound(&GameState->ScratchArena, 10);
                        AppendStringToCompound(&FPSComp, (u8 *)FPSString);
                        RemoveRenderText(Renderer, &FPSText);
                        RenderText(GameState, font_Small, &FPSComp,
                                   &DisplayInfo->ColorPalette.ForegroundText, &FPSText, -0.9999f, FPSParent, 
                                   V2(0, -GetFontAscent(GameState, font_Small, FPSComp)));
                        DeleteStringCompound(&GameState->ScratchArena, &FPSComp);
                    }
                    else dUpdateRate += GameState->Time.dTime/0.1f;
                    FPSCount = (FPSCount+1)%100;
                }
#endif
                AnimateErrorMessage(Renderer, &GameState->UserErrorText, GameState->Time.dTime);
                ProcessThreadErrors(GameState);
                
                if(Input->KeyChange[KEY_F9] == KeyDown) 
                {
                    SongSlotHeightToggle = !SongSlotHeightToggle;
                    SetSongSlotHeightMode(GameState, SongSlotHeightToggle);
                }
                if(Input->KeyChange[KEY_VOLUME_UP] == KeyDown) DebugLog(10, "UP!\n");
                if(Input->KeyChange[KEY_VOLUME_DOWN] == KeyDown) DebugLog(10, "DOWN!\n");
                
                // *******************************************
                // Mode Handling *****************************
                // *******************************************
                GameState->ModeFlags = 0;
                if(DisplayInfo->MusicPath.TextField.IsActive)    GameState->ModeFlags |= mode_MusicPath;
                if(GameState->StyleSettings.IsActive)            GameState->ModeFlags |= mode_StyleSettings;
                if(IsSearchOpen(DisplayInfo))                    GameState->ModeFlags |= mode_Search;
                if(DisplayInfo->PlaylistUI.RenameField.IsActive) GameState->ModeFlags |= mode_PLRename;
                
                if(IsActive(GameState, mode_MusicPath))
                {
                    if(IsActive(GameState, mode_StyleSettings))
                    {
                        OnCancelStyleSettings(&GameState->StyleSettings);
                        GameState->ModeFlags &= ~mode_StyleSettings;
                    }
                }
                if(IsActive(GameState, mode_MusicPath) ||
                   IsActive(GameState, mode_StyleSettings))
                {
                    if(IsActive(GameState, mode_Search))
                    {
                        InterruptSearch(Renderer, MusicInfo);
                        GameState->ModeFlags &= ~mode_Search;
                    }
                }
                if(IsActive(GameState, mode_MusicPath)   ||
                   IsActive(GameState, mode_StyleSettings) ||
                   IsActive(GameState, mode_Search)) 
                {
                    if(IsActive(GameState, mode_PLRename))
                    {
                        SetPlaylistRenameActive(GameState, false);
                        GameState->ModeFlags &= ~mode_PLRename;
                    }
                }
                
                // *******************************************
                // Thread handling ****************************
                // *******************************************
                EmptyJobQueueWhenPossible(JobQueue);
                
                if(GameState->CheckMusicPath.State == threadState_Finished) 
                    CheckForMusicPathMP3sChanged_End(&GameState->CheckMusicPath, &DisplayInfo->MusicPath);
                
                // *******************************************
                // Input handling ****************************
                // *******************************************
                ProcessPendingMessages(Input, Window);
                
                
                if(Input->Pressed[KEY_CONTROL_LEFT] || Input->Pressed[KEY_CONTROL_RIGHT])
                {
                    if(Input->Pressed[KEY_UP])
                    {
                        Renderer->FontSizes.Sizes[font_Small].Size  += 1;
                        Renderer->FontSizes.Sizes[font_Medium].Size += 1;
                        Renderer->FontSizes.Sizes[font_Big].Size    += 1;
                        ChangeFontSizes(GameState, Renderer->FontSizes);
                    }
                    if(Input->Pressed[KEY_DOWN])
                    {
                        if(Renderer->FontSizes.Sizes[font_Small].Size > 1)
                        {
                            Renderer->FontSizes.Sizes[font_Small].Size  -= 1;
                            Renderer->FontSizes.Sizes[font_Medium].Size -= 1;
                            Renderer->FontSizes.Sizes[font_Big].Size    -= 1;
                            ChangeFontSizes(GameState, Renderer->FontSizes);
                        }
                    }
                }
                
                
                if(Input->KeyChange[KEY_ESCAPE] == KeyDown)
                {
                    if(IsActive(GameState, mode_MusicPath) && !CrawlInfoOut.ThreadIsRunning) 
                    {
                        // TODO:: Maybe rather kill crawl thread if escape is pressed?
                        // TODO:: Do not save when I directly exit from here
                        OnMusicPathPressed(&DisplayInfo->MusicBtnInfo);
                    }
                    else if(IsActive(GameState, mode_StyleSettings))
                    {
                        SetActive(&GameState->StyleSettings, false);
                    }
                    else if(IsActive(GameState, mode_Search))
                    {
                        InterruptSearch(Renderer, MusicInfo);
                    }
                    else if(IsActive(GameState, mode_PLRename))
                    {
                        OnRenamePlaylistClick(GameState);
                    }
                    else 
                    {
                        SetActive(&DisplayInfo->Quit, true);
                        if((GameState->Time.GameTime - DisplayInfo->Quit.LastEscapePressTime) <= DOUBLE_CLICK_TIME)
                        {
                            DisplayInfo->Quit.WindowExit = true;
                            DisplayInfo->Quit.Time *= 1.0f/Layout->QuitCurtainAnimationMultiplies;
                        }
                        DisplayInfo->Quit.LastEscapePressTime = GameState->Time.GameTime;
                    }
                }
                if(Input->Pressed[KEY_ALT_LEFT] && Input->KeyChange[KEY_F4] == KeyDown) 
                {
                    SetActive(&DisplayInfo->Quit, true);
                    DisplayInfo->Quit.WindowExit = true;
                    DisplayInfo->Quit.Time *= 1.0f/Layout->QuitCurtainAnimationMultiplies;;
                }
                
                if(DisplayInfo->Quit.AnimationStart)
                {
                    v2 Size = V2(Renderer->Window.CurrentDim.Dim);
                    if(Input->Pressed[KEY_ESCAPE] || DisplayInfo->Quit.WindowExit)
                    {
                        if(DisplayInfo->Quit.dAnim < 0) DisplayInfo->Quit.dAnim = 0;
                        if(QuitAnimation(&DisplayInfo->Quit, 1, V2(Size.x/2.0f, Size.y), Size))
                        {
                            IsRunning = false;
                            continue;
                        }
                        DisplayInfo->Quit.LastEscapePressTime = GameState->Time.GameTime;
                    }
                    else if((GameState->Time.GameTime - DisplayInfo->Quit.LastEscapePressTime) > DOUBLE_CLICK_TIME)
                    {
                        if(QuitAnimation(&DisplayInfo->Quit, -1, V2(Size.x/2.0f, Size.y), Size)) 
                            SetActive(&DisplayInfo->Quit, false);
                    }
                }
                
                if(Input->KeyChange[KEY_F5] == KeyDown) 
                {
                    if(DisplayInfo->Popups.IsActive) OnShortcutHelpOff(&DisplayInfo->Popups);
                    else OnShortcutHelpOn(&DisplayInfo->Popups);
                    ToggleButtonVisuals(DisplayInfo->Help, DisplayInfo->Popups.IsActive);
                }
                if(Input->KeyChange[KEY_F6] == KeyDown) UpdateColorPalette(DisplayInfo, true);
                if(Input->KeyChange[KEY_F7] == KeyDown) 
                {
                    SetActive(&GameState->StyleSettings, !GameState->StyleSettings.IsActive);
                }
                if(Input->KeyChange[KEY_F8] == KeyDown) 
                {
                    if(DisplayInfo->MusicPath.TextField.IsActive) 
                    {
                        if(!CrawlInfoOut.ThreadIsRunning) OnMusicPathQuitPressed(&DisplayInfo->MusicBtnInfo);
                    }
                    else OnMusicPathPressed(&DisplayInfo->MusicBtnInfo);
                }
                
                if(IsActive(GameState, mode_MusicPath))
                {
                    HandleActiveMusicPath(DisplayInfo, Input, &CrawlInfoOut);
                } // This is only reachable if MusicPath stuff is not on screen
                else 
                {
                    Assert(!CrawlInfoOut.ThreadIsRunning);
                    
                    // Dragging stuff ******
                    b32 LeftMouseUpAndNoDragging = false;
                    if(Input->KeyChange[KEY_LMB] == KeyDown)
                    {
                        DisplayInfo->MouseBtnDownLocation = Input->MouseP;
                        
                        if(!OnDraggingStart(DragableList, Renderer, Input->MouseP))
                        {
                            if(!IsActive(GameState, mode_StyleSettings)) 
                                CheckSlotDragDrop(Input, GameState, &DisplayInfo->DragDrop);
                        }
                    }
                    if(Input->Pressed[KEY_LMB])
                    {
                        if(DragableList->DraggingID >= 0) OnDragging(DragableList, Renderer, Input->MouseP);
                        else if(DisplayInfo->DragDrop.Dragging) 
                        {
                            DoDragDropAnim(GameState, &DisplayInfo->DragDrop);
                            EvalDragDropPosition(GameState, &DisplayInfo->DragDrop);
                        }
                    }
                    if(Input->KeyChange[KEY_LMB] == KeyUp)
                    {
                        if(DragableList->DraggingID >= 0) OnDraggingEnd(DragableList, Renderer, Input->MouseP);
                        else 
                        {
                            if(DisplayInfo->DragDrop.Dragging) StopDragDrop(GameState, &DisplayInfo->DragDrop);
                            LeftMouseUpAndNoDragging = true;
                        }
                    }
                    
                    if(!IsActive(GameState, mode_StyleSettings) || !IsInRect(GameState->StyleSettings.Background, Input->MouseP))
                    {
                        UpdateButtons(Renderer, Input);
                        
                        if(LeftMouseUpAndNoDragging)
                        {
                            column_type ChangedSelection = CheckColumnsForSelectionChange(DisplayInfo->MouseBtnDownLocation);
                            if(ChangedSelection != columnType_None && ChangedSelection != columnType_Song)
                            {
                                UpdateSortingInfoChanged(Renderer, MusicInfo, MP3Info, ChangedSelection);
                                AddJobs_LoadOnScreenMP3s(GameState, JobQueue);
                                
                                if(ChangedSelection == columnType_Playlists && IsActive(GameState, mode_PLRename))
                                    SetPlaylistRenameActive(GameState, false);
                            }
                        }
                        
                    }
                    
                    ProcessTextField(Renderer, GameState->Time.dTime, Input, &DisplayInfo->PlaylistUI.RenameField);
                    if(Input->KeyChange[KEY_ENTER] == KeyDown)
                    {
                        if(DisplayInfo->PlaylistUI.RenameField.IsActive)
                        {
                            SaveNewPlaylistName(GameState);
                            OnRenamePlaylistClick(GameState);
                        }
                    }
                    //if(Input->KeyChange[KEY_F9]  == KeyDown)  SaveMP3LibraryFile(GameState, MP3Info);
                    //if(Input->KeyChange[KEY_F10] == KeyDown) AddJob_CheckMusicPathChanged(CheckMusicPath);
                    // To use F12 in VS look at: https://conemu.github.io/en/GlobalHotKeys.html
                    
                    if(IsActive(GameState, mode_StyleSettings))
                    {
                        HandleActiveStyleSettings(GameState, &GameState->StyleSettings, Input);
                    }
                    else
                    {
                        // Command keys ******
                        if(NoModifiers(Input))
                        {
                            if(Input->KeyChange[KEY_F1] == KeyDown) OnSearchPressed(&DisplayInfo->Genre.SearchInfo);
                            if(Input->KeyChange[KEY_F2] == KeyDown) OnSearchPressed(&DisplayInfo->Artist.SearchInfo);
                            if(Input->KeyChange[KEY_F3] == KeyDown) OnSearchPressed(&DisplayInfo->Album.SearchInfo);
                            if(Input->KeyChange[KEY_F4] == KeyDown) OnSearchPressed(&DisplayInfo->Song.Base.SearchInfo);
                        }
                        // ******************
                        
                        TestHoveringEdgeDrags(GameState, Input->MouseP, DisplayInfo);
                        
                        HandlePlaylistButtonAnimation(GameState, DisplayInfo->PlaylistUI.Add, &DisplayInfo->PlaylistUI.AddCurtain, playlistBtnType_Add);
                        HandlePlaylistButtonAnimation(GameState, DisplayInfo->PlaylistUI.AddSelection, &DisplayInfo->PlaylistUI.AddSelectionCurtain, playlistBtnType_AddSelection);
                        HandlePlaylistButtonAnimation(GameState, DisplayInfo->PlaylistUI.Remove, &DisplayInfo->PlaylistUI.RemoveCurtain, playlistBtnType_Remove);
                        
                        // Next/Previous song *******
                        if(Input->KeyChange[KEY_RIGHT] == KeyDown || Input->KeyChange[KEY_NEXT]  == KeyDown)
                        {
                            OnNextSong(&DisplayInfo->MusicBtnInfo);
                        }
                        else if(Input->KeyChange[KEY_LEFT]     == KeyDown || Input->KeyChange[KEY_PREVIOUS] == KeyDown )
                        {
                            OnPreviousSong(&DisplayInfo->MusicBtnInfo);
                        }
                        
                        // Mute music *******
                        if(Input->KeyChange[KEY_MUTE] == KeyDown) 
                        {
                            DebugLog(10, "MUTE!\n");
                            if(MusicInfo->MuteVolume < 0)
                            {
                                MusicInfo->MuteVolume = GameState->SoundThreadInterface->ToneVolume;
                                ChangeVolume(GameState, 0);
                            }
                            else
                            {
                                ChangeVolume(GameState, MusicInfo->MuteVolume);
                                MusicInfo->MuteVolume = -1;
                            }
                            
                        }
                        
                        // Stop/Start music ***********
                        if(Input->KeyChange[KEY_SPACE] == KeyDown ||
                           Input->KeyChange[KEY_PLAY_PAUSE] == KeyDown)
                        {
                            if(GameState->ModeFlags > 0 && Input->KeyChange[KEY_SPACE] == KeyDown) ;
                            else
                            {
                                if(MusicInfo->IsPlaying) 
                                    OnPlayPauseSongToggleOff(&DisplayInfo->MusicBtnInfo);
                                else
                                    OnPlayPauseSongToggleOn(&DisplayInfo->MusicBtnInfo);
                            }
                        }
                        if(Input->KeyChange[KEY_STOP] == KeyDown)
                        {
                            OnStopSong(&DisplayInfo->MusicBtnInfo);
                        }
                        
                        // Change volume ******
                        if(!Input->Pressed[KEY_CONTROL_LEFT] && !Input->Pressed[KEY_CONTROL_RIGHT])
                        {
                            if(Input->Pressed[KEY_UP] || Input->Pressed[KEY_ADD] ||
                               (Input->Pressed[KEY_PLUS] && DisplayInfo->SearchIsActive < 0)) // Plus only when no search is open
                            {
                                ChangeVolume(GameState, GameState->SoundThreadInterface->ToneVolume+0.01f);
                            }
                            else if(Input->Pressed[KEY_DOWN] || Input->Pressed[KEY_SUBTRACT] || 
                                    (Input->Pressed[KEY_MINUS] && DisplayInfo->SearchIsActive < 0))
                            {
                                ChangeVolume(GameState, GameState->SoundThreadInterface->ToneVolume-0.01f);
                            }
                        }
                        
                        // Scrolling ********
                        if(Input->WheelAmount != 0)
                        {
                            playlist_info *Playlist = MusicInfo->Playlist_;
                            if(IsInRect(DisplayInfo->Song.Base.Background, Input->MouseP))
                            {
                                ScrollDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Song.Base, (r32)Input->WheelAmount);
                                UpdateColumnVerticalSliderPosition(&DisplayInfo->Song.Base,
                                                                   Playlist->Song.Displayable.A.Count);
                                
                                ScrollLoadInfo.LoadFinished = false;
                                ScrollLoadInfo.dTime = 0;
                            }
                            if(IsInRect(DisplayInfo->Genre.Background, Input->MouseP))
                            {
                                ScrollDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Genre, (r32)Input->WheelAmount);
                                UpdateColumnVerticalSliderPosition(&DisplayInfo->Genre, Playlist->Genre.Displayable.A.Count);
                            }
                            if(IsInRect(DisplayInfo->Artist.Background, Input->MouseP))
                            {
                                ScrollDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Artist, (r32)Input->WheelAmount);
                                UpdateColumnVerticalSliderPosition(&DisplayInfo->Artist, Playlist->Artist.Displayable.A.Count);
                            }
                            if(IsInRect(DisplayInfo->Album.Background, Input->MouseP))
                            {
                                ScrollDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Album, (r32)Input->WheelAmount);
                                UpdateColumnVerticalSliderPosition(&DisplayInfo->Album, Playlist->Album.Displayable.A.Count);
                            }
                            if(IsInRect(DisplayInfo->Playlists.Background, Input->MouseP))
                            {
                                ScrollDisplayColumn(Renderer, MusicInfo, &DisplayInfo->Playlists, (r32)Input->WheelAmount);
                                UpdateColumnVerticalSliderPosition(&DisplayInfo->Playlists, Playlist->Playlists.Displayable.A.Count);
                            }
                        }
                        
                        if(!ScrollLoadInfo.LoadFinished) // If user finished scrolling, after WaitTime we load onscreen songs
                        {
                            if(ScrollLoadInfo.dTime >= 1.0f)
                            {
                                ScrollLoadInfo.dTime = 0.0f;
                                ScrollLoadInfo.LoadFinished = true;
                                OnSongDragEnd(0, {}, 0, 0);
                            }
                            else ScrollLoadInfo.dTime += GameState->Time.dTime/ScrollLoadInfo.WaitTime;
                        }
                        
                        ProcessAllSearchBars(GameState);
                        
                        if(PrevIsPlaying != MusicInfo->IsPlaying)
                        {
                            ToggleButtonVisuals(DisplayInfo->PlayPause, MusicInfo->IsPlaying);
                        }
                    }
                    
                    b32 TimelineFreeze = GetIsPlayingPreload(GameState->SoundThreadInterface);
                    if(DisplayInfo->PlayingSongPanel.TimelineFreezeChange != TimelineFreeze)
                    {
                        DisplayInfo->PlayingSongPanel.TimelineFreezeChange = TimelineFreeze;
                        if(TimelineFreeze) SetTransparency(&DisplayInfo->PlayingSongPanel.Timeline, 0.5f);
                        else               SetTransparency(&DisplayInfo->PlayingSongPanel.Timeline, 1.0f);
                    }
                }
                
                // *******************************************
                // Threading *********************************
                // *******************************************
                if(PlayingSong->DecodeID >= 0)
                {
                    if(!MP3Info->DecodeInfo.CurrentlyDecoding[PlayingSong->DecodeID])
                    {
                        if(MP3Info->DecodeInfo.CurrentlyDecoding[PlayingSong->DecodeID] != SongChangedIsCurrentlyDecoding)
                        {
                            SongChangedIsCurrentlyDecoding = false;
                            FinishChangeSong(GameState, PlayingSong);
                            SetNewPlayingSong(Renderer, &DisplayInfo->PlayingSongPanel, Layout, MusicInfo);
                        }
                    }
                    else 
                    {
                        SongChangedIsCurrentlyDecoding = true;
                    }
                    
                    
                    // For loading the entire song
                    if(!MP3Info->DecodeInfo.PlayingDecoded.CurrentlyDecoding)
                    {
                        if(MP3Info->DecodeInfo.PlayingDecoded.CurrentlyDecoding != SongChangedIsCurrentlyFullyDecoding)
                        {
                            SongChangedIsCurrentlyFullyDecoding = false;
                            FinishChangeEntireSong(GameState, PlayingSong);
                            SetNewPlayingSong(Renderer, &DisplayInfo->PlayingSongPanel, Layout, MusicInfo);
                        }
                    }
                    else SongChangedIsCurrentlyFullyDecoding = true;
                }
                
                r32 PlayedSeconds = GetPlayedTime(GameState->SoundThreadInterface);
                if((u32)(DisplayInfo->PlayingSongPanel.CurrentTime/1000.0f) != (u32)PlayedSeconds)
                {
                    UpdatePanelTime(GameState, &DisplayInfo->PlayingSongPanel, Layout, PlayedSeconds);
                }
                UpdatePanelTimeline(&DisplayInfo->PlayingSongPanel, PlayedSeconds);
                
                ProcessShortcutPopup(&DisplayInfo->Popups, GameState->Time.dTime, Input->MouseP);
                
                // *******************************************
                // Sound *************************************
                // *******************************************
                b32 SongFinished = false;
                if(GetIsFinishedPlaying(GameState->SoundThreadInterface))
                {
                    SongFinished = true;
                    MusicInfo->IsPlaying = false;
                    if(PlayingSong->DisplayableID >= 0 || MusicInfo->UpNextList.A.Count > 0)
                    {
                        HandleChangeToNextSong(GameState);
                        if(MusicInfo->PlayingSong.DisplayableID >= 0) 
                            MusicInfo->IsPlaying = true;
                    }
                }
                if(PrevIsPlaying != MusicInfo->IsPlaying || SongFinished)
                {
                    PushIsPlaying(GameState->SoundThreadInterface, MusicInfo->IsPlaying);
                    PrevIsPlaying = MusicInfo->IsPlaying;
                }
                
                
                // *******************************************
                // Rendering *********************************
                // *******************************************
                *ReRender = true;
                if(!Renderer->Minimized) DisplayBufferInWindow(DeviceContext, Renderer);
                
                
                // Allocation debug stuff
#if DEBUG_TD
                CollectArenaDebugFrameData(&GameState->FixArena.DebugData);
                CollectArenaDebugFrameData(&GameState->ScratchArena.DebugData);
                CollectArenaDebugFrameData(&GameState->JobThreadsArena.DebugData);
                CollectArenaDebugFrameData(&GameState->SoundThreadArena.DebugData);
#endif
                
                // ****************************************************************
                // Sleep if we are faster than targeted framerate. ****************
                // ****************************************************************
                r32 dFrameWorkTime = GetSecondsElapsed(GameState->Time.PerfCountFrequency, CurrentCycleCount, GetWallClock());
                if(dFrameWorkTime < GameState->Time.GoalFrameRate)
                {
                    DWORD SleepMS = (DWORD)(1000.0f * (GameState->Time.GoalFrameRate - dFrameWorkTime));
                    if(GameState->Time.SleepIsGranular)
                    {
                        if(SleepMS > 0) Sleep(SleepMS);
                    }
                    else
                    {
                        // NOTE:: Just guessing that 10 ms might be enough for the scheduling granularity 
                        // to not oversleep.
                        if(SleepMS > 10) Sleep(SleepMS); 
                    }
                }
                
                SwapBuffers(DeviceContext);
            }
            
            SaveSettingsFile(GameState, &GameState->Settings);
            SaveMP3LibraryFile(GameState, MP3Info);
            DebugLog(25, "Settings saved.\n");
        }
        else
        {
            DebugLog(255, "%i\n", GetLastError());
        }
    }
    
    return 0;
}
