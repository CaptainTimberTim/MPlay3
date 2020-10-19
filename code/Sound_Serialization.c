#include "Sound_Serialization.h"

// NOTE:: Settings file specifications
//
// MPlay3Settings
// Version 4
// 
// Volume: <Between 0 and 1>
// LastPlayingSong: <FileName>
// ColorPalette: <ID>
// GenreArtistEdgeXPercent: <Between 0 and 1>
// ArtistAlbumEdgeXPercent: <Between 0 and 1>
// AlbumSongEdgeXPercent: <Between 0 and 1>
// WindowDimX: <Between GlobalMinWindowWidth and MAX_I32>
// WindowDimY: <Between GlobalMinWindowHeight and MAX_I32>
// Looping: <0/1>
// Shuffle: <0/1>
// Palette: <Name>  // Palette with all following color values can occur multiple times
// Text: <R255> <G255> <B255>
// ForegroundText: <R255> <G255> <B255>
// ErrorText: <R255> <G255> <B255>
// Foreground: <R255> <G255> <B255>
// Slot: <R255> <G255> <B255>
// SliderBackground: <R255> <G255> <B255>
// SliderGrabThing: <R255> <G255> <B255>
// ButtonActive: <R255> <G255> <B255>
// Selected: <R255> <G255> <B255>
// PlayingSong: <R255> <G255> <B255>

inline void
ProcessNextPaletteColor(u8 **C, u8 *ColorName, v3 *Color)
{
    u8 Length = 0;
    *C += StringLength(ColorName);
    Color->r = (r32)ProcessNextI32InString(*C, ' ', Length);
    *C += Length+1;
    Color->g = (r32)ProcessNextI32InString(*C, ' ', Length);
    *C += Length+1;
    Color->b = (r32)ProcessNextI32InString(*C, '\n', Length);
    
    AdvanceToNewline(C);
}

internal settings
TryLoadSettingsFile(game_state *GameState)
{
    settings Result = {};
    
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+SETTINGS_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, SETTINGS_FILE_NAME);
    if(ReadEntireFile(&GameState->ScratchArena, &Data, FilePath.S))
    {
        u8 *C = Data.Data;
        
        if(StringCompare(C, (u8 *)"MPlay3Settings", 0, StringLength((u8 *)"MPlay3Settings")))
        {
            AdvanceToNewline(&C);
            
            u8 *VersionString = (u8 *)"Version ";
            u32 VersionLength = StringLength(VersionString);
            
            if(StringCompare(C, VersionString, 0, VersionLength) &&
               CharToU32(C[VersionLength]) == SETTINGS_CURRENT_VERSION)
            {
                AdvanceToNewline(&C);
                AdvanceToNewline(&C);
                u8 Length;
                
                C += StringLength((u8 *)"Volume: ");
                Result.Volume = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"LastPlayingSong: ");
                Result.PlayingSongID.ID = ProcessNextI32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"ColorPalette: ");
                Result.ColorPaletteID = ProcessNextU32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"GenreArtistEdgeXPercent: ");
                Result.GenreArtistEdgeXPercent = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"ArtistAlbumEdgeXPercent: ");
                Result.ArtistAlbumEdgeXPercent = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"AlbumSongEdgeXPercent: ");
                Result.AlbumSongEdgeXPercent = ProcessNextR32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"WindowDimensionX: ");
                Result.WindowDimX = ProcessNextU32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"WindowDimensionY: ");
                Result.WindowDimY = ProcessNextU32InString(C, '\n', Length);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"Looping: ");
                Result.Looping = ProcessNextB32InString(C);
                AdvanceToNewline(&C);
                
                C += StringLength((u8 *)"Shuffle: ");
                Result.Shuffle = ProcessNextB32InString(C);
                AdvanceToNewline(&C);
                
                // Prescan for color palette count
                u8 *PreC = C;
                Result.PaletteCount = 0;
                while(StringCompare(PreC, (u8 *)"Palette: ", 0, 9))
                {
                    For(11) AdvanceToNewline(&PreC);
                    Result.PaletteCount++;
                }
                Result.PaletteMaxCount = Result.PaletteCount+10;
                Result.PaletteNames  = AllocateArray(&GameState->FixArena, Result.PaletteMaxCount, string_c);
                Result.Palettes      = AllocateArray(&GameState->FixArena, Result.PaletteMaxCount, color_palette);
                
                string_c PaletteName = NewStringCompound(&GameState->ScratchArena, 100);
                For(Result.PaletteCount)
                {
                    C += StringLength((u8 *)"Palette: ");
                    CopyStringToCompound(&PaletteName, C, (u8)'\n');
                    Result.PaletteNames[It] = NewStringCompound(&GameState->FixArena, PaletteName.Pos);
                    AppendStringCompoundToCompound(Result.PaletteNames+It, &PaletteName);
                    ResetStringCompound(PaletteName);
                    
                    color_palette *Palette = Result.Palettes+It;
                    AdvanceToNewline(&C);
                    
                    ProcessNextPaletteColor(&C, (u8 *)"Text: ", &Palette->Text);
                    ProcessNextPaletteColor(&C, (u8 *)"ForegroundText: ", &Palette->ForegroundText);
                    ProcessNextPaletteColor(&C, (u8 *)"ErrorText: ", &Palette->ErrorText);
                    ProcessNextPaletteColor(&C, (u8 *)"Foreground: ", &Palette->Foreground);
                    ProcessNextPaletteColor(&C, (u8 *)"Slot: ", &Palette->Slot);
                    ProcessNextPaletteColor(&C, (u8 *)"SliderBackground: ", &Palette->SliderBackground);
                    ProcessNextPaletteColor(&C, (u8 *)"SliderGrabThing: ", &Palette->SliderGrabThing);
                    ProcessNextPaletteColor(&C, (u8 *)"ButtonActive: ", &Palette->ButtonActive);
                    ProcessNextPaletteColor(&C, (u8 *)"Selected: ", &Palette->Selected);
                    ProcessNextPaletteColor(&C, (u8 *)"PlayingSong: ", &Palette->PlayingSong);
                }
                DeleteStringCompound(&GameState->ScratchArena, &PaletteName);
            }
            else
            {
                DebugLog(255, "WARNING:: Library file had wrong version. Needs Version %i!\n", SETTINGS_CURRENT_VERSION);
            }
            
            // If anything is wrong with the saved values, just use the default percentages
            if(Result.GenreArtistEdgeXPercent < 0.037143f || Result.GenreArtistEdgeXPercent > 0.920000f || 
               Result.ArtistAlbumEdgeXPercent < 0.058571f || Result.ArtistAlbumEdgeXPercent > 0.941429f ||
               Result.AlbumSongEdgeXPercent   < 0.087143f || Result.AlbumSongEdgeXPercent   > 0.962857f || 
               Result.GenreArtistEdgeXPercent >= Result.ArtistAlbumEdgeXPercent ||
               Result.ArtistAlbumEdgeXPercent >= Result.AlbumSongEdgeXPercent) 
            {
                Result.GenreArtistEdgeXPercent = 0.2f;
                Result.ArtistAlbumEdgeXPercent = 0.4f;
                Result.AlbumSongEdgeXPercent   = 0.6f;
            }
            
            if(Result.WindowDimX < GlobalMinWindowWidth)  Result.WindowDimX = GlobalMinWindowWidth;
            if(Result.WindowDimY < GlobalMinWindowHeight) Result.WindowDimY = GlobalMinWindowHeight;
        }
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
    
    return Result;
}

internal void
SaveSettingsFile(game_state *GameState, settings *Settings)
{
    string_c SaveData = NewStringCompound(&GameState->ScratchArena, 50000);
    
    AppendStringToCompound(&SaveData, (u8 *)"MPlay3Settings\nVersion ");
    I32ToString(&SaveData, SETTINGS_CURRENT_VERSION);
    AppendStringToCompound(&SaveData, (u8 *)"\n\n");
    
    NewLocalString(FileVolume,       50, "Volume: ");
    NewLocalString(FileLastSong,     50, "LastPlayingSong: ");
    NewLocalString(FileColorPalette, 50, "ColorPalette: ");
    NewLocalString(FileGenreArtist,  50, "GenreArtistEdgeXPercent: ");
    NewLocalString(FileArtistAlbum,  50, "ArtistAlbumEdgeXPercent: ");
    NewLocalString(FileAlbumSong,    50, "AlbumSongEdgeXPercent: ");
    NewLocalString(WindowDimX,       50, "WindowDimensionX: ");
    NewLocalString(WindowDimY,       50, "WindowDimensionY: ");
    NewLocalString(Looping,          50, "Looping: ");
    NewLocalString(Shuffle,          50, "Shuffle: ");
    
    v2i Dim = GetWindowSize();
    R32ToString(&FileVolume, GameState->SoundThreadInterface->ToneVolume);
    I32ToString(&FileLastSong, GameState->MusicInfo.PlayingSong.FileID.ID);
    I32ToString(&FileColorPalette, GameState->MusicInfo.DisplayInfo.ColorPaletteID);
    R32ToString(&FileGenreArtist, GameState->MusicInfo.DisplayInfo.GenreArtist.XPercent);
    R32ToString(&FileArtistAlbum, GameState->MusicInfo.DisplayInfo.ArtistAlbum.XPercent);
    R32ToString(&FileAlbumSong, GameState->MusicInfo.DisplayInfo.AlbumSong.XPercent);
    I32ToString(&WindowDimX, Dim.x);
    I32ToString(&WindowDimY, Dim.y);
    I32ToString(&Looping, GameState->MusicInfo.Looping == playLoop_Loop);
    I32ToString(&Shuffle, GameState->MusicInfo.IsShuffled);
    
    string_c LB = NewStaticStringCompound("\n");
    ConcatStringCompounds(21, &SaveData, &FileVolume, &LB, &FileLastSong, &LB, &FileColorPalette, &LB, &FileGenreArtist, &LB, &FileArtistAlbum, &LB, &FileAlbumSong, &LB, &WindowDimX, &LB, &WindowDimY, &LB, &Looping, &LB, &Shuffle, &LB);
    
    string_c Palette           = NewStaticStringCompound("Palette: ");
    string_c P_Text            = NewStaticStringCompound("\nText: ");
    string_c P_ForegroundText  = NewStaticStringCompound("\nForegroundText: ");
    string_c P_ErrorText       = NewStaticStringCompound("\nErrorText: ");
    string_c P_Foreground      = NewStaticStringCompound("\nForeground: ");
    string_c P_Slot            = NewStaticStringCompound("\nSlot: ");
    string_c P_SliderBG        = NewStaticStringCompound("\nSliderBackground: ");
    string_c P_SliderGrabThing = NewStaticStringCompound("\nSliderGrabThing: ");
    string_c P_ButtonActive    = NewStaticStringCompound("\nButtonActive: ");
    string_c P_Selected        = NewStaticStringCompound("\nSelected: ");
    string_c P_PlayingSong     = NewStaticStringCompound("\nPlayingSong: ");
    
    For(Settings->PaletteCount)
    {
        ConcatStringCompounds(3, &SaveData, &Palette, Settings->PaletteNames+It);
        
        AppendStringCompoundToCompound(&SaveData, &P_Text);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Text));
        AppendStringCompoundToCompound(&SaveData, &P_ForegroundText);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].ForegroundText));
        AppendStringCompoundToCompound(&SaveData, &P_ErrorText);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].ErrorText));
        AppendStringCompoundToCompound(&SaveData, &P_Foreground);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Foreground));
        AppendStringCompoundToCompound(&SaveData, &P_Slot);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Slot));
        AppendStringCompoundToCompound(&SaveData, &P_SliderBG);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].SliderBackground));
        AppendStringCompoundToCompound(&SaveData, &P_SliderGrabThing);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].SliderGrabThing));
        AppendStringCompoundToCompound(&SaveData, &P_ButtonActive);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].ButtonActive));
        AppendStringCompoundToCompound(&SaveData, &P_Selected);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].Selected));
        AppendStringCompoundToCompound(&SaveData, &P_PlayingSong);
        V3iToString(&SaveData, ' ', V3i(Settings->Palettes[It].PlayingSong));
        AppendCharToCompound(&SaveData, '\n');
    }
    
    
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+SETTINGS_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, SETTINGS_FILE_NAME);
    if(WriteEntireFile(&GameState->ScratchArena, FilePath.S, SaveData.Pos, SaveData.S))
    {
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
    DeleteStringCompound(&GameState->ScratchArena, &SaveData);
}

inline void
ApplySettings(game_state *GameState, settings Settings)
{
    ChangeVolume(GameState, Settings.Volume);
    music_info *MusicInfo = &GameState->MusicInfo;
    
    MusicInfo->PlayingSong.PlaylistID = FileIDToPlaylistID(&MusicInfo->Playlist, Settings.PlayingSongID);
    if(Settings.PlayingSongID >= (i32)GameState->MP3Info->FileInfo.Count) Settings.PlayingSongID.ID = -1;
    MusicInfo->PlayingSong.FileID = Settings.PlayingSongID;
    
    ChangeSong(GameState, &MusicInfo->PlayingSong); 
    
    
    MusicInfo->DisplayInfo.ColorPaletteID = Settings.ColorPaletteID; 
    UpdateColorPalette(&MusicInfo->DisplayInfo, false);
    
    MusicInfo->DisplayInfo.GenreArtist.XPercent = Settings.GenreArtistEdgeXPercent;
    MusicInfo->DisplayInfo.ArtistAlbum.XPercent = Settings.ArtistAlbumEdgeXPercent;
    MusicInfo->DisplayInfo.AlbumSong.XPercent = Settings.AlbumSongEdgeXPercent;
    
    ProcessEdgeDragOnResize(&GameState->Renderer, &MusicInfo->DisplayInfo);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Genre, &MusicInfo->SortingInfo.Genre);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Artist, &MusicInfo->SortingInfo.Artist);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Album, &MusicInfo->SortingInfo.Album);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Song.Base, &MusicInfo->SortingInfo.Song);
    UpdateHorizontalSliders(&GameState->Renderer, &MusicInfo->DisplayInfo, &MusicInfo->SortingInfo);
    
    if(Settings.PlayingSongID >= 0)
    {
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Genre, Settings.PlayingSongID);
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Artist, Settings.PlayingSongID);
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Album, Settings.PlayingSongID);
        BringDisplayableEntryOnScreen(&MusicInfo->DisplayInfo.Song.Base, Settings.PlayingSongID);
    }
    
    ApplyWindowResize(GameState->Renderer.Window.WindowHandle, Settings.WindowDimX, Settings.WindowDimY, true);
    
    if(Settings.Looping) 
    {
        OnLoopPlaylistToggleOn(&MusicInfo->DisplayInfo.MusicBtnInfo);
        ToggleButtonVisuals(MusicInfo->DisplayInfo.LoopPlaylist, true);
    }
    if(Settings.Shuffle) 
    {
        OnShufflePlaylistToggleOn(&MusicInfo->DisplayInfo.MusicBtnInfo);
        ToggleButtonVisuals(MusicInfo->DisplayInfo.ShufflePlaylist, true);
    }
}

// NOTE:: Library file specification ************************
// 
// MP3Lib 
// Version 3
// P: PATH
// C: COUNT
//
// P: SUB_PATH_1
//
// >FILE_1_NAME
// >FILE_1_TITLE
// >FILE_1_ARTIST
// >FILE_1_ALBUM
// >FILE_1_GENRE
// >FILE_1_TRACK
// >FILE_1_YEAR
// >FILE_1_DURATION
// 
// >FILE_2_NAME
// ...
// >FILE_1_DURATION
//
// P: SUB_PATH_2
//
// >FILE_1_NAME
// ...
// >FILE_1_DURATION
// 

inline u32
CountToNewline(u8 *C)
{
    u32 Count = 0;
    while(*C != '\r' && *C++ != '\n') Count++;
    return Count;
}

inline u32
CopyUntilNewline(u8 *C, string_c *Result)
{
    u32 Count = 0;
    while(*C != '\r' && *C != '\n')
    {
        AppendCharToCompound(Result, *C++);
        Count++;
    }
    if(*C == '\r') Count++;
    return ++Count;
}

internal b32
ConfirmLibraryWithCorrectVersionExists(game_state *GameState, u32 VersionToCheckFor, u32 *FileInfoCount)
{
    b32 Result = false;
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    u32 BegginingCount = StringLength((u8 *)"MP3Lib\nVersion XX\nP: \nC: XXXXX\n");
    if(ReadBeginningOfFile(&GameState->ScratchArena, &Data, FilePath.S, BegginingCount+255))
    {
        u8 *C = Data.Data;
        u8 *LibString = (u8 *)"MP3Lib";
        if(StringCompare(C, LibString, 0, StringLength(LibString)))
        {
            C += StringLength(LibString)+1; // MP3Lib
            if(C[0] == '\r' || C[0] == '\n') C += 1;
            
            u8 *VersionString = (u8 *)"Version ";
            if(StringCompare(C, VersionString, 0, StringLength(VersionString)))
            {
                C += StringLength(VersionString); // Version 
                u8 L;
                if(ProcessNextU32InString(C, '-', L) == VersionToCheckFor)
                {
                    Result = true;
                    AdvanceToNewline(&C);
                    AdvanceToNewline(&C);
                    C += StringLength((u8 *)"C: ");
                    *FileInfoCount = ProcessNextU32InString(C, '-', L);
                }
            }
        }
        FreeFileMemory(&GameState->ScratchArena, Data.Data);
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
    
    return Result;
}

internal b32
CompareMP3LibraryFileSavedPath(game_state *GameState, string_c *PathToCompare)
{
    b32 Result = false;
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(ReadEntireFile(&GameState->ScratchArena, &Data, FilePath.S))
    {
        u8 *C = Data.Data;
        if(StringCompare(C, (u8 *)"MP3Lib", 0, 6))
        {
            C += 7; // MP3Lib
            C += 10; // Version 3
            C += 3; // P:_
            string_c FolderPath = NewStringCompound(&GameState->ScratchArena, 255);
            C += CopyUntilNewline(C, &FolderPath);
            
            if(CompareStringCompounds(&FolderPath, PathToCompare))
            {
                Result = true;
            }
            
            DeleteStringCompound(&GameState->ScratchArena, &FolderPath);
        }
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
    
    return Result;
}

internal void
LoadMP3LibraryFile(game_state *GameState, mp3_info *Info)
{
    read_file_result Data = {};
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(ReadEntireFile(&GameState->ScratchArena, &Data, FilePath.S))
    {
        mp3_file_info *MP3FileInfo = &Info->FileInfo;
        
        u8 *C = Data.Data;
        
        u8 *LibID = (u8 *)"MP3Lib";
        u8 *VersionString = (u8 *)"Version ";
        
        b32 FileIsCorrect = StringCompare(C, LibID, 0, StringLength(LibID));
        AdvanceToNewline(&C);
        FileIsCorrect = FileIsCorrect && StringCompare(C, VersionString, 0, StringLength(VersionString));
        C += StringLength(VersionString);
        u8 N = 0;
        u32 Version = ProcessNextU32InString(C, '\n', N);
        FileIsCorrect = FileIsCorrect && LIBRARY_CURRENT_VERSION == Version;
        
        if(!FileIsCorrect)
        {
            DebugLog(255, "WARNING:: Library file had wrong version. Needs Version %i!\n", LIBRARY_CURRENT_VERSION);
        }
        else
        {
            AdvanceToNewline(&C);
            C += 3;
            Info->FolderPath = NewStringCompound(&GameState->FixArena, 255);
            C += CopyUntilNewline(C, &Info->FolderPath);
            
            C += 3;
            string_c CountS = NewStringCompound(&GameState->ScratchArena, CountToNewline(C));
            C += CopyUntilNewline(C, &CountS);
            u32 SongCount = ConvertU32FromString(CountS.S, CountS.Pos);
            DeleteStringCompound(&GameState->ScratchArena, &CountS);
            
            AdvanceToNewline(&C);
            string_c CurrentSubPath = NewStringCompound(&GameState->ScratchArena, 255);
            For(SongCount)
            {
                if(*C == 'P')
                {
                    C += 3;
                    WipeStringCompound(&CurrentSubPath);
                    C += CopyUntilNewline(C, &CurrentSubPath);
                    AdvanceToNewline(&C);
                }
                
                mp3_metadata *MD = MP3FileInfo->Metadata + It;
                
                MP3FileInfo->SubPath[It] = NewStringCompound(&GameState->JobThreadsArena, CurrentSubPath.Pos);
                AppendStringCompoundToCompound(MP3FileInfo->SubPath+It, &CurrentSubPath);
                MP3FileInfo->FileName[It] = NewStringCompound(&GameState->JobThreadsArena, CountToNewline(++C));
                C += CopyUntilNewline(C, MP3FileInfo->FileName+It);
                
                u32 Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Title = NewStringCompound(&GameState->FixArena, Count);
                    C += CopyUntilNewline(C, &MD->Title);
                    MD->FoundFlags |= metadata_Title;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Artist = NewStringCompound(&GameState->FixArena, Count);
                    C += CopyUntilNewline(C, &MD->Artist);
                    MD->FoundFlags |= metadata_Artist;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Album = NewStringCompound(&GameState->FixArena, Count);
                    C += CopyUntilNewline(C, &MD->Album);
                    MD->FoundFlags |= metadata_Album;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->Genre = NewStringCompound(&GameState->FixArena, Count);
                    C += CopyUntilNewline(C, &MD->Genre);
                    MD->FoundFlags |= metadata_Genre;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->TrackString = NewStringCompound(&GameState->FixArena, Count);
                    C += CopyUntilNewline(C, &MD->TrackString);
                    u8 Length = 0;
                    MD->Track = ProcessNextU32InString(MD->TrackString.S, '\0', Length);
                    MD->FoundFlags |= metadata_Track;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    MD->YearString = NewStringCompound(&GameState->FixArena, Count);
                    C += CopyUntilNewline(C, &MD->YearString);
                    u8 Length = 0;
                    MD->Year = ProcessNextU32InString(MD->YearString.S, '\0', Length);
                    MD->FoundFlags |= metadata_Year;
                }
                else AdvanceToNewline(&C);
                Count = CountToNewline(++C);
                if(Count > 0)
                {
                    string_c Duration = NewStringCompound(&GameState->ScratchArena, Count);
                    C += CopyUntilNewline(C, &Duration);
                    u8 Length = 0;
                    MD->Duration = ProcessNextU32InString(Duration.S, '\0', Length);
                    MillisecondsToMinutes(MD->Duration, &MD->DurationString);
                    DeleteStringCompound(&GameState->ScratchArena, &Duration);
                    MD->FoundFlags |= metadata_Duration;
                }
                else AdvanceToNewline(&C);
                
                MP3FileInfo->Count++;
                AdvanceToNewline(&C);
            }
            DeleteStringCompound(&GameState->ScratchArena, &CurrentSubPath);
        }
        FreeFileMemory(&GameState->ScratchArena, Data.Data);
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
}

internal void
SaveMP3LibraryFile(game_state *GameState, mp3_info *Info)
{
    string_c NL    = NewStaticStringCompound("\n");
    string_c Colon = NewStaticStringCompound("P: ");
    string_c Inset = NewStaticStringCompound(">");
    
    mp3_file_info *MP3FileInfo = &Info->FileInfo;
    if(MP3FileInfo->Count == 0) return;
    u32 StringSize = (30+255+5) + 7*255*MP3FileInfo->Count;
    string_c SaveData = NewStringCompound(&GameState->ScratchArena, StringSize);
    
    AppendStringToCompound(&SaveData, (u8 *)"MP3Lib\n");
    AppendStringToCompound(&SaveData, (u8 *)"Version 3\n");
    ConcatStringCompounds(4, &SaveData, &Colon, &Info->FolderPath, &NL);
    char CountS[25];
    sprintf_s(CountS, "C: %i\n\n", MP3FileInfo->Count);
    AppendStringToCompound(&SaveData, (u8 *)CountS);
    
    u32 WrittenDataCount = 0;
    b32 *Written = AllocateArray(&GameState->ScratchArena, Info->FileInfo.MaxCount, b32);
    string_c CurrentSubPath = NewStringCompound(&GameState->ScratchArena, 255);
    while(WrittenDataCount < MP3FileInfo->Count)
    {
        AppendStringCompoundToCompound(&SaveData, &Colon);
        For(MP3FileInfo->Count)
        {
            if(!Written[It])
            {
                ConcatStringCompounds(4, &SaveData, MP3FileInfo->SubPath+It, &NL, &NL);
                AppendStringCompoundToCompound(&CurrentSubPath, MP3FileInfo->SubPath+It);
                break;
            }
        }
        For(MP3FileInfo->Count)
        {
            if(!Written[It] && 
               CompareStringCompounds(&CurrentSubPath, MP3FileInfo->SubPath+It))
            {
                mp3_metadata *MD = MP3FileInfo->Metadata+It;
                
                ConcatStringCompounds(4, &SaveData, Inset, MP3FileInfo->FileName[It], NL);
                if(MD->FoundFlags & metadata_Title)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Title, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Artist)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Artist, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Album)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Album, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Genre)
                {
                    ConcatStringCompounds(4, &SaveData, Inset, MD->Genre, NL);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Track && MD->Track > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Track);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Year && MD->Year > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Year);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, Inset, NL);
                if(MD->FoundFlags & metadata_Duration && MD->Duration > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n\n", MD->Duration);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(4, &SaveData, Inset, NL, NL);
                
                Written[It] = true;
                WrittenDataCount++;
            }
        }
        
        WipeStringCompound(&CurrentSubPath);
    }
    DeleteStringCompound(&GameState->ScratchArena, &CurrentSubPath);
    FreeMemory(&GameState->ScratchArena, Written);
    
    // TODO:: Rename old save file as backup, before writing and after successful write delete the old one.
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(WriteEntireFile(&GameState->ScratchArena, FilePath.S, SaveData.Pos, SaveData.S))
    {
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
    DeleteStringCompound(&GameState->ScratchArena, &SaveData);
}

internal void
WipeMP3LibraryFile(game_state *GameState)
{
    string_c FilePath = NewStringCompound(&GameState->ScratchArena, GameState->DataPath.Pos+LIBRARY_FILE_NAME.Pos);
    ConcatStringCompounds(3, &FilePath, &GameState->DataPath, LIBRARY_FILE_NAME);
    if(WriteEntireFile(&GameState->ScratchArena, FilePath.S, 0, 0))
    {
    }
    DeleteStringCompound(&GameState->ScratchArena, &FilePath);
}
