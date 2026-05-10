# OpenFOAM GUI

A professional CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

📖 **Online Documentation**: [openfoamgui.readthedocs.io](https://openfoamgui.readthedocs.io/)

---

## Features

| Module | Description |
|--------|-------------|
| **Case Browser** | Lazy-loading tree view, multi-case, filter, multi-format files |
| **Code Editor** | Multi-tab, syntax highlighting, line numbers, 3-way auto-completion, auto-indent |
| **C++ Completion** | 200+ STL keywords, containers, algorithms, smart pointers, streams |
| **Python Completion** | Python 3 builtins, numpy/pandas |
| **OpenFOAM Completion** | 500+ keywords: 120+ solvers, BC types, schemes, turbulence, shell scripts |
| **#include Headers** | 50+ C/C++/OF header suggestions on `#include <` |
| **Boundary Conditions** | RTM parameter table, 120+ BC types, patch browser, smart suggestions |
| **Turbulence Model** | RAS/LES model configuration with model-specific parameters |
| **Schemes & Solvers** | Structured editing of `fvSchemes` and `fvSolution` |
| **snappyHexMesh** | Full `snappyHexMeshDict` configuration panel |
| **Dict Panel** | 22+ dictionary types: `blockMeshDict`, `controlDict`, `waveProperties` (7 wave models), etc. |
| **Drag-to-Resize** | Drag splitters to adjust panel areas (param tree / preview / section list) |
| **Sync Boundaries** | One-click sync `blockMeshDict` patches → all `0/` field files |
| **File Viewer** | PNG/JPG/SVG/EPS (zoom, fit); PDF/Office via system default |
| **Run Python** | Execute `.py` from editor, configurable interpreter path |
| **Run C++** | Compile with `g++ -std=c++17 -O2` + run, configurable compiler path |
| **ParaView** | One-click launch, auto `.foam` file, path config, download guide |
| **Terminal** | Open system terminal in case directory |
| **Toolbar** | Drag-to-reorder, View menu toggles, Reset Default Layout |

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+O` | Open Case |
| `Ctrl+S` | Save |
| `Ctrl+W` | Close Tab |
| `Ctrl+F` | Find |
| `Ctrl+/` | Toggle Comment |
| `Ctrl+Z` / `Ctrl+Y` | Undo / Redo |
| `Ctrl+B` | Toggle BC Panel |
| `Ctrl+Shift+P` | Run Python |
| `Ctrl+Shift+C` | Run C++ |
| `Ctrl+N` | New File |
| `Ctrl+Shift+N`| New Folder |

---

## Editor Features

### Auto Completion
Three independent systems (C++, Python, OpenFOAM), toggleable via **Edit** menu. Activates after 2+ characters. OpenFOAM completion includes 120+ solvers, shell scripts (Allrun/Allclean), and utilities across all OF versions (v2012–v2512). Bash files also trigger OF completion for run-script editing.

### Auto Indent
Enter preserves indentation. `{` on empty line generates code block. Cursor in `{}` + Enter pushes `}` down.

### #include Headers
Typing `<` after `#include` triggers 50+ C/C++/OpenFOAM header suggestions.

---

## Sync Boundaries

`View → Sync Boundaries` copies blockMesh boundary names into all `0/` or `0.orig/` field files. Open editor tabs auto-reload.

| Patch Type | Field | Default BC |
|-----------|-------|-------------|
| `wall` | U | `noSlip` |
| `wall` | p/p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut | `nutkWallFunction` |
| `empty`/`symmetry`/`wedge`/`cyclic` | any | same as type |

---

## File Viewer

| Format | Viewer |
|--------|--------|
| PNG, JPG, BMP, GIF, WebP, ICO | Native zoom (− / + / Fit / 1:1) |
| SVG | QSvgWidget |
| EPS, EPSF, PS | Ghostscript → PNG (auto-detect) |
| PDF, DOC, DOCX, XLS, XLSX | System default |

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2+)
- **OpenFOAM** (v2012–v2512)
- **ParaView** 5.10+ (optional)
- **Python 3.x** (optional, for Run Python)
- **g++** (optional, for Run C++; MinGW includes it)
- **Ghostscript** (optional, for EPS; TeX Live includes it)

## Build

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# Output: release/OpenFOAMGUI.exe
```

**Qt modules**: Core, GUI, Widgets, SvgWidgets · **Standard**: C++17

---

## Project Structure

```
OpenFOAMGUI/
├── .readthedocs.yaml          # Read the Docs config
├── mkdocs.yml                 # MkDocs documentation config
├── docs/                      # Online documentation source
├── src/                       # Source code (19 source pairs)
│   ├── main.cpp, mainwindow.* # Entry point + main window
│   ├── casebrowser.*          # Case directory tree (lazy-loading, rename)
│   ├── codeeditor.*           # Editor + 500+ keyword auto-completion
│   ├── fileviewer.*           # Image/EPS/PDF/Office viewer
│   ├── ofhighlighter.*        # OpenFOAM/C++ syntax highlighting
│   ├── ofparser.*             # OpenFOAM file parser
│   ├── ofmeshreader.*         # OpenFOAM polyMesh reader
│   ├── languagedetector.*     # File language detection
│   ├── linenumberarea.*       # Line number margin widget
│   ├── bcpanel.* + bctypedatabase.*  # BC panel + 120+ BC types
│   ├── turbulencepanel.* + turbulencemodeldatabase.*
│   ├── schemespanel.*         # fvSchemes / fvSolution
│   ├── snappypanel.*          # snappyHexMeshDict
│   ├── dictpanel.*            # 22+ dictionary types (waveProperties, etc.)
│   └── bychen.ico             # Application icon
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
