#include "Sound_Serialization.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Settings file specifications ~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// MPlay3Settings
// Version 5
//
// # NOTE 1:: If you don't want this file and the Library 
// # file to be in the same directory as the executable, you 
// # can put them into the '%appdata%\Roaming\MPlay3Data'
// # folder. If they can be found -by the program- in 
// # that specific location, it will overwrite them in there.
//
// # NOTE 2:: If you want to use a different font than the
// # default, you can add it here right behind the 'FontPath'
// # identifier. Only *.ttf files work! With FontHeightOffset 
// # the vertical position of the text can be adjusted 
// # (only integers allowed).
//
// FontPath: <Path/to/font>
// FontHeightOffset: <0>
// Volume: <Between 0 and 1>
// LastPlayingSong: <FileName>
// ColorPalette: <ID>
// GenreArtistEdgeXPercent: <Between 0 and 1>
// ArtistAlbumEdgeXPercent: <Between 0 and 1>
// AlbumSongEdgeXPercent: <Between 0 and 1>
// WindowDimensionX: <Between GlobalMinWindowWidth and MAX_I32>
// WindowDimensionY: <Between GlobalMinWindowHeight and MAX_I32>
// Looping: <0/1>
// Shuffle: <0/1>
// UsedFontCache: <font name>|<font name>
// ActivePlaylist: <Name>
// 
// Palette: <Name>  // Palette with all following color values can occur multiple times
//     Text: <R255> <G255> <B255>
//     ForegroundText: <R255> <G255> <B255>
//     ErrorText: <R255> <G255> <B255>
//     Foreground: <R255> <G255> <B255>
//     Slot: <R255> <G255> <B255>
//     SliderBackground: <R255> <G255> <B255>
//     SliderGrabThing: <R255> <G255> <B255>
//     ButtonActive: <R255> <G255> <B255>
//     Selected: <R255> <G255> <B255>
//     PlayingSong: <R255> <G255> <B255>

inline u32
CountToNewline(u8 *C)
{
    u32 Count = 0;
    while(*C != 0 && *C != '\r' && *C++ != '\n') Count++;
    return Count;
}

inline u32
CountToNewlineOrDelimeter(u8 *C, u8 Delimeter)
{
    u32 Count = 0;
    while(*C != 0 && *C != '\r' && *C != '\n' && *C++ != Delimeter) Count++;
    return Count;
}

inline u32
CountToDelimeters(u8 *C, u8 *Delimeters, u32 DeliCount)
{
    u32 Count = 0;
    while(*C != 0) 
    {
        b32 Break = false;
        For(DeliCount)
        {
            Break = *C == Delimeters[It];
            if(Break) break;
        }
        if(Break) break;
        C++;
        Count++;
    }
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

inline void
TryEatComment(u8 **C)
{
    if(**C == '#') AdvanceToNewline(C);
}

inline void
ProcessNextPaletteColor(u8 **C, u32 NameLength, v3 *Color)
{
    u8 Length = 0;
    *C += NameLength;
    EatLeadingSpaces(C);
    Color->r = (r32)Clamp(ProcessNextI32InString(*C, ' ', Length), 0, 255);
    *C += Length;
    EatLeadingSpaces(C);
    Color->g = (r32)Clamp(ProcessNextI32InString(*C, ' ', Length), 0, 255);
    *C += Length;
    EatLeadingSpaces(C);
    Color->b = (r32)Clamp(ProcessNextI32InString(*C, (u8 *)"\n ", 2, Length), 0, 255);
    
    AdvanceToNewline(C);
}

internal settings
TryLoadSettingsFile(game_state *GameState)
{
    settings Result = {};
    Result.WindowDimX = GameState->Layout.WindowWidth;
    Result.WindowDimY = GameState->Layout.WindowHeight;
    
    Result.CachedFontNames = AllocateStruct(&GameState->FixArena, font_name_list);
    *Result.CachedFontNames = {};
    
    Result.PaletteMaxCount = 15; // @HardLimit
    string_c *PaletteNames  = AllocateArray(&GameState->ScratchArena, Result.PaletteMaxCount, string_c);
    color_palette *Palettes = AllocateArray(&GameState->ScratchArena, Result.PaletteMaxCount, color_palette);
    
    read_file_result Data = {};
    if(ReadEntireFile(&GameState->ScratchArena, &Data, GameState->SettingsPath.S))
    {
        u8 *C = Data.Data;
        
        string_c FileIDS = NewStaticStringCompound("MPlay3Settings");
        if(StringCompare(C, FileIDS.S, 0, FileIDS.Pos))
        {
            AdvanceToNewline(&C);
            
            string_c VersionS                       = NewStaticStringCompound("Version ");
            string_c FontPathS                      = NewStaticStringCompound("FontPath:");
            string_c FontHeightS                    = NewStaticStringCompound("FontHeightOffset:");
            string_c VolumeS                        = NewStaticStringCompound("Volume:");
            string_c LastPlayingSongS               = NewStaticStringCompound("LastPlayingSong:");
            string_c ColorPaletteS                  = NewStaticStringCompound("ColorPalette:");
            string_c PlaylistsGenreEdgeXPercentageS = NewStaticStringCompound("PlaylistsGenreEdgeXPercent:");
            string_c GenreArtistEdgeXPercentageS    = NewStaticStringCompound("GenreArtistEdgeXPercent:");
            string_c ArtistAlbumEdgeXPercentageS    = NewStaticStringCompound("ArtistAlbumEdgeXPercent:");
            string_c AlbumSongEdgeXPercentageS      = NewStaticStringCompound("AlbumSongEdgeXPercent:");
            string_c WindowDimXS                    = NewStaticStringCompound("WindowDimensionX:");
            string_c WindowDimYS                    = NewStaticStringCompound("WindowDimensionY:");
            string_c LoopingS                       = NewStaticStringCompound("Looping:");
            string_c ShuffleS                       = NewStaticStringCompound("Shuffle:");
            string_c CachedFontS                    = NewStaticStringCompound("UsedFontCache:");
            string_c ActivePlaylistS                = NewStaticStringCompound("ActivePlaylist:");
            
            string_c PaletteS                       = NewStaticStringCompound("Palette:");
            string_c TextS                          = NewStaticStringCompound("Text:");
            string_c ForegroundTextS                = NewStaticStringCompound("ForegroundText:");
            string_c ErrorTextS                     = NewStaticStringCompound("ErrorText:");
            string_c ForegroundS                    = NewStaticStringCompound("Foreground:");
            string_c SlotS                          = NewStaticStringCompound("Slot:");
            string_c SliderBackgroundS              = NewStaticStringCompound("SliderBackground:");
            string_c SliderGrabThingS               = NewStaticStringCompound("SliderGrabThing:");
            string_c ButtonActiveS                  = NewStaticStringCompound("ButtonActive:");
            string_c SelectedS                      = NewStaticStringCompound("Selected:");
            string_c PlayingSongS                   = NewStaticStringCompound("PlayingSong:");
            
            u8 L; // Not used for anything.
            
            while(*C)
            {
                if(*C == ' ') EatLeadingSpaces(&C);
                
                if(*C == '#') ; // Comment, skip.
                else if(*C == '\n') ; // Blank line, skip.
                else if(StringCompare(C, VersionS.S, 0, VersionS.Pos)) ; // Version line, skip for now.
                else if(StringCompare(C, FontPathS.S, 0, FontPathS.Pos))
                {
                    C += FontPathS.Pos;
                    EatLeadingSpaces(&C);
                    u32 PLen = CountToNewlineOrDelimeter(C, ' ');
                    Result.FontPath = NewStringCompound(&GameState->FixArena, PLen);
                    CopyStringToCompound(&Result.FontPath, C, 0u, PLen);
                }
                else if(StringCompare(C, FontHeightS.S, 0, FontHeightS.Pos))
                {
                    C += FontHeightS.Pos;
                    EatLeadingSpaces(&C);
                    Result.FontHeightOffset = ProcessNextI32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, VolumeS.S, 0, VolumeS.Pos))
                {
                    C += VolumeS.Pos;
                    EatLeadingSpaces(&C);
                    Result.Volume = ProcessNextR32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, LastPlayingSongS.S, 0, LastPlayingSongS.Pos))
                {
                    C += LastPlayingSongS.Pos;
                    EatLeadingSpaces(&C);
                    Result.PlayingSongID.ID = ProcessNextI32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, ColorPaletteS.S, 0, ColorPaletteS.Pos))
                {
                    C += ColorPaletteS.Pos;
                    EatLeadingSpaces(&C);
                    Result.ColorPaletteID = ProcessNextU32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, PlaylistsGenreEdgeXPercentageS.S, 0, PlaylistsGenreEdgeXPercentageS.Pos))
                {
                    C += PlaylistsGenreEdgeXPercentageS.Pos;
                    EatLeadingSpaces(&C);
                    Result.PlaylistsGenreEdgeXPercent = ProcessNextR32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, GenreArtistEdgeXPercentageS.S, 0, GenreArtistEdgeXPercentageS.Pos))
                {
                    C += GenreArtistEdgeXPercentageS.Pos;
                    EatLeadingSpaces(&C);
                    Result.GenreArtistEdgeXPercent = ProcessNextR32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, ArtistAlbumEdgeXPercentageS.S, 0, ArtistAlbumEdgeXPercentageS.Pos))
                {
                    C += ArtistAlbumEdgeXPercentageS.Pos;
                    EatLeadingSpaces(&C);
                    Result.ArtistAlbumEdgeXPercent = ProcessNextR32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, AlbumSongEdgeXPercentageS.S, 0, AlbumSongEdgeXPercentageS.Pos))
                {
                    C += AlbumSongEdgeXPercentageS.Pos;
                    EatLeadingSpaces(&C);
                    Result.AlbumSongEdgeXPercent = ProcessNextR32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, WindowDimXS.S, 0, WindowDimXS.Pos))
                {
                    C += WindowDimXS.Pos;
                    EatLeadingSpaces(&C);
                    Result.WindowDimX = ProcessNextU32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, WindowDimYS.S, 0, WindowDimYS.Pos))
                {
                    C += WindowDimYS.Pos;
                    EatLeadingSpaces(&C);
                    Result.WindowDimY = ProcessNextU32InString(C, (u8 *)"\n ", 2, L);
                }
                else if(StringCompare(C, LoopingS.S, 0, LoopingS.Pos))
                {
                    C += LoopingS.Pos;
                    EatLeadingSpaces(&C);
                    Result.Looping = ProcessNextB32InString(C);
                }
                else if(StringCompare(C, ShuffleS.S, 0, ShuffleS.Pos))
                {
                    C += ShuffleS.Pos;
                    EatLeadingSpaces(&C);
                    Result.Shuffle = ProcessNextB32InString(C);
                }
                else if(StringCompare(C, CachedFontS.S, 0, CachedFontS.Pos))
                {
                    C += CachedFontS.Pos;
                    EatLeadingSpaces(&C);
                    
                    Result.CachedFontNames->MaxCount = 10;
                    string_c *FontNames = AllocateArray(&GameState->ScratchArena, Result.CachedFontNames->MaxCount, string_c);
                    while(*C != '\n')
                    {
                        if(Result.CachedFontNames->Count >= Result.CachedFontNames->MaxCount)
                        {
                            Result.CachedFontNames->MaxCount += 10;
                            FontNames = ReallocateArray(&GameState->ScratchArena, FontNames, 
                                                        Result.CachedFontNames->Count, Result.CachedFontNames->MaxCount, string_c);
                        }
                        
                        u32 PLen = CountToDelimeters(C, (u8 *)"| \r\n", 4);
                        FontNames[Result.CachedFontNames->Count] = NewStringCompound(&GameState->FixArena, PLen);
                        CopyStringToCompound(FontNames + Result.CachedFontNames->Count, C, 0u, PLen);
                        ++Result.CachedFontNames->Count;
                        
                        C += PLen;
                        EatLeadingSpaces(&C);
                        if(*C == '|') ++C;
                        EatLeadingSpaces(&C);
                    }
                    
                    Result.CachedFontNames->MaxCount = Result.CachedFontNames->Count;
                    Result.CachedFontNames->Names = AllocateArray(&GameState->FixArena, Result.CachedFontNames->MaxCount, string_c);
                    For(Result.CachedFontNames->Count) Result.CachedFontNames->Names[It] = FontNames[It];
                }
                else if(StringCompare(C, PaletteS.S, 0, PaletteS.Pos))
                {
                    if(Result.PaletteCount >= Result.PaletteMaxCount)
                    {
                        Result.PaletteMaxCount += 10;
                        PaletteNames = ReallocateArray(&GameState->ScratchArena, PaletteNames, 
                                                       Result.PaletteCount, Result.PaletteMaxCount, string_c);
                        Palettes     = ReallocateArray(&GameState->ScratchArena, Palettes, 
                                                       Result.PaletteCount, Result.PaletteMaxCount, color_palette);
                    }
                    
                    C += PaletteS.Pos;
                    EatLeadingSpaces(&C);
                    u32 PLen = CountToNewline(C);
                    PaletteNames[Result.PaletteCount] = NewStringCompound(&GameState->FixArena, COLOR_PALETTE_MAX_NAME_LENGTH+1);
                    CopyStringToCompound(PaletteNames+Result.PaletteCount, C, (u8)'\n', true);
                    
                    AdvanceToNewline(&C);
                    color_palette *Palette = Palettes+Result.PaletteCount;
                    
                    ProcessNextPaletteColor(&C, TextS.Pos,             &Palette->Text);
                    ProcessNextPaletteColor(&C, ForegroundTextS.Pos,   &Palette->ForegroundText);
                    ProcessNextPaletteColor(&C, ErrorTextS.Pos,        &Palette->ErrorText);
                    ProcessNextPaletteColor(&C, ForegroundS.Pos,       &Palette->Foreground);
                    ProcessNextPaletteColor(&C, SlotS.Pos,             &Palette->Slot);
                    ProcessNextPaletteColor(&C, SliderBackgroundS.Pos, &Palette->SliderBackground);
                    ProcessNextPaletteColor(&C, SliderGrabThingS.Pos,  &Palette->SliderGrabThing);
                    ProcessNextPaletteColor(&C, ButtonActiveS.Pos,     &Palette->ButtonActive);
                    ProcessNextPaletteColor(&C, SelectedS.Pos,         &Palette->Selected);
                    ProcessNextPaletteColor(&C, PlayingSongS.Pos,      &Palette->PlayingSong);
                    
                    ++Result.PaletteCount;
                    continue; // We do this, because in ProcessNextPaletteColor we do a AdvancetoNewline.
                }
                else if(StringCompare(C, ActivePlaylistS.S, 0, ActivePlaylistS.Pos))
                {
                    C += ActivePlaylistS.Pos;
                    EatLeadingSpaces(&C);
                    u32 PLen = CountToDelimeters(C, (u8 *)"\r\n", 2);
                    Result.ActivePlaylist = NewStringCompound(&GameState->FixArena, PLen);
                    CopyStringToCompound(&Result.ActivePlaylist, C, 0u, PLen);
                }
                
                AdvanceToNewline(&C);
            }
            
            Result.PaletteMaxCount = Result.PaletteCount+10;
            Result.PaletteNames    = AllocateArray(&GameState->FixArena, Result.PaletteMaxCount, string_c);
            Result.Palettes        = AllocateArray(&GameState->FixArena, Result.PaletteMaxCount, color_palette);
            For(Result.PaletteCount)
            {
                Result.PaletteNames[It] = PaletteNames[It];
                Result.Palettes[It]     = Palettes[It];
            }
            
        }
    }
    
    // If anything is wrong with the saved values, just use the default percentages
    if(Result.PlaylistsGenreEdgeXPercent < 0.0f || Result.PlaylistsGenreEdgeXPercent > 1.0f || 
       Result.GenreArtistEdgeXPercent    < 0.0f || Result.GenreArtistEdgeXPercent    > 1.0f || 
       Result.ArtistAlbumEdgeXPercent    < 0.0f || Result.ArtistAlbumEdgeXPercent    > 1.0f ||
       Result.AlbumSongEdgeXPercent      < 0.0f || Result.AlbumSongEdgeXPercent      > 1.0f || 
       Result.PlaylistsGenreEdgeXPercent >= Result.GenreArtistEdgeXPercent ||
       Result.GenreArtistEdgeXPercent    >= Result.ArtistAlbumEdgeXPercent ||
       Result.ArtistAlbumEdgeXPercent    >= Result.AlbumSongEdgeXPercent) 
    {
        Result.PlaylistsGenreEdgeXPercent = GameState->Layout.PlaylistsGenreXP;
        Result.GenreArtistEdgeXPercent    = GameState->Layout.GenreArtistXP;
        Result.ArtistAlbumEdgeXPercent    = GameState->Layout.ArtistAlbumXP;
        Result.AlbumSongEdgeXPercent      = GameState->Layout.AlbumSongXP;
    }
    
    if(Result.WindowDimX < GlobalMinWindowWidth)  Result.WindowDimX = GlobalMinWindowWidth;
    if(Result.WindowDimY < GlobalMinWindowHeight) Result.WindowDimY = GlobalMinWindowHeight;
    
    return Result;
}

internal void
SaveSettingsFile(game_state *GameState, settings *Settings)
{
    string_c SaveData = NewStringCompound(&GameState->ScratchArena, 50000);
    
    AppendStringToCompound(&SaveData, (u8 *)"MPlay3Settings\nVersion ");
    I32ToString(&SaveData, SETTINGS_CURRENT_VERSION);
    
    AppendStringToCompound(&SaveData, (u8 *) "\n\n# NOTE 1:: If you don't want this file and the Library\n# file to be in the same directory as the executable, you\n# can put them into the '%appdata%\\Roaming\\MPlay3Data'\n# folder. If they can be found -by the program- in\n# that specific location, it will overwrite them in there.\n\n");
    
    AppendStringToCompound(&SaveData, (u8 *) "# NOTE 2:: If you want to use a different font than the\n# default, you can add it here right behind the 'FontPath'\n# identifier. Only *.ttf files work! With FontHeightOffset\n# the vertical position of the text can be adjusted\n# (only integers allowed).\n\n");
    
    NewLocalString(FontPath,          280, "FontPath: ");
    NewLocalString(FileFontOffset,     50, "FontHeightOffset: ");
    NewLocalString(FileVolume,         50, "Volume: ");
    NewLocalString(FileLastSong,       50, "LastPlayingSong: ");
    NewLocalString(FileColorPalette,   50, "ColorPalette: ");
    NewLocalString(FilePlaylistsGenre, 50, "PlaylistsGenreEdgeXPercent: ");
    NewLocalString(FileGenreArtist,    50, "GenreArtistEdgeXPercent: ");
    NewLocalString(FileArtistAlbum,    50, "ArtistAlbumEdgeXPercent: ");
    NewLocalString(FileAlbumSong,      50, "AlbumSongEdgeXPercent: ");
    NewLocalString(WindowDimX,         50, "WindowDimensionX: ");
    NewLocalString(WindowDimY,         50, "WindowDimensionY: ");
    NewLocalString(Looping,            50, "Looping: ");
    NewLocalString(Shuffle,            50, "Shuffle: ");
    NewLocalString(CachedFontNames,   500, "UsedFontCache: ");
    NewLocalString(ActivePlaylist, PLAYLIST_MAX_NAME_LENGTH+50, "ActivePlaylist: ");
    
    v2i Dim = GetWindowSize();
    file_id FileID = NewFileID(-1);
    if(GameState->MusicInfo.PlayingSong.PlaylistID >= 0) 
        FileID = GetFileID(&GameState->MusicInfo.Playlist_->Song, GameState->MusicInfo.PlayingSong.PlaylistID);
    AppendStringCompoundToCompound(&FontPath, &Settings->FontPath);
    I32ToString(&FileFontOffset, Settings->FontHeightOffset);
    R32ToString(&FileVolume, GameState->SoundThreadInterface->ToneVolume);
    I32ToString(&FileLastSong, FileID.ID);
    I32ToString(&FileColorPalette, GameState->MusicInfo.DisplayInfo.ColorPaletteID);
    R32ToString(&FilePlaylistsGenre, GameState->MusicInfo.DisplayInfo.PlaylistsGenre.XPercent);
    R32ToString(&FileGenreArtist, GameState->MusicInfo.DisplayInfo.GenreArtist.XPercent);
    R32ToString(&FileArtistAlbum, GameState->MusicInfo.DisplayInfo.ArtistAlbum.XPercent);
    R32ToString(&FileAlbumSong, GameState->MusicInfo.DisplayInfo.AlbumSong.XPercent);
    I32ToString(&WindowDimX, Dim.x);
    I32ToString(&WindowDimY, Dim.y);
    I32ToString(&Looping, GameState->MusicInfo.Playlists.List[0].Looping == playLoop_Loop);
    I32ToString(&Shuffle, GameState->MusicInfo.Playlists.List[0].IsShuffled);
    
    AppendStringCompoundToCompound(&ActivePlaylist, GetPlaylistName(&GameState->MusicInfo, GameState->MusicInfo.Playlist_));
    i32 NameEndP = FindLastOccurrenceOfCharInStringCompound(&ActivePlaylist, '(');
    Assert(NameEndP >= 0);
    ActivePlaylist.Pos = NameEndP-1;
    
    string_c LB   = NewStaticStringCompound("\n");
    ConcatStringCompounds(30, &SaveData, &FontPath, &LB, &FileFontOffset, &LB, &FileVolume, &LB, &FileLastSong, &LB, &FileColorPalette, &LB, &FilePlaylistsGenre, &LB, &FileGenreArtist, &LB, &FileArtistAlbum, &LB, &FileAlbumSong, &LB, &WindowDimX, &LB, &WindowDimY, &LB, &Looping, &LB, &Shuffle, &LB, &CachedFontNames, &LB, &ActivePlaylist);
    
    // Save out used font names
    if(Settings->CachedFontNames)
    {
        For(Settings->CachedFontNames->Count)
        {
            AppendStringCompoundToCompound(&SaveData, Settings->CachedFontNames->Names+It);
            AppendCharToCompound(&SaveData, '|');
        }
    }
    AppendCharToCompound(&SaveData, '\n');
    
    // Save out user created color palettes
    string_c Palette           = NewStaticStringCompound("\nPalette: ");
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
    
    
    if(!WriteEntireFile(&GameState->ScratchArena, GameState->SettingsPath.S, SaveData.Pos, SaveData.S))
    {
        NewLocalString(ErrorMsg, 300, "ERROR:: Could not write out settings file!");
        PushErrorMessage(GameState, ErrorMsg);
        DebugLog(255, "%s\n", ErrorMsg.S);
    }
    DeleteStringCompound(&GameState->ScratchArena, &SaveData);
}

inline void
ApplySettings(game_state *GameState, settings Settings)
{
    ChangeVolume(GameState, Settings.Volume);
    music_info *MusicInfo = &GameState->MusicInfo;
    
    
    playlist_info *Playlist = GetPlaylist(GameState, Settings.ActivePlaylist);
    if(Playlist) SwitchPlaylist(GameState, Playlist);
    else Playlist = GameState->MusicInfo.Playlist_;
    
    MusicInfo->PlayingSong.PlaylistID    = GetPlaylistID(&MusicInfo->Playlist_->Song, Settings.PlayingSongID);
    MusicInfo->PlayingSong.DisplayableID = GetDisplayableID(MusicInfo, MusicInfo->PlayingSong.PlaylistID);
    
    ChangeSong(GameState, &MusicInfo->PlayingSong); 
    
    
    MusicInfo->DisplayInfo.ColorPaletteID = Settings.ColorPaletteID; 
    UpdateColorPalette(&MusicInfo->DisplayInfo, false);
    
    MusicInfo->DisplayInfo.PlaylistsGenre.XPercent = Settings.PlaylistsGenreEdgeXPercent;
    MusicInfo->DisplayInfo.GenreArtist.XPercent = Settings.GenreArtistEdgeXPercent;
    MusicInfo->DisplayInfo.ArtistAlbum.XPercent = Settings.ArtistAlbumEdgeXPercent;
    MusicInfo->DisplayInfo.AlbumSong.XPercent = Settings.AlbumSongEdgeXPercent;
    
    ProcessEdgeDragOnResize(&GameState->Renderer, &MusicInfo->DisplayInfo);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Genre, Playlist->Genre.Displayable.A.Count);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Artist, Playlist->Artist.Displayable.A.Count);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Album, Playlist->Album.Displayable.A.Count);
    FitDisplayColumnIntoSlot(&GameState->Renderer, &MusicInfo->DisplayInfo.Song.Base, Playlist->Song.Displayable.A.Count);
    UpdateHorizontalSliders(MusicInfo);
    
    playlist_id PlaylistID = GetPlaylistID(&MusicInfo->Playlist_->Song, Settings.PlayingSongID);
    if(PlaylistID >= 0)
    {
        BringDisplayableEntryOnScreen(MusicInfo, &MusicInfo->DisplayInfo.Genre,     PlaylistID);
        BringDisplayableEntryOnScreen(MusicInfo, &MusicInfo->DisplayInfo.Artist,    PlaylistID);
        BringDisplayableEntryOnScreen(MusicInfo, &MusicInfo->DisplayInfo.Album,     PlaylistID);
        BringDisplayableEntryOnScreen(MusicInfo, &MusicInfo->DisplayInfo.Song.Base, PlaylistID);
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Library file specification ~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
// >FILE_HASH
// 
// >FILE_2_NAME
// ...
// >FILE_1_DURATION
// >FILE_HASH
//
// P: SUB_PATH_2
//
// >FILE_1_NAME
// ...
// >FILE_1_DURATION
// >FILE_HASH
// 

internal b32
ConfirmLibraryWithCorrectVersionExists(game_state *GameState, u32 VersionToCheckFor, u32 *FileInfoCount)
{
    b32 Result = false;
    read_file_result Data = {};
    u32 BegginingCount = StringLength((u8 *)"MP3Lib\nVersion XX\nP: \nC: XXXXX\n");
    if(ReadBeginningOfFile(&GameState->ScratchArena, &Data, GameState->LibraryPath.S, BegginingCount+255))
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
        FreeFileMemory(&GameState->ScratchArena, Data);
    }
    
    return Result;
}

internal b32
ConfirmLibraryMusicPathExists(game_state *GameState)
{
    b32 Result = false;
    
    u32 BegginingCount = StringLength((u8 *)"MP3Lib\nVersion XX\nP: \nC: XXXXX\n");
    read_file_result File = {};
    if(ReadBeginningOfFile(&GameState->ScratchArena, &File, GameState->LibraryPath.S, BegginingCount+256))
    {
        u8 *C = File.Data;
        
        AdvanceToNewline(&C); // MP3Lib
        AdvanceToNewline(&C); // Version XX
        C += 3; // P:_
        NewEmptyLocalString(FolderPath, 256);
        C += CopyUntilNewline(C, &FolderPath);
        AppendCharToCompound(&FolderPath, '*');
        
        string_w FolderPathWide = {};
        ConvertString8To16(&GameState->ScratchArena, &FolderPath, &FolderPathWide);
        
        WIN32_FIND_DATAW FileData = {};
        HANDLE FileHandle = FindFirstFileExW(FolderPathWide.S, FindExInfoBasic, &FileData, 
                                             FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
        Result = FileHandle != INVALID_HANDLE_VALUE;
    }
    
    return Result;
}

internal b32
CompareMP3LibraryFileSavedPath(game_state *GameState, string_c *PathToCompare)
{
    b32 Result = false;
    read_file_result Data = {};
    if(ReadEntireFile(&GameState->ScratchArena, &Data, GameState->LibraryPath.S))
    {
        u8 *C = Data.Data;
        if(StringCompare(C, (u8 *)"MP3Lib", 0, 6))
        {
            AdvanceToNewline(&C); // MP3Lib
            AdvanceToNewline(&C); // Version XX
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
    
    return Result;
}

internal void
LoadMP3LibraryFile(game_state *GameState, mp3_info *Info)
{
    read_file_result Data = {};
    if(ReadEntireFile(&GameState->ScratchArena, &Data, GameState->LibraryPath.S))
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
                MP3FileInfo->FileNames_[It] = NewStringCompound(&GameState->JobThreadsArena, CountToNewline(++C));
                C += CopyUntilNewline(C, MP3FileInfo->FileNames_+It);
                
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
                    DeleteStringCompound(&GameState->ScratchArena, &Duration);
                    MD->FoundFlags |= metadata_Duration;
                }
                else AdvanceToNewline(&C);
                
                MP3FileInfo->Count_++;
                AdvanceToNewline(&C);
            }
            DeleteStringCompound(&GameState->ScratchArena, &CurrentSubPath);
        }
        FreeFileMemory(&GameState->ScratchArena, Data);
    }
}

internal void
SaveMP3LibraryFile(game_state *GameState, mp3_info *Info)
{
    string_c NL    = NewStaticStringCompound("\n");
    string_c Colon = NewStaticStringCompound("P: ");
    string_c Inset = NewStaticStringCompound(">");
    
    mp3_file_info *MP3FileInfo = &Info->FileInfo;
    if(MP3FileInfo->Count_ == 0) return;
    u32 StringSize = (30+255+5) + 7*255*MP3FileInfo->Count_;
    string_c SaveData = NewStringCompound(&GameState->ScratchArena, StringSize);
    
    AppendStringToCompound(&SaveData, (u8 *)"MP3Lib\n");
    AppendStringToCompound(&SaveData, (u8 *)"Version ");
    I32ToString(&SaveData, LIBRARY_CURRENT_VERSION);
    AppendCharToCompound(&SaveData, '\n');
    ConcatStringCompounds(4, &SaveData, &Colon, &Info->FolderPath, &NL);
    char CountS[25];
    sprintf_s(CountS, "C: %i\n\n", MP3FileInfo->Count_);
    AppendStringToCompound(&SaveData, (u8 *)CountS);
    
    u32 WrittenDataCount = 0;
    b32 *Written = AllocateArray(&GameState->ScratchArena, Info->FileInfo.MaxCount_, b32);
    string_c CurrentSubPath = NewStringCompound(&GameState->ScratchArena, 255);
    while(WrittenDataCount < MP3FileInfo->Count_)
    {
        AppendStringCompoundToCompound(&SaveData, &Colon);
        For(MP3FileInfo->Count_)
        {
            if(!Written[It])
            {
                ConcatStringCompounds(4, &SaveData, MP3FileInfo->SubPath+It, &NL, &NL);
                AppendStringCompoundToCompound(&CurrentSubPath, MP3FileInfo->SubPath+It);
                break;
            }
        }
        For(MP3FileInfo->Count_)
        {
            if(!Written[It] && 
               CompareStringCompounds(&CurrentSubPath, MP3FileInfo->SubPath+It))
            {
                mp3_metadata *MD = MP3FileInfo->Metadata+It;
                
                ConcatStringCompounds(4, &SaveData, &Inset, MP3FileInfo->FileNames_[It], &NL);
                if(MD->FoundFlags & metadata_Title)
                {
                    ConcatStringCompounds(4, &SaveData, &Inset, MD->Title, &NL);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                if(MD->FoundFlags & metadata_Artist)
                {
                    ConcatStringCompounds(4, &SaveData, &Inset, MD->Artist, &NL);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                if(MD->FoundFlags & metadata_Album)
                {
                    ConcatStringCompounds(4, &SaveData, &Inset, MD->Album, &NL);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                if(MD->FoundFlags & metadata_Genre)
                {
                    ConcatStringCompounds(4, &SaveData, &Inset, MD->Genre, &NL);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                if(MD->FoundFlags & metadata_Track && MD->Track > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Track);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                if(MD->FoundFlags & metadata_Year && MD->Year > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Year);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                if(MD->FoundFlags & metadata_Duration && MD->Duration > 0)
                {
                    char B[25];
                    sprintf_s(B, ">%i\n", MD->Duration);
                    AppendStringToCompound(&SaveData, (u8 *)B);
                }
                else ConcatStringCompounds(3, &SaveData, &Inset, &NL);
                
                // NoHash:: AppendStringCompoundToCompound(&SaveData, &Inset);
                // NoHash:: U32ToString(&SaveData, MP3FileInfo->Hashes[It]);
                // NoHash:: ConcatStringCompounds(3, &SaveData, &NL, &NL);
                
                AppendStringCompoundToCompound(&SaveData, &NL);
                Written[It] = true;
                WrittenDataCount++;
            }
        }
        
        WipeStringCompound(&CurrentSubPath);
    }
    DeleteStringCompound(&GameState->ScratchArena, &CurrentSubPath);
    FreeMemory(&GameState->ScratchArena, Written);
    
    // TODO:: Rename old save file as backup, before writing and after successful write delete the old one.
    if(!WriteEntireFile(&GameState->ScratchArena, GameState->LibraryPath.S, SaveData.Pos, SaveData.S))
    {
        NewLocalString(ErrorMsg, 300, "ERROR:: Could not write out library file!");
        PushErrorMessage(GameState, ErrorMsg);
        DebugLog(255, "%s\n", ErrorMsg.S);
    }
    DeleteStringCompound(&GameState->ScratchArena, &SaveData);
}

internal void
WipeMP3LibraryFile(game_state *GameState)
{
    if(!WriteEntireFile(&GameState->ScratchArena, GameState->LibraryPath.S, 0, 0))
    {
        NewLocalString(ErrorMsg, 300, "ERROR:: Could not access library file!");
        PushErrorMessage(GameState, ErrorMsg);
        DebugLog(255, "%s\n", ErrorMsg.S);
    }
}

internal loaded_bitmap
DecodeIcon(arena_allocator *Arena, u32 Width, u32 Height, u8 *Data, u32 Size)
{
    loaded_bitmap Result = {1, };
    Result.Width = Width;
    Result.Height = Height;
    Result.Pixels = AllocateArray(Arena, Width*Height, u32);
    u8 *Pixel = (u8 *)Result.Pixels;
    For(Size)
    {
        u8 Count = *Data++;
        For(Count, P) 
        {
            
            *Pixel++ = Data[0];
            *Pixel++ = Data[1];
            *Pixel++ = Data[2];
            *Pixel++ = Data[3];
        }
        Data += 4;
        It   += 4;
    }
    
    return Result;
}

/* NoHash::
internal u32
CreateHash(string_c Name, u64 CreationDate)
{
    u64 Result = CreationDate;
    
    u32 Prime  = 176544817;
    u64 Prime2 = 184467439259;
    
    For(Name.Pos) 
    {
        if(It%2) Result += Name.S[It]*Prime;
        else     Result += Name.S[It]*Prime2;
        Result = Result%MAX_UINT32;
    }
    
    return (u32)Result;
}
*/

enum existing_file_status
{
    existingFileStatue_NoExist,
    existingFileStatue_ExistDifferentCreation,
    existingFileStatue_ExistAndMatch,
};

internal existing_file_status
CheckForCreationTimeSimilarity(game_state *GS, string_c PlaylistPath, u64 CreationTime)
{
    existing_file_status Result = existingFileStatue_NoExist;
    
    string_w WidePlaylistPath = {};
    ConvertString8To16(&GS->ScratchArena, &PlaylistPath, &WidePlaylistPath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WidePlaylistPath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        u64 NewCreationTime = (u64)FileData.ftCreationTime.dwHighDateTime << 32 | FileData.ftCreationTime.dwLowDateTime;
        if(NewCreationTime == CreationTime)
        {
            Result = existingFileStatue_ExistAndMatch;
        }
        else Result = existingFileStatue_ExistDifferentCreation;
    }
    
    return Result;
}

internal u64
GetFileCreationDate(game_state *GS, string_c PlaylistPath)
{
    u64 Result = 0;
    
    string_w WidePlaylistPath = {};
    ConvertString8To16(&GS->ScratchArena, &PlaylistPath, &WidePlaylistPath);
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WidePlaylistPath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        Result = (u64)FileData.ftCreationTime.dwHighDateTime << 32 | FileData.ftCreationTime.dwLowDateTime;
    }
    
    return Result;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Playlist save file definition: ~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// MPlay3Playlist
// Version: <VERSION>
// Name: <PLAYLIST_NAME>
// Count: <SONG_COUNT>
// Shuffled: <0 or 1>
// Looping: <0 or 1>
//
// # NOTE 1:: These playlist files will always 
// # be placed at the same location as the 
// # 'MPlay3Library.save' file. For a bit more 
// # information on what that means, look into
// # the 'MPlay3Settings.save' file's NOTE 1.
//
// # NOTE 2:: The filename of the playlist files
// # needs to start with the string 'MPlay3Playlist_*'
// # and end with '.save'. If you want, it is
// # possible to put anything inbetween these, 
// # to sort them as you like.
// 
// P: <SUB_PATH_1>
// ><FILENAME_1>
// ><FILENAME_...>
// ><FILENAME_N>
// 
// P: <SUB_PATH_2>
// >...
// 
// P: <SUB_PATH_N>
// ><FILENAME_M>
//

internal void
SavePlaylist(game_state *GS, playlist_info *Playlist)
{
    mp3_file_info *FileInfo = &GS->MP3Info->FileInfo;
    i32 PlaylistID = GetPlaylistID(&GS->MusicInfo, Playlist);
    Assert(PlaylistID >= 0);
    Assert(PlaylistID != 0); // First playlist is always _all_ songs, which does not need to be saved.
    
    string_c SaveData = NewStringCompound(&GS->ScratchArena, 1500);
    
    AppendStringToCompound(&SaveData, (u8 *)"MPlay3Playlist\nVersion: ");
    I32ToString(&SaveData, PLAYLIST_CURRENT_VERSION);
    
    AppendStringToCompound(&SaveData, (u8 *)"\nName: ");
    NewLocalString(PLName, PLAYLIST_MAX_NAME_LENGTH, Playlist->Playlists.Batch.Names[PlaylistID].S);
    i32 NameEndP = FindLastOccurrenceOfCharInStringCompound(&PLName, '(');
    Assert(NameEndP >= 0);
    PLName.Pos = NameEndP-1;
    AppendStringCompoundToCompound(&SaveData, &PLName);
    
    AppendStringToCompound(&SaveData, (u8 *)"\nCount: ");
    I32ToString(&SaveData, Playlist->Song.FileIDs.A.Count);
    
    AppendStringToCompound(&SaveData, (u8 *)"\nShuffled: ");
    I32ToString(&SaveData, Playlist->IsShuffled);
    
    AppendStringToCompound(&SaveData, (u8 *)"\nLooping: ");
    I32ToString(&SaveData, Playlist->Looping);
    
    AppendStringToCompound(&SaveData, (u8 *)"\n\n");
    
    AppendStringToCompound(&SaveData, (u8 *)"# NOTE 1:: These playlist files will always\n# be placed at the same location as the\n# 'MPlay3Library.save' file. For a bit more\n# information on what that means, look into\n# the 'MPlay3Settings.save' file's NOTE 1.\n\n");
    AppendStringToCompound(&SaveData, (u8 *)"# NOTE 2:: The filename of the playlist files\n# need to start with the string 'MPlay3Playlist_'\n# and end with '.save'. If you want, it is\n# possible to put anything inbetween these,\n# to sort them as you like.\n\n");
    
    string_c SMem = NewStaticStringCompound("ThisIsNoSubPathIJustWantToMakeSureThatTheFirstLoopCompareAlwaysFails!");
    string_c *CurrentSubPath = &SMem;
    
    // Because the Playlists can have the FileIDs list in any random order, we sort them
    // before saving to the file. This bunches up all subpath again, making the file much
    // more readable and a bit smaller as well.
    array_u32 SortFileIDs = CreateArray(&GS->ScratchArena, Playlist->Song.FileIDs.A.Count);
    AppendArray(&SortFileIDs, &Playlist->Song.FileIDs.A);
    ShuffleStack(&SortFileIDs);
    QuickSort3(SortFileIDs);
    
    For(SortFileIDs.Count)
    {
        file_id FileID = NewFileID(Get(&SortFileIDs, It));
        
        if(!CompareStringCompounds(CurrentSubPath, FileInfo->SubPath+FileID.ID))
        {
            CurrentSubPath = FileInfo->SubPath+FileID.ID;
            AppendStringToCompound(&SaveData, (u8 *)"\nP: ");
            AppendStringCompoundToCompound(&SaveData, CurrentSubPath);
            AppendCharToCompound(&SaveData, '\n');
        }
        
        AppendCharToCompound(&SaveData, '>');
        AppendStringCompoundToCompound(&SaveData, FileInfo->FileNames_+FileID.ID);
        AppendCharToCompound(&SaveData, '\n');
        
        if(SaveData.Pos + 260 >= SaveData.Length) 
        {
            u32 NewLength = SaveData.Length*2;
            SaveData.S = ReallocateMemory(&GS->ScratchArena, SaveData.S, SaveData.Length, NewLength);
            SaveData.Length = NewLength;
        }
    }
    
    
    // When we create a new playlist, we need to make sure that there is not already
    // a playlist with the exact same name. If so we try again and increment the name
    // each time. 
    NewLocalString(Appendix, 5, "0");
    u32 AppendixCount = 0;
    NewEmptyLocalString(PlaylistPath, 260);
    existing_file_status FileExist = existingFileStatue_ExistAndMatch;
    do {
        if(Playlist->Filename_.Pos) AppendStringCompoundToCompound(&PlaylistPath, &Playlist->Filename_);
        else
        {
            ResetStringCompound(PlaylistPath);
            AppendStringCompoundToCompound(&PlaylistPath, &GS->PlaylistPath);
            if(PlaylistID < 100) AppendCharToCompound(&PlaylistPath, '0');
            if(PlaylistID < 10)  AppendCharToCompound(&PlaylistPath, '0');
            I32ToString(&PlaylistPath, PlaylistID);
            AppendCharToCompound(&PlaylistPath, '_');
            AppendStringCompoundToCompound(&PlaylistPath, &Appendix);
            AppendStringToCompound(&PlaylistPath, (u8 *)".save");
            
            FileExist = CheckForCreationTimeSimilarity(GS, PlaylistPath, Playlist->FileCreationDate);
            
            if(FileExist != existingFileStatue_ExistDifferentCreation)
            {
                // If we are here and build our own path, we also need to save it for later.
                Playlist->Filename_ = NewStringCompound(&GS->FixArena, PlaylistPath.Pos);
                AppendStringCompoundToCompound(&Playlist->Filename_, &PlaylistPath);
            }
            ResetStringCompound(Appendix);
            I32ToString(&Appendix, ++AppendixCount);
        }
    } while(FileExist == existingFileStatue_ExistDifferentCreation);
    
    // TODO:: Rename old save file as backup, before writing and after successful write delete the old one.
    if(!WriteEntireFile(&GS->ScratchArena, PlaylistPath.S, SaveData.Pos, SaveData.S))
    {
        NewLocalString(ErrorMsg, 300, "ERROR:: Could not write out playlist file file nr. ");
        I32ToString(&ErrorMsg, PlaylistID);
        AppendCharToCompound(&ErrorMsg, '!');
        PushErrorMessage(GS, ErrorMsg);
        DebugLog(255, "%s\n", ErrorMsg.S);
    }
    else
    {
        Playlist->FileCreationDate = GetFileCreationDate(GS, PlaylistPath);
    }
    DeleteStringCompound(&GS->ScratchArena, &SaveData);
}

enum load_playlist_state
{
    loadPlaylistState_NothingFound         = -4,
    loadPlaylistState_WrongIdentifier      = -3,
    loadPlaylistState_WrongVersion         = -2,
    loadPlaylistState_LoadedButEmpty       = -1,
    loadPlaylistState_LoadedSucessfully    =  0,
    loadPlaylistState_LoadedNeedsReSaving  =  1,
};

struct load_playlist_result
{
    load_playlist_state LoadState;
    array_file_id FileIDs;
    string_c Name;
    b32 Shuffled;
    b32 Looping;
};

internal load_playlist_result
LoadPlaylist(game_state *GS, string_c PlaylistPath)
{
    load_playlist_result Result = {loadPlaylistState_NothingFound};
    
    mp3_file_info *FileInfo = &GS->MP3Info->FileInfo;
    
    read_file_result Data = {};
    if(ReadEntireFile(&GS->ScratchArena, &Data, PlaylistPath.S))
    {
        u8 *C = Data.Data;
        u8 L;
        
        string_c PlaylistFileID = NewStaticStringCompound("MPlay3Playlist");
        if(!CompareStringAndCompound(&PlaylistFileID, C, PlaylistFileID.Pos)) 
        {
            NewLocalString(ErrorMsg, 300, "ERROR:: Playlist file is not a playlist file. It has to have 'MPlay3Playlist' at the beginning!");
            PushErrorMessage(GS, ErrorMsg);
            DebugLog(302, "%s\n", ErrorMsg.S);
            Result.LoadState = loadPlaylistState_WrongIdentifier;
            return Result;
        }
        AdvanceToNewline(&C);
        
        NewLocalString(VersionString, 20, "Version: ");
        I32ToString(&VersionString, PLAYLIST_CURRENT_VERSION);
        if(!CompareStringAndCompound(&VersionString, C, VersionString.Pos)) 
        {
            C += 9; // "Version: "
            i32 V = ProcessNextI32InString(C, '\n', L);
            
            NewLocalString(ErrorMsg, 300, "ERROR:: Playlist file is the wrong Version. Wanted: ");
            I32ToString(&ErrorMsg, PLAYLIST_CURRENT_VERSION);
            AppendStringToCompound(&ErrorMsg, (u8 *)", found: ");
            I32ToString(&ErrorMsg, V);
            AppendCharToCompound(&ErrorMsg, '.');
            PushErrorMessage(GS, ErrorMsg);
            DebugLog(255, "%s\n", ErrorMsg.S);
            Result.LoadState = loadPlaylistState_WrongVersion;
            return Result;
        }
        AdvanceToNewline(&C);
        
        C += 6; // "Name: "
        u32 HardLimit = PLAYLIST_MAX_NAME_LENGTH + 10;  // @HardLimit, Limit to 100 chars for now.
        u32 NameLength = CountToNewline(C);
        Result.Name  = NewStringCompound(&GS->ScratchArena, HardLimit);
        CopyStringToCompound(&Result.Name, C, 0u, Min(NameLength, HardLimit));
        AdvanceToNewline(&C);
        
        C += 7; // "Count: "
        u32 SongCount      = ProcessNextI32InString(C, '\n', L);
        Result.FileIDs.A = CreateArray(&GS->ScratchArena, FileInfo->Count_);
        AdvanceToNewline(&C);
        
        C += 10; // "Shuffled: "
        Result.Shuffled = ProcessNextB32InString(C);
        AdvanceToNewline(&C);
        
        C += 9; // "Looping: "
        Result.Looping = ProcessNextB32InString(C);
        AdvanceToNewline(&C);
        
        NewEmptyLocalString(CurrentSubPath, 260); 
        NewEmptyLocalString(Filename, 260); 
        NewLocalString(PathID, 4, "P: ");
        file_id SubPathStartID = {0};
        For(SongCount)
        {
            if(*C == 0) break;
            while(*C == '#' || *C == '\r' || *C == '\n') AdvanceToNewline(&C);
            if(CompareStringAndCompound(&PathID, C, PathID.Pos)) 
            {
                C += 3; // "P: "
                WipeStringCompound(&CurrentSubPath);
                CopyStringToCompound(&CurrentSubPath, C, (u8 *)"\n\r", 2);
                AdvanceToNewline(&C);
                SubPathStartID = GetFirstFileID(FileInfo, &CurrentSubPath);
            }
            
            if(*C == 0)   break;
            if(*C != '>') AdvanceToNewline(&C);
            if(*C == 0)   break;
            
            C += 1; // '>'
            WipeStringCompound(&Filename);
            CopyStringToCompound(&Filename, C, (u8 *)"\n\r", 2);
            
            file_id FileID = GetFileID(FileInfo, &CurrentSubPath, &Filename, SubPathStartID);
            if(FileID >= 0) Push(&Result.FileIDs, FileID);
            
            AdvanceToNewline(&C);
        }
        
        if(Result.FileIDs.A.Count == 0 && SongCount > 0)
        {
            NewLocalString(ErrorMsg, 300, "WARNING:: Playlist '");
            AppendStringCompoundToCompound(&ErrorMsg, &Result.Name);
            AppendStringToCompound(&ErrorMsg, (u8 *)"' had a count of ");
            I32ToString(&ErrorMsg, SongCount);
            AppendStringToCompound(&ErrorMsg, (u8 *)", but nothing could be found.");
            
            DebugLog(150, "%s\n", ErrorMsg.S);
            PushErrorMessage(GS, ErrorMsg);
            Result.LoadState = loadPlaylistState_LoadedButEmpty;
        }
        else if(Result.FileIDs.A.Count != SongCount) 
        {
            NewLocalString(ErrorMsg, 300, "WARNING:: Playlist '");
            AppendStringCompoundToCompound(&ErrorMsg, &Result.Name);
            AppendStringToCompound(&ErrorMsg, (u8 *)"' indicated a song count of ");
            I32ToString(&ErrorMsg, SongCount);
            AppendStringToCompound(&ErrorMsg, (u8 *)", but we found only ");
            I32ToString(&ErrorMsg, Result.FileIDs.A.Count);
            AppendStringToCompound(&ErrorMsg, (u8 *)" songs in it.");
            
            DebugLog(150, "%s\n", ErrorMsg.S);
            PushErrorMessage(GS, ErrorMsg);
            Result.LoadState = loadPlaylistState_LoadedNeedsReSaving;
        }
        else Result.LoadState = loadPlaylistState_LoadedSucessfully;
        
        AppendStringToCompound(&Result.Name, (u8 *)" (");
        I32ToString(&Result.Name, Result.FileIDs.A.Count);
        AppendCharToCompound(&Result.Name, ')');
        
        FreeFileMemory(&GS->ScratchArena, Data);
    }
    return Result;
}

internal void
LoadAllPlaylists(game_state *GS)
{
    RestartTimer("Load Playlists");
    NewLocalString(PlaylistPath, 260, GS->PlaylistPath.S);
    AppendCharToCompound(&PlaylistPath, '*');
    string_w WidePlaylistPath = {};
    ConvertString8To16(&GS->ScratchArena, &PlaylistPath, &WidePlaylistPath);
    
    NewLocalString(PathWithoutFilePrefix, 260, PlaylistPath.S);
    i32 L = FindLastOccurrenceOfCharInStringCompound(&PathWithoutFilePrefix, '\\') + 1;
    
    WIN32_FIND_DATAW FileData = {};
    HANDLE FileHandle = FindFirstFileExW(WidePlaylistPath.S, 
                                         FindExInfoBasic, 
                                         &FileData, 
                                         FindExSearchNameMatch, 
                                         NULL, 
                                         FIND_FIRST_EX_LARGE_FETCH);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        b32 HasNextFile = true;
        string_compound FileType = NewStringCompound(&GS->ScratchArena, 16);
        
        while(HasNextFile)
        {
            string_c FileName = {};
            ConvertString16To8(&GS->ScratchArena, FileData.cFileName, &FileName);
            
            if     (CompareStringAndCompound(&FileName, (u8 *)".") ) {}
            else if(CompareStringAndCompound(&FileName, (u8 *)"..")) {}
            else
            {
                if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {}
                else
                {
                    i32 LastDot = FindLastOccurrenceOfCharInStringCompound(&FileName, '.');
                    if(LastDot > 0)
                    {
                        PasteStringCompoundIntoCompound(&FileType, 0, &FileName, LastDot, FileName.Pos-LastDot);
                        u8 *FileExtension = (u8 *)".save";
                        if(CompareStringAndCompound(&FileType, FileExtension))
                        {
                            // We found a potential playlist file.
                            PathWithoutFilePrefix.Pos = L;
                            AppendStringCompoundToCompound(&PathWithoutFilePrefix, &FileName);
                            
                            load_playlist_result LoadResult = LoadPlaylist(GS, PathWithoutFilePrefix);
                            
                            if(LoadResult.LoadState == loadPlaylistState_LoadedSucessfully ||
                               LoadResult.LoadState == loadPlaylistState_LoadedNeedsReSaving)
                            {
                                playlist_info *NewPL = CreateEmptyPlaylist(&GS->FixArena, &GS->MusicInfo);
                                NewPL->Filename_ = NewStringCompound(&GS->FixArena, PathWithoutFilePrefix.Pos);
                                AppendStringCompoundToCompound(&NewPL->Filename_, &PathWithoutFilePrefix);
                                
                                FillPlaylistWithFileIDs(&GS->MusicInfo, &GS->MP3Info->FileInfo, NewPL, LoadResult.FileIDs);
                                AddPlaylistToSortingColumn(&GS->MusicInfo, NewPL, LoadResult.Name);
                                SyncPlaylists_playlist_column(&GS->MusicInfo);
                                
                                NewPL->IsShuffled = LoadResult.Shuffled;
                                NewPL->Looping  = LoadResult.Looping ? playLoop_Loop : playLoop_NoLoop;
                                NewPL->FileCreationDate = (u64)FileData.ftCreationTime.dwHighDateTime << 32 | FileData.ftCreationTime.dwLowDateTime;
                                
                                if(LoadResult.LoadState == loadPlaylistState_LoadedNeedsReSaving)
                                    SavePlaylist(GS, NewPL);
                            }
                        }
                        ResetStringCompound(FileType);
                    }
                }
            }
            HasNextFile = FindNextFileW(FileHandle, &FileData);
            DeleteStringCompound(&GS->ScratchArena, &FileName);
        } 
        DeleteStringCompound(&GS->ScratchArena, &FileType);
    }
    SnapTimer("Load Playlists");
}

internal void
SaveShuffledState(game_state *GS, playlist_info *Playlist)
{
    read_file_result FileData = {};
    Assert(Playlist->Filename_.Pos > 0);
    if(ReadBeginningOfFile(&GS->ScratchArena, &FileData, Playlist->Filename_.S, PLAYLIST_MAX_NAME_LENGTH + 80))
    {
        string_c Shuffle = NewStaticStringCompound("Shuffled: ");
        i32 ShufflePos = Find(FileData.Data, Shuffle);
        if(ShufflePos >= 0)
        {
            NewEmptyLocalString(PLShuffle, 2);
            I32ToString(&PLShuffle, Playlist->IsShuffled);
            if(!WriteToFile(&GS->ScratchArena, Playlist->Filename_.S, 1, PLShuffle.S, ShufflePos))
            {
                DebugLog(250, "Could not save shuffle for playlist %s.", Playlist->Filename_.S);
            }
        }
    }
}

internal void
SaveLoopingState(game_state *GS, playlist_info *Playlist)
{
    read_file_result FileData = {};
    Assert(Playlist->Filename_.Pos > 0);
    if(ReadBeginningOfFile(&GS->ScratchArena, &FileData, Playlist->Filename_.S, PLAYLIST_MAX_NAME_LENGTH + 80))
    {
        string_c Looping = NewStaticStringCompound("Looping: ");
        i32 LoopingPos = Find(FileData.Data, Looping);
        if(LoopingPos >= 0)
        {
            NewEmptyLocalString(PLLooping, 2);
            I32ToString(&PLLooping, Playlist->Looping == playLoop_Loop);
            if(!WriteToFile(&GS->ScratchArena, Playlist->Filename_.S, 1, PLLooping.S, LoopingPos))
            {
                DebugLog(250, "Could not save shuffle for playlist %s.", Playlist->Filename_.S);
            }
        }
    }
}


