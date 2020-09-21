#pragma once

#include "String_TD.h"


struct read_file_result
{
    u32 Size;
    u8 *Data;
};

// General file methods
internal b32 WriteEntireFile(bucket_allocator *Bucket, u8 *Filename, u32 MemorySize, void *Memory);
internal b32 WriteEntireFile(string_w *Filename, u32 MemorySize, void *Memory);
internal void   FreeFileMemory(memory_bucket_container *BucketContainer, void *Memory);
internal b32 ReadEntireFile(memory_bucket_container *BucketContainer, read_file_result *FileData, u8 *Filename);
internal b32 ReadEntireFile(memory_bucket_container *BucketContainer, read_file_result *FileData, string_w *Filename);
internal b32 ReadBeginningOfFile(memory_bucket_container *Bucket, read_file_result *FileData, u8 *Filename, i32 ReadAmount);
internal b32 ReadBeginningOfFile(memory_bucket_container *Bucket, read_file_result *FileData, string_w *Filename, i32 ReadAmount);
internal b32 AppendToFile(memory_bucket_container *BucketContainer, char *FileName, u32 MemorySize, void *Memory);

// .obj loader methods


struct bit_scan_result
{
    b32 Found;
    u32 Index;
};

enum face_type
{
    NO_VALUE,
    VERTEX,
    VERTEX_TEXCOORD,
    VERTEX_NORMAL,
    VERTEX_TEXCOORD_NORMAL,
};

struct mtl_data
{
    hash_table NameIndex; // Get index for mtl data via the name string
    i32 Count;
    
    v3 *Ka; // Ambient
    v3 *Kd; // Diffuse
    v3 *Ks; // Specular
    v3 *Tf; // Transmission filter
    r32 *Ns; // Spec exponent
    r32 *Ni; // Refraction index
    r32 *Tr; // Transparency
    u32 *Illum; // Illumination model: lookup in list
    u8 *Map_Ka; // Ambient texture map
    u8 *Map_Kd; // Diffuse texture map
    u8 *Map_bump; // Bumpmap
};

struct obj_material_group
{
    //char Name[MAX_PATH];
    u8 MatName[50];
    
    u32 *Indices;
    u32  IndiceCount;
};

struct model_information
{
    v3 MinBound;
    v3 MaxBound;
};

struct obj_data
{
    char Name[MAX_PATH];
    
    v3 *Vertice;
    v2 *TexCoords;
    v3 *Normals;
    u8 *SmoothingValue;
    u32 Count;
    face_type FaceType;
    
    // Gives information if faces have normals, texcoords, both or nothing
    obj_material_group *SurfaceGroups;
    u32 GroupCount;
    
    mtl_data Materials;
    model_information Infos;
};

internal b32 LoadOBJFile(obj_data *Object, u8 *Filename, bucket_allocator *BucketAlloc);
inline   u32 ProcessVectorLine(r32 *Results, u32 VectorDim, u8 *LineChar);
inline   face_type IdentifyFaceType(u8 *Character);
internal u8 *EvaluateNextFaceIndexGroup(hash_table *FaceTable, obj_data *Object, obj_material_group *CurrentGroup, u8 *Character, v3 *Vertice, v2 *TexCoords, v3 *Normals);
internal obj_material_group CollectAllGroupsInOne(obj_data *Object, bucket_allocator *BucketAlloc);
internal b32 LoadMtlFile(mtl_data *Materials, bucket_allocator *BucketAlloc, u8 *Path, u8 *CutPath);

// .bmp loader methods
enum bitmap_color_format
{
    colorFormat_RGBA,
    colorFormat_RGB,
    colorFormat_BGR,
    colorFormat_Alpha,
};

struct loaded_bitmap
{
    b32 WasLoaded;
    u32 Width;
    u32 Height;
    u32 *Pixels;
    bitmap_color_format ColorFormat;
    u32 Pitch; // Width*sizeof(One color (e.g. u32));
};

inline   bit_scan_result FindLeastSignificantSetBit(u32 Value);
internal loaded_bitmap LoadBMPImage(memory_bucket_container *BucketContainer, u8 *FileName);

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    i32 Width;
    i32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    i32 HorzResolution;
    i32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
    
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)
