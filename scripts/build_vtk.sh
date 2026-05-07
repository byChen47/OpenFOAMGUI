#!/bin/bash
# ============================================================
#  Build VTK 9.4 for MinGW64 / Qt 6.10
#  Run this script once to install VTK for 3D visualization.
#  After building, re-run qmake && make to link VTK.
# ============================================================
set -e

VTK_VER="9.4.0"
VTK_DIR="$HOME/vtk"
MINGW_DIR="D:/3.Wpsandother/mingw64"
QT_DIR="D:/3.Wpsandother/Qt/setting/6.10.2/mingw_64"

echo "=== Downloading VTK ${VTK_VER} ==="
if [ ! -f "/tmp/vtk-${VTK_VER}.tar.gz" ]; then
    curl -L --retry 5 --retry-delay 10 \
        -o "/tmp/vtk-${VTK_VER}.tar.gz" \
        "https://www.vtk.org/files/release/9.4/VTK-${VTK_VER}.tar.gz"
fi

echo "=== Extracting VTK ==="
rm -rf "${VTK_DIR}"
mkdir -p "${VTK_DIR}/src" "${VTK_DIR}/build"
tar -xzf "/tmp/vtk-${VTK_VER}.tar.gz" -C "${VTK_DIR}/src" --strip-components=1

echo "=== Configuring VTK (MinGW / Qt6) ==="
export PATH="${MINGW_DIR}/bin:${QT_DIR}/bin:$PATH"

cd "${VTK_DIR}/build"
cmake "${VTK_DIR}/src" \
    -G "MinGW Makefiles" \
    -DCMAKE_C_COMPILER="${MINGW_DIR}/bin/gcc.exe" \
    -DCMAKE_CXX_COMPILER="${MINGW_DIR}/bin/g++.exe" \
    -DCMAKE_BUILD_TYPE=Release \
    -DVTK_GROUP_ENABLE_Rendering=YES \
    -DVTK_GROUP_ENABLE_StandAlone=NO \
    -DVTK_GROUP_ENABLE_Web=NO \
    -DVTK_GROUP_ENABLE_Imaging=NO \
    -DVTK_GROUP_ENABLE_Views=NO \
    -DVTK_MODULE_ENABLE_VTK_GUISupportQt=YES \
    -DVTK_MODULE_ENABLE_VTK_RenderingQt=YES \
    -DVTK_MODULE_ENABLE_VTK_IOGeometry=YES \
    -DVTK_MODULE_ENABLE_VTK_IOParallel=NO \
    -DVTK_MODULE_ENABLE_VTK_IOImage=NO \
    -DVTK_MODULE_ENABLE_VTK_InteractionStyle=YES \
    -DVTK_MODULE_ENABLE_VTK_RenderingFreeType=NO \
    -DVTK_MODULE_ENABLE_VTK_FiltersSources=YES \
    -DVTK_MODULE_ENABLE_VTK_FiltersGeometry=YES \
    -DCMAKE_INSTALL_PREFIX="${VTK_DIR}/install" \
    -DBUILD_SHARED_LIBS=ON \
    -DVTK_QT_VERSION=6 \
    -DQt6_DIR="${QT_DIR}/lib/cmake/Qt6"

echo "=== Building VTK (may take 30-60 minutes) ==="
mingw32-make -j$(nproc)

echo "=== Installing VTK ==="
mingw32-make install

echo ""
echo "=== VTK build complete! ==="
echo "VTK installed to: ${VTK_DIR}/install"
echo ""
echo "To enable VTK in OpenFOAMGUI:"
echo "  1. Edit OpenFOAMGUI.pro, switch meshviewer_vtk.cp -> meshviewer.cpp"
echo "  2. Add VTK include/lib paths to the .pro file"
echo "  3. Rebuild: qmake && mingw32-make -f Makefile.Release"
