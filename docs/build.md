# Build Instructions

## Requirements

| Tool | Version | Notes |
|------|---------|-------|
| **Qt** | 6.10.2 | MinGW 64-bit |
| **MinGW-w64** | GCC 15.2.0+ | |
| **Git** | 2.x | Optional |

## Project Configuration

The project uses qmake with `OpenFOAMGUI.pro`:

```makefile
QT       += core gui widgets svgwidgets
CONFIG   += c++17 console

mingw {
    QMAKE_LFLAGS_CONSOLE = -Wl,-subsystem,windows -mthreads
}

TARGET   = OpenFOAMGUI
TEMPLATE = app
```

- **Qt modules**: Core, GUI, Widgets, SvgWidgets
- **C++ standard**: C++17 with GNU extensions (`-std=gnu++1z`)
- **Linker**: `-Wl,-subsystem,windows` suppresses console window

## Command-Line Build (Windows)

```bash
# Set up environment
export PATH="D:/Qt/6.10.2/mingw_64/bin:$PATH"
export PATH="D:/mingw64/bin:$PATH"

# Navigate to project
cd OpenFOAMGUI

# Generate Makefile
qmake OpenFOAMGUI.pro

# Build (Release)
mingw32-make -f Makefile.Release

# Output: release/OpenFOAMGUI.exe
```

### Build Targets

```bash
mingw32-make -f Makefile.Release       # Release build (-O2)
mingw32-make -f Makefile.Debug         # Debug build (-g)
mingw32-make -f Makefile.Release clean # Clean artifacts
```

### Compiler Flags (Release)

```
-O2 -std=gnu++1z -Wall -Wextra -fexceptions -mthreads
-DUNICODE -D_UNICODE -DWIN32 -DMINGW_HAS_SECURE_API=1
```

### Linker

```
-Wl,-s -Wl,-subsystem,windows -mthreads
```

### Linked Libraries

```
libQt6SvgWidgets.a  libQt6Svg.a  libQt6Widgets.a  libQt6Gui.a  libQt6Core.a
```

## Qt Creator Build

1. Open **Qt Creator**
2. **File → Open File or Project...** → select `OpenFOAMGUI.pro`
3. Select the **MinGW 64-bit** kit (Qt 6.10.2)
4. Choose **Release** configuration
5. **Build → Build Project** (`Ctrl+B`)

## Deployment

For standalone distribution, use `windeployqt`:

```bash
windeployqt --release --no-translations release/OpenFOAMGUI.exe
```

This copies all required Qt DLLs and plugins. The `qt.conf` file ensures plugins are found:

```ini
[Paths]
Plugins = .
```
