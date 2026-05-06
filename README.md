# OpenFOAM GUI

A CFD case manager and editor for OpenFOAM, built with Qt 6.

Open, browse, edit, and save OpenFOAM case files with syntax highlighting, case structure awareness, and integrated configuration panels.

## Features

- **Case Directory Browser** — tree view of `0/`, `constant/`, `system/` and time directories
- **Syntax Highlighting** — full support for OpenFOAM dictionary syntax
- **Multi-tab Editor** — edit multiple files simultaneously with line numbers and current-line highlighting
- **File Type Detection** — automatic language detection with status bar tooltips
- **Boundary Conditions Panel** — visual editing of field BCs (U, p, p_rgh, k, epsilon, omega, nut, T, etc.)
- **Turbulence Model Panel** — configure RAS/LES turbulence models via `turbulenceProperties`
- **Discretisation & Solvers Panel** — edit `fvSchemes` and `fvSolution` with structured forms
- **snappyHexMesh Panel** — configure snappyHexMeshDict parameters
- **General Dict Panel** — edit blockMeshDict, controlDict, decomposeParDict, and more
- **ParaView Integration** — launch ParaView directly from the Case menu
- **Recent Cases** — quick access to recently opened case directories

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0)
- **OpenFOAM** (v2012 through v2512)
- **ParaView** (optional, for post-processing)

## Build

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
```

Or open `OpenFOAMGUI.pro` in Qt Creator and build with the MinGW 64-bit kit.

## Usage

1. Launch `OpenFOAMGUI.exe`
2. **File → Open Case** (`Ctrl+O`) and select an OpenFOAM case directory
3. Double-click any file in the **Case Browser** dock to open it in the editor
4. Use the right-side panel to configure boundary conditions, turbulence models, discretisation schemes, or mesh settings
5. **Save** (`Ctrl+S`) to write changes back to the case

## Project Structure

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── mainwindow.h / .cpp         # Main window, menus, toolbar, dock widgets
│   ├── casebrowser.h / .cpp        # Case directory tree browser
│   ├── codeeditor.h / .cpp         # Code editor with line numbers
│   ├── ofhighlighter.h / .cpp      # OpenFOAM syntax highlighter
│   ├── ofparser.h / .cpp           # OpenFOAM file parser
│   ├── languagedetector.h / .cpp   # File language auto-detection
│   ├── linenumberarea.h / .cpp     # Line number margin widget
│   ├── bcpanel.h / .cpp            # Boundary conditions panel
│   ├── bctypedatabase.h / .cpp     # BC type definitions database
│   ├── turbulencepanel.h / .cpp    # Turbulence model panel
│   ├── turbulencemodeldatabase.h/.cpp
│   ├── schemespanel.h / .cpp       # fvSchemes / fvSolution panel
│   ├── snappypanel.h / .cpp        # snappyHexMeshDict panel
│   ├── dictpanel.h / .cpp          # Generic dictionary panel
│   └── bychen.ico                  # Application icon
├── OpenFOAMGUI.pro                 # Qt project file
├── resources.qrc                   # Qt resource file
└── README.md
```

## License

This project is provided for educational and research purposes in CFD workflows.
