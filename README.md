# OpenFOAM GUI v2.0.5

A professional CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

📖 **Online Documentation**: [openfoamgui.readthedocs.io](https://openfoamgui.readthedocs.io/)

---

## Features

| Module | Description |
|--------|-------------|
| **Case Browser** | Lazy-loading tree, multi-case, filter, rename, auto-refresh (file watcher) |
| **Code Editor** | Multi-tab, syntax highlighting, line numbers, 3-way auto-completion, auto-indent, bracket matching |
| **C++ Completion** | 200+ STL keywords, containers, algorithms, smart pointers, streams |
| **Python Completion** | Python 3 builtins, numpy/pandas |
| **OpenFOAM Completion** | 500+ keywords: 120+ solvers, BC types, shell scripts (Allrun/Allclean) |
| **#include Headers** | 50+ C/C++/OF header suggestions on `#include <` |
| **Boundary Conditions** | RTM parameter table, 120+ BC types, patch browser, smart suggestions |
| **Turbulence Model** | 20 RANS/LES/DES models with coefficient tables + Inlet Calculator |
| **BL Calculator (Y+)** | y+ → firstLayerThickness for snappyHexMesh addLayersControls, with derivation panel |
| **Turbulence Calculator** | Inlet k / epsilon / omega / nut / nuTilda for ALL OF turbulence model families |
| **Schemes & Solvers** | Structured editing of `fvSchemes` and `fvSolution` |
| **snappyHexMesh** | Full `snappyHexMeshDict` panel + Y+ → addLayersControls sync |
| **Dict Panel** | 20+ dictionary types: `blockMeshDict`, `controlDict`, `waveProperties` (7 wave models), etc. |
| **Drag-to-Resize** | Drag splitters to adjust panel areas (param tree / preview / section list) |
| **Sync Boundaries** | One-click sync `blockMeshDict` patches → all `0/` field files |
| **File Viewer** | PNG/JPG/SVG/EPS (zoom, fit); PDF/Office via system default |
| **Run Python** | Execute `.py` from editor, configurable interpreter path |
| **Run C++** | Compile with `g++ -std=c++17 -O2` + run, configurable compiler path |
| **ParaView** | One-click launch, auto `.foam` file, path config, download guide |
| **Terminal** | Open system terminal in case directory |
| **Dark Theme** | Ctrl+T toggle, Fusion dark palette, persisted |
| **Find/Replace** | Inline find (Ctrl+F), F3 find-next, Replace (Ctrl+H) with replace-all |
| **Toolbar** | Drag-to-reorder, View → Toolbar Buttons toggles, Reset Default Layout |

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+O` | Open Case |
| `Ctrl+S` | Save |
| `Ctrl+W` | Close Tab |
| `Ctrl+F` | Find |
| `Ctrl+H` | Replace |
| `F3` | Find Next |
| `Ctrl+/` | Toggle Comment |
| `Ctrl+T` | Dark / Light Theme |
| `Ctrl+Z` / `Ctrl+Y` | Undo / Redo |
| `Ctrl+B` | Toggle BC Panel |
| `Ctrl+Shift+P` | Run Python |
| `Ctrl+Shift+C` | Run C++ |
| `Ctrl+N` | New File |

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2+)
- **OpenFOAM** (v2012–v2512)
- **ParaView** 5.10+ (optional)
- **Python 3.x** (optional, for Run Python)
- **g++** (optional, for Run C++; MinGW includes it)
- **Ghostscript** (optional, for EPS; TeX Live includes it)

## Install

### Windows

Download `OpenFOAMGUI.exe` from [Releases](https://github.com/byChen47/OpenFOAMGUI/releases) — portable, no installation required.

### Linux (.deb)

Download `openfoamgui_2.0.5_all.deb` from [Releases](https://github.com/byChen47/OpenFOAMGUI/releases/tag/OpenFOAMGUI_V2.0.5) and install:

```bash
sudo dpkg -i openfoamgui_2.0.5_all.deb
sudo openfoamgui-build   # one-time compile
openfoamgui              # launch
```

Requirements: Qt 6.2+ (`qt6-base-dev`) or Qt 5.15+ (`qtbase5-dev`).

## Build from Source

```bash
# Windows (MinGW)
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# Output: release/OpenFOAMGUI.exe

# Linux
qmake6 OpenFOAMGUI.pro   # or qmake
make -f Makefile.Release -j$(nproc)
# Output: release/OpenFOAMGUI
```

**Qt modules**: Core, GUI, Widgets, Svg, SvgWidgets · **Standard**: C++17

---

## Project Structure

```
OpenFOAMGUI/
├── .readthedocs.yaml          # Read the Docs config
├── mkdocs.yml                 # MkDocs documentation config
├── docs/                      # Online documentation source
├── src/                       # Source code (17 source pairs)
│   ├── main.cpp, mainwindow.* # Entry point + main window
│   ├── casebrowser.*          # Case directory tree (lazy-load, rename, watcher)
│   ├── codeeditor.*           # Editor + 500+ keyword auto-completion
│   ├── fileviewer.*           # Image/EPS/PDF/Office viewer
│   ├── ofhighlighter.*        # OpenFOAM/C++ syntax highlighting
│   ├── ofparser.*             # OpenFOAM file parser
│   ├── ofmeshreader.*         # OpenFOAM polyMesh reader
│   ├── languagedetector.*     # File language detection
│   ├── linenumberarea.*       # Line number margin widget
│   ├── bcpanel.* + bctypedatabase.*  # BC panel + 120+ BC types
│   ├── turbulencepanel.* + turbulencemodeldatabase.*  # Turbulence + inlet calculator
│   ├── schemespanel.*         # fvSchemes / fvSolution
│   ├── snappypanel.*          # snappyHexMeshDict + Y+ calculator
│   ├── dictpanel.*            # 20+ dictionary types (waveProperties, etc.)
│   └── bychen.ico             # Application icon
├── tests/                     # Unit tests (19 tests, ofparser + language)
├── .github/workflows/         # CI: Windows MinGW + Linux GCC
├── ROADMAP.md                 # Development roadmap
├── UI_REFACTOR_PLAN.md        # UI refactoring plan
├── CHANGELOG.md, README.md, README_CN.md
└── qt.conf, resources.qrc, OpenFOAMGUI.pro
```

## Links

- **Online Docs**: [openfoamgui.readthedocs.io](https://openfoamgui.readthedocs.io/)
- **GitHub**: [github.com/byChen47/OpenFOAMGUI](https://github.com/byChen47/OpenFOAMGUI)
- **ParaView**: [paraview.org/download](https://www.paraview.org/download/)
- **Python**: [python.org/downloads](https://www.python.org/downloads/)

## License

Educational and research use in CFD workflows.
