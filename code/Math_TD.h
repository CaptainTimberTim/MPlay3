#pragma once
#include <math.h>

#define Pi32 3.14159265359f

#define MIN_INT16 0x8000
#define MAX_INT16 0x7FFF
#define MIN_UINT16 0x0000
#define MAX_UINT16 0xFFFF

#define MIN_INT32  0x80000000 //2147483648
#define MAX_INT32  0x7FFFFFFF
#define MIN_UINT32 0x00000000
#define MAX_UINT32 0xFFFFFFFF
#define MIN_REAL32 (r32)-3.4E+38
#define MAX_REAL32 (r32)3.4E+38
#define MIN_REAL64 -1.7E+308
#define MAX_REAL64 1.7E+308

// Struct defines ******************

union v2
{
    struct
    {
        r32 x, y;
    };
    r32 E[2];
};

union v2i
{
    struct
    {
        i32 x, y;
    };
    i32 E[2];
};

union v3
{
    struct
    {
        r32 x, y, z;
    };
    
    struct
    {
        r32 r, g, b;
    };
    
    struct
    {
        v2 xy;
        r32 _z;
    };
    struct
    {
        r32 _x;
        v2 yz;
    };
    r32 E[3];
};

union v3i
{
    struct
    {
        i32 x, y, z;
    };
    
    i32 E[3];
};

union v4
{
    struct
    {
        r32 x, y, z, w;
    };
    
    struct
    {
        r32 r, g, b, a;
    };
    
    r32 E[4];
};

typedef v4 color;

union quaternion
{
    struct 
    {
        r32 w, x, y, z;
    };
    struct 
    {
        r32 a, b, c, d;
    };
    struct
    {
        r32 s;
        v3 v;
    };
    r32 E[4];
};

#if 1
union matrix
{
    struct 
    {
        r32 m00, m01, m02, m03;
        r32 m10, m11, m12, m13;
        r32 m20, m21, m22, m23;
        r32 m30, m31, m32, m33;
    };
    r32 E[16];
};
#else
union matrix
{
    struct 
    {
        r32 m00, m10, m20, m30;
        r32 m01, m11, m21, m31;
        r32 m02, m12, m22, m32;
        r32 m03, m13, m23, m33;
    };
    r32 E[16];
};
#endif
typedef matrix m4;


struct ray
{
    v3 Origin;
    v3 Direction;
};

struct plane
{
    v3 Point;
    v3 Normal;
};

// General *********************

inline void MathPrint(i32 A);
inline void MathPrint(r32 A);
inline void MathPrint(v2 A);
inline void MathPrint(v3 A);
inline void MathPrint(v2i A);
inline void MathPrint(v3i A);
inline void MathPrint(v4 A);
inline void MathPrint(quaternion A);
inline void MathPrint(matrix M);
inline void MathPrint(ray R);

inline void MathPrintln(i32 A);
inline void MathPrintln(r32 A);
inline void MathPrintln(v2 A);
inline void MathPrintln(v3 A);
inline void MathPrintln(v2i A);
inline void MathPrintln(v3i A);
inline void MathPrintln(v4 A);
inline void MathPrintln(quaternion A);
inline void MathPrintln(matrix M);
inline void MathPrintln(ray R);

// General math ****************

inline i32 Abs(i32 Value);
inline r32 Abs(r32 Value);
inline r32 Pow(r32 Base, i32 Exponent);
inline void Clamp(i32 *Value, i32 Minimum, i32 Maximum);
inline void Clamp(r32 *Value, r32 Minimum, r32 Maximum);
inline r32  Clamp(r32 Value, r32 Minimum, r32 Maximum);
inline r32  Clamp01(r32 Value);
inline i32  Clamp(i32 Value, i32 Minimum, i32 Maximum);
inline i32 Floor(r32 Value);
inline i32 Ceiling(r32 Value);
inline r32 FloorR32(r32 Value);
inline r32 CeilingR32(r32 Value);
inline i32 RoundReal32ToInt32(r32 Value);
inline u32 RoundReal32ToUInt32(r32 Value);
inline r32 SafeRatioN(r32 Numerator, r32 Divisor, r32 N);
inline r32 SafeRatio1(r32 Numerator, r32 Divisor);
inline u32 SafeTruncateUInt64(u64 Value);
inline r32 Square(r32 A);
inline r32 DegToRad(r32 Degree);
inline r32 RadToDeg(r32 Radian);
inline r32 Lerp(r32 A, r32 B, r32 T);
inline r32 Lerp(i32 A, i32 B, r32 T);
inline i32 Max(i32 A, i32 B);
inline r32 Max(r32 A, r32 B);
inline u32 Max(u32 A, u32 B);
inline u64 Max(u64 A, u64 B);
inline i32 Min(i32 A, i32 B);
inline r32 Min(r32 A, r32 B);
inline r32 Mod(r32 V, r32 M);
inline r64 Mod(r64 V, r64 M);

inline r32 SafeDiv(r32 A, r32 B);
inline i32 SafeDiv(i32 A, i32 B);
// Vector Math *****************

// 2D Vectors
inline v2 V2(r32 X, r32 Y);
inline v2 V2(r32 X);
inline v2 V2(v2i X);
inline v2 operator+  (v2 A, v2 B);
inline v2 operator+  (v2 A, r32 B);
inline v2 operator+  (r32 A, v2 B);
inline v2 &operator+=(v2 &A, v2 B);
inline v2 operator-  (v2 A);
inline v2 operator-  (v2 A, v2 B);
inline v2 operator-  (v2 A, r32 B);
inline v2 operator-  (r32 A, v2 B);
inline v2 &operator-=(v2 &A, v2 &B);
inline v2 operator*  (v2 A, r32 B);
inline v2 operator*  (r32 A, v2 B);
inline v2 &operator*=(v2 &A, r32 B);
inline v2 operator/  (v2 A, r32 B);
inline v2 &operator/=(v2 &A, r32 &B);
inline r32 Dot(v2 A, v2 B);
inline r32 Cross(v2 A, v2 B);
inline r32 LengthSquared(v2 A);
inline r32 Length(v2 A);
inline r32 Distance(v2 A, v2 B);
inline v2 Normalize(v2 A);
inline v2 HadamardProduct(v2 A, v2 B);
inline v2 HadamardDivision(v2 A, v2 B);

// 2D integer Vectors
inline v2i V2i(i32 X, i32 Y);
inline v2i V2i(i32 X);
inline v2i operator+  (v2i A, v2i B);
inline v2i operator+  (v2i A, i32 B);
inline v2i operator+  (i32 A, v2i B);
inline v2i &operator+=(v2i &A, v2i &B);
inline v2i operator-  (v2i A);
inline v2i operator-  (v2i A, v2i B);
inline v2i operator-  (v2i A, i32 B);
inline v2i operator-  (i32 A, v2i B);
inline v2i &operator-=(v2i &A, v2i &B);
inline v2i operator*  (v2i A, i32 B);
inline v2i operator*  (i32 A, v2i B);
inline v2i &operator*=(v2i &A, i32 B);
inline v2i operator/  (v2i A, i32 B);
inline v2i &operator/=(v2i &A, i32 &B);
inline v2  operator/  (v2i A, r32 B);
inline i32 Dot(v2i A, v2i B);
inline i32 Cross(v2i A, v2i B);
inline i32 LengthSquared(v2i A);
inline r32 Length(v2i A);
inline r32 Distance(v2i A, v2i B);
inline v2 Normalize(v2i A);
inline v2i HadamardProduct(v2i A, v2i B);
inline v2i HadamardDivision(v2i A, v2i B);
inline v2 HadamardProduct(v2 A, v2i B);
inline v2 HadamardDivision(v2 A, v2i B);
inline v2 HadamardDivision(v2i A, v2 B);

// 3D Vectors
inline v3 V3(r32 X, r32 Y, r32 Z);
inline v3 V3(r32 V);
inline v3 operator+  (v3 A, v3 B);
inline v3 operator+  (v3 A, r32 B);
inline v3 operator+  (r32 A, v3 B);
inline v3 &operator+=(v3 &A, v3 &B);
inline v3 operator-  (v3 A);
inline v3 operator-  (v3 A, v3 B);
inline v3 operator-  (v3 A, r32 B);
inline v3 operator-  (r32 A, v3 B);
inline v3 &operator-=(v3 &A, v3 &B);
inline v3 operator*  (v3 A, r32 B);
inline v3 operator*  (r32 A, v3 B);
inline v3 &operator*=(v3 &A, r32 B);
inline v3 operator/  (v3 A, r32 B);
inline v3 &operator/=(v3 &A, r32 &B);
inline b32 operator!=(v3 A, v3 B);
inline r32 Dot(v3 A, v3 B);
inline v3 Cross(v3 A, v3 B);
inline r32 LengthSquared(v3 A);
inline r32 Length(v3 A);
inline r32 Distance(v3 A, v3 B);
inline v3 HadamardProduct(v3 A, v3 B);
inline v3 HadamardDivision(v3 A, v3 B);
inline v3 Normalize(v3 A);
inline u32 Vector3ToUInt32(v3 Value);
inline r32 AngleBetween(v3 A, v3 B);
inline v3  AxisBetween(v3 A, v3 B);
inline void Zero(v3 *V);
inline v3 TripleCross(v3 A, v3 B, v3 C);
inline v3 Clamp01(v3 V);

// 3D integer Vectors
inline v3 V3i(u32 X, u32 Y, u32 Z);
inline v3i V3i(v3 Vec);
inline v3 operator+  (v3 A, v3 B);
inline v3 operator+  (v3 A, r32 B);
inline v3 operator+  (r32 A, v3 B);
inline v3 &operator+=(v3 &A, v3 &B);
inline v3 operator-  (v3 A);
inline v3 operator-  (v3 A, v3 B);
inline v3 operator-  (v3 A, r32 B);
inline v3 operator-  (r32 A, v3 B);
inline v3 &operator-=(v3 &A, v3 &B);
inline v3 operator*  (v3 A, r32 B);
inline v3 operator*  (r32 A, v3 B);
inline v3 &operator*=(v3 &A, r32 B);
inline v3 operator/  (v3 A, r32 B);
inline v3 &operator/=(v3 &A, r32 &B);

// 4D Vectors
inline v4 V4(r32 W, r32 X, r32 Y, r32 Z);
inline v4 operator+  (v4 A, v4 B);
inline v4 operator+  (v4 A, r32 B);
inline v4 operator+  (r32 A, v4 B);
inline v4 &operator+=(v4 &A, v4 &B);
inline v4 operator-  (v4 A);
inline v4 operator-  (v4 A, v4 B);
inline v4 operator-  (v4 A, r32 B);
inline v4 operator-  (r32 A, v4 B);
inline v4 &operator-=(v4 &A, v4 &B);
inline v4 operator*  (v4 A, r32 B);
inline v4 operator*  (r32 A, v4 B);
inline v4 &operator*=(v4 &A, r32 B);
inline v4 operator/  (v4 A, r32 B);
inline v4 &operator/=(v4 &A, r32 &B);
inline r32 Dot(v4 A, v4 B);
inline v4 Cross(v4 A, v4 B);
inline r32 LengthSquared(v4 A);
inline r32 Length(v4 A);
inline r32 Distance(v4 A, v4 B);
inline v4 Normalize(v4 A);
inline v4 HadamardProduct(v4 A, v4 B);

// Quaternion Math ****************

inline quaternion Quaternion ();
inline quaternion Quaternion (r32 s, r32 x, r32 y, r32 z);
inline quaternion operator+  (quaternion  Q1, quaternion Q2);
inline quaternion &operator+=(quaternion &Q1, quaternion &Q2);
inline quaternion operator-  (quaternion  Q1, quaternion Q2);
inline quaternion &operator-=(quaternion &Q1, quaternion &Q2);
inline quaternion operator*  (quaternion  Q1, quaternion Q2);
inline quaternion &operator*=(quaternion &Q1, quaternion &Q2);
inline v3         operator*  (quaternion  Q, v3 P);
inline quaternion  operator*  (r32 S, quaternion  Q);
inline quaternion operator/  (quaternion Q1, r32 R);
inline quaternion &operator/=(quaternion &Q1, r32 &R);
inline quaternion Conjugate(quaternion Q);
inline r32        Norm(quaternion Q);
inline r32        Length(quaternion Q);
inline quaternion Normalize(quaternion Q);
inline void       Normalize(quaternion *Q);
inline quaternion Inverse(quaternion Q);
inline quaternion EulerToQuaternion(r32 Pitch, r32 Roll, r32 Yaw);
inline quaternion EulerToQuaternion(v3 E);
inline v3         QuaternionToEuler(quaternion Q);
inline v3         QuaternionToEulerDeg(quaternion Q);
inline m4         QuaternionToMatrix(quaternion Q);
inline quaternion MatrixToQuaternion(m4 M);


// Matrix Math ********************
#define Identity() { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }

inline matrix Matrix();
inline matrix Matrix(r32 m00, r32 m01, r32 m02, r32 m03, 
                     r32 m10, r32 m11, r32 m12, r32 m13, 
                     r32 m20, r32 m21, r32 m22, r32 m23, 
                     r32 m30, r32 m31, r32 m32, r32 m33);
inline matrix operator*(matrix &M1, matrix &M2);
inline matrix &operator*=(matrix &M1, matrix &M2);
inline v4     operator*(matrix &M, v4 V);
inline v3     operator*(matrix &M, v3 V);
//inline matrix Identity();
inline void   Transpose(matrix *M);
inline void   Invert(matrix &M);
inline v3     TransformV3x4(matrix &M, v3 V);
inline v3     TransformV3x3(matrix &M, v3 V);
inline v4     TransformV4x4(matrix &M, v4 V);
inline r32 Determinant(matrix &M);

// Affine matrix transformations
inline matrix Multiply(matrix &M1, matrix &M2);
inline void   Translation(matrix &M, r32 X, r32 Y, r32 Z);
inline void   Translation(matrix &M, v3 V);
inline matrix Translation(v3 V);
inline matrix Translation(r32 X, r32 Y, r32 Z);
inline void   RotateAroundX(matrix &M, r32 Angle);
inline void   RotateAroundY(matrix &M, r32 Angle);
inline void   RotateAroundZ(matrix &M, r32 Angle);
inline matrix RotateAroundX(r32 Angle);
inline matrix RotateAroundY(r32 Angle);
inline matrix RotateAroundZ(r32 Angle);
inline void   RotateYawPitchRoll(matrix &M, r32 Yaw, r32 Pitch, r32 Roll);
inline void   RotateYawPitchRoll(matrix &M, v3 Angles);
inline void   RotateAroundAxis(matrix &M, v3 Axis, r32 Angle);
inline void   Scale(matrix &M, r32 SX, r32 SY, r32 SZ);
inline void   Scale(matrix &M, v3 Scalings);
inline void   Scale(matrix &M, r32 Scaler);
inline matrix Scale(r32 Scaler);
inline matrix Scale(v3 Scalings);
inline matrix Scale(r32 SX, r32 SY, r32 SZ);
inline void   ScaleMul(m4 *M, r32 S);
inline void   ScaleDiv(m4 *M, r32 S);

inline void   LookAt(matrix &M, v3 Target, v3 Up, v3 Pos);

inline void   SetScale(m4 *M, r32 S);
inline v3     GetScale(m4 *M);
inline r32    GetRotationAngle(matrix &M);
inline v3     GetRotationAxis(matrix &M);
inline void   SetTranslation(matrix *M, v3 T);
inline v3     GetTranslation(matrix *M);

// Special orhtogonal matirx transformations
inline v3     Left(matrix &M);
inline v3     Right(matrix &M);
inline v3     Up(matrix &M);
inline v3     Down(matrix &M);
inline v3     Forward(matrix &M);
inline v3     Backward(matrix &M);
inline v3     Translation(matrix &M);
inline void   Up(matrix &M, v3 Up);
inline void   Right(matrix &M, v3 Up);
inline void   Forward(matrix &M, v3 Up);

inline b32 IsIdentity(m4 *M);

// projective matrix transformations
inline void   Perspective(matrix &M, r32 FOVY, r32 AspectRatio, 
                          r32 NearPlane, r32 FarPlane); // FOVY = FOV for Y
inline void   Orthographic(matrix &M, r32 Width, r32 Height, r32 Near, r32 Far);

// Linear algrebra


internal v3 RayPlaneIntersection(ray Ray, plane Plane);
inline r32 PointPlaneDistance(v3 Point, plane Plane);

inline v3 NearestPointFromLineSegmentToP(v3 A, v3 B, v3 P);