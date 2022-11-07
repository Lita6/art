@echo off

call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set PATH=d:\programming\github\art\project\cmds;%PATH%

cd project\code
call 4coder
call rdbg
cls