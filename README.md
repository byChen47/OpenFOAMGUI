# OpenFOAM GUI

A CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

Open, browse, edit, and save OpenFOAM case files with syntax highlighting, case structure awareness, and integrated configuration panels.

---

## Features Overview

| Module | Description |
|--------|-------------|
| **Case Browser** | Tree view of `0/`, `constant/`, `system/` directories; multi-case support; file filter |
| **Code Editor** | Multi-tab editor with line numbers, syntax highlighting, tab context menu |
| **Syntax Highlighter** | Full OpenFOAM dictionary syntax: keywords, scalars, vectors, macros, dimensions |
| **File Type Detection** | Auto-detects OpenFOAM field files, dictionaries with tooltips |
| **Boundary Conditions Panel** | Visual BC editing with RTM parameter tables, patch browser, smart suggestions |
| **Turbulence Model Panel** | RAS/LES model selection with model-specific parameter forms |
| **Discretisation & Solvers Panel** | Structured editing of `fvSchemes` and `fvSolution` |
| **snappyHexMesh Panel** | Full snappyHexMeshDict configuration with section navigation |
| **General Dict Panel** | Edit blockMeshDict, controlDict, decomposeParDict, topoSetDict, etc. |
| **Sync Boundaries** | One-click sync blockMeshDict patches to all 0/ field files |
| **File Viewer** | PNG/JPG/SVG/EPS images with zoom controls; PDF/Office via system default app |
| **ParaView Integration** | One-click launch ParaView from the Case menu |
| **Terminal** | Open system terminal in the case directory |
| **Recent Cases** | Quick access to recently opened cases (up to 10) |

---

## Boundary Conditions Panel

The BC Panel matches the official OpenFOAM documentation format (RTM — Required / Type / Mandatory).

### Supported Fields

| Field Type | FoamFile Class | Example Fields | Icon |
|------------|---------------|----------------|------|
| **Scalar** | `volScalarField` | `p`, `p_rgh`, `k`, `epsilon`, `omega`, `nut`, `T`, `alpha.*` | `S` |
| **Vector** | `volVectorField` | `U`, `v` | `V` |

### Category Filter Tabs

| Tab | Examples |
|-----|----------|
| **All** | All types for current field |
| **Basic** | `fixedValue`, `zeroGradient`, `fixedGradient`, `mixed`, `calculated` |
| **Wall** | `noSlip`, `slip`, `kqRWallFunction`, `epsilonWallFunction`, `omegaWallFunction`, `nutkWallFunction` |
| **Inlet** | `flowRateInletVelocity`, `pressureInletOutletVelocity`, `totalPressure`, `turbulentIntensityKineticEnergyInlet` |
| **Outlet** | `inletOutlet`, `advective`, `waveTransmissive`, `freestream` |
| **Pressure** | `totalPressure`, `fixedFluxPressure`, `fixedMean`, `fanPressure` |
| **Mapped** | `mappedFixedValue`, `timeVaryingMappedFixedValue`, `mappedMixed`, `fixedProfile` |
| **Constraint** | `empty`, `symmetry`, `wedge`, `cyclic`, `cyclicAMI`, `processor` |
| **Coded** | `codedFixedValue`, `codedMixed` |

### Features

- **RTM Parameter Table** — Property | Description | Type | Required | Default, editable defaults
- **Patch Browser** — color-coded patch types, click to auto-select BC
- **Smart BC Suggestions** — heuristic rules based on field name + patch name
- **Code Preview** — dark-themed preview panel
- **Apply to Editor** — insert BC snippet at cursor position
- **100+ BC types** verified against OpenFOAM v2206 source `.H` files

---

## Sync Boundaries

`Case → Sync Boundaries` one-click synchronises boundary patch names from `system/blockMeshDict` to every field file in the `0/` (or `0.orig/`) time directory.

### Default BC Assignment

| Patch Type | Field | Default BC |
|-----------|-------|-------------|
| `wall` | U, v | `noSlip` |
| `wall` | p, p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut, alphat | `nutkWallFunction` |
| `wall` | T, other scalars | `zeroGradient` |
| `empty` | any | `empty` |
| `symmetry` | any | `symmetry` |
| `wedge` | any | `wedge` |
| `cyclic` | any | `cyclic` |
| `patch` | any | `zeroGradient` |

---

## File Viewer

| Category | Extensions | Viewer |
|----------|-----------|--------|
| **Raster Images** | PNG, JPG, JPEG, BMP, GIF, WebP, ICO | Native viewer with zoom (− / + / Fit / 1:1) |
| **Vector — SVG** | SVG | QSvgWidget renderer |
| **Vector — EPS** | EPS, EPSF, PS | Ghostscript → PNG (auto-detect from PATH, TeX Live) |
| **PDF** | PDF | System default PDF viewer |
| **Office** | DOC, DOCX, XLS, XLSX, PPT, PPTX | System default application |

### Open With...

Right-click any file in the **Case Browser** and choose **Open With...** to select an external program.

---

## Editor & Case Browser

### Code Editor
- **Syntax Highlighting** — OpenFOAM / C++ keywords, scalars, vectors, macros
- **Line Numbers** — with current-line highlighting
- **Multi-tab** — `Ctrl+W` close, `Ctrl+Tab` switch, right-click menu (Close Current / Others / All)
- **Unsaved Changes** — `*` indicator + close confirmation
- **Comment/Uncomment** — `Ctrl+/`
- **Find** — `Ctrl+F`

### Case Browser
- Tree view: time dirs → `constant/` → `system/`
- File-type-aware icons, filter box
- Right-click: Open File, **Open With...**, Close Case, Refresh, New File/Folder, Delete

---

## ParaView Integration

1. **Case → ParaView** launches ParaView with the current case
2. `.foam` file auto-created in case root
3. Auto-detects ParaView at common paths; configurable via **Case → ParaView Path...**

### Download
[https://www.paraview.org/download/](https://www.paraview.org/download/)

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0+)
- **OpenFOAM** (v2012 through v2512)
- **ParaView** 5.10+ (optional)
- **Ghostscript** (optional, for EPS rendering; included in TeX Live)

## Build

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# Output: release/OpenFOAMGUI.exe
```

### Project Configuration

```pro
QT       += core gui widgets svgwidgets
CONFIG   += c++17 console

SOURCES  += src/main.cpp src/mainwindow.cpp src/fileviewer.cpp \
            src/casebrowser.cpp src/codeeditor.cpp src/ofhighlighter.cpp \
            src/ofparser.cpp src/linenumberarea.cpp src/languagedetector.cpp \
            src/bctypedatabase.cpp src/bcpanel.cpp \
            src/turbulencemodeldatabase.cpp src/turbulencepanel.cpp \
            src/schemespanel.cpp src/snappypanel.cpp src/dictpanel.cpp \
            src/ofmeshreader.cpp

HEADERS  += src/mainwindow.h src/fileviewer.h \
            src/casebrowser.h src/codeeditor.h src/ofhighlighter.h \
            src/ofparser.h src/linenumberarea.h src/languagedetector.h \
            src/bctypedatabase.h src/bcpanel.h \
            src/turbulencemodeldatabase.h src/turbulencepanel.h \
            src/schemespanel.h src/snappypanel.h src/dictpanel.h \
            src/ofmeshreader.h
```

**Qt modules**: Core, GUI, Widgets, SvgWidgets (SVG rendering)
**C++17** with `-std=gnu++1z`
**Libraries**: `Qt6SvgWidgets Qt6Svg Qt6Widgets Qt6Gui Qt6Core`

---

## Usage

1. Launch `OpenFOAMGUI.exe`
2. **Case → Open Case** (`Ctrl+O`) and select an OpenFOAM case directory
3. Double-click any file in the **Case Browser** to open it in the editor
4. Right-side panel auto-switches context:
   - Field files → **Boundary Conditions**
   - `turbulenceProperties` → **Turbulence Model**
   - `fvSchemes` / `fvSolution` → **Discretisation & Solvers**
   - `snappyHexMeshDict` → **snappyHexMesh**
   - `blockMeshDict`, `controlDict` → **General Dict**
5. **Save** (`Ctrl+S`) to write changes

---

## Project Structure

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                       # Entry point + plugin path setup
│   ├── mainwindow.h/.cpp              # Main window, menus, toolbar, docks
│   ├── casebrowser.h/.cpp             # Case directory tree browser
│   ├── codeeditor.h/.cpp              # Code editor + line numbers
│   ├── fileviewer.h/.cpp              # Image/EPS/PDF/Office file viewer
│   ├── ofhighlighter.h/.cpp           # OpenFOAM/C++ syntax highlighter
│   ├── ofparser.h/.cpp                # OpenFOAM file header/keyword parser
│   ├── ofmeshreader.h/.cpp            # OpenFOAM polyMesh parser (points/faces/boundary)
│   ├── languagedetector.h/.cpp        # File language auto-detection
│   ├── linenumberarea.h/.cpp          # Line number margin widget
│   ├── bcpanel.h/.cpp                 # Boundary conditions panel
│   ├── bctypedatabase.h/.cpp          # 100+ BC type definitions
│   ├── turbulencepanel.h/.cpp         # RAS/LES turbulence model panel
│   ├── turbulencemodeldatabase.h/.cpp # Turbulence model definitions
│   ├── schemespanel.h/.cpp            # fvSchemes / fvSolution panel
│   ├── snappypanel.h/.cpp             # snappyHexMeshDict panel
│   ├── dictpanel.h/.cpp               # Generic dictionary panel
│   └── bychen.ico                     # Application icon
├── OpenFOAMGUI.pro                    # Qt project file
├── resources.qrc                      # Qt resource file
├── qt.conf                            # Plugin path config
├── README.md                          # English documentation
├── README_CN.md                       # Chinese documentation
└── CHANGELOG.md                       # Modification diary
```

## License

This project is provided for educational and research purposes in CFD workflows.
