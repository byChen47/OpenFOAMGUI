# OpenFOAM GUI

基于 Qt 6.10.2 的专业 CFD 算例管理器与编辑器。

📖 **在线文档**：[openfoamgui.readthedocs.io](https://openfoamgui.readthedocs.io/)

---

## 功能

| 模块 | 说明 |
|------|------|
| **算例浏览器** | 延迟加载树形视图，多算例，过滤，多格式文件 |
| **代码编辑器** | 多标签页，语法高亮，行号，三类自动补全，自动缩进 |
| **C++ 补全** | 200+ STL 关键词：容器、算法、智能指针、流 |
| **Python 补全** | Python 3 内置函数、numpy/pandas |
| **OpenFOAM 补全** | 500+ 关键词：120+ 求解器、BC 类型、湍流模型、Shell 脚本 |
| **#include 头文件** | 输入 `#include <` 触发 50+ 头文件建议 |
| **边界条件** | RTM 参数表、120+ BC 类型、Patch 浏览器、智能推荐 |
| **湍流模型** | RAS/LES 模型配置，模型参数 |
| **格式与求解器** | 结构化编辑 `fvSchemes` 和 `fvSolution` |
| **snappyHexMesh** | 完整 `snappyHexMeshDict` 配置面板 |
| **字典面板** | 22+ 字典类型：`blockMeshDict`、`controlDict`、`waveProperties`（7 种波浪模型）等 |
| **拖拽调整** | 拖拽分割条自由调整面板区域大小（参数表/预览/截面列表） |
| **Sync Boundaries** | 一键同步 `blockMeshDict` 边界 → 全部 `0/` 场文件 |
| **文件查看器** | PNG/JPG/SVG/EPS（缩放）；PDF/Office 系统默认 |
| **Run Python** | 编辑器运行 `.py`，可配置解释器路径 |
| **Run C++** | `g++ -std=c++17 -O2` 编译运行，可配置编译器路径 |
| **ParaView** | 一键启动，自动 `.foam`，路径配置，下载引导 |
| **终端** | 算例目录打开系统终端 |
| **工具栏** | 拖拽排序，View 菜单开关，恢复默认布局 |

---

## 快捷键

| 按键 | 操作 |
|------|------|
| `Ctrl+O` | 打开算例 |
| `Ctrl+S` | 保存 |
| `Ctrl+W` | 关闭标签 |
| `Ctrl+F` | 查找 |
| `Ctrl+/` | 注释切换 |
| `Ctrl+Z` / `Ctrl+Y` | 撤销/重做 |
| `Ctrl+B` | BC 面板 |
| `Ctrl+Shift+P` | 运行 Python |
| `Ctrl+Shift+C` | 运行 C++ |
| `Ctrl+N` | 新建文件 |
| `Ctrl+Shift+N`| 新建文件夹 |

---

## 编辑器功能

### 自动补全
三类独立系统（C++、Python、OpenFOAM），通过 **Edit** 菜单开关。输入 2+ 字符触发。OpenFOAM 补全含 120+ 求解器、Shell 脚本（Allrun/Allclean）及工具，覆盖 v2012–v2512 全版本。Bash 文件也触发 OF 补全，方便编辑运行脚本。

### 自动缩进
Enter 保持缩进。空行 `{` 生成代码块。`{}` 中 Enter 推下 `}`。

### #include 头文件
`#include <` 后触发 50+ C/C++/OpenFOAM 头文件建议。

---

## Sync Boundaries

`View → Sync Boundaries` 将 blockMesh 边界名同步到 `0/` 或 `0.orig/`。已打开编辑器标签自动刷新。

| Patch 类型 | 场 | 默认 BC |
|-----------|-----|---------|
| `wall` | U | `noSlip` |
| `wall` | p/p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut | `nutkWallFunction` |
| `empty`/`symmetry`/`wedge`/`cyclic` | 任意 | 同类型 |

---

## 文件查看器

| 格式 | 查看方式 |
|------|---------|
| PNG, JPG, BMP, GIF, WebP, ICO | 原生缩放 (− / + / Fit / 1:1) |
| SVG | QSvgWidget |
| EPS, EPSF, PS | Ghostscript → PNG（自动检测） |
| PDF, DOC, DOCX, XLS, XLSX | 系统默认 |

---

## 环境要求

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2+)
- **OpenFOAM** (v2012–v2512)
- **ParaView** 5.10+（可选）
- **Python 3.x**（可选，Run Python）
- **g++**（可选，Run C++；MinGW 自带）
- **Ghostscript**（可选，EPS；TeX Live 自带）

## 编译

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# 输出：release/OpenFOAMGUI.exe
```

**Qt 模块**：Core, GUI, Widgets, SvgWidgets · **标准**：C++17

---

## 项目结构

```
OpenFOAMGUI/
├── .readthedocs.yaml          # Read the Docs 配置
├── mkdocs.yml                 # MkDocs 文档配置
├── docs/                      # 在线文档源文件
├── src/                       # 源代码（19 对源文件）
│   ├── main.cpp, mainwindow.* # 入口 + 主窗口
│   ├── casebrowser.*          # 算例树（延迟加载，重命名）
│   ├── codeeditor.*           # 编辑器 + 500+ 关键词自动补全
│   ├── fileviewer.*           # 图片/EPS/PDF/Office 查看器
│   ├── ofhighlighter.*        # OpenFOAM/C++ 语法高亮
│   ├── ofparser.*             # OpenFOAM 文件解析
│   ├── ofmeshreader.*         # OpenFOAM polyMesh 读取
│   ├── languagedetector.*     # 语言检测
│   ├── linenumberarea.*       # 行号边栏
│   ├── bcpanel.* + bctypedatabase.*  # BC 面板 + 120+ BC 类型
│   ├── turbulencepanel.* + turbulencemodeldatabase.*
│   ├── schemespanel.*         # fvSchemes / fvSolution
│   ├── snappypanel.*          # snappyHexMeshDict
│   ├── dictpanel.*            # 22+ 字典类型（waveProperties 等）
│   └── bychen.ico             # 应用图标
├── CHANGELOG.md, README.md, README_CN.md
└── qt.conf, resources.qrc, OpenFOAMGUI.pro
```

## 链接

- **在线文档**：[openfoamgui.readthedocs.io](https://openfoamgui.readthedocs.io/)
- **GitHub**：[github.com/byChen47/OpenFOAMGUI](https://github.com/byChen47/OpenFOAMGUI)
- **ParaView**：[paraview.org/download](https://www.paraview.org/download/)
- **Python**：[python.org/downloads](https://www.python.org/downloads/)

## 许可证

CFD 工作流中的教育和研究用途。
