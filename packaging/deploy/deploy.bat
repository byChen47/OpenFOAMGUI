@echo off
REM ============================================================
REM  OpenFOAM GUI Deployment Script (Windows)
REM  Copies required Qt DLLs and platform plugins alongside the exe
REM ============================================================
setlocal enabledelayedexpansion

set "QT_DIR=D:\3.Wpsandother\Qt\setting\6.10.2\mingw_64"
set "MINGW_DIR=D:\3.Wpsandother\mingw64\bin"
set "TARGET_DIR=%~dp0"

echo Deploying OpenFOAM GUI...
echo Qt dir: %QT_DIR%
echo Target: %TARGET_DIR%

REM --- Copy main executable ---
if exist "..\..\release\OpenFOAMGUI.exe" (
    copy /Y "..\..\release\OpenFOAMGUI.exe" "%TARGET_DIR%OpenFOAMGUI.exe"
    echo Copied OpenFOAMGUI.exe
) else if exist "%TARGET_DIR%OpenFOAMGUI.exe" (
    echo OpenFOAMGUI.exe already present
) else (
    echo ERROR: OpenFOAMGUI.exe not found! Build first.
    exit /b 1
)

REM --- Use windeployqt to copy all Qt dependencies ---
for %%t in ("%QT_DIR%\bin\windeployqt.exe") do set "WDQ=%%~t"
if not defined WDQ (
    for %%t in (windeployqt.exe windeployqt6.exe) do (
        where %%t >nul 2>&1 && set "WDQ=%%t"
    )
)

if defined WDQ (
    echo Running windeployqt...
    "%WDQ%" "%TARGET_DIR%OpenFOAMGUI.exe" --release --qmldir "..\.."
) else (
    echo windeployqt not found, using manual copy...

    REM --- Qt libraries ---
    set "LIBS=Qt6Core Qt6Gui Qt6Widgets Qt6Svg Qt6SvgWidgets"
    for %%L in (%LIBS%) do (
        if exist "%QT_DIR%\bin\%%L.dll" (
            copy /Y "%QT_DIR%\bin\%%L.dll" "%TARGET_DIR%"
            echo Copied %%L.dll
        )
    )

    REM --- MinGW runtime ---
    for %%D in (libgcc_s_seh-1 libstdc++-6 libwinpthread-1) do (
        if exist "%MINGW_DIR%\%%D.dll" (
            copy /Y "%MINGW_DIR%\%%D.dll" "%TARGET_DIR%"
            echo Copied %%D.dll
        )
    )

    REM --- Platform plugin ---
    if not exist "%TARGET_DIR%platforms" mkdir "%TARGET_DIR%platforms"
    copy /Y "%QT_DIR%\plugins\platforms\qwindows.dll" "%TARGET_DIR%platforms\"
    echo Copied platforms\qwindows.dll

    REM --- Styles plugin ---
    if not exist "%TARGET_DIR%styles" mkdir "%TARGET_DIR%styles"
    copy /Y "%QT_DIR%\plugins\styles\qwindowsvistastyle.dll" "%TARGET_DIR%styles\"
    echo Copied styles\qwindowsvistastyle.dll

    REM --- SVG plugins ---
    if not exist "%TARGET_DIR%imageformats" mkdir "%TARGET_DIR%imageformats"
    copy /Y "%QT_DIR%\plugins\imageformats\*.dll" "%TARGET_DIR%imageformats\" 2>nul
    echo Copied imageformats plugins

    if not exist "%TARGET_DIR%iconengines" mkdir "%TARGET_DIR%iconengines"
    copy /Y "%QT_DIR%\plugins\iconengines\*.dll" "%TARGET_DIR%iconengines\" 2>nul
    echo Copied iconengines plugins
)

REM --- Ghostscript (for PDF/EPS rendering) ---
if exist "%~dp0ghostscript\bin\gswin64c.exe" (
    if not exist "%TARGET_DIR%ghostscript" (
        echo Copying Ghostscript...
        xcopy /E /I /Y "%~dp0ghostscript" "%TARGET_DIR%ghostscript" >nul
        echo Copied Ghostscript (35 MB)
    ) else (
        echo Ghostscript already present
    )
) else (
    echo Ghostscript not found in source, skipping.
)

echo.
echo Deployment complete.
echo Run: %TARGET_DIR%OpenFOAMGUI.exe
endlocal
