/* date = June 3rd 2021 7:59 am */
#ifndef _ERROR_HANDLING__T_D_H
#define _ERROR_HANDLING__T_D_H

#include "Renderer_TD.h"

enum error_codes
{
    errorCode_DecodingCanceled =  1,
    errorCode_NoError          =  0,
    errorCode_FileLoadFailed   = -1,
    errorCode_DecodingFailed   = -2,
    errorCode_EmptyFile        = -3,
    errorCode_Text             = -4,
};

struct user_error_text
{
    render_text Message;
    b32 IsAnimating;
    r32 dAnim;
    r32 AnimTime;
};

#define MAX_THREAD_ERRORS 20
struct error_item
{
    error_codes Code;
    i32 ID;
    string_c ErrorText;
};

struct thread_error_list
{
    error_item Errors[MAX_THREAD_ERRORS];
    u32 Count;
    
    b32 RemoveDecode;
    HANDLE Mutex;
};

internal void PushErrorMessage(struct game_state *GS, string_c ErrorText);
internal void PushErrorMessage(struct game_state *GS, error_item Error);

inline void AnimateErrorMessage(renderer *Renderer, user_error_text *ErrorInfo, r32 dTime);






#endif //_ERROR_HANDLING__T_D_H
