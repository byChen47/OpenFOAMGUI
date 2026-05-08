# OpenFOAM GUI

基于 Qt 6.10.2 的 OpenFOAM CFD 算例管理器与编辑器。

支持语法高亮、自动补全、算例结构感知和集成化配置面板。

---

## 功能

| 模块 | 说明 |
|------|------|
| **算例浏览器** | 树形浏览 `0/`、`constant/`、`system/`，大算例延迟加载 |
| **代码编辑器** | 多标签页、语法高亮、行号、自动补全、自动缩进 |
| **自动补全** | C++（200+ STL）、Python、OpenFOAM（250+ BC/格式）— 可独立开关 |
| **边界条件** | RTM 参数表、Patch 浏览器、100+ BC 类型 |
| **湍流模型** | RAS/LES 模型选择与参数配置 |
| **离散格式与求解器** | 结构化编辑 `fvSchemes` 和 `fvSolution` |
| **snappyHexMesh** | 完整 snappyHexMeshDict 配置 |
| **通用字典** | blockMeshDict、controlDict、decomposeParDict 等 |
| **Sync Boundaries** | 一键同步 blockMeshDict 边界到所有 0/ 场文件 |
| **文件查看器** | PNG/JPG/SVG/EPS 图片缩放查看；PDF/Office 调用系统默认程序 |
| **Run Python** | 编辑器运行 Python 脚本，可配置解释器路径 |
| **ParaView** | 一键启动、自动检测、下载引导 |
| **终端** | 算例目录打开系统终端 |
| **自定义工具栏** | 拖拽排序、View 菜单开关、一键恢复默认布局 |

---

## 快速开始

1. 启动 `OpenFOAMGUI.exe`
2. **Case → Open Case** (`Ctrl+O`) 选择算例
3. 双击 Case Browser 中的文件进行编辑
4. 右侧面板自动切换上下文（BC/湍流/格式/snappyHexMesh/字典）
5. **View → 3D Mesh Viewer**（底部 Dock）查看网格几何

---

## 编辑器功能

| 快捷键 | 操作 |
|--------|------|
| `Ctrl+O` | 打开算例 |
| `Ctrl+S` | 保存 |
| `Ctrl+W` | 关闭标签 |
| `Ctrl+F` | 查找 |
| `Ctrl+/` | 注释/取消注释 |
| `Ctrl+Z` / `Ctrl+Y` | 撤销 / 重做 |
| `Ctrl+Shift+P` | 运行 Python |
| `Ctrl+B` | 切换 BC 面板 |

**自动补全** — Edit 菜单独立控制 C++、Python、OpenFOAM 三类补全。
输入 `#include <` 触发头文件建议。

**自动缩进** — Enter 保持当前缩进。空行输入 `{` 自动生成格式化代码块。
光标在 `{}` 内按 Enter 时 `}` 推到下一行。

---

## Sync Boundaries

`View → Sync Boundaries` 将 `blockMeshDict` 的边界名称同步到 `0/`（或 `0.orig/`）的所有场文件。已打开编辑器标签页中的文件自动刷新。

### 默认 BC 分配

| Patch 类型 | 场 | 默认 BC |
|-----------|-----|---------|
| `wall` | U, v | `noSlip` |
| `wall` | p, p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut | `nutkWallFunction` |
| `empty` | 任意 | `empty` |
| `symmetry` | 任意 | `symmetry` |
| `wedge` | 任意 | `wedge` |
| `cyclic` | 任意 | `cyclic` |

---

## 文件查看器

| 类别 | 扩展名 | 查看方式 |
|------|--------|---------|
| 光栅图片 | PNG, JPG, BMP, GIF, WebP, ICO | 原生查看 + 缩放 (− / + / Fit / 1:1) |
| SVG | SVG | QSvgWidget 渲染 |
| EPS | EPS, EPSF, PS | Ghostscript → PNG（自动检测） |
| PDF | PDF | 系统默认 |
| Office | DOC, DOCX, XLS, XLSX, PPT, PPTX | 系统默认 |

---

## ParaView 集成

- **Case → ParaView** 启动 ParaView，自动创建 `.foam` 文件
- 自动检测 ParaView 路径；可手动配置 **Case → ParaView Path...**
- 未安装时弹窗提示下载地址

---

## 工具栏自定义

- 拖拽按钮重新排序
- **View 菜单**控制各按钮显示/隐藏
- **View → Reset Default Layout** 恢复默认排列

---

## 环境要求

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0+)
- **OpenFOAM** (v2012 ~ v2512)
- **ParaView** 5.10+（可选）
- **Ghostscript**（可选，用于 EPS；TeX Live 自带）

## 编译

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# 输出：release/OpenFOAMGUI.exe
```

**Qt 模块**：Core, GUI, Widgets, SvgWidgets
**C++17** with `-std=gnu++1z`

---

## 项目结构

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                       # 入口
│   ├── mainwindow.h/.cpp              # 主窗口、菜单、工具栏、Dock
│   ├── casebrowser.h/.cpp             # 算例目录树浏览器
│   ├── codeeditor.h/.cpp              # 编辑器 + 自动补全
│   ├── fileviewer.h/.cpp              # 图片/EPS/PDF/Office 查看器
│   ├── ofhighlighter.h/.cpp           # 语法高亮器
│   ├── ofparser.h/.cpp                # 文件解析器
│   ├── ofmeshreader.h/.cpp            # OpenFOAM polyMesh 读取器
│   ├── languagedetector.h/.cpp        # 语言检测
│   ├── linenumberarea.h/.cpp          # 行号边栏
│   ├── bcpanel.h/.cpp                 # 边界条件面板
│   ├── bctypedatabase.h/.cpp          # 100+ BC 类型数据库
│   ├── turbulencepanel.h/.cpp         # 湍流模型面板
│   ├── turbulencemodeldatabase.h/.cpp # 湍流模型定义
│   ├── schemespanel.h/.cpp            # 离散格式面板
│   ├── snappypanel.h/.cpp             # snappyHexMesh 面板
│   ├── dictpanel.h/.cpp               # 通用字典面板
│   └── bychen.ico                     # 应用图标
├── OpenFOAMGUI.pro                    # Qt 项目文件
├── resources.qrc                      # Qt 资源
├── qt.conf                            # 插件路径
├── README.md
├── README_CN.md
└── CHANGELOG.md
```

## 许可证

本项目仅供 CFD 工作流程中的教育和研究用途。
