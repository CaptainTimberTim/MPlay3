#include "Input_TD.h"

internal void
LoadXInput()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll");
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

internal void
PullGamepadInput()
{
    For(XUSER_MAX_COUNT)
    {
        XINPUT_STATE ControllerState = {};
        if(XInputGetState(It, &ControllerState) == ERROR_SUCCESS)
        {
            // ControllerState.dwPacketNumber has info on how many times input was given since last pull
            XINPUT_GAMEPAD *Gamepad = &ControllerState.Gamepad;
            // TODO:: Actually implement this! 
        }
        else
        {
            // no controller available
        }
    }
}


inline void
ResetKeys(input_info *Input)
{
    For(KEY_CODE_COUNT)
    {
        Input->KeyChange[It] = NoChange;
        Input->TapCount[It] = 0;
    }
    Input->CharCount = 0;
    Input->WheelAmount = 0;
    Input->_MouseMoved = false;
}

inline void
UpdateSingleKey(input_info *Input, i32 Key, b32 WasDown, b32 IsDown)
{
    Input->Pressed[Key] = IsDown;
    if((WasDown && IsDown) || (!WasDown && !IsDown)) Input->KeyChange[Key] = NoChange;
    else if(WasDown && !IsDown) Input->KeyChange[Key] = KeyUp;
    else if(!WasDown && IsDown) Input->KeyChange[Key] = KeyDown;
    
    if(IsDown) Input->TapCount[Key]++;
    
#if 0
    DebugLog(255, "TapC: %i, Pressed: %i, Change: %i\n", Input->TapCount[Key], Input->Pressed[Key], Input->KeyChange[Key]);
#endif
}

internal b32
UpdateKeyChange(input_info *Input, u32 WinKey, u64 LParam)
{
    b32 Result = true;
    b32 WasDown = ((LParam & (1 << 30)) != 0);
    b32 IsDown = ((LParam & (1 << 31)) == 0);
    
    switch(WinKey)
    {
        case VK_LEFT:  UpdateSingleKey(Input, KEY_LEFT, WasDown, IsDown); break;
        case VK_RIGHT: UpdateSingleKey(Input, KEY_RIGHT, WasDown, IsDown); break;
        case VK_UP:    UpdateSingleKey(Input, KEY_UP, WasDown, IsDown); break;
        case VK_DOWN:  UpdateSingleKey(Input, KEY_DOWN, WasDown, IsDown); break;
        
        case VK_ESCAPE: UpdateSingleKey(Input, KEY_ESCAPE, WasDown, IsDown); break;
        case VK_RETURN: UpdateSingleKey(Input, KEY_ENTER, WasDown, IsDown); break;
        case VK_BACK:   UpdateSingleKey(Input, KEY_BACKSPACE, WasDown, IsDown); break;
        
        case VK_LSHIFT:   UpdateSingleKey(Input, KEY_SHIFT_LEFT, WasDown, IsDown); break;
        case VK_RSHIFT:   UpdateSingleKey(Input, KEY_SHIFT_RIGHT, WasDown, IsDown); break;
        case VK_LCONTROL: UpdateSingleKey(Input, KEY_CONTROL_LEFT, WasDown, IsDown); break;
        case VK_RCONTROL: UpdateSingleKey(Input, KEY_CONTROL_RIGHT, WasDown, IsDown); break;
        case VK_LMENU:    UpdateSingleKey(Input, KEY_ALT_LEFT, WasDown, IsDown); break;
        case VK_RMENU:    UpdateSingleKey(Input, KEY_ALT_RIGHT, WasDown, IsDown); break;
        
        case VK_LBUTTON: UpdateSingleKey(Input, KEY_LMB, WasDown, IsDown); break;
        case VK_RBUTTON: UpdateSingleKey(Input, KEY_RMB, WasDown, IsDown); break;
        case VK_MBUTTON: UpdateSingleKey(Input, KEY_MMB, WasDown, IsDown); break;
        
        case 176: UpdateSingleKey(Input, KEY_NEXT,       WasDown, IsDown); break;
        case 177: UpdateSingleKey(Input, KEY_PREVIOUS,   WasDown, IsDown); break;
        case 178: UpdateSingleKey(Input, KEY_STOP,       WasDown, IsDown); break; 
        case 179: UpdateSingleKey(Input, KEY_PLAY_PAUSE, WasDown, IsDown); break; 
        
        case VK_F1:  UpdateSingleKey(Input, KEY_F1, WasDown, IsDown); break;
        case VK_F2:  UpdateSingleKey(Input, KEY_F2, WasDown, IsDown); break;
        case VK_F3:  UpdateSingleKey(Input, KEY_F3, WasDown, IsDown); break;
        case VK_F4:  UpdateSingleKey(Input, KEY_F4, WasDown, IsDown); break;
        case VK_F5:  UpdateSingleKey(Input, KEY_F5, WasDown, IsDown); break;
        case VK_F6:  UpdateSingleKey(Input, KEY_F6, WasDown, IsDown); break;
        case VK_F7:  UpdateSingleKey(Input, KEY_F7, WasDown, IsDown); break;
        case VK_F8:  UpdateSingleKey(Input, KEY_F8, WasDown, IsDown); break;
        case VK_F9:  UpdateSingleKey(Input, KEY_F9, WasDown, IsDown); break;
        case VK_F10: UpdateSingleKey(Input, KEY_F10, WasDown, IsDown); break;
        case VK_F11: UpdateSingleKey(Input, KEY_F11, WasDown, IsDown); break;
        case VK_F12: UpdateSingleKey(Input, KEY_F12, WasDown, IsDown); break;
        
        // All these will also be reprocessed to a WM_CHAR
        case VK_SPACE:  UpdateSingleKey(Input, KEY_SPACE, WasDown, IsDown); Result = false; break;
        case 'A': UpdateSingleKey(Input, KEY_A, WasDown, IsDown); Result = false; break;
        case 'B': UpdateSingleKey(Input, KEY_B, WasDown, IsDown); Result = false; break;
        case 'C': UpdateSingleKey(Input, KEY_C, WasDown, IsDown); Result = false; break;
        case 'D': UpdateSingleKey(Input, KEY_D, WasDown, IsDown); Result = false; break;
        case 'E': UpdateSingleKey(Input, KEY_E, WasDown, IsDown); Result = false; break;
        case 'F': UpdateSingleKey(Input, KEY_F, WasDown, IsDown); Result = false; break;
        case 'G': UpdateSingleKey(Input, KEY_G, WasDown, IsDown); Result = false; break;
        case 'H': UpdateSingleKey(Input, KEY_H, WasDown, IsDown); Result = false; break;
        case 'I': UpdateSingleKey(Input, KEY_I, WasDown, IsDown); Result = false; break;
        case 'J': UpdateSingleKey(Input, KEY_J, WasDown, IsDown); Result = false; break;
        case 'K': UpdateSingleKey(Input, KEY_K, WasDown, IsDown); Result = false; break;
        case 'L': UpdateSingleKey(Input, KEY_L, WasDown, IsDown); Result = false; break;
        case 'M': UpdateSingleKey(Input, KEY_M, WasDown, IsDown); Result = false; break;
        case 'N': UpdateSingleKey(Input, KEY_N, WasDown, IsDown); Result = false; break;
        case 'O': UpdateSingleKey(Input, KEY_O, WasDown, IsDown); Result = false; break;
        case 'P': UpdateSingleKey(Input, KEY_P, WasDown, IsDown); Result = false; break;
        case 'Q': UpdateSingleKey(Input, KEY_Q, WasDown, IsDown); Result = false; break;
        case 'R': UpdateSingleKey(Input, KEY_R, WasDown, IsDown); Result = false; break;
        case 'S': UpdateSingleKey(Input, KEY_S, WasDown, IsDown); Result = false; break;
        case 'T': UpdateSingleKey(Input, KEY_T, WasDown, IsDown); Result = false; break;
        case 'U': UpdateSingleKey(Input, KEY_U, WasDown, IsDown); Result = false; break;
        case 'V': UpdateSingleKey(Input, KEY_V, WasDown, IsDown); Result = false; break;
        case 'W': UpdateSingleKey(Input, KEY_W, WasDown, IsDown); Result = false; break;
        case 'X': UpdateSingleKey(Input, KEY_X, WasDown, IsDown); Result = false; break;
        case 'Y': UpdateSingleKey(Input, KEY_Y, WasDown, IsDown); Result = false; break;
        case 'Z': UpdateSingleKey(Input, KEY_Z, WasDown, IsDown); Result = false; break;
        
        case '0': UpdateSingleKey(Input, KEY_0, WasDown, IsDown); Result = false; break;
        case '1': UpdateSingleKey(Input, KEY_1, WasDown, IsDown); Result = false; break;
        case '2': UpdateSingleKey(Input, KEY_2, WasDown, IsDown); Result = false; break;
        case '3': UpdateSingleKey(Input, KEY_3, WasDown, IsDown); Result = false; break;
        case '4': UpdateSingleKey(Input, KEY_4, WasDown, IsDown); Result = false; break;
        case '5': UpdateSingleKey(Input, KEY_5, WasDown, IsDown); Result = false; break;
        case '6': UpdateSingleKey(Input, KEY_6, WasDown, IsDown); Result = false; break;
        case '7': UpdateSingleKey(Input, KEY_7, WasDown, IsDown); Result = false; break;
        case '8': UpdateSingleKey(Input, KEY_8, WasDown, IsDown); Result = false; break;
        case '9': UpdateSingleKey(Input, KEY_9, WasDown, IsDown); Result = false; break;
        
        default: 
        //DebugLog(255, "The pressed button is not supported: %i\n", WinKey);
        Result = false;
    }
    
    return Result;
}

internal void
UpdateTypedCharacters(input_info *Input, u8 Char)
{
    Input->Chars[Input->CharCount++] = Char;
}

inline v2
GetRelativeMousePos(input_info *Input)
{
    v2 Result = Input->MouseP;
    return Result;
}

inline v2i
GetGlobalMousePos()
{
    v2i Result = {};
    POINT MouseP = {};
    if(GetCursorPos(&MouseP))
    {
        Result.x = MouseP.x;
        Result.y = MouseP.y;
    }
    
    return Result;
}

inline b32
GetMousePosInWindow(input_info *Input, i32 WindowWidth, i32 WindowHeight, v2 *Pos)
{
    b32 Result = false;
    
    if(Input->MouseP.x >= 0 && Input->MouseP.y >= 0 &&
       Input->MouseP.x < WindowWidth && Input->MouseP.y < WindowHeight)
    {
        Result = true;
        *Pos = Input->MouseP;
    }
    
    return Result;
}

inline b32
GetMousePosInViewport(input_info *Input, i32 WindowWidth, i32 WindowHeight, v2 *Pos)
{
    b32 Result = false;
    
    Result = (Input->MouseP.x >= 0 && Input->MouseP.y >= 0 &&
              Input->MouseP.x < WindowWidth && Input->MouseP.y < WindowHeight);
    
    Pos->x = (r32)Input->MouseP.x/WindowWidth;
    Pos->y = 1.0f - (r32)Input->MouseP.y/WindowHeight;
    
    return Result;
}

internal i32
KeyCodeToWindowsKey(key_code KeyCode)
{
    i32 Result = -1;
    switch(KeyCode)
    {
        case KEY_A: Result = 'A'; break;
        case KEY_B: Result = 'B'; break;
        case KEY_C: Result = 'C'; break;
        case KEY_D: Result = 'D'; break;
        case KEY_E: Result = 'E'; break;
        case KEY_F: Result = 'F'; break;
        case KEY_G: Result = 'G'; break;
        case KEY_H: Result = 'H'; break;
        case KEY_I: Result = 'I'; break;
        case KEY_J: Result = 'J'; break;
        case KEY_K: Result = 'K'; break;
        case KEY_L: Result = 'L'; break;
        case KEY_M: Result = 'M'; break;
        case KEY_N: Result = 'N'; break;
        case KEY_O: Result = 'O'; break;
        case KEY_P: Result = 'P'; break;
        case KEY_Q: Result = 'Q'; break;
        case KEY_R: Result = 'R'; break;
        case KEY_S: Result = 'S'; break;
        case KEY_T: Result = 'T'; break;
        case KEY_U: Result = 'U'; break;
        case KEY_V: Result = 'V'; break;
        case KEY_W: Result = 'W'; break;
        case KEY_X: Result = 'X'; break;
        case KEY_Y: Result = 'Y'; break;
        case KEY_Z: Result = 'Z'; break;
        
        case KEY_0: Result = '0'; break;
        case KEY_1: Result = '1'; break;
        case KEY_2: Result = '2'; break;
        case KEY_3: Result = '3'; break;
        case KEY_4: Result = '4'; break;
        case KEY_5: Result = '5'; break;
        case KEY_6: Result = '6'; break;
        case KEY_7: Result = '7'; break;
        case KEY_8: Result = '8'; break;
        case KEY_9: Result = '9'; break;
        
        case KEY_UP: Result = VK_UP; break;
        case KEY_DOWN: Result = VK_DOWN; break;
        case KEY_LEFT: Result = VK_LEFT; break;
        case KEY_RIGHT: Result = VK_RIGHT; break;
        case KEY_ESCAPE: Result = VK_ESCAPE; break;
        case KEY_ENTER: Result = VK_RETURN; break;
        case KEY_SPACE: Result = VK_SPACE; break;
        case KEY_BACKSPACE: Result = VK_BACK; break;
        case KEY_SHIFT_RIGHT: Result = VK_RSHIFT; break;
        case KEY_SHIFT_LEFT: Result = VK_LSHIFT; break;
        case KEY_CONTROL_RIGHT: Result = VK_RCONTROL; break;
        case KEY_CONTROL_LEFT: Result = VK_LCONTROL; break;
        case KEY_ALT_RIGHT: Result = VK_RMENU; break;
        case KEY_ALT_LEFT: Result = VK_LMENU; break;
        case KEY_LMB: Result = VK_LBUTTON; break;
        case KEY_RMB: Result = VK_RBUTTON; break;
        case KEY_MMB: Result = VK_MBUTTON; break;
        
        case KEY_F1: Result = VK_F1; break;
        case KEY_F2: Result = VK_F2; break;
        case KEY_F3: Result = VK_F3; break;
        case KEY_F4: Result = VK_F4; break;
        case KEY_F5: Result = VK_F5; break;
        case KEY_F6: Result = VK_F6; break;
        case KEY_F7: Result = VK_F7; break;
        case KEY_F8: Result = VK_F8; break;
        case KEY_F9: Result = VK_F9; break;
        case KEY_F10: Result = VK_F10; break;
        case KEY_F11: Result = VK_F11; break;
        case KEY_F12: Result = VK_F12; break;
        
        case KEY_NEXT:       Result = 176; break;
        case KEY_PREVIOUS:   Result = 177; break;
        case KEY_STOP:       Result = 178; break;
        case KEY_PLAY_PAUSE: Result = 179; break;
    }
    return Result;
}

internal void
HandleFocusRegain(input_info *Input)
{
    For(KEY_CODE_COUNT)
    {
        if(Input->Pressed[It])
        {
            if(GetKeyState(KeyCodeToWindowsKey((key_code)It)) & (1 << 15)) ;// If high-order bit is set, key is down.
            else Input->Pressed[It] = false;
        }
    }
}














