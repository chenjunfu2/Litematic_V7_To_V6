@echo off
setlocal enabledelayedexpansion

if "%~1"=="" (
    echo Error: Missing architecture argument. Usage: %~0 [x64^|arm64]
    exit /b 1
)

set "ARCH=%~1"

if /I "%ARCH%"=="x64" (
    set "PLATFORM=x64"
) else if /I "%ARCH%"=="arm64" (
    set "PLATFORM=ARM64"
) else (
    echo Error: Unsupported architecture "%ARCH%". Supported: x64, arm64.
    exit /b 1
)

set "OUTPUT_DIR=.\artifacts\windows-%ARCH%"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo Error: vswhere.exe not found. Please install Visual Studio or add msbuild to PATH.
    exit /b 1
)

set "MSBUILD="
for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    set "MSBUILD=%%i"
    goto :MSBuildFound
)
:MSBuildFound
if not defined MSBUILD (
    echo Error: No MSBuild found by vswhere.
    exit /b 1
)
echo MSBuild path "%MSBUILD%"

set "DEVENV="
for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -find **\devenv.com`) do (
    set "DEVENV=%%i"
    goto :DevenvFound
)
:DevenvFound
if not defined DEVENV (
    echo Error: No devenv.com found by vswhere. Cannot upgrade solution.
    exit /b 1
)
echo Upgrading solution with "%DEVENV%"

"%DEVENV%" Litematic_V7_To_V6.sln /Upgrade
del /Q UpgradeLog*.htm 2>nul

"%MSBUILD%" Litematic_V7_To_V6.sln /p:Configuration=Release /p:Platform=%PLATFORM% /m
if %errorlevel% neq 0 exit /b %errorlevel%

mkdir "%OUTPUT_DIR%"
copy /Y ".\%PLATFORM%\Release\Litematic_V7_To_V6.exe" "%OUTPUT_DIR%\"
copy /Y ".\%PLATFORM%\Release\NBT_Compare.exe" "%OUTPUT_DIR%\"
copy /Y ".\%PLATFORM%\Release\NBT_Print.exe" "%OUTPUT_DIR%\"
