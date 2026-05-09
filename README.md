# OpenFOAM GUI

A professional CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

---

## Features

| Module | Description |
|--------|-------------|
| **Case Browser** | Lazy-loading tree view with multi-case, filter, multi-format file support |
| **Code Editor** | Multi-tab with syntax highlighting, line numbers, 3-way auto-completion, auto-indent |
| **C++ Auto Completion** | 200+ keywords: STL containers, algorithms, smart pointers, streams, C stdlib |
| **Python Auto Completion** | Python 3 keywords, builtins, numpy/pandas hints |
| **OpenFOAM Auto Completion** | 250+ keywords: all BC types, schemes, solvers, snappyHexMesh, turbulence models |
| **#include Headers** | 50+ C/C++/OF header suggestions on `#include <` |
| **Boundary Conditions** | RTM parameter table, 100+ BC types, patch browser, smart suggestions |
| **Turbulence Model** | RAS/LES model configuration with model-specific parameters |
| **Schemes & Solvers** | Structured editing of `fvSchemes` and `fvSolution` |
| **snappyHexMesh** | Full `snappyHexMeshDict` configuration panel |
| **General Dict** | `blockMeshDict`, `controlDict`, `decomposeParDict`, etc. |
| **Sync Boundaries** | One-click sync `blockMeshDict` patches → all `0/` field files |
| **File Viewer** | PNG/JPG/SVG/EPS (zoom, fit); PDF/Office via system default |
| **Run Python** | Execute `.py` scripts from editor, configurable interpreter path |
| **Run C++** | Compile with `g++ -std=c++17 -O2` and run, configurable compiler path |
| **ParaView** | One-click launch, auto `.foam` file, path config, download guide |
| **Terminal** | Open system terminal in case directory |
| **Customizable Toolbar** | Drag-to-reorder, View menu toggles, Reset Default Layout |

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+O` | Open Case |
| `Ctrl+S` | Save |
| `Ctrl+W` | Close Tab |
| `Ctrl+F` | Find |
| `Ctrl+/` | Toggle Comment |
| `Ctrl+Z/Y` | Undo / Redo |
| `Ctrl+B` | Toggle BC Panel |
| `Ctrl+Shift+P` | Run Python |
| `Ctrl+Shift+C` | Run C++ |

---

## Editor

**Auto Completion** — Edit menu provides independent toggles for C++, Python, and OpenFOAM. Completion activates after 2+ characters typed.

**Auto Indent** — Enter preserves current indentation. `{` on empty line generates a formatted code block. Cursor inside `{}` + Enter pushes closing brace down.

**#include Headers** — Typing `<` after `#include` triggers 50+ header suggestions. Type to filter.

---

## Sync Boundaries

`View → Sync Boundaries` copies blockMesh boundary names into all field files in `0/` or `0.orig/`. Open editor tabs auto-reload.

| Patch Type | Field | Default BC |
|-----------|-------|-------------|
| `wall` | U | `noSlip` |
| `wall` | p/p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut | `nutkWallFunction` |
| `empty` / `symmetry` / `wedge` / `cyclic` | any | (same as type) |

---

## File Viewer

| Format | Viewer |
|--------|--------|
| PNG, JPG, BMP, GIF, WebP, ICO | Native zoomable viewer (− / + / Fit / 1:1) |
| SVG | QSvgWidget |
| EPS, EPSF, PS | Ghostscript → PNG (auto-detect) |
| PDF, DOC, DOCX, XLS, XLSX | System default application |

---

## ParaView

- **Case → ParaView** launches ParaView with auto-created `.foam` file
- Configure path: **Case → ParaView Path...**
- Shows download URL if not installed

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2+)
- **OpenFOAM** (v2012–v2512)
- **ParaView** 5.10+ (optional)
- **Python** 3.x (optional, for Run Python)
- **g++** (optional, for Run C++; MinGW includes it)

## Build

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
```

**Qt modules**: Core, GUI, Widgets, SvgWidgets
**Standard**: C++17

---

## Project Structure

```
src/
├── main.cpp                  Entry point
├── mainwindow.h/.cpp         Main window, menus, toolbar, docks
├── casebrowser.h/.cpp        Case directory tree (lazy-loading)
├── codeeditor.h/.cpp         Editor with auto-completion
├── fileviewer.h/.cpp         Image/EPS/PDF/Office viewer
├── ofhighlighter.h/.cpp      OpenFOAM/C++ syntax highlighting
├── ofparser.h/.cpp           OpenFOAM file parser
├── ofmeshreader.h/.cpp       OpenFOAM polyMesh reader
├── languagedetector.h/.cpp   File language detection
├── linenumberarea.h/.cpp     Line number widget
├── bcpanel.h/.cpp            Boundary conditions panel
├── bctypedatabase.h/.cpp     100+ BC type database
├── turbulencepanel.h/.cpp    Turbulence model panel
├── turbulencemodeldatabase.h/.cpp
├── schemespanel.h/.cpp       fvSchemes/fvSolution panel
├── snappypanel.h/.cpp        snappyHexMeshDict panel
├── dictpanel.h/.cpp          Generic dictionary panel
└── bychen.ico                Application icon
```

## License

Educational and research use in CFD workflows.
