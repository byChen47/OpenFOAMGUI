# OpenFOAM GUI — UI 重构方案

基于对完整代码库的专业审查（Qt 工程师 + CFD 开发者双重视角），制定以下 UI 调整和功能增删方案。

---

## 一、核心定位

当前应用定位是 **"文件编辑器 + 参考面板"**，但 CFD 工程师的实际工作流是 **"编辑 → 运行 → 检查输出 → 修改 → 再运行"**，需要的是一个 **CFD 工作流 IDE**。最大问题是用户必须在本应用和外部终端之间频繁切换。

---

## 二、建议删除的功能

| 删除项 | 原因 | 涉及位置 |
|--------|------|----------|
| **Run C++** | CFD 工程师极少在 case 目录内编译 C++，属于通用 IDE 功能 | `mainwindow.cpp` onRunCpp(), onConfigureCpp(), Case 菜单+工具栏 |
| **Run Python** | CFD 中 Python 主要用于后处理（Jupyter），非编辑器内运行。保留 `.py` 语法高亮 | `mainwindow.cpp` onRunPython(), onConfigurePython() |
| **工具栏隐藏按钮** | New File/New Folder 隐藏占用 View 菜单空间但不提供价值 | View 菜单 toolbar toggles |
| **Save As / Save All 菜单** | Ctrl+S 足够覆盖，Save All 极少使用 | File 菜单 |
| **Ctrl+Shift+W (Close Case)** | 与 Ctrl+W (Close Tab) 易混淆 | m_closeCaseAction 快捷键移除 |

**删除后清理**：
- Case 菜单: 移除 Run Python, Run C++, Python Path, C++ Compiler Path
- 工具栏: 从 10 按钮减到 6 按钮
- 状态变量: 移除 m_pythonPath, m_cppCompilerPath
- QSettings: 移除 pythonPath, cppCompilerPath 持久化
- 释放约 300 行代码

---

## 三、UI 布局重构

### 当前布局
```
┌──────────┬───────────────────────────┐
│ Case     │ Tabs                      │
│ Browser  │                           │
│ (260px)  │                           │
│          ├───────────────────────────┤
│          │ Right Panel (320px)       │
│          │ [BC / Turb / Schemes /    │
│          │  Snappy / Dict stacked]   │
└──────────┴───────────────────────────┘
```
- 右侧面板仅 320px 固定，5 面板堆叠
- 底部完全空闲

### 建议布局
```
┌──────────┬───────────────────┬───────────┐
│ Case     │ Tabs              │ Context   │
│ Browser  │ (Editor/Viewer)   │ Panel     │
│ (220px)  │                   │ (flexible)│
│          │                   │ - BC      │
│          │                   │ - Turb    │
│          │                   │ - Schemes │
│          │                   │ - Dict    │
│          ├───────────────────┴───────────┤
│          │ Integrated Terminal          │
│          │ (toggle: Ctrl+`)             │
│          │ $ blockMesh                   │
│          │ $ simpleFoam                  │
└──────────┴───────────────────────────────┘
```
- Case Browser 缩至 220px
- 右侧面板弹性宽度
- **底部新增集成终端**（最重要的改动）

---

## 四、新增功能（按优先级）

### P0 — 必须添加

#### 1. 集成终端 (Integrated Terminal)

最重要缺失功能。基于 `QPlainTextEdit` + `QProcess` 实现：

```cpp
class TerminalWidget : public QWidget {
    QPlainTextEdit *m_output;  // 历史输出 (read-only)
    QLineEdit      *m_input;   // 命令输入
    QProcess       *m_process; // 执行 bash/cmd
    QString         m_workDir; // 当前 case 目录
};
```

- 快捷键 `Ctrl+\`` 切换显示/隐藏
- 工作目录自动同步到当前打开的 case 目录
- 支持 `cd` 命令
- 保持命令历史（上下箭头）
- 添加为 `QDockWidget`，位置 `Qt::BottomDockWidgetArea`

#### 2. 求解器启动器 (Solver Launcher)

Case 菜单新增 "Run Solver..." 对话框：

```
┌─────────────────────────────────────┐
│ Run Solver                          │
│                                     │
│ Solver:  [simpleFoam       ▾]       │
│ Parallel: [4] cores                 │
│                                     │
│ ☐ mesh only (blockMesh)             │
│ ☐ decompose first                   │
│ ☑ run solver                        │
│                                     │
│ Command: mpirun -np 4 simpleFoam -parallel │
│                                     │
│ [Cancel]              [Run]         │
└─────────────────────────────────────┘
```

- solver 列表从 codeeditor.cpp 已有 120+ solver 名称读取
- 自动检测 system/decomposeParDict 是否存在
- 自动检测 constant/polyMesh 是否需要首先生成
- 输出实时流到集成终端

#### 3. showTerminalOutput 异步化

当前 `showTerminalOutput()` (mainwindow.cpp:1548) 使用 `proc.waitForFinished()` **阻塞 UI**。必须改为：

```cpp
connect(proc, &QProcess::finished, this, [=](int exitCode) {
    // 显示输出（非阻塞）
});
```

### P1 — 应该添加

#### 4. 面板 Run 按钮

Dict Panel 和 Snappy Panel 当前只有参数编辑，无法运行工具。每个 Panel 底部添加 "Run"：

```
[Insert Full Template] [Preview] [Run blockMesh]
```

流程：检查未保存修改 → 提示保存 → 在集成终端执行命令 → 实时显示输出。

适用面板：
- DictPanel (blockMesh) → `Run blockMesh`
- DictPanel (topoSet) → `Run topoSet`
- SnappyPanel → `Run snappyHexMesh`
- 新增 `Run checkMesh`（任何 case）

#### 5. Case 验证器

`Case → Validate Case` — 运行前自动检查：

```
- constant/polyMesh/ 是否存在？
- 0/ 或 0.orig/ 包含必需场文件？
- boundaryField patches 与 blockMeshDict 匹配？
- fvSchemes / fvSolution 一致性？
- controlDict endTime > startTime？
- decomposeParDict numberOfSubdomains > 0？
```

实现：`validateCase(casePath) → QStringList warnings`

#### 6. 面板基类重构

5 个面板 80% 的 `setupUI()` 重复（header 图标+标题+路径、参数表、preview 按钮栏）。抽取基类：

```cpp
class PanelBase : public QWidget {
    Q_OBJECT
public:
    virtual void loadFile(const QString &path, const QString &content) = 0;
    virtual void clear();
    void setEditor(CodeEditor *editor) { m_editor = editor; }
protected:
    void setupHeader(const QString &title, const QColor &accent);
    void addPreviewButton(QBoxLayout *layout);
    CodeEditor *m_editor = nullptr;
    QLabel *m_headerLabel, *m_pathLabel;
    QTextEdit *m_previewEdit;
};
```

好处：新增面板只需实现 `loadFile()`，UI 风格自动统一。

### P2 — 可以添加

#### 7. 新 Case 模板向导

`Case → New Case from Template...`

从 `$FOAM_TUTORIALS` 复制模板 case，自动替换 case 名称。

#### 8. 拖放打开 Case

欢迎文本已承诺但未实现。添加 `dragEnterEvent` / `dropEvent` 到 MainWindow。

---

## 五、菜单简化

### File 菜单（简化后）
```
Recent Cases       ▸
Recent Files       ▸
──────────────
Save          Ctrl+S
──────────────
New File
New Folder
──────────────
Close Tab     Ctrl+W
──────────────
Exit          Alt+F4
```

### Case 菜单（简化后）
```
Open Case...       Ctrl+O
Close Case
──────────────
New File
New Folder
Delete
Clean Time Dirs
──────────────
Run Solver...
Validate Case
──────────────
ParaView
ParaView Path...
──────────────
Sync Boundaries
Refresh
```

### 工具栏（简化后，6 按钮）
```
[Open Case] [Save] [Delete] [BC Panel] [Terminal] [ParaView]
```

---

## 六、前后对比

| 指标 | 当前 | 重构后 |
|------|------|--------|
| 工具栏按钮 | 10 | 6 |
| Case 菜单项 | 14 | 13 |
| 与 OF 无关的功能 | Run C++, Run Python | 0 |
| 集成终端 | 无 | 有 (Ctrl+\`) |
| 求解器启动 | 手动终端 | Solver Launcher 对话框 |
| 面板代码行数 | 各 ~400-700 行 | 基类 ~150 + 各 ~200-400 行 |
| 文件验证 | 无 | Validate Case |
| showTerminalOutput | 阻塞 UI | 异步非阻塞 |
| 面板 Run 按钮 | 无 | 每个工具面板 1 个 |

---

## 七、实施顺序

1. **P0** — 删除 Run C++/Run Python → 集成终端 → 求解器启动器 → showTerminalOutput 异步化
2. **P1** — 面板 Run 按钮 → Case 验证器 → PanelBase 重构
3. **P2** — 模板向导 → 拖放 → 细节打磨
