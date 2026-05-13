#include "snappypanel.h"
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
#include <QMenu>
#include <QScrollBar>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QComboBox>
#include <QtMath>

SnappyPanel::SnappyPanel(QWidget *parent) : QWidget(parent) { setupUI(); initData(); }

void SnappyPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0); root->setSpacing(0);

    auto *hdr = new QFrame();
    hdr->setStyleSheet("QFrame#snappyHdr { background: #F0F0F5; border-bottom: 2px solid #8E44AD; padding: 6px 10px; }");
    hdr->setObjectName("snappyHdr");
    auto *hl = new QHBoxLayout(hdr); hl->setContentsMargins(8, 6, 8, 6);
    auto *ico = new QLabel("M"); ico->setFixedSize(26, 26); ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background: #8E44AD; color: white; border-radius: 13px; font-weight: bold; font-size: 13px;");
    hl->addWidget(ico);
    m_headerLabel = new QLabel("No snappyHexMeshDict selected");
    m_headerLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #333;");
    hl->addWidget(m_headerLabel, 1);
    root->addWidget(hdr);

    m_pathLabel = new QLabel();
    m_pathLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px 10px; background: #FAFAFA;");
    root->addWidget(m_pathLabel);

    auto *ms = new QSplitter(Qt::Horizontal); ms->setChildrenCollapsible(false);

    auto *lp = new QWidget(); auto *ll = new QVBoxLayout(lp); ll->setContentsMargins(4, 2, 2, 2);
    m_sectionList = new QListWidget();
    m_sectionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_sectionList->setStyleSheet(
        "QListWidget { font-size: 12px; border: 1px solid #DDD; border-radius: 3px; background: white; }"
        "QListWidget::item { padding: 5px 8px; }"
        "QListWidget::item:selected { background: #8E44AD; color: white; }"
        "QListWidget::item:hover:!selected { background: #F0E6F5; }");
    ll->addWidget(m_sectionList);
    ms->addWidget(lp);

    auto *rp = new QWidget(); auto *rl = new QVBoxLayout(rp); rl->setContentsMargins(6, 4, 6, 4); rl->setSpacing(4);

    m_sectionDesc = new QLabel();
    m_sectionDesc->setWordWrap(true);
    m_sectionDesc->setStyleSheet("color: #555; font-size: 11px; padding: 4px 6px; background: #FFFFF5; border: 1px solid #F0E0B0; border-radius: 3px;");
    rl->addWidget(m_sectionDesc);

    auto *ph = new QLabel("Parameters (right-click to insert):");
    ph->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(ph);

    m_paramTree = new QTreeWidget();
    m_paramTree->setHeaderLabels({"Parameter", "Type", "Default (double-click to edit)", "Description"});
    m_paramTree->setRootIsDecorated(false);
    m_paramTree->setAlternatingRowColors(true);
    m_paramTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_paramTree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_paramTree->setMinimumHeight(150);
    m_paramTree->setStyleSheet(
        "QTreeWidget { font-size: 11px; border: 1px solid #DDD; border-radius: 3px; }"
        "QTreeWidget::item { padding: 2px 4px; }"
        "QTreeWidget::item:selected { background: #8E44AD; color: white; }"
        "QHeaderView::section { background: #EEE; padding: 3px 6px; font-size: 10px; font-weight: bold; border: none; }");
    rl->addWidget(m_paramTree, 2);

    auto *exHdr = new QLabel("Sample Block (right-click section to insert):");
    exHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(exHdr);

    m_previewEdit = new QTextEdit();
    m_previewEdit->setReadOnly(true);
    m_previewEdit->setFont(QFont("Consolas", 10));
    m_previewEdit->setMaximumHeight(120);
    m_previewEdit->setStyleSheet(
        "QTextEdit { background: #1E1E1E; color: #DCDCDC; border: 1px solid #333; border-radius: 3px; padding: 6px; }");
    rl->addWidget(m_previewEdit);

    ms->addWidget(rp); ms->setStretchFactor(0, 30); ms->setStretchFactor(1, 70);
    root->addWidget(ms, 1);

    connect(m_sectionList, &QListWidget::currentItemChanged, this, &SnappyPanel::onSectionChanged);
    connect(m_paramTree, &QTreeWidget::customContextMenuRequested, this, &SnappyPanel::onParamContextMenu);
    connect(m_paramTree, &QTreeWidget::itemChanged, this, &SnappyPanel::onParamEdited);
    connect(m_sectionList, &QListWidget::customContextMenuRequested, this, &SnappyPanel::onSectionContextMenu);
}

// ════════════════════════════════════════════════════════════════════
// Data — all snappyHexMesh parameters from OpenFOAM v2206 source
// ════════════════════════════════════════════════════════════════════

void SnappyPanel::initData()
{
    m_sections.append({
        "Top-Level Switches",
        "Master controls for the three snappyHexMesh stages.",
        {},
        "castellatedMesh true;\nsnap            true;\naddLayers       true;\n"
    });
    m_sections.last().params = {
        {"castellatedMesh", "bool", "true", "Generate castellated (refined Cartesian) mesh"},
        {"snap", "bool", "true", "Snap mesh vertices to geometry surfaces"},
        {"addLayers", "bool", "true", "Add boundary layers to snapped mesh"},
    };

    m_sections.append({
        "geometry",
        "Definition of all searchable surfaces (STL, OBJ, box, sphere, etc.) for refinement, snapping, and layers.\n"
        "Each surface has a name, a type, and optionally a file.",
        {},
        "geometry\n{\n    mySurface.stl\n    {\n        type triSurfaceMesh;\n        name mySurface;\n    }\n}\n"
    });
    m_sections.last().params = {
        {"<surfaceName>", "dict", "{}", "Surface dictionary with type and file/parameters"},
        {"type", "keyword", "triSurfaceMesh", "Surface type: triSurfaceMesh / searchableBox / searchableSphere / searchableCylinder / searchablePlane"},
        {"file", "fileName", "", "Path to geometry file (.stl, .obj)"},
        {"name", "word", "", "Optional name override"},
        {"scale", "scalar", "1.0", "Optional scaling factor for the surface"},
    };

    m_sections.append({
        "castellatedMeshControls",
        "Settings for the castellated (refinement) mesh generation stage.\n"
        "Controls cell count limits, refinement levels per surface/region, and mesh selection.",
        {},
        "castellatedMeshControls\n{\n    maxLocalCells 1000000;\n    maxGlobalCells 2000000;\n"
        "    minRefinementCells 100;\n    nCellsBetweenLevels 1;\n"
        "    locationInMesh (0 0 0);\n"
        "    refinementSurfaces { ... }\n    refinementRegions { ... }\n}\n"
    });
    m_sections.last().params = {
        {"maxLocalCells", "label", "1000000", "Max cells per processor before load balancing"},
        {"maxGlobalCells", "label", "2000000", "Overall cell limit. Refinement stops upon reaching this."},
        {"minRefinementCells", "label", "100", "Stop refining if <= this many cells selected."},
        {"nCellsBetweenLevels", "label", "1", "Buffer layers between refinement levels (1 = 2:1). Larger = slower."},
        {"locationInMesh", "vector", "(0 0 0)", "Point inside mesh region to keep. NEVER on a face."},
        {"allowFreeStandingZoneFaces", "bool", "true", "Allow free-standing zone faces."},
        {"resolveFeatureAngle", "scalar", "60", "Angle above which to resolve feature edges (degrees)."},
        {"features", "list", "()", "Feature edge refinement list."},
        {"refinementSurfaces", "dict", "{}", "Per-surface refinement levels (mode: distance/inside/outside)."},
        {"refinementRegions", "dict", "{}", "Per-region refinement levels (mode: inside/outside/distance)."},
    };

    m_sections.append({
        "refinementSurfaces (sub-dict)",
        "Specifies refinement level for cells intersecting a surface. Three modes:\n"
        "  - distance: levels per distance band (descending order)\n"
        "  - inside: all cells inside the closed surface\n"
        "  - outside: all cells outside the closed surface",
        {},
        "refinementSurfaces\n{\n    myPatch\n    {\n        level (0 0);\n"
        "        regions { ... }\n    }\n}\n"
    });
    m_sections.last().params = {
        {"level", "list", "(0 0)", "Refinement level: (minLevel maxLevel)"},
        {"mode", "keyword", "distance", "Refinement mode: distance / inside / outside"},
        {"levels", "list", "((1.0 2))", "Distance-mode: ((distance level) ...) in descending order"},
        {"patchInfo { type wall; }", "dict", "{}", "Optional patch type override"},
        {"faceZone", "word", "", "Optional face zone for this surface"},
        {"cellZone", "word", "", "Optional cell zone for this surface"},
        {"cellZoneInside", "keyword", "inside", "Method: inside / outside / insidePoint"},
    };

    m_sections.append({
        "snapControls",
        "Settings for the snapping stage. Moves mesh vertices onto geometry surfaces.\n"
        "Iterative relaxation process with mesh quality control.",
        {},
        "snapControls\n{\n    nSmoothPatch 3;\n    tolerance 4.0;\n"
        "    nSolveIter 30;\n    nRelaxIter 5;\n}\n"
    });
    m_sections.last().params = {
        {"nSmoothPatch", "label", "3", "Number of patch smoothing iterations before snapping."},
        {"tolerance", "scalar", "4.0", "Relative tolerance for point attraction (× local max edge length)."},
        {"nSolveIter", "label", "30", "Number of mesh displacement relaxation iterations per snap."},
        {"nRelaxIter", "label", "5", "Max number of snapping relaxation iterations."},
        {"nFeatureSnapIter", "label", "10", "Max iterations for feature edge snapping."},
        {"implicitFeatureSnap", "bool", "false", "Use implicit feature snapping (detects features automatically)."},
        {"explicitFeatureSnap", "bool", "true", "Use explicit feature snapping (uses feature edges)."},
        {"multiRegionFeatureSnap", "bool", "false", "Snap across multiple regions."},
    };

    m_sections.append({
        "addLayersControls",
        "Settings for boundary layer addition. Extrudes near-wall cells into prism layers.\n"
        "Supports relative/absolute sizing, per-patch specification, and mesh quality controls.",
        {},
        "addLayersControls\n{\n    relativeSizes false;\n    expansionRatio 1.0;\n"
        "    finalLayerThickness 0.3;\n    minThickness 0.25;\n"
        "    nGrow 0;\n    featureAngle 130;\n    nLayerIter 50;\n    layers { ... }\n}\n"
    });
    m_sections.last().params = {
        {"relativeSizes", "bool", "false", "Thickness relative to undistorted cell size (true) or absolute (false)."},
        {"expansionRatio", "scalar", "1.0", "Expansion ratio between consecutive layers (>1 = growing, <1 = shrinking)."},
        {"finalLayerThickness", "scalar", "0.3", "Thickness of layer furthest from wall (if relativeSizes = relative)."},
        {"firstLayerThickness", "scalar", "-", "ALTERNATIVE: thickness of first layer (wall-adjacent)."},
        {"thickness", "scalar", "-", "ALTERNATIVE: overall thickness of all layers combined."},
        {"minThickness", "scalar", "0.25", "Minimum overall thickness. Layer below this → don't add."},
        {"nGrow", "label", "0", "Number of connected face growth steps for uncovered faces."},
        {"featureAngle", "scalar", "130", "Max angle (degrees) for layer extrusion. 0=flat, 90=perpendicular."},
        {"slipFeatureAngle", "scalar", "30", "Max angle for mesh slip at non-patched sides."},
        {"maxFaceThicknessRatio", "scalar", "0.5", "Stop layer on highly warped cells."},
        {"nSmoothSurfaceNormals", "label", "1", "Smoothing iterations for surface normals."},
        {"nSmoothThickness", "label", "10", "Smoothing iterations for layer thickness."},
        {"nRelaxIter", "label", "5", "Max snapping relaxation iterations during layer addition."},
        {"nBufferCellsNoExtrude", "label", "0", "Buffer cells created for new layer terminations."},
        {"nLayerIter", "label", "50", "Overall max layer addition iterations."},
        {"nRelaxedIter", "label", "20", "After this iteration, use relaxed meshQualityControls."},
        {"layers", "dict", "{}", "Per-patch layer specification (nSurfaceLayers, expansionRatio, etc.)."},
    };

    m_sections.append({
        "meshQualityControls",
        "Generic mesh quality settings. Used at every undoable phase to determine mesh validity.\n"
        "The 'relaxed' sub-dict (optional) provides relaxed criteria after nRelaxedIter iterations.",
        {},
        "meshQualityControls\n{\n    maxNonOrtho 65;\n    maxBoundarySkewness 20;\n"
        "    maxInternalSkewness 4;\n    maxConcave 80;\n"
        "    minVol 1e-13;\n    minTetVol 1e-20;\n    minArea -1;\n"
        "    minTwist 0.05;\n    minDeterminant 1;\n    minFaceWeight 0.05;\n"
        "    minVolRatio 0.01;\n    nSmoothScale 4;\n    errorReduction 0.75;\n}\n"
    });
    m_sections.last().params = {
        {"maxNonOrtho", "scalar", "65", "Maximum non-orthogonality (degrees). 180 = disabled."},
        {"maxBoundarySkewness", "scalar", "20", "Maximum boundary face skewness. <0 = disabled."},
        {"maxInternalSkewness", "scalar", "4", "Maximum internal face skewness."},
        {"maxConcave", "scalar", "80", "Max concavity allowed (degrees). 180 = disabled."},
        {"minVol", "scalar", "1e-13", "Minimum cell pyramid volume. Large negative = disabled."},
        {"minTetVol", "scalar", "1e-20", "Minimum tet (face-centre decomposition) volume."},
        {"minTetQuality", "scalar", "-1", "Minimum tet quality criterion."},
        {"minArea", "scalar", "-1", "Minimum face area. <0 = disabled."},
        {"minTwist", "scalar", "0.05", "Minimum face twist (dot product of normals). <-1 = disabled."},
        {"minDeterminant", "scalar", "1", "Minimum normalised cell determinant. 1=hex, <=0=illegal."},
        {"minFaceWeight", "scalar", "0.05", "Minimum face weight (0 to 0.5)."},
        {"minVolRatio", "scalar", "0.01", "Minimum volume ratio (0 to 1)."},
        {"minTriangleTwist", "scalar", "-1", "Minimum triangle twist (>0 for Fluent)."},
        {"nSmoothScale", "label", "4", "Number of error distribution iterations."},
        {"errorReduction", "scalar", "0.75", "Amount to scale back displacement at error points."},
    };

    m_sections.append({
        "Advanced / Debug",
        "Developer and debugging options.",
        {},
        "debug 0;\nmergeTolerance 1E-6;\n"
    });
    m_sections.last().params = {
        {"debug", "label", "0", "Output flags: 0=final only, 1=intermediate, 2=cellLevel field, 4=.obj files."
         " Combine with bitwise OR (e.g., 5 = 1+4)."},
        {"mergeTolerance", "scalar", "1E-6", "Merge tolerance (fraction of bounding box). Write tolerance must be higher."},
        {"maxIterations", "label", "300", "Maximum number of total iterations across all stages."},
        {"nSmoothNormals", "label", "3", "Number of normal smoothing iterations after all stages."},
    };
}

// ════════════════════════════════════════════════════════════════════
// Load
// ════════════════════════════════════════════════════════════════════

void SnappyPanel::loadFile(const QString &filePath, const QString &)
{
    m_filePath = filePath;
    QFileInfo fi(filePath);
    m_headerLabel->setText(QString("<b>%1</b> — snappyHexMesh Mesh Generator").arg(fi.fileName()));
    m_pathLabel->setText(filePath);

    m_sectionList->clear();
    for (const auto &s : m_sections) {
        auto *item = new QListWidgetItem(s.name);
        item->setData(Qt::UserRole, s.name);
        m_sectionList->addItem(item);
    }
    if (!m_sections.isEmpty()) {
        m_sectionList->setCurrentRow(0);
        showSection(m_sections.first());
    }
}

void SnappyPanel::clear()
{
    m_headerLabel->setText("No snappyHexMeshDict selected");
    m_pathLabel->clear();
    m_sectionList->clear();
    m_paramTree->clear();
    m_sectionDesc->clear();
    m_previewEdit->clear();
}

void SnappyPanel::onSectionChanged(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    QString name = item->data(Qt::UserRole).toString();
    for (const auto &s : m_sections) {
        if (s.name == name) { showSection(s); return; }
    }
}

void SnappyPanel::showSection(const ShmSection &section)
{
    m_currentSection = section;
    m_sectionDesc->setText(section.description);

    m_paramTree->clear();
    for (const auto &p : section.params) {
        auto *item = new QTreeWidgetItem(m_paramTree);
        item->setText(0, p.name);
        item->setText(1, p.type);

        // Use user-edited value if available, otherwise default
        QString val = m_userValues.value(p.name, p.defaultValue);
        item->setText(2, val);

        item->setText(3, p.description);
        item->setToolTip(0, p.description);
        item->setToolTip(3, p.description);

        // Make item editable (Qt 6: flags apply to all columns, only col 2 matters)
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    m_paramTree->resizeColumnToContents(0);
    m_paramTree->resizeColumnToContents(1);
    m_paramTree->resizeColumnToContents(2);

    updateSampleBlock();
}

void SnappyPanel::onParamEdited(QTreeWidgetItem *item, int column)
{
    if (column != 2 || !item) return;
    QString paramName = item->text(0);
    QString newVal = item->text(2).trimmed();
    m_userValues[paramName] = newVal;
    updateSampleBlock();
}

void SnappyPanel::updateSampleBlock()
{
    // Generate a preview using the current parameter values
    QString block;
    for (const auto &p : m_currentSection.params) {
        QString val = m_userValues.value(p.name, p.defaultValue);
        if (p.name.startsWith("<")) continue; // skip placeholder names
        block += QString("%1   %2;\n").arg(p.name, -28).arg(val);
    }
    m_previewEdit->setPlainText(block.trimmed().isEmpty()
                                ? m_currentSection.sampleBlock : block);
}

// ────────────────────────────────────────────────────────────────────
// Context menus
// ────────────────────────────────────────────────────────────────────

void SnappyPanel::onParamContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_paramTree->itemAt(pos);
    if (!item) return;

    QString name = item->text(0);
    // Use user-edited value if available, otherwise the tree cell value
    QString defVal = m_userValues.value(name, item->text(2));

    QMenu menu;
    // 1) Value only
    QAction *valAct = menu.addAction(QString("Insert \"%1\" (value only)").arg(name));
    connect(valAct, &QAction::triggered, [this, name]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor();
        c.insertText(name);
        m_editor->setTextCursor(c);
    });
    // 2) Parameter with current value
    QAction *paramAct = menu.addAction(
        QString("Insert \"%1   %2;\" (with current value)").arg(name, defVal));
    connect(paramAct, &QAction::triggered, [this, name, defVal]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor();
        c.insertText(QString("%1   %2;\n").arg(name, defVal));
        m_editor->setTextCursor(c);
    });
    menu.exec(m_paramTree->mapToGlobal(pos));
}

void SnappyPanel::onSectionContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_sectionList->itemAt(pos);
    if (!item) return;

    QString secName = item->data(Qt::UserRole).toString();
    QString block;
    for (const auto &s : m_sections) {
        if (s.name == secName) { block = s.sampleBlock; break; }
    }
    if (block.isEmpty()) return;

    QMenu menu;
    QAction *act = menu.addAction(QString("Insert \"%1\" block").arg(secName));
    connect(act, &QAction::triggered, [this, block]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor();
        c.insertText(block);
        m_editor->setTextCursor(c);
        QApplication::clipboard()->setText(block);
    });
    menu.exec(m_sectionList->mapToGlobal(pos));
}

// ── Boundary Layer Calculator (y+ → addLayersControls) ──
void SnappyPanel::onCalcBL()
{
    auto *dlg = new QDialog(nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle("BL Calculator — y+ → addLayersControls");
    dlg->resize(450, 420);
    auto *l = new QVBoxLayout(dlg);

    auto *form = new QFormLayout();
    auto *yPlus = new QDoubleSpinBox(); yPlus->setRange(0.1, 5000); yPlus->setValue(30);
    yPlus->setDecimals(1); yPlus->setSuffix(" (1 = laminar-like, 30 = log-layer, 100+ = wall-func)");
    form->addRow("Target y+:", yPlus);

    auto *Uref = new QDoubleSpinBox(); Uref->setRange(0.01, 500); Uref->setValue(10);
    Uref->setDecimals(2); Uref->setSuffix(" m/s (freestream velocity)");
    form->addRow("Freestream U:", Uref);

    auto *Lref = new QDoubleSpinBox(); Lref->setRange(0.001, 100); Lref->setValue(1);
    Lref->setDecimals(3); Lref->setSuffix(" m (plate length or chord)");
    form->addRow("Reference L:", Lref);

    auto *rhoVal = new QDoubleSpinBox(); rhoVal->setRange(0.01, 2000); rhoVal->setValue(1.225);
    rhoVal->setDecimals(3); rhoVal->setSuffix(" kg/m³ (air=1.225, water=998)");
    form->addRow("Density ρ:", rhoVal);

    auto *nuVal = new QDoubleSpinBox(); nuVal->setRange(1e-7, 1e-3); nuVal->setValue(1.5e-5);
    nuVal->setDecimals(8); nuVal->setSuffix(" m²/s (air=1.5e-5, water=1e-6)");
    nuVal->setSingleStep(1e-6);
    form->addRow("Kinematic viscosity ν:", nuVal);

    auto *nLayers = new QSpinBox(); nLayers->setRange(1, 50); nLayers->setValue(5);
    nLayers->setSuffix(" layers");
    form->addRow("Number of layers:", nLayers);

    auto *expRatio = new QDoubleSpinBox(); expRatio->setRange(1.0, 2.0); expRatio->setValue(1.2);
    expRatio->setDecimals(2); expRatio->setSingleStep(0.05);
    expRatio->setSuffix(" (1.1-1.3 recommended)");
    form->addRow("Expansion ratio:", expRatio);

    auto *resultLbl = new QLabel();
    resultLbl->setStyleSheet("QLabel { background: #F5F0F5; padding: 8px; border-radius: 3px; "
                             "font-family: Consolas; font-size: 10px; }");
    resultLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    l->addLayout(form);
    l->addWidget(resultLbl);

    // ── Theory / derivation — opens in separate dialog ──
    auto *showTheoryBtn = new QPushButton("📖 Show Derivation & Physics...");
    showTheoryBtn->setStyleSheet("QPushButton { text-align:left; border:1px solid #E0D0F0; "
                                 "background:#FAF5FF; color:#8E44AD; font-size:10px; "
                                 "font-weight:bold; padding:3px 8px; border-radius:3px; }"
                                 "QPushButton:hover { background:#F0E0FF; }");
    l->addWidget(showTheoryBtn);

    connect(showTheoryBtn, &QPushButton::clicked, [this]() {
        QDialog *td = new QDialog(nullptr); // independent window
        td->setWindowTitle("BL Derivation & Physics");
        td->resize(750, 620);
        td->setMinimumSize(500, 400);
        td->setAttribute(Qt::WA_DeleteOnClose);
        auto *tl = new QVBoxLayout(td);
        tl->setContentsMargins(0, 0, 0, 0);

        auto *te = new QTextEdit();
        te->setReadOnly(true);
        te->setFont(QFont("Segoe UI", 10));
        te->setStyleSheet("QTextEdit { background:#FFFFFF; border:1px solid #E0E0E0; "
                          "border-radius:4px; padding:12px; color:#333; line-height:1.6; }");
        te->setHtml(
            "<h2 style='color:#8E44AD;'>Calculation Chain — y⁺ → firstLayerThickness</h2>"
            "<p><i>Prandtl–von Kármán turbulent boundary layer theory</i></p>"

            "<h3 style='color:#555;'>Step 1 — Reynolds Number</h3>"
            "<p><b>Re = U·L / ν</b></p>"
            "<p>Ratio of inertial to viscous forces. Determines laminar (Re < 5×10⁵) "
            "or turbulent (Re > 5×10⁵) boundary layer state.</p>"

            "<h3 style='color:#555;'>Step 2 — Skin Friction Coefficient</h3>"
            "<p><b>Cf = 0.027 / Re^(1/7)</b></p>"
            "<p><b>Origin:</b> The 1/7th power-law velocity profile <i>u/U = (y/δ)^(1/7)</i> "
            "was discovered experimentally by Prandtl and von Kármán (1920s). "
            "It fits turbulent flat-plate data well for 5×10⁵ < Re < 10⁷.</p>"
            "<p><b>Derivation:</b></p>"
            "<ol>"
            "<li>Insert <i>u/U = (y/δ)^(1/7)</i> into Kármán momentum integral: "
            "<i>C<sub>f</sub>/2 = dθ/dx</i></li>"
            "<li>Momentum thickness: <i>θ = ∫₀ᵟ(u/U)(1−u/U)dy = (7/72)δ</i></li>"
            "<li>Wall shear from Blasius pipe-flow analogy: "
            "<i>C<sub>f</sub>/2 = 0.0225·Re<sub>δ</sub><sup>−1/4</sup></i></li>"
            "<li>Turbulent BL growth: <i>δ/x = 0.37·Re<sub>x</sub><sup>−1/5</sup></i></li>"
            "<li>Eliminate δ → <i>Cf = 0.0276·Re<sub>x</sub><sup>−1/7</sup></i></li>"
            "</ol>"
            "<p><b>Note:</b> The exponent 1/7 comes directly from the velocity profile assumption. "
            "Alternative: Cf = 0.0592/Re^(1/5) (Prandtl-Schlichting, wider Re range).</p>"

            "<h3 style='color:#555;'>Step 3 — Wall Shear Stress</h3>"
            "<p><b>τ<sub>w</sub> = ½ ρ U² · Cf</b></p>"
            "<p>This is the <b>definition</b> of Cf, not an empirical formula:</p>"
            "<p style='background:#F5F0FF; padding:8px;'><i>"
            "C<sub>f</sub> ≡ τ<sub>w</sub> / (½ρU²) → τ<sub>w</sub> = ½ρU²·C<sub>f</sub></i></p>"
            "<p>τ<sub>w</sub> [Pa] is the force per unit area the fluid exerts parallel to the wall. "
            "Cf ≈ 0.003–0.005 for typical turbulent flows.</p>"

            "<h3 style='color:#555;'>Step 4 — Friction Velocity</h3>"
            "<p><b>u<sub>τ</sub> = √(τ<sub>w</sub> / ρ)</b></p>"
            "<p>Definition of the <b>friction velocity</b> — the characteristic velocity "
            "scale of near-wall turbulence. Named because it has dimensions of velocity "
            "but is derived from wall shear, not from the mean flow.</p>"
            "<p>Used to non-dimensionalise near-wall flow: <i>y⁺ = y·u<sub>τ</sub>/ν, "
            "u⁺ = u/u<sub>τ</sub></i></p>"

            "<h3 style='color:#555;'>Step 5 — First Layer Thickness</h3>"
            "<p><b>y₁ = y⁺ · ν / u<sub>τ</sub></b></p>"
            "<p>Rearranged from the wall scaling law <i>y⁺ = y·u<sub>τ</sub>/ν</i>. "
            "Given a target y⁺ (user input), computes the dimensional wall distance "
            "needed for the first mesh cell center.</p>"
            "<p><b>y⁺ Guidelines:</b></p>"
            "<table border='1' cellpadding='4' cellspacing='0' style='border-collapse:collapse; font-size:9pt;'>"
            "<tr style='background:#F0E0FF;'><th>y⁺ Range</th><th>Wall Treatment</th><th>Solver</th></tr>"
            "<tr><td>y⁺ ≤ 1</td><td>Resolved to viscous sublayer (no wall functions)</td>"
            "<td>kOmegaSST, SpalartAllmaras (low-Re)</td></tr>"
            "<tr><td>y⁺ ≈ 30</td><td>Log-layer first cell (standard wall functions)</td>"
            "<td>kEpsilon, kOmega (high-Re)</td></tr>"
            "<tr><td>y⁺ 1–30</td><td>Buffer layer — avoid! Neither fully resolved nor fully modelled</td>"
            "<td>—</td></tr>"
            "<tr><td>y⁺ > 100</td><td>Coarse wall functions</td><td>kEpsilon (rough walls)</td></tr>"
            "</table>"

            "<h3 style='color:#555;'>Geometric Series (Multi-layer)</h3>"
            "<p>For <i>n</i> layers with expansion ratio <i>r</i>:</p>"
            "<p>Final layer: <i>y<sub>n</sub> = y₁ · r<sup>n−1</sup></i></p>"
            "<p>Total thickness: <i>Y = y₁ · (r<sup>n</sup> − 1) / (r − 1)</i></p>"

            "<h3 style='color:#555;'>Validation Check</h3>"
            "<p><b>δ<sub>99</sub> = 0.37·L / Re<sup>1/5</sup></b></p>"
            "<p>Turbulent BL nominal thickness (where u = 0.99U). "
            "Total layer thickness should be ~0.1–0.3 × δ<sub>99</sub>.</p>"

            "<hr>"
            "<h3 style='color:#555;'>Mapping to snappyHexMesh</h3>"
            "<table border='1' cellpadding='4' cellspacing='0' style='border-collapse:collapse; font-size:9pt;'>"
            "<tr style='background:#F0E0FF;'><th>Parameter</th><th>Value</th></tr>"
            "<tr><td>relativeSizes</td><td>false (use absolute dimensions)</td></tr>"
            "<tr><td>firstLayerThickness</td><td>= y₁ from Step 5</td></tr>"
            "<tr><td>expansionRatio</td><td>= r (user input, typically 1.1–1.3)</td></tr>"
            "<tr><td>finalLayerThickness</td><td>= y₁ × r^(n−1)</td></tr>"
            "<tr><td>minThickness</td><td>= y₁ × 0.1 (safety margin)</td></tr>"
            "<tr><td>nSurfaceLayers</td><td>= n (user input)</td></tr>"
            "</table>"

            "<hr>"
            "<p style='color:#888; font-size:9pt;'><b>References:</b><br>"
            "Schlichting, H. (1979). <i>Boundary-Layer Theory</i>, 7th ed., McGraw-Hill.<br>"
            "White, F.M. (2006). <i>Viscous Fluid Flow</i>, 3rd ed., McGraw-Hill.<br>"
            "Prandtl, L. (1927). Über den Reibungswiderstand strömender Luft. "
            "<i>Ergebnisse der Aerodynamischen Versuchsanstalt zu Göttingen</i>, III.</p>"
        );
        tl->addWidget(te, 1);

        auto *closeBtn = new QPushButton("Close");
        closeBtn->setStyleSheet("QPushButton { padding:6px 24px; }");
        tl->addWidget(closeBtn);
        connect(closeBtn, &QPushButton::clicked, td, &QDialog::accept);

        td->show();
    });

    auto calc = [=]() -> QString {
        double yp = yPlus->value();
        double U = Uref->value();
        double L = Lref->value();
        double nu = nuVal->value();
        double rho = rhoVal->value();
        double Re = U * L / nu;

        // Turbulent flat-plate skin friction
        // Cf = 0.027 / Re_x^(1/7) — valid for 5×10^5 < Re < 10^7 (1/7th power-law)
        // Cf = 0.0592 / Re_x^(1/5) — alternative for wider Re range
        double Cf_local = 0.027 / qPow(Re, 1.0 / 7.0);
        double tau_w = 0.5 * rho * U * U * Cf_local;
        double u_tau = qSqrt(tau_w / rho);
        double y_first = yp * nu / u_tau;
        int n = nLayers->value();
        double r = expRatio->value();

        // Geometric series: total = a * (r^n - 1) / (r - 1), final = a * r^(n-1)
        double totalThick = (qAbs(r - 1.0) < 1e-6) ? y_first * n
            : y_first * (qPow(r, n) - 1.0) / (r - 1.0);
        double finalThick = y_first * qPow(r, n - 1);
        double minThick = y_first * 0.1; // 10% of first layer

        // Estimated BL thickness for validation
        double delta99 = 0.37 * L / qPow(Re, 1.0 / 5.0);

        return QString(
            "Re_L = %1  (turbulent: %2)\n"
            "Cf (local at x=L) = %3\n"
            "τ_w = %4 Pa  |  u_τ = %5 m/s\n"
            "Estimated BL thickness δ_99 ≈ %6 m\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
            "First  layer  = %7 m  (%8 mm)  ← target y+ = %9\n"
            "Final  layer  = %10 m  (%11 mm)\n"
            "Total  layers = %12 m  (%13 mm)\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
            "addLayersControls\n{\n"
            "    relativeSizes false;\n"
            "    firstLayerThickness %14;\n"
            "    expansionRatio %15;\n"
            "    finalLayerThickness %16;\n"
            "    minThickness %17;\n"
            "    nGrow 0;\n"
            "    featureAngle 130;\n"
            "    nLayerIter 50;\n"
            "    nSmoothSurfaceNormals 1;\n"
            "    nSmoothThickness 10;\n"
            "    maxFaceThicknessRatio 0.5;\n"
            "    layers\n"
            "    {\n"
            "        \"(wall|WALL.*)\"\n"
            "        {\n"
            "            nSurfaceLayers %18;\n"
            "        }\n"
            "    }\n"
            "}\n"
        ).arg(Re, 0, 'g', 4)
         .arg((Re > 5e5) ? "yes" : "transitional — increase Re or U")
         .arg(Cf_local, 0, 'g', 4)
         .arg(tau_w, 0, 'g', 4).arg(u_tau, 0, 'g', 4)
         .arg(delta99, 0, 'g', 4)
         .arg(y_first, 0, 'g', 4).arg(y_first * 1000, 0, 'f', 3).arg(yp, 0, 'f', 1)
         .arg(finalThick, 0, 'g', 4).arg(finalThick * 1000, 0, 'f', 3)
         .arg(totalThick, 0, 'g', 4).arg(totalThick * 1000, 0, 'f', 3)
         .arg(y_first, 0, 'g', 4).arg(r, 0, 'f', 2).arg(finalThick, 0, 'g', 4)
         .arg(minThick, 0, 'g', 4).arg(n);
    };

    resultLbl->setText(calc());
    connect(yPlus, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });
    connect(Uref, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });
    connect(Lref, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });
    connect(rhoVal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });
    connect(nuVal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });
    connect(nLayers, QOverload<int>::of(&QSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });
    connect(expRatio, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { resultLbl->setText(calc()); });

    auto *btns = new QHBoxLayout();
    btns->addStretch();
    auto *cancel = new QPushButton("Close");
    auto *insert = new QPushButton("Insert addLayersControls");
    insert->setStyleSheet("QPushButton { background:#8E44AD; color:white; padding:6px 16px; "
                          "border:none; border-radius:3px; font-weight:bold; }");
    btns->addWidget(cancel); btns->addWidget(insert);
    l->addLayout(btns);
    connect(cancel, &QPushButton::clicked, dlg, &QDialog::close);
    connect(insert, &QPushButton::clicked, dlg, [this, resultLbl, dlg]() {
        QString newBlock = resultLbl->text();
        int blockStart = newBlock.indexOf("addLayersControls");
        if (blockStart < 0) return;
        newBlock = newBlock.mid(blockStart);

        if (!m_editor) { QApplication::clipboard()->setText(newBlock); dlg->close(); return; }

        QString content = m_editor->toPlainText();
        // Find existing addLayersControls block by brace counting
        int addIdx = content.indexOf("addLayersControls");
        if (addIdx >= 0) {
            int braceIdx = content.indexOf('{', addIdx);
            if (braceIdx >= 0) {
                int depth = 1, pos = braceIdx + 1;
                while (pos < content.size() && depth > 0) {
                    if (content[pos] == '{') depth++;
                    else if (content[pos] == '}') depth--;
                    pos++;
                }
                // Replace old block with new
                m_editor->selectAll();
                QString updated = content.left(addIdx) + newBlock +
                                  content.mid(pos);
                m_editor->textCursor().insertText(updated);
            }
        } else {
            // No existing block — append at end
            QTextCursor c = m_editor->textCursor();
            c.movePosition(QTextCursor::End);
            c.insertText("\n" + newBlock);
            m_editor->setTextCursor(c);
        }
        QApplication::clipboard()->setText(newBlock);
        dlg->close();
    });
    dlg->show();
}
