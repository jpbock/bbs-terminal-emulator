@echo off
REM BBS Terminal Emulator - Build Script for Windows
REM Usage: build.bat [Debug|Release]

setlocal enabledelayedexpansion

set BUILD_TYPE=%1
if "%BUILD_TYPE%"==" set BUILD_TYPE=Release

echo ============================================
echo BBS Terminal Emulator Build Script
echo ============================================
echo.
echo Build Type: %BUILD_TYPE%
echo.

REM Check for CMake
where cmake >nul 2>nul
if !errorlevel! neq 0 (
    echo ERROR: CMake not found. Please install CMake 3.20 or later.
    exit /b 1
)

echo CMake found: 
for /f "tokens=*" %%i in ('cmake --version ^| findstr /r "version"') do echo %%i
echo.

REM Create build directory
if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure with CMake
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if !errorlevel! neq 0 (
    echo ERROR: CMake configuration failed
    cd ..
    exit /b 1
)

echo.
echo Building project...
cmake --build . --config %BUILD_TYPE% -- /p:Configuration=%BUILD_TYPE% /p:Platform=x64
if !errorlevel! neq 0 (
    echo ERROR: Build failed
    cd ..
    exit /b 1
)

echo.
echo ============================================
echo Build completed successfully!
echo ============================================
echo.
echo Output: bin\%BUILD_TYPE%\bbs-terminal.exe
echo.

cd ..

REM Ask to run the application
set /p RUN="Run application? (y/n): "
if /i "%RUN%"=="y" (
    echo Starting application...
    start build\bin\%BUILD_TYPE%\bbs-terminal.exe
)

exit /b 0
