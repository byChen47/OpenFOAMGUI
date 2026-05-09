# Changelog

## Version 2.0

### New Features
- **3-way Auto Completion**: C++ (200+ keywords), Python, OpenFOAM (250+ BCs/schemes) with independent Edit menu toggles
- **#include Header Completion**: 50+ header suggestions triggered after `#include <`
- **Run Python**: Execute `.py` scripts from editor with configurable interpreter path
- **Run C++**: Compile with `g++ -std=c++17 -O2` and run, configurable compiler path
- **Auto Indent**: Enter preserves indentation, `{` generates formatted block, cursor in `{}` pushes `}` down
- **Terminal-style Output**: Unified dialog for Python/C++ output with dark/light toggle and Copy button
- **Customizable Toolbar**: Drag-to-reorder, View menu toggles for each button, Reset Default Layout
- **Sync Boundaries**: Auto-reloads modified files in open editor tabs
- **Lazy Case Loading**: Time directories and subdirectories load on expand for instant case opening
- **Instant File Operations**: New File/Folder/Delete no longer trigger full tree rebuild
- **ParaView**: Improved not-found dialog with download URL, launch status feedback
- **Language-specific Completion**: Strict separation — Python files only get Python keywords, etc.
- **Custom Toolbar Icons**: Painted icons for Terminal, Python, C++, Sync Boundaries, ParaView

### Performance
- Case tree lazy-loading for large cases
- Instant new file/folder/delete operations
- Optimized header includes across all source files

### Documentation
- English and Chinese README
- Read the Docs integration with MkDocs
- Full feature documentation, usage guide, build instructions

## Version 1.0

### Initial Release
- Case Browser with tree view of 0/, constant/, system/
- Code Editor with syntax highlighting and line numbers
- Boundary Conditions Panel with RTM parameter tables
- Turbulence Model Panel
- Schemes & Solvers Panel
- snappyHexMesh Panel
- General Dictionary Panel
- Sync Boundaries
- File Viewer (images, PDF, Office)
- ParaView Integration
- Terminal launcher
- Recent Cases
