# OpenFOAM GUI

基于 Qt 6.10.2 构建的 OpenFOAM CFD 算例管理器与编辑器。

浏览、编辑和保存 OpenFOAM 算例文件，支持语法高亮、算例结构感知和集成化配置面板。

---

## 功能概览

| 模块 | 说明 |
|------|------|
| **算例浏览器** | 树形展示 `0/`、`constant/`、`system/` 目录；支持多算例 |
| **代码编辑器** | 多标签页编辑器，行号、语法高亮、标签页右键菜单 |
| **语法高亮** | OpenFOAM 字典语法：关键字、标量、矢量、宏、量纲 |
| **文件类型检测** | 自动识别 OpenFOAM 场文件、字典文件 |
| **边界条件面板** | RTM 参数表可视化编辑、Patch 浏览、智能推荐 |
| **湍流模型面板** | RAS/LES 模型选择，模型参数表单 |
| **离散格式与求解器面板** | 结构化编辑 `fvSchemes` 和 `fvSolution` |
| **snappyHexMesh 面板** | 完整 snappyHexMeshDict 配置 |
| **通用字典面板** | 编辑 blockMeshDict、controlDict、decomposeParDict 等 |
| **Sync Boundaries** | 一键将 blockMeshDict 边界名称同步到 0/ 文件夹所有场文件 |
| **File Viewer** | PNG/JPG/SVG/EPS 图片原生查看（含缩放）；PDF/Office 调用系统默认程序 |
| **ParaView 集成** | 菜单一键启动 ParaView 后处理 |
| **终端** | 算例目录打开系统终端 |
| **最近算例** | 快速访问最近算例（最多 10 个） |

---

## 边界条件面板

BC 面板匹配 OpenFOAM 官方文档格式（RTM — Required / Type / Mandatory）。

### 支持场类型

| 场类型 | FoamFile Class | 示例 |
|--------|---------------|------|
| **标量** | `volScalarField` | `p`, `p_rgh`, `k`, `epsilon`, `omega`, `nut`, `T`, `alpha.*` |
| **矢量** | `volVectorField` | `U`, `v` |

### 分类筛选

| 标签 | 示例类型 |
|------|---------|
| **All** | 当前场所有类型 |
| **Basic** | `fixedValue`, `zeroGradient`, `fixedGradient`, `mixed`, `calculated` |
| **Wall** | `noSlip`, `slip`, `kqRWallFunction`, `epsilonWallFunction`, `omegaWallFunction` |
| **Inlet** | `flowRateInletVelocity`, `pressureInletOutletVelocity`, `totalPressure` |
| **Outlet** | `inletOutlet`, `advective`, `waveTransmissive`, `freestream` |
| **Pressure** | `totalPressure`, `fixedFluxPressure`, `fixedMean`, `fanPressure` |
| **Mapped** | `mappedFixedValue`, `timeVaryingMappedFixedValue`, `mappedMixed` |
| **Constraint** | `empty`, `symmetry`, `wedge`, `cyclic`, `cyclicAMI`, `processor` |
| **Coded** | `codedFixedValue`, `codedMixed` |

### 功能

- **RTM 参数表** — Property | Description | Type | Required | Default，默认值可编辑
- **Patch 浏览器** — 颜色编码 patch 类型，点击自动选中 BC
- **智能推荐** — 根据场名 + patch 名启发式推荐最合适的 BC
- **代码预览** — 暗色主题预览面板
- **Apply to Editor** — 在光标位置插入 BC 代码片段
- **100+ BC 类型** 对照 OpenFOAM v2206 源码 `.H` 文件验证

---

## Sync Boundaries

`Case → Sync Boundaries` 一键将 `system/blockMeshDict` 的边界 patch 名称同步到 `0/`（或 `0.orig/`）时间目录下的所有场文件。

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

## File Viewer（文件查看器）

| 类别 | 扩展名 | 查看方式 |
|------|--------|---------|
| **光栅图片** | PNG, JPG, BMP, GIF, WebP, ICO | 原生查看 + 缩放控件（− / + / Fit / 1:1） |
| **矢量 — SVG** | SVG | QSvgWidget 渲染 |
| **矢量 — EPS** | EPS, EPSF, PS | Ghostscript → PNG（自动检测 PATH、TeX Live 中的 GS） |
| **PDF** | PDF | 系统默认 PDF 阅读器 |
| **Office** | DOC, DOCX, XLS, XLSX, PPT, PPTX | 系统默认程序 |

### 其他程序打开

在**算例浏览器**中右键任意文件，选择 **Open With...** 可选择外部程序打开。

---

## 编辑器与算例浏览器

### 代码编辑器
- **语法高亮** — OpenFOAM/C++ 关键字、标量、矢量、宏
- **行号** — 当前行高亮
- **多标签页** — `Ctrl+W` 关闭，`Ctrl+Tab` 切换，右键菜单（关闭当前/其他/全部）
- **未保存更改** — `*` 标记 + 关闭确认
- **注释/取消注释** — `Ctrl+/`
- **查找** — `Ctrl+F`

### 算例浏览器
- 树形结构：时间目录 → `constant/` → `system/`
- 文件类型感知图标、过滤器
- 右键菜单：打开文件、**Open With...**、关闭算例、刷新、新建、删除

---

## ParaView 集成

1. **Case → ParaView** 启动 ParaView 加载当前算例
2. 自动在算例根目录创建 `.foam` 文件
3. 自动检测 ParaView 路径；可手动配置：**Case → ParaView Path...**

下载：[https://www.paraview.org/download/](https://www.paraview.org/download/)

---

## 环境要求

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0+)
- **OpenFOAM** (v2012 ~ v2512)
- **ParaView** 5.10+（可选）
- **Ghostscript**（可选，用于 EPS 渲染；TeX Live 自带）

## 编译

```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# 输出：release/OpenFOAMGUI.exe
```

**.pro 配置**：
- Qt 模块：Core, GUI, Widgets, SvgWidgets
- C++17，GNU 扩展
- 链接：Qt6SvgWidgets, Qt6Svg, Qt6Widgets, Qt6Gui, Qt6Core

## 使用说明

1. 启动 `OpenFOAMGUI.exe`
2. **Case → Open Case** (`Ctrl+O`) 选择算例目录
3. 在**算例浏览器**中双击文件打开到编辑器
4. 右侧面板自动切换上下文：
   - 场文件 → **边界条件**
   - `turbulenceProperties` → **湍流模型**
   - `fvSchemes` / `fvSolution` → **离散格式与求解器**
   - `snappyHexMeshDict` → **snappyHexMesh**
   - `blockMeshDict`, `controlDict` → **通用字典**
5. **保存** (`Ctrl+S`) 写回算例

---

## 项目结构

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                       # 入口 + 插件路径设置
│   ├── mainwindow.h/.cpp              # 主窗口、菜单、工具栏、停靠面板
│   ├── casebrowser.h/.cpp             # 算例目录树浏览器
│   ├── codeeditor.h/.cpp              # 代码编辑器 + 行号
│   ├── fileviewer.h/.cpp              # 图片/EPS/PDF/Office 文件查看器
│   ├── ofhighlighter.h/.cpp           # OpenFOAM/C++ 语法高亮器
│   ├── ofparser.h/.cpp                # OpenFOAM 文件头/关键字解析
│   ├── ofmeshreader.h/.cpp            # OpenFOAM polyMesh 解析器
│   ├── languagedetector.h/.cpp        # 文件语言检测
│   ├── linenumberarea.h/.cpp          # 行号边栏
│   ├── bcpanel.h/.cpp                 # 边界条件面板
│   ├── bctypedatabase.h/.cpp          # 100+ BC 类型数据库
│   ├── turbulencepanel.h/.cpp         # 湍流模型面板
│   ├── turbulencemodeldatabase.h/.cpp # 湍流模型定义
│   ├── schemespanel.h/.cpp            # fvSchemes / fvSolution 面板
│   ├── snappypanel.h/.cpp             # snappyHexMeshDict 面板
│   ├── dictpanel.h/.cpp               # 通用字典面板
│   └── bychen.ico                     # 应用图标
├── OpenFOAMGUI.pro                    # Qt 项目文件
├── resources.qrc                      # Qt 资源文件
├── qt.conf                            # 插件路径配置
├── README.md                          # 英文文档
├── README_CN.md                       # 中文文档
└── CHANGELOG.md                       # 修改日记
```

## 许可证

本项目仅供 CFD 工作流程中的教育和研究用途。
