#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define internal        static
#define local_persist   static 
#define global_variable static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32     b32;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  r32;
typedef double r64;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

// NOTE:: For loop simplification macro. first param: Count until, second param(optional): Iterater name prefix ...It
#define For(until, ...) \
for(u32 (__VA_ARGS__##It) = 0; \
(__VA_ARGS__##It) < (until); \
++(__VA_ARGS__##It))
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define RemoveItem(Array, ArrayCount, RemovePos, ArrayType) { \
u32 MoveSize  = (ArrayCount-RemovePos)*sizeof(ArrayType); \
u8 *GoalAddress  = ((u8 *)Array) + (RemovePos*sizeof(ArrayType)); \
u8 *StartAddress = GoalAddress + sizeof(ArrayType); \
memmove_s(GoalAddress, MoveSize, StartAddress, MoveSize); \
} 

#define Combine1(X, Y) X##Y
#define Combine(X, Y) Combine1(X, Y)

#define DebugLog(Count, Text, ...) { \
char Combine(B, __LINE__)[Count]; \
sprintf_s(Combine(B, __LINE__), Text, __VA_ARGS__);\
OutputDebugStringA(Combine(B, __LINE__)); \
}

#if DEBUG_TD
#define Assert(Expression)  {            \
if(!(Expression))                        \
{                                        \
DebugLog(1000, "Assert fired at:\nLine: %i\nFile: %s\n", __LINE__, __FILE__); \
*(int *)0 = 0;                       \
} }                                       
#else
#define Assert(Expression)
#endif
#define InvalidCodePath    Assert(!"InvalidCodePath")
#define NotImplemented     Assert(!"NotImplementedYet")
#define InvalidDefaultCase default: {Assert(false)}


// TODO:: Implement STBI_MALLOC, STBI_REALLOC and STBI_FREE!
#define STB_IMAGE_IMPLEMENTATION
//#define STBI_FAILURE_USERMSG
#define STBI_NO_FAILURE_STRINGS
#include "STB_Image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "STB_Truetype.h"

#define MINIMP3_IMPLEMENTATION
#include "MiniMP3_Ext.h"

#include "Math_TD.h"
struct time_management
{
    i32 GameHz;
    r32 GoalFrameRate;
    r32 dTime;
    r64 GameTime;
    r32 CurrentTimeSpeed;
};
internal b32 TryGetClipboardText(struct string_compound *String);
internal void ApplyWindowResize(HWND Window, i32 NewWidth, i32 NewHeight, b32 ForceResize = false);
inline v2i GetWindowSize();

#include "Input_TD.cpp"
#include "String_TD.h"
#include "StandardUtilities_TD.cpp"
#include "Threading_TD.c"
#include "Math_TD.cpp"
#include "FileUtilities_TD.cpp"
#include "GL_TD.c"

#if DEBUG_TD
#define STBI_ASSERT(x) if(!(x)) {*(int *)0 = 0;}
#else
#define STBI_ASSERT(x)
#endif

global_variable game_state GlobalGameState;

global_variable i32 GlobalMinWindowWidth  = 344;
global_variable i32 GlobalMinWindowHeight = 190;

#include "Sound_Backend_TD.c"
#include "Sound_UI_TD.c"
#include "Sound_TD.c"
#include "Renderer_TD.cpp"
#include "UI_TD.c"
#include "GameBasics_TD.cpp"

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
        music_sorting_info *SortingInfo = &GameState->MusicInfo.SortingInfo;
        
        PerformScreenTransform(Renderer);
        ProcessEdgeDragOnResize(Renderer, DisplayInfo);
        
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &SortingInfo->Genre, &DisplayInfo->Genre);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &SortingInfo->Artist, &DisplayInfo->Artist);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &SortingInfo->Album, &DisplayInfo->Album);
        ProcessWindowResizeForDisplayColumn(Renderer, &GlobalGameState.MusicInfo, &SortingInfo->Song, &DisplayInfo->Song.Base);
        
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
        case WM_CLOSE: { IsRunning = false; } break;
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
            EndPaint(Window, &Paint);
            ReleaseDC(Window, DeviceContext);
        } break;
        case WM_SETCURSOR: 
        {
            if (GlobalGameState.CursorState == cursorState_Arrow) 
                Result = DefWindowProcA(Window, Message, WParam, LParam);
            else if(GlobalGameState.CursorState == cursorState_Drag) 
                SetCursor(DragCursor);
        } break;
        case WM_SETFOCUS:
        {
            HandleFocusRegain(&GlobalGameState.Input);
        } break;
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
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
    while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT: { IsRunning = false; } break;
            case WM_CHAR: 
            {
                UpdateTypedCharacters(Input, (u8)Message.wParam);
            } break;
            case WM_SYSKEYDOWN: 
            case WM_SYSKEYUP: 
            case WM_KEYDOWN: 
            case WM_KEYUP:
            {
                u32 Scancode = (Message.lParam & 0x00ff0000) >> 16;
                i32 Extended  = (Message.lParam & 0x01000000) != 0;
                
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
                            DispatchMessage(&Message);
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
        HANDLE Data = GetClipboardData(CF_TEXT);
        if (Data)
        {
            // Lock the handle to get the actual text pointer
            u8 *Text = (u8 *)GlobalLock(Data);
            if (Text)
            {
                Result = true;
                
                if(StringLength(Text) >= String->Length) Text[String->Length-1] = 0;
                else AppendStringToCompound(String, Text);
                
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

inline LARGE_INTEGER
GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline r32
GetSecondsElapsed(i64 PerfCountFrequency, LARGE_INTEGER Start, LARGE_INTEGER End)
{
    r32 Result = ((r32)(End.QuadPart - Start.QuadPart) / (r32)PerfCountFrequency);
    return(Result);
}

#ifdef CONSOLE_APP
int main(int argv, char** arcs) 
#else
i32 CALLBACK 
WinMain(HINSTANCE Instance, 
        HINSTANCE PrevInstance, 
        LPSTR CmdLine, 
        i32 ShowCmd)
#endif
{
    // Hey, baby! Check out the nil-value _I'm_ dereferencing.
    
    ArrowCursor = LoadCursor(0, IDC_ARROW);
    DragCursor = LoadCursor(0, IDC_SIZEWE);
    
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = WindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = ArrowCursor;
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WindowClass.lpszClassName = "MPlay3ClassName";
    
    if(RegisterClassA(&WindowClass))
    {
        // Initializing GameState
        game_state *GameState = &GlobalGameState;
        *GameState = {};
        GameState->DataPath = NewStaticStringCompound("..\\data\\");
        GameState->Time.GameHz           = 60; // TODO:: Get monitor refresh rate!?
        GameState->Time.GoalFrameRate    = 1.0f/GameState->Time.GameHz;
        GameState->Time.dTime            = 0.0f;
        GameState->Time.GameTime         = 0.0f;
        GameState->Time.CurrentTimeSpeed = 1.0f;
        GameState->Input                 = {};
        input_info *Input = &GameState->Input;
        
        // Initializing clock
        LARGE_INTEGER PerfCountFrequencyResult;
        QueryPerformanceFrequency(&PerfCountFrequencyResult);
        i64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
        LARGE_INTEGER PrevCycleCount = GetWallClock();
        LARGE_INTEGER FlipWallClock  = GetWallClock();
        
        SetRandomSeed(FlipWallClock.QuadPart);
        
        // Initializing Allocator
        GameState->Bucket = {};
        if(!CreateBucketAllocator(&GameState->Bucket, Gigabytes(2), Megabytes(500))) return -1;
        if(!CreateBucketAllocator(&GameState->SoundThreadBucket, Megabytes(1), Megabytes(250))) return -1;
        u8 BucketStatus[100];
        BucketAllocatorFillStatus(&GameState->Bucket, BucketStatus);
        printf("%s", BucketStatus);
        
        // NOTE:: Loading Settings file
        settings Settings = TryLoadSettingsFile(GameState);
        
        u32 InitialWindowWidth  = Settings.WindowDimX;//1416;
        u32 InitialWindowHeight = Settings.WindowDimY;//1039;
        HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "MPlay3", 
                                      WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, 
                                      CW_USEDEFAULT, InitialWindowWidth, InitialWindowHeight, 
                                      0, 0, Instance, 0);
        
        if(Window)
        {
            HDC DeviceContext = GetDC(Window);
            InitOpenGL(Window);
            
            // ********************************************
            // Threading***********************************
            // ********************************************
            
            HANDLE JobHandles[THREAD_COUNT];
            circular_job_queue *JobQueue = &GameState->JobQueue;
            *JobQueue = {};
            job_thread_info JobInfos[THREAD_COUNT];
            InitializeJobThreads(&GameState->Bucket, JobHandles, JobQueue, JobInfos);
            
            sound_thread_data SoundThreadData = {};
            sound_thread SoundThreadInfo = {};
            GameState->SoundThreadInterface = &SoundThreadInfo.Interface;
            SoundThreadInfo.Interface.SoundMutex = CreateMutexExA(0, 0, 0, MUTEX_ALL_ACCESS);
            SoundThreadInfo.Interface.ToneVolume = SoundThreadInfo.SoundInfo.ToneVolume = 0.5f;
            
            NewSoundInstance(&SoundThreadInfo.SoundInfo.SoundInstance, Window, 48000, 2, GameState->Time.GameHz);
            SoundThreadInfo.SoundSamples = PushArrayOnBucket(&GameState->Bucket.Fixed, 
                                                             SoundThreadInfo.SoundInfo.SoundInstance.BufferSize, 
                                                             i16);
            
            InitializeSoundThread(&SoundThreadData, ProcessSound, &SoundThreadInfo);
            
            // Initializing sound
            //sound_info SoundInfo = {};
            
            
            renderer *Renderer = &GameState->Renderer;
            *Renderer = InitializeRenderer(GameState, Window);
            
            ReshapeGLWindow(&GameState->Renderer);
            b32 *ReRender = &Renderer->Rerender;
            
            
            
            // Preparing base structs
            music_info *MusicInfo = &GameState->MusicInfo;
            playing_song *PlayingSong = &MusicInfo->PlayingSong;
            *PlayingSong = {-1, -1, -1};
            mp3_info *MP3Info = 0;
            
            // Preparing arguments for loading files
            b32 LoadedLibraryFile = false;
            u32 FileInfoCount = 0;
            if(ConfirmLibraryWithCorrectVersionExists(GameState, CURRENT_LIBRARY_VERSION, &FileInfoCount))
            {
                MP3Info = CreateMP3InfoStruct(&GameState->Bucket.Fixed, FileInfoCount);
                LoadMP3LibraryFile(GameState, MP3Info);
                LoadedLibraryFile = true;
            }
            else
            {
                MP3Info = CreateMP3InfoStruct(&GameState->Bucket.Fixed, 0);
                CreateOrWipeStringComp(&GameState->Bucket.Fixed, &MP3Info->FolderPath, 255);
            }
            GameState->MP3Info = MP3Info;
            MP3Info->MusicInfo = &GameState->MusicInfo;
            
            MusicInfo->Playlist.Songs  = CreateArray(&GameState->Bucket.Fixed, MP3Info->FileInfo.Count+200);
            MusicInfo->Playlist.UpNext = CreateArray(&GameState->Bucket.Fixed, 200);
            
            music_sorting_info *SortingInfo = &MusicInfo->SortingInfo;
            CreateMusicSortingInfo(&GameState->Bucket, MP3Info);
            FillDisplayables(SortingInfo, &MP3Info->FileInfo, &MusicInfo->DisplayInfo);
            if(LoadedLibraryFile) SortDisplayables(SortingInfo, &MP3Info->FileInfo);
            UseDisplayableAsPlaylist(MusicInfo);
            
            // ********************************************
            // UI rendering stuff   ***********************
            // ********************************************
            r32 WWidth = (r32)Renderer->Window.FixedDim.Width;
            r32 WMid    = WWidth*0.5f;
            r32 WHeight = (r32)Renderer->Window.FixedDim.Height;
            
            Renderer->FontInfo.BigFont    = InitSTBBakeFont(GameState, 75);
            Renderer->FontInfo.MediumFont = InitSTBBakeFont(GameState, 50);
            Renderer->FontInfo.SmallFont  = InitSTBBakeFont(GameState, 25);
            
            
            MusicInfo->DisplayInfo.MusicBtnInfo = {GameState, PlayingSong};
            InitializeDisplayInfo(&MusicInfo->DisplayInfo, GameState, MP3Info);
            music_display_info *DisplayInfo = &MusicInfo->DisplayInfo;
            
            
            r32 SlotHeight = 30;
            r32 DisplayColumnStartY = DisplayInfo->EdgeTop->ID->Vertice[0].y;
            CreateDisplayColumn(&GameState->Bucket.Fixed, Renderer, DisplayInfo, &DisplayInfo->Genre, &SortingInfo->Genre,
                                columnType_Genre, SlotHeight, -0.025f, DisplayInfo->EdgeLeft,
                                DisplayInfo->GenreArtist.Edge, 28);
            MoveDisplayColumn(Renderer, &DisplayInfo->Genre);
            ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &SortingInfo->Genre, &DisplayInfo->Genre);
            
            CreateDisplayColumn(&GameState->Bucket.Fixed, Renderer, DisplayInfo, &DisplayInfo->Artist, &SortingInfo->Artist,
                                columnType_Artist, SlotHeight, -0.05f, DisplayInfo->GenreArtist.Edge,
                                DisplayInfo->ArtistAlbum.Edge, 20);
            MoveDisplayColumn(Renderer, &DisplayInfo->Artist);
            ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &SortingInfo->Artist, &DisplayInfo->Artist);
            
            CreateDisplayColumn(&GameState->Bucket.Fixed, Renderer, DisplayInfo, &DisplayInfo->Album, &SortingInfo->Album,
                                columnType_Album, SlotHeight, -0.075f, DisplayInfo->ArtistAlbum.Edge,
                                DisplayInfo->AlbumSong.Edge, 20);
            MoveDisplayColumn(Renderer, &DisplayInfo->Album);
            ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &SortingInfo->Album, &DisplayInfo->Album);
            
            CreateDisplayColumn(&GameState->Bucket.Fixed, Renderer, DisplayInfo, Parent(&DisplayInfo->Song),
                                &SortingInfo->Song, columnType_Song, 100.0f, -0.1f, DisplayInfo->AlbumSong.Edge,
                                DisplayInfo->EdgeRight, 35);
            MoveDisplayColumn(Renderer, &DisplayInfo->Song.Base);
            ProcessWindowResizeForDisplayColumn(Renderer, MusicInfo, &SortingInfo->Song, &DisplayInfo->Song.Base);
            
            //UpdateAllDisplayColumns(GameState);
            
            // ********************************************
            // Dragging************************************
            // ********************************************
            drag_list *DragableList = &GameState->DragableList;
            DragableList->DraggingID = -1;
            
            
            column_edge_drag EdgeGenreArtistDrag = {DisplayInfo->EdgeLeft, 40, DisplayInfo->ArtistAlbum.Edge, 30, 
                &DisplayInfo->GenreArtist.XPercent, DisplayInfo, SortingInfo};
            AddDragable(DragableList, DisplayInfo->GenreArtist.Edge, {}, {OnDisplayColumnEdgeDrag, &EdgeGenreArtistDrag}, {});
            
            column_edge_drag EdgeArtistAlbumDrag = {DisplayInfo->GenreArtist.Edge, 30, DisplayInfo->AlbumSong.Edge, 30, 
                &DisplayInfo->ArtistAlbum.XPercent, DisplayInfo, SortingInfo};
            AddDragable(DragableList, DisplayInfo->ArtistAlbum.Edge, {}, {OnDisplayColumnEdgeDrag, &EdgeArtistAlbumDrag}, {});
            
            column_edge_drag EdgeAlbumSongDrag = {DisplayInfo->ArtistAlbum.Edge, 40, DisplayInfo->EdgeRight, 40, 
                &DisplayInfo->AlbumSong.XPercent, DisplayInfo, SortingInfo};
            AddDragable(DragableList, DisplayInfo->AlbumSong.Edge, {}, {OnDisplayColumnEdgeDrag, &EdgeAlbumSongDrag}, {});
            
            
            drag_slider_data GenreDrag  = {MusicInfo, &DisplayInfo->Genre, &SortingInfo->Genre};
            drag_slider_data ArtistDrag = {MusicInfo, &DisplayInfo->Artist, &SortingInfo->Artist};
            drag_slider_data AlbumDrag  = {MusicInfo, &DisplayInfo->Album, &SortingInfo->Album};
            drag_slider_data SongDrag   = {MusicInfo, &DisplayInfo->Song.Base, &SortingInfo->Song};
            
            AddDragable(DragableList, DisplayInfo->Genre.SliderHorizontal.Background,  
                        {OnSliderDragStart, &GenreDrag}, {OnHorizontalSliderDrag, &GenreDrag}, {});
            AddDragable(DragableList, DisplayInfo->Artist.SliderHorizontal.Background, 
                        {OnSliderDragStart, &ArtistDrag}, {OnHorizontalSliderDrag, &ArtistDrag}, {});
            AddDragable(DragableList, DisplayInfo->Album.SliderHorizontal.Background,  
                        {OnSliderDragStart, &AlbumDrag}, {OnHorizontalSliderDrag, &AlbumDrag}, {});
            AddDragable(DragableList, DisplayInfo->Song.Base.SliderHorizontal.Background,
                        {OnSliderDragStart, &SongDrag}, {OnHorizontalSliderDrag, &SongDrag},{});
            
            AddDragable(DragableList, DisplayInfo->Genre.SliderVertical.Background,
                        {OnSliderDragStart, &GenreDrag}, {OnVerticalSliderDrag, &GenreDrag}, {});
            AddDragable(DragableList, DisplayInfo->Artist.SliderVertical.Background,
                        {OnSliderDragStart, &ArtistDrag}, {OnVerticalSliderDrag, &ArtistDrag}, {});
            AddDragable(DragableList, DisplayInfo->Album.SliderVertical.Background,
                        {OnSliderDragStart, &AlbumDrag}, {OnVerticalSliderDrag, &AlbumDrag}, {});
            AddDragable(DragableList, DisplayInfo->Song.Base.SliderVertical.Background,
                        {OnSliderDragStart, &SongDrag}, {OnVerticalSliderDrag, &SongDrag}, {OnSongDragEnd, &SongDrag});
            
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
            
            if(!LoadedLibraryFile) OnMusicPathPressed(&DisplayInfo->MusicBtnInfo);
            
            
            check_music_path *CheckMusicPath = &GameState->CheckMusicPath;
            AddJob_CheckMusicPathChanged(CheckMusicPath);
            ApplySettings(GameState, Settings);
            
            
            // ********************************************
            // Threading **********************************
            // ********************************************
            AddJobs_LoadOnScreenMP3s(GameState, JobQueue);
            
            
            crawl_thread_out CrawlInfoOut = {false, false, false, 0};
            GameState->CrawlInfo = {MP3Info, NewStringCompound(&GameState->Bucket.Fixed, 255), &CrawlInfoOut};
            
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
            
            b32 SongChangedIsCurrentlyDecoding = false;
            b32 PrevIsPlaying = MusicInfo->IsPlaying;
            scroll_load_info ScrollLoadInfo = {0, 0.5f, true};
            IsRunning = true;
            while(IsRunning)
            {
                LARGE_INTEGER CurrentCycleCount = GetWallClock();
                GameState->Time.dTime = GetSecondsElapsed(PerfCountFrequency, PrevCycleCount, CurrentCycleCount);
                PrevCycleCount = CurrentCycleCount;
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
                        string_c FPSComp = NewStringCompound(&GameState->Bucket.Transient, 10);
                        AppendStringToCompound(&FPSComp, (u8 *)FPSString);
                        RemoveRenderText(&FPSText);
                        CreateRenderText(Renderer, Renderer->FontInfo.SmallFont, 
                                         &FPSComp, &DisplayInfo->ColorPalette.ForegroundText, &FPSText, -0.9f, FPSParent);
                        DeleteStringCompound(&GameState->Bucket.Transient, &FPSComp);
                    }
                    else dUpdateRate += GameState->Time.dTime/0.1f;
                    FPSCount = (FPSCount+1)%100;
                }
#endif
                
                // *******************************************
                // Thread handling ****************************
                // *******************************************
                EmptyJobQueueWhenPossible(JobQueue);
                
                if(CheckMusicPath->State == threadState_Finished) 
                    CheckForMusicPathMP3sChanged_End(CheckMusicPath, &DisplayInfo->MusicPath);
                
                // *******************************************
                // Input handling ****************************
                // *******************************************
                ProcessPendingMessages(Input, Window);
                
                if(Input->KeyChange[KEY_ESCAPE] == KeyDown)
                {
                    if(DisplayInfo->MusicPath.TextField.IsActive && 
                       !CrawlInfoOut.ThreadIsRunning) 
                    {
                        // TODO:: Maybe rather kill crawl thread if escape is pressed?
                        // TODO:: Do not save when I directly exit from here
                        OnMusicPathPressed(&DisplayInfo->MusicBtnInfo);
                    }
                    else if(IsSearchOpen(DisplayInfo))
                    {
                        InterruptSearch(Renderer, DisplayInfo, SortingInfo);
                    }
                    else
                    {
                        IsRunning = false;
                        continue;
                    }
                }
                if(Input->Pressed[KEY_ALT_LEFT] && Input->KeyChange[KEY_F4] == KeyDown) 
                {
                    IsRunning = false;
                    continue;
                }
                
                
                if(DisplayInfo->MusicPath.TextField.IsActive)
                {
                    HandleActiveMusicPath(DisplayInfo, Input, &CrawlInfoOut);
                }
                else // This is only reachable if MusicPath stuff is not on screen
                {
                    Assert(!CrawlInfoOut.ThreadIsRunning);
                    Assert(!DisplayInfo->MusicPath.TextField.IsActive);
                    
                    TestHoveringEdgeDrags(GameState, Input->MouseP, DisplayInfo);
                    
                    // Command keys ******
                    if(Input->KeyChange[KEY_F1] == KeyDown) OnSearchPressed(&DisplayInfo->Genre.SearchInfo);
                    if(Input->KeyChange[KEY_F2] == KeyDown) OnSearchPressed(&DisplayInfo->Artist.SearchInfo);
                    if(Input->KeyChange[KEY_F3] == KeyDown) OnSearchPressed(&DisplayInfo->Album.SearchInfo);
                    if(Input->KeyChange[KEY_F4] == KeyDown) OnSearchPressed(&DisplayInfo->Song.Base.SearchInfo);
                    if(Input->KeyChange[KEY_F9] == KeyDown)
                    {
                        SaveMP3LibraryFile(GameState, MP3Info);
                        DebugLog(25, "Saved library file.\n");
                    }
                    if(Input->KeyChange[KEY_F10] == KeyDown)
                    {
                        AddJob_CheckMusicPathChanged(CheckMusicPath);
                        //OutputDebugStringA("Start crawling");
                        //CreateNewMetadata(GameState);
                        //OutputDebugStringA(" finished!\n");
                    }
                    // To use F12 in VS look at: https://conemu.github.io/en/GlobalHotKeys.html
                    if(Input->KeyChange[KEY_F11] == KeyDown) 
                    {
                        DisplayInfo->ColorPaletteID = ++DisplayInfo->ColorPaletteID%5;
                        UpdateColorPalette(DisplayInfo);
                    }
                    // ******************
                    
                    if(Input->KeyChange[KEY_RIGHT] == KeyDown ||
                       Input->KeyChange[KEY_NEXT]  == KeyDown)
                    {
                        if(PlayingSong->PlaylistID >= 0)
                        {
                            HandleChangeToNextSong(GameState);
                        }
                    }
                    else if(Input->KeyChange[KEY_LEFT]     == KeyDown ||
                            Input->KeyChange[KEY_PREVIOUS] == KeyDown )
                    {
                        if(PlayingSong->PlaylistID >= 0)
                        {
                            if(GetPlayedTime(GameState->SoundThreadInterface) < 5.0f)
                            {
                                SetPreviousSong(&MusicInfo->Playlist, PlayingSong, MusicInfo->Looping);
                                ChangeSong(GameState, PlayingSong);
                                KeepPlayingSongOnScreen(Renderer, MusicInfo);
                            }
                            else
                            {
                                ChangeSong(GameState, PlayingSong);
                            }
                        }
                    }
                    
                    if(Input->KeyChange[KEY_SPACE] == KeyDown ||
                       Input->KeyChange[KEY_PLAY_PAUSE] == KeyDown)
                    {
                        if(DisplayInfo->SearchIsActive >= 0 && Input->KeyChange[KEY_SPACE] == KeyDown) ;
                        else
                        {
                            MusicInfo->IsPlaying = !MusicInfo->IsPlaying;
                            if(!MusicInfo->IsPlaying) 
                            {
                                PushSoundBufferClear(GameState->SoundThreadInterface);
                                DisplayInfo->PlayPause->State = buttonState_Unpressed;
                                DisplayInfo->PlayPause->Entry->ID->Color = DisplayInfo->PlayPause->BaseColor;
                            }
                            else
                            {
                                DisplayInfo->PlayPause->State = buttonState_Pressed;
                                DisplayInfo->PlayPause->Entry->ID->Color = DisplayInfo->PlayPause->DownColor;
                            }
                        }
                    }
                    if(Input->KeyChange[KEY_STOP] == KeyDown)
                    {
                        if(PlayingSong->PlaylistID >= 0)
                        {
                            ChangeSong(GameState, PlayingSong);
                            MusicInfo->IsPlaying = false;
                            PushSoundBufferClear(GameState->SoundThreadInterface);
                        }
                    }
                    
                    if(Input->Pressed[KEY_UP])
                    {
                        PushToneVolume(GameState->SoundThreadInterface, 0.01f);
                    }
                    else if(Input->Pressed[KEY_DOWN])
                    {
                        PushToneVolume(GameState->SoundThreadInterface, -0.01f);
                    }
                    
                    UpdateButtons(Renderer, Input);
                    
                    if(Input->KeyChange[KEY_LMB] == KeyDown)
                    {
                        OnDraggingStart(DragableList, Renderer, Input->MouseP);
                    }
                    if(Input->Pressed[KEY_LMB] && 
                       DragableList->DraggingID >= 0)
                    {
                        OnDragging(DragableList, Renderer, Input->MouseP);
                    }
                    else if(DragableList->DraggingID >= 0) 
                    {
                        OnDraggingEnd(DragableList, Renderer, Input->MouseP);
                    }
                    else if(Input->KeyChange[KEY_LMB] == KeyUp &&
                            DragableList->DraggingID < 0)
                    {
                        column_type ChangedSelection = columnType_None;
                        if(IsInRect(DisplayInfo->Song.Base.Background, Input->MouseP))
                        {
                            if(UpdateSelectionArray(Renderer, &SortingInfo->Song, &DisplayInfo->Song.Base, Input))
                            {
                                UpdateColumnColor(&DisplayInfo->Song.Base, &SortingInfo->Song);
                            }
                        }
                        else if(IsInRect(DisplayInfo->Genre.Background, Input->MouseP))
                        {
                            ChangedSelection = UpdateSelectionArray(Renderer, &SortingInfo->Genre, 
                                                                    &DisplayInfo->Genre, Input);
                            if(ChangedSelection)
                            {
                                PropagateSelectionChange(SortingInfo);
                                DisplayInfo->Artist.DisplayCursor = 0;
                                DisplayInfo->Album.DisplayCursor  = 0;
                            }
                        }
                        else if(IsInRect(DisplayInfo->Artist.Background, Input->MouseP))
                        {
                            ChangedSelection = UpdateSelectionArray(Renderer, &SortingInfo->Artist, 
                                                                    &DisplayInfo->Artist, Input);
                            if(ChangedSelection)
                            {
                                PropagateSelectionChange(SortingInfo);
                                DisplayInfo->Album.DisplayCursor  = 0;
                            }
                        }
                        else if(IsInRect(DisplayInfo->Album.Background, Input->MouseP))
                        {
                            ChangedSelection = UpdateSelectionArray(Renderer, &SortingInfo->Album, 
                                                                    &DisplayInfo->Album, Input);
                        }
                        
                        if(ChangedSelection != columnType_None)
                        {
                            UpdateSelectionChanged(Renderer, MusicInfo, MP3Info, ChangedSelection);
                            AddJobs_LoadOnScreenMP3s(GameState, JobQueue);
                        }
                    }
                    
                    if(Input->WheelAmount != 0)
                    {
                        
                        if(IsInRect(DisplayInfo->Song.Base.Background, Input->MouseP))
                        {
                            ScrollDisplayColumn(Renderer, &DisplayInfo->Song.Base, (r32)Input->WheelAmount);
                            UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Song.Base, &SortingInfo->Song);
                            UpdateColumnVerticalSliderPosition(Renderer, &DisplayInfo->Song.Base, &SortingInfo->Song);
                        }
                        if(IsInRect(DisplayInfo->Genre.Background, Input->MouseP))
                        {
                            ScrollDisplayColumn(Renderer, &DisplayInfo->Genre, (r32)Input->WheelAmount);
                            UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Genre, &SortingInfo->Genre);
                            UpdateColumnVerticalSliderPosition(Renderer, &DisplayInfo->Genre, &SortingInfo->Genre);
                        }
                        if(IsInRect(DisplayInfo->Artist.Background, Input->MouseP))
                        {
                            ScrollDisplayColumn(Renderer, &DisplayInfo->Artist, (r32)Input->WheelAmount);
                            UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Artist, &SortingInfo->Artist);
                            UpdateColumnVerticalSliderPosition(Renderer, &DisplayInfo->Artist, &SortingInfo->Artist);
                        }
                        if(IsInRect(DisplayInfo->Album.Background, Input->MouseP))
                        {
                            ScrollDisplayColumn(Renderer, &DisplayInfo->Album, (r32)Input->WheelAmount);
                            UpdateColumnHorizontalSlider(Renderer, &DisplayInfo->Album, &SortingInfo->Album);
                            UpdateColumnVerticalSliderPosition(Renderer, &DisplayInfo->Album, &SortingInfo->Album);
                        }
                        
                        ScrollLoadInfo.LoadFinished = false;
                        ScrollLoadInfo.dTime = 0;
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
                    
                    // Search bar  ******************
                    ProcessActiveSearch(Renderer, &DisplayInfo->Genre, GameState->Time.dTime, Input,
                                        &MusicInfo->SortingInfo.Genre);
                    ProcessActiveSearch(Renderer, &DisplayInfo->Artist, GameState->Time.dTime, Input,
                                        &MusicInfo->SortingInfo.Artist);
                    ProcessActiveSearch(Renderer, &DisplayInfo->Album, GameState->Time.dTime, Input,
                                        &MusicInfo->SortingInfo.Album);
                    ProcessActiveSearch(Renderer, &DisplayInfo->Song.Base, GameState->Time.dTime, Input,
                                        &MusicInfo->SortingInfo.Song, &MP3Info->FileInfo);
                    
                    if(PrevIsPlaying != MusicInfo->IsPlaying)
                    {
                        PrevIsPlaying = MusicInfo->IsPlaying;
                        ToggleButtonVisuals(DisplayInfo->PlayPause, MusicInfo->IsPlaying);
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
                            SetTheNewPlayingSong(Renderer, &DisplayInfo->PlayingSongPanel, MusicInfo);
                        }
                    }
                    else SongChangedIsCurrentlyDecoding = true;
                }
                
                r32 PlayedSeconds = GetPlayedTime(GameState->SoundThreadInterface);
                if((u32)(DisplayInfo->PlayingSongPanel.CurrentTime/1000.0f) != (u32)PlayedSeconds)
                {
                    UpdatePanelTime(Renderer, &DisplayInfo->PlayingSongPanel, PlayedSeconds);
                }
                UpdatePanelTimeline(&DisplayInfo->PlayingSongPanel, PlayedSeconds);
                
                // *******************************************
                // Sound *************************************
                // *******************************************
                if(GetIsFinishedPlaying(GameState->SoundThreadInterface))
                {
                    MusicInfo->IsPlaying = false;
                    if(PlayingSong->PlaylistID >= 0 || MusicInfo->Playlist.UpNext.Count > 0)
                    {
                        PushIsPlaying(GameState->SoundThreadInterface, MusicInfo->IsPlaying);
                        HandleChangeToNextSong(GameState);
                        if(MusicInfo->PlayingSong.PlaylistID >= 0) MusicInfo->IsPlaying = true;
                    }
                }
                if(!MusicInfo->IsPlaying || !MusicInfo->CurrentlyChangingSong)
                    PushIsPlaying(GameState->SoundThreadInterface, MusicInfo->IsPlaying);
                
                // *******************************************
                // Rendering *********************************
                // *******************************************
                *ReRender = true;
                if(!Renderer->Minimized) DisplayBufferInWindow(DeviceContext, Renderer);
                
                FlipWallClock = GetWallClock();
                
            }
            
            SaveSettingsFile(GameState);
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
