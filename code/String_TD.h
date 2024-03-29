#pragma once
#include "Math_TD.h"
#include "Allocator_TD.h"
#include "StandardUtilities_TD.h"

#ifndef CombineDefine
#define CombineDefine1(X, Y) X##Y
#define CombineDefine(X, Y) CombineDefine1(X, Y)
#endif

struct string_compound
{
    u8 *S;
    u32 Pos;
    u32 Length;
};
typedef string_compound string_c;

#define NewStaticStringCompound(String) {(u8 *)String, StringLength((u8 *)String), StringLength((u8 *)String)}
inline string_c NewStringCompound(arena_allocator *Arena, u32 Size);
inline void DeleteStringCompound(arena_allocator *Arena, string_compound *S);
#define ResetStringCompound(SC) {(SC).Pos = 0; (SC).S[0] = '\0';}

// Creates an empty string_c on the stack. No allocation needed.
#define NewEmptyLocalString(Name, Count)  \
u8 CombineDefine(StringStorage, __LINE__)[Count]; \
string_c Name = {CombineDefine(StringStorage, __LINE__), 0, Count}

// Creates a string_c on the stack with the given string. No allocation needed.
#define NewLocalString(Name, Count, String)  NewEmptyLocalString(Name, Count);\
AppendStringToCompound(&Name, (u8 *)String)

inline void AppendCharToCompound(string_compound *Comp, u8 C);
inline void AppendStringToCompound(string_compound *Comp, u8 *String); 
inline void PrependString(string_c *PrepandTo, string_c String); 
inline void CopyStringToCompound(string_compound *Comp, u8 *String, u32 StartPos);
inline void CopyStringToCompound(string_compound *Comp, u8 *String, u32 From, u32 To);
inline void CopyStringToCompound(string_compound *Comp, u8 *String, u8 Delimiter, b32 stopIfFull = false);
inline void CopyStringToCompound(string_compound *Comp, u8 *String, u8 *Delimiters, u32 DeliCount, b32 stopIfFull = false);
inline void AppendStringCompoundToCompound(string_compound *Comp1, string_compound *Comp2);
inline void CopyIntoCompound(string_c *Into, string_c *Copy);

inline void Append(string_compound *Comp, u8 C);      // Equivalent to AppendCharToCompound
inline void Append(string_c *Comp1, u8 *String);      // Equivalent to AppendStringToCompound
inline void Append(string_c *Comp1, string_c *Comp2); // Equivalent to AppendStringCompoundToCompound

inline void WipeStringCompound(string_compound *Comp);
inline void CreateOrWipeStringComp(arena_allocator *Arena, string_c *C);

inline void CombineStringCompounds(string_compound *Comp1, string_compound *Comp2);
inline void CombineStringCompounds(string_compound *Buffer, string_compound *Comp1, string_compound *Comp2);
inline void ConcatStringCompounds(u32 ConcatCount, string_c *S1, ...);

inline i32 Find(string_c FindIn, u8 FindWhat);
inline i32 Find(string_c FindIn, string_c FindWhat);
inline i32 Find(u8 *FindIn, string_c FindWhat);
inline i32 FindFirstOccurrenceOfCharInStringCompound(string_compound *S, u8 Char);
inline i32 FindLastOccurrenceOfCharInStringCompound(string_compound *S, u8 Char);
inline void CutStringCompoundAt(string_compound *Comp, u32 P);
inline b32 CompareStringCompounds(string_compound *S1, string_compound *S2);
inline b32 CompareStringAndCompound(string_compound *Comp, u8 *S);
inline b32 CompareStringAndCompound(string_compound *Comp, u8 *S, u32 StringCompareLength);

inline b32 StringEndsWith(string_c S, string_c StringEnd);
inline b32 StringStartsWith(string_c S, string_c StringEnd);

inline void EatTrailingSpaces(string_c *S);
inline void EatLeadingSpaces(string_c *S);
inline void EatTrailingSpaces(u8 *S);
inline void EatLeadingSpaces(u8 *S);
internal b32 ContainsAB_CaseInsensitive(string_c *String, string_c *Contains);

inline u32 StringLength(u8 *StringToCount);
inline b32 StringCompare(u8 *S1, u8 *S2, u32 CompareStart, u32 CompareEnd);
inline u8 *AdvanceToChar(u8 *Char, u8 Delimiter);
inline u8 *AdvanceAfterChar(u8 *Char, u8 Delimiter);
inline u8 *AdvanceToLineEnd(u8 *Char);
inline u8 *AdvanceToNextLine(u8 *Char);
inline u8 *AdvanceAfterConsecutiveGivenChar(u8 *Character, u8 Given);
inline i32 FirstOccurranceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter);
inline i32 LastOccurrenceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter);
inline u32 StringLengthUntilChar(u8 *Character, u8 Delimiter);
inline u32 CopyStringUntilChar(u8 *Dest, u8 *Source, u8 Delimiter);
inline void CopyString(u8 *Dest, u8 *Source, u32 Size);
inline void CombineStrings(u8 *Dest, u8 *S1, u8 *S2);

// Conversion procedures
inline void I32ToString(string_c *Comp, i32 I);
inline void U32ToString(string_c *Comp, u32 I);
inline void R32ToString(string_c *S, r32 Value);
inline void R32ToString(u8 *Buffer, r32 Value);
inline void V3ToString(string_c *S, u8 Delimiter, v3 Vec);
inline r32 CharToR32(char C);
inline u32 CharToU32(u8 C);
inline u32 ProcessNextU32InString(u8 *Character, u8 Delimiter, u8 &NumberLength);
inline i32 ProcessNextI32InString(u8 *Character, u8 Delimiter, u8 &NumberLength);
inline r32 ProcessNextR32InString(u8 *Character, u8 Delimiter, u8 &NumberLength);
inline u32 ProcessNextU32InString(u8 *Character, u8 *Delimiters, u32 DeliCount, u8 &NumberLength);
inline i32 ProcessNextI32InString(u8 *Character, u8 *Delimiters, u32 DeliCount, u8 &NumberLength);
inline r32 ProcessNextR32InString(u8 *Character, u8 *Delimiters, u32 DeliCount, u8 &NumberLength);
inline b32 ProcessNextB32InString(u8 *Character);
inline void ConvertV3To15Char(v3 P, u8 *Result);
inline u32 ConvertU32FromString(u8 *Character, u32 NumberLength);
inline u32 ConvertU32FromString(string_c *String);

// Local string stuff
#define R32ToLocalString(Name, Count, Value) NewEmptyLocalString(Name, Count) \
Name.Pos = sprintf_s((char *)Name.S, Count, "%f", Value);

#define I32ToLocalString(Name, Count, Value) NewEmptyLocalString(Name, Count) \
I32ToString(&(Name), Value);

// Conversion to wide chars
struct string_wide
{
    wchar_t *S;
    u32 Pos;
    u32 Length;
};
typedef string_wide string_w;

inline string_w NewStringW(arena_allocator *Arena, u32 Size);
inline void WipeStringW(string_w *S);
inline void DeleteStringW(arena_allocator *Arena, string_w *S);
inline u32 StringLength(wchar_t *StringToCount);
inline b32 ConvertString8To16(arena_allocator *Arena, string_c *In, string_w *Out);
inline b32 ConvertString8To16(arena_allocator *Arena, u8 *In, string_w *Out);
inline b32 ConvertString16To8(arena_allocator *Arena, string_w *In, string_c *Out);
inline b32 ConvertString16To8(arena_allocator *Arena, wchar_t *In, string_c *Out);
inline b32 ConvertString16To8(wchar_t *In, string_c *Out); // Only use with Out parameter already having a appropriate size!
inline b32 ConvertChar16To8(wchar_t In, u8 *Out, i32 *OutSize);


internal void BinaryToHexString(u64 Binary, string_c *Hex);
internal void DecimalToHexString(u32 Decimal, string_c *Hex);

inline string_c
NewStringCompound(arena_allocator *Arena, u32 Size)
{
    Assert(Size >= 0);
    string_c Result;
    Result.S = AllocateArray(Arena, Size+1, u8);
    Result.S[Size] = 0;
    Result.Pos = 0;
    Result.Length = Size;
    WipeStringCompound(&Result);
    return Result;
}

inline void 
DeleteStringCompound(arena_allocator *Arena, string_compound *S)
{
    FreeMemory(Arena, S->S);
}

inline void
AppendCharToCompound(string_compound *Comp, u8 C)
{
    Assert(Comp->Pos+1 <= Comp->Length);
    Comp->S[Comp->Pos++] = C;
    Comp->S[Comp->Pos] = '\0';
}

inline void
Append(string_compound *Comp, u8 C)
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
Append(string_compound *Comp, u8 *String)
{
    while(*String)
    {
        Comp->S[Comp->Pos++]  = *String++;
        Assert(Comp->Pos <= Comp->Length);
    }
    Comp->S[Comp->Pos] = '\0';
}

inline void 
PrependString(string_c *PrependTo, string_c String)
{
    Assert(PrependTo->Length >= PrependTo->Pos+String.Pos);
    for(u32 It = PrependTo->Pos-1; It >= 0; --It)
    {
        u8 Tmp = PrependTo->S[It];
        PrependTo->S[String.Pos+It] = Tmp;
    }
    For(String.Pos) PrependTo->S[It] = String.S[It];
    PrependTo->Pos += String.Pos;
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
    Assert(To >= From);
    Assert(Comp->Length-Comp->Pos >= To-From);
    
    u8 *S = String+From;
    For(To-From)
    {
        Comp->S[Comp->Pos++] = *S++;
    }
    Comp->S[Comp->Pos] = '\0';
}

inline void 
CopyStringToCompound(string_compound *Comp, u8 *String, u8 Delimiter, b32 stopIfFull)
{
    while(*String != Delimiter && 
          *String != 0)
    {
        Assert(*String);
        Comp->S[Comp->Pos++] = *String;
        String++;
        if(stopIfFull && Comp->Pos == Comp->Length) break;
        Assert(Comp->Pos <= Comp->Length);
    }
}

inline void 
CopyStringToCompound(string_compound *Comp, u8 *String, u8 *Delimiters, u32 DeliCount, b32 stopIfFull)
{
    while(*String != 0)
    {
        b32 Break = false;
        For(DeliCount)
        {
            if(*String == Delimiters[It]) Break = true;
            if(Break) break;
        }
        if(Break) break;
        
        Assert(*String);
        Comp->S[Comp->Pos++] = *String;
        String++;
        if(stopIfFull && Comp->Pos == Comp->Length) break;
        Assert(Comp->Pos <= Comp->Length);
    }
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
Append(string_compound *Comp1, string_compound *Comp2)
{
    Assert((Comp1->Length-Comp1->Pos) >= Comp2->Pos);
    For(Comp2->Pos)
    {
        Comp1->S[Comp1->Pos++] = Comp2->S[It];
    }
    Comp1->S[Comp1->Pos] = '\0';
}

inline void 
CopyIntoCompound(string_c *Into, string_c *Copy)
{
    Assert(Copy->Pos <= Into->Length);
    MemoryCopy(Into->S, Copy->S, Copy->Pos);
    Into->Pos = Copy->Pos;
    Into->S[Into->Pos] = '\0';
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
    ClearArray(Comp->S, Comp->Length, u8);
    Comp->Pos = 0;
}

inline void 
CreateOrWipeStringComp(arena_allocator *Arena, string_c *C, i32 Size)
{
    
    if(C->Length == 0) 
    {
        *C = NewStringCompound(Arena, Size);
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
Find(u8 *FindIn, string_c FindWhat)
{
    i32 Result = -1;
    
    u32 It = 0;
    i32 FoundLength = 0;
    while(FindIn != 0)
    {
        if(FindIn[It] == FindWhat.S[FoundLength])
        {
            ++FoundLength;
            if(FindWhat.Pos == (u32)FoundLength) 
            {
                Result = It+1;
                break;
            }
        }
        else 
        {
            It -= Max(0, FoundLength-1);
            FoundLength = 0;
        }
        ++It;
    }
    
    return Result;
}

inline i32 
Find(string_c FindIn, string_c FindWhat)
{
    i32 Result = -1;
    
    i32 FoundLength = 0;
    For(FindIn.Pos)
    {
        if(FindIn.S[It] == FindWhat.S[FoundLength])
        {
            ++FoundLength;
            if(FindWhat.Pos == (u32)FoundLength) 
            {
                Result = It+1;
                break;
            }
        }
        else 
        {
            It -= Max(0, FoundLength-1);
            FoundLength = 0;
        }
    }
    
    return Result;
}

inline i32 
Find(string_c FindIn, u8 FindWhat)
{
    i32 Result = -1;
    
    For(FindIn.Pos)
    {
        if(FindIn.S[It] == FindWhat)
        {
            Result = It;
            break;
        }
    }
    
    return Result;
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

inline b32 
StringEndsWith(string_c S, string_c StringEnd)
{
    if(S.Pos < StringEnd.Pos) return false;
    b32 Result = true;
    
    For(StringEnd.Pos)
    {
        if(S.S[S.Pos-StringEnd.Pos+It] != StringEnd.S[It])
        {
            Result = false;
            break;
        }
    }
    return Result;
}

inline b32 
StringStartsWith(string_c S, string_c StringEnd)
{
    if(S.Pos < StringEnd.Pos) return false;
    b32 Result = true;
    
    For(StringEnd.Pos)
    {
        if(S.S[It] != StringEnd.S[It])
        {
            Result = false;
            break;
        }
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

inline void 
EatLeadingSpaces(u8 **S)
{
    while(*S[0] == ' ') (S[0])++;
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
R32ToString(string_c *S, r32 Value)
{
    S->Pos += sprintf_s((char *)(S->S+S->Pos), S->Length-S->Pos, "%f", Value);
}

inline void
R32ToString(u8 *Buffer, r32 Value)
{
    sprintf_s((char *)Buffer, sizeof(Buffer), "%f", Value);
}

inline void 
V3ToString(string_c *S, u8 Delimiter, v3 Vec)
{
    R32ToString(S, Vec.x);
    AppendCharToCompound(S, Delimiter);
    R32ToString(S, Vec.y);
    AppendCharToCompound(S, Delimiter);
    R32ToString(S, Vec.z);
}

inline void 
V3iToString(string_c *S, u8 Delimiter, v3i Vec)
{
    I32ToString(S, Vec.x);
    AppendCharToCompound(S, Delimiter);
    I32ToString(S, Vec.y);
    AppendCharToCompound(S, Delimiter);
    I32ToString(S, Vec.z);
}

inline u32
ConvertU32FromString(u8 *Character, u32 NumberLength)
{
    u32 Result = 0;
    For(NumberLength)
    {
        Result *= 10;
        Result += CharToU32(*Character++);
    }
    return Result;
}

inline u32 
ConvertU32FromString(string_c *String)
{
    u32 Result = 0;
    For(String->Pos)
    {
        Result *= 10;
        Result += CharToU32(String->S[It]);
    }
    return Result;
}

inline u32
ProcessNextU32InString(u8 *Character, u8 Delimiter, u8 &NumberLength)
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
ProcessNextI32InString(u8 *Character, u8 Delimiter, u8 &NumberLength)
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
ProcessNextR32InString(u8 *Character, u8 Delimiter, u8 &NumberLength)
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
            Result = Result*Pow(10.0f, (r32)E10Muliplier);
        }
    }
    Result *= Sign;
    return Result;
}


inline u32 ProcessNextU32InString(u8 *Character, u8 *Delimiters, u32 DeliCount, u8 &NumberLength)
{
    i32 Result = 0;
    NumberLength  = 0;
    while(*Character != '\n' && 
          *Character != '\r' && 
          *Character != '\0')
    {
        b32 Break = false;
        For(DeliCount) 
        {
            Break = *Character == Delimiters[It];
            if(Break) break;
        }
        if(Break) break;
        
        Result *= 10;
        Result += CharToU32(*Character++);
        NumberLength++;
    }
    return Result;
}

inline i32 ProcessNextI32InString(u8 *Character, u8 *Delimiters, u32 DeliCount, u8 &NumberLength)
{
    i32 Sign = (*Character == '-') ? -1 : 1;
    u8 SignRes  = (Sign == 1) ? 0 : 1;
    Character += SignRes;
    i32 Result = ProcessNextU32InString(Character, Delimiters, DeliCount, NumberLength);
    Result *= Sign;
    NumberLength += SignRes;
    return Result;
}

inline r32 ProcessNextR32InString(u8 *Character, u8 *Delimiters, u32 DeliCount, u8 &NumberLength)
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
        b32 Break = false;
        For(DeliCount) 
        {
            Break = *Character == Delimiters[It];
            if(Break) break;
        }
        
        if(Break || *Character == '\n' || *Character == '\r' || *Character == '\0')
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
        while(*Character != '\n' && 
              *Character != '\r' && 
              *Character != '\0')
        {
            b32 Break = false;
            For(DeliCount) 
            {
                Break = *Character == Delimiters[It];
                if(Break) break;
            }
            if(Break) break;
            
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
            i32 E10Muliplier = ProcessNextI32InString(Character, Delimiters, DeliCount, NumberLengthB);
            Character += ++NumberLengthB;
            NumberLength += NumberLengthB;
            Result = Result*Pow(10.0f, (r32)E10Muliplier);
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
U32ToString(string_compound *Comp, u32 I)
{
    // TODO:: Fix this, this is bad... but works.
    u64 I64 = I;
    u64 Size = 10;
    while(I64 >= Size) Size *= 10;
    Size /= 10;
    
    while(Size)
    {
        u64 Tmp = (I64/Size);
        Comp->S[Comp->Pos++] = (u8)Tmp+48;
        I64 = I64 - (Tmp*Size);
        Size /= 10;
        Assert(Comp->Pos <= Comp->Length);
    }
    Comp->S[Comp->Pos] = 0;
}

inline void
WipeStringW(string_w *S)
{
    ClearArray(S->S, S->Length, wchar_t);
    S->Pos = 0;
}

inline string_w
NewStringW(arena_allocator *Arena, u32 Size)
{
    string_w Result;
    Result.S = AllocateArray(Arena, Size+1, wchar_t);
    Result.S[Size] = 0;
    Result.Pos = 0;
    Result.Length = Size;
    WipeStringW(&Result);
    return Result;
}

inline void 
DeleteStringW(arena_allocator *Arena, string_w *S)
{
    FreeMemory(Arena, S->S);
}


inline u32
StringLength(wchar_t *S)
{
    u32 Count = 0;
    while(*S++) ++Count;
    return Count;
}

inline b32
ConvertString8To16(arena_allocator *Arena, string_c *In, string_w *Out)
{
    b32 Result = false;
    i32 Size = MultiByteToWideChar(CP_UTF8, 0, (char *)In->S, -1, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringW(Arena, Size);
        // TODO:: Write your own code for this? 
        MultiByteToWideChar(CP_UTF8, 0, (char *)In->S, -1, Out->S, Size);
        Out->Pos = Size;
        Result = true;
    }
    return Result;
}

inline b32
ConvertString8To16(arena_allocator *Arena, u8 *In, string_w *Out)
{
    b32 Result = false;
    i32 Size = MultiByteToWideChar(CP_UTF8, 0, (char *)In, -1, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringW(Arena, Size);
        // TODO:: Write your own code for this? 
        MultiByteToWideChar(CP_UTF8, 0, (char *)In, -1, Out->S, Size);
        Out->Pos = Size;
        Result = true;
    }
    return Result;
}

inline b32
ConvertString16To8(arena_allocator *Arena, string_w *In, string_c *Out)
{
    b32 Result = false;
    
    i32 Size = WideCharToMultiByte(CP_UTF8, 0, In->S, -1, 0, 0, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringCompound(Arena, Size);
        // TODO:: Write your own code for this? 
        WideCharToMultiByte(CP_UTF8, 0, In->S, -1, (char *)Out->S, Size, 0, 0);
        Out->Pos = Size-1;
        Result = true;
    }
    
    return Result;
}

inline b32
ConvertString16To8(arena_allocator *Arena, wchar_t *In, string_c *Out)
{
    b32 Result = false;
    
    i32 Size = WideCharToMultiByte(CP_UTF8, 0, In, -1, 0, 0, 0, 0);
    if(Size > 0)
    {
        *Out = NewStringCompound(Arena, Size);
        // TODO:: Write your own code for this? 
        WideCharToMultiByte(CP_UTF8, 0, In, -1, (char *)Out->S, Size, 0, 0);
        Out->Pos = Size-1;
        Result = true;
    }
    
    return Result;
}

inline b32 // Only use with Out parameter already having a appropriate size!
ConvertString16To8(wchar_t *In, string_c *Out)
{
    b32 Result = false;
    
    i32 Size = WideCharToMultiByte(CP_UTF8, 0, In, -1, 0, 0, 0, 0);
    if(Size > 0 && Size <= (i32)Out->Length)
    {
        // TODO:: Write your own code for this? 
        WideCharToMultiByte(CP_UTF8, 0, In, -1, (char *)Out->S, Size, 0, 0);
        Out->Pos = Size-1;
        Result = true;
    }
    
    return Result;
}

inline b32
ConvertChar16To8(wchar_t In, u8 *Out, i32 *OutSize)
{
    b32 Result = false;
    Assert(Out);
    Assert(OutSize);
    
    wchar_t In2[2] = {In, 0};
    i32 Size = WideCharToMultiByte(CP_UTF8, 0, In2, -1, 0, 0, 0, 0);
    if(Size > 0 && Size <= *OutSize)
    {
        // TODO:: Write your own code for this? 
        WideCharToMultiByte(CP_UTF8, 0, In2, -1, (char *)Out, Size, 0, 0);
        *OutSize = Size-1;
        Result = true;
    }
    
    return Result;
}

inline void
RemoveLastUTF8Char(string_c *S)
{
    u32 Count = 0;
    for(u32 It = S->Pos-1; It >= 0; --It)
    {
        // I Mask everything except the first two bits
        // and if the result has the first bit set and 
        // not the second, then I know it has to be a 
        // utf-8 continuation byte.
        if((S->S[It] & 0b11000000) == 0b10000000) ++Count;
        else 
        {
            ++Count;
            break;
        }
    }
    S->Pos -= Count;
    S->S[S->Pos] = 0;
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

internal void
BinaryToHexString(u64 Binary, string_c *Hex)
{
    struct hex_bin { u32 Bin; u8 Hex; };
    hex_bin HexBin[] = {
        {0000, '0'}, {0001, '1'}, {0010, '2'}, {0011, '3'}, {0100, '4'}, {0101, '5'}, {0110, '6'}, {0111, '7'}, 
        {1000, '8'}, {1001, '9'}, {1010, 'A'}, {1011, 'B'}, {1100, 'C'}, {1101, 'D'}, {1110, 'E'}, {1111, 'F'}
    };
    i32 Mask = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
    i32 Step = 10000;
    
    while(Binary > 0)
    {
        u32 Next = Binary & Mask;
        Binary /= Step;
        For(ArrayCount(HexBin))
        {
            if(HexBin[It].Bin != Next) continue;
            AppendCharToCompound(Hex, HexBin[It].Hex);
            break;
        }
    }
}

internal void
DecimalToHexString(u32 Decimal, string_c *Hex)
{
    u8 HexBin[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };
    i32 Mask = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;
    u8 Cache[8];
    u8 Count = 0;
    
    while(Decimal > 0)
    {
        u32 Next = Decimal & Mask;
        Decimal = Decimal >> 4;
        Cache[Count++] = HexBin[Next];
    }
    for(i32 It = Count-1; It >= 0; --It)
        AppendCharToCompound(Hex, Cache[It]);
}
















