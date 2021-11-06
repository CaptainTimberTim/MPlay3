/* date = March 10th 2021 7:07 pm */
#ifndef _DEFINITIONS__T_D_H
#define _DEFINITIONS__T_D_H
#include <stdint.h>


// NOTE:: internal is for procedures, to let the compiler know that it is only for this compilation unit
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
#define For(until, ...) for(u32 (__VA_ARGS__##It) = 0; (__VA_ARGS__##It) < (until); ++(__VA_ARGS__##It))

// NOTE:: Counts the size of a fixed array.
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// NOTE:: Meta-Macro, converts a macro back to a 'one-line' statement, which requires a semicolon at the end.
#define ToStatement(E) do{E}while(0)

#ifndef _Combine // Combine both params to a single name
#define __CD2(X, Y) X##Y
#define _Combine(X, Y) __CD2(X, Y)
#endif

// NOTE:: General macro for removing an entry/item from an arbitrary array. Moving all following items one up.
#define RemoveItem(Array, ArrayCount, RemovePos, ArrayType) \
ToStatement( \
u32 MoveSize = (ArrayCount-RemovePos)*sizeof(ArrayType); \
u8 *Goal = ((u8 *)Array) + (RemovePos*sizeof(ArrayType)); \
u8 *Start = Goal + sizeof(ArrayType); \
memmove_s(Goal, MoveSize, Start, MoveSize); \
) 

// NOTE:: Cuts off the path from the file
inline i32 LastOccurrenceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter);
#define __FILENAME__ ((LastOccurrenceOfCharacterInString('\\', (u8 *)__FILE__, '\0') > 0) ? (__FILE__ + LastOccurrenceOfCharacterInString('\\', (u8 *)__FILE__, '\0')+1) : (__FILE__))

// NOTE:: Simple DebugLog that simplifies just printing stuff to the debug output.
//        If DEBUG_LOG_INFO is defined, it prints out file and line number.
#define DEBUG_LOG_INFO
#ifdef DEBUG_LOG_INFO
#define DebugLog(Count, Text, ...) { \
char _LogMemory1[Count]; \
sprintf_s(_LogMemory1, Text, __VA_ARGS__);\
char _LogMemory2[Count+260]; \
sprintf_s(_LogMemory2, "%s(%i): %s", __FILENAME__, __LINE__, _LogMemory1);\
OutputDebugStringA(_LogMemory2); \
}
#else
#define DebugLog(Count, Text, ...) { \
char B[Count]; \
sprintf_s(B, Text, __VA_ARGS__);\
OutputDebugStringA(B); \
}
#endif

// NOTE:: Simple DebugLog that simplifies just printing stuff to the debug output.
#define DebugPrint(Count, Text, ...) { \
char B[Count]; \
sprintf_s(B, Text, __VA_ARGS__);\
printf(B); \
}

#define MemoryCopy(dest, source, sizeInBytes) memcpy(dest, source, sizeInBytes)
#define MemorySet(memory, setTo, sizeInBytes) memset(memory, setTo, sizeInBytes)

#if DEBUG_TD
#define MAX_ASSERT_SIZE 512 + MAX_PATH
#define Assert(Expression, ...)                                                                  \
ToStatement(if(!(Expression)) {                                                              \
NewEmptyLocalString(AssertText, MAX_ASSERT_SIZE);                            \
Append(&AssertText, (u8 *)"Info: \n\n"##__VA_ARGS__##"\n\n");                \
/* 10 is the size of the first Append when nothing is in __VA_ARGS__. */     \
if(AssertText.Pos == 10) ResetStringCompound(AssertText);                    \
Append(&AssertText, (u8 *)"Assert fired at:\nFile: "##__FILE__##"\nLine: "); \
I32ToString(&AssertText, __LINE__);                                          \
MessageBoxA(0, (char *)AssertText.S, "Assert", MB_OK);                       \
*(int *)0 = 0; })
#else
#define Assert(Expression)
#endif

#define InvalidCodePath    Assert(!"InvalidCodePath");
#define NotImplemented     Assert(!"NotImplementedYet");
#define InvalidDefaultCase default: {Assert(!"InvalidDefaultCase");}

#endif //_DEFINITIONS__T_D_H
