#include "schemespanel.h"
#include "codeeditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileInfo>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QFont>
#include <QFrame>
#include <QHeaderView>
#include <QRegularExpression>
#include <QScrollArea>
#include <QGroupBox>
#include <QFormLayout>
#include <QMenu>

SchemesPanel::SchemesPanel(QWidget *parent) : QWidget(parent) { setupUI(); initSchemeData(); }

void SchemesPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0); root->setSpacing(0);

    // Header
    auto *hdr = new QFrame();
    hdr->setStyleSheet("QFrame#schHdr { background: #F5F0F0; border-bottom: 2px solid #D73A0F; padding: 6px 10px; }");
    hdr->setObjectName("schHdr");
    auto *hl = new QHBoxLayout(hdr);
    hl->setContentsMargins(8, 6, 8, 6);
    auto *ico = new QLabel("S");
    ico->setFixedSize(26, 26); ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background: #D73A0F; color: white; border-radius: 13px; font-weight: bold; font-size: 13px;");
    hl->addWidget(ico);
    m_headerLabel = new QLabel("No fvSchemes/fvSolution file selected");
    m_headerLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #333;");
    m_headerLabel->setWordWrap(true);
    hl->addWidget(m_headerLabel, 1);
    root->addWidget(hdr);

    m_pathLabel = new QLabel();
    m_pathLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px 10px; background: #FAFAFA;");
    m_pathLabel->setWordWrap(true);
    root->addWidget(m_pathLabel);

    // File type tabs (fvSchemes | fvSolution)
    m_tabBar = new QWidget();
    m_tabBar->setStyleSheet("background: #F5F5F5; border-bottom: 1px solid #DDD;");
    m_tabBar->setObjectName("tabBar");
    auto *tl = new QHBoxLayout(m_tabBar); tl->setContentsMargins(4, 4, 4, 4); tl->setSpacing(2);
    m_tabGroup = new QButtonGroup(this); m_tabGroup->setExclusive(true);
    root->addWidget(m_tabBar);

    // Main split
    auto *ms = new QSplitter(Qt::Horizontal);
    ms->setChildrenCollapsible(false);

    // Left: scheme categories
    auto *lp = new QWidget();
    auto *ll = new QVBoxLayout(lp); ll->setContentsMargins(4, 2, 2, 2);
    m_categoryList = new QListWidget();
    m_categoryList->setStyleSheet(
        "QListWidget { font-size: 12px; border: 1px solid #DDD; border-radius: 3px; background: white; }"
        "QListWidget::item { padding: 5px 8px; }"
        "QListWidget::item:selected { background: #D73A0F; color: white; }"
        "QListWidget::item:hover:!selected { background: #FDE8E3; }");
    ll->addWidget(m_categoryList);
    ms->addWidget(lp);

    // Right: detail
    auto *rp = new QWidget();
    auto *rl = new QVBoxLayout(rp); rl->setContentsMargins(6, 4, 6, 4); rl->setSpacing(4);

    m_catDescLabel = new QLabel();
    m_catDescLabel->setWordWrap(true);
    m_catDescLabel->setStyleSheet("color: #555; font-size: 11px; padding: 4px 6px; background: #FFFFF0; border: 1px solid #F0E0B0; border-radius: 3px;");
    rl->addWidget(m_catDescLabel);

    m_recommendLabel = new QLabel();
    m_recommendLabel->setWordWrap(true);
    m_recommendLabel->setStyleSheet("color: #2E7D32; font-size: 11px; font-weight: bold; padding: 2px 4px;");
    rl->addWidget(m_recommendLabel);

    auto *optHdr = new QLabel("Available Options:");
    optHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(optHdr);

    m_optionTree = new QTreeWidget();
    m_optionTree->setHeaderLabels({"Scheme / Option", "Description", "Order"});
    m_optionTree->setRootIsDecorated(false);
    m_optionTree->setAlternatingRowColors(true);
    m_optionTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_optionTree->setMinimumHeight(150);
    m_optionTree->setStyleSheet(
        "QTreeWidget { font-size: 11px; border: 1px solid #DDD; border-radius: 3px; }"
        "QTreeWidget::item { padding: 3px 6px; }"
        "QTreeWidget::item:selected { background: #D73A0F; color: white; }"
        "QHeaderView::section { background: #EEE; padding: 3px 6px; font-size: 10px; font-weight: bold; border: none; }");
    rl->addWidget(m_optionTree, 2);

    auto *exHdr = new QLabel("Configuration Preview:");
    exHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(exHdr);

    m_previewEdit = new QTextEdit();
    m_previewEdit->setReadOnly(true);
    m_previewEdit->setFont(QFont("Consolas", 10));
    m_previewEdit->setMaximumHeight(150);
    m_previewEdit->setStyleSheet(
        "QTextEdit { background: #1E1E1E; color: #DCDCDC; border: 1px solid #333; border-radius: 3px; padding: 6px; }");
    rl->addWidget(m_previewEdit);

    auto *btnRow = new QHBoxLayout();
    m_previewBtn = new QPushButton("Refresh Preview");
    m_previewBtn->setStyleSheet("QPushButton { padding: 5px 14px; border: 1px solid #CCC; border-radius: 3px; background: #F5F5F5; font-size: 12px; } QPushButton:hover { background: #E8E8E8; }");
    m_applyBtn = new QPushButton("Apply to Editor");
    m_applyBtn->setEnabled(false);
    m_applyBtn->setStyleSheet("QPushButton { padding: 5px 18px; border: none; border-radius: 3px; background: #D73A0F; color: white; font-weight: bold; font-size: 12px; } QPushButton:hover { background: #B5300C; } QPushButton:disabled { background: #CCC; color: #888; }");
    btnRow->addWidget(m_previewBtn); btnRow->addStretch(); btnRow->addWidget(m_applyBtn);
    rl->addLayout(btnRow);

    ms->addWidget(rp);
    ms->setStretchFactor(0, 30);
    ms->setStretchFactor(1, 70);
    root->addWidget(ms, 1);

    connect(m_categoryList, &QListWidget::currentItemChanged, this, &SchemesPanel::onSchemeCategoryChanged);
    connect(m_optionTree, &QTreeWidget::itemClicked, this, &SchemesPanel::onOptionClicked);
    connect(m_optionTree, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QTreeWidgetItem *item = m_optionTree->itemAt(pos);
        if (!item) return;
        // Select the item
        onOptionClicked(item, 0);
        QString chosen = item->text(0);
        QMenu menu;

        // 1) Value only
        QAction *valAct = menu.addAction(
            QString("Insert \"%1\" (value only)").arg(chosen));
        connect(valAct, &QAction::triggered, [this, chosen]() {
            if (!m_editor) return;
            QTextCursor c = m_editor->textCursor();
            c.insertText(chosen);
            m_editor->setTextCursor(c);
        });

        // 2) Single block
        QAction *insertAct = menu.addAction(
            QString("Insert \"%1\" → %2 (single block)")
            .arg(chosen, m_currentCategory.name));
        connect(insertAct, &QAction::triggered, [this, chosen]() {
            if (!m_editor) return;
            QString snip = buildSingleCategorySnippet(m_currentCategory, chosen);
            if (snip.isEmpty()) return;
            QTextCursor c = m_editor->textCursor();
            c.insertText(snip);
            m_editor->setTextCursor(c);
            QApplication::clipboard()->setText(snip);
            m_applyBtn->setText("✓ Inserted!");
            QTimer::singleShot(1500, this, [this]() { m_applyBtn->setText("Apply to Editor"); });
        });

        // 3) Full config
        QAction *insertFullAct = menu.addAction("Insert Full Configuration");
        connect(insertFullAct, &QAction::triggered, [this]() { onApplySnippet(); });
        menu.exec(m_optionTree->mapToGlobal(pos));
    });
    connect(m_previewBtn, &QPushButton::clicked, this, &SchemesPanel::onRefreshPreview);
    connect(m_applyBtn, &QPushButton::clicked, this, &SchemesPanel::onApplySnippet);
#if QT_CONFIG(shortcut)
    connect(m_tabGroup, &QButtonGroup::idClicked, this, &SchemesPanel::onFileTypeTabChanged);
#endif
}

// ════════════════════════════════════════════════════════════════════
// Data initialization
// ════════════════════════════════════════════════════════════════════

void SchemesPanel::initSchemeData()
{
    // ── fvSchemes categories ──

    m_schemeCategories.append({
        "ddtSchemes", "Time derivative (∂/∂t) discretisation. Controls time accuracy and stability.",
        "Euler",
        {{"Euler", "1st-order implicit. Unconditionally stable, dissipative.", "1st, 稳"},
         {"localEuler", "Local time-stepping. Pseudo-transient for steady-state convergence.", "1st, 伪瞬态"},
         {"backward", "2nd-order implicit. Less dissipative than Euler. Requires storage.", "2nd, 推荐"},
         {"CrankNicolson", "2nd-order implicit. Mix of Euler and backward. ψ coefficient controls blend (0=Euler, 1=CN).", "2nd, 精确"},
         {"steadyState", "Steady-state. Neglects time derivative entirely.", "定常"},
         {"CoEuler", "Courant-number limited Euler. Automatic time step adjustment.", "自适应"}}
    });

    m_schemeCategories.append({
        "gradSchemes", "Gradient (∇) computation. Foundation for diffusion, convection, and pressure terms.",
        "Gauss linear",
        {{"Gauss linear", "Central differencing. 2nd-order on regular mesh.", "2nd, 默认"},
         {"Gauss linear corrected", "Central differencing with non-orthogonal correction.", "2nd, 推荐"},
         {"Gauss linear uncorrected", "Central differencing without correction. Faster, less accurate on skewed mesh.", "2nd, 快"},
         {"Gauss cubicCorrected", "4th-order on uniform mesh. Higher accuracy.", "4th"},
         {"Gauss midPoint", "2nd-order using face midpoint values.", "2nd"},
         {"leastSquares", "Least-squares gradient. Good for polyhedral/unstructured meshes.", "变阶, 推荐"},
         {"pointCellsLeastSquares", "Point-cell least squares. More compact stencil.", "变阶"},
         {"faceMDLimited leastSquares", "Face-MD limited least squares. Bounds gradients for stability.", "限制器"}}
    });

    m_schemeCategories.append({
        "divSchemes", "Divergence (∇·) / convection discretisation. Most critical for stability and accuracy.",
        "Gauss linear",
        {{"Gauss linear", "Central differencing. 2nd-order but may oscillate for convection-dominated flows.", "2nd"},
         {"Gauss linearUpwind", "Linear upwind (LUD). 2nd-order with upwind bias. Good balance.", "2nd, 推荐"},
         {"Gauss upwind", "1st-order upwind. Very stable but highly diffusive.", "1st, 稳定"},
         {"Gauss limitedLinear", "Limited linear. TVD limiter (ψ=1). Bounded 2nd-order.", "2nd, TVD"},
         {"Gauss vanLeer", "van Leer TVD limiter. Smoother than limitedLinear.", "2nd, TVD"},
         {"Gauss MUSCL", "MUSCL scheme. 3rd-order with limiter.", "3rd, TVD"},
         {"Gauss QUICK", "3rd-order upwind-biased on hex mesh.", "3rd"},
         {"Gauss SFCD", "2nd-order central with smoothing. Self-filtered.", "2nd, 稳定"},
         {"Gauss Gamma", "Gamma differencing. Blends central and upwind.", "混合"},
         {"bounded Gauss linearUpwind", "Bounded linear upwind for scalar transport (multiphase VOF).", "2nd, 有界"}}
    });

    m_schemeCategories.append({
        "laplacianSchemes", "Laplacian / diffusion (∇·(Γ∇)) discretisation. For viscous and diffusive terms.",
        "Gauss linear orthogonal",
        {{"Gauss linear orthogonal", "Orthogonal correction only. Simplest, for orthogonal meshes.", "2nd"},
         {"Gauss linear corrected", "Full non-orthogonal correction. Accurate on skewed meshes.", "2nd, 推荐"},
         {"Gauss linear uncorrected", "No correction. Fast, less accurate on non-orthogonal meshes.", "2nd, 快"},
         {"Gauss linear limited", "Limited correction. Stability control with ψ factor.", "限制校正"},
         {"Gauss harmonic orthogonal", "Harmonic interpolation for discontinuous diffusivity.", "2nd, 界面"}}
    });

    m_schemeCategories.append({
        "interpolationSchemes", "Cell-to-face interpolation. Used for flux calculations and field reconstruction.",
        "linear",
        {{"linear", "Linear / central interpolation. 2nd-order.", "2nd, 默认"},
         {"midPoint", "Mid-point interpolation with weighting.", "2nd"},
         {"cubic", "Cubic interpolation. Higher order on structured mesh.", "3rd"},
         {"upwind", "Upwind interpolation. 1st-order, stable.", "1st"},
         {"vanLeer", "van Leer TVD interpolation.", "2nd, TVD"}}
    });

    m_schemeCategories.append({
        "snGradSchemes", "Surface normal gradient. Used in Laplacian and pressure correction.",
        "orthogonal",
        {{"orthogonal", "Simple orthogonal correction. OK for orthogonal meshes.", "2nd"},
         {"corrected", "Non-orthogonal correction. Recommended for non-orthogonal meshes.", "2nd, 推荐"},
         {"uncorrected", "No correction. Fast but inaccurate on skewed meshes.", "2nd, 快"},
         {"limited", "Limited correction for stability on poor meshes.", "限制校正"}}
    });

    // ── fvSolution: solver categories ──

    m_solverCategories.append({
        "solvers", "Linear solver configuration for each field. Symmetric vs asymmetric matrix solvers.",
        "PCG / smoothSolver",
        {{"PCG", "Preconditioned Conjugate Gradient. For symmetric matrices (pressure).", "对称, 推荐"},
         {"PBiCGStab", "Preconditioned Bi-Conjugate Gradient Stabilized. For asymmetric matrices (velocity, turbulence).", "非对称, 推荐"},
         {"GAMG", "Geometric-Algebraic Multi-Grid. Excellent for pressure (symmetric). Fast convergence.", "多重网格, 压力"},
         {"smoothSolver", "Generic iterative smoother (Gauss-Seidel, symGaussSeidel, DILU).", "通用"},
         {"PBiCG", "Preconditioned Bi-Conjugate Gradient. Classic asymmetric solver.", "非对称"},
         {"diagonal", "Diagonal solver. Only for very simple problems.", "简单, 快"}}
    });

    m_solverCategories.append({
        "preconditioners", "Matrix preconditioner for accelerating linear solver convergence.",
        "DIC / DILU",
        {{"DIC", "Diagonal Incomplete Cholesky. For symmetric matrices.", "对称"},
         {"DILU", "Diagonal Incomplete LU. For asymmetric matrices.", "非对称, 推荐"},
         {"FDIC", "Faster DIC. Precomputed diagonal, less memory.", "对称, 快"},
         {"diagonal", "Diagonal (Jacobi) preconditioner. Simplest.", "简单"},
         {"GAMG", "Geometric-Algebraic Multi-Grid preconditioner.", "多重网格"},
         {"none", "No preconditioner.", "无"}}
    });

    m_solverCategories.append({
        "smoothes", "Smoother for iterative solvers (smoothSolver, GAMG).",
        "symGaussSeidel",
        {{"symGaussSeidel", "Symmetric Gauss-Seidel. Good all-round smoother.", "推荐"},
         {"GaussSeidel", "Asymmetric Gauss-Seidel smoother.", "标准"},
         {"DILUGaussSeidel", "DILU-preconditioned Gauss-Seidel. Faster convergence.", "更快"},
         {"nonBlockingGaussSeidel", "Non-blocking Gauss-Seidel for parallel efficiency.", "并行优化"},
         {"DICGaussSeidel", "DIC-preconditioned Gauss-Seidel. For symmetric matrices.", "对称, 快"}}
    });

    m_solverCategories.append({
        "PIMPLE / PISO / SIMPLE", "Pressure-velocity coupling algorithm settings.",
        "PIMPLE",
        {{"nOuterCorrectors", "Number of outer corrections (PIMPLE/PISO loops). 1 = PISO, >1 = PIMPLE.", "≥1"},
         {"nCorrectors", "Number of pressure correctors per outer loop. Typically 2-3.", "≥1"},
         {"nNonOrthogonalCorrectors", "Non-orthogonal correctors. 0 for orthogonal mesh, 1-2 for skewed.", "0-2"},
         {"momentumPredictor", "Enable/disable momentum predictor. yes (default) for most cases.", "yes/no"},
         {"pRefCell", "Reference cell for pressure level. Typically 0.", "index"},
         {"pRefValue", "Reference pressure value at pRefCell.", "0"}}
    });

    m_solverCategories.append({
        "relaxationFactors", "Under-relaxation for equation and field updates. Controls stability vs convergence speed.",
        "equations { .* 1; }",
        {{"equations", "Equation under-relaxation (<1 = more stable, slower). Under equations { ... }.", "<1"},
         {"fields", "Field under-relaxation. Controls field update. Under fields { ... }.", "<1"},
         {"U", "Velocity under-relaxation. Typically 0.3-0.7 (SIMPLE) or 0.7-0.9 (PISO/PIMPLE).", "0.3-0.9"},
         {"p", "Pressure under-relaxation. Typically 0.3 (SIMPLE) or 0.7-0.9 (PISO/PIMPLE).", "0.3-0.9"},
         {"k / epsilon / omega", "Turbulence under-relaxation. Typically 0.5-0.7.", "0.5-0.7"}}
    });
}

// ════════════════════════════════════════════════════════════════════
// Load file
// ════════════════════════════════════════════════════════════════════

void SchemesPanel::loadFile(const QString &filePath, const QString &content)
{
    m_filePath = filePath;
    m_currentContent = content;
    QFileInfo fi(filePath);
    m_fileType = fi.fileName();
    m_userChoices.clear();

    m_headerLabel->setText(QString("<b>%1</b> — %2").arg(fi.fileName(),
        fi.fileName() == "fvSchemes" ? "Discretisation Schemes" : "Linear Solver Settings"));
    m_pathLabel->setText(filePath);

    bool isFvSchemes = (m_fileType == "fvSchemes");
    populateFileTypeTabs(isFvSchemes);

    // Populate category list
    m_categoryList->clear();
    auto &cats = isFvSchemes ? m_schemeCategories : m_solverCategories;
    for (const auto &c : cats) {
        auto *item = new QListWidgetItem(c.name);
        item->setData(Qt::UserRole, c.name);
        m_categoryList->addItem(item);
    }

    m_applyBtn->setEnabled(true);

    if (!cats.isEmpty()) {
        m_categoryList->setCurrentRow(0);
        showSchemeCategory(cats.first());
    }
}

void SchemesPanel::clear()
{
    m_headerLabel->setText("No fvSchemes/fvSolution file selected");
    m_pathLabel->clear();
    m_categoryList->clear();
    m_optionTree->clear();
    m_catDescLabel->clear();
    m_recommendLabel->clear();
    m_previewEdit->clear();
    m_applyBtn->setEnabled(false);
}

void SchemesPanel::populateFileTypeTabs(bool isFvSchemes)
{
    for (auto *b : m_tabButtons) delete b;
    m_tabButtons.clear();
    struct { int id; QString lb; } tabs[] = {
        {0, "fvSchemes"}, {1, "fvSolution"},
    };
    for (auto &t : tabs) {
        auto *btn = new QToolButton();
        btn->setText(t.lb); btn->setCheckable(true); btn->setAutoRaise(true);
        btn->setStyleSheet(
            "QToolButton { padding: 4px 14px; border-radius: 10px; font-size: 11px; font-weight: bold; color: #555; }"
            "QToolButton:hover { background: #FDE8E3; color: #D73A0F; }"
            "QToolButton:checked { background: #D73A0F; color: white; }");
        m_tabGroup->addButton(btn, t.id);
        m_tabButtons[t.id] = btn;
        static_cast<QHBoxLayout*>(m_tabBar->layout())->addWidget(btn);
    }
    int sel = isFvSchemes ? 0 : 1;
    if (m_tabButtons.contains(sel)) m_tabButtons[sel]->setChecked(true);
}

void SchemesPanel::onFileTypeTabChanged(int index)
{
    // Reload with the other file type content
    // For now, just reload categories
    m_categoryList->clear();
    auto &cats = (index == 0) ? m_schemeCategories : m_solverCategories;
    for (const auto &c : cats) {
        auto *item = new QListWidgetItem(c.name);
        item->setData(Qt::UserRole, c.name);
        m_categoryList->addItem(item);
    }
    if (!cats.isEmpty()) {
        m_categoryList->setCurrentRow(0);
        showSchemeCategory(cats.first());
    }
}

// ════════════════════════════════════════════════════════════════════
// Scheme display
// ════════════════════════════════════════════════════════════════════

void SchemesPanel::onSchemeCategoryChanged(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    QString name = item->data(Qt::UserRole).toString();
    for (const auto &c : m_schemeCategories)
        if (c.name == name) { showSchemeCategory(c); return; }
    for (const auto &c : m_solverCategories)
        if (c.name == name) { showSchemeCategory(c); return; }
}

void SchemesPanel::showSchemeCategory(const SchemeCategory &cat)
{
    m_currentCategory = cat;
    m_catDescLabel->setText(cat.description);
    m_recommendLabel->setText("Default: " + cat.defaultScheme);

    m_optionTree->clear();
    for (const auto &opt : cat.options) {
        auto *item = new QTreeWidgetItem(m_optionTree);
        item->setText(0, opt.name);
        item->setText(1, opt.description);
        item->setText(2, opt.recommendation);
        item->setData(0, Qt::UserRole, opt.name);
        item->setToolTip(0, opt.description);

        // Highlight recommended/default
        if (opt.name == cat.defaultScheme || opt.recommendation.contains("推荐")) {
            QFont f = item->font(0); f.setBold(true); item->setFont(0, f);
            item->setBackground(2, QColor("#E8F5E9"));
        }
    }

    onRefreshPreview();
}

void SchemesPanel::onOptionClicked(QTreeWidgetItem *item, int /*col*/)
{
    if (!item) return;
    QString name = item->data(0, Qt::UserRole).toString();
    m_userChoices[m_currentCategory.name] = name;
    onRefreshPreview();
}

void SchemesPanel::onRefreshPreview()
{
    m_previewEdit->setPlainText(buildFullConfig());
}

QString SchemesPanel::buildFullConfig() const
{
    QString cfg;
    auto &cats = (m_tabButtons.contains(0) && m_tabButtons[0]->isChecked())
                     ? m_schemeCategories : m_solverCategories;

    for (const auto &c : cats) {
        QString val = m_userChoices.value(c.name, c.defaultScheme);
        if (c.name == "solvers") {
            cfg += "solvers\n{\n    p\n    {\n        solver          PCG;\n"
                   "        preconditioner  DIC;\n        tolerance       1e-6;\n"
                   "        relTol          0.01;\n    }\n\n"
                   "    U\n    {\n        solver          PBiCGStab;\n"
                   "        preconditioner  DILU;\n        tolerance       1e-6;\n"
                   "        relTol          0.0;\n    }\n"
                   "    \"(k|epsilon|omega|nut|nuTilda|T|alphat).*\"\n    {\n"
                   "        solver          PBiCGStab;\n"
                   "        preconditioner  DILU;\n        tolerance       1e-6;\n"
                   "        relTol          0.0;\n    }\n}\n\n";
        } else if (c.name == "PIMPLE / PISO / SIMPLE") {
            cfg += "PIMPLE\n{\n    nOuterCorrectors    1;\n"
                   "    nCorrectors         3;\n"
                   "    nNonOrthogonalCorrectors 0;\n"
                   "    momentumPredictor   yes;\n}\n\n";
        } else if (c.name == "relaxationFactors") {
            cfg += "relaxationFactors\n{\n    equations { \".*\" 1; }\n}\n\n";
        } else if (c.name == "preconditioners" || c.name == "smoothes") {
            // Handled within solvers block
            cfg += QString("// %1 — configured within solver entries\n\n").arg(c.name);
        } else {
            cfg += c.name + "\n{\n    default         " + val + ";\n}\n\n";
        }
    }
    return cfg.trimmed();
}

QString SchemesPanel::buildSingleCategorySnippet(const SchemeCategory &cat,
                                                   const QString &chosenOption) const
{
    QString snip;
    if (cat.name == "solvers") {
        snip = "solvers\n{\n    // solver configuration\n}\n";
    } else if (cat.name == "PIMPLE / PISO / SIMPLE") {
        snip = "PIMPLE\n{\n    nOuterCorrectors    1;\n"
               "    nCorrectors         3;\n    nNonOrthogonalCorrectors 0;\n"
               "    momentumPredictor   yes;\n}\n";
    } else if (cat.name == "relaxationFactors") {
        snip = "relaxationFactors\n{\n    equations { \".*\" 1; }\n}\n";
    } else if (cat.name == "preconditioners" || cat.name == "smoothes") {
        snip = ""; // part of solver entries
    } else {
        snip = cat.name + "\n{\n    default         " + chosenOption + ";\n}\n";
    }
    return snip;
}

void SchemesPanel::onApplySnippet()
{
    if (!m_editor) return;
    onRefreshPreview();
    QString snip = m_previewEdit->toPlainText();
    QTextCursor c = m_editor->textCursor();
    c.insertText(snip);
    m_editor->setTextCursor(c);
    QApplication::clipboard()->setText(snip);
    m_applyBtn->setText("✓ Applied!");
    QTimer::singleShot(1500, this, [this]() { m_applyBtn->setText("Apply to Editor"); });
}
