# MPlay3

MPlay3 is a music player that lists and plays all .mp3 files in a given folder (more info: https://handmade.network/p/mplay3/). 
It lists all Genres, Artists, Albums and Songs in four different columns. If one or multiple entries in a column are selected all subsequent columns will be adjusted to only display the entries corresponding to the selected ones. <br>
For example: if the genre _rock_ is selected, then only artists that are in the category _rock_ are shown in the artist column. Based on that, only albums created by the artists that are still visible are in the album column and only the songs on those albums are then shown in the song column.<br>
This enables you to very quickly assemble a list of songs. Additional features are basic stuff like music shuffle, looping, a search for each column and mulitple color-palettes.


The base principle for this project is to do almost everything myself. The only libraries included are the <a href="https://github.com/nothings/stb">stb_image.h and stb_truetype.h</a> for image and font loading and <a href="https://github.com/lieff/minimp3">MiniMp3.h</a> for the .mp3 decoding part. Everything else is written by me. With the rare code snippets here and there taken from somewhere else. The end-goal of this project is to replace all external library code at some point.<br>
A small video of MPlay3 is on youtube: https://www.youtube.com/watch?v=pW9DGD-zUSw&ab_channel=TimDierks

<br><br>
### Only works on Windows!

<br><br>
## Building the repository

To build this project, the only thing required is the Visual Studio compiler and maybe an editor.
I have a "build.bat" in the code directory, which I use for building.
In the "build" directory is a Visual Studio project file, which is only used for debugging.

In the base folder is a C_Environment_Startup.lnk which starts a command prompt, Visual Studio (2019) and the editor 4Coder (http://4coder.net/) through *.bat files.
If one or more of the paths to the other applications are different they can be changed in the corresponding *.bat files. You can find Visual Studio's vcvarsall.bat path in the shell.bat and the path to 4coder in the editor.bat.
