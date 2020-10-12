@echo off
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set path="%cd%";"%cd%\code";%path%
"editor"