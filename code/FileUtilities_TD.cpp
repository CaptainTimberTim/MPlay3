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
ReadEndOfFile(memory_bucket_container *Bucket, read_file_result *FileData, u8 *Filename, i32 ReadAmount)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(&Bucket->Parent->Transient, Filename, &FileNameWide);
    Result = ReadEndOfFile(Bucket, FileData, &FileNameWide, ReadAmount);
    DeleteStringW(&Bucket->Parent->Transient, &FileNameWide);
    
    return Result;
}

internal b32 
ReadEndOfFile(memory_bucket_container *Bucket, read_file_result *FileData, string_w *Filename, i32 ReadAmount)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            Assert(FileSize.QuadPart < MAX_UINT32);
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            FileData->Data = (u8 *)PushSizeOnBucket(Bucket, ReadAmount);
            if (FileData->Data)
            {
                // Now move the file pointer to start reading from the end
                LARGE_INTEGER MoveAmount = FileSize;
                MoveAmount.QuadPart = ((MoveAmount.QuadPart - ReadAmount) < 0) ? 0 : (MoveAmount.QuadPart - ReadAmount);
                if(SetFilePointerEx(FileHandle, MoveAmount, 0, FILE_BEGIN))
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
                    DebugLog(255, "ERROR:: Could not move FilePointer.\n");
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


