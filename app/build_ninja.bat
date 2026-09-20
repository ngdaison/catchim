@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1

set CMAKE="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set NINJA="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

echo === Configuring with Ninja ===
%CMAKE% -B build -G Ninja -DCMAKE_MAKE_PROGRAM=%NINJA% -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1

echo === Building ===
%NINJA% -C build
if errorlevel 1 exit /b 1

echo === Running Parity Tests ===
.\build\bin\catchim_tests.exe
