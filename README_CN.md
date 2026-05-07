# OpenFOAM GUI

基于 Qt 6.10.2 构建的 OpenFOAM CFD 算例管理器与编辑器。

浏览、编辑和保存 OpenFOAM 算例文件，支持语法高亮、算例结构感知和集成化配置面板 — 无需手动编辑字典文件。

---

## 目录

- [功能概览](#功能概览)
- [边界条件面板 (Boundary Conditions)](#边界条件面板-boundary-conditions)
- [湍流模型面板 (Turbulence Model)](#湍流模型面板-turbulence-model)
- [离散格式与求解器面板 (fvSchemes & fvSolution)](#离散格式与求解器面板-fvschemes--fvsolution)
- [snappyHexMesh 面板](#snappyhexmesh-面板)
- [通用字典面板](#通用字典面板)
- [一键同步边界 (Sync Boundaries)](#一键同步边界-sync-boundaries)
- [文件查看器](#文件查看器)
- [编辑器与算例浏览器](#编辑器与算例浏览器)
- [ParaView 集成](#paraview-集成)
- [环境要求](#环境要求)
- [编译方法](#编译方法)
- [使用说明](#使用说明)
- [项目结构](#项目结构)
- [许可证](#许可证)

---

## 功能概览

| 模块 | 说明 |
|------|------|
| **算例浏览器** | 树形展示 `0/`、`constant/`、`system/` 目录；支持多算例；文件过滤 |
| **代码编辑器** | 多标签页编辑器，行号显示，当前行高亮，语法高亮 |
| **语法高亮** | 完整 OpenFOAM 字典语法：关键字、标量、矢量、宏、量纲 |
| **文件类型检测** | 自动识别 OpenFOAM 场文件、字典文件等，并显示提示信息 |
| **边界条件面板** | RTM 参数表可视化编辑、Patch 浏览、智能推荐 |
| **湍流模型面板** | RAS/LES 模型选择，模型参数表单 |
| **离散格式与求解器面板** | 结构化编辑 `fvSchemes` 和 `fvSolution` |
| **snappyHexMesh 面板** | 完整 snappyHexMeshDict 配置，分区导航 |
| **通用字典面板** | 编辑 blockMeshDict、controlDict、decomposeParDict 等 |
| **ParaView 集成** | 菜单一键启动 ParaView 后处理 |
| **Sync Boundaries** | 一键将 blockMeshDict 边界名称同步到 0/ 文件夹所有场文件 |
| **终端** | 在算例目录打开系统终端 |
| **最近算例** | 快速访问最近打开的算例（最多 10 个） |

---

## 边界条件面板 (Boundary Conditions)

BC 面板是功能最丰富的模块，设计匹配 OpenFOAM 官方文档格式（RTM — **R**equired / **T**ype / **M**andatory）。

### 支持的类型

从 OpenFOAM 文件头自动检测场类型，并提供上下文感知的 BC 列表：

| 场类型 | FoamFile Class | 示例场 | 图标 |
|--------|---------------|--------|------|
| **标量** | `volScalarField` | `p`, `p_rgh`, `k`, `epsilon`, `omega`, `nut`, `T`, `alpha.*` | `S` |
| **矢量** | `volVectorField` | `U`, `v` | `V` |
| **张量** | `volTensorField` | `tau`, `R` | `T` |
| **对称张量** | `volSymmTensorField` | — | `T` |
| **面标量** | `surfaceScalarField` | `phi` | — |
| **面矢量** | `surfaceVectorField` | `Uf` | — |

### 场感知 BC 排序

BC 类型根据场名智能排序。例如，打开 **U** 场文件时：
- `noSlip`、`slip`（壁面类型）排在最前
- `flowRateInletVelocity`、`pressureInletOutletVelocity`（入口类型）紧随其后
- 其他适用类型按类别分组排列

打开 **p** 或 **p_rgh** 时，`zeroGradient`、`fixedFluxPressure`、`totalPressure` 优先排列。

### 分类筛选标签

所有 BC 类型按 8 个可筛选类别组织：

| 标签 | 类别 | 示例类型 |
|------|------|----------|
| **All** | 当前场所有类型 | — |
| **Basic** | 基础 BC | `fixedValue`, `zeroGradient`, `fixedGradient`, `mixed`, `calculated` |
| **Wall** | 壁面 BC + 壁函数 | `noSlip`, `slip`, `kqRWallFunction`, `epsilonWallFunction`, `omegaWallFunction`, `nutkWallFunction` |
| **Inlet** | 入口条件 | `flowRateInletVelocity`, `pressureInletOutletVelocity`, `totalPressure`, `turbulentIntensityKineticEnergyInlet`, `turbulentDFSEMInlet` |
| **Outlet** | 出口条件 | `inletOutlet`, `advective`, `waveTransmissive`, `freestream`, `flowRateOutletVelocity` |
| **Pressure** | 压力专用 | `totalPressure`, `fixedFluxPressure`, `fixedMean`, `fanPressure`, `uniformTotalPressure` |
| **Mapped** | 映射/耦合 | `mappedFixedValue`, `timeVaryingMappedFixedValue`, `mappedMixed`, `fixedProfile` |
| **Constraint** | 几何约束 | `empty`, `symmetry`, `wedge`, `cyclic`, `cyclicAMI`, `processor` |
| **Coded** | 用户自定义 (C++) | `codedFixedValue`, `codedMixed` |

### 类型搜索

BC 类型列表上方的搜索框支持按名称或描述实时过滤（例如输入 "wall" 显示所有壁函数类型）。

### RTM 参数表

选中 BC 类型后显示匹配 OpenFOAM 官方文档格式的详细参数表：

| 列 | 说明 |
|----|------|
| **Property** | 参数关键字（如 `value`, `gradient`, `p0`, `gamma`） |
| **Description** | 参数的中文说明 |
| **Type** | 参数类型（`scalar`, `vector`, `word`, `Function1<scalar>` 等） |
| **Required** | `yes`（红色）或 `no`（绿色）— 继承自官方源代码 `.H` 文件 |
| **Default** | 默认值；**可编辑** — 双击修改后自动更新预览 |

`type` 行始终显示在顶部并带有高亮背景。必填参数列在可选参数之前。

#### 可编辑默认值

**Default** 列单元格可编辑。修改默认值后实时更新预览。你可以：
- 设置自定义 `value`（如入口 U 的 `uniform (10 0 0)`）
- 调整模型常数（如壁函数的 `Cmu`, `kappa`, `E`）
- 配置入口参数（如 `flowRate`, `intensity`）

### Patch 浏览器

加载场文件后，自动解析 `boundaryField` 块中的所有 patch 并显示：

- **Patch 名称**及当前**类型**（颜色编码）：
  - 蓝色：fixedValue / uniformFixedValue / flowRate 类型
  - 红色：noSlip / slip / wall 类型
  - 绿色：zeroGradient / empty / symmetry / wedge
  - 橙色：inlet/outlet 类型
  - 紫色：mapped / coupled 类型
  - 灰色：calculated
- 点击 patch 自动在列表中选中其当前 BC 类型
- 如果 patch 未配置类型 (`?`)，面板根据以下信息智能推荐最合适的 BC：
  - 场名（U, p, k, epsilon, omega, nut, alpha.*, T）
  - Patch 名称（inlet, outlet, wall, atmosphere, symmetry）

### 智能 BC 推荐

点击未配置的 patch（类型 `?`）时，面板应用启发式规则：

| 场 | Patch 名称包含 | 推荐 BC |
|----|---------------|---------|
| U | wall, plate, bottom, top | `noSlip` |
| U | inlet, in, entry | `flowRateInletVelocity` |
| U | outlet, out, exit | `pressureInletOutletVelocity` |
| p / p_rgh | wall | `fixedFluxPressure` |
| p / p_rgh | inlet | `totalPressure` |
| p / p_rgh | outlet | `zeroGradient` |
| k / epsilon / omega | wall | `kqRWallFunction` / `epsilonWallFunction` / `omegaWallFunction` |
| nut | wall | `nutkWallFunction` |
| alpha.* | wall | `zeroGradient` |
| alpha.* | outlet | `inletOutlet` |
| T | wall | `zeroGradient` |
| 任意 | symmetry, sym | `symmetry` |

### 代码预览

暗色主题的代码预览面板实时显示生成的字典片段。可选参数以 `// (optional)` 注释标注。

### 应用 / 插入操作

- **Apply to Editor（应用到编辑器）** — 在当前光标位置插入 BC 代码片段
- **Refresh Preview（刷新预览）** — 用当前参数值重新生成预览
- **右键上下文菜单**：
  - *Insert type name only* — 仅插入 BC 类型名称
  - *Insert BC Snippet* — 插入完整的格式化代码块
  - *Insert for patch \<name\>* — 使用选定的 patch 名称插入

### 完整 BC 类型数据库

内置数据库包含 **100+ 种边界条件类型**，均对照 OpenFOAM v2206 源代码 `.H` 文件验证：

| 类别 | 数量 | 示例 |
|------|------|------|
| **Basic** | 10 | `fixedValue`, `zeroGradient`, `fixedGradient`, `mixed`, `calculated`, `scaledFixedValue`, `uniformFixedValue`, `uniformFixedGradient` |
| **Wall** | 16 | `noSlip`, `slip`, `partialSlip`, `movingWallVelocity`, `rotatingWallVelocity`, `translatingWallVelocity`, `fixedNormalSlip`, `activeBaffleVelocity` + 8 种壁函数 |
| **Inlet** | 22 | `flowRateInletVelocity`, `pressureInletOutletVelocity`, `turbulentIntensityKineticEnergyInlet`, `turbulentDFSEMInlet`, `cylindricalInletVelocity`, `swirlFlowRateInletVelocity`, `pressurePIDControlInletVelocity` 等 |
| **Outlet** | 14 | `inletOutlet`, `outletInlet`, `advective`, `waveTransmissive`, `freestream`, `supersonicFreestream`, `flowRateOutletVelocity`, `outletMachNumberPressure`, `acousticWaveTransmissive` 等 |
| **Pressure** | 16 | `totalPressure`, `uniformTotalPressure`, `prghTotalPressure`, `fixedFluxPressure`, `fixedMean`, `fanPressure`, `uniformDensityHydrostaticPressure`, `plenumPressure`, `rotatingTotalPressure`, `syringePressure` 等 |
| **Mapped** | 7 | `mappedFixedValue`, `timeVaryingMappedFixedValue`, `mappedMixed`, `mappedFlowRate`, `fixedProfile`, `mappedFixedInternalValue`, `mappedVelocityFluxFixedValue` |
| **Special** | 10 | `fixedJump`, `fixedJumpAMI`, `uniformJump`, `uniformJumpAMI`, `interfaceCompression`, `waveSurfacePressure` 等 |
| **Coded** | 2 | `codedFixedValue`, `codedMixed` |
| **Constraint** | 14 | `empty`, `symmetry`, `symmetryPlane`, `wedge`, `cyclic`, `cyclicAMI`, `cyclicACMI`, `cyclicSlip`, `processor`, `processorCyclic`, `patch`, `wall` |
| **Turbulence Wall Functions** | 13 | `kqRWallFunction`, `epsilonWallFunction`, `omegaWallFunction`, `nutkWallFunction`, `nutUSpaldingWallFunction`, `nutUWallFunction`, `nutkRoughWallFunction`, `nutURoughWallFunction`, `nutLowReWallFunction`, `nutUBlendedWallFunction`, `nutUTabulatedWallFunction`, `kLowReWallFunction`, `alphatWallFunction` |

---

## 湍流模型面板 (Turbulence Model)

通过可视化表单界面配置 `constant/turbulenceProperties`。

### 功能

- **RAS / LES 切换** — 在雷诺平均和大涡模拟模型之间切换
- **模型选择** — 下拉列表包含全部 OpenFOAM 湍流模型：
  - RAS: `laminar`, `kEpsilon`, `kOmega`, `kOmegaSST`, `kOmegaSSTLM`, `RNGkEpsilon`, `realizableKE`, `LaunderSharmaKE`, `kOmega2006`, `v2f`, `LRR`, `SSG`, `EBRSM`
  - LES: `Smagorinsky`, `WALE`, `dynamicKEqn`, `kEqn`, `dynamicLagrangian`, `DeardorffDiffStress`, `SpalartAllmaras`
- **Turbulence / Print coeffs** — 开关控制
- **模型参数** — 每种模型自动加载对应参数（如 `Cmu`, `C1`, `C2`, `sigmak`, `sigmaEps`, `alphaK1`, `alphaOmega1`, `Ce`, `Ck` 等）
- **可编辑参数表**，含描述和默认值
- **预览**生成的 `turbulenceProperties` 字典
- **Apply to Editor** 插入配置块

---

## 离散格式与求解器面板 (fvSchemes & fvSolution)

通过结构化表单配置 `system/fvSchemes` 和 `system/fvSolution`。

### fvSchemes 类别

| 类别 | 说明 | 常用选项 |
|------|------|----------|
| `ddtSchemes` | 时间导数 | `Euler`, `backward`, `steadyState`, `CrankNicolson`, `localEuler` |
| `gradSchemes` | 梯度 | `Gauss linear`, `leastSquares`, `cellLimited Gauss linear` |
| `divSchemes` | 散度 | `Gauss linear`, `Gauss linearUpwind`, `Gauss limitedLinear` |
| `laplacianSchemes` | 拉普拉斯 | `Gauss linear corrected`, `Gauss linear orthogonal` |
| `interpolationSchemes` | 插值 | `linear`, `cubicCorrection`, `midPoint` |
| `snGradSchemes` | 面法向梯度 | `corrected`, `orthogonal`, `limited` |
| `fluxRequired` | 通量计算 | `none`, `p`, `phi` |

### fvSolution 类别

| 类别 | 说明 |
|------|------|
| **Solvers** | 线性求解器选择（PCG, PBiCG, GAMG, smoothSolver），含预条件器、容差、相对容差 |
| **Relaxation Factors** | 场级亚松弛因子（U, p, k, epsilon, omega 等） |
| **PIMPLE / SIMPLE** | 算法控制：`nOuterCorrectors`, `nCorrectors`, `momentumPredictor`, `pRefCell`, `pRefValue` |

### 功能

- **fvSchemes / fvSolution 切换**标签页
- **每个类别**有独立的子区域，含下拉框、数值微调框和文本输入框
- **预设格式**快速设置（如"二阶格式"、"一阶迎风格式"）
- **Apply to Editor** 写入配置字典

---

## snappyHexMesh 面板

通过结构化导航配置 `system/snappyHexMeshDict`。

### 分区

| 分区 | 关键参数 |
|------|----------|
| **Geometry** | 网格细化区域，文件引用 |
| **Castellated Mesh** | `maxGlobalCells`, `maxLocalCells`, `minRefinementCells`, `nCellsBetweenLevels`, `resolveFeatureAngle`, `allowFreeStandingZoneFaces` |
| **Features** | 特征边细化：`file`, `levels` |
| **Refinement Surfaces** | 每个面：`level`, `patchInfo` |
| **Snapping** | `nSmoothPatch`, `tolerance`, `nSolveIter`, `nRelaxIter` |
| **Layering** | `nSurfaceLayers`, `expansionRatio`, `finalLayerThickness`, `minThickness`, `featureAngle`, `slipFeatureAngle` |
| **Mesh Quality** | `maxNonOrtho`, `maxBoundarySkewness`, `maxInternalSkewness`, `maxConcave`, `minVol`, `minTetQuality`, `minArea`, `minTwist` |

### 功能

- 基于分区的导航列表
- 每个参数显示类型、默认值和说明
- 可编辑值，实时预览
- **Apply to Editor** 写入完整 `snappyHexMeshDict`

---

## 通用字典面板

使用预定义参数集配置多种 OpenFOAM 字典。

### 支持的字典

| 字典 | 关键参数 |
|------|----------|
| **blockMeshDict** | `convertToMeters`, vertices, blocks, edges, boundaries（含预定义 patch 类型） |
| **controlDict** | `startFrom`, `startTime`, `stopAt`, `endTime`, `deltaT`, `writeControl`, `writeInterval`, `purgeWrite`, `writeFormat`, `writePrecision`, `runTimeModifiable` |
| **decomposeParDict** | `numberOfSubdomains`, `method` (simple, hierarchical, scotch, metis, manual), `coeffs` |
| **topoSetDict** | Set 定义，操作 |
| **dynamicMeshDict** | 动网格配置 |
| **refineMeshDict** | 细化参数 |

### 功能

- 字典选择器（根据文件名自动检测）
- 参数表含类型、默认值和说明
- 可编辑值
- **Apply to Editor** 写入格式化字典

---

## 一键同步边界 (Sync Boundaries)

`Case → Sync Boundaries` 一键将 `system/blockMeshDict` 中的边界 patch 名称同步到 `0/`（或 `0.orig/`）时间目录下的所有场文件中。

### 用途

在 `blockMeshDict` 中修改网格——添加、删除或重命名边界 patch 后，每个场文件（U, p, k, epsilon 等）的 `boundaryField` 段必须包含每个 patch 的对应条目。手动更新所有场文件既繁琐又容易出错。此功能可自动化完成。

### 工作流程

1. 在 `system/`（或 `constant/polyMesh/`）中定位 `blockMeshDict`
2. 使用**括号深度计数**（非正则）解析 `boundary (...)` 块，正确处理面列表中的嵌套括号
3. 提取每个 patch 的名称及其类型（`patch`, `wall`, `empty`, `symmetry` 等）
4. 查找第一个时间目录（`0/`, `0.orig/` 或 `0.xxx/`）
5. 对该目录下的每个场文件：
   - 解析 `boundaryField { ... }` 块（同样使用深度计数）
   - 识别 blockMeshDict 中在场文件中**缺失**的 patch
   - 用合适的默认边界条件追加缺失的 patch

### 默认 BC 分配规则

缺失的 patch 根据 blockMeshDict 中的 **patch 类型**和**场名**分配默认边界条件：

| Patch 类型 | 场 | 默认 BC |
|-----------|-----|---------|
| `wall` | U, v | `noSlip` |
| `wall` | p, p_rgh | `fixedFluxPressure` |
| `wall` | k | `kqRWallFunction` |
| `wall` | epsilon | `epsilonWallFunction` |
| `wall` | omega | `omegaWallFunction` |
| `wall` | nut, alphat | `nutkWallFunction` |
| `wall` | T, 其他标量 | `zeroGradient` |
| `empty` | 任意 | `empty` |
| `symmetry` / `symmetryPlane` | 任意 | `symmetry` |
| `wedge` | 任意 | `wedge` |
| `cyclic` / `cyclicAMI` | 任意 | `cyclic` |
| `patch` | U, v | `zeroGradient` |
| `patch` | 其他 | `zeroGradient` |

### 不会执行的操作

- **不会删除**已有的边界条目 — 仅追加缺失项
- **不会修改**已有 patch 的 BC 类型 — 保留用户配置
- **不会触碰**第一个时间目录之外的文件

### 状态反馈

执行后状态栏显示摘要：

> `Synced 5 field file(s) in 0 — added 4 missing patch(es).`
> （已在 0/ 目录同步 5 个场文件，添加了 4 个缺失的 patch。）

或

> `All field files are already in sync.`
> （所有场文件已保持同步。）

---

## 文件查看器

应用程序支持在标签页中直接查看多种文件格式，无需依赖外部程序。

### 支持格式

| 类别 | 扩展名 | 查看方式 |
|------|--------|---------|
| **光栅图片** | PNG, JPG, JPEG, BMP, GIF, WebP, ICO | QPixmap 可滚动原生查看 |
| **矢量图片** | SVG | QSvgWidget 矢量渲染 |
| **EPS / PostScript** | EPS, EPSF, PS | Ghostscript → PNG（自动检测 GS）；QPixmap 回退 |
| **PDF 文档** | PDF | 信息卡片 + 手动打开按钮 |
| **Office 文档** | DOC, DOCX, XLS, XLSX, PPT, PPTX | 信息卡片 + 手动打开按钮 |

### EPS 渲染

EPS 文件通过三级管线在应用内渲染：

1. **QPixmap** — 部分平台可直接加载 EPS
2. **Ghostscript** — 自动搜索常见安装路径，转 EPS 为临时 PNG 显示
3. **错误提示** — 无可用的渲染器时，状态栏提示安装 Ghostscript

### 其他程序打开

在**算例浏览器**中右键任意文件，选择 **Open With...** 可选择一个外部程序打开该文件。

---

## 编辑器与算例浏览器

### 代码编辑器

- **语法高亮** — OpenFOAM 关键字、C++ 关键字、标量、矢量、张量、宏（`#include`, `#ifdef`）、FoamFile 头、单位/量纲
- **行号** — 当前行高亮
- **多标签页** — Ctrl+W 关闭文件；Ctrl+Tab 切换文件
- **未保存更改** — 标签页标题 `*` 标记 + 关闭确认
- **注释/取消注释** — `Ctrl+/` 根据文件语言切换 `//` 或 `#`
- **查找** — `Ctrl+F` 文本搜索
- **撤销/重做** — `Ctrl+Z` / `Ctrl+Y`
- **文件类型检测** — 根据文件头模式自动检测 OpenFOAM 或通用文件

### 算例浏览器

- 树形结构映射算例目录：时间目录 → `constant/`（含 `polyMesh/` 等子目录）→ `system/`
- 文件类型感知图标（场文件、字典文件、通用文件）
- 文件描述提示
- **过滤器**快速搜索文件
- **右键上下文菜单**：
  - 打开文件 / 关闭算例 / 刷新算例
  - 新建文件 / 新建文件夹 / 删除
- **多算例支持** — 可同时打开管理多个算例

---

## ParaView 集成

无需手动创建 `.foam` 文件或打开终端，从 GUI 界面直接启动 ParaView 进行后处理。

### 工作方式

1. **Case → ParaView**（或工具栏 ParaView 按钮）启动 ParaView 并加载当前 OpenFOAM 算例
2. 自动在算例根目录创建 `.foam` 占位文件（ParaView 的 OpenFOAM 阅读器需要此文件）
3. ParaView 以 `--data=<caseName>.foam` 打开，即可进行后处理

### ParaView 路径配置

应用程序按以下顺序搜索 ParaView：

**Windows：**
```
1. 用户配置路径 (Case → ParaView Path...)
2. paraview.exe (系统 PATH)
3. C:/Program Files/ParaView 5.13/bin/paraview.exe
4. C:/Program Files/ParaView 5.12/bin/paraview.exe
5. C:/Program Files/ParaView 5.11/bin/paraview.exe
6. C:/Program Files/ParaView 5.10/bin/paraview.exe
7. C:/Program Files/ParaView/bin/paraview.exe
```

**Linux：**
```
1. 用户配置路径
2. paraview (系统 PATH)
3. /usr/bin/paraview
4. /usr/local/bin/paraview
```

如果找不到 ParaView，会弹出对话框提示浏览可执行文件。你也可以随时手动配置路径：

- **Case → ParaView Path...** — 打开文件浏览器选择 ParaView 可执行文件
- 配置的路径会跨会话持久保存（存储在 `QSettings` 中）

### 下载 ParaView

ParaView 是免费、开源、跨平台的数据分析和可视化应用程序。

| 平台 | 下载链接 |
|------|----------|
| **Windows** | [ParaView 5.13.2 Windows (64-bit)](https://www.paraview.org/download/) |
| **Linux** | [ParaView 5.13.2 Linux (64-bit)](https://www.paraview.org/download/) |
| **macOS** | [ParaView 5.13.2 macOS](https://www.paraview.org/download/) |
| **全部版本** | [https://www.paraview.org/download/](https://www.paraview.org/download/) |

> **建议**：安装 ParaView 5.10 或更高版本以获得最佳的 OpenFOAM 阅读器兼容性。Windows 安装时默认路径 `C:/Program Files/ParaView <version>/` 将被 GUI 自动检测。

---

## 环境要求

- **Qt 6.10.2** (MinGW 64-bit)
- **MinGW-w64** (GCC 15.2.0+)
- **OpenFOAM** (v2012 至 v2512)
- **ParaView** 5.10+ (可选，用于 Case → ParaView 后处理)

## 编译方法

### 前置条件

确保以下工具已安装并可用：

| 工具 | 需要版本 | Windows 典型路径 |
|------|---------|-------------------|
| **Qt** | 6.10.2 (MinGW 64-bit) | `D:/Qt/6.10.2/mingw_64/` |
| **MinGW-w64** | GCC 15.2.0+ | `D:/mingw64/` |
| **Git** | 2.x (版本控制) | `D:/Git/` |

### 项目配置 (OpenFOAMGUI.pro)

```pro
QT       += core gui widgets
CONFIG   += c++17 console

# Windows: GUI 应用（无控制台窗口），MinGW 链接器标志
mingw {
    QMAKE_LFLAGS_CONSOLE = -Wl,-subsystem,windows -mthreads
}

TARGET   = OpenFOAMGUI
TEMPLATE = app

SOURCES  += src/main.cpp src/mainwindow.cpp src/casebrowser.cpp \
            src/codeeditor.cpp src/ofhighlighter.cpp src/ofparser.cpp \
            src/linenumberarea.cpp src/languagedetector.cpp \
            src/bctypedatabase.cpp src/bcpanel.cpp \
            src/turbulencemodeldatabase.cpp src/turbulencepanel.cpp \
            src/schemespanel.cpp src/snappypanel.cpp src/dictpanel.cpp

HEADERS  += src/mainwindow.h src/casebrowser.h src/codeeditor.h \
            src/ofhighlighter.h src/ofparser.h src/linenumberarea.h \
            src/languagedetector.h src/bctypedatabase.h src/bcpanel.h \
            src/turbulencemodeldatabase.h src/turbulencepanel.h \
            src/schemespanel.h src/snappypanel.h src/dictpanel.h

RESOURCES += resources.qrc
RC_ICONS   = src/bychen.ico
```

- **C++17** 标准，支持 GNU 扩展 (`-std=gnu++1z`)
- **Qt 模块**：Core, GUI, Widgets
- **链接器**：`-Wl,-subsystem,windows` 隐藏 Windows 控制台窗口
- **资源文件**：`resources.qrc` 嵌入应用图标；`RC_ICONS` 设置 Windows `.exe` 图标

### 命令行编译 (Windows)

打开终端（Git Bash、MSYS2 或命令提示符），设置环境（请根据实际路径修改命令中的路径）：

```bash
# 将 Qt 和 MinGW 添加到 PATH
export PATH="D:/Qt/6.10.2/mingw_64/bin:$PATH"
export PATH="D:/mingw64/bin:$PATH"

# 进入项目目录
cd OpenFOAMGUI

# 步骤 1：从 .pro 生成 Makefile
qmake OpenFOAMGUI.pro

# 步骤 2：编译（Release 配置）
mingw32-make -f Makefile.Release

# 输出文件：release/OpenFOAMGUI.exe
```

**构建目标：**

| 命令 | 说明 |
|------|------|
| `mingw32-make -f Makefile.Release` | Release 编译（优化 `-O2`） |
| `mingw32-make -f Makefile.Debug` | Debug 编译（含符号 `-g`） |
| `mingw32-make -f Makefile.Release clean` | 清理 Release 编译产物 |
| `mingw32-make -f Makefile.Release distclean` | 删除所有生成文件 |

**编译器标志 (Release)：**
```
-O2 -std=gnu++1z -Wall -Wextra -fexceptions -mthreads
-DUNICODE -D_UNICODE -DWIN32 -DMINGW_HAS_SECURE_API=1
```

**链接器标志 (Release)：**
```
-Wl,-s -Wl,-subsystem,windows -mthreads
```

**链接库：**
```
libQt6Widgets.a  libQt6Gui.a  libQt6Core.a
```

### Qt Creator 编译

1. 打开 **Qt Creator**
2. **文件 → 打开文件或项目...** → 选择 `OpenFOAMGUI.pro`
3. 提示时选择 **MinGW 64-bit** 套件 (Qt 6.10.2)
4. 选择 **Release** 构建配置
5. 点击 **构建 → 构建项目**（或 `Ctrl+B`）
6. 可执行文件输出到 `release/OpenFOAMGUI.exe`

### Linux / macOS 编译

```bash
# 确保已安装 Qt 6.10+ 和 GCC/Clang
qmake6 OpenFOAMGUI.pro     # 或 qmake（取决于发行版）
make -f Makefile.Release

# 输出文件：release/OpenFOAMGUI
```

> **注意**：`RC_ICONS` 和 Windows 专用链接器标志在非 Windows 平台上通过 `mingw { }` 条件块忽略。

---

## 使用说明

1. 启动 `OpenFOAMGUI.exe`
2. **Case → Open Case**（`Ctrl+O`）选择一个 OpenFOAM 算例目录
3. 在**算例浏览器**（Case Browser）中双击任意文件打开到编辑器
4. 右侧面板自动切换上下文：
   - 场文件（U, p, k 等）→ **边界条件**面板
   - `turbulenceProperties` → **湍流模型**面板
   - `fvSchemes` / `fvSolution` → **离散格式与求解器**面板
   - `snappyHexMeshDict` → **snappyHexMesh** 面板
   - `blockMeshDict`, `controlDict` 等 → **通用字典**面板
5. 在面板中配置参数，然后点击 **Apply to Editor** 插入/修改
6. **保存**（`Ctrl+S`）将更改写回算例

---

## 项目结构

```
OpenFOAMGUI/
├── src/
│   ├── main.cpp                       # 应用程序入口
│   ├── mainwindow.h / .cpp            # 主窗口、菜单、工具栏、停靠窗口
│   ├── casebrowser.h / .cpp           # 算例目录树浏览器
│   ├── codeeditor.h / .cpp            # 代码编辑器（行号显示）
│   ├── ofhighlighter.h / .cpp         # OpenFOAM/C++ 语法高亮器
│   ├── ofparser.h / .cpp              # OpenFOAM 文件头/关键字解析器
│   ├── languagedetector.h / .cpp      # 文件语言自动检测（OF vs C++）
│   ├── linenumberarea.h / .cpp        # 行号边栏控件
│   ├── bcpanel.h / .cpp               # 边界条件面板（RTM 表 + Patch 浏览）
│   ├── bctypedatabase.h / .cpp        # 100+ BC 类型定义数据库
│   ├── turbulencepanel.h / .cpp       # RAS/LES 湍流模型面板
│   ├── turbulencemodeldatabase.h / .cpp  # 湍流模型定义
│   ├── schemespanel.h / .cpp          # fvSchemes / fvSolution 面板
│   ├── snappypanel.h / .cpp           # snappyHexMeshDict 面板
│   ├── dictpanel.h / .cpp             # 通用字典面板
│   └── bychen.ico                     # 应用程序图标
├── OpenFOAMGUI.pro                    # Qt 项目文件
├── resources.qrc                      # Qt 资源文件
├── README.md                          # 英文说明文档
└── README_CN.md                       # 中文说明文档（本文件）
```

## 许可证

本项目仅供 CFD 工作流程中的教育和研究用途。
