# OpenFOAM GUI

A CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

Browse, edit, and save OpenFOAM case files with syntax highlighting, auto-completion, case structure awareness, and integrated configuration panels.

---

## Features

| Module | Description |
|--------|-------------|
| **Case Browser** | Tree view of `0/`, `constant/`, `system/` with lazy loading for large cases |
| **Code Editor** | Multi-tab editor with syntax highlighting, line numbers, auto-completion, auto-indent |
| **Auto Completion** | C++ (200+ STL keywords), Python, OpenFOAM (250+ BCs/schemes) — toggleable independently |
| **Boundary Conditions** | Visual BC editing with RTM parameter tables, patch browser, 100+ BC types |
| **Turbulence Model** | RAS/LES model selection with model-specific parameter forms |
| **Discretisation & Solvers** | Structured editing of `fvSchemes` and `fvSolution` |
| **snappyHexMesh** | Full snappyHexMeshDict configuration |
| **General Dict** | Edit blockMeshDict, controlDict, decomposeParDict, etc. |
| **Sync Boundaries** | One-click sync blockMeshDict patches to all 0/ field files |
| **File Viewer** | PNG/JPG/SVG/EPS images with zoom; PDF/Office via system default |
| **Run Python** | Run Python scripts from editor, configurable interpreter path |
| **ParaView Integration** | One-click launch with auto-detection and download guide |
| **Terminal** | Open system terminal in the case directory |
| **Customizable Toolbar** | Drag to reorder, View menu toggles, Reset Default Layout |

---

## Quick Start

1. Launch `OpenFOAMGUI.exe`
2. **Case → Open Case** (`Ctrl+O`) to select an OpenFOAM case
3. Double-click files in the Case Browser to edit
4. Right panel auto-switches context (BC/Turbulence/Schemes/snappyHexMesh/Dict)
5. **View → 3D Mesh Viewer** (bottom dock) for geometry preview

---

## Editor Features

| Shortcut | Action |
|----------|--------|
| `Ctrl+O` | Open Case |
| `Ctrl+S` | Save |
| `Ctrl+W` | Close Tab |
| `Ctrl+F` | Find |
| `Ctrl+/` | Toggle Comment |
| `Ctrl+Z` / `Ctrl+Y` | Undo / Redo |
| `Ctrl+Shift+P` | Run Python |
| `Ctrl+B` | Toggle BC Panel |

**Auto Completion** — Edit menu toggles C++, Python, and OpenFOAM completions independently.
Typing `#include <` triggers header file suggestions.

**Auto Indent** — Enter preserves indentation. `{` on an empty line auto-generates formatted block.
Cursor inside `{}` + Enter pushes the closing brace to the next line.

---

## Sync Boundaries

`View → Sync Boundaries` synchronises `blockMeshDict` boundary patch names to all field files in `0/` (or `0.orig/`). Modified files are automatically reloaded in open editor tabs.

### Default BC Assignment

| Patch Type | Field | Default BC |
|-----------|-------|-------------|
| `wall` | U, v | `noSlip` |
| `wall` | p, p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut | `nutkWallFunction` |
| `empty` | any | `empty` |
| `symmetry` | any | `symmetry` |
| `wedge` | any | `wedge` |
| `cyclic` | any | `cyclic` |

---

## File Viewer

| Category | Extensions | Viewer |
|----------|-----------|--------|
| Raster Images | PNG, JPG, BMP, GIF, WebP, ICO | Native viewer with zoom (− / + / Fit / 1:1) |
| SVG | SVG | QSvgWidget renderer |
| EPS | EPS, EPSF, PS | Ghostscript → PNG (auto-detect) |
| PDF | PDF | System default |
| Office | DOC, DOCX, XLS, XLSX, PPT, PPTX | System default |

---

## ParaView Integration

- **Case → ParaView** launches ParaView with auto-created `.foam` file
- Auto-detects ParaView at common paths; configurable via **Case → ParaView Path...**
- Shows download URL if not installed

---

## Toolbar Customization

- Drag toolbar buttons to reorder
- **View menu** toggles show/hide each button
- **View → Reset Default Layout** restores original arrangement

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0+)
- **OpenFOAM** (v2012 through v2512)
- **ParaView** 5.10+ (optional)
- **Ghostscript** (optional, for EPS; TeX Live includes it)

## Build

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# Output: release/OpenFOAMGUI.exe
```

**Qt modules**: Core, GUI, Widgets, SvgWidgets
**C++17** with `-std=gnu++1z`

---

## Project Structure

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                       # Entry point
│   ├── mainwindow.h/.cpp              # Main window, menus, toolbar, docks
│   ├── casebrowser.h/.cpp             # Case directory tree browser
│   ├── codeeditor.h/.cpp              # Editor with auto-completion
│   ├── fileviewer.h/.cpp              # Image/EPS/PDF/Office file viewer
│   ├── ofhighlighter.h/.cpp           # Syntax highlighter
│   ├── ofparser.h/.cpp                # File header/keyword parser
│   ├── ofmeshreader.h/.cpp            # OpenFOAM polyMesh reader
│   ├── languagedetector.h/.cpp        # Language auto-detection
│   ├── linenumberarea.h/.cpp          # Line number margin
│   ├── bcpanel.h/.cpp                 # Boundary conditions panel
│   ├── bctypedatabase.h/.cpp          # 100+ BC type definitions
│   ├── turbulencepanel.h/.cpp         # Turbulence model panel
│   ├── turbulencemodeldatabase.h/.cpp # Turbulence model definitions
│   ├── schemespanel.h/.cpp            # fvSchemes / fvSolution panel
│   ├── snappypanel.h/.cpp             # snappyHexMeshDict panel
│   ├── dictpanel.h/.cpp               # Generic dictionary panel
│   └── bychen.ico                     # Application icon
├── OpenFOAMGUI.pro                    # Qt project file
├── resources.qrc                      # Qt resource file
├── qt.conf                            # Plugin path config
├── README.md
├── README_CN.md
└── CHANGELOG.md
```

## License

This project is provided for educational and research purposes in CFD workflows.
