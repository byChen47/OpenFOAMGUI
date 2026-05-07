@echo off
setlocal enabledelayedexpansion
REM ============================================================
REM  OpenFOAM GUI — Enigma Virtual Box Packaging Script
REM  1. Sync latest build to deploy directory
REM  2. Generate .evb project file
REM  3. Run Enigma VB to produce single boxed EXE
REM ============================================================

set "ENIGMA=D:\3.Wpsandother\Enigma\setting\Enigma Virtual Box\enigmavbconsole.exe"
set "SRC_DIR=%~dp0..\release"
set "DST_DIR=%~dp0deploy"
set "BOXED_EXE=%DST_DIR%\OpenFOAMGUI_boxed.exe"
set "EVB_FILE=%DST_DIR%\OpenFOAMGUI.evb"

if not exist "%ENIGMA%" (
    echo ERROR: Enigma Virtual Box not found at %ENIGMA%
    exit /b 1
)

echo Step 1: Syncing release/ to deploy/ ...
if not exist "%DST_DIR%" mkdir "%DST_DIR%"

REM Copy the executable
copy /Y "%SRC_DIR%\OpenFOAMGUI.exe" "%DST_DIR%\" >nul

REM Copy all DLLs
for %%f in ("%SRC_DIR%\*.dll") do copy /Y "%%f" "%DST_DIR%\" >nul

REM Copy plugin directories
for %%d in (platforms styles imageformats iconengines generic networkinformation tls) do (
    if exist "%SRC_DIR%\%%d" (
        if not exist "%DST_DIR%\%%d" mkdir "%DST_DIR%\%%d"
        xcopy /E /Y /Q "%SRC_DIR%\%%d\*" "%DST_DIR%\%%d\" >nul
    )
)

REM Copy Ghostscript
if exist "%SRC_DIR%\ghostscript" (
    if not exist "%DST_DIR%\ghostscript" mkdir "%DST_DIR%\ghostscript"
    xcopy /E /Y /Q "%SRC_DIR%\ghostscript\*" "%DST_DIR%\ghostscript\" >nul
    echo Ghostscript copied.
)

echo Deploy directory synced.

REM ── Generate .evb project ──
echo Step 2: Generating EVB project ...

set "TAB=    "
(
echo ^<?xml encoding="utf-16"?^>
echo ^<^>
echo %TAB%^<InputFile^>%DST_DIR%\OpenFOAMGUI.exe^</InputFile^>
echo %TAB%^<OutputFile^>%BOXED_EXE%^</OutputFile^>
echo %TAB%^<Files^>
echo %TAB%^<Enabled^>True^</Enabled^>
echo %TAB%^<DeleteExtractedOnExit^>True^</DeleteExtractedOnExit^>
echo %TAB%^<CompressFiles^>True^</CompressFiles^>
echo %TAB%^<Files^>
echo %TAB%%TAB%^<File^>
echo %TAB%%TAB%%TAB%^<Type^>3^</Type^>
echo %TAB%%TAB%%TAB%^<Name^>%%DEFAULT FOLDER%%^</Name^>
echo %TAB%%TAB%%TAB%^<Action^>0^</Action^>
echo %TAB%%TAB%%TAB%^<OverwriteDateTime^>False^</OverwriteDateTime^>
echo %TAB%%TAB%%TAB%^<OverwriteAttributes^>False^</OverwriteAttributes^>
echo %TAB%%TAB%%TAB%^<HideFromDialogs^>0^</HideFromDialogs^>
echo %TAB%%TAB%%TAB%^<Files^>
) > "%EVB_FILE%"

REM Add all files from deploy directory (excluding the boxed exe itself)
for /r "%DST_DIR%" %%f in (*) do (
    set "full=%%f"
    set "rel=!full:%DST_DIR%\=!"
    set "rel=!rel:\=/!"
    if /I not "!rel!"=="OpenFOAMGUI_boxed.exe" (
        if /I not "!rel!"=="OpenFOAMGUI.evb" (
            echo %TAB%%TAB%%TAB%%TAB%^<File^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<Type^>2^</Type^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<Name^>!rel!^</Name^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<File^>%%f^</File^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<ActiveX^>False^</ActiveX^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<ActiveXInstall^>False^</ActiveXInstall^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<Action^>0^</Action^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<OverwriteDateTime^>False^</OverwriteDateTime^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<OverwriteAttributes^>False^</OverwriteAttributes^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<PassCommandLine^>False^</PassCommandLine^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%%TAB%^<HideFromDialogs^>0^</HideFromDialogs^> >> "%EVB_FILE%"
            echo %TAB%%TAB%%TAB%%TAB%^</File^> >> "%EVB_FILE%"
        )
    )
)

REM Close XML
(
echo %TAB%%TAB%%TAB%^</Files^>
echo %TAB%%TAB%^</File^>
echo %TAB%^</Files^>
echo %TAB%^</Files^>
echo ^</^>
) >> "%EVB_FILE%"

echo EVB project generated: %EVB_FILE%

REM ── Run Enigma VB ──
echo Step 3: Running Enigma Virtual Box ...
"%ENIGMA%" "%EVB_FILE%"

if exist "%BOXED_EXE%" (
    echo.
    echo ==========================================
    echo  SUCCESS!
    echo  Boxed EXE: %BOXED_EXE%
    for %%A in ("%BOXED_EXE%") do echo  Size: %%~zA bytes
    echo ==========================================
) else (
    echo ERROR: Packaging failed.
    exit /b 1
)
endlocal
