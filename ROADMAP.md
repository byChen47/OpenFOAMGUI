# OpenFOAM GUI — 后续提升计划 (Roadmap)

基于对完整代码库的审查，从 Qt 开发者和 CFD 工程师双重视角制定。

---

## Phase 1 — 紧急修复（稳定性 + 内存泄漏）✅ 已完成

| # | 问题 | 文件 | 状态 |
|---|------|------|------|
| 1.1 | **内存泄漏** — `keyPressEvent()` 每次按键 `new QStringListModel`，从未释放。改为持久 `m_completionModel` 仅调 `setStringList()` | `codeeditor.h/cpp` | ✅ |
| 1.2 | **版本号不一致** — `main.cpp:10` `setApplicationVersion("1.0.0")` vs About 对话框 v2.0.3 | `main.cpp` | ✅ |
| 1.3 | **closeEvent 半保存** — 关闭时逐标签确认，Cancel 时已保存的标签不撤销。改为先收集全部未保存标签，确认后再批量保存 | `mainwindow.cpp` | ✅ |

---

## Phase 2 — 代码质量（Qt 开发者视角）✅ 已完成

| # | 问题 | 文件 | 状态 |
|---|------|------|------|
| 2.1 | **代码重复** — `openFileInTab()` 和 `onTabChanged()` 中文件→面板路由 100% 重复（~50 行）。抽取为 `routeFileToPanel()`，改用 `QSet<QString>` + 顺便修复 onTabChanged 缺少 `waveProperties`/`waveProperties.input` 的 bug | `mainwindow.h/cpp` | ✅ |
| 2.2 | **硬编码路径** — `D:/3.Wpsandother/mingw64/bin`（开发者个人路径）等。移除个人路径，Python/g++ 自动检测仅保留通用路径 | `mainwindow.cpp` | ✅ |
| 2.3 | **编译配置矛盾** — `CONFIG += console` 与 mingw block `-Wl,-subsystem,windows` 冲突。清理 PRO 文件，移除冗余行 | `OpenFOAMGUI.pro` | ✅ |
| 2.4 | **High-DPI 支持** — 缺少高分辨率屏幕适配。添加 `PassThrough` 缩放策略 + Qt5 兼容 `AA_EnableHighDpiScaling` | `main.cpp` | ✅ |
| 2.5 | **Widget 泄漏** — `onCloseTab` 调用 `removeTab()` 但不 delete widget。添加 `widget->deleteLater()`，信号自动断开 | `mainwindow.cpp` | ✅ |
| 2.6 | **括号匹配** — 缺少 `{}()[]` 对高亮。在 `highlightCurrentLine()` 增加嵌套深度跟踪，蓝色高亮匹配对 | `codeeditor.cpp` | ✅ |
| 2.7 | **Find/Replace + F3** — 仅有 Find（QInputDialog），无 Replace、无 F3。新增 `onFindNext` (F3)、`onReplaceText` (Ctrl+H)、Replace/Replace All 对话框 | `mainwindow.h/cpp` | ✅ |

---

## Phase 3 — CFD 功能增强（CFD 工程师视角）✅ 已完成

| # | 问题 | 文件 | 状态 |
|---|------|------|------|
| 3.1 | **File Watcher** — 求解器运行时产生新时间目录，用户必须手动 F5。添加 `QFileSystemWatcher` + `m_knownDirs` 智能检测，仅新目录触发增量刷新 | `casebrowser.h/cpp` | ✅ |
| 3.2 | **`readField()` 未实现** — 头文件声明但 cpp 未定义（会导致链接错误）。实现 uniform/nonuniform/vector 三种格式解析，新增 `OFMeshData::cellValues` + `fieldName` | `ofmeshreader.h/cpp` | ✅ |
| 3.3 | **湍流模型扩展** — 仅覆盖 kEpsilon/realizableKE/RNGkEpsilon 等。新增 **kOmega2006**、**v2f**（Durbin v²-f 4 方程）、**SpalartAllmarasDES**（CDES=0.65）、**kOmegaSSTDES**（DDES/FSST），含公式推导和参考文献 | `turbulencemodeldatabase.cpp` | ✅ |
| 3.4 | **BC 去重** — `pressureInletOutletParSlipVelocity` 重复定义两处，参数不一致。删除不完整的第一条 | `bctypedatabase.cpp` | ✅ |
| 3.5 | **Allrun/Allclean 生成** — 已尝试但移除（与 watcher 冲突导致树刷新问题） | — | ❌ 已移除 |

---

## Phase 4 — 工程基础设施

| # | 内容 | 优先级 |
|---|------|--------|
| 4.1 | **测试框架** — 零测试。从 `ofparser` 单元测试（header 解析）、`OFMeshReader` 单元测试（polyMesh 读取）、`LanguageDetector` 单元测试、Boundary sync 回归测试开始 | 高 |
| 4.2 | **CI/CD** — `.github/workflows/build.yml`，Windows MinGW + Linux GCC 双平台编译验证 | 中 |
| 4.3 | **代码格式化** — `.clang-format` + `.editorconfig` 统一缩进风格 | 中 |
| 4.4 | **跨平台适配** — 文件查看器 `findGhostscript()` 仅 Windows、终端启动硬编码 `cmd.exe`、ParaView 路径针对 Windows 注册表 | 中 |

---

## Phase 5 — 用户体验

| # | 内容 | 优先级 |
|---|------|--------|
| 5.1 | **暗色主题** — 当前仅终端输出对话框有暗色切换，扩展为全局主题 | 低 |
| 5.2 | **最近文件历史** — 仅记录最近 case 目录，缺少最近打开的具体文件 | 低 |
| 5.3 | **输出面板** — Run Python/C++ 每次通过菜单触发，增加类似 VS Code Terminal 的运行历史和状态面板 | 中 |
| 5.4 | **一键并行工作流** — `decomposeParDict` 已有配置，缺少 "Decompose → Run → Reconstruct" 一键流程 | 中 |
| 5.5 | **postProcessing 可视化** — `sampleDict`/`probes`/`forces` 输出缺少内联图表（曲线图） | 低 |

---

## 额外 — 功能缺口（代码审查发现，未排期）

| # | 问题 | 文件 | 严重程度 |
|---|------|------|----------|
| E.1 | `readPoints()` 使用 `toFloat()` 解析坐标，OpenFOAM 内部用 double，大坐标会丢失精度 | `ofmeshreader.cpp:45-47` | 中 |
| E.2 | 不支持 binary polyMesh 格式 | `ofmeshreader.cpp` | 低 |
| E.3 | `isTimeDirectory()` 使用 `toDouble()`，locale-dependent（逗号小数点的系统会失败） | `casebrowser.cpp:256` | 中 |
| E.4 | `caseForFile()` 简单前缀匹配，嵌套 case（`damBreak` / `damBreak/fine`）会错误匹配父 case | `casebrowser.cpp` | 中 |
| E.5 | `bcpanel.cpp:514-549` 使用贪婪正则解析 `boundaryField`，嵌套花括号（如 `codedFixedValue`）会失败 | `bcpanel.cpp` | 高 |
| E.6 | `ofhighlighter.cpp:162` 数字正则末尾多一个 `)`，可能是语法错误 | `ofhighlighter.cpp` | 中 |
| E.7 | `schemespanel.cpp` `solvers` 块硬编码 `"(k\|epsilon\|omega).*"`，不覆盖 `nut`/`nuTilda`/`T` | `schemespanel.cpp:467` | 中 |
| E.8 | `turbulencepanel.cpp:275-286` 只提取 `simulationType` 和模型名，不解析现有系数，面板无法显示当前配置 | `turbulencepanel.cpp` | 中 |
| E.9 | 缺少 `fvModels` / `fvConstraints` / `chemistryProperties` / `phaseProperties` 配置面板 | `dictpanel.cpp` | 低 |
| E.10 | 无 Fortran 文件检测（`.f90`/`.f95`/`.f`），自定义 BC 会被分类为 Unknown | `languagedetector.cpp` | 低 |
| E.11 | `dictpanel.cpp` 1588 行，数据初始化函数应拆分到独立文件 | `dictpanel.cpp` | 低 |
