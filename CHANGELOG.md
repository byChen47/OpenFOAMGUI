# 修改日记 (Change Log)

## 2026-05-06

### 欢迎页面关闭修复

**问题**：启动后 Welcome 标签页无法关闭。`setTabsClosable(true)` 使 Welcome 标签页（QLabel）也出现关闭按钮，但点击关闭后立即重建，形成"弹回循环"。同时，许多编辑器操作在 Welcome 标签页激活时静默失败。

**修复**（`src/mainwindow.cpp`）：
- Welcome 标签页的关闭按钮正常显示，点击 × 或 Ctrl+W 可正常关闭
- 关闭 Welcome 后不再自动重建，标签区域为空
- 无编辑器时触发的操作（Save、Find、Undo、Redo、Comment）在状态栏给出提示消息
- `onCloseTab` 中去除 QLabel 拦截逻辑，去除 `count() == 0` 时自动重建逻辑

---

### 工具栏精简与菜单重组

**问题**：工具栏按钮过多，File 和 Case 菜单有重复功能项。

**修复**（`src/mainwindow.cpp`）：

| 操作 | 之前位置 | 之后位置 |
|------|---------|---------|
| Clean Time Dirs | Toolbar | 移除 |
| Undo / Redo | Toolbar | 保留在 Edit 菜单 |
| Find / Comment | Toolbar | 保留在 Edit 菜单 |
| Refresh Case | Toolbar | 移除 |
| Save All | Toolbar | Edit 菜单 |
| Open Case | File + Case | 仅 Case |
| Close Case | File + Case | 仅 Case |

**最终 Toolbar**：`Open Case · Close Case | Save | New File · New Folder · Delete | BC Panel · Terminal · ParaView`

**最终菜单结构**：
```
File             Edit            Case
────             ────            ────
Recent Cases >   Undo            Open Case       Ctrl+O
──────────────── Redo            Close Case      Ctrl+Shift+W
Save     Ctrl+S  ────────        ────────────
Save As          Find    Ctrl+F  New File        Ctrl+N
Save All         Comment Ctrl+/  New Folder      Ctrl+Shift+N
────────────────                  Delete          Del
Close Tab Ctrl+W                  Clean Time Dirs
────────────────                  Sync Boundaries
Exit     Ctrl+Q                   ────────────
                                  ParaView
                                  ParaView Path...
                                  ────────────
                                  Refresh Case    F5
```

---

### Close Case 功能修复（3 次迭代）

**问题 1**：工具栏和右键"Close Case"无法关闭算例。

**根因**：
- `onCloseCase` 用 `findChild<QTreeWidgetItem*>()` 定位树控件，不靠谱
- `closeCase` 仅搜索顶层节点，嵌套 subcase 项无法被找到

**修复 1**（`src/mainwindow.cpp`）：
- `onCloseCase` 改为直接使用 `m_caseBrowser->tree()`
- 遇到 subcase 时向父级追溯找到 caseroot 再关闭

**修复 2**（`src/casebrowser.cpp`）：
- `closeCase` 改为 `QTreeWidgetItemIterator` 递归搜索全树

**修复 3**（`src/casebrowser.cpp`）：
- 放弃 `findItems("", Qt::MatchContains, 0)` 空字符串 hack
- 放弃 `QTreeWidgetItemIterator`
- 实现纯递归函数 `findItemByPath()`，用 `std::function` 递归遍历，100% 保证找到目标节点
- subcase 项关闭时追溯到父级 caseroot，移除整个顶层算例节点

---

### 一键同步边界（Sync Boundaries）

**功能**：`Case → Sync Boundaries` 将 `blockMeshDict` 边界 patch 名称一键同步到 `0/` 文件夹所有场文件的 `boundaryField` 中。

**实现**（`src/mainwindow.cpp`）：
- 解析 `system/blockMeshDict`（或 `constant/polyMesh/blockMeshDict`）的 `boundary (...)` 块
- 使用**括号深度计数**（非正则）解析，正确处理嵌套括号
- 提取 patch 名称和类型（patch, wall, empty, symmetry 等）
- 定位第一个时间目录（`0/` 或 `0.orig/`）
- 遍历每个场文件，解析 `boundaryField { ... }` 块（同样深度计数）
- 对缺失的 patch 追加条目，分配智能默认 BC

**智能默认 BC 分配规则**：

| Patch 类型 | 场 | 默认 BC |
|-----------|-----|---------|
| wall | U, v | `noSlip` |
| wall | p, p_rgh | `fixedFluxPressure` |
| wall | k | `kqRWallFunction` |
| wall | epsilon | `epsilonWallFunction` |
| wall | omega | `omegaWallFunction` |
| wall | nut, alphat | `nutkWallFunction` |
| wall | T, 其他标量 | `zeroGradient` |
| empty | 任意 | `empty` |
| symmetry | 任意 | `symmetry` |
| wedge | 任意 | `wedge` |
| cyclic | 任意 | `cyclic` |
| patch | 任意 | `zeroGradient` |

**修复**（`src/mainwindow.cpp`）：
- `totalUpdated` 计数器修复：从永远为 0 改为正确返回 `syncBoundariesForCase` 的返回值
- `boundaryField` 解析改用深度计数替代贪婪正则
- patch 名正则增加负向先行断言 `{(?!\s*\|)` 避免误匹配 table 花括号

---

### 标签页改进

**问题 1**：打开文件后标签页标题带有 `[OpenFOAM]` 语言后缀，占用空间且不必要。

**修复**（`src/mainwindow.cpp`）：标签页标题仅显示文件名，移除 `langSuffix` 拼接。

**问题 2**：打开多个文件后只能逐个关闭，缺乏批量关闭能力。

**修复**（`src/mainwindow.cpp`）：
- 右键点击标签页弹出上下文菜单，提供三个选项：
  - **Close Current Tab**（关闭当前文件，Ctrl+W）
  - **Close Other Tabs**（关闭其他文件）
  - **Close All Tabs**（关闭全部文件）
- Welcome 标签页不弹出菜单
- 每个关闭操作触发未保存更改提示
- 从右向左关闭，避免索引偏移

---

### 文档更新

**README.md**：
- BC 面板的完整 RTM 参数表文档
- 100+ BC 类型数据库详细列表
- ParaView 集成说明（路径搜索顺序、配置方法、下载地址）
- 编译方法详细说明（前置条件、.pro 配置、命令行/Qt Creator 编译、编译器/链接器参数）
- Sync Boundaries 功能完整说明

**README_CN.md**（新增）：
- 英文 README 的完整中文翻译
- 全部功能表格和描述均已中文化

**CHANGELOG.md**（本文件，新增）：
- 记录项目所有修改日记

---

### 图片、PDF 和 Office 文档查看支持

**功能**：在算例浏览器中双击图片、PDF、Word、Excel 文件时，以适当的方式查看。

**文件类型处理策略**（`src/mainwindow.cpp` — `openFileInTab`）：

| 文件类型 | 扩展名 | 查看方式 |
|---------|--------|---------|
| **光栅图片** | PNG, JPG, JPEG, BMP, GIF, WebP, ICO | QScrollArea + QLabel(QPixmap) 原生显示 |
| **EPS 矢量图** | EPS, EPSF, PS | QPixmap → Ghostscript 渲染 → 系统默认程序（三级回退） |
| **矢量图片** | SVG | QSvgWidget 矢量渲染 |
| **PDF** | PDF | 自动用系统默认程序打开；标签页显示信息卡片 + 手动打开按钮 |
| **Word** | DOC, DOCX | 自动用系统默认程序打开 |
| **Excel** | XLS, XLSX | 自动用系统默认程序打开 |
| **PowerPoint** | PPT, PPTX | 自动用系统默认程序打开 |

**技术细节**：
- SVG 支持需要 Qt6SvgWidgets 模块（`QT += svgwidgets`）
- QtPdf 模块未安装，PDF 通过 `QDesktopServices::openUrl` 调用系统关联程序
- 图片标签页不触发编辑器操作（Save、Undo 等，`currentEditor()` 返回 nullptr）
- 关闭图片/文档标签页时不弹出未保存提示

**项目文件变更**：`OpenFOAMGUI.pro` 新增 `svgwidgets` 模块。

---

### EPS 图片查看支持

**功能**：支持 EPS（Encapsulated PostScript）矢量图的原生查看。

**三级渲染回退策略**（`src/mainwindow.cpp`）：

| 优先级 | 方法 | 说明 |
|--------|------|------|
| 1 | QPixmap 直接加载 | 部分平台/插件可原生读取 |
| 2 | Ghostscript 渲染 | 自动搜索 `gswin64c`/`gs`，将 EPS 转为临时 PNG |
| 3 | 系统默认程序 | 所有渲染器不可用时外部打开 |

Ghostscript 搜索路径包括 PATH、`C:/Program Files/gs/*/bin/gswin64c.exe` 等常见安装位置。

---

### 编译环境

| 工具 | 版本 | 路径 |
|------|------|------|
| Qt | 6.10.2 MinGW 64-bit | `D:/3.Wpsandother/Qt/setting/6.10.2/mingw_64/` |
| MinGW-w64 | GCC 15.2.0 | `D:/3.Wpsandother/mingw64/` |
| Git | 2.52.0 | `D:/3.Wpsandother/gitForWindow/setting/Git/` |

**编译命令**：
```bash
qmake OpenFOAMGUI.pro
mingw32-make -f Makefile.Release
# 输出：release/OpenFOAMGUI.exe
```
