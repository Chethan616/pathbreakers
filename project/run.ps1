# Configure (first run only), build and launch the Operator Assistant app.
# Usage: .\run.ps1   (run from inside project\, or from anywhere: & D:\CATERPILLAR\project\run.ps1)

$ErrorActionPreference = "Stop"

$ScriptDir = $PSScriptRoot
$BuildDir = Join-Path $ScriptDir "build-cli"

# Defaults match this machine's Qt install; override by setting env vars before running, e.g.:
#   $env:QT_PREFIX = "D:/Qt/6.9.0/mingw_64"; .\run.ps1
$QtPrefix = if ($env:QT_PREFIX) { $env:QT_PREFIX } else { "C:/Qt/6.11.2/mingw_64" }
$MingwBin = if ($env:MINGW_BIN) { $env:MINGW_BIN } else { "C:/Qt/Tools/mingw1310_64/bin" }
$CMakeBin = if ($env:CMAKE_BIN) { $env:CMAKE_BIN } else { "C:/Qt/Tools/CMake_64/bin/cmake.exe" }
$NinjaBin = if ($env:NINJA_BIN) { $env:NINJA_BIN } else { "C:/Qt/Tools/Ninja/ninja.exe" }

if (-not (Test-Path $CMakeBin)) {
    Write-Error "cmake.exe not found at $CMakeBin - set `$env:CMAKE_BIN or edit run.ps1."
}

$env:PATH = "$MingwBin;$QtPrefix/bin;" + $env:PATH

if (-not (Test-Path $BuildDir)) {
    Write-Host "==> Configuring (first run)..."
    & $CMakeBin -S $ScriptDir -B $BuildDir -G "Ninja" `
        -DCMAKE_PREFIX_PATH=$QtPrefix `
        -DCMAKE_C_COMPILER="$MingwBin/gcc.exe" `
        -DCMAKE_CXX_COMPILER="$MingwBin/g++.exe" `
        -DCMAKE_MAKE_PROGRAM=$NinjaBin
    if ($LASTEXITCODE -ne 0) { Write-Error "Configure failed." }
}

Write-Host "==> Building..."
& $CMakeBin --build $BuildDir
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed." }

Write-Host "==> Launching..."
Start-Process -FilePath (Join-Path $BuildDir "appproject.exe")
Write-Host "Started."
