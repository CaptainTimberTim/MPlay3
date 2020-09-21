@echo off

IF NOT EXIST "../build" mkdir "../build"
pushd "../build"

set buildFlags= -DDEBUG_TD=1 -DCONSOLE_APP_
set optimize= -O2

	REM Now using MD instead of -MTd for external symbol resolving of linker
set compilerFlags= -MD -Gm- -nologo -GR- -EHa- -Od -Oi -FC -Z7 
set compilerWarningLevel= -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4005 -wd4239 -wd4706 -wd4127

	REM subsystem for x86 need 5.1 || 'console' for main(), 'windows' for WinMain
set linkerFlags= /link -incremental:no -opt:ref 
	REM -subsystem:windows,5.2 
set libIncludes=  User32.lib Gdi32.lib winmm.lib opengl32.lib
	REM /NODEFAULTLIB:LIBCMT kernel32.lib shell32.lib  glew32s.lib glfw3.lib

	REM This is for OpenGL, also the additional libIncludes, of course
	REM set addIncludeDir= /I "..\data\glew-2.1.0\include" /I "..\data\glfw-3.2.1\include"
	REM set addLinkerLibs= /LIBPATH:"..\data\glew-2.1.0\lib\Release\x64" -LIBPATH:"..\data\glfw-3.2.1\lib-vc2015"

cl -F20000000 -EHsc %compilerFlags% %buildFlags% %compilerWarningLevel% "../code/Main.cpp" %linkerFlags%  %libIncludes% 



REM cl -EHsc %optimize% -nologo %buildFlags% %addIncludeDir% %compilerWarningLevel% "../code/Main.cpp" %linkerFlags% %addLinkerLibs%  %libIncludes% 

popd