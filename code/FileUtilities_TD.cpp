#include "FileUtilities_TD.h"

internal b32
WriteEntireFile(bucket_allocator *Bucket, u8 *Filename, u32 MemorySize, void *Memory)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(&Bucket->Transient, Filename, &FileNameWide);
    Result = WriteEntireFile(&FileNameWide, MemorySize, Memory);
    DeleteStringW(&Bucket->Transient, &FileNameWide);
    
    return Result;
}

internal b32
WriteEntireFile(string_w *Filename, u32 MemorySize, void *Memory)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            Result = (BytesWritten == MemorySize);
            //File write successfully
        }
        else
        {
            //Could not write the file
        } 
        CloseHandle(FileHandle);
    }
    else
    {
        //Could not open File
    }
    return(Result);
}

internal void 
FreeFileMemory(memory_bucket_container *BucketContainer, void *Memory)
{
    if(Memory && !BucketContainer->IsFixedBucket)
    {
        PopFromTransientBucket(BucketContainer, Memory);
    }
}

internal b32 
ReadEntireFile(memory_bucket_container *BucketContainer, read_file_result *FileData, u8 *Filename)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(&BucketContainer->Parent->Transient, Filename, &FileNameWide);
    Result = ReadEntireFile(BucketContainer, FileData, &FileNameWide);
    DeleteStringW(&BucketContainer->Parent->Transient, &FileNameWide);
    
    return Result;
}

internal b32 
ReadEntireFile(memory_bucket_container *BucketContainer, read_file_result *FileData, string_w *Filename)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            //FileData->Contents = (char *)VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            FileData->Data = (u8 *)PushSizeOnBucket(BucketContainer, FileSize32+1);
            if (FileData->Data)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, FileData->Data, FileSize32, &BytesRead, 0) &&
                    (FileSize32 == BytesRead))
                {
                    FileData->Size = FileSize32+1;
                    FileData->Data[FileData->Size-1] = 0;
                    Result = true;
                }
                else
                {
                    FreeFileMemory(BucketContainer, FileData->Data);
                    FileData->Data = 0;
                    printf("Could not read the file.\n");
                }
            }
            else
            {
                printf("Could not get Memory for file.\n");
            }
        }
        else
        {
            printf("Could not get file size.\n");
        }
        CloseHandle(FileHandle);
    }
    else
    {
        //printf("Could not open File: \"%s\".\n", Filename);
    }
    return(Result);
}

internal b32 
ReadBeginningOfFile(memory_bucket_container *Bucket, read_file_result *FileData, u8 *Filename, i32 ReadAmount)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(&Bucket->Parent->Transient, Filename, &FileNameWide);
    Result = ReadBeginningOfFile(Bucket, FileData, &FileNameWide, ReadAmount);
    DeleteStringW(&Bucket->Parent->Transient, &FileNameWide);
    
    return Result;
}

internal b32 
ReadBeginningOfFile(memory_bucket_container *Bucket, read_file_result *FileData, string_w *Filename, i32 ReadAmount)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            FileData->Data = (u8 *)PushSizeOnBucket(Bucket, ReadAmount);
            if (FileData->Data)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, FileData->Data, ReadAmount, &BytesRead, 0))
                {
                    FileData->Size = BytesRead;
                    Result = true;
                }
                else
                {
                    FreeFileMemory(Bucket, FileData->Data);
                    FileData->Data = 0;
                    printf("Could not read the file.\n");
                }
            }
            else
            {
                printf("Could not get Memory for file.\n");
            }
        }
        else
        {
            printf("Could not get file size.\n");
        }
        CloseHandle(FileHandle);
    }
    else
    {
        //printf("Could not open File: \"%s\".\n", Filename);
    }
    return(Result);
}

internal b32
AppendToFile(memory_bucket_container *BucketContainer, u8 *FileName, u32 MemorySize, void *Memory)
{
    b32 Result = false;
    read_file_result FileData = {};
    if(ReadEntireFile(BucketContainer, &FileData, FileName))
    {
        MemorySize += FileData.Size;
        char AllData[10000];
        sprintf_s(AllData, "%s\n%s\n\0",FileData.Data, (char *)Memory);
        if(WriteEntireFile(BucketContainer->Parent, FileName, MemorySize, AllData))
        {
            Result = true;
        }
        PopFromTransientBucket(BucketContainer, FileData.Data);
    }
    return Result;
}

inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};
#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    return(Result);
}

internal loaded_bitmap
LoadBMPImage(memory_bucket_container *BucketContainer, u8 *FileName)
{
    loaded_bitmap Result = {};
    Result.WasLoaded = false;
    read_file_result FileData = {};
    if(ReadEntireFile(BucketContainer, &FileData, FileName))
    {
        Result.WasLoaded = true;
        bitmap_header *Header = (bitmap_header *)FileData.Data;
        u32 *Pixels = (u32 *)((u8 *)FileData.Data + Header->BitmapOffset);
        Result.Pixels = Pixels;
        Assert(Header->Height >= 0);
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        if(Header->Compression == 3)
        {
            // NOTE:: Byte order in memory is determined by Header
            // so we have to read out masks and convert the pixels
            u32 RedMask   = Header->RedMask;
            u32 GreenMask = Header->GreenMask;
            u32 BlueMask  = Header->BlueMask;
            u32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
            bit_scan_result RedShift   = FindLeastSignificantSetBit(RedMask);
            bit_scan_result GreenShift = FindLeastSignificantSetBit(GreenMask);
            bit_scan_result BlueShift  = FindLeastSignificantSetBit(BlueMask);
            bit_scan_result AlphaShift = FindLeastSignificantSetBit(AlphaMask);
            Assert(RedShift.Found);
            Assert(GreenShift.Found);
            Assert(BlueShift.Found);
            Assert(AlphaShift.Found);
            u32 *SourceDest = Pixels;
            for(i32 Y = 0; Y < Header->Height; ++Y)
            {
                for(i32 X = 0; X < Header->Width; ++X)
                {
                    u32 C = *SourceDest;
#if 0
                    *SourceDest++ = ((((C >>   RedShift.Index) & 0xFF) << 24) | 
                                     (((C >> GreenShift.Index) & 0xFF) << 16) | 
                                     (((C >>  BlueShift.Index) & 0xFF) <<  8) |
                                     (((C >> AlphaShift.Index) & 0xFF) <<  0));
#else
                    *SourceDest++ = ((((C >> AlphaShift.Index) & 0xFF) << 24) | 
                                     (((C >>  BlueShift.Index) & 0xFF) << 16) | 
                                     (((C >>   RedShift.Index) & 0xFF) <<  8) |
                                     (((C >> GreenShift.Index) & 0xFF) <<  0));
#endif
                }
            }
            Result.ColorFormat = colorFormat_RGBA;
        }
        else if(Header->Compression == 0)
        {
            if(Header->BitsPerPixel == 24)
            {
                Result.ColorFormat = colorFormat_BGR;
            }
        }
        else
        {
            Assert(Header->Compression == -1);
        }
    }
    return Result;
}

inline u32 
ProcessVectorLine(r32 *Results, u32 VectorDim, u8 *LineChar)
{
    u32 EndOfLineCount = 0;
    for(u32 XYZCount = 0; XYZCount < VectorDim; ++XYZCount)
    {
        r32 *NewVal = Results + XYZCount;
        u8 NumberLength = 0;
        *NewVal = ProcessNextR32InString(LineChar, ' ', NumberLength);
        LineChar += NumberLength;
        EndOfLineCount += NumberLength;
    }
    return EndOfLineCount;
}

inline face_type
IdentifyFaceType(u8 *Character)
{
    face_type Result = NO_VALUE;
    u8 AdvanceChar = 0;
    i32 Position = FirstOccurrenceOfCharacterInString('/', Character, '\n');
    if(Position == -1)
    {
        Result = VERTEX; // Only vertice
    }
    else
    {
        ProcessNextU32InString(Character, '/', AdvanceChar);
        Character += AdvanceChar + 1;
        if(*Character == '/')
        {
            Result = VERTEX_NORMAL; // Vertice + normal
        }
        else
        {
            ProcessNextU32InString(Character, ' ', AdvanceChar);
            u8 CheckAdvance = 0;
            ProcessNextU32InString(Character, '/', CheckAdvance);
            if(AdvanceChar < CheckAdvance)
            {
                Result = VERTEX_TEXCOORD; // Vertice + texcoord
            }
            else
            {
                Result = VERTEX_TEXCOORD_NORMAL; // Vertice + texcoord + normal
            }
        }
    }
    return Result;
}

internal void
EvaluateNextFaceWithNormal(hash_table *FaceTable, obj_data *Object, obj_material_group *CurrentGroup, u8 *Character, v3 *Vertice, v2 *TexCoords, v3 *Normals)
{
    u32 NextVertI[4];
    u32 NextTexI[4];
    u8 *KeyPosition[4];
    u8 KeyLength[4];
    u8 Counter = 0;
    u8 AdvanceChar = 0;
    for(u32 Index = 0; Index < 4; ++Index)
    {
        Counter++;
        KeyPosition[Index] = Character;
        KeyLength[Index]   = (u8)StringLengthUntilChar(Character, ' ');
        Assert(KeyLength[Index] < 15); // NOTE:: obj to big for KeyLength. Need to adjust!
        switch(Object->FaceType)
        {
            case VERTEX:
            {
                NextVertI[Index] = ProcessNextU32InString(Character, ' ', AdvanceChar)-1;
                Character += AdvanceChar;
            } break;
            
            case VERTEX_TEXCOORD:
            {
                NextVertI[Index] = ProcessNextU32InString(Character, '/', AdvanceChar)-1;
                Character += AdvanceChar + 1;
                NextTexI[Index] = ProcessNextU32InString(Character, ' ', AdvanceChar)-1;
                Character += AdvanceChar;
            } break;
        }
        Character = AdvanceAfterConsecutiveGivenChar(Character, ' ');
        if(*Character == '\n' && Index == 2) { break; }
    }
    v3 P0 = *(Vertice + NextVertI[0]);
    v3 P1 = *(Vertice + NextVertI[1]);
    v3 P2 = *(Vertice + NextVertI[2]);
    v3 Normal = Normalize(Cross(P1 - P0, P2 - P0));
    u8 NormalKey[15];
    ConvertV3To15Char(Normal, NormalKey);
    for(u8 Index = 0; Index < Counter; ++Index)
    {
        if(Index == 3)
        {
            *(CurrentGroup->Indices + CurrentGroup->IndiceCount) = *(CurrentGroup->Indices + (CurrentGroup->IndiceCount - 3));
            *(CurrentGroup->Indices + (CurrentGroup->IndiceCount + 1)) = *(CurrentGroup->Indices + (CurrentGroup->IndiceCount - 1));
            CurrentGroup->IndiceCount += 2;
        }
        u8 Key[HASH_TABLE_KEY_LENGTH];
        CopyString(Key, KeyPosition[Index], KeyLength[Index]);
        CombineStrings(Key, Key, (u8 *)"/");
        CombineStrings(Key, Key, NormalKey);
        //sprintf_s((char *)Key, HASH_TABLE_KEY_LENGTH, "%s/%s", Key, NormalKey);
        switch(Object->FaceType)
        {
            case VERTEX:
            {
                if(AddToHashTable(FaceTable, Key, Object->Count))
                {
                    *(Object->Vertice + Object->Count) = *(Vertice + NextVertI[Index]);
                    *(Object->Normals + Object->Count) = Normal;
                    *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = Object->Count++;
                }
                else
                {
                    u32 FaceIndex;
                    GetFromHashTable(FaceTable, Key, FaceIndex);
                    *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = FaceIndex;
                }
            } break;
            
            case VERTEX_TEXCOORD:
            {
                if(AddToHashTable(FaceTable, Key, Object->Count))
                {
                    *(Object->Vertice + Object->Count)   = *(Vertice + NextVertI[Index]);
                    *(Object->TexCoords + Object->Count) = *(TexCoords + NextTexI[Index]);
                    *(Object->Normals + Object->Count)   = Normal;
                    *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = Object->Count++;
                }
                else
                {
                    u32 FaceIndex;
                    GetFromHashTable(FaceTable, Key, FaceIndex);
                    *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = FaceIndex;
                }
            } break;
        }
    }
}

internal u8 *
EvaluateNextFaceIndexGroup(hash_table *FaceTable, obj_data *Object, obj_material_group *CurrentGroup, u8 *Character, v3 *Vertice, v2 *TexCoords, v3 *Normals)
{
    u8 AdvanceChar = 0;
    switch(Object->FaceType)
    {
        case VERTEX:
        {
            u32 VertexIndex = ProcessNextU32InString(Character, ' ', AdvanceChar)-1;
            Character += AdvanceChar;
            *(Object->Vertice + Object->Count) = *(Vertice + VertexIndex);
            *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = Object->Count++;
        } 
        break;
        case VERTEX_TEXCOORD:
        {
            u32 KeyLength  = StringLengthUntilChar(Character, ' ');
            u8 Key[HASH_TABLE_KEY_LENGTH];
            CopyString(Key, Character, KeyLength);
            if(AddToHashTable(FaceTable, Key, Object->Count))
            {
                u32 VertexIndex = ProcessNextU32InString(Character, '/', AdvanceChar)-1;
                Character += AdvanceChar + 1;
                u32 TexCoordIndex = ProcessNextU32InString(Character, ' ', AdvanceChar)-1;
                Character += AdvanceChar;
                *(Object->Vertice   + Object->Count) = *(Vertice   + VertexIndex);
                *(Object->TexCoords + Object->Count) = *(TexCoords + TexCoordIndex);
                *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = Object->Count++;
            }
            else
            {
                Character += KeyLength;
                u32 FaceIndex;
                GetFromHashTable(FaceTable, Key, FaceIndex);
                *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = FaceIndex;
            }
        } 
        break;
        case VERTEX_NORMAL:
        {
            u32 KeyLength  = StringLengthUntilChar(Character, ' ');
            u8 Key[HASH_TABLE_KEY_LENGTH];
            CopyString(Key, Character, KeyLength);
            if(AddToHashTable(FaceTable, Key, Object->Count))
            {
                u32 VertexIndex = ProcessNextU32InString(Character, '/', AdvanceChar)-1;
                Character += AdvanceChar + 2;
                u32 NormalIndex = ProcessNextU32InString(Character, ' ', AdvanceChar)-1;
                Character += AdvanceChar;
                *(Object->Vertice + Object->Count) = *(Vertice + VertexIndex);
                *(Object->Normals + Object->Count) = *(Normals + NormalIndex);
                *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = Object->Count++;
            }
            else
            {
                Character += KeyLength;
                u32 FaceIndex;
                GetFromHashTable(FaceTable, Key, FaceIndex);
                *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = FaceIndex;
            }
        }
        break;
        case VERTEX_TEXCOORD_NORMAL:
        {
            u32 KeyLength  = StringLengthUntilChar(Character, ' ');
            u8 Key[HASH_TABLE_KEY_LENGTH];
            CopyString(Key, Character, KeyLength);
            if(AddToHashTable(FaceTable, Key, Object->Count))
            {
                u32 VertexIndex = ProcessNextU32InString(Character, '/', AdvanceChar)-1;
                Character += AdvanceChar + 1;
                u32 TexCoordIndex = ProcessNextU32InString(Character, '/', AdvanceChar)-1;
                Character += AdvanceChar + 1;
                u32 NormalIndex = ProcessNextU32InString(Character, ' ', AdvanceChar)-1;
                Character += AdvanceChar;
                *(Object->Vertice   + Object->Count) = *(Vertice   + VertexIndex);
                *(Object->TexCoords + Object->Count) = *(TexCoords + TexCoordIndex);
                *(Object->Normals   + Object->Count) = *(Normals   + NormalIndex);
                *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = Object->Count++;
            }
            else
            {
                Character += KeyLength;
                u32 FaceIndex;
                GetFromHashTable(FaceTable, Key, FaceIndex);
                *(CurrentGroup->Indices + CurrentGroup->IndiceCount++) = FaceIndex;
            }
        } 
        break;
    }
    return Character;
}

internal b32
LoadOBJFile(obj_data *Object, u8 *Filename, bucket_allocator *BucketAlloc)
{
    // TODO(TD): This does not yet really work with the new bucket allocator.
    // could use some work!
    b32 Result = false;
    read_file_result File = {};
    if(ReadEntireFile(&BucketAlloc->Transient, &File, Filename))
    {
        u8 MaterialPath[MAX_PATH];
        u8 CurrentSurfaceName[MAX_PATH];
        u8 *TMPName = (u8 *)"Unnamed Object";
        CopyString(CurrentSurfaceName, TMPName, StringLength(TMPName));
        u32 CurrentSmoothingValue = 0;
        hash_table FaceTable = {};
        Object->Infos.MinBound = V3(MAX_REAL32, MAX_REAL32, MAX_REAL32);
        Object->Infos.MaxBound = V3(MIN_REAL32, MIN_REAL32, MIN_REAL32);
        // NOTE:: Memory footprint is at least 8 times the original filesize
        // during processing. For every new material it increases
        Assert(BucketAlloc->Transient.Bucket[0].Size > File.Size*8);
        u32 VertexCount = 0,  TexCoordCount = 0,  NormalCount = 0;
        u32 ArraySizes = File.Size;
        memory_bucket_container *Container = &BucketAlloc->Transient;
        v3 *Vertice       = (v3 *)PushSizeOnBucket(Container, ArraySizes);
        v2 *TexCoords     = (v2 *)PushSizeOnBucket(Container, ArraySizes);
        v3 *Normals       = (v3 *)PushSizeOnBucket(Container, ArraySizes);
        Object->Vertice   = (v3 *)PushSizeOnBucket(Container, ArraySizes);
        Object->TexCoords = (v2 *)PushSizeOnBucket(Container, ArraySizes);
        Object->Normals   = (v3 *)PushSizeOnBucket(Container, ArraySizes);
        Object->SurfaceGroups = PushArrayOnBucket(Container, ArraySizes/10, obj_material_group);
        obj_material_group *CurrentGroup = &Object->SurfaceGroups[Object->GroupCount];
        CurrentGroup->IndiceCount = 0;
        CurrentGroup->Indices = (u32 *)PushSizeOnBucket(Container, ArraySizes);
        u32 MaxV3ArraySize = ArraySizes/sizeof(v3);
        u32 MaxV2ArraySize = ArraySizes/sizeof(v2);
        u32 MaxObjMaterialGroupArraySize = ArraySizes/10;
        u32 MaxUint32ArraySize = ArraySizes/sizeof(u32);
        
        u8 *Character = (u8 *)File.Data;
        FaceTable = HashTable(Container, File.Size/10);
        u8 CutPath[MAX_PATH];
        while(*Character != '\0')
        {
            switch(*Character)
            {
                case '#': // Comment in file, ignore.
                {
                    Character = AdvanceToNextLine(Character);
                } 
                break;
                case 'm': // Path to material file
                {
                    Character += 7;
                    u32 CutPathCount = LastOccurrenceOfCharacterInString('\\', (u8 *)Filename, '\0') + 1;
                    CopyString(CutPath, Filename, CutPathCount);
                    CutPath[CutPathCount] = '\0';
                    CopyString(MaterialPath, CutPath, CutPathCount);
                    u8 *Path = (u8 *)MaterialPath + CutPathCount;
                    while(*Character != '\n')
                    {
                        *Path++ = *Character++;
                    }
                    *Path = '\0';
                    Object->Materials.NameIndex = HashTable(&BucketAlloc->Transient, 100);
                }
                break;
                case 'o': // Arbitrary name of object
                {
                    Character += 2;
                    u8 *Path = (u8 *)Object->Name;
                    while(*Character != '\n')
                    {
                        *Path++ = *Character++;
                    }
                    *Path = '\0';
                }
                break;
                case 'v': // Some form of vector
                {
                    Character++;
                    if(*Character == ' ') // Vector is a vertex
                    {
                        Character = AdvanceAfterConsecutiveGivenChar(Character, ' ');
                        Character += ProcessVectorLine(Vertice[VertexCount].E, 3, Character);
                        if(Object->Infos.MinBound.x > Vertice[VertexCount].x)
                        {
                            Object->Infos.MinBound.x = Vertice[VertexCount].x;
                        }
                        if(Object->Infos.MinBound.y > Vertice[VertexCount].y)
                        {
                            Object->Infos.MinBound.y = Vertice[VertexCount].y;
                        }
                        if(Object->Infos.MinBound.z > Vertice[VertexCount].z)
                        {
                            Object->Infos.MinBound.z = Vertice[VertexCount].z;
                        }
                        if(Object->Infos.MaxBound.x < Vertice[VertexCount].x)
                        {
                            Object->Infos.MaxBound.x = Vertice[VertexCount].x;
                        }
                        if(Object->Infos.MaxBound.y < Vertice[VertexCount].y)
                        {
                            Object->Infos.MaxBound.y = Vertice[VertexCount].y;
                        }
                        if(Object->Infos.MaxBound.z < Vertice[VertexCount].z)
                        {
                            Object->Infos.MaxBound.z = Vertice[VertexCount].z;
                        }
                        VertexCount++;
                        Assert(VertexCount < MaxV3ArraySize);
                    }
                    else if(*Character == 't') // Vector is a texture coordinate
                    {
                        Character++;
                        Character = AdvanceAfterConsecutiveGivenChar(Character, ' ');
                        v2 *NewTex = (TexCoords + TexCoordCount++);
                        Character += ProcessVectorLine(NewTex->E, 2, Character);
                        Assert(TexCoordCount < MaxV2ArraySize);
                    }
                    else // Vector is a normal
                    {
                        Character++;
                        Character = AdvanceAfterConsecutiveGivenChar(Character, ' ');
                        Character += ProcessVectorLine(Normals[NormalCount++].E, 3, Character);
                        Assert(NormalCount < MaxV3ArraySize);
                    }
                } 
                break;
                case 'g': // Arbitrary name of a group
                {
                    Character += 2;
                    u8 *Path = (u8 *)CurrentSurfaceName;
                    while(*Character != '\n')
                    {
                        *Path++ = *Character++;
                    }
                    *Path = '\0';
                }
                break;
                case 'u': // Name of a Material for following group + new surface group
                {
                    Character += 7;
                    Assert(Object->GroupCount < MaxObjMaterialGroupArraySize);
                    
                    u8 NewMat[50];
                    u32 MatNameLength = 0;
                    u8 *Name = NewMat;
                    while(*Character != '\n')
                    {
                        *Name++ = *Character++;
                        MatNameLength++;
                    }
                    *Name = '\0';
                    MatNameLength++;
                    if(AddToHashTable(&Object->Materials.NameIndex, NewMat, MAX_UINT32))
                    {
                        if(Object->GroupCount++ > 0)
                        {
                            CurrentGroup = Object->SurfaceGroups + (Object->GroupCount - 1);
                            CurrentGroup->IndiceCount = 0;
                            CurrentGroup->Indices = (u32 *)PushSizeOnBucket(Container, ArraySizes);
                        }
                        CopyString((u8 *)CurrentGroup->MatName, NewMat, MatNameLength);
                    }
                    else
                    {
                        // TODO:: Can we find the group without linearly going through
                        // the list and string-comparing everything?
                        for(u32 Index = 0; Index < Object->GroupCount; ++Index)
                        {
                            if(StringCompare(NewMat, (Object->SurfaceGroups + Index)->MatName, 0, MatNameLength))
                            {
                                CurrentGroup = Object->SurfaceGroups + Index;
                                break;
                            }
                        }
                    }
                }
                break;
                case 's': // Smoothing group :: most likely not working properly
                {
                    //InvalidCodePath;
                    printf("InvalidCodePath");
                    // TODO:: Smoothing value is currently not being integrated. 
                    Character += 2;
                    if(*Character != 'o') // Smoothing value
                    {
                        CurrentSmoothingValue = CharToU32(*Character++);
                        if(*Character != '\n' && *Character != '\0')
                        {
                            CurrentSmoothingValue *= 10;
                            CurrentSmoothingValue += CharToU32(*Character++);
                        }
                    }
                    else // turn off current smoothing
                    {
                        CurrentSmoothingValue = 0;
                        Character += 3;
                    }
                }
                break;
                case 'f': // Description of a surface
                {
                    Character += 2;
                    Assert(CurrentGroup); // If this happens, we need another/different place for creating groups other than usemtl
                    Assert(Object->Count < MaxV3ArraySize);
                    Assert(CurrentGroup->IndiceCount < MaxUint32ArraySize);
                    if(Object->FaceType == NO_VALUE) 
                    {
                        Object->FaceType = IdentifyFaceType(Character);
                    }
                    switch(Object->FaceType)
                    {
                        case VERTEX:
                        case VERTEX_TEXCOORD:
                        {
                            EvaluateNextFaceWithNormal(&FaceTable, Object, CurrentGroup, Character, Vertice, TexCoords, Normals);
                        } break;
                        
                        case VERTEX_NORMAL:
                        case VERTEX_TEXCOORD_NORMAL:
                        {
                            for(u32 Index = 0; Index < 3; ++Index)
                            {
                                Character = EvaluateNextFaceIndexGroup(&FaceTable, Object, CurrentGroup, Character, Vertice, TexCoords, Normals);
                                // 
                                // NOTE:: Currently supporting max 4 Vertice per face. If triggered, need to implement something for that!
                                Character = AdvanceAfterConsecutiveGivenChar(Character, ' ');
                                if(*Character != '\n' && Index == 2)
                                {
                                    *(CurrentGroup->Indices + CurrentGroup->IndiceCount) = *(CurrentGroup->Indices + (CurrentGroup->IndiceCount - 3));
                                    *(CurrentGroup->Indices + (CurrentGroup->IndiceCount + 1)) = *(CurrentGroup->Indices + (CurrentGroup->IndiceCount - 1));
                                    CurrentGroup->IndiceCount += 2;
                                    Character = EvaluateNextFaceIndexGroup(&FaceTable, Object, CurrentGroup, Character, Vertice, TexCoords, Normals);
                                }
                            }
                        } break;
                    }
                    Character = AdvanceToLineEnd(Character);
                }
                break;
                default:
                {
                    Character = AdvanceToNextLine(Character);
                }
                break;
            }
        }
        FreeFileMemory(&BucketAlloc->Transient, File.Data);
        DeleteHashTableTransient(BucketAlloc, &FaceTable);
        PopFromTransientBucket(BucketAlloc, Vertice);  
        PopFromTransientBucket(BucketAlloc, TexCoords);  
        PopFromTransientBucket(BucketAlloc, Normals);  
        
        Object->Vertice = PushArrayFromTransientToFixedBucket(BucketAlloc, Object->Vertice, Object->Count, v3, false);
        Object->Normals = PushArrayFromTransientToFixedBucket(BucketAlloc, Object->Normals, Object->Count, v3, false);
        switch(Object->FaceType)
        {
            case VERTEX_TEXCOORD:
            {
                Object->TexCoords = PushArrayFromTransientToFixedBucket(BucketAlloc, Object->TexCoords, Object->Count, v2, false);
            } break;
            
            case VERTEX_NORMAL:
            {
                PopFromTransientBucket(BucketAlloc, Object->TexCoords);
            } break;
            
            case VERTEX_TEXCOORD_NORMAL:
            {
                Object->TexCoords = PushArrayFromTransientToFixedBucket(BucketAlloc, Object->TexCoords, Object->Count, v2, false);
            }
        }
        for(u32 Index = 0; Index < Object->GroupCount; ++Index)
        {
            obj_material_group *Group = Object->SurfaceGroups + Index;
            Group->Indices = PushArrayFromTransientToFixedBucket(BucketAlloc, Group->Indices, Group->IndiceCount, u32, false);
        }
        Object->SurfaceGroups = PushArrayFromTransientToFixedBucket(BucketAlloc, Object->SurfaceGroups, Object->GroupCount, obj_material_group, false);
        Result = LoadMtlFile(&Object->Materials, BucketAlloc, MaterialPath, CutPath);
    }
    return Result;
}

internal obj_material_group
CollectAllGroupsInOne(obj_data *Object, bucket_allocator *BucketAlloc)
{
    obj_material_group Result = {};
    for(u32 Index = 0; Index < Object->GroupCount; ++Index)
    {
        Result.IndiceCount += Object->SurfaceGroups[Index].IndiceCount;
    }
    Result.Indices = PushArrayOnBucket(&BucketAlloc->Fixed, Result.IndiceCount, u32);
    u32 PersCounter = 0;
    for(u32 GroupIndex = 0; GroupIndex < Object->GroupCount; ++GroupIndex)
    {
        obj_material_group Group = Object->SurfaceGroups[GroupIndex];
        for(u32 IndiceIndex = 0; IndiceIndex < Group.IndiceCount; ++IndiceIndex)
        {
            Result.Indices[PersCounter++] = Group.Indices[IndiceIndex];
        }
    }
    return Result;
}

internal b32
LoadMtlFile(mtl_data *Materials, bucket_allocator *BucketAlloc, u8 *Path, u8 *CutPath)
{
    b32 Result = false;
    read_file_result FileData;
    if(ReadEntireFile(&BucketAlloc->Transient, &FileData, Path))
    {
        Materials->Count = -1;
        u32 MCount = Materials->NameIndex.Count;
        Materials->Ka = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, v3);
        SetReal32ArrayToGiven((r32 *)Materials->Ka, MCount*3, -1.0);
        Materials->Kd = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, v3);
        SetReal32ArrayToGiven((r32 *)Materials->Kd, MCount*3, -1.0);
        Materials->Ks = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, v3);
        SetReal32ArrayToGiven((r32 *)Materials->Ks, MCount*3, -1.0);
        Materials->Tf = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, v3);
        SetReal32ArrayToGiven((r32 *)Materials->Tf, MCount, -1.0);
        Materials->Ns    = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, r32);
        Materials->Ni    = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, r32);
        Materials->Tr    = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, r32);
        Materials->Illum = PushArrayOnBucket(&BucketAlloc->Fixed, MCount, u32);
        Materials->Map_Ka   = (u8 *)PushSizeOnBucket(&BucketAlloc->Fixed, MCount*sizeof(u8)*MAX_PATH);
        Materials->Map_Kd   = (u8 *)PushSizeOnBucket(&BucketAlloc->Fixed, MCount*sizeof(u8)*MAX_PATH);
        Materials->Map_bump = (u8 *)PushSizeOnBucket(&BucketAlloc->Fixed, MCount*sizeof(u8)*MAX_PATH);
        ClearToGiven(Materials->Map_Ka, MCount*sizeof(u8)*MAX_PATH, 0);
        ClearToGiven(Materials->Map_Kd, MCount*sizeof(u8)*MAX_PATH, 0);
        ClearToGiven(Materials->Map_bump, MCount*sizeof(u8)*MAX_PATH, 0);
        u8 *Character = (u8 *)FileData.Data;
        u32 CutPathLength = StringLength(CutPath);
        while(*Character != '\0')
        {
            if(*Character == '\t') 
            {
                Character++;
            }
            switch(*Character)
            {
                case 'n': // New material
                {
                    Character += 7;
                    u8 Name[MAX_PATH];
                    Character += CopyStringUntilChar(Name, Character, '\n');
                    UpdateValueInHashTable(&Materials->NameIndex, Name, ++Materials->Count);
                } break;
                case 'N':
                {
                    Character++;
                    switch(*Character)
                    {
                        case 's': // Specular exponent
                        {
                            Character += 2;
                            u8 Length = 0;
                            *(Materials->Ns + Materials->Count) = ProcessNextR32InString(Character, '\n', Length);
                            Character += Length;
                        } break;
                        case 'i': // Refraction index
                        {
                            Character += 2;
                            u8 Length = 0;
                            *(Materials->Ni + Materials->Count) = ProcessNextR32InString(Character, '\n', Length);
                            Character += Length;
                        } break;
                    }
                } break;
                case 'K':
                {
                    u8 *NextChar = ++Character;
                    v3 NewVec = {};
                    Character += 2;
                    Character += ProcessVectorLine(NewVec.E, 3, Character);
                    Character = AdvanceToLineEnd(Character);
                    switch(*NextChar)
                    {
                        case 'a': // Ambient
                        {
                            *(Materials->Ka + Materials->Count) = NewVec;
                        } break;
                        case 'd': // Diffuse
                        {
                            *(Materials->Kd + Materials->Count) = NewVec;
                        } break;
                        case 's': // Specular
                        {
                            *(Materials->Ks + Materials->Count) = NewVec;
                        } break;
                        case 'e': { /*Nothing, ingore*/ } break;
                    }
                } break;
                case 'T':
                {
                    Character++;
                    switch(*Character)
                    {
                        case 'f': // Transmission filter
                        {
                            Character += 2;
                            Character += ProcessVectorLine((Materials->Tf + Materials->Count)->E, 3, Character);
                            Character = AdvanceToLineEnd(Character);
                        } break;
                        case 'r': // Transparency
                        {
                            Character += 2;
                            u8 Length = 0;
                            *(Materials->Tr + Materials->Count) = ProcessNextR32InString(Character, '\n', Length);
                            Character += Length;
                        } break;
                    }
                } break;
                case 'm':
                {
                    Character += 5;
                    switch(*Character)
                    {
                        case 'a': // Ambient texture map
                        {
                            Character += 2;
                            u8 *MapKa = Materials->Map_Ka + (Materials->Count*MAX_PATH);
                            CopyString(MapKa, CutPath, CutPathLength);
                            MapKa += CutPathLength;
                            Character += CopyStringUntilChar(MapKa, Character, '\n');
                        } break;
                        case 'd': // Diffuse texture map
                        {
                            Character += 2;
                            u8 *MapKd = Materials->Map_Kd + (Materials->Count*MAX_PATH);
                            CopyString(MapKd, CutPath, CutPathLength);
                            MapKd += CutPathLength;
                            Character += CopyStringUntilChar(MapKd, Character, '\n');
                        } break;
                        case 'u': // Bumpmap
                        {
                            Character += 4;
                            u8 *MapBump = Materials->Map_bump + (Materials->Count*MAX_PATH);
                            CopyString(MapBump, CutPath, CutPathLength);
                            MapBump += CutPathLength;
                            Character += CopyStringUntilChar(MapBump, Character, '\n');
                        } break;
                    }
                } break;
                case 'b': // Bumpmap, again
                {
                    Character += 4;
                    u8 *MapBump = Materials->Map_bump + (Materials->Count*MAX_PATH);
                    CopyString(MapBump, CutPath, CutPathLength);
                    MapBump += CutPathLength;
                    Character += CopyStringUntilChar(MapBump, Character, '\n');
                } break;
                case 'i': // Illumination model
                {
                    Character += 6;
                    u8 Length;
                    *(Materials->Illum + Materials->Count) = ProcessNextU32InString(Character, '\n', Length);
                    Character += Length;
                } break;
                default:
                {
                    Character = AdvanceToNextLine(Character);
                }
            }
        }
        FreeFileMemory(&BucketAlloc->Transient, FileData.Data);
        Materials->Count++; // Starting at -1, needs to increment to rectify value
        Result = true;
    }
    return Result;
}

internal loaded_bitmap
ConvertBMPToBlackWhiteTransparent(memory_bucket_container *BucketContainer, loaded_bitmap *ExistingBitmap)
{
    loaded_bitmap Result = {};
    u32 Count = ExistingBitmap->Width*ExistingBitmap->Height;
    
    Result.ColorFormat = colorFormat_RGBA;
    Result.Width = ExistingBitmap->Width;
    Result.Height = ExistingBitmap->Height;
    Result.Pixels = PushArrayOnBucket(BucketContainer, Count, u32);
    ClearToGiven(Result.Pixels, Count, 0);
    
    u8 *AlphaMap = (u8 *)ExistingBitmap->Pixels;
    u32 BitAlign = 4 - ((3*ExistingBitmap->Width)%4);
    For(Count)
    {
        u8 P = *AlphaMap;
        Result.Pixels[It] = ((0xFF-P)<<24)|(P<<16)|(P<<8)|P;
        
        AlphaMap += 3;
        if(It%ExistingBitmap->Width == 0)
        {
            AlphaMap += BitAlign;
        }
    }
    
    return Result;
}


