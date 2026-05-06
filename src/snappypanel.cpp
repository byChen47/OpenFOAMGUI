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
