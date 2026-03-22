@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 >nul 2>&1
if not exist build (
    cmake -G Ninja -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=cl.exe
    if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
)
cmake --build build %*
