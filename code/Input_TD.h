#pragma once
#include <xinput.h>
#include "Math_TD.h"

// NOTE:: Defining and loading all xinput functions manually, for campaility reasons!
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub) { return 0; }
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) { return 0; }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// INPUT HANDLING
enum key_code
{
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, 
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, 
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, 
    
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 
    KEY_ESCAPE, KEY_ENTER, KEY_SPACE, KEY_BACKSPACE,
    KEY_SHIFT_RIGHT, KEY_SHIFT_LEFT, 
    KEY_CONTROL_RIGHT, KEY_CONTROL_LEFT, 
    KEY_ALT_RIGHT, KEY_ALT_LEFT,
    KEY_LMB, KEY_RMB, KEY_MMB,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, 
    // Sound buttons
    KEY_NEXT, KEY_PREVIOUS, KEY_STOP, KEY_PLAY_PAUSE,
    KEY_PLUS, KEY_MINUS, KEY_ADD, KEY_SUBTRACT,
    
    KEY_CODE_COUNT // NOTE(Tim):: This must always be the last key_code!
};

enum key_change
{
    NoChange,
    KeyDown, 
    KeyUp,
};

#define MAX_CHAR_PER_FRAME 20
#define MAX_HOTKEYS 10
struct input_info
{
    key_change KeyChange[KEY_CODE_COUNT];
    b32 Pressed[KEY_CODE_COUNT];
    i32 TapCount[KEY_CODE_COUNT];
    
    u8 Chars[MAX_CHAR_PER_FRAME];
    u32 CharCount;
    
    v2 MouseP;
    i32 WheelAmount;
    b32 _MouseMoved;
    
    key_code HotKeys[MAX_HOTKEYS];
    i32 _HotKeyCount;
};
