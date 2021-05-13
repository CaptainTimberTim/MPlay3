#include "Math_TD.h"
// General

inline void 
MathPrintln(i32 A)
{
    printf("%i\n", A);
}

inline void 
MathPrint(i32 A)
{
    printf("%i", A);
}

inline void 
MathPrintln(r32 A)
{
    printf("%f\n", A);
}

inline void 
MathPrint(r32 A)
{
    printf("%f", A);
}

inline void 
MathPrintln(v2 A)
{
    printf("%f, %f\n", A.x, A.y);
}

inline void 
MathPrint(v2 A)
{
    printf("%f, %f", A.x, A.y);
}

inline void 
MathPrintln(v2i A)
{
    printf("%i, %i\n", A.x, A.y);
}

inline void 
MathPrint(v2i A)
{
    printf("%i, %i", A.x, A.y);
}

inline void 
MathPrintln(v3 A)
{
    printf("%f, %f, %f\n", A.x, A.y, A.z);
}

inline void 
MathPrint(v3 A)
{
    printf("%f, %f, %f", A.x, A.y, A.z);
}

inline void 
MathPrintln(v3i A)
{
    printf("%i, %i, %i\n", A.x, A.y, A.z);
}

inline void 
MathPrint(v3i A)
{
    printf("%i, %i, %i", A.x, A.y, A.z);
}

inline void 
MathPrintln(v4 A)
{
    printf("%f, %f, %f, %f\n", A.w, A.x, A.y, A.z);
}

inline void 
MathPrint(v4 A)
{
    printf("%f, %f, %f, %f", A.w, A.x, A.y, A.z);
}

inline void 
MathPrintln(quaternion q)
{
    printf("%f, %f, %f, %f\n", q.s, q.x, q.y, q.z);
}

inline void 
MathPrint(quaternion q)
{
    printf("%f, %f, %f, %f", q.s, q.x, q.y, q.z);
}

inline void 
MathPrintln(matrix M)
{
    printf("%f, %f, %f, %f\n", M.E[0], M.E[1], M.E[2], M.E[3] );
    printf("%f, %f, %f, %f\n", M.E[4], M.E[5], M.E[6], M.E[7] );
    printf("%f, %f, %f, %f\n", M.E[8], M.E[9], M.E[10], M.E[11] );
    printf("%f, %f, %f, %f\n", M.E[12], M.E[13], M.E[14], M.E[15] );
}

inline void 
MathPrint(matrix M)
{
    printf("%f, %f, %f, %f", M.E[0], M.E[1], M.E[2], M.E[3] );
    printf("%f, %f, %f, %f", M.E[4], M.E[5], M.E[6], M.E[7] );
    printf("%f, %f, %f, %f", M.E[8], M.E[9], M.E[10], M.E[11] );
    printf("%f, %f, %f, %f", M.E[12], M.E[13], M.E[14], M.E[15] );
}

inline void 
MathPrintln(ray R)
{
    printf("Origin: %f, %f, %f, Direction: %f, %f, %f \n", 
           R.Origin.x, R.Origin.y, R.Origin.z, R.Direction.x, R.Direction.y, R.Direction.z);
}

inline void 
MathPrint(ray R)
{
    printf("Origin: %f, %f, %f, Direction: %f, %f, %f", 
           R.Origin.x, R.Origin.y, R.Origin.z, R.Direction.x, R.Direction.y, R.Direction.z);
}

// General Math

inline i32 
Abs(i32 Value)
{
    return abs(Value);
}

inline r32 
Abs(r32 Value)
{
    return fabsf(Value);
}

inline r32
Pow(r32 Base, r32 Exponent)
{
    r32 Result = 0;
    // TODO:: Stop using library and use compiler stuff...
    Result = powf(Base, Exponent);
    
    return Result;
}

inline void
Clamp(i32 *Value, i32 Minimum, i32 Maximum)
{
    if(*Value < Minimum)
    {
        *Value = Minimum;
    }
    else if(*Value > Maximum)
    {
        *Value = Maximum;
    }
}

inline void
Clamp(r32 *Value, r32 Minimum, r32 Maximum)
{
    if(*Value < Minimum)
    {
        *Value = Minimum;
    }
    else if(*Value > Maximum)
    {
        *Value = Maximum;
    }
}

inline r32
Clamp(r32 Value, r32 Minimum, r32 Maximum)
{
    r32 Result = 0;
    Result = (Value < Minimum) ? Minimum : ((Value > Maximum) ? Maximum : Value);
    return Result;
}

inline r32  
Clamp01(r32 Value)
{
    return Clamp(Value, 0.0f, 1.0f);
}

inline i32 
Clamp(i32 Value, i32 Minimum, i32 Maximum)
{
    i32 Result = 0;
    Result = (Value < Minimum) ? Minimum : ((Value > Maximum) ? Maximum : Value);
    return Result;
}

inline i32
Floor(r32 Value)
{
    i32 Result = 0;
    
    Result = (i32)(Value + 65536.0f) - 65536;
    
    return(Result);
}

inline i32 
Ceiling(r32 Value)
{
    i32 Result = 0;
    
    Result = 65536 - (i32)(65536.0f - Value);
    
    return(Result);
}

inline r32
FloorR32(r32 Value)
{
    r32 Result = 0;
    
    Result = (r32)((i32)(Value + 65536.0f) - 65536);
    
    return(Result);
}

inline r32 
CeilingR32(r32 Value)
{
    r32 Result = 0;
    
    Result = (r32)(65536 - (i32)(65536.0f - Value));
    
    return(Result);
}

inline i32
RoundReal32ToInt32(r32 Value)
{
    i32 Result;
    if(Value > 0){
        Result = (i32)(Value+0.5f);
    }
    else
    {
        Result = (i32)(Value-0.5f);
    }
    return(Result);
}

inline u32
SafeTruncateUInt64(u64 Value)
{
    u32 Result = (u32)Value;
    return(Result);
}

inline u32
RoundReal32ToUInt32(r32 Value)
{
    u32 Result;
    
    Result = (u32)(Value+0.5f);
    return(Result);
}

inline r32
SafeRatioN(r32 Numerator, r32 Divisor, r32 N)
{
    r32 Result = N;
    
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }
    
    return(Result);
}

inline r32
SafeRatio1(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
    return(Result);
}

inline r32
Square(r32 A)
{
    r32 Result;
    
    Result = A * A;
    
    return(Result);
}

inline i32
Square(i32 A)
{
    i32 Result;
    
    Result = A * A;
    
    return(Result);
}

inline r32
Sqrt(r32 A)
{
    r32 Result = sqrtf(A);
    return Result;
}

inline r32
Sqrt(u32 A)
{
    r32 Result = sqrtf((r32)A);
    return Result;
}

inline r32
DegToRad(r32 Degree)
{
    r32 Result = 0.0f;
    
    Result = (Degree*Pi32)/180.0f;
    
    return Result;
}

inline r32
RadToDeg(r32 Radian)
{
    r32 Result = 0.0f;
    
    Result = (Radian*180)/Pi32;
    
    return Result;
}

inline r32 
Log(r32 x)
{
    return logf(x);
}

inline r32 
Lerp(r32 A, r32 B, r32 T)
{
    r32 Result = A + (B - A)*T;
    return Result;
}

inline r32 
Lerp(i32 A, i32 B, r32 T)
{
    r32 Result = A + (B - A)*T;
    return Result;
}

inline r32
Root2_8(r32 V)
{
    r32 Result = Pow(V, 1.0f/2.8f);
    return Result;
}

inline r32
GraphFirstQuickThenSlow(r32 x)
{
    return -Pow(x-1, 4) + 1;
}

inline i32
Max(i32 A, i32 B)
{
    i32 Result = max(A, B);
    return Result;
}

inline r32
Max(r32 A, r32 B)
{
    r32 Result = max(A, B);
    return Result;
}

inline u64
Max(u64 A, u64 B)
{
    u64 Result = max(A, B);
    return Result;
}

inline u32
Max(u32 A, u32 B)
{
    u32 Result = max(A, B);
    return Result;
}

inline i32
Min(i32 A, i32 B)
{
    i32 Result = min(A, B);
    return Result;
}

inline u32
Min(u32 A, u32 B)
{
    u32 Result = min(A, B);
    return Result;
}

inline r32
Min(r32 A, r32 B)
{
    r32 Result = min(A, B);
    return Result;
}

inline r32
ACos(r32 A)
{
    r32 Result = acosf(A);
    return Result;
}

inline r32
SafeDiv(r32 A, r32 B)
{
    r32 Result = 0;
    
    if(B != 0) Result = A/B;
    
    return Result;
}

inline i32
SafeDiv(i32 A, i32 B)
{
    i32 Result = 0;
    
    if(B != 0) Result = A/B;
    
    return Result;
}

inline r32 
Mod(r32 V, r32 M)
{
    return fmodf(V, M);
}

inline r64
Mod(r64 V, r64 M)
{
    return fmod(V, M);
}

// Vector math
// V2

inline v2
V2(r32 X, r32 Y)
{
    v2 Result;
    Result.x = X;
    Result.y = Y;
    return(Result);
}

inline v2 
V2(r32 X)
{
    v2 Result;
    Result.x = X;
    Result.y = X;
    return(Result);
}

inline v2
V2(v2i V)
{
    v2 Result = {(r32)V.x, (r32)V.y};
    return Result;
}

inline v2 
operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2
operator+(v2 A, r32 B)
{
    v2 Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    
    return(Result);
}

inline v2
operator+(r32 A, v2 B)
{
    v2 Result;
    
    Result = B + A;
    
    return(Result);
}

inline v2
operator+(v2 A, v2i B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return(A);
}

inline v2
operator-(v2 A)
{
    v2 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2 
operator-(v2 A, v2 B)
{
    v2 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline v2
operator-(v2 A, r32 B)
{
    v2 Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    
    return(Result);
}

inline v2
operator-(r32 A, v2 B)
{
    v2 Result;
    
    Result = B - A;
    
    return(Result);
}

inline v2 
operator-(v2 A, v2i B)
{
    v2 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline v2 &
operator-=(v2 &A, v2 &B)
{
    A = A - B;
    
    return(A);
}

inline v2 &
operator-=(v2 &A, v2i &B)
{
    A = A - B;
    
    return(A);
}

inline v2 
operator*(v2 A, r32 B)
{
    
    v2 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    
    return(Result);
}

inline v2 
operator*(r32 A, v2 B)
{
    
    v2 Result;
    
    Result = B * A;
    
    return(Result);
}

inline v2 & 
operator*=(v2 &A, r32 B)
{
    A = A * B;
    
    return(A);
}

inline v2
operator/(v2 A, r32 B)
{
    v2 Result = {};
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    
    return Result;
}

inline v2 &
operator/=(v2 &A, r32 &B)
{
    A.x = A.x / B;
    A.y = A.y / B;
    
    return A;
}

inline r32
Dot(v2 A, v2 B)
{
    r32 Result;
    
    Result = A.x*B.x + A.y*B.y;
    
    return(Result);
}

inline r32
Cross(v2 A, v2 B)
{
    r32 Result;
    
    Result = A.x*B.y - A.y*B.x;
    
    return(Result);
}

inline r32
LengthSquared(v2 A)
{
    r32 Result;
    
    Result = A.x*A.x + A.y*A.y;
    
    return(Result);
}

inline r32
Length(v2 A)
{
    r32 Result;
    
    Result = (r32)sqrt(LengthSquared(A));
    
    return(Result);
}

inline r32
Distance(v2 A, v2 B)
{
    r32 Result;
    
    Result = Length(A - B);
    
    return(Result);
}

inline v2
Normalize(v2 A)
{
    v2 Result = {};
    
    Result = A/Length(A);
    
    return Result;
}

inline v2
HadamardProduct(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    
    return(Result);
}

inline v2
Scale(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    
    return(Result);
}

inline v2
HadamardDivision(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    
    return(Result);
}

inline v2
Lerp(v2 A, v2 B, r32 T)
{
    v2 Result = {};
    
    Result.x = Lerp(A.x, B.x, T);
    Result.y = Lerp(A.y, B.y, T);
    
    return Result;
}

inline v2
Rotate(v2 P, v2 RotationP, r32 AngleRad)
{
    v2 Result = {};
    
    P -= RotationP;
    
    Result.x = P.x*cosf(AngleRad) - P.y*sinf(AngleRad);
    Result.y = P.y*cosf(AngleRad) + P.x*sinf(AngleRad);
    
    Result += RotationP;
    
    return Result;
}

// Vector2i integer32

inline v2i
V2i(i32 X, i32 Y)
{
    v2i Result;
    Result.x = X;
    Result.y = Y;
    return(Result);
}

inline v2i 
V2i(i32 X)
{
    v2i Result;
    Result.x = X;
    Result.y = X;
    return(Result);
}

inline v2i 
operator+(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2i
operator+(v2i A, i32 B)
{
    v2i Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    
    return(Result);
}

inline v2i
operator+(i32 A, v2i B)
{
    v2i Result;
    
    Result = B + A;
    
    return(Result);
}

inline v2i &
operator+=(v2i &A, v2i &B)
{
    A = A + B;
    
    return(A);
}

inline v2i
operator-(v2i A)
{
    v2i Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2i 
operator-(v2i A, v2i B)
{
    v2i Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline v2i
operator-(v2i A, i32 B)
{
    v2i Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    
    return(Result);
}

inline v2i
operator-(i32 A, v2i B)
{
    v2i Result;
    
    Result = B - A;
    
    return(Result);
}

inline v2i &
operator-=(v2i &A, v2i &B)
{
    A = A - B;
    
    return(A);
}

inline v2i 
operator*(v2i A, i32 B)
{
    
    v2i Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    
    return(Result);
}

inline v2i 
operator*(i32 A, v2i B)
{
    
    v2i Result;
    
    Result = B * A;
    
    return(Result);
}

inline v2i & 
operator*=(v2i &A, i32 B)
{
    A = A * B;
    
    return(A);
}

inline v2i
operator/(v2i A, i32 B)
{
    v2i Result = {};
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    
    return Result;
}

inline v2i &
operator/=(v2i &A, i32 &B)
{
    A.x = A.x / B;
    A.y = A.y / B;
    
    return A;
}

inline v2
operator/(v2i A, r32 B)
{
    v2 Result = {};
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    
    return Result;
}

inline i32
Dot(v2i A, v2i B)
{
    i32 Result;
    
    Result = A.x*B.x + A.y*B.y;
    
    return(Result);
}

inline i32
Cross(v2i A, v2i B)
{
    i32 Result;
    
    Result = A.x*B.y - A.y*B.x;
    
    return(Result);
}

inline i32
LengthSquared(v2i A)
{
    i32 Result;
    
    Result = A.x*A.x + A.y*A.y;
    
    return(Result);
}

inline r32
Length(v2i A)
{
    r32 Result;
    
    Result = (r32)sqrt(LengthSquared(A));
    
    return(Result);
}

inline r32
Distance(v2i A, v2i B)
{
    r32 Result;
    
    Result = Length(A - B);
    
    return(Result);
}

inline v2
Normalize(v2i A)
{
    v2 Result = {};
    
    Result = A/Length(A);
    
    return Result;
}

inline v2i
HadamardProduct(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    
    return(Result);
}

inline v2i
HadamardDivision(v2i A, v2i B)
{
    v2i Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    
    return(Result);
}

inline v2
HadamardProduct(v2 A, v2i B)
{
    v2 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    
    return(Result);
}

inline v2
HadamardDivision(v2 A, v2i B)
{
    v2 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    
    return(Result);
}

inline v2
HadamardDivision(v2i A, v2 B)
{
    v2 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    
    return(Result);
}

// V3 *****************

inline v3
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    return(Result);
}

inline v3 
V3(r32 V)
{
    v3 Result;
    Result.x = V;
    Result.y = V;
    Result.z = V;
    return(Result);
}

inline v3 
operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

inline v3
operator+(v3 A, r32 B)
{
    v3 Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;
    
    return(Result);
}

inline v3
operator+(r32 A, v3 B)
{
    v3 Result;
    
    Result = B + A;
    
    return(Result);
}

inline v3 &
operator+=(v3 &A, v3 &B)
{
    A = A + B;
    
    return(A);
}

inline v3
operator-(v3 A)
{
    v3 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    
    return(Result);
}

inline v3 
operator-(v3 A, v3 B)
{
    v3 Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}


inline v3
operator-(v3 A, r32 B)
{
    v3 Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;
    
    return(Result);
}

inline v3
operator-(r32 A, v3 B)
{
    v3 Result;
    
    Result = B - A;
    
    return(Result);
}


inline v3 &
operator-=(v3 &A, v3 &B)
{
    A = A - B;
    
    return(A);
}

inline v3 
operator*(v3 A, r32 B)
{
    
    v3 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    
    return(Result);
}

inline v3 
operator*(r32 A, v3 B)
{
    
    v3 Result;
    
    Result = B * A;
    
    return(Result);
}

inline v3 & 
operator*=(v3 &A, r32 B)
{
    A = A * B;
    
    return(A);
}

inline v3 
operator/(v3 A, r32 B)
{
    v3 Result = {};
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;
    
    return Result;
}

inline v3 &
operator/=(v3 &A, r32 &B)
{
    A.x = A.x / B;
    A.y = A.y / B;
    A.z = A.z / B;
    
    return A;
}

inline b32
operator!=(v3 A, v3 B)
{
    b32 Result = (A.x != B.x ||
                  A.y != B.y ||
                  A.z != B.z);
    return Result;
}

inline b32
operator==(v3 A, v3 B)
{
    b32 Result = (A.x == B.x &&
                  A.y == B.y &&
                  A.z == B.z);
    return Result;
}

inline r32 
Dot(v3 A, v3 B)
{
    r32 Result = 0.0f;
    
    Result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return Result;
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result = {};
    
    Result.x = (A.y*B.z) - (A.z*B.y);
    Result.y = (A.z*B.x) - (A.x*B.z);
    Result.z = (A.x*B.y) - (A.y*B.x);
    
    return Result;
}

inline r32
LengthSquared(v3 A)
{
    r32 Result;
    
    Result = Dot(A, A);
    
    return(Result);
}

inline r32
Length(v3 A)
{
    r32 Result;
    
    Result = (r32)sqrt(LengthSquared(A));
    
    return(Result);
}

inline r32
Distance(v3 A, v3 B)
{
    r32 Result;
    
    Result = Length(A - B);
    
    return(Result);
}

inline v3
HadamardProduct(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    
    return(Result);
}

inline v3
HadamardDivision(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    
    return(Result);
}

inline v3 
Normalize(v3 A)
{
    v3 Result = {};
    
    Result = A/Length(A);
    
    return Result;
}

inline u32
Vector3ToUInt32(v3 Value)
{
    u32 Result;
    Result = (RoundReal32ToUInt32(Value.x * 255.0f) << 16)|
        (RoundReal32ToUInt32(Value.y * 255.0f) << 8)|
        RoundReal32ToUInt32(Value.z *255.0f);
    return(Result);
}

inline r32
AngleBetween(v3 A, v3 B)
{
    r32 Result = 0;
    
    Result = ACos(Dot(Normalize(A), Normalize(B)));
    
    return Result;
}

inline v3
AxisBetween(v3 A, v3 B)
{
    v3 Result = {};
    
    Result = Normalize(Cross(A, B));
    
    return Result;
}

inline void 
Zero(v3 *V)
{
    V->x = 0;
    V->y = 0;
    V->z = 0;
}

inline v3 
TripleCross(v3 A, v3 B, v3 C)
{
    v3 Result = Cross(Cross(A, B), C);
    return Result;
}

inline v3
Lerp(v3 A, v3 B, r32 T)
{
    v3 Result = {};
    
    Result.x = Lerp(A.x, B.x, T);
    Result.y = Lerp(A.y, B.y, T);
    Result.z = Lerp(A.z, B.z, T);
    
    return Result;
}

inline v3 
Clamp01(v3 V)
{
    v3 Result = {};
    
    Result.x = Clamp01(V.x);
    Result.y = Clamp01(V.y);
    Result.z = Clamp01(V.z);
    
    return Result;
}

// V3 integer 

inline v3
V3i(u32 X, u32 Y, u32 Z)
{
    v3 Result;
    Result.x = (r32)X;
    Result.y = (r32)Y;
    Result.z = (r32)Z;
    return(Result);
}

inline v3i
V3i(v3 Vec)
{
    v3i Result;
    
    Result.x = (i32)Vec.x;
    Result.y = (i32)Vec.y;
    Result.z = (i32)Vec.z;
    
    return Result;
}

inline v3i 
operator+(v3i A, v3i B)
{
    v3i Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

inline v3i
operator+(v3i A, i32 B)
{
    v3i Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;
    
    return(Result);
}

inline v3i
operator+(i32 A, v3i B)
{
    v3i Result;
    
    Result = B + A;
    
    return(Result);
}

inline v3i &
operator+=(v3i &A, v3i &B)
{
    A = A + B;
    
    return(A);
}

inline v3i
operator-(v3i A)
{
    v3i Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    
    return(Result);
}

inline v3i 
operator-(v3i A, v3i B)
{
    v3i Result = {};
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}


inline v3i
operator-(v3i A, i32 B)
{
    v3i Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;
    
    return(Result);
}

inline v3i
operator-(i32 A, v3i B)
{
    v3i Result;
    
    Result = B - A;
    
    return(Result);
}


inline v3i &
operator-=(v3i &A, v3i &B)
{
    A = A - B;
    
    return(A);
}

inline v3i 
operator*(v3i A, i32 B)
{
    
    v3i Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    
    return(Result);
}

inline v3i 
operator*(i32 A, v3i B)
{
    
    v3i Result;
    
    Result = B * A;
    
    return(Result);
}

inline v3i & 
operator*=(v3i &A, i32 B)
{
    A = A * B;
    
    return(A);
}

inline v3i 
operator/(v3i A, i32 B)
{
    v3i Result = {};
    
    Result.x = A.x / B;
    Result.y = A.y / B;
    Result.z = A.z / B;
    
    return Result;
}

inline v3i &
operator/=(v3i &A, i32 &B)
{
    A.x = A.x / B;
    A.y = A.y / B;
    A.z = A.z / B;
    
    return A;
}



// V4

inline v4
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return(Result);
}

inline v4
V4(r32 A)
{
    return V4(A, A, A, A);
}

inline v4 
operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return(Result);
}

inline v4
operator+(v4 A, r32 B)
{
    v4 Result;
    
    Result.x = A.x + B;
    Result.y = A.y + B;
    Result.z = A.z + B;
    Result.w = A.w + B;
    
    return(Result);
}

inline v4
operator+(r32 A, v4 B)
{
    v4 Result;
    
    Result = B + A;
    
    return(Result);
}

inline v4 &
operator+=(v4 &A, v4 &B)
{
    A = A + B;
    
    return(A);
}

inline v4
operator-(v4 A)
{
    v4 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;
    
    return(Result);
}

inline v4 
operator-(v4 A, v4 B)
{
    v4 Result = {};
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
    
    return(Result);
}


inline v4
operator-(v4 A, r32 B)
{
    v4 Result;
    
    Result.x = A.x - B;
    Result.y = A.y - B;
    Result.z = A.z - B;
    Result.w = A.w - B;
    
    return(Result);
}

inline v4
operator-(r32 A, v4 B)
{
    v4 Result;
    
    Result = B - A;
    
    return(Result);
}


inline v4 &
operator-=(v4 &A, v4 &B)
{
    A = A - B;
    
    return(A);
}

inline v4 
operator*(v4 A, r32 B)
{
    
    v4 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;
    
    return(Result);
}

inline v4 
operator*(r32 A, v4 B)
{
    
    v4 Result;
    
    Result = B * A;
    
    return(Result);
}

inline v4 & 
operator*=(v4 &A, r32 B)
{
    A = A * B;
    
    return(A);
}

inline v4 
operator/(v4 A, r32 B)
{
    v4 Result = {};
    
    Result.x = A.x/B;
    Result.y = A.y/B;
    Result.z = A.z/B;
    Result.w = A.w/B;
    
    return Result;
}

inline v4 &
operator/=(v4 &A, r32 &B)
{
    A.x = A.x/B;
    A.y = A.y/B;
    A.z = A.z/B;
    A.w = A.w/B;
    
    return A;
}

inline b32
operator==(v2 A, v2 B)
{
    b32 Result = (A.x == B.x &&
                  A.y == B.y);
    return Result;
}

inline r32 
Dot(v4 A, v4 B)
{
    r32 Result = 0.0f;
    
    Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
    
    return Result;
}

inline r32
LengthSquared(v4 A)
{
    r32 Result;
    
    Result = Dot(A, A);
    
    return(Result);
}

inline r32
Length(v4 A)
{
    r32 Result;
    
    Result = (r32)sqrt(LengthSquared(A));
    
    return(Result);
}

inline r32
Distance(v4 A, v4 B)
{
    r32 Result;
    
    Result = Length(A - B);
    
    return(Result);
}

inline v4
Normalize(v4 A)
{
    v4 Result = {};
    
    Result = A/Length(A);
    
    return Result;
}

inline v4
Lerp(v4 A, v4 B, r32 T)
{
    v4 Result = {};
    
    Result.x = Lerp(A.x, B.x, T);
    Result.y = Lerp(A.y, B.y, T);
    Result.z = Lerp(A.z, B.z, T);
    Result.w = Lerp(A.w, B.w, T);
    
    return Result;
}

inline v4 
HadamardProduct(v4 A, v4 B)
{
    v4 Result = {};
    
    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    Result.z = A.z*B.z;
    Result.w = A.w*B.w;
    
    return Result;
}

// Quaternion

inline quaternion
Quaternion()
{
    quaternion Result = {};
    
    Result.s = 1;
    
    return Result;
}

inline quaternion
Quaternion(r32 s, r32 x, r32 y, r32 z)
{
    quaternion Result = {};
    
    Result.s = s;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

inline quaternion
operator+(quaternion Q1, quaternion Q2)
{
    quaternion Result = {};
    
    Result.s = Q1.s + Q2.s;
    Result.v = Q1.v + Q2.v;
    
    return Result;
}

inline quaternion &
operator+=(quaternion &Q1, quaternion &Q2)
{
    Q1.s = Q1.s + Q2.s;
    Q1.v = Q1.v + Q2.v;
    
    return Q1;
}

inline quaternion
operator-(quaternion Q1, quaternion Q2)
{
    quaternion Result = {};
    
    Result.s = Q1.s - Q2.s;
    Result.v = Q1.v - Q2.v;
    
    return Result;
}

inline quaternion &
operator-=(quaternion &Q1, quaternion &Q2)
{
    Q1.s = Q1.s - Q2.s;
    Q1.v = Q1.v - Q2.v;
    
    return Q1;
}

inline quaternion
operator*(quaternion Q1, quaternion Q2)
{
    quaternion Result = {};
    
    Result.s = Q1.s*Q2.s - Dot(Q1.v, Q2.v);
    Result.v = Cross(Q1.v, Q2.v) + Q1.s*Q2.v + Q2.s*Q1.v;
    
    return Result;
}

inline quaternion &
operator*=(quaternion &Q1, quaternion &Q2)
{
    Q1.s = Q1.s*Q2.s - Dot(Q1.v, Q2.v);
    Q1.v = Cross(Q1.v, Q2.v) + Q1.s*Q2.v + Q2.s*Q1.v;
    
    return Q1;
}

inline quaternion  
operator* (r32 S, quaternion Q)
{
    quaternion Result = {};
    
    Result.s = Q.s*S;
    Result.x = Q.x*S;
    Result.y = Q.y*S;
    Result.z = Q.z*S;
    
    return Result;
}

inline quaternion 
operator/(quaternion Q1, r32 R)
{
    quaternion Result = {};
    
    Result.s = Q1.s/R;
    Result.v = Q1.v/R;
    
    return Result;
}

inline quaternion & 
operator/=(quaternion &Q1, r32 &R)
{
    Q1.s = Q1.s/R;
    Q1.v = Q1.v/R;
    
    return Q1;
}

inline quaternion
Conjugate(quaternion Q)
{
    quaternion Result = {};
    
    Result.s = Q.s;
    Result.v = -Q.v;
    
    return Result;
}

inline r32
Norm(quaternion Q)
{
    r32 Result = {};
    
    Result = Q.s*Q.s + LengthSquared(Q.v);
    
    return Result;
}

inline r32
Length(quaternion Q)
{
    r32 Result = 0.0f;
    
    Result = (r32)sqrt(Norm(Q));
    
    return Result;
}

inline quaternion
Normalize(quaternion Q)
{
    quaternion Result = {};
    
    Result = Q/Length(Q);
    
    return Result;
}

inline void
Normalize(quaternion *Q)
{
    *Q = *Q/Length(*Q);
}

inline quaternion
Inverse(quaternion Q)
{
    quaternion Result = {};
    
    Result = Conjugate(Q)/Norm(Q);
    
    return Result;
}

inline v3
operator*(quaternion Q, v3 P)
{
    v3 Result = {};
    quaternion PQ = Quaternion(0, P.x, P.y, P.z);
    
    Result = (Q*PQ*Conjugate(Q)).v;
    
    return Result;
}


inline quaternion 
EulerToQuaternion(r32 Roll, r32 Pitch, r32 Yaw)
{
    quaternion Result = {};
    
    Yaw   = DegToRad(Yaw);
    Pitch = DegToRad(Pitch);
    Roll  = DegToRad(Roll);
    
    r32 YawCos   = (r32)cos(Yaw   * 0.5f);
    r32 YawSin   = (r32)sin(Yaw   * 0.5f);
    r32 PitchCos = (r32)cos(Pitch * 0.5f);
    r32 PitchSin = (r32)sin(Pitch * 0.5f);
    r32 RollCos  = (r32)cos(Roll  * 0.5f);
    r32 RollSin  = (r32)sin(Roll  * 0.5f);
    
    Result.s = RollCos*PitchCos*YawCos + RollSin*PitchSin*YawSin;
    Result.x = RollSin*PitchCos*YawCos - RollCos*PitchSin*YawSin;
    Result.y = RollCos*PitchSin*YawCos + RollSin*PitchCos*YawSin;
    Result.z = RollCos*PitchCos*YawSin - RollSin*PitchSin*YawCos;
    
    return Result;
}

inline quaternion 
EulerToQuaternion(v3 E)
{
    quaternion Result = {};
    
    Result = EulerToQuaternion(E.x, E.y, E.z);
    
    return Result;
}

inline v3 
QuaternionToEuler(quaternion Q)
{
    v3 Result = {};
    
    r32 RollSin = 2.0f*(Q.s*Q.x + Q.y*Q.z);
    r32 RollCos = 1.0f - 2.0f*(Q.x*Q.x + Q.y*Q.y);
    Result.x = (r32)atan2(RollSin, RollCos);
    
    r32 PitchSin = 2.0f*(Q.s*Q.y - Q.z*Q.x);
    if(fabs(PitchSin) >= 1)
    {
        // NOTE:: CopySign takes the first value and
        // puts the sign of the second value on it.
        Result.y = (r32)copysign(Pi32/2, PitchSin);
    }
    else
    {
        Result.y = (r32)asin(PitchSin);
    }
    
    r32 YawSin = 2.0f*(Q.s*Q.z + Q.x*Q.y);
    r32 YawCos = 1.0f - 2.0f*(Q.y*Q.y + Q.z*Q.z);
    Result.z = (r32)atan2(YawSin, YawCos);
    
    return Result;
}

inline v3 
QuaternionToEulerDeg(quaternion Q)
{
    v3 Result = QuaternionToEuler(Q);
    
    Result.x = RadToDeg(Result.x);
    Result.y = RadToDeg(Result.y);
    Result.z = RadToDeg(Result.z);
    
    return Result;
}

inline m4         
QuaternionToMatrix(quaternion Q)
{
#if 0    
    r32 A2 = Square(Q.a);
    r32 B2 = Square(Q.b);
    r32 C2 = Square(Q.c);
    r32 D2 = Square(Q.d);
    
    m4 Result = 
    {
        (A2+B2-C2-D2), (2*Q.b*Q.c - 2*Q.a*Q.d), (2*Q.b*Q.d + 2*Q.a*Q.c), 0,
        (2*Q.b*Q.c - 2*Q.a*Q.d), (A2-B2+C2-D2), (2*Q.c*Q.d + 2*Q.a*Q.b), 0,
        (2*Q.b*Q.d + 2*Q.a*Q.c), (2*Q.c*Q.d + 2*Q.a*Q.b), (A2-B2-C2+D2), 0,
        0, 0, 0, 1,
    };
#else 
    r32 W2 = Square(Q.w);
    r32 X2 = Square(Q.x);
    r32 Y2 = Square(Q.y);
    r32 Z2 = Square(Q.z);
    
    m4 Result =
    {
        (1 - 2*Y2 - 2*Z2), (2*Q.x*Q.y - 2*Q.z*Q.w), (2*Q.x*Q.z + 2*Q.y*Q.w), 0, 
        (2*Q.x*Q.y + 2*Q.z*Q.w), (1 - 2*X2 - 2*Z2), (2*Q.y*Q.z - 2*Q.x*Q.w), 0, 
        (2*Q.x*Q.z - 2*Q.y*Q.w), (2*Q.y*Q.z + 2*Q.x*Q.w), (1 - 2*X2 - 2*Y2), 0,
        0, 0, 0, 1
    };
    
#endif
    return Result;
}

inline quaternion 
MatrixToQuaternion(m4 M)
{
    quaternion Result = {};
    
    r32 Tr = M.m00 + M.m11 + M.m22;
    if(Tr > 0.0f)
    {
        r32 S = Sqrt(Tr+1.0f)*2;
        Result.w = 0.25f*S;
        Result.x = (M.m21 - M.m12)/S;
        Result.y = (M.m02 - M.m20)/S;
        Result.z = (M.m10 - M.m01)/S;
    }
    else if((M.m00 > M.m11) && (M.m00 > M.m22))
    {
        r32 S = Sqrt(1.0f + M.m00 - M.m11 - M.m22)*2;
        Result.w = (M.m21 - M.m12)/S;
        Result.x = 0.25f*S;
        Result.y = (M.m01 - M.m10)/S;
        Result.z = (M.m02 - M.m20)/S;
    }
    else if(M.m11 > M.m22)
    {
        r32 S = Sqrt(1.0f + M.m11 - M.m00 - M.m22)*2;
        Result.w = (M.m02 - M.m20)/S;
        Result.x = (M.m01 - M.m10)/S;
        Result.y = 0.25f*S;
        Result.z = (M.m12 - M.m21)/S;
    }
    else
    {
        r32 S = Sqrt(1.0f + M.m22 - M.m00 - M.m11)*2;
        Result.w = (M.m10 - M.m01)/S;
        Result.x = (M.m02 - M.m20)/S;
        Result.y = (M.m12 - M.m21)/S;
        Result.z = 0.25f*S;
    }
    
    return Result;
}


// Matrix math **************************

inline matrix 
Matrix()
{
    matrix Result = Identity();
    return Result;
}

inline matrix 
Matrix(r32 m00, r32 m01, r32 m02, r32 m03, 
       r32 m10, r32 m11, r32 m12, r32 m13, 
       r32 m20, r32 m21, r32 m22, r32 m23, 
       r32 m30, r32 m31, r32 m32, r32 m33)
{
    matrix Result = 
    {
        m00, m01, m02, m03, 
        m10, m11, m12, m13, 
        m20, m21, m22, m23, 
        m30, m31, m32, m33 
    };
    return Result;
}

inline matrix 
operator*(matrix &M1, matrix &M2)
{
    matrix Result = {};
    
    Result = Multiply(M1, M2);
    
    return Result;
}

inline matrix &
operator*=(matrix &M1, matrix &M2)
{
    
    M1 = Multiply(M1, M2);
    return M1;
}

inline v4     
operator*(matrix &M, v4 V)
{
    v4 Result = {};
    
    Result = TransformV4x4(M, V);
    
    return Result;
}

inline v3     
operator*(matrix &M, v3 V)
{
    v3 Result = {};
    
    Result = TransformV3x4(M, V);
    
    return Result;
}

#if 0
inline matrix 
Identity()
{
    matrix Result = {};
    
    Result.m00 = Result.m11 = Result.m22 = Result.m33 = 1;
    
    return Result;
}
#endif

inline void   
Transpose(matrix *M)
{
    matrix Result = 
    {
        M->m00, M->m10, M->m20, M->m30,
        M->m01, M->m11, M->m21, M->m31,
        M->m02, M->m12, M->m22, M->m32,
        M->m03, M->m13, M->m23, M->m33
    };
    
    *M = Result;
}

inline void   
Invert(matrix &M)
{
    r32 num5  = M.m00;
    r32 num4  = M.m01;
    r32 num3  = M.m02;
    r32 num2  = M.m03;
    r32 num9  = M.m10;
    r32 num8  = M.m11;
    r32 num7  = M.m12;
    r32 num6  = M.m13;
    r32 num17 = M.m20;
    r32 num16 = M.m21;
    r32 num15 = M.m22;
    r32 num14 = M.m23;
    r32 num13 = M.m30;
    r32 num12 = M.m31;
    r32 num11 = M.m32;
    r32 num10 = M.m33;
    r32 num23 = (num15 * num10) - (num14 * num11);
    r32 num22 = (num16 * num10) - (num14 * num12);
    r32 num21 = (num16 * num11) - (num15 * num12);
    r32 num20 = (num17 * num10) - (num14 * num13);
    r32 num19 = (num17 * num11) - (num15 * num13);
    r32 num18 = (num17 * num12) - (num16 * num13);
    r32 num39 =   ((num8 * num23) - (num7 * num22)) + (num6 * num21);
    r32 num38 = -(((num9 * num23) - (num7 * num20)) + (num6 * num19));
    r32 num37 =   ((num9 * num22) - (num8 * num20)) + (num6 * num18);
    r32 num36 = -(((num9 * num21) - (num8 * num19)) + (num7 * num18));
    r32 num   = SafeRatio1(1.0f, ((((num5 * num39) + (num4 * num38)) + (num3 * num37)) + (num2 * num36)));
    M.m00 = num39 * num;
    M.m10 = num38 * num;
    M.m20 = num37 * num;
    M.m30 = num36 * num;
    M.m01 = -(((num4 * num23) - (num3 * num22)) + (num2 * num21)) * num;
    M.m11 =  (((num5 * num23) - (num3 * num20)) + (num2 * num19)) * num;
    M.m21 = -(((num5 * num22) - (num4 * num20)) + (num2 * num18)) * num;
    M.m31 =  (((num5 * num21) - (num4 * num19)) + (num3 * num18)) * num;
    r32 num35 = (num7 * num10) - (num6 * num11);
    r32 num34 = (num8 * num10) - (num6 * num12);
    r32 num33 = (num8 * num11) - (num7 * num12);
    r32 num32 = (num9 * num10) - (num6 * num13);
    r32 num31 = (num9 * num11) - (num7 * num13);
    r32 num30 = (num9 * num12) - (num8 * num13);
    M.m02 =  (((num4 * num35) - (num3 * num34)) + (num2 * num33)) * num;
    M.m12 = -(((num5 * num35) - (num3 * num32)) + (num2 * num31)) * num;
    M.m22 =  (((num5 * num34) - (num4 * num32)) + (num2 * num30)) * num;
    M.m32 = -(((num5 * num33) - (num4 * num31)) + (num3 * num30)) * num;
    r32 num29 = (num7 * num14) - (num6 * num15);
    r32 num28 = (num8 * num14) - (num6 * num16);
    r32 num27 = (num8 * num15) - (num7 * num16);
    r32 num26 = (num9 * num14) - (num6 * num17);
    r32 num25 = (num9 * num15) - (num7 * num17);
    r32 num24 = (num9 * num16) - (num8 * num17);
    M.m03 = -(((num4 * num29) - (num3 * num28)) + (num2 * num27)) * num;
    M.m13 = (((num5 * num29) - (num3 * num26)) + (num2 * num25)) * num;
    M.m23 = -(((num5 * num28) - (num4 * num26)) + (num2 * num24)) * num;
    M.m33 = (((num5 * num27) - (num4 * num25)) + (num3 * num24)) * num;
}

inline v3     
TransformV3x4(matrix &M, v3 V)
{
    v3 Result = {};
    
    Result.x = M.m00*V.x + M.m01*V.y + M.m02*V.z + M.m03;
    Result.y = M.m10*V.x + M.m11*V.y + M.m12*V.z + M.m13;
    Result.z = M.m20*V.x + M.m21*V.y + M.m22*V.z + M.m23;
    r32 W = M.m30*V.x + M.m31*V.y + M.m32*V.z + M.m33;
    
    Result /= W;
    
    return Result;
}

inline v3     
TransformV3x3(matrix &M, v3 V)
{
    v3 Result = {};
    
    Result.x = M.m00*V.x + M.m01*V.y + M.m02*V.z;
    Result.y = M.m10*V.x + M.m11*V.y + M.m12*V.z;
    Result.z = M.m20*V.x + M.m21*V.y + M.m22*V.z;
    
    return Result;
}

inline v4     
TransformV4x4(matrix &M, v4 V)
{
    v4 Result = {};
    
    Result.x = M.m00*V.x + M.m01*V.y + M.m02*V.z + M.m03*V.w;
    Result.y = M.m10*V.x + M.m11*V.y + M.m12*V.z + M.m13*V.w;
    Result.z = M.m20*V.x + M.m21*V.y + M.m22*V.z + M.m23*V.w;
    Result.w = M.m30*V.x + M.m31*V.y + M.m32*V.z + M.m33*V.w;
    
    return Result;
}

inline r32 
Determinant(matrix &M)
{
    r32 Result = 0.0f;
    
    Result = 
        M.m00*(M.m11*M.m22 - M.m12*M.m21) - 
        M.m01*(M.m10*M.m22 - M.m12*M.m20) + 
        M.m02*(M.m10*M.m21 - M.m11*M.m20);
    
    return Result;
}


// Affine matrix transformations
inline matrix 
Multiply(matrix &M1, matrix &M2)
{
    matrix Result = 
    {
        M1.m00*M2.m00 + M1.m01*M2.m10 + M1.m02*M2.m20 + M1.m03*M2.m30,
        M1.m00*M2.m01 + M1.m01*M2.m11 + M1.m02*M2.m21 + M1.m03*M2.m31,
        M1.m00*M2.m02 + M1.m01*M2.m12 + M1.m02*M2.m22 + M1.m03*M2.m32,
        M1.m00*M2.m03 + M1.m01*M2.m13 + M1.m02*M2.m23 + M1.m03*M2.m33,
        
        M1.m10*M2.m00 + M1.m11*M2.m10 + M1.m12*M2.m20 + M1.m13*M2.m30,
        M1.m10*M2.m01 + M1.m11*M2.m11 + M1.m12*M2.m21 + M1.m13*M2.m31,
        M1.m10*M2.m02 + M1.m11*M2.m12 + M1.m12*M2.m22 + M1.m13*M2.m32,
        M1.m10*M2.m03 + M1.m11*M2.m13 + M1.m12*M2.m23 + M1.m13*M2.m33,
        
        M1.m20*M2.m00 + M1.m21*M2.m10 + M1.m22*M2.m20 + M1.m23*M2.m30,
        M1.m20*M2.m01 + M1.m21*M2.m11 + M1.m22*M2.m21 + M1.m23*M2.m31,
        M1.m20*M2.m02 + M1.m21*M2.m12 + M1.m22*M2.m22 + M1.m23*M2.m32,
        M1.m20*M2.m03 + M1.m21*M2.m13 + M1.m22*M2.m23 + M1.m23*M2.m33,
        
        M1.m30*M2.m00 + M1.m31*M2.m10 + M1.m32*M2.m20 + M1.m33*M2.m30,
        M1.m30*M2.m01 + M1.m31*M2.m11 + M1.m32*M2.m21 + M1.m33*M2.m31,
        M1.m30*M2.m02 + M1.m31*M2.m12 + M1.m32*M2.m22 + M1.m33*M2.m32,
        M1.m30*M2.m03 + M1.m31*M2.m13 + M1.m32*M2.m23 + M1.m33*M2.m33
    };
    
    return Result;
}

inline void   
Translation(matrix &M, r32 X, r32 Y, r32 Z)
{
    M.m00 = 1; M.m01 = 0; M.m02 = 0; M.m03 = X;
    M.m10 = 0; M.m11 = 1; M.m12 = 0; M.m13 = Y;
    M.m20 = 0; M.m21 = 0; M.m22 = 1; M.m23 = Z;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
}

inline void   
Translation(matrix &M, v3 V)
{
    Translation(M, V.x, V.y, V.z);
}

inline matrix
Translation(v3 V)
{
    matrix M = {};
    Translation(M, V.x, V.y, V.z);
    return M;
}

inline matrix
Translation(r32 X, r32 Y, r32 Z)
{
    matrix M = {};
    Translation(M, X, Y, Z);
    return M;
}

inline void   
RotateAroundX(matrix &M, r32 Angle)
{
    M.m00 = 1; M.m01 = 0; M.m02 = 0; M.m03 = 0;
    M.m10 = 0;                       M.m13 = 0;
    M.m20 = 0;                       M.m23 = 0;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
    
    M.m11 = M.m22 = (r32)cos(Angle);
    M.m21 = (r32)sin(Angle);
    M.m12 = -M.m21;
}

inline void   
RotateAroundY(matrix &M, r32 Angle)
{
    
    M.m01 = 0;                       M.m03 = 0;
    M.m10 = 0; M.m11 = 1; M.m12 = 0; M.m13 = 0;
    M.m21 = 0;                       M.m23 = 0;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
    
    M.m00 = M.m22 = (r32)cos(Angle);
    M.m02 = (r32)sin(Angle);
    M.m20 = -M.m02;
}

inline void   
RotateAroundZ(matrix &M, r32 Angle)
{
    M.m02 = 0; M.m03 = 0;
    M.m12 = 0; M.m13 = 0;
    M.m20 = 0; M.m21 = 0; M.m22 = 1; M.m23 = 0;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
    
    M.m00 = M.m11 = (r32)cos(Angle);
    M.m10 = (r32)sin(Angle);
    M.m01 = -M.m10;
}

inline matrix
RotateAroundX(r32 Angle)
{
    matrix M = {};
    M.m00 = 1; M.m01 = 0; M.m02 = 0; M.m03 = 0;
    M.m10 = 0;                       M.m13 = 0;
    M.m20 = 0;                       M.m23 = 0;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
    
    M.m11 = M.m22 = (r32)cos(Angle);
    M.m21 = (r32)sin(Angle);
    M.m12 = -M.m21;
    return M;
}

inline matrix
RotateAroundY(r32 Angle)
{
    matrix M = {};
    M.m01 = 0;                       M.m03 = 0;
    M.m10 = 0; M.m11 = 1; M.m12 = 0; M.m13 = 0;
    M.m21 = 0;                       M.m23 = 0;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
    
    M.m00 = M.m22 = (r32)cos(Angle);
    M.m02 = (r32)sin(Angle);
    M.m20 = -M.m02;
    return M;
}

inline matrix
RotateAroundZ(r32 Angle)
{
    matrix M = {};
    M.m02 = 0; M.m03 = 0;
    M.m12 = 0; M.m13 = 0;
    M.m20 = 0; M.m21 = 0; M.m22 = 1; M.m23 = 0;
    M.m30 = 0; M.m31 = 0; M.m32 = 0; M.m33 = 1;
    
    M.m00 = M.m11 = (r32)cos(Angle);
    M.m10 = (r32)sin(Angle);
    M.m01 = -M.m10;
    return M;
}

inline void   
RotateYawPitchRoll(matrix &M, r32 Yaw, r32 Pitch, r32 Roll)
{
    r32 CosX = (r32)cos(Pitch);
    r32 CosY = (r32)cos(Yaw);
    r32 CosZ = (r32)cos(Roll);
    
    r32 SinX = (r32)sin(Pitch);
    r32 SinY = (r32)sin(Yaw);
    r32 SinZ = (r32)sin(Roll);
    
    M.m00 = CosZ*CosY + SinZ*SinX*SinY;
    M.m10 = SinZ*CosX;
    M.m20 = -CosZ*SinY + SinZ*SinX*CosY;
    M.m30 = 0;
    
    M.m01 = -SinZ*CosY + CosZ*SinX*SinY;
    M.m11 = CosZ*CosX;
    M.m21 = SinZ*SinY + CosZ*SinX*CosY;
    M.m31 = 0;
    
    M.m02 = CosX*SinY;
    M.m12 = -SinX;
    M.m22 = CosX*CosY;
    M.m32 = 0;
    
    M.m03 = M.m13 = M.m23 = 0;
    M.m33 = 1;
}

inline void   
RotateYawPitchRoll(matrix &M, v3 Angles)
{
    RotateYawPitchRoll(M, Angles.x, Angles.y, Angles.z);
}

inline void   
RotateAroundAxis(matrix &M, v3 Axis, r32 Angle)
{
    r32 Si   = (r32)sin(Angle);
    r32 Co   = (r32)cos(Angle);
    r32 OMCo = 1 - Co;
    v3 Ax       = Axis;
    Ax = Normalize(Ax);
    
    M.m00 = (Ax.x*Ax.x)*OMCo + Co;
    M.m01 = (Ax.x*Ax.y)*OMCo - (Ax.z*Si);
    M.m02 = (Ax.x*Ax.z)*OMCo + (Ax.y*Si);
    M.m03 = 0;
    
    M.m10 = (Ax.y*Ax.x)*OMCo + (Ax.z*Si);
    M.m11 = (Ax.y*Ax.y)*OMCo + Co;
    M.m12 = (Ax.y*Ax.z)*OMCo - (Ax.x*Si);
    M.m13 = 0;
    
    M.m20 = (Ax.z*Ax.x)*OMCo - (Ax.y*Si);
    M.m21 = (Ax.z*Ax.y)*OMCo + (Ax.x*Si);
    M.m22 = (Ax.z*Ax.z)*OMCo + Co;
    M.m23 = 0;
    
    M.m30 = 0;
    M.m31 = 0;
    M.m32 = 0;
    M.m33 = 1;
}

inline void   
Scale(matrix &M, r32 SX, r32 SY, r32 SZ)
{
    M.m00 = SX; M.m01 = 0;  M.m02 = 0;  M.m03 = 0;
    M.m10 = 0;  M.m11 = SY; M.m12 = 0;  M.m13 = 0;
    M.m20 = 0;  M.m21 = 0;  M.m22 = SZ; M.m23 = 0;
    M.m30 = 0;  M.m31 = 0;  M.m32 = 0;  M.m33 = 1;
}

inline void  
Scale(matrix &M, v3 Scalings)
{
    Scale(M, Scalings.x, Scalings.y, Scalings.z);
}

inline void 
Scale(matrix &M, r32 Scaler)
{
    Scale(M, Scaler, Scaler, Scaler);
}


inline matrix
Scale(r32 Scaler)
{
    matrix M = {};
    Scale(M, Scaler, Scaler, Scaler);
    return M;
}


inline matrix
Scale(v3 Scalings)
{
    matrix M = {};
    Scale(M, Scalings.x, Scalings.y, Scalings.z);
    return M;
}

inline matrix 
Scale(r32 SX, r32 SY, r32 SZ)
{
    matrix M = {};
    Scale(M, SX, SY, SZ);
    return M;
}

inline void
SetScale(m4 *M, v3 Scale)
{
    //Assert(M->m01 == 0);
    M->m00 = Scale.x;
    M->m11 = Scale.y;
    M->m22 = Scale.z;
}

inline void
SetScale(m4 *M, r32 S)
{
    M->m00 = S;
    M->m11 = S;
    M->m22 = S;
}

inline v3
GetScale(m4 *M)
{
    v3 Result = {M->m00, M->m11, M->m22};
    return Result;
}

inline void
ScaleMul(m4 *M, r32 S)
{
    M->m00 *= S;
    M->m11 *= S;
    M->m22 *= S;
}

inline void
ScaleDiv(m4 *M, r32 S)
{
    M->m00 /= S;
    M->m11 /= S;
    M->m22 /= S;
}


inline void 
LookAt(matrix *M, v3 Target, v3 Up, v3 Pos)
{
    v3 F = Target - Pos;
    F    = Normalize(F);
    v3 U = Up;
    U    = Normalize(U);
    v3 R = Cross(F, U);
    R    = Normalize(R);
    U    = Cross(R, F);
    M->m00 = R.x;  M->m01 = R.y;  M->m02 = R.z;  M->m03 = -(Dot(R, Pos));
    M->m10 = U.x;  M->m11 = U.y;  M->m12 = U.z;  M->m13 = -(Dot(U, Pos));
    M->m20 = -F.x; M->m21 = -F.y; M->m22 = -F.z; M->m23 =  (Dot(F, Pos));
    M->m30 = 0;    M->m31 = 0;    M->m32 = 0;    M->m33 = 1;
}

inline r32 
GetRotationAngle(matrix &M)
{
    r32 Result = 0.0f;
    
    Result = ((M.m00 + M.m11 + M.m22 + M.m33) - 1)*0.5f;
    if(Result > 1 || Result < -1)
    {
        Result = 0.0f;
    }
    
    return (r32)acos(Result);
}

inline v3   
GetRotationAxis(matrix &M)
{
    v3 Result = {1, 1, 1};
    
    r32 Tmp = (r32)sqrt(Square(M.m21 - M.m12) + 
                        Square(M.m02 - M.m20) + 
                        Square(M.m10 - M.m01));
    if(Tmp > 0)
    {
        Result.x = (M.m21 - M.m12)/Tmp;
        Result.y = (M.m02 - M.m20)/Tmp;
        Result.z = (M.m10 - M.m01)/Tmp;
    }
    
    return Result;
}

inline v3     
GetTranslation(matrix *M)
{
    v3 Result = {};
    
    Result.x = M->m03;
    Result.y = M->m13;
    Result.z = M->m23;
    
    return Result;
}

inline void   
SetTranslation(matrix *M, v3 T)
{
    M->m03 = T.x;
    M->m13 = T.y;
    M->m23 = T.z;
}

// Special orhtogonal matirx transformations
inline v3   
Left(matrix &M)
{
    v3 Result = {-M.m00, -M.m10, -M.m20};
    
    return Result;
}

inline v3  
Right(matrix &M)
{
    v3 Result = {M.m00, M.m10, M.m20};
    
    return Result;
}

inline v3 
Up(matrix &M)
{
    v3 Result = {M.m01, M.m11, M.m21};
    
    return Result;
}

inline v3  
Down(matrix &M)
{
    v3 Result = {-M.m01, -M.m11, -M.m21};
    
    return Result;
}

inline v3 
Forward(matrix &M)
{
    v3 Result = {M.m02, M.m12, M.m22};
    
    return Result;
}

inline v3  
Backward(matrix &M)
{
    v3 Result = {-M.m02, -M.m12, -M.m22};
    
    return Result;
}

inline v3  
Translation(matrix &M)
{
    v3 Result = {M.m03, M.m13, M.m23};
    return Result;
}

inline v3  
Scaling(matrix M)
{
    v3 Result = {M.m00, M.m11, M.m22};
    
    return Result;
}

inline void 
Right(matrix &M, v3 Up)
{
    M.m00 = Up.x;
    M.m10 = Up.y;
    M.m20 = Up.z;
}

inline void 
Up(matrix &M, v3 Up)
{
    M.m01 = Up.x;
    M.m11 = Up.y;
    M.m21 = Up.z;
}

inline void  
Forward(matrix &M, v3 Up)
{
    M.m02 = Up.x;
    M.m12 = Up.y;
    M.m22 = Up.z;
}

inline b32 
IsIdentity(m4 *M)
{
    b32 Result = (M->m00 == 1 && M->m01 == 0 && M->m02 == 0 && M->m03 == 0 &&
                  M->m10 == 0 && M->m11 == 1 && M->m12 == 0 && M->m13 == 0 &&
                  M->m20 == 0 && M->m21 == 0 && M->m22 == 1 && M->m23 == 0 &&
                  M->m30 == 0 && M->m31 == 0 && M->m32 == 0 && M->m33 == 1);
    
    return Result;
}


// projective matrix transformations
inline void 
Perspective(matrix *M, r32 FOVY, r32 AspectRatio, 
            r32 NearPlane, r32 FarPlane) // FOVY = FOV for Y

{
    Assert(NearPlane < FarPlane);
    
    r32 F = 1.0f/tanf(DegToRad(FOVY)*0.5f);
    r32 NearMinusFar = NearPlane-FarPlane;
    
    M->m01 = M->m02 = M->m03 = 0;
    M->m10 = M->m12 = M->m13 = 0;
    M->m20 = M->m21 = 0;
    M->m30 = M->m31 = M->m33 = 0;
    M->m32 = -1;
    
    M->m00 = F / AspectRatio;
    M->m11 = F;
    M->m22 = (FarPlane+NearPlane)/NearMinusFar;
    M->m23 = 2.0f*FarPlane*NearPlane/NearMinusFar;
}

inline void 
Orthographic(matrix &M, r32 Width, r32 Height, r32 Near, r32 Far)
{
    r32 FMN = 1.0f/(Far-Near);
    M.m00 = 2.0f/Width;   M.m01 = 0.0f;         M.m02 = 0.0f;      M.m03 = 0.0f;
    M.m10 = 0.0f;         M.m11 = 2.0f/Height;  M.m12 = 0.0f;      M.m13 = 0.0f;
    M.m20 = 0.0f;         M.m21 = 0.0f;         M.m22 = -2.0f*FMN; M.m23 = -(Far+Near)*FMN;
    M.m30 = 0.0f;         M.m31 = 0.0f;         M.m32 = 0.0f;      M.m33 = 1.0f;
}

// Linear algrebra

internal v3
RayPlaneIntersection(ray Ray, plane Plane)
{
    v3 Result = Plane.Point;
    
    Ray.Direction = Normalize(Ray.Direction);
    
    r32 DNU = Dot(Plane.Normal, Ray.Direction);
    if (DNU != 0.0f)
    {
        r32 s = Dot(Plane.Normal, Plane.Point - Ray.Origin)/DNU;
        Result = Ray.Origin + s*Ray.Direction;
    }
    
    return Result;
}

inline r32
RayOriginPlaneDistance(ray Ray, plane Plane)
{
    r32 DNU = Dot(Plane.Normal, Normalize(Ray.Direction));
    r32 Result = DNU ? Dot(Plane.Normal, Plane.Point - Ray.Origin)/DNU : 0.0f;
    
    return Result;
}

inline r32
PointPlaneDistance(v3 Point, plane Plane)
{
    r32 Result = -Dot(Plane.Normal, Plane.Point - Point);
    return Result;
}

inline v3
Reflect(v3 V, v3 N)
{
    v3 Result = V - 2*(Dot(V, N))*N;
    
    return Result;
}

inline v3 
NearestPointFromLineSegmentToP(v3 A, v3 B, v3 P)
{
    v3 Result = {};
    
    v3 V = B - A;
    v3 U = A - P;
    
    r32 T = -(Dot(V, U)/Dot(V, V));
    
    if(T < 0) Result = A;
    else if(T > 1) Result = B;
    else Result = (1-T)*A + T*B - P;
    
    return Result;
}

internal v3 
NearestPointFromTriangleToP(v3 A, v3 B, v3 C, v3 P)
{
    v3 Result = {};
    
    v3 AB = B - A;
    v3 AC = C - A;
    v3 BC = C - B;
    
    r32 SNom   = Dot(P - A, AB);
    r32 SDeNom = Dot(P - B, A - B); 
    
    r32 TNom   = Dot(P - A, AC);
    r32 TDeNom = Dot(P - C, A - C);
    
    if(SNom <= 0.0f && TNom <= 0.0f) Result = A;
    else
    {
        r32 UNom   = Dot(P - B, BC);
        r32 UDeNom = Dot(P - C, B - C);
        
        if     (SDeNom <= 0.0f && UNom <= 0.0f) Result = B;
        else if(TDeNom <= 0.0f && UDeNom <= 0.0f) Result = C;
        else
        {
            v3 N   = Cross(AB, AC);
            r32 VC = Dot(N, Cross(A - P, B - P));
            
            if(VC <= 0.0f && SNom >= 0.0f && SDeNom >= 0.0f) 
                Result = A + SNom/(SNom+SDeNom)*AB;
            else
            {
                r32 VA = Dot(N, Cross(B - P, C - P));
                
                if(VA <= 0.0f && UNom >= 0.0f && UDeNom >= 0.0f) 
                    Result = B + UNom/(UNom+UDeNom)*BC;
                else
                {
                    r32 VB = Dot(N, Cross(C - P, A - P));
                    
                    if(VB <= 0.0f && TNom >= 0.0f && TDeNom >= 0.0f)
                        Result = A + TNom/(TNom+TDeNom)*AC;
                    else
                    {
                        r32 U = VA/(VA+VB+VC);
                        r32 V = VB/(VA+VB+VC);
                        r32 W = 1.0f - U - V;
                        Result = U*A + V*B + W*C;
                    }
                }
            }
        }
    }
    
    return Result;
}