@echo off

IF NOT EXIST "../build" mkdir "../build"
pushd "../build"

REM -showIncludes
set flags= -nologo -GR- -EHa- -Z7 -FeMPlay3.exe -F4000000 -DUNICODE -DRESOURCE_FILE=2
set optimizeFlags= -O2 -MT  -Oi -DDEBUG_TD=1
set debugFlags=    -Od -MTd -FC -DDEBUG_TD=1

set compilerWarnings= -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4005 -wd4239 -wd4706 -wd4127 -wd4390

REM subsystem for x86 need 5.1 || 'console' for main(), 'windows' for WinMain
set linkerFlags= /link -incremental:no -opt:ref 

REM -subsystem:windows,5.2 
set libIncludes=  User32.lib Gdi32.lib winmm.lib Shell32.lib Ole32.lib
REM opengl32.lib // Lives now in the GL_TD.h, a bit cleaner?

set resources= "../data/resources/Resources.res"
REM rc -x -nologo -fo %resources% "../data/resources/Resource.rc"

cl %flags% %debugFlags% %compilerWarnings% "../code/Main.cpp" %linkerFlags% %libIncludes% %resources%
REM cl %flags% %optimizeFlags% %compilerWarnings% "../code/Main.cpp" %linkerFlags% %libIncludes% %resources%



REM cl -nologo -GR- -EHa- -Z7 %debugFlags% %compilerWarnings% "../code/ImageToCArray_TD.cpp" %linkerFlags% %libIncludes%

popd