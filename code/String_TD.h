#pragma once
#include "Math_TD.h"
#include "StandardUtilities_TD.h"

struct string_compound
{
    u8 *S;
    u32 Pos;
    u32 Length;
};
typedef string_compound string_c;

#define NewStaticStringCompound(String) {(u8 *)String, StringLength((u8 *)String), StringLength((u8 *)String)}
inline string_c NewStringCompound(memory_bucket_container *BucketContainer, u32 Size);
inline void DeleteStringCompound(memory_bucket_container *BucketContainer, string_compound *S);
#define ResetStringCompound(SC) {(SC).Pos = 0; (SC).S[0] = '\0';}

inline void AppendCharToCompound(string_compound *Comp, u8 C);
inline void AppendStringToCompound(string_compound *Comp, u8 *String); 
inline void CopyStringToCompound(string_compound *Comp, u8 *String, u32 StartPos);
inline void AppendStringCompoundToCompound(string_compound *Comp1, string_compound *Comp2); 

inline void WipeStringCompound(string_compound *Comp);
inline void CreateOrWipeStringComp(memory_bucket_container *Bucket, string_c *C);

inline void CombineStringCompounds(string_compound *Comp1, string_compound *Comp2);
inline void CombineStringCompounds(string_compound *Buffer, string_compound *Comp1, string_compound *Comp2);
inline void ConcatStringCompounds(u32 ConcatCount, string_c *S1, ...);

inline i32 FindFirstOccurrenceOfCharInStringCompound(string_compound *S, u8 Char);
inline i32 FindLastOccurrenceOfCharInStringCompound(string_compound *S, u8 Char);
inline void CutStringCompoundAt(string_compound *Comp, u32 P);
inline b32 CompareStringCompounds(string_compound *S1, string_compound *S2);
inline b32 CompareStringAndCompound(string_compound *Comp, u8 *S);

inline void I32ToString(string_compound *Comp, i32 I);
inline void EatTrailingSpaces(string_c *S);
inline void EatLeadingSpaces(string_c *S);
internal b32 ContainsAB_CaseInsensitive(string_c *String, string_c *Contains);

internal u32 StringLength(u8 *StringToCount);
inline b32 StringCompare(u8 *S1, u8 *S2, u32 CompareStart, u32 CompareEnd);
inline u8 *AdvanceToChar(u8 *Char, u8 Delimiter);
inline u8 *AdvanceAfterChar(u8 *Char, u8 Delimiter);
inline u8 *AdvanceToLineEnd(u8 *Char);
inline u8 *AdvanceToNextLine(u8 *Char);
inline u8 *AdvanceAfterConsecutiveGivenChar(u8 *Character, u8 Given);
inline r32 CharToR32(char C);
inline u32 CharToU32(u8 C);
inline void R32ToString(u8 *Buffer, r32 Value);
inline u32 ProcessNextU32InString(u8 *Character, char Delimiter, u8 &NumberLength);
inline i32 ProcessNextI32InString(u8 *Character, char Delimiter, u8 &NumberLength);
inline r32 ProcessNextR32InString(u8 *Character, char Delimiter, u8 &NumberLength);
inline b32 ProcessNextB32InString(u8 *Character);
inline i32 FirstOccurranceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter);
inline u32 StringLengthUntilChar(u8 *Character, u8 Delimiter);
inline u32 CopyStringUntilChar(u8 *Dest, u8 *Source, u8 Delimiter);
inline void CopyString(u8 *Dest, u8 *Source, u32 Size);
inline void CombineStrings(u8 *Dest, u8 *S1, u8 *S2);
inline void ConvertV3To15Char(v3 P, u8 *Result);
inline u32 ConvertU32FromString(u8 *Character, u32 NumberLength);

// Conversion to wide chars
struct string_wide
{
    wchar_t *S;
    u32 Pos;
    u32 Length;
};
typedef string_wide string_w;

inline string_w NewStringW(memory_bucket_container *Bucket, u32 Size);
inline void WipeStringW(string_w *S);
inline void DeleteStringW(memory_bucket_container *BucketContainer, string_w *S);
inline b32 ConvertString8To16(memory_bucket_container *Bucket, string_c *In, string_w *Out);
inline b32 ConvertString8To16(memory_bucket_container *Bucket, u8 *In, string_w *Out);
inline b32 ConvertString16To8(memory_bucket_container *Bucket, string_w *In, string_c *Out);
inline b32 ConvertString16To8(memory_bucket_container *Bucket, wchar_t *In, string_c *Out);




inline string_c
NewStringCompound(memory_bucket_container *BucketContainer, u32 Size)
{
    Assert(Size >= 0);
    string_c Result;
    Result.S = PushArrayOnBucket(BucketContainer, Size+1, u8);
    Result.S[Size] = 0;
    Result.Pos = 0;
    Result.Length = Size;
    WipeStringCompound(&Result);
    return Result;
}

inline void 
DeleteStringCompound(memory_bucket_container *BucketContainer, string_compound *S)
{
    PopFromTransientBucket(BucketContainer, S->S);
}

inline void
AppendCharToCompound(string_compound *Comp, u8 C)
{
    Assert(Comp->Pos+1 <= Comp->Length);
    Comp->S[Comp->Pos++] = C;
    Comp->S[Comp->Pos] = '\0';
}

inline void
AppendStringToCompound(string_compound *Comp, u8 *String)
{
    while(*String)
    {
        Comp->S[Comp->Pos++]  = *String++;
        Assert(Comp->Pos <= Comp->Length);
    }
    Comp->S[Comp->Pos] = '\0';
}

inline void
CopyStringToCompound(string_compound *Comp, u8 *String, u32 StartPos)
{
    Comp->Pos = StartPos;
    AppendStringToCompound(Comp, String);
    Comp->S[Comp->Pos] = '\0';
}

inline void
CopyStringToCompound(string_compound *Comp, u8 *String, u32 From, u32 To)
{
    Assert(To > From);
    Assert(Comp->Length-Comp->Pos >= To-From);
    
    u8 *S = String+From;
    For(To-From)
    {
        Comp->S[Comp->Pos++] = *S++;
    }
    Comp->S[Comp->Pos] = '\0';
}

inline void 
AppendStringCompoundToCompound(string_compound *Comp1, string_compound *Comp2)
{
    Assert((Comp1->Length-Comp1->Pos) >= Comp2->Pos);
    For(Comp2->Pos)
    {
        Comp1->S[Comp1->Pos++] = Comp2->S[It];
    }
    Comp1->S[Comp1->Pos] = '\0';
}

inline void
PasteStringCompoundIntoCompound(string_compound *C1, u32 C1Start, string_compound *C2, u32 C2Start, u32 PasteLength)
{
    Assert((C1Start+PasteLength) <= C1->Length);
    Assert((C2Start+PasteLength) <= C2->Length);
    
    For(PasteLength)
    {
        C1->S[C1Start+It] = C2->S[C2Start+It];
    }
    if(C1->Pos < C1Start+PasteLength) C1->Pos = C1Start+PasteLength;
}

inline void 
WipeStringCompound(string_compound *Comp)
{
    ClearToGiven(Comp->S, Comp->Length*sizeof(u8), 0);
    Comp->Pos = 0;
}

inline void 
CreateOrWipeStringComp(memory_bucket_container *Bucket, string_c *C, i32 Size)
{
    
    if(C->Length == 0) 
    {
        *C = NewStringCompound(Bucket, Size);
    }
    else if(C->Pos != 0)
    {
        WipeStringCompound(C);
    }
}

inline void 
CombineStringCompounds(string_compound *Comp1, string_compound *Comp2)
{
    Assert(Comp1->Length > Comp1->Pos + Comp2->Pos);
    
    AppendStringCompoundToCompound(Comp1, Comp2);
}

inline void 
CombineStringCompounds(string_compound *Buffer, string_compound *Comp1, string_compound *Comp2)
{
    Assert(Buffer->Length >= Comp1->Length + Comp2->Length);
    CombineStringCompounds(Buffer, Comp1);
    CombineStringCompounds(Buffer, Comp2);
}

inline void
ConcatStringCompounds(u32 ConcatCount, string_c *S1, ...)
{
    va_list Args;
    va_start(Args, S1);
    
    For(ConcatCount-1)
    {
        string_c *NextArg = &va_arg(Args, string_c);
        AppendStringCompoundToCompound(S1, NextArg);
    }
    va_end(Args);
}

inline i32
FindFirstOccurrenceOfCharInStringCompound(string_compound *S, u8 Char)
{
    i32 Result = -1;
    For(S->Pos)
    {
        if(S->S[It] == Char)
        {
            Result = It;
            break;
        }
    }
    return Result;
}

inline i32
FindLastOccurrenceOfCharInStringCompound(string_compound *S, u8 Char)
{
    i32 Result = -1;
    for(i32 It = S->Pos-1; It >= 0; It--)
    {
        if(S->S[It] == Char)
        {
            Result = It;
            break;
        }
    }
    return Result;
}

inline void
CutStringCompoundAt(string_compound *Comp, u32 P)
{
    Comp->S[P] = '\0';
    Comp->Pos = P;
}

inline b32 
CompareStringCompounds(string_compound *S1, string_compound *S2)
{
    b32 Result = true;
    
    if(S1->Pos != S2->Pos)
    {
        Result = false;
    } 
    else
    {
        For(S1->Pos)
        {
            if(S1->S[It] != S2->S[It])
            {
                Result = false;
                break;
            }
        }
    }
    return Result;
}

inline b32 
CompareStringAndCompound(string_compound *Comp, u8 *S)
{
    b32 Result = true;
    
    For(Comp->Pos)
    {
        if(Comp->S[It] != *S || *S == '\0')
        {
            Result = false;
            break;
        }
        S++;
    }
    if(*S != 0) 
    {
        Result = false;
    }
    
    return Result;
}

inline b32 
CompareStringAndCompound(string_compound *Comp, u8 *S, u32 StringCompareLength)
{
    b32 Result = true;
    
    for(u32 It = 0;
        It < Comp->Pos &&
        It < StringCompareLength;
        It++)
    {
        if(Comp->S[It] != *S)
        {
            Result = false;
            break;
        }
        S++;
    }
    
    return Result;
}

inline void 
EatTrailingSpaces(string_c *S)
{
    if(S->Pos == 0) return;
    
    for(u32 It = S->Pos-1; It > 0; It--)
    {
        if(S->S[It] == ' ')
        {
            S->S[It] = '\0';
            S->Pos--;
        }
        else break;
    }
}

inline void 
EatLeadingSpaces(string_c *S)
{
    u32 Eat = 0;
    
    For(S->Pos)
    {
        if(S->S[It] == ' ') Eat++;
        else break;
    }
    
    if(Eat > 0)
    {
        For(S->Pos-Eat)
        {
            S->S[It] = S->S[Eat+It];
        }
        S->S[S->Pos-Eat] = 0;
        S->Pos -= Eat;
    }
}

inline u32
StringLength(u8 *StringToCount)
{
    u32 Count = 0;
    while(*StringToCount++)
    {
        ++Count;
    }
    return(Count);
}

inline b32
StringCompare(u8 *S1, u8 *S2, u32 CompareStart, u32 CompareEnd)
{
    b32 Result = true;
    u32 S2Count = 0;
    for(u32 Count = CompareStart; 
        Count < CompareEnd; 
        ++Count, ++S2Count)
    {
        if(S1[Count] != S2[S2Count])
        {
            Result = false;
            break;
        }
    }
    return Result;
}

inline u8 *
AdvanceToChar(u8 *Char, u8 Delimiter)
{
    while(*Char != Delimiter)
    {
        Char++;
    }
    return Char;
}

inline u8 *
AdvanceAfterChar(u8 *Char, u8 Delimiter)
{
    Char = AdvanceToChar(Char++, Delimiter);
    return Char;
}

inline u8 *
AdvanceToLineEnd(u8 *Char)
{
    while(*Char != '\n' && *Char != '\0')
    {
        Char++;
    }
    return Char;
}

inline u8 *
AdvanceToNextLine(u8 *Char)
{
    Char = AdvanceToLineEnd(Char++);
    return Char;
}

inline u8 *
AdvanceAfterConsecutiveGivenChar(u8 *Character, u8 Given)
{
    while(*Character == Given) 
    {
        Character++;
    }
    return Character;
}

inline r32
CharToR32(char C)
{
    r32 Result = 0.0f;
    Result = (r32)C - 48;
    return Result;
}

inline u32
CharToU32(u8 C)
{
    u32 Result = 0;
    Result = (u32)C - 48;
    return Result;
}

inline void
R32ToString(u8 *Buffer, r32 Value)
{
    // TODO:: Implement this myself!
    snprintf((char *)Buffer, sizeof(Buffer), "%f", Value);
}

inline u32
ConvertU32FromString(u8 *Character, u32 NumberLength)
{
    i32 Result = 0;
    For(NumberLength)
    {
        Result *= 10;
        Result += CharToU32(*Character++);
    }
    return Result;
}

inline u32
ProcessNextU32InString(u8 *Character, char Delimiter, u8 &NumberLength)
{
    i32 Result = 0;
    NumberLength  = 0;
    while(*Character != Delimiter && 
          *Character != '\n' && 
          *Character != '\r' && 
          *Character != '\0')
    {
        Result *= 10;
        Result += CharToU32(*Character++);
        NumberLength++;
    }
    return Result;
}

inline i32
ProcessNextI32InString(u8 *Character, char Delimiter, u8 &NumberLength)
{
    i32 Sign = (*Character == '-') ? -1 : 1;
    u8 SignRes  = (Sign == 1) ? 0 : 1;
    Character += SignRes;
    i32 Result = ProcessNextU32InString(Character, Delimiter, NumberLength);
    Result *= Sign;
    NumberLength += SignRes;
    return Result;
}

inline b32
ProcessNextB32InString(u8 *Character)
{
    b32 Result = false;
    if(*Character == '1') Result = true;
    return Result;
}

inline r32
ProcessNextR32InString(u8 *Character, char Delimiter, u8 &NumberLength)
{
    r32 Result   = 0;
    i32 Sign      = (*Character == '-') ? -1 : 1;
    u8 SignRes  = (Sign == 1) ? 0 : 1;
    Character      += SignRes;
    NumberLength    = SignRes;
    r32 DecimalPoint = 0.1f;
    b32 HasNoDecimal = false;
    while(*Character != '.')
    {
        if(*Character == Delimiter || *Character == '\n' || *Character == '\r' || *Character == '\0')
        {
            HasNoDecimal = true;
            break;
        }
        Result *= 10;
        Result += CharToR32(*Character);
        Character++;
        NumberLength++;
    }
    Character++;
    NumberLength++;
    if(!HasNoDecimal)
    {
        u32 HasE10Multiplier = false;
        while(*Character != Delimiter && 
              *Character != '\n' && 
              *Character != '\r' && 
              *Character != '\0')
        {
            if(*Character == 'e' || *Character == 'E')
            {
                HasE10Multiplier = true;
                break;
            }
            else
            {
                Result += CharToR32(*Character)*DecimalPoint;
                DecimalPoint *= 0.1f;
            }
            Character++;
            NumberLength++;
        }
        if(*Character != '\n' && *Character != '\r')
        {
            NumberLength++;
        }
        Character++;
        
        if(HasE10Multiplier)
        {
            u8 NumberLengthB = 0;
            i32 E10Muliplier = ProcessNextI32InString(Character, Delimiter, NumberLengthB);
            Character += ++NumberLengthB;
            NumberLength += NumberLengthB;
            Result = Result*Pow(10, E10Muliplier);
        }
    }
    Result *= Sign;
    return Result;
}

inline i32
FirstOccurrenceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter)
{
    i32 Result = -1;
    i32 Counter = 0;
    while(*String != Delimiter)
    {
        if(*String++ == CharToFind)
        {
            Result = Counter;
            break;
        }
        Counter++;
    }
    return Result;
}

inline i32
LastOccurrenceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter)
{
    i32 Result = -1;
    i32 Counter = 0;
    while(*String != Delimiter)
    {
        if(*String++ == CharToFind)
        {
            Result = Counter;
        }
        Counter++;
    }
    return Result;
}

inline u32
StringLengthUntilChar(u8 *Character, u8 Delimiter)
{
    u32 Result = 0;
    while(*Character != Delimiter && *Character++ != '\n')
    {
        Result++;
    }
    return Result;
}

inline u32
CopyStringUntilChar(u8 *Dest, u8 *Source, u8 Delimiter)
{
    b32 Result = 0;
    
    while(*Source != '\n' &&*Source != '\0')
    {
        *Dest++ = *Source++;
        Result++;
    }
    *Dest = '\0';
    return Result;
}

inline void
CopyString(u8 *Dest, u8 *Source, u32 Length)
{
    memcpy_s(Dest, Length,  Source, Length);
    *(Dest + Length) = '\0';
}

inline void 
CombineStrings(u8 *Dest, u8 *S1, u8 *S2)
{
    u32 S1Length = StringLength(S1);
    CopyString(Dest, S1, S1Length);
    Dest += S1Length;
    CopyString(Dest, S2, StringLength(S2));
}

inline void
ConvertV3To15Char(v3 P, u8 *Result)
{
    u8 Count = 0;
    if(P.x < 0)
    {
        *(Result + Count++) = '-';
        P.x = (r32)fabs(P.x);
    }
    else
    {
        *(Result + Count++) = '0';
    }
    *(Result + Count++) = (u8)(P.x) + 48;
    P.x = (P.x - (i32)P.x)*10;
    *(Result + Count++) = (u8)(P.x) + 48;
    P.x = (P.x - (i32)P.x)*10;
    *(Result + Count++) = (u8)(P.x) + 48;
    P.x = (P.x - (i32)P.x)*10;
    *(Result + Count++) = (u8)(P.x) + 48;
    if(P.y < 0)
    {
        *(Result + Count++) = '-';
        P.y = (r32)fabs(P.y);
    }
    else
    {
        *(Result + Count++) = '0';
    }
    *(Result + Count++) = (u8)(P.y) + 48;
    P.y = (P.y - (i32)P.y)*10;
    *(Result + Count++) = (u8)(P.y) + 48;
    P.y = (P.y - (i32)P.y)*10;
    *(Result + Count++) = (u8)(P.y) + 48;
    P.y = (P.y - (i32)P.y)*10;
    *(Result + Count++) = (u8)(P.y) + 48;
    if(P.z < 0)
    {
        *(Result + Count++) = '-';
        P.z = (r32)fabs(P.z);
    }
    else
    {
        *(Result + Count++) = '0';
    }
    *(Result + Count++) = (u8)(P.z) + 48;
    P.z = (P.z - (i32)P.z)*10;
    *(Result + Count++) = (u8)(P.z) + 48;
    P.z = (P.z - (i32)P.z)*10;
    *(Result + Count++) = (u8)(P.z) + 48;
    P.z = (P.z - (i32)P.z)*10;
    *(Result + Count++) = (u8)(P.z) + 48;
}

inline void
I32ToString(string_compound *Comp, i32 I)
{
    if(I < 0) 
    {
        Comp->S[Comp->Pos++] = '-';
        I = Abs(I);
    }
    
    i32 Size = 10;
    while(I >= Size) Size *= 10;
    Size /= 10;
    
    while(Size)
    {
        i32 Tmp = I/Size;
        Comp->S[Comp->Pos++] = (u8)Tmp+48;
        I = I - (Tmp*Size);
        Size /= 10;
        Assert(Comp->Pos <= Comp->Length);
    }
    Comp->S[Comp->Pos] = 0;
}

inline void
WipeStringW(string_w *S)
{
    ClearToGiven(S->S, S->Length*sizeof(wchar_t), 0);
    S->Pos = 0;
}

inline string_w
NewStringW(memory_bucket_container *Bucket, u32 Size)
{
    string_w Result;
    Result.S = PushArrayOnBucket(Bucket, Size+1, wchar_t);
    Result.S[Size] = 0;
    Result.Pos = 0;
    Result.Length = Size;
    WipeStringW(&Result);
    return Result;
}

inline void 
DeleteStringW(memory_bucket_container *BucketContainer, string_w *S)
{
    PopFromTransientBucket(BucketContainer, S->S);
}

inline b32
ConvertString8To16(memory_bucket_container *Bucket, string_c *In, string_w *Out)
{
    b32 Result = false;
    i32 Size = MultiByteToWideChar(CP_UTF8, 0, (char *)In->S, -1, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringW(Bucket, Size);
        // TODO:: Write your own code for this? 
        MultiByteToWideChar(CP_UTF8, 0, (char *)In->S, -1, Out->S, Size);
        Out->Pos = Size;
        Result = true;
    }
    return Result;
}

inline b32
ConvertString8To16(memory_bucket_container *Bucket, u8 *In, string_w *Out)
{
    b32 Result = false;
    i32 Size = MultiByteToWideChar(CP_UTF8, 0, (char *)In, -1, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringW(Bucket, Size);
        // TODO:: Write your own code for this? 
        MultiByteToWideChar(CP_UTF8, 0, (char *)In, -1, Out->S, Size);
        Out->Pos = Size;
        Result = true;
    }
    return Result;
}

inline b32
ConvertString16To8(memory_bucket_container *Bucket, string_w *In, string_c *Out)
{
    b32 Result = false;
    
    i32 Size = WideCharToMultiByte(CP_UTF8, 0, In->S, -1, 0, 0, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringCompound(Bucket, Size);
        // TODO:: Write your own code for this? 
        WideCharToMultiByte(CP_UTF8, 0, In->S, -1, (char *)Out->S, Size, 0, 0);
        Out->Pos = Size-1;
        Result = true;
    }
    
    return Result;
}

inline b32
ConvertString16To8(memory_bucket_container *Bucket, wchar_t *In, string_c *Out)
{
    b32 Result = false;
    
    i32 Size = WideCharToMultiByte(CP_UTF8, 0, In, -1, 0, 0, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringCompound(Bucket, Size);
        // TODO:: Write your own code for this? 
        WideCharToMultiByte(CP_UTF8, 0, In, -1, (char *)Out->S, Size, 0, 0);
        Out->Pos = Size-1;
        Result = true;
    }
    
    return Result;
}

inline b32
IsStringCompANumber(string_c *SC)
{
    b32 Result = true;
    
    For(SC->Pos)
    {
        if(SC->S[It] < 0x30 ||
           SC->S[It] > 0x39)
        {
            Result = false;
            break;
        }
    }
    
    return Result;
}

inline u8
CharToLowerCase(u8 Char)
{
    u8 Result = Char;
    
    if(Char >= 65 && Char <=90 ||
       Char == 196 || Char == 214 || Char == 220)
    {
        Result += 32;
    }
    
    return Result;
}

internal b32 
ContainsAB_CaseInsensitive(string_c *String, string_c *Contains)
{
    b32 Result = false;
    
    u32 Count1 = 0;
    u32 Count2 = 0;
    while(true)
    {
        if(Count1 == String->Pos) break;
        if(Count2 == Contains->Pos) 
        {
            Result = true;
            break;
        }
        
        if(CharToLowerCase(String->S[Count1+Count2]) == CharToLowerCase(Contains->S[Count2]))
        {
            Count2++;
        }
        else 
        {
            Count1++;
            Count2 = 0;
        }
    }
    
    return Result;
}
















