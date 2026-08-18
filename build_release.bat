@echo off
setlocal enabledelayedexpansion

set "VERSION=0.1.0"
set "ROOT=%~dp0"
set "BUILD=%ROOT%build"
set "DIST=%ROOT%dist"
set "PACKAGE=%BUILD%\package"
set "ARCHIVE=%DIST%\AnimSyncTogether-v%VERSION%.zip"

echo ============================================================
echo  AnimSync Together v%VERSION% - Release Build
echo ============================================================

if not exist "%DIST%" mkdir "%DIST%"
if exist "%PACKAGE%" rmdir /s /q "%PACKAGE%"
if exist "%ARCHIVE%" del /q "%ARCHIVE%"

echo.
echo [1/4] Configuring...
cmake -S "%ROOT%" -B "%BUILD%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :fail

echo.
echo [2/4] Building...
cmake --build "%BUILD%" --config Release
if errorlevel 1 goto :fail

if not exist "%PACKAGE%\SKSE\Plugins\AnimSyncTogether.dll" (
    echo ERROR: packaged DLL was not produced.
    goto :fail
)

echo.
echo [3/4] Creating Vortex archive...
powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%PACKAGE%\*' -DestinationPath '%ARCHIVE%' -Force"
if errorlevel 1 goto :fail

echo.
echo [4/4] Done.
echo Archive: %ARCHIVE%
exit /b 0

:fail
echo.
echo BUILD FAILED.
exit /b 1
