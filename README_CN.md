# OpenFOAM GUI

基于 Qt 6.10.2 的专业 CFD 算例管理器与编辑器。

---

## 功能

| 模块 | 说明 |
|------|------|
| **算例浏览器** | 延迟加载树形视图，多算例、过滤、多格式文件支持 |
| **代码编辑器** | 多标签页、语法高亮、行号、三类自动补全、自动缩进 |
| **C++ 自动补全** | 200+ 关键词：STL 容器、算法、智能指针、流、C 标准库 |
| **Python 自动补全** | Python 3 关键词、内置函数、numpy/pandas 提示 |
| **OpenFOAM 自动补全** | 250+ 关键词：全部 BC 类型、格式、求解器、snappyHexMesh、湍流模型 |
| **#include 头文件** | 输入 `#include <` 触发 50+ 头文件建议 |
| **边界条件** | RTM 参数表、100+ BC 类型、Patch 浏览器、智能推荐 |
| **湍流模型** | RAS/LES 模型选择，模型参数配置 |
| **格式与求解器** | 结构化编辑 `fvSchemes` 和 `fvSolution` |
| **snappyHexMesh** | 完整 `snappyHexMeshDict` 配置面板 |
| **通用字典** | `blockMeshDict`、`controlDict`、`decomposeParDict` 等 |
| **Sync Boundaries** | 一键同步 `blockMeshDict` 边界 → 全部 `0/` 场文件 |
| **文件查看器** | PNG/JPG/SVG/EPS（缩放）；PDF/Office 调用系统默认 |
| **Run Python** | 编辑器运行 `.py` 脚本，可配置解释器路径 |
| **Run C++** | `g++ -std=c++17 -O2` 编译运行，可配置编译器路径 |
| **ParaView** | 一键启动、自动 `.foam`、路径配置、下载引导 |
| **终端** | 算例目录打开系统终端 |
| **自定义工具栏** | 拖拽排序、View 菜单开关、恢复默认布局 |

---

## 快捷键

| 按键 | 操作 |
|------|------|
| `Ctrl+O` | 打开算例 |
| `Ctrl+S` | 保存 |
| `Ctrl+W` | 关闭标签 |
| `Ctrl+F` | 查找 |
| `Ctrl+/` | 注释切换 |
| `Ctrl+Z/Y` | 撤销/重做 |
| `Ctrl+B` | BC 面板 |
| `Ctrl+Shift+P` | 运行 Python |
| `Ctrl+Shift+C` | 运行 C++ |

---

## 编辑器

**自动补全** — Edit 菜单独立控制 C++、Python、OpenFOAM 三类。输入 2+ 字符触发。

**自动缩进** — Enter 保持缩进。空行 `{` 生成格式化代码块。`{}` 中按 Enter 推下 `}`。

**#include 头文件** — `#include <` 后触发 50+ 头文件建议，输入过滤。

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
| PDF, DOC, DOCX, XLS, XLSX | 系统默认程序 |

---

## ParaView

- **Case → ParaView** 启动 ParaView，自动创建 `.foam` 文件
- 配置路径：**Case → ParaView Path...**
- 未安装时提示下载地址

---

## 环境要求

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2+)
- **OpenFOAM** (v2012–v2512)
- **ParaView** 5.10+（可选）
- **Python** 3.x（可选，Run Python）
- **g++**（可选，Run C++；MinGW 自带）

## 编译

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
```

**Qt 模块**：Core, GUI, Widgets, SvgWidgets
**标准**：C++17

---

## 项目结构

```
src/
├── main.cpp                  入口
├── mainwindow.h/.cpp         主窗口、菜单、工具栏、Dock
├── casebrowser.h/.cpp        算例树（延迟加载）
├── codeeditor.h/.cpp         编辑器 + 自动补全
├── fileviewer.h/.cpp         图片/EPS/PDF/Office 查看器
├── ofhighlighter.h/.cpp      OpenFOAM/C++ 语法高亮
├── ofparser.h/.cpp           OpenFOAM 文件解析
├── ofmeshreader.h/.cpp       OpenFOAM polyMesh 读取
├── languagedetector.h/.cpp   语言检测
├── linenumberarea.h/.cpp     行号边栏
├── bcpanel.h/.cpp            边界条件面板
├── bctypedatabase.h/.cpp     100+ BC 类型数据库
├── turbulencepanel.h/.cpp    湍流模型面板
├── turbulencemodeldatabase.h/.cpp
├── schemespanel.h/.cpp       fvSchemes/fvSolution 面板
├── snappypanel.h/.cpp        snappyHexMeshDict 面板
├── dictpanel.h/.cpp          通用字典面板
└── bychen.ico                应用图标
```

## 许可证

CFD 工作流中的教育和研究用途。
