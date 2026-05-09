# Features

## Case Browser

- Lazy-loading tree view: only loads directory contents when expanded
- Multi-case support with tabbed interface
- File filter box for quick search
- File-type-aware icons (field files, dictionaries, images)
- Right-click context menu: Open, Open With..., Close Case, Refresh, New File/Folder, Delete

## Code Editor

### Syntax Highlighting
OpenFOAM dictionary syntax, C++ keywords, macros (`#include`, `#ifdef`), FoamFile headers, units/dimensions.

### Auto Completion
Three independent completion systems, toggleable via **Edit** menu:

| Language | Keywords | Toggle |
|----------|----------|--------|
| C++ | 200+ STL containers, algorithms, smart pointers, streams, C stdlib | Edit → C++ Auto Completion |
| Python | Python 3 builtins, numpy, pandas | Edit → Python Auto Completion |
| OpenFOAM | 250+ BC types, schemes, solvers, snappyHexMesh, turbulence models | Edit → OpenFOAM Auto Completion |

### #include Header Completion
Typing `<` after `#include` triggers 50+ header suggestions (C, C++, OpenFOAM).

### Auto Indent & Smart Braces
- Enter preserves current indentation level
- `{` on an empty line generates a formatted code block with closing `}`
- Cursor inside `{}` + Enter pushes closing brace to the next line

### Tab Management
- Right-click tab for Close Current / Close Others / Close All
- `Ctrl+W` close, `Ctrl+Tab` switch, `*` unsaved indicator

## Boundary Conditions Panel

- **RTM Parameter Table**: Property | Description | Type | Required | Default
- Editable default values with real-time preview
- **100+ BC types** verified against OpenFOAM v2206 `.H` sources
- **Patch Browser**: auto-parses `boundaryField`, color-coded by type
- **Smart Suggestions**: heuristic BC recommendation based on field + patch name
- **Category Filters**: All, Basic, Wall, Inlet, Outlet, Pressure, Mapped, Constraint, Coded

## Turbulence Model Panel

- RAS / LES toggle
- All OpenFOAM turbulence models (kEpsilon, kOmegaSST, Smagorinsky, WALE, etc.)
- Model-specific parameters with descriptions and defaults

## Schemes & Solvers Panel

- Structured editing of `fvSchemes` (ddt, grad, div, laplacian, interpolation, snGrad, fluxRequired)
- `fvSolution` (solvers, relaxation factors, PIMPLE/SIMPLE parameters)
- Preset format suggestions

## snappyHexMesh Panel

Full `snappyHexMeshDict` configuration with section navigation:
- Geometry, Castellated Mesh, Features, Refinement Surfaces, Snapping, Layering, Mesh Quality

## General Dict Panel

- `blockMeshDict`, `controlDict`, `decomposeParDict`, `topoSetDict`, `dynamicMeshDict`, `refineMeshDict`
- Auto-detection based on file name

## Sync Boundaries

`View → Sync Boundaries` copies blockMesh boundary names into all field files in `0/` or `0.orig/`. Open editor tabs auto-reload.

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

## File Viewer

| Format | Viewer |
|--------|--------|
| PNG, JPG, BMP, GIF, WebP, ICO | Native zoomable viewer (− / + / Fit / 1:1) |
| SVG | QSvgWidget vector renderer |
| EPS, EPSF, PS | Ghostscript → PNG (auto-detect) |
| PDF, DOC, DOCX, XLS, XLSX, PPT, PPTX | System default application |

Right-click any file → **Open With...** to choose an external program.

## Run Python

- **Case → Run Python** (`Ctrl+Shift+P`) executes current `.py` file
- Auto-detects Python from PATH; configurable via **Case → Python Path...**
- Terminal-style output dialog with dark/light toggle and copy button
- 60-second timeout protection

## Run C++

- **Case → Run C++** (`Ctrl+Shift+C`) compiles with `g++ -std=c++17 -O2` and runs
- Auto-detects g++ from MinGW/MSYS2 paths; configurable via **Case → C++ Compiler Path...**
- Terminal-style output dialog with compilation errors display

## ParaView Integration

- **Case → ParaView** launches ParaView with auto-created `.foam` file
- Auto-detects ParaView at common paths; configurable via **Case → ParaView Path...**
- Shows download URL if not installed
- Path persisted in QSettings

## Terminal

**Case → Terminal** (`Ctrl+`` `) opens Windows Terminal or Command Prompt in the case directory.

## Customizable Toolbar

- Drag buttons to reorder within toolbar
- **View menu** toggles show/hide each toolbar button
- **View → Reset Default Layout** restores original arrangement
- Custom painted icons for Python, C++, Terminal, Sync Boundaries, ParaView

## Menu Structure

```
File              Edit                Case                View
────              ────                ────                ────
Recent Cases >    Undo                Open Case           Show New File
────────────────  Redo                Close Case          Show New Folder
Save     Ctrl+S   ────────────        ────────────        Show BC Panel
Save As           Find    Ctrl+F      New File            Show Terminal
Save All          Comment Ctrl+/      New Folder          Show Run Python
────────────────  ────────────        Delete              Show Run C++
New File          C++ Completion      Clean Time Dirs     Show Sync Boundaries
New Folder        Python Completion   ────────────        Show ParaView
────────────────  OpenFOAM Completion Run Python          ────────────
Close Tab Ctrl+W                      Run C++             Reset Layout
────────────────                      ParaView            ────────────
Exit     Ctrl+Q                       ParaView Path...    Case Browser
                                      Python Path...      BC Panel
                                      C++ Compiler Path...
                                      ────────────
                                      Refresh Case
```
