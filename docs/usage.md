# Usage Guide

## Getting Started

1. Launch `OpenFOAMGUI.exe`
2. **Case → Open Case** (`Ctrl+O`) and select an OpenFOAM case directory
3. Double-click any file in the **Case Browser** dock to open it in the editor
4. The right-side panel automatically switches based on file type

### Opening Cases

- **File → Recent Cases** provides quick access to previously opened cases (up to 10)
- Cases with valid OpenFOAM structure are detected automatically
- Multiple cases can be opened simultaneously

## Editing Files

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+O` | Open Case |
| `Ctrl+S` | Save current file |
| `Ctrl+Shift+S` | Save all open files |
| `Ctrl+Shift+A` | Save As... |
| `Ctrl+W` | Close current tab |
| `Ctrl+Q` | Exit application |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl+F` | Find text |
| `Ctrl+/` | Toggle comment |
| `Ctrl+B` | Toggle BC Panel |
| `Ctrl+N` | New File |
| `Ctrl+Shift+N` | New Folder |
| `Delete` | Delete selected file/folder |
| `Ctrl+Shift+P` | Run Python |
| `Ctrl+Shift+C` | Run C++ |

### Auto Completion

1. Type 2+ characters to trigger suggestions
2. Press **Enter** or **Tab** to accept the highlighted completion
3. Press **Escape** to dismiss the popup
4. Use the **Edit menu** to toggle completion for C++, Python, or OpenFOAM independently

### Tab Management

- Right-click any tab for:
    - **Close Current Tab** — close this file
    - **Close Other Tabs** — close all except this one
    - **Close All Tabs** — close all open files

## Boundary Conditions

1. Open a field file (e.g., `0/U`, `0/p`)
2. The **Boundary Conditions** panel shows:
    - **Patch Browser** — all patches in the file with their current BC types
    - **BC Type List** — applicable BC types for this field, grouped by category
3. Click a patch → its current BC type is highlighted
4. Select a new BC type → see parameters and preview
5. Click **Apply to Editor** to insert the BC snippet
6. Or right-click a BC type for more insert options

## Sync Boundaries

After modifying `system/blockMeshDict`:

1. **View → Sync Boundaries**
2. All field files in `0/` or `0.orig/` are scanned
3. Missing boundary patches are added with appropriate default BCs
4. Already-open editor tabs are refreshed automatically

## Running Python Scripts

1. Open or create a `.py` file in the editor
2. **Case → Run Python** (`Ctrl+Shift+P`)
3. The script is saved and executed
4. Output appears in a terminal-style dialog with **Copy** and **Dark/Light** toggle

### Configuring Python Path

If Python is not auto-detected:

1. **Case → Python Path...**
2. Browse to `python.exe` (e.g., `C:/Python312/python.exe`)
3. The path is saved for future sessions

## Compiling C++ Programs

1. Open a `.cpp` file in the editor
2. **Case → Run C++** (`Ctrl+Shift+C`)
3. The file is compiled with `g++ -std=c++17 -O2`
4. Compilation errors are shown if any
5. On success, the compiled `.exe` runs automatically

### Configuring C++ Compiler Path

1. **Case → C++ Compiler Path...**
2. Browse to `g++.exe` (e.g., `D:/mingw64/bin/g++.exe`)
3. The path is saved for future sessions

## ParaView Post-Processing

1. Open a case
2. **Case → ParaView** creates a `.foam` file and launches ParaView
3. The `.foam` file is created in the case root directory

### Configuring ParaView Path

If ParaView is not auto-detected:

1. **Case → ParaView Path...**
2. Browse to `paraview.exe`

## Customizing the Toolbar

- Drag buttons to reorder them
- Use **View menu** checkboxes to show/hide individual buttons
- **View → Reset Default Layout** to restore defaults
