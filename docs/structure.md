# Project Structure

```
OpenFOAMGUI/
├── .readthedocs.yaml          # Read the Docs configuration
├── mkdocs.yml                 # MkDocs documentation config
├── OpenFOAMGUI.pro            # Qt project file
├── OpenFOAMGUI_resource.rc    # Windows resource file
├── resources.qrc              # Qt resource declarations
├── qt.conf                    # Qt plugin path configuration
├── CHANGELOG.md               # Modification history
├── README.md                  # English documentation
├── README_CN.md               # Chinese documentation
├── docs/                      # MkDocs documentation source
│   ├── index.md               # Home page
│   ├── features.md            # Feature documentation
│   ├── usage.md               # Usage guide
│   ├── build.md               # Build instructions
│   ├── structure.md           # Project structure (this file)
│   └── changelog.md           # Version changelog
└── src/                       # Source code
    ├── main.cpp               # Application entry point
    ├── mainwindow.h           # Main window header
    ├── mainwindow.cpp         # Main window implementation
    ├── casebrowser.h          # Case directory tree header
    ├── casebrowser.cpp        # Case directory tree implementation
    ├── codeeditor.h           # Code editor header
    ├── codeeditor.cpp         # Code editor + auto-completion
    ├── fileviewer.h           # File viewer header
    ├── fileviewer.cpp         # Image/EPS/PDF/Office viewer
    ├── ofhighlighter.h        # Syntax highlighter header
    ├── ofhighlighter.cpp      # Syntax highlighter implementation
    ├── ofparser.h             # File parser header
    ├── ofparser.cpp           # File parser implementation
    ├── ofmeshreader.h         # polyMesh reader header
    ├── ofmeshreader.cpp       # polyMesh reader implementation
    ├── languagedetector.h     # Language detector header
    ├── languagedetector.cpp   # Language detector implementation
    ├── linenumberarea.h       # Line number widget header
    ├── linenumberarea.cpp     # Line number widget implementation
    ├── bcpanel.h              # BC panel header
    ├── bcpanel.cpp            # BC panel implementation
    ├── bctypedatabase.h       # BC type database header
    ├── bctypedatabase.cpp     # BC type database (100+ types)
    ├── turbulencepanel.h      # Turbulence panel header
    ├── turbulencepanel.cpp    # Turbulence panel implementation
    ├── turbulencemodeldatabase.h
    ├── turbulencemodeldatabase.cpp
    ├── schemespanel.h         # Schemes panel header
    ├── schemespanel.cpp       # Schemes panel implementation
    ├── snappypanel.h          # snappyHexMesh panel header
    ├── snappypanel.cpp        # snappyHexMesh panel implementation
    ├── dictpanel.h            # Dictionary panel header
    ├── dictpanel.cpp          # Dictionary panel implementation
    └── bychen.ico             # Application icon
```

## Architecture

### Core Components

| Component | Files | Purpose |
|-----------|-------|---------|
| **Application** | `main.cpp` | Entry point, QApplication setup |
| **Main Window** | `mainwindow.h/.cpp` | Central UI controller: menus, toolbar, dock widgets, signal routing |
| **Case Browser** | `casebrowser.h/.cpp` | OpenFOAM case directory tree with lazy loading |
| **Code Editor** | `codeeditor.h/.cpp` | Tabbed editor with syntax highlighting and auto-completion |
| **File Viewer** | `fileviewer.h/.cpp` | Multi-format file rendering (images, EPS, PDF/Office) |

### Configuration Panels

| Panel | Files | Purpose |
|-------|-------|---------|
| **BC Panel** | `bcpanel.h/.cpp` | Boundary condition editor (RTM table, patch browser) |
| **BC Database** | `bctypedatabase.h/.cpp` | 100+ BC type definitions |
| **Turbulence** | `turbulencepanel.h/.cpp` | RAS/LES model configuration |
| **Schemes** | `schemespanel.h/.cpp` | fvSchemes / fvSolution editing |
| **snappyHexMesh** | `snappypanel.h/.cpp` | snappyHexMeshDict configuration |
| **General Dict** | `dictpanel.h/.cpp` | Generic OpenFOAM dictionary editor |

### Utility Components

| Component | Files | Purpose |
|-----------|-------|---------|
| **Highlighter** | `ofhighlighter.h/.cpp` | OpenFOAM/C++ syntax highlighting |
| **Parser** | `ofparser.h/.cpp` | OpenFOAM file header/keyword parsing |
| **Mesh Reader** | `ofmeshreader.h/.cpp` | OpenFOAM polyMesh format reader |
| **Language Detector** | `languagedetector.h/.cpp` | File language auto-detection |
| **Line Numbers** | `linenumberarea.h/.cpp` | Editor line number margin widget |

### Data Flow

```
User Input → MainWindow → CaseBrowser / CodeEditor / Panels
                ↓
         ofparser / ofhighlighter / bctypedatabase
                ↓
         File system (read/write OpenFOAM files)
```
