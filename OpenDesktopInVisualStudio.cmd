@echo off
pushd %~dp0

echo Generating Visual Studio projects with CMake...
cmake --preset vs-x86

if %ERRORLEVEL% neq 0 (
    echo CMake generation failed
    pause
    exit /b %ERRORLEVEL%
)

echo Opening Visual Studio...
start "" devenv "build\vs-x86\clarinoid_desktop.sln"
