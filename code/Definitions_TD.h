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

#ifndef __CD // Combine both params to a single name
#define __CD2(X, Y) X##Y
#define __CD(X, Y) __CD2(X, Y)
#endif

// NOTE:: General macro for removing an entry/item from an arbitrary array. Moving all following items one up.
#define RemoveItem(Array, ArrayCount, RemovePos, ArrayType) { \
u32 MoveSize = (ArrayCount-RemovePos)*sizeof(ArrayType); \
u8 *Goal = ((u8 *)Array) + (RemovePos*sizeof(ArrayType)); \
u8 *Start = Goal + sizeof(ArrayType); \
memmove_s(Goal, MoveSize, Start, MoveSize); \
} 

// NOTE:: Cuts off the path from the file
inline i32 LastOccurrenceOfCharacterInString(u8 CharToFind, u8 *String, u8 Delimiter);
#define __FILENAME__ ((LastOccurrenceOfCharacterInString('\\', (u8 *)__FILE__, '\0') > 0) ? (__FILE__ + LastOccurrenceOfCharacterInString('\\', (u8 *)__FILE__, '\0')+1) : (__FILE__))

// NOTE:: Simple DebugLog that simplifies just printing stuff to the debug output.
//        If DEBUG_LOG_INFO is defined, it prints out file and line number.
#define DEBUG_LOG_INFO
#ifdef DEBUG_LOG_INFO
#define DebugLog(Count, Text, ...) { \
char B[Count]; \
sprintf_s(B, Text, __VA_ARGS__);\
char C##__LINE__[Count+260]; \
sprintf_s(C##__LINE__, "%s(%i): %s", __FILENAME__, __LINE__, B);\
OutputDebugStringA(C##__LINE__); \
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

#define ToStatement(E) do{E}while(0)

#if DEBUG_TD
#define Assert(Expression)                                                                        \
ToStatement(if(!(Expression)) {                                                               \
DebugLog(1000, "Assert fired at:\nLine: %i\nFile: %s\n", __LINE__, __FILE__); \
*(int *)0 = 0;                                                                \
})                                       
#else
#define Assert(Expression)
#endif

#define InvalidCodePath    Assert(!"InvalidCodePath");
#define NotImplemented     Assert(!"NotImplementedYet");
#define InvalidDefaultCase default: {Assert(!"InvalidDefaultCase");}



#endif //_DEFINITIONS__T_D_H
