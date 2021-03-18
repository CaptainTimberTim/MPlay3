#include <windows.h>
#define STB_IMAGE_IMPLEMENTATION
#include "Libraries\\STB_Image.h"

// TODO:: add basic huffman encoding for decoded?

#include "Definitions_TD.h"
#include "Math_TD.c"
#include "Allocator_TD.c"
#include "String_TD.h"
#include "FileUtilities_TD.c"

internal loaded_bitmap LoadImage_STB(read_file_result Memory);

internal loaded_bitmap EncodeToSmallFormat(arena_allocator *Arena, loaded_bitmap Bitmap);

i32 
main(i32 ArgC, char *ArgV[])
{
    DebugPrint(250, "Start converting image...\n");
    
    if(ArgC < 3)
    {
        DebugPrint(255, "ERROR:: We need two arguments: \n> 1. is a file which lists all files to convert into a C-Array\n> 2. is the out-file\n> 3. (Optional) 'DoDecode' will decode the .png files before generating the arrays.\n");
        return 1;
    }
    b32 DoDecode = (ArgC > 3);
    
    arena_allocator Allocator = {};
    Allocator.Flags |= arenaFlags_IsTransient;
    
    string_c ListingPath = NewStaticStringCompound(ArgV[1]);
    string_c OutfilePath = NewStaticStringCompound(ArgV[2]);
    DebugPrint(255, "ImagePath:   %s\n", ListingPath.S);
    DebugPrint(255, "OutfilePath: %s\n", OutfilePath.S);
    if(!WriteEntireFile(&Allocator, OutfilePath.S, 13, "#pragma once\n"))
    {
        DebugPrint(255, "ERROR:: Could not start writing into out-file.\n");
        return 1;
    }
    
    
    read_file_result Listing = {};
    if(!ReadEntireFile(&Allocator, &Listing, ListingPath.S))
    {
        DebugPrint(255, "ERROR:: Could not load list-file!\n");
        return 1;
    }
    
    u32 LineCount = Listing.Size > 0;
    u8 *Bytes = Listing.Data;
    For(Listing.Size)
    {
        if(*Bytes == '\n') 
        {
            LineCount++;
            *Bytes = 0;
        }
        if(*Bytes == '\r') *Bytes = 0;
        ++Bytes;
    }
    
    For(LineCount)
    {
        read_file_result Image = {};
        if(!ReadEntireFile(&Allocator, &Image, Listing.Data))
        {
            DebugPrint(255, "ERROR:: Could not read file: %s\n", Listing.Data);
            return 1;
        }
        b32 DoingDecoding = false;
        i32 Width = 0, Height = 0;
        if(DoDecode)
        {
            u8 *Line = Listing.Data;
            u32 P = LastOccurrenceOfCharacterInString('.', Line, 0);
            Line += P + 1;
            if(Line[0] == 'p' && Line[1] == 'n' && Line[2] == 'g')
            {
                loaded_bitmap ImageMem = LoadImage_STB(Image);
                
                loaded_bitmap EncodedImage = EncodeToSmallFormat(&Allocator, ImageMem);
                Image.Size = EncodedImage.Pitch; //EncodedImage.Width*EncodedImage.Height*sizeof(u32);
                Image.Data = (u8 *)EncodedImage.Pixels;
                Width  = EncodedImage.Width;
                Height = EncodedImage.Height;
                stbi_image_free(ImageMem.Pixels);
                
                DoingDecoding = true;
            }
        }
        DebugPrint(255, "...processing \"%s\"\n", Listing.Data);
        
        string_c ImageName = NewStringCompound(&Allocator, 255);
        AppendStringToCompound(&ImageName, Listing.Data);
        i32 SlashP = FindLastOccurrenceOfCharInStringCompound(&ImageName, '\\');
        if(SlashP < 0) SlashP = FindLastOccurrenceOfCharInStringCompound(&ImageName, '/');
        if(SlashP < 0) SlashP = 0;
        ImageName.S += SlashP+1;
        ImageName.Pos -= SlashP+1;
        
        i32 DotP = FindLastOccurrenceOfCharInStringCompound(&ImageName, '.');
        if(DotP >= 0) ImageName.Pos = DotP;
        
        string_c OutText = NewStringCompound(&Allocator, Image.Size*8 + 500); // Size*6 because of '0x00, ' chars from 1
        
        if(DoingDecoding)
        {
            AppendStringToCompound(&OutText, (u8 *)"global_variable u32 const ");
            AppendStringCompoundToCompound(&OutText, &ImageName);
            AppendStringToCompound(&OutText, (u8 *)"_Width = ");
            I32ToString(&OutText, Width);
            
            AppendStringToCompound(&OutText, (u8 *)";\nglobal_variable u32 const ");
            AppendStringCompoundToCompound(&OutText, &ImageName);
            AppendStringToCompound(&OutText, (u8 *)"_Height = ");
            I32ToString(&OutText, Height);
        }
        else
        {
            AppendStringToCompound(&OutText, (u8 *)"global_variable u32 const ");
            AppendStringCompoundToCompound(&OutText, &ImageName);
            AppendStringToCompound(&OutText, (u8 *)"_DataCount = ");
            I32ToString(&OutText, Image.Size);
        }
        
        AppendStringToCompound(&OutText, (u8 *)";\nglobal_variable u8 const ");
        AppendStringCompoundToCompound(&OutText, &ImageName);
        AppendStringToCompound(&OutText, (u8 *)"_Data[] = {");
        
        u8 *Pixel = Image.Data;
        For(Image.Size)
        {
            if(It%100 == 0) AppendCharToCompound(&OutText, '\n');
            u8 Q = *Pixel++;
            u8 Hex[] = { '0', 'x', ' ', ' ', 0};
            u32 RevCount = 3;
            For(2)
            {
                u8 R = Q%16;
                Q    = Q/16;
                
                if(R < 10) R += 48;
                else       R += 55;
                Hex[RevCount--] = R;
            }
            
            AppendStringToCompound(&OutText, Hex);
            AppendStringToCompound(&OutText, (u8 *)", ");
        }
        AppendStringToCompound(&OutText, (u8 *)"\n};\n\n\n\n");
        
        if(!AppendToFile(&Allocator, OutfilePath.S, OutText.Pos, OutText.S))
        {
            DebugPrint(255, "ERROR:: Could not write into out-file.\n");
            return 1;
        }
        
        if(It < LineCount-1)
        {
            while(*Listing.Data++ != 0) ;
            while(*Listing.Data == 0) Listing.Data++;
        }
    }
    
    DebugPrint(255, "SUCCESS:: Wrote binary data to %s\n", OutfilePath.S);
    
    return 0;
}

internal loaded_bitmap 
LoadImage_STB(read_file_result Memory)
{
    loaded_bitmap Result = {};
    
    read_file_result ImageTest = {};
    i32 BG_X, BG_Y, BG_N;
    Result.Pixels = (u32 *)stbi_load_from_memory(Memory.Data, Memory.Size, &BG_X, &BG_Y, &BG_N, 0);
    Result.Width = BG_X;
    Result.Height = BG_Y;
    if(Result.Pixels) 
    {
        Result.WasLoaded = true;
        switch(BG_N)
        {
            case 3:
            {
                Result.ColorFormat = colorFormat_RGB;
            } break;
            case 4:
            {
                Result.ColorFormat = colorFormat_RGBA;
            } break;
            
            InvalidDefaultCase
        }
    }
    
    return Result;
}

internal loaded_bitmap
EncodeToSmallFormat(arena_allocator *Arena, loaded_bitmap Bitmap)
{
    loaded_bitmap Result = Bitmap;
    u8 *Data = AllocateArray(Arena, Bitmap.Width*Bitmap.Height*sizeof(u32), u8);
    u8 *SavedData = Data;
    
    u32 It = 0;
    while(It < (Bitmap.Width*Bitmap.Height))
    {
        u8 Count = 0;
        u32 PrevPix = *Bitmap.Pixels;
        while(PrevPix == *Bitmap.Pixels) 
        {
            ++Bitmap.Pixels;
            ++It;
            ++Count;
            if(Count == 255) break;
        }
        
        *Data++ = Count;
        u8 *TmpPix = (u8 *)(&PrevPix);
        For(4) *Data++ = *TmpPix++;
        
        Result.Pitch += 5; // Count the array fill.
    }
    
    Result.Pixels = (u32 *)SavedData;
    
    return Result;
}

