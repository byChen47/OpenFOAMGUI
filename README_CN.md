# OpenFOAM GUI v2.0.5

基于 Qt 6.10.2 的专业 CFD 算例管理器与编辑器。

📖 **在线文档**：[openfoamgui.readthedocs.io](https://openfoamgui.readthedocs.io/)

---

## 功能

| 模块 | 说明 |
|------|------|
| **算例浏览器** | 延迟加载树形视图，多算例，过滤，重命名，文件监视自动刷新 |
| **代码编辑器** | 多标签页，语法高亮，行号，三类自动补全，自动缩进，括号匹配 |
| **C++ 补全** | 200+ STL 关键词 |
| **Python 补全** | Python 3 内置函数 |
| **OpenFOAM 补全** | 500+ 关键词：120+ 求解器、BC 类型、Shell 脚本（Allrun/Allclean） |
| **#include 头文件** | 输入 `#include <` 触发 50+ 头文件建议 |
| **边界条件** | RTM 参数表、120+ BC 类型、Patch 浏览器、智能推荐 |
| **湍流模型** | 20 种 RAS/LES/DES 模型，含系数表 + 入口参数计算器 |
| **BL 计算器 (Y+)** | y⁺ → 第一层网格厚度，含完整推导面板，自动同步 snappyHexMesh addLayersControls |
| **湍流计算器** | 输入 U/I/L/ν → 计算 k/ε/ω/ν_t/ν̃，覆盖全部 OF 湍流模型族 |
| **格式与求解器** | 结构化编辑 `fvSchemes` 和 `fvSolution` |
| **snappyHexMesh** | 完整 `snappyHexMeshDict` 面板 + Y⁺ → addLayersControls 同步 |
| **字典面板** | 20+ 字典类型：`blockMeshDict`、`waveProperties`（7 种波浪模型）等 |
| **拖拽调整** | 拖拽分割条自由调整面板区域大小 |
| **Sync Boundaries** | 一键同步 `blockMeshDict` 边界 → 全部 `0/` 场文件 |
| **文件查看器** | PNG/JPG/SVG/EPS（缩放）；PDF/Office 系统默认 |
| **Run Python** | 编辑器运行 `.py`，可配置解释器路径 |
| **Run C++** | `g++ -std=c++17 -O2` 编译运行，可配置编译器路径 |
| **ParaView** | 一键启动，自动 `.foam` 文件 |
| **终端** | 算例目录打开系统终端 |
| **暗色主题** | Ctrl+T 切换，Fusion 深色调色板，可持久化 |
| **查找/替换** | 行内查找 (Ctrl+F)、F3 查找下一个、替换 (Ctrl+H) |
| **工具栏** | 拖拽排序，View → Toolbar Buttons 开关，恢复默认布局 |

---

## 快捷键

| 按键 | 操作 |
|------|------|
| `Ctrl+O` | 打开算例 |
| `Ctrl+S` | 保存 |
| `Ctrl+W` | 关闭标签 |
| `Ctrl+F` | 查找 |
| `Ctrl+H` | 替换 |
| `F3` | 查找下一个 |
| `Ctrl+/` | 注释切换 |
| `Ctrl+T` | 暗色/亮色主题 |
| `Ctrl+Z` / `Ctrl+Y` | 撤销/重做 |
| `Ctrl+B` | BC 面板 |
| `Ctrl+Shift+P` | 运行 Python |
| `Ctrl+Shift+C` | 运行 C++ |
| `Ctrl+N` | 新建文件 |

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
# 输出：release/OpenFOAMGUI.exe（无控制台窗口）
```

**Qt 模块**：Core, GUI, Widgets, Svg, SvgWidgets · **标准**：C++17

---

## 项目结构

```
OpenFOAMGUI/
├── .readthedocs.yaml          # Read the Docs 配置
├── mkdocs.yml                 # MkDocs 文档配置
├── docs/                      # 在线文档源文件
├── src/                       # 源代码（17 对源文件）
│   ├── main.cpp, mainwindow.* # 入口 + 主窗口
│   ├── casebrowser.*          # 算例树（延迟加载，重命名，文件监视）
│   ├── codeeditor.*           # 编辑器 + 500+ 关键词自动补全
│   ├── fileviewer.*           # 图片/EPS/PDF/Office 查看器
│   ├── ofhighlighter.*        # OpenFOAM/C++ 语法高亮
│   ├── ofparser.*             # OpenFOAM 文件解析
│   ├── ofmeshreader.*         # OpenFOAM polyMesh 读取
│   ├── languagedetector.*     # 语言检测
│   ├── linenumberarea.*       # 行号边栏
│   ├── bcpanel.* + bctypedatabase.*  # BC 面板 + 120+ BC 类型
│   ├── turbulencepanel.* + turbulencemodeldatabase.*  # 湍流模型 + 入口计算器
│   ├── schemespanel.*         # fvSchemes / fvSolution
│   ├── snappypanel.*          # snappyHexMeshDict + Y⁺ 计算器
│   ├── dictpanel.*            # 20+ 字典类型（waveProperties 等）
│   └── bychen.ico             # 应用图标
├── tests/                     # 单元测试（19 项，ofparser + 语言检测）
├── .github/workflows/         # CI：Windows MinGW + Linux GCC
├── ROADMAP.md                 # 开发路线图
├── UI_REFACTOR_PLAN.md        # UI 重构方案
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
