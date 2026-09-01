# BBS Terminal Emulator - PowerShell Build Script
# Usage: .\build.ps1 -Configuration Release -Platform x64

param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    
    [ValidateSet('Win32', 'x64')]
    [string]$Platform = 'x64',
    
    [switch]$Clean,
    [switch]$Run
)

$ErrorActionPreference = 'Stop'

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "BBS Terminal Emulator - PowerShell Build Script" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Configuration: $Configuration" -ForegroundColor Green
Write-Host "Platform: $Platform" -ForegroundColor Green
Write-Host ""

# Check for CMake
try {
    $cmakeVersion = cmake --version 2>$null | Select-Object -First 1
    Write-Host "CMake: $cmakeVersion" -ForegroundColor Green
} catch {
    Write-Host "ERROR: CMake not found. Please install CMake 3.20 or later." -ForegroundColor Red
    exit 1
}

# Clean build directory if requested
if ($Clean -and (Test-Path 'build')) {
    Write-Host ""
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force -Path 'build'
}

# Create build directory
if (-not (Test-Path 'build')) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path 'build' | Out-Null
}

cd build

# Configure project
Write-Host ""
Write-Host "Configuring project with CMake..." -ForegroundColor Yellow
$cmakeArgs = @(
    '..',
    '-G', '"Visual Studio 17 2022"',
    '-A', $Platform,
    '-DCMAKE_BUILD_TYPE=' + $Configuration
)

$cmakeCmd = "cmake " + ($cmakeArgs -join ' ')
Invoke-Expression $cmakeCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    cd ..
    exit 1
}

# Build project
Write-Host ""
Write-Host "Building project..." -ForegroundColor Yellow
cmake --build . --config $Configuration -- /p:Configuration=$Configuration /p:Platform=$Platform

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    cd ..
    exit 1
}

cd ..

Write-Host ""
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Output: bin\$Configuration\bbs-terminal.exe" -ForegroundColor Green
Write-Host ""

# Run application if requested
if ($Run) {
    Write-Host "Starting application..." -ForegroundColor Yellow
    & ".\bin\$Configuration\bbs-terminal.exe"
}
