# OpenFOAM GUI

A CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

Open, browse, edit, and save OpenFOAM case files with syntax highlighting, case structure awareness, and integrated configuration panels — no manual dictionary editing required.

---

## Table of Contents

- [Features Overview](#features-overview)
- [Boundary Conditions Panel](#boundary-conditions-panel)
- [Turbulence Model Panel](#turbulence-model-panel)
- [Discretisation & Solvers Panel](#discretisation--solvers-panel)
- [snappyHexMesh Panel](#snappyhexmesh-panel)
- [General Dict Panel](#general-dict-panel)
- [Editor & Case Browser](#editor--case-browser)
- [Requirements](#requirements)
- [Build](#build)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [License](#license)

---

## Features Overview

| Module | Description |
|--------|-------------|
| **Case Browser** | Tree view of `0/`, `constant/`, `system/` directories; multi-case support; file filter |
| **Code Editor** | Multi-tab editor with line numbers, current-line highlighting, syntax highlighting |
| **Syntax Highlighter** | Full OpenFOAM dictionary syntax: keywords, scalars, vectors, macros, dimensions |
| **File Type Detection** | Auto-detects OpenFOAM field files, dictionaries, fvSchemes, etc. with tooltips |
| **Boundary Conditions Panel** | Visual BC editing with RTM parameter tables, patch browser, smart suggestions |
| **Turbulence Model Panel** | RAS/LES model selection with model-specific parameter forms |
| **Discretisation & Solvers Panel** | Structured editing of `fvSchemes` and `fvSolution` |
| **snappyHexMesh Panel** | Full snappyHexMeshDict configuration with section navigation |
| **General Dict Panel** | Edit blockMeshDict, controlDict, decomposeParDict, topoSetDict, etc. |
| **ParaView Integration** | One-click launch ParaView from the Case menu |
| **Terminal** | Open system terminal in the case directory |
| **Recent Cases** | Quick access to recently opened cases (up to 10) |

---

## Boundary Conditions Panel

The BC Panel is the most feature-rich module, designed to match the official OpenFOAM documentation format (RTM — **R**equired / **T**ype / **M**andatory).

### Supported Fields

Automatically detects field type from the OpenFOAM header and provides context-aware BC lists:

| Field Type | FoamFile Class | Example Fields | Icon |
|------------|---------------|----------------|------|
| **Scalar** | `volScalarField` | `p`, `p_rgh`, `k`, `epsilon`, `omega`, `nut`, `T`, `alpha.*` | `S` |
| **Vector** | `volVectorField` | `U`, `v` | `V` |
| **Tensor** | `volTensorField` | `tau`, `R` | `T` |
| **SymmTensor** | `volSymmTensorField` | — | `T` |
| **SurfaceScalar** | `surfaceScalarField` | `phi` | — |
| **SurfaceVector** | `surfaceVectorField` | `Uf` | — |

### Field-Aware BC Sorting

BC types are intelligently sorted for each field. For example, opening a **U** field file promotes:
- `noSlip`, `slip` (wall types) to the top
- `flowRateInletVelocity`, `pressureInletOutletVelocity` (inlet types) next
- Followed by other applicable types grouped by category

Opening **p** or **p_rgh** promotes `zeroGradient`, `fixedFluxPressure`, `totalPressure` to the top.

### Category Filter Tabs

All BC types are organized into 8 filterable categories:

| Tab | Categories Included | Example Types |
|-----|---------------------|---------------|
| **All** | All types for current field | — |
| **Basic** | Fundamental BCs | `fixedValue`, `zeroGradient`, `fixedGradient`, `mixed`, `calculated` |
| **Wall** | Wall BCs + wall functions | `noSlip`, `slip`, `kqRWallFunction`, `epsilonWallFunction`, `omegaWallFunction`, `nutkWallFunction` |
| **Inlet** | Inlet conditions | `flowRateInletVelocity`, `pressureInletOutletVelocity`, `totalPressure`, `turbulentIntensityKineticEnergyInlet`, `turbulentDFSEMInlet` |
| **Outlet** | Outlet conditions | `inletOutlet`, `advective`, `waveTransmissive`, `freestream`, `flowRateOutletVelocity` |
| **Pressure** | Pressure-specific | `totalPressure`, `fixedFluxPressure`, `fixedMean`, `fanPressure`, `uniformTotalPressure` |
| **Mapped** | Mapped / coupled | `mappedFixedValue`, `timeVaryingMappedFixedValue`, `mappedMixed`, `fixedProfile` |
| **Constraint** | Patch constraints | `empty`, `symmetry`, `wedge`, `cyclic`, `cyclicAMI`, `processor` |
| **Coded** | User-coded (C++) | `codedFixedValue`, `codedMixed` |

### Type Search

A search box above the BC type list filters types in real-time by name or description (e.g. typing "wall" shows all wall-function types).

### RTM Parameter Table

Selecting a BC type displays a detailed parameter table matching the official OpenFOAM documentation:

| Column | Description |
|--------|-------------|
| **Property** | Parameter keyword (e.g. `value`, `gradient`, `p0`, `gamma`) |
| **Description** | Human-readable explanation of the parameter |
| **Type** | Parameter type (`scalar`, `vector`, `word`, `Function1<scalar>`, etc.) |
| **Required** | `yes` (red) or `no` (green) — inherited from the official source `.H` files |
| **Default** | Default value; **editable** — double-click to customize before inserting |

The `type` row is always shown at the top with a highlighted background. Required parameters are listed before optional ones.

#### Editable Default Values

The **Default** column cells are editable. Change the default value before applying, and the preview updates in real-time. This allows you to:
- Set custom `value` (e.g. `uniform (10 0 0)` for a U inlet)
- Adjust model constants (e.g. `Cmu`, `kappa`, `E` for wall functions)
- Configure inlet parameters (e.g. `flowRate`, `intensity`)

### Patch Browser

When a field file is loaded, all patches in the `boundaryField` block are parsed and displayed:

- **Patch name** with current **type** (color-coded):
  - Blue: fixedValue / uniformFixedValue / flowRate types
  - Red: noSlip / slip / wall types
  - Green: zeroGradient / empty / symmetry / wedge
  - Orange: inlet/outlet types
  - Purple: mapped / coupled types
  - Gray: calculated
- Click a patch to auto-select its current BC type in the list
- If a patch has no type (`?`), the panel suggests the most appropriate BC based on:
  - Field name (U, p, k, epsilon, omega, nut, alpha.*, T)
  - Patch name (inlet, outlet, wall, atmosphere, symmetry)

### Smart BC Suggestions

When clicking an unconfigured patch (type `?`), the panel applies heuristic rules:

| Field | Patch Name Contains | Suggested BC |
|-------|---------------------|--------------|
| U | wall, plate, bottom, top | `noSlip` |
| U | inlet, in, entry | `flowRateInletVelocity` |
| U | outlet, out, exit | `pressureInletOutletVelocity` |
| p / p_rgh | wall | `fixedFluxPressure` |
| p / p_rgh | inlet | `totalPressure` |
| p / p_rgh | outlet | `zeroGradient` |
| k / epsilon / omega | wall | `kqRWallFunction` / `epsilonWallFunction` / `omegaWallFunction` |
| nut | wall | `nutkWallFunction` |
| alpha.* | wall | `zeroGradient` |
| alpha.* | outlet | `inletOutlet` |
| T | wall | `zeroGradient` |
| any | symmetry, sym | `symmetry` |

### Code Preview

A dark-themed code preview panel shows the generated dictionary snippet in real-time as you change parameters. Optional parameters are shown commented out with `// (optional)` annotations.

### Apply / Insert Actions

- **Apply to Editor** — inserts the BC snippet at the current cursor position in the editor
- **Refresh Preview** — regenerates the preview with current parameter values
- **Right-click context menu** on any BC type:
  - *Insert type name only* — inserts just the BC type word
  - *Insert BC Snippet* — inserts the full formatted block with parameters
  - *Insert for patch \<name\>* — inserts with the selected patch name

### Complete BC Type Database

The built-in database contains **100+ boundary condition types** verified against OpenFOAM v2206 source `.H` files, including:

| Category | Count | Examples |
|----------|-------|----------|
| **Basic** | 10 | `fixedValue`, `zeroGradient`, `fixedGradient`, `mixed`, `calculated`, `scaledFixedValue`, `uniformFixedValue`, `uniformFixedGradient` |
| **Wall** | 16 | `noSlip`, `slip`, `partialSlip`, `movingWallVelocity`, `rotatingWallVelocity`, `translatingWallVelocity`, `fixedNormalSlip`, `activeBaffleVelocity` + 8 wall functions |
| **Inlet** | 22 | `flowRateInletVelocity`, `pressureInletOutletVelocity`, `turbulentIntensityKineticEnergyInlet`, `turbulentDFSEMInlet`, `turbulentDigitalFilterInlet`, `cylindricalInletVelocity`, `swirlFlowRateInletVelocity`, `pressurePIDControlInletVelocity`, `variableHeightFlowRateInletVelocity`, etc. |
| **Outlet** | 14 | `inletOutlet`, `outletInlet`, `advective`, `waveTransmissive`, `freestream`, `supersonicFreestream`, `flowRateOutletVelocity`, `outletMachNumberPressure`, `acousticWaveTransmissive`, etc. |
| **Pressure** | 16 | `totalPressure`, `uniformTotalPressure`, `prghTotalPressure`, `fixedFluxPressure`, `fixedMean`, `fanPressure`, `uniformDensityHydrostaticPressure`, `plenumPressure`, `rotatingTotalPressure`, `syringePressure`, etc. |
| **Mapped** | 7 | `mappedFixedValue`, `timeVaryingMappedFixedValue`, `mappedMixed`, `mappedFlowRate`, `fixedProfile`, `mappedFixedInternalValue`, `mappedVelocityFluxFixedValue` |
| **Special** | 10 | `fixedJump`, `fixedJumpAMI`, `uniformJump`, `uniformJumpAMI`, `fanPressure`, `interfaceCompression`, `waveSurfacePressure`, etc. |
| **Coded** | 2 | `codedFixedValue`, `codedMixed` |
| **Constraint** | 14 | `empty`, `symmetry`, `symmetryPlane`, `wedge`, `cyclic`, `cyclicAMI`, `cyclicACMI`, `cyclicSlip`, `processor`, `processorCyclic`, `patch`, `wall` |
| **Turbulence Wall Functions** | 8 | `kqRWallFunction`, `epsilonWallFunction`, `omegaWallFunction`, `nutkWallFunction`, `nutUSpaldingWallFunction`, `nutUWallFunction`, `nutkRoughWallFunction`, `nutURoughWallFunction`, `nutLowReWallFunction`, `nutUBlendedWallFunction`, `nutUTabulatedWallFunction`, `kLowReWallFunction`, `alphatWallFunction` |

---

## Turbulence Model Panel

Configures `constant/turbulenceProperties` through a visual form interface.

### Features

- **RAS / LES toggle** — switches between Reynolds-Averaged and Large-Eddy simulation models
- **Model selection** — dropdown list of all OpenFOAM turbulence models:
  - RAS: `laminar`, `kEpsilon`, `kOmega`, `kOmegaSST`, `kOmegaSSTLM`, `RNGkEpsilon`, `realizableKE`, `LaunderSharmaKE`, `kOmega2006`, `v2f`, `LRR`, `SSG`, `EBRSM`
  - LES: `Smagorinsky`, `WALE`, `dynamicKEqn`, `kEqn`, `dynamicLagrangian`, `DeardorffDiffStress`, `SpalartAllmaras`
- **Turbulence / Print coeffs** — on/off switches
- **Model-specific parameters** — automatically loaded for each model (e.g. `Cmu`, `C1`, `C2`, `sigmak`, `sigmaEps`, `alphaK1`, `alphaOmega1`, `Ce`, `Ck`, etc.)
- **Editable parameter table** with descriptions and default values
- **Preview** of the generated `turbulenceProperties` dictionary
- **Apply to Editor** inserts the configured block

---

## Discretisation & Solvers Panel

Configures `system/fvSchemes` and `system/fvSolution` through a structured form.

### fvSchemes Categories

| Category | Description | Common Options |
|----------|-------------|----------------|
| `ddtSchemes` | Time derivative | `Euler`, `backward`, `steadyState`, `CrankNicolson`, `localEuler` |
| `gradSchemes` | Gradient | `Gauss linear`, `leastSquares`, `cellLimited Gauss linear` |
| `divSchemes` | Divergence | `Gauss linear`, `Gauss linearUpwind`, `Gauss limitedLinear` |
| `laplacianSchemes` | Laplacian | `Gauss linear corrected`, `Gauss linear orthogonal` |
| `interpolationSchemes` | Interpolation | `linear`, `cubicCorrection`, `midPoint` |
| `snGradSchemes` | Surface normal gradient | `corrected`, `orthogonal`, `limited` |
| `fluxRequired` | Flux calculation | `none`, `p`, `phi` |

### fvSolution Categories

| Category | Description |
|----------|-------------|
| **Solvers** | Linear solver selection (PCG, PBiCG, GAMG, smoothSolver) with preconditioner, tolerance, relTol |
| **Relaxation Factors** | Field-level under-relaxation (U, p, k, epsilon, omega, etc.) |
| **PIMPLE / SIMPLE** | Algorithm controls: `nOuterCorrectors`, `nCorrectors`, `momentumPredictor`, `pRefCell`, `pRefValue` |

### Features

- **Toggle between fvSchemes / fvSolution** via tabs
- **Each category** has its own sub-section with appropriate dropdowns, spinboxes, and text fields
- **Preset schemes** for quick setup (e.g. "2nd order", "1st order upwind")
- **Apply to Editor** writes the configured dictionary

---

## snappyHexMesh Panel

Configures `system/snappyHexMeshDict` with structured navigation.

### Sections

| Section | Key Parameters |
|---------|---------------|
| **Geometry** | Mesh refinement regions, file references |
| **Castellated Mesh** | `maxGlobalCells`, `maxLocalCells`, `minRefinementCells`, `nCellsBetweenLevels`, `resolveFeatureAngle`, `allowFreeStandingZoneFaces` |
| **Features** | Feature edge refinement: `file`, `levels` |
| **Refinement Surfaces** | Per-surface: `level`, `patchInfo` |
| **Snapping** | `nSmoothPatch`, `tolerance`, `nSolveIter`, `nRelaxIter` |
| **Layering** | `nSurfaceLayers`, `expansionRatio`, `finalLayerThickness`, `minThickness`, `featureAngle`, `slipFeatureAngle` |
| **Mesh Quality** | `maxNonOrtho`, `maxBoundarySkewness`, `maxInternalSkewness`, `maxConcave`, `minVol`, `minTetQuality`, `minArea`, `minTwist` |

### Features

- Section-based navigation with a category list
- Each parameter displayed with type, default value, and description
- Editable values with real-time preview
- **Apply to Editor** writes the full `snappyHexMeshDict`

---

## General Dict Panel

Configures various OpenFOAM dictionaries with pre-defined parameter sets.

### Supported Dictionaries

| Dictionary | Key Parameters |
|------------|---------------|
| **blockMeshDict** | `convertToMeters`, vertices, blocks, edges, boundaries (with predefined patch types) |
| **controlDict** | `startFrom`, `startTime`, `stopAt`, `endTime`, `deltaT`, `writeControl`, `writeInterval`, `purgeWrite`, `writeFormat`, `writePrecision`, `runTimeModifiable` |
| **decomposeParDict** | `numberOfSubdomains`, `method` (simple, hierarchical, scotch, metis, manual), `coeffs` |
| **topoSetDict** | Set definitions, actions |
| **dynamicMeshDict** | Dynamic mesh configuration |
| **refineMeshDict** | Refinement parameters |

### Features

- Dictionary selector (auto-detects based on file name)
- Parameter table with type, default, and description
- Editable values
- **Apply to Editor** writes the formatted dictionary

---

## Editor & Case Browser

### Code Editor

- **Syntax Highlighting** — OpenFOAM keywords, C++ keywords, scalars, vectors, tensors, macros (`#include`, `#ifdef`), FoamFile headers, units/dimensions
- **Line Numbers** — with current-line highlighting
- **Multi-tab** — Ctrl+W closes a single file; Ctrl+Tab switches between files
- **Unsaved Changes** — `*` indicator in tab title and confirmation on close
- **Comment/Uncomment** — `Ctrl+/` toggles `//` or `#` based on file language
- **Find** — `Ctrl+F` with text search
- **Undo/Redo** — `Ctrl+Z` / `Ctrl+Y`
- **File Type Detection** — Auto-detects OpenFOAM vs generic based on header patterns

### Case Browser

- Tree view mirroring case structure: time dirs → `constant/` (with sub-dirs like `polyMesh/`) → `system/`
- File-type-aware icons (field files, dictionaries, generic)
- Tooltips with file descriptions
- **Filter** box for quick file search
- **Right-click context menu**:
  - Open File / Close Case / Refresh Case
  - New File / New Folder / Delete
- **Multi-case support** — open and manage multiple cases simultaneously

---

## ParaView Integration

Launch ParaView directly from the GUI for post-processing without manually creating `.foam` files or opening a terminal.

### How It Works

1. **Case → ParaView** (or the ParaView toolbar button) launches ParaView with the current OpenFOAM case
2. A `.foam` dummy file is automatically created in the case root directory (required by ParaView's native OpenFOAM reader)
3. ParaView opens with `--data=<caseName>.foam`, ready for post-processing

### ParaView Path Configuration

The application searches for ParaView in the following order:

**On Windows:**
```
1. User-configured path (set via Case → ParaView Path...)
2. paraview.exe (in system PATH)
3. C:/Program Files/ParaView 5.13/bin/paraview.exe
4. C:/Program Files/ParaView 5.12/bin/paraview.exe
5. C:/Program Files/ParaView 5.11/bin/paraview.exe
6. C:/Program Files/ParaView 5.10/bin/paraview.exe
7. C:/Program Files/ParaView/bin/paraview.exe
```

**On Linux:**
```
1. User-configured path
2. paraview (in system PATH)
3. /usr/bin/paraview
4. /usr/local/bin/paraview
```

If ParaView is not found, a dialog prompts you to browse for the executable. You can also manually configure the path anytime:

- **Case → ParaView Path...** — opens a file browser to select the ParaView executable
- The configured path is persisted across sessions (stored in `QSettings`)

### Download ParaView

ParaView is a free, open-source, multi-platform data analysis and visualization application.

| Platform | Download |
|----------|----------|
| **Windows** | [ParaView 5.13.2 Windows (64-bit)](https://www.paraview.org/download/) |
| **Linux** | [ParaView 5.13.2 Linux (64-bit)](https://www.paraview.org/download/) |
| **macOS** | [ParaView 5.13.2 macOS](https://www.paraview.org/download/) |
| **All versions** | [https://www.paraview.org/download/](https://www.paraview.org/download/) |

> **Recommended**: Install ParaView 5.10 or later for best compatibility with OpenFOAM native reader. During installation on Windows, the default path `C:/Program Files/ParaView <version>/` will be auto-detected by the GUI.

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0+)
- **OpenFOAM** (v2012 through v2512)
- **ParaView** 5.10+ (optional, for post-processing via Case → ParaView)

## Build

### Prerequisites

Ensure the following tools are installed and available:

| Tool | Required Version | Typical Path (Windows) |
|------|-----------------|------------------------|
| **Qt** | 6.10.2 (MinGW 64-bit) | `D:/3.Wpsandother/Qt/setting/6.10.2/mingw_64/` |
| **MinGW-w64** | GCC 15.2.0+ | `D:/3.Wpsandother/mingw64/` |
| **Git** | 2.x (for version control) | `D:/3.Wpsandother/gitForWindow/setting/Git/` |

### Build Configuration (OpenFOAMGUI.pro)

```pro
QT       += core gui widgets
CONFIG   += c++17 console

# Windows: GUI application (no console window), MinGW linker flags
mingw {
    QMAKE_LFLAGS_CONSOLE = -Wl,-subsystem,windows -mthreads
}

TARGET   = OpenFOAMGUI
TEMPLATE = app

SOURCES  += src/main.cpp src/mainwindow.cpp src/casebrowser.cpp \
            src/codeeditor.cpp src/ofhighlighter.cpp src/ofparser.cpp \
            src/linenumberarea.cpp src/languagedetector.cpp \
            src/bctypedatabase.cpp src/bcpanel.cpp \
            src/turbulencemodeldatabase.cpp src/turbulencepanel.cpp \
            src/schemespanel.cpp src/snappypanel.cpp src/dictpanel.cpp

HEADERS  += src/mainwindow.h src/casebrowser.h src/codeeditor.h \
            src/ofhighlighter.h src/ofparser.h src/linenumberarea.h \
            src/languagedetector.h src/bctypedatabase.h src/bcpanel.h \
            src/turbulencemodeldatabase.h src/turbulencepanel.h \
            src/schemespanel.h src/snappypanel.h src/dictpanel.h

RESOURCES += resources.qrc
RC_ICONS   = src/bychen.ico
```

- **C++17** standard with GNU extensions (`-std=gnu++1z`)
- **Qt modules**: Core, GUI, Widgets
- **Linker**: `-Wl,-subsystem,windows` suppresses the console window on Windows
- **Resources**: `resources.qrc` embeds the application icon; `RC_ICONS` sets the Windows `.exe` icon

### Command-Line Build (Windows)

Open a terminal (Git Bash, MSYS2, or Command Prompt) and set up the environment:

```bash
# Add Qt and MinGW to PATH
export PATH="D:/3.Wpsandother/Qt/setting/6.10.2/mingw_64/bin:$PATH"
export PATH="D:/3.Wpsandother/mingw64/bin:$PATH"

# Navigate to the project directory
cd d:/3.OpenFOAM/0.OpenFOAM/OpenFOAMGUI

# Step 1: Generate Makefile from .pro
qmake OpenFOAMGUI.pro

# Step 2: Build (Release configuration)
mingw32-make -f Makefile.Release

# Output: release/OpenFOAMGUI.exe
```

**Build targets:**

| Command | Description |
|---------|-------------|
| `mingw32-make -f Makefile.Release` | Release build (optimized, `-O2`) |
| `mingw32-make -f Makefile.Debug` | Debug build (with symbols, `-g`) |
| `mingw32-make -f Makefile.Release clean` | Clean release build artifacts |
| `mingw32-make -f Makefile.Release distclean` | Remove all generated files |

**Compiler flags (Release):**
```
-O2 -std=gnu++1z -Wall -Wextra -fexceptions -mthreads
-DUNICODE -D_UNICODE -DWIN32 -DMINGW_HAS_SECURE_API=1
```

**Linker flags (Release):**
```
-Wl,-s -Wl,-subsystem,windows -mthreads
```

**Linked libraries:**
```
libQt6Widgets.a  libQt6Gui.a  libQt6Core.a
```

### Qt Creator Build

1. Open **Qt Creator**
2. **File → Open File or Project...** → select `OpenFOAMGUI.pro`
3. When prompted, select the **MinGW 64-bit** kit (Qt 6.10.2)
4. Choose **Release** build configuration
5. Click **Build → Build Project** (or `Ctrl+B`)
6. The executable is output to `release/OpenFOAMGUI.exe`

### Linux / macOS Build

```bash
# Ensure Qt 6.10+ and GCC/Clang are installed
qmake6 OpenFOAMGUI.pro     # or qmake (depending on your distro)
make -f Makefile.Release

# Output: release/OpenFOAMGUI
```

> **Note**: The `RC_ICONS` and Windows-specific linker flags are ignored on non-Windows platforms via the `mingw { }` conditional block.

## Usage

1. Launch `OpenFOAMGUI.exe`
2. **File → Open Case** (`Ctrl+O`) and select an OpenFOAM case directory
3. Double-click any file in the **Case Browser** dock to open it in the editor
4. The right-side panel automatically switches context:
   - Field files (U, p, k, etc.) → **Boundary Conditions** panel
   - `turbulenceProperties` → **Turbulence Model** panel
   - `fvSchemes` / `fvSolution` → **Discretisation & Solvers** panel
   - `snappyHexMeshDict` → **snappyHexMesh** panel
   - `blockMeshDict`, `controlDict`, etc. → **General Dict** panel
5. Configure parameters in the panel, then click **Apply to Editor** to insert/modify
6. **Save** (`Ctrl+S`) to write changes back to the case

---

## Project Structure

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                       # Application entry point
│   ├── mainwindow.h / .cpp            # Main window, menus, toolbar, dock widgets
│   ├── casebrowser.h / .cpp           # Case directory tree browser
│   ├── codeeditor.h / .cpp            # Code editor with line numbers
│   ├── ofhighlighter.h / .cpp         # OpenFOAM/C++ syntax highlighter
│   ├── ofparser.h / .cpp              # OpenFOAM file header/keyword parser
│   ├── languagedetector.h / .cpp      # File language auto-detection (OF vs C++)
│   ├── linenumberarea.h / .cpp        # Line number margin widget
│   ├── bcpanel.h / .cpp               # Boundary conditions panel (RTM table + patches)
│   ├── bctypedatabase.h / .cpp        # 100+ BC type definitions database
│   ├── turbulencepanel.h / .cpp       # RAS/LES turbulence model panel
│   ├── turbulencemodeldatabase.h / .cpp  # Turbulence model definitions
│   ├── schemespanel.h / .cpp          # fvSchemes / fvSolution panel
│   ├── snappypanel.h / .cpp           # snappyHexMeshDict panel
│   ├── dictpanel.h / .cpp             # Generic dictionary panel
│   └── bychen.ico                     # Application icon
├── OpenFOAMGUI.pro                    # Qt project file
├── resources.qrc                      # Qt resource file
└── README.md
```

## License

This project is provided for educational and research purposes in CFD workflows.
