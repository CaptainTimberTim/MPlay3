/* date = October 19th 2020 10:09 am */
#ifndef _SOUND__SERIALIZATION_H
#define _SOUND__SERIALIZATION_H

global_variable string_c SETTINGS_FILE_NAME NewStaticStringCompound("MPlay3Settings.save");

#define LIBRARY_CURRENT_VERSION 3
#define SETTINGS_CURRENT_VERSION 4
struct settings
{
    r32 Volume = 0.5f;
    file_id PlayingSongID = {-1};
    u32 ColorPaletteID = 4;
    r32 GenreArtistEdgeXPercent = -1.0f;
    r32 ArtistAlbumEdgeXPercent = -1.0f;
    r32 AlbumSongEdgeXPercent = -1.0f;
    i32 WindowDimX = 1416;
    i32 WindowDimY = 1039;
    b32 Looping = false;
    b32 Shuffle = false;
    
    string_c *PaletteNames;
    color_palette *Palettes;
    u32 PaletteCount;
    u32 PaletteMaxCount;
};

internal settings TryLoadSettingsFile(game_state *GameState);
internal void SaveSettingsFile(game_state *GameState, settings *Settings);
inline void   ApplySettings(game_state *GameState, settings Settings);

internal b32  ConfirmLibraryWithCorrectVersionExists(game_state *GameState, u32 VersionToCheckFor, u32 *FileInfoCount);
internal b32  CompareMP3LibraryFileSavedPath(game_state *GameState, string_c *PathToCompare);
internal void LoadMP3LibraryFile(game_state *GameState, mp3_info *Info);
internal void SaveMP3LibraryFile(game_state *GameState, mp3_info *Info);
internal void WipeMP3LibraryFile(game_state *GameState);


#endif //_SOUND__SERIALIZATION_H
