# OpenFOAM GUI — Deployment Files

This directory contains the runtime dependencies required to run `OpenFOAMGUI.exe` distribution.

## Files

| File | Purpose |
|------|---------|
| `OpenFOAMGUI.exe` | Main application executable |
| `Qt6Core.dll` | Qt Core module |
| `Qt6Gui.dll` | Qt GUI module |
| `Qt6Widgets.dll` | Qt Widgets module |
| `Qt6Svg.dll` | Qt SVG module (SVG image support) |
| `Qt6SvgWidgets.dll` | Qt SVG Widgets module |
| `libgcc_s_seh-1.dll` | MinGW GCC runtime |
| `libstdc++-6.dll` | MinGW C++ standard library |
| `libwinpthread-1.dll` | MinGW POSIX threads |
| `platforms/qwindows.dll` | Qt Windows platform plugin |
| `styles/qwindowsvistastyle.dll` | Qt Windows style plugin |
| `imageformats/*.dll` | Qt image format plugins (PNG, JPG, BMP, etc.) |
| `iconengines/*.dll` | Qt icon engine plugins |

## Auto-Deployment

Run `deploy.bat` to automatically copy all required DLLs and plugins from the Qt and MinGW installations.

```bat
deploy.bat
```

The script uses `windeployqt` if available; otherwise falls back to manual copy.

## Manual Deployment

Copy these files alongside `OpenFOAMGUI.exe`:

```
deploy/
├── OpenFOAMGUI.exe
├── Qt6Core.dll
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── Qt6Svg.dll
├── Qt6SvgWidgets.dll
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── platforms/
│   └── qwindows.dll
├── styles/
│   └── qwindowsvistastyle.dll
├── imageformats/
│   └── *.dll
└── iconengines/
    └── *.dll
```
