# MPlay3

MPlay3 is a music player that lists and plays all .mp3 files in a given folder. 
It lists all Genres, Artists, Albums and Songs in four different columns. If one or multiple entries in a column are selected all subsequent columns will be adjusted to only display the entries corresponding to the selected ones. <br>
For example: if the genre _rock_ is selected, then only artists that are in the category _rock_ are shown in the artist column. Based on that, only albums created by the artists that are still visible are in the album column and only the songs on those albums are then shown in the song column.<br>
This enables you to very quickly assemble a list of songs. Additional features are basic stuff like music shuffle, looping, a search for each column and mulitple color-palettes.


The base principle for this project is to do almost everything myself. The only libraries included are the <a href="https://github.com/nothings/stb">stb_image.h and stb_truetype.h</a> for image and font loading and <a href="https://github.com/lieff/minimp3">MiniMp3.h</a> for the .mp3 decoding part. Everything else is written by me. With the rare code snippets here and there taken from somewhere else. The end-goal of this project is to replace all external library code at some point.<br>
A small video of MPlay3 and a bit more motivation for this project is on my website: http://if-on-a-summers-night-a-programmer.com/project_hobby_MPlay3.html (The video is a bit old and is missing stuff thats already in the program right now)


<br><br>
## Building the repository

To build this project, the only thing required is the Visual Studio compiler.
I have a "build.bat" in the code directory, which I use for building.
In the "build" directory is a Visual Studio project file, which is only used for debugging.

To use the project exactly like me, all files in the base directory need their paths changed (the path in the shortcut file as well). That said, it is really not necessary to do.
