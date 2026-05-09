# OpenFOAM GUI

A professional CFD case manager and editor for OpenFOAM, built with Qt 6.10.2.

Browse, edit, and save OpenFOAM case files with syntax highlighting, auto-completion, case structure awareness, and integrated configuration panels.

---

## Quick Links

- [Features](features.md) — full list of capabilities
- [Usage Guide](usage.md) — how to use the application
- [Build Instructions](build.md) — compile from source
- [Project Structure](structure.md) — codebase layout
- [Changelog](changelog.md) — version history
- [GitHub Repository](https://github.com/byChen47/OpenFOAMGUI)

---

## Key Features

| Module | Description |
|--------|-------------|
| **Case Browser** | Lazy-loading tree view, multi-case, filter support |
| **Code Editor** | Multi-tab, syntax highlighting, auto-completion, auto-indent |
| **C++ Completion** | 200+ STL keywords, header suggestions |
| **Python Completion** | Python 3 builtins and libraries |
| **OpenFOAM Completion** | 250+ BC types, schemes, solvers |
| **Boundary Conditions** | RTM parameter table, 100+ BC types, smart suggestions |
| **Sync Boundaries** | One-click sync blockMeshDict to 0/ field files |
| **Run Python / C++** | Execute scripts from editor, configurable paths |
| **ParaView Integration** | One-click launch, auto-detection |
| **File Viewer** | PNG/JPG/SVG/EPS with zoom, PDF/Office via system |

---

## Requirements

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2+)
- **OpenFOAM** (v2012–v2512)
- **ParaView** 5.10+ (optional)
- **Python** 3.x (optional)
- **g++** (optional, for Run C++)
