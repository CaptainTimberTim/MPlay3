#include "FileUtilities_TD.h"

internal b32
WriteEntireFile(arena_allocator *Arena, u8 *Filename, u32 MemorySize, void *Memory)
{
    b32 Result = false;
    
    Assert(Arena->Flags & arenaFlags_IsTransient);
    string_w FileNameWide = {};
    ConvertString8To16(Arena, Filename, &FileNameWide);
    Result = WriteEntireFile(&FileNameWide, MemorySize, Memory);
    DeleteStringW(Arena, &FileNameWide);
    
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

internal b32
WriteToFile(arena_allocator *Arena, u8 *Filename, u32 MemorySize, void *Memory, u32 WriteOffsetFromStart)
{
    b32 Result = false;
    
    Assert(Arena->Flags & arenaFlags_IsTransient);
    string_w FileNameWide = {};
    ConvertString8To16(Arena, Filename, &FileNameWide);
    Result = WriteToFile(&FileNameWide, MemorySize, Memory, WriteOffsetFromStart);
    DeleteStringW(Arena, &FileNameWide);
    
    return Result;
}

internal b32
WriteToFile(string_w *Filename, u32 MemoryWriteSize, void *Memory, u32 WriteOffsetFromStart)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        
        LARGE_INTEGER MoveAmount = {WriteOffsetFromStart};
        if(SetFilePointerEx(FileHandle, MoveAmount, 0, FILE_BEGIN))
        {
            if(WriteFile(FileHandle, Memory, MemoryWriteSize, &BytesWritten, 0))
            {
                Result = (BytesWritten == MemoryWriteSize);
                //File write successfully
            }
            else
            {
                //Could not write the file
            } 
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
FreeFileMemory(arena_allocator *Arena, read_file_result File)
{
    if(File.Data && Arena->Flags & arenaFlags_IsTransient)
    {
        FreeMemory(Arena, File.Data);
    }
}

internal b32
DeleteFile(string_w Filename)
{
    b32 Result = false;
    
    if(DeleteFileW(Filename.S))
    {
        Result = true;
    }
    return Result;
}

internal b32
DeleteFile(arena_allocator *Arena, string_c Filename)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(Arena, &Filename, &FileNameWide);
    Result = DeleteFile(FileNameWide);
    DeleteStringW(Arena, &FileNameWide);
    
    return Result;
}

internal b32 
ReadEntireFile(arena_allocator *Arena, read_file_result *FileData, u8 *Filename)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(Arena, Filename, &FileNameWide); // TODO:: #ArenaAllocator, should maybe use scratch memory somhow...
    Result = ReadEntireFile(Arena, FileData, &FileNameWide);
    DeleteStringW(Arena, &FileNameWide);
    
    return Result;
}

internal b32 
ReadEntireFile(arena_allocator *Arena, read_file_result *FileData, string_w *Filename)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            FileData->Data = AllocateMemory(Arena, FileSize32+1);
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
                    FreeFileMemory(Arena, *FileData);
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
ReadBeginningOfFile(arena_allocator *Arena, read_file_result *FileData, u8 *Filename, u32 ReadAmount)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(Arena, Filename, &FileNameWide); // TODO:: #ArenaAllocator, should maybe use scratch memory somhow...
    Result = ReadBeginningOfFile(Arena, FileData, &FileNameWide, ReadAmount);
    DeleteStringW(Arena, &FileNameWide);
    
    return Result;
}

internal b32 
ReadBeginningOfFile(arena_allocator *Arena, read_file_result *FileData, string_w *Filename, u32 ReadAmount)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileW(Filename->S, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            ReadAmount = (ReadAmount > FileSize32) ? FileSize32+1 : ReadAmount+1;
            FileData->Data = AllocateMemory(Arena, ReadAmount);
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
                    FreeFileMemory(Arena, *FileData);
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
ReadEndOfFile(arena_allocator *Arena, read_file_result *FileData, u8 *Filename, u32 ReadAmount)
{
    b32 Result = false;
    
    string_w FileNameWide = {};
    ConvertString8To16(Arena, Filename, &FileNameWide); // TODO:: #ArenaAllocator, should maybe use scratch memory somhow...
    Result = ReadEndOfFile(Arena, FileData, &FileNameWide, ReadAmount);
    DeleteStringW(Arena, &FileNameWide);
    
    return Result;
}

internal b32 
ReadEndOfFile(arena_allocator *Arena, read_file_result *FileData, string_w *Filename, u32 ReadAmount)
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
            FileData->Data = AllocateMemory(Arena, ReadAmount);
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
                        FreeFileMemory(Arena, *FileData);
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
AppendToFile(arena_allocator *Arena, u8 *FileName, u32 MemorySize, void *Memory)
{
    b32 Result = false;
    read_file_result FileData = {};
    if(ReadEntireFile(Arena, &FileData, FileName))
    {
        u8 *AllData = AllocateMemory(Arena, FileData.Size+MemorySize-1);
        u8 *DataStart = AllData;
        u8 *FileData2 = FileData.Data;
        For(FileData.Size-1) *AllData++ = *FileData2++;
        //*AllData++ = '\n';
        u8 *UMemory = (u8 *)Memory;
        For(MemorySize) *AllData++ = *UMemory++;
        //*AllData++ = '\n';
        //*AllData++ = 0;
        
        MemorySize += FileData.Size-1;
        if(WriteEntireFile(Arena, FileName, MemorySize, DataStart))
        {
            Result = true;
        }
        FreeFileMemory(Arena, FileData);
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
LoadBMPImage(arena_allocator *Arena, u8 *FileName)
{
    loaded_bitmap Result = {};
    Result.WasLoaded = false;
    read_file_result FileData = {};
    if(ReadEntireFile(Arena, &FileData, FileName))
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
ConvertBMPToBlackWhiteTransparent(arena_allocator *Arena, loaded_bitmap *ExistingBitmap)
{
    loaded_bitmap Result = {};
    u32 Count = ExistingBitmap->Width*ExistingBitmap->Height;
    
    Result.ColorFormat = colorFormat_RGBA;
    Result.Width = ExistingBitmap->Width;
    Result.Height = ExistingBitmap->Height;
    Result.Pixels = AllocateArray(Arena, Count, u32);
    ClearMemory(Result.Pixels, Count);
    
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

internal void
OutputLastWindowsError()
{
    i32 Result = GetLastError();
    
    switch(Result)
    {
        case ERROR_SUCCESS:         DebugLog(150, "ERROR_SUCCESS\n"); break;
        case ERROR_FILE_NOT_FOUND:  DebugLog(150, "ERROR_FILE_NOT_FOUND\n"); break;
        case ERROR_ACCESS_DENIED:   DebugLog(150, "ERROR_ACCESS_DENIED\n"); break;
        case ERROR_PATH_NOT_FOUND:  DebugLog(150, "ERROR_PATH_NOT_FOUND\n"); break;
        
        
        default: {
            DebugLog(150, "Error not in list, value: %i\n", Result);
        }
    }
}

internal b32
CheckFileExists(arena_allocator *ScratchArena, string_c FilePath)
{
    if(FilePath.Pos == 0) return false;
    
    string_w WideFilePath = {};
    ConvertString8To16(ScratchArena, &FilePath, &WideFilePath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WideFilePath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    
    b32 Result = false;
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ; // If directory, this is not valid!
        else Result = true;
    }
    
    return Result;
}

internal b32
CheckPathValidity(arena_allocator *ScratchArena, string_c Path)
{
    if(Path.Pos == 0) return false;
    
    NewLocalString(StarPath, 260, Path.S);
    if(Path.S[Path.Pos-1] == ':' || Path.S[Path.Pos-1] == '\\') 
        AppendCharToCompound(&StarPath, '*');
    
    string_w WidePath = {};
    ConvertString8To16(ScratchArena, &StarPath, &WidePath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WidePath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    
    b32 Result = FileHandle != INVALID_HANDLE_VALUE;
    return Result;
}

internal b32
CheckPathValidity(arena_allocator *ScratchArena, string_c Path, i32 *FirstWrongCharPos)
{
    if(Path.Pos <= 1) return false; // Skip path length of 0 or 1, as we minimum want for example: 'C:' or '..'.
    if(FirstWrongCharPos == NULL) return false;
    Assert(Path.Pos <= 260);
    
    NewEmptyLocalString(StarPath, 260);
    StarPath.S[0] = Path.S[0];
    
    b32 Result = true;
    *FirstWrongCharPos = 0;
    while(Result && *FirstWrongCharPos < (i32)Path.Pos)
    {
        StarPath.Pos = ++(*FirstWrongCharPos) + 1;
        StarPath.S[*FirstWrongCharPos] = Path.S[*FirstWrongCharPos];
        
        AppendCharToCompound(&StarPath, '*');
        
        string_w WidePath = {};
        ConvertString8To16(ScratchArena, &StarPath, &WidePath);
        
        WIN32_FIND_DATAW FileData = {};
        HANDLE FileHandle = FindFirstFileExW(WidePath.S, 
                                             FindExInfoBasic, 
                                             &FileData, 
                                             FindExSearchNameMatch, 
                                             NULL, 
                                             FIND_FIRST_EX_LARGE_FETCH);
        
        Result = FileHandle != INVALID_HANDLE_VALUE;
        DeleteStringW(ScratchArena, &WidePath);
    }
    
    // We handle the first two letters as a bundle. So when we fail _and_
    // the Pos is at 1 we want the first letter to be false as well.
    if(*FirstWrongCharPos == 1 && !Result) 
        *FirstWrongCharPos = 0;
    
    return Result;
}

