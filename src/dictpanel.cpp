#include "dictpanel.h"
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

DictPanel::DictPanel(QWidget *parent) : QWidget(parent) { setupUI(); }

void DictPanel::setupUI()
{
    auto *root = new QVBoxLayout(this); root->setContentsMargins(0,0,0,0); root->setSpacing(0);
    auto *hdr = new QFrame();
    hdr->setStyleSheet("QFrame#dictHdr { background: #F5F0E8; border-bottom: 2px solid #E67E22; padding: 6px 10px; }");
    hdr->setObjectName("dictHdr");
    auto *hl = new QHBoxLayout(hdr); hl->setContentsMargins(8,6,8,6);
    auto *ico = new QLabel("D"); ico->setFixedSize(26,26); ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background: #E67E22; color: white; border-radius: 13px; font-weight: bold; font-size: 13px;");
    hl->addWidget(ico);
    m_headerLabel = new QLabel("No dictionary selected");
    m_headerLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #333;");
    hl->addWidget(m_headerLabel,1);
    root->addWidget(hdr);
    m_pathLabel = new QLabel();
    m_pathLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px 10px; background: #FAFAFA;");
    root->addWidget(m_pathLabel);

    auto *ms = new QSplitter(Qt::Horizontal); ms->setChildrenCollapsible(false);
    auto *lp = new QWidget(); auto *ll = new QVBoxLayout(lp); ll->setContentsMargins(4,2,2,2);
    m_sectionList = new QListWidget();
    m_sectionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_sectionList->setStyleSheet(
        "QListWidget { font-size: 12px; border: 1px solid #DDD; border-radius: 3px; background: white; }"
        "QListWidget::item { padding: 5px 8px; }"
        "QListWidget::item:selected { background: #E67E22; color: white; }"
        "QListWidget::item:hover:!selected { background: #FDE8D0; }");
    ll->addWidget(m_sectionList);
    ms->addWidget(lp);

    auto *rp = new QWidget(); auto *rl = new QVBoxLayout(rp); rl->setContentsMargins(6,4,6,4); rl->setSpacing(4);
    m_sectionDesc = new QLabel(); m_sectionDesc->setWordWrap(true);
    m_sectionDesc->setStyleSheet("color: #555; font-size: 11px; padding: 4px 6px; background: #FFFFF5; border: 1px solid #F0E0B0; border-radius: 3px;");
    rl->addWidget(m_sectionDesc);
    auto *ph = new QLabel("Parameters (right-click to insert, double-click Default to edit):");
    ph->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(ph);

    m_paramTree = new QTreeWidget();
    m_paramTree->setHeaderLabels({"Parameter", "Type", "Default", "Description"});
    m_paramTree->setRootIsDecorated(false);
    m_paramTree->setAlternatingRowColors(true);
    m_paramTree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_paramTree->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_paramTree->setMinimumHeight(150);
    m_paramTree->setStyleSheet(
        "QTreeWidget { font-size: 11px; border: 1px solid #DDD; border-radius: 3px; }"
        "QTreeWidget::item { padding: 2px 4px; }"
        "QTreeWidget::item:selected { background: #E67E22; color: white; }"
        "QHeaderView::section { background: #EEE; padding: 3px 6px; font-size: 10px; font-weight: bold; border: none; }");
    rl->addWidget(m_paramTree, 2);

    auto *exHdr = new QLabel("Sample Block (right-click section to insert):");
    exHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(exHdr);
    m_previewEdit = new QTextEdit(); m_previewEdit->setReadOnly(true);
    m_previewEdit->setFont(QFont("Consolas", 10)); m_previewEdit->setMaximumHeight(120);
    m_previewEdit->setStyleSheet("QTextEdit { background: #1E1E1E; color: #DCDCDC; border: 1px solid #333; border-radius: 3px; padding: 6px; }");
    rl->addWidget(m_previewEdit);

    ms->addWidget(rp); ms->setStretchFactor(0,30); ms->setStretchFactor(1,70);
    root->addWidget(ms,1);

    connect(m_sectionList, &QListWidget::currentItemChanged, this, &DictPanel::onSectionChanged);
    connect(m_paramTree, &QTreeWidget::customContextMenuRequested, this, &DictPanel::onParamContextMenu);
    connect(m_paramTree, &QTreeWidget::itemChanged, this, &DictPanel::onParamEdited);
    connect(m_sectionList, &QListWidget::customContextMenuRequested, this, &DictPanel::onSectionContextMenu);
}

// ════════════════════════════════════════════════════════════════════
// blockMeshDict data
// ════════════════════════════════════════════════════════════════════

void DictPanel::initBlockMeshData()
{
    m_blockMeshSections.append({
        "Top-Level",
        "Global settings for the block mesh.",
        {},
        "scale   1.0;\n\nvertices\n(\n);\n\nblocks\n(\n);\n\nedges\n(\n);\n\nboundary\n(\n);\n"
    });
    m_blockMeshSections.last().params = {
        {"scale", "scalar", "1.0", "Global scaling factor applied to all vertex coordinates."},
        {"convertToMeters", "scalar", "1.0", "Unit conversion factor (e.g., 0.001 for mm → m)."},
    };

    m_blockMeshSections.append({
        "vertices",
        "List of vertex coordinates (x y z). Minimum 8 vertices for a single hex block.\n"
        "Can use #calc expressions for parametric geometry.",
        {},
        "vertices\n(\n    (0 0 0)    // vertex 0\n    (2 0 0)    // vertex 1\n    ...\n);\n"
    });
    m_blockMeshSections.last().params = {
        {"(x y z)", "vector list", "(0 0 0)", "Vertex coordinate. Vertices are numbered starting at 0."},
    };

    m_blockMeshSections.append({
        "blocks",
        "Hexahedral block definitions. Each block = 8 vertices + cell counts + grading.\n"
        "Ordering: (v0 v1 v2 v3 v4 v5 v6 v7) following right-hand rule.",
        {},
        "blocks\n(\n    hex (0 1 5 4 12 13 17 16) (20 10 1) simpleGrading (1 1 1)\n);\n"
    });
    m_blockMeshSections.last().params = {
        {"hex", "keyword", "hex", "Block type (hex is the only option for blockMesh)."},
        {"(v0..v7)", "label list", "(0 1 2 3 4 5 6 7)", "8 vertex indices defining the hex block corners."},
        {"(nx ny nz)", "label list", "(20 10 1)", "Number of cells in x, y, z directions."},
        {"simpleGrading", "keyword", "simpleGrading", "Grading type: simpleGrading / edgeGrading."},
        {"(gx gy gz)", "scalar list", "(1 1 1)", "Cell expansion ratios (>1 = larger at end, <1 = smaller at end)."},
    };

    m_blockMeshSections.append({
        "edges",
        "Curved edge definitions. Connect vertices with arcs, splines, or lines.\n"
        "Used for non-planar geometry.",
        {},
        "edges\n(\n    arc  0 1 (0.5 -0.1 0)    // circular arc through point\n    spline 2 3 (...points...)    // spline through points\n);\n"
    });
    m_blockMeshSections.last().params = {
        {"arc", "keyword", "arc", "Circular arc edge: arc v0 v1 (interpolationPoint)."},
        {"spline", "keyword", "spline", "Spline edge: spline v0 v1 (p1 p2 ... pN)."},
        {"line", "keyword", "line", "Straight line edge: line v0 v1 (optional: interpolationPoints)."},
        {"polyLine", "keyword", "polyLine", "Polyline edge: polyLine v0 v1 (p1 p2 ...)."},
        {"BSpline", "keyword", "BSpline", "B-spline edge: BSpline v0 v1 (p1 p2 ...)."},
        {"project", "keyword", "project", "Project edge onto geometry surface."},
    };

    m_blockMeshSections.append({
        "boundary",
        "Patch definitions. Each patch = type + face list.\n"
        "Faces defined as: (v0 v1 v2 v3) for each face of each block.",
        {},
        "boundary\n(\n    inlet\n    {\n        type patch;\n        faces ((0 4 7 3));\n    }\n);\n"
    });
    m_blockMeshSections.last().params = {
        {"<patchName>", "dict", "{}", "Patch dictionary with type and faces list."},
        {"type", "keyword", "patch", "Patch type: patch / wall / symmetryPlane / empty / wedge / cyclic."},
        {"faces", "face list", "()", "List of block faces: (v0 v1 v2 v3) per face."},
    };

    m_blockMeshSections.append({
        "mergePatchPairs",
        "Optional: merge patch pairs for non-conformal block connections.",
        {},
        "mergePatchPairs\n(\n);\n"
    });
    m_blockMeshSections.last().params = {
        {"(master slave)", "patch pair", "()", "Pair of patch names to merge. Empty list = no merging."},
    };
}

// ════════════════════════════════════════════════════════════════════
// topoSetDict data
// ════════════════════════════════════════════════════════════════════

void DictPanel::initTopoSetData()
{
    m_topoSetSections.append({
        "Top-Level",
        "TopoSet performs cell/face/point set manipulations. Each 'action' creates or modifies a set.",
        {},
        "actions\n(\n    { name c0; type cellSet; action new; source regionToCell; insidePoints ((0 0 0)); }\n);\n"
    });
    m_topoSetSections.last().params = {
        {"actions", "list of dicts", "()", "List of set manipulation actions. Each is a sub-dictionary."},
    };

    m_topoSetSections.append({
        "Action Parameters",
        "Each action dictionary requires: name, type, action, and source-specific parameters.",
        {},
        "{\n    name    c0;\n    type    cellSet;\n    action  new;\n    source  regionToCell;\n    insidePoints ((0 0 0));\n}\n"
    });
    m_topoSetSections.last().params = {
        {"name", "word", "c0", "Name of the set being created/modified."},
        {"type", "keyword", "cellSet", "Set type: cellSet / faceSet / pointSet / cellZoneSet / faceZoneSet / pointZoneSet."},
        {"action", "keyword", "new", "Action: new / add / subtract / subset / invert / clear / remove / list."},
        {"source", "keyword", "regionToCell", "Source type for selection."},
    };

    m_topoSetSections.append({
        "Source Types (cellSet)",
        "Available sources for cell selection. Each source has its own parameters.",
        {},
        "source boxToCell;\nbox (0 0 -1)(1 1 1);\n"
    });
    m_topoSetSections.last().params = {
        {"boxToCell", "keyword", "", "Select cells inside a bounding box. Needs: box (min) (max)."},
        {"regionToCell", "keyword", "", "Select cells reachable from insidePoints. Needs: insidePoints ((x y z))."},
        {"sphereToCell", "keyword", "", "Select cells inside sphere. Needs: centre (x y z); radius r."},
        {"cylinderToCell", "keyword", "", "Select cells inside cylinder. Needs: p1, p2, radius."},
        {"cellToCell", "keyword", "", "Copy cells from another set. Needs: set <name>."},
        {"surfaceToCell", "keyword", "", "Select cells relative to surface. Needs: surface, outsidePoints."},
        {"rotatedBoxToCell", "keyword", "", "Select cells in rotated box. Needs: origin, i, j, k."},
        {"fieldToCell", "keyword", "", "Select cells by field value. Needs: field, min, max."},
        {"targetVolumeToCell", "keyword", "", "Select cells to meet target volume."},
        {"zoneToCell", "keyword", "", "Select cells in a cellZone."},
    };

    m_topoSetSections.append({
        "Source Types (faceSet / pointSet)",
        "Sources for face and point selection.",
        {},
        "source patchToFace;\npatch movingWall;\n"
    });
    m_topoSetSections.last().params = {
        {"patchToFace", "keyword", "", "Select faces on a patch. Needs: patch <name>."},
        {"faceToFace", "keyword", "", "Copy faces from another faceSet. Needs: set <name>."},
        {"normalToFace", "keyword", "", "Select faces with certain normal. Needs: normal (nx ny nz); cos angle."},
        {"labelToFace", "keyword", "", "Select faces by label range. Needs: min, max."},
        {"patchToPoint", "keyword", "", "Select points on a patch."},
        {"labelToPoint", "keyword", "", "Select points by label range."},
        {"surfaceToPoint", "keyword", "", "Select points near a surface."},
    };
}

// ════════════════════════════════════════════════════════════════════
// dynamicMeshDict data
// ════════════════════════════════════════════════════════════════════

void DictPanel::initDynamicMeshData()
{
    m_dynamicMeshSections.append({
        "Top-Level",
        "Dynamic mesh configuration. Selects the fvMesh class and motion solver.",
        {},
        "dynamicFvMesh    staticFvMesh;\n\nmotionSolverLibs (fvMotionSolvers);\n\nsolver    displacementLaplacian;\n"
    });
    m_dynamicMeshSections.last().params = {
        {"dynamicFvMesh", "keyword", "staticFvMesh", "fvMesh class: staticFvMesh / dynamicMotionSolverFvMesh / dynamicOversetFvMesh / dynamicRefineFvMesh / solidBodyMotionFvMesh / multiSolidBodyMotionFvMesh."},
        {"motionSolverLibs", "word list", "(fvMotionSolvers)", "Additional libraries for motion solvers."},
        {"solver", "keyword", "displacementLaplacian", "Motion solver: displacementLaplacian / velocityLaplacian / displacementComponentLaplacian / displacementInterpolation / solidBody."},
    };

    m_dynamicMeshSections.append({
        "Motion Solvers",
        "Available motion solvers and their configurations.",
        {},
        "solver    displacementLaplacian;\ndisplacementLaplacianCoeffs\n{\n    diffusivity    uniform 1;\n}\n"
    });
    m_dynamicMeshSections.last().params = {
        {"displacementLaplacian", "keyword", "", "Laplacian for displacement. Uses diffusivity for non-uniform mesh motion."},
        {"velocityLaplacian", "keyword", "", "Laplacian for velocity. Similar to displacementLaplacian."},
        {"solidBody", "keyword", "", "Solid-body motion (translation + rotation). No internal deformation."},
        {"multiSolidBodyMotionSolver", "keyword", "", "Multiple rigid bodies with individual motions."},
        {"displacementSBRStress", "keyword", "", "Solid-body rotation stress solver."},
        {"displacementInterpolation", "keyword", "", "Interpolation-based displacement from boundary motion."},
    };

    m_dynamicMeshSections.append({
        "Diffusivity Models",
        "Controls how mesh deformation is distributed through the domain.",
        {},
        "diffusivity    uniform 1;\n"
    });
    m_dynamicMeshSections.last().params = {
        {"uniform", "keyword", "uniform 1", "Uniform diffusivity. Good for simple motions."},
        {"directional", "keyword", "", "Direction-dependent diffusivity. Needs: diffusivity (dx dy dz)."},
        {"motionDirectional", "keyword", "", "Diffusivity based on motion direction."},
        {"inverseDistance", "keyword", "", "Inverse-distance from boundaries. Good for boundary-layer preserving."},
        {"inverseVolume", "keyword", "", "Inverse cell volume. Smaller cells get less motion."},
        {"file", "keyword", "", "Read diffusivity from a volScalarField."},
    };

    m_dynamicMeshSections.append({
        "Solid Body Motion",
        "Prescribed rigid-body motion (translation + rotation).",
        {},
        "solidBodyMotionFvMeshCoeffs\n{\n    motionFunction    rotatingMotion;\n    origin    (0 0 0);\n    axis    (0 0 1);\n    omega    10;\n}\n"
    });
    m_dynamicMeshSections.last().params = {
        {"motionFunction", "keyword", "rotatingMotion", "Motion type: rotatingMotion / linearMotion / oscillatingRotatingMotion / oscillatingLinearMotion / tabulated6DoFMotion / SDA (spherical angular damper)."},
        {"origin", "vector", "(0 0 0)", "Origin of rotation."},
        {"axis", "vector", "(0 0 1)", "Axis of rotation."},
        {"omega", "scalar", "0", "Angular velocity [rad/s]."},
        {"velocity", "vector", "(0 0 0)", "Linear velocity for linearMotion."},
        {"amplitude", "vector", "(0 0 0)", "Oscillation amplitude for oscillating motions."},
        {"period", "scalar", "1", "Oscillation period [s]."},
    };
}

// ════════════════════════════════════════════════════════════════════
// Load file
// ════════════════════════════════════════════════════════════════════

void DictPanel::loadFile(const QString &filePath, const QString &)
{
    m_filePath = filePath;
    m_userValues.clear();
    QFileInfo fi(filePath);
    m_fileType = fi.fileName();

    QString title;
    if (m_fileType == "blockMeshDict") {
        if (m_blockMeshSections.isEmpty()) initBlockMeshData();
        m_currentSections = &m_blockMeshSections;
        title = "blockMesh — Mesh Generator";
    } else if (m_fileType == "topoSetDict") {
        if (m_topoSetSections.isEmpty()) initTopoSetData();
        m_currentSections = &m_topoSetSections;
        title = "topoSet — Cell/Face/Point Set Manipulation";
    } else if (m_fileType == "dynamicMeshDict") {
        if (m_dynamicMeshSections.isEmpty()) initDynamicMeshData();
        m_currentSections = &m_dynamicMeshSections;
        title = "dynamicMesh — Dynamic Mesh Motion";
    } else if (m_fileType == "controlDict") {
        if (m_controlDictSections.isEmpty()) initControlDictData();
        m_currentSections = &m_controlDictSections;
        title = "controlDict — Simulation Time Control";
    } else if (m_fileType == "decomposeParDict") {
        if (m_decomposeParDictSections.isEmpty()) initDecomposeParDictData();
        m_currentSections = &m_decomposeParDictSections;
        title = "decomposePar — Parallel Decomposition";
    } else if (m_fileType == "refineMeshDict") {
        if (m_refineMeshDictSections.isEmpty()) initRefineMeshDictData();
        m_currentSections = &m_refineMeshDictSections;
        title = "refineMesh — Mesh Refinement";
    } else if (m_fileType == "transportProperties") {
        if (m_transportSections.isEmpty()) initTransportPropertiesData();
        m_currentSections = &m_transportSections;
        title = "Transport Properties — Fluid Rheology";
    } else if (m_fileType == "thermophysicalProperties") {
        if (m_thermoSections.isEmpty()) initThermophysicalData();
        m_currentSections = &m_thermoSections;
        title = "Thermophysical Properties — Material Model";
    } else if (m_fileType == "radiationProperties") {
        if (m_radiationSections.isEmpty()) initRadiationData();
        m_currentSections = &m_radiationSections;
        title = "Radiation Properties — Radiative Heat Transfer";
    } else if (m_fileType == "combustionProperties") {
        if (m_combustionSections.isEmpty()) initCombustionData();
        m_currentSections = &m_combustionSections;
        title = "Combustion Properties — Reaction Model";
    } else if (m_fileType == "sampleDict") {
        if (m_sampleDictSections.isEmpty()) initSampleDictData();
        m_currentSections = &m_sampleDictSections;
        title = "sampleDict — Data Sampling";
    } else if (m_fileType == "setFieldsDict") {
        if (m_setFieldsSections.isEmpty()) initSetFieldsData();
        m_currentSections = &m_setFieldsSections;
        title = "setFieldsDict — Field Initialization";
    } else if (m_fileType == "forces" || m_fileType == "forceCoeffs") {
        if (m_forcesSections.isEmpty()) initForcesData();
        m_currentSections = &m_forcesSections;
        title = "Forces/ForceCoeffs — Force Monitoring";
    } else if (m_fileType == "fvConstraints") {
        if (m_fvConstraintsSections.isEmpty()) initFvConstraintsData();
        m_currentSections = &m_fvConstraintsSections;
        title = "fvConstraints — Field Constraints";
    } else {
        return;
    }

    m_headerLabel->setText(QString("<b>%1</b> — %2").arg(fi.fileName(), title));
    m_pathLabel->setText(filePath);

    m_sectionList->clear();
    for (const auto &s : *m_currentSections) {
        m_sectionList->addItem(s.name);
    }
    if (!m_currentSections->isEmpty()) {
        m_sectionList->setCurrentRow(0);
        showSection(m_currentSections->first());
    }
}

void DictPanel::clear()
{
    m_headerLabel->setText("No dictionary selected");
    m_pathLabel->clear();
    m_sectionList->clear();
    m_paramTree->clear();
    m_sectionDesc->clear();
    m_previewEdit->clear();
    m_currentSections = nullptr;
}

void DictPanel::onSectionChanged(QListWidgetItem *item)
{
    if (!item || !m_currentSections) return;
    int idx = m_sectionList->row(item);
    if (idx >= 0 && idx < m_currentSections->size())
        showSection((*m_currentSections)[idx]);
}

void DictPanel::showSection(const DictSection &section)
{
    m_currentSection = section;
    m_sectionDesc->setText(section.description);

    m_paramTree->clear();
    for (const auto &p : section.params) {
        auto *item = new QTreeWidgetItem(m_paramTree);
        item->setText(0, p.name);
        item->setText(1, p.type);
        item->setText(2, m_userValues.value(p.name, p.defaultValue));
        item->setText(3, p.description);
        item->setToolTip(0, p.description);
        item->setToolTip(3, p.description);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    }
    m_paramTree->resizeColumnToContents(0);
    m_paramTree->resizeColumnToContents(1);
    m_paramTree->resizeColumnToContents(2);
    updateSampleBlock();
}

void DictPanel::onParamEdited(QTreeWidgetItem *item, int column)
{
    if (column != 2 || !item) return;
    m_userValues[item->text(0)] = item->text(2).trimmed();
    updateSampleBlock();
}

void DictPanel::updateSampleBlock()
{
    QString block;
    for (const auto &p : m_currentSection.params) {
        QString val = m_userValues.value(p.name, p.defaultValue);
        if (p.name.contains("(")) continue;
        block += QString("%1   %2;\n").arg(p.name, -28).arg(val);
    }
    m_previewEdit->setPlainText(block.trimmed().isEmpty()
                                ? m_currentSection.sampleBlock : block);
}

// ────────────────────────────────────────────────────────────────────
// Context menus
// ────────────────────────────────────────────────────────────────────

void DictPanel::onParamContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_paramTree->itemAt(pos);
    if (!item) return;
    QString name = item->text(0);
    QString defVal = m_userValues.value(name, item->text(2));

    QMenu menu;
    QAction *vAct = menu.addAction(QString("Insert \"%1\" (value only)").arg(name));
    connect(vAct, &QAction::triggered, [this, name]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor(); c.insertText(name); m_editor->setTextCursor(c);
    });
    QAction *pAct = menu.addAction(QString("Insert \"%1   %2;\" (with value)").arg(name, defVal));
    connect(pAct, &QAction::triggered, [this, name, defVal]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor();
        c.insertText(QString("%1   %2;\n").arg(name, defVal));
        m_editor->setTextCursor(c);
    });
    menu.exec(m_paramTree->mapToGlobal(pos));
}

void DictPanel::onSectionContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_sectionList->itemAt(pos);
    if (!item || !m_currentSections) return;
    int idx = m_sectionList->row(item);
    if (idx < 0 || idx >= m_currentSections->size()) return;
    QString block = (*m_currentSections)[idx].sampleBlock;
    if (block.isEmpty()) return;

    QMenu menu;
    QAction *act = menu.addAction(QString("Insert \"%1\" block").arg((*m_currentSections)[idx].name));
    connect(act, &QAction::triggered, [this, block]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor(); c.insertText(block); m_editor->setTextCursor(c);
        QApplication::clipboard()->setText(block);
    });
    menu.exec(m_sectionList->mapToGlobal(pos));
}

// ════════════════════════════════════════════════════════════════════
// controlDict data
// ════════════════════════════════════════════════════════════════════

void DictPanel::initControlDictData()
{
    m_controlDictSections.append({
        "Time Control",
        "Controls the simulation time range, time step, and start/stop behaviour.",
        {},
        "startFrom       startTime;\nstartTime       0;\nstopAt          endTime;\nendTime         1;\ndeltaT          0.001;\n"
    });
    m_controlDictSections.last().params = {
        {"application", "word", "interFoam", "Solver/application name."},
        {"startFrom", "keyword", "startTime", "Start criterion: firstTime / startTime / latestTime."},
        {"startTime", "scalar", "0", "Simulation start time [s]."},
        {"stopAt", "keyword", "endTime", "Stop criterion: endTime / writeNow / noWriteNow / nextWrite."},
        {"endTime", "scalar", "1", "Simulation end time [s]."},
        {"deltaT", "scalar", "0.001", "Time step size [s]. Used if adjustTimeStep = no."},
    };

    m_controlDictSections.append({
        "Write Control",
        "Controls when and how results are written to time directories.",
        {},
        "writeControl    adjustable;\nwriteInterval   0.05;\npurgeWrite      0;\nwriteFormat     ascii;\nwritePrecision  6;\nwriteCompression off;\n"
    });
    m_controlDictSections.last().params = {
        {"writeControl", "keyword", "adjustable", "Write trigger: timeStep / runTime / cpuTime / adjustable / clockTime."},
        {"writeInterval", "scalar", "0.05", "Write interval in units of writeControl."},
        {"purgeWrite", "label", "0", "Keep only N most recent time dirs. 0 = keep all."},
        {"writeFormat", "keyword", "ascii", "Format: ascii / binary."},
        {"writePrecision", "label", "6", "Number of significant digits for ascii output."},
        {"writeCompression", "keyword", "off", "Compress output: on / off / gzip."},
        {"timeFormat", "keyword", "general", "Time directory naming: fixed / scientific / general."},
        {"timePrecision", "label", "6", "Precision for time directory naming."},
        {"runTimeModifiable", "keyword", "yes", "Allow runtime modification of controlDict."},
    };

    m_controlDictSections.append({
        "Adaptive Time Stepping",
        "Settings for automatic time step adjustment based on Courant number.",
        {},
        "adjustTimeStep  yes;\nmaxCo           1;\nmaxAlphaCo      1;\nmaxDeltaT       1;\n"
    });
    m_controlDictSections.last().params = {
        {"adjustTimeStep", "keyword", "no", "Enable automatic time step adjustment: on/yes/true or off/no/false."},
        {"maxCo", "scalar", "1.0", "Maximum Courant number for time step adjustment."},
        {"maxAlphaCo", "scalar", "1.0", "Maximum interfacial Courant number (VOF multiphase)."},
        {"maxDeltaT", "scalar", "1.0", "Maximum allowable time step [s]."},
        {"maxDi", "scalar", "1.0", "Maximum diffusion number for time step adjustment."},
    };

    m_controlDictSections.append({
        "Functions",
        "Optional function objects executed at runtime (forces, probes, sampling, etc.).",
        {},
        "functions\n{\n    // function objects\n}\n"
    });
    m_controlDictSections.last().params = {
        {"functions", "dict", "{}", "Sub-dictionary of function objects for runtime post-processing."},
        {"libs", "word list", "()", "Additional libraries for function objects."},
    };
}

// ════════════════════════════════════════════════════════════════════
// decomposeParDict data
// ════════════════════════════════════════════════════════════════════

void DictPanel::initDecomposeParDictData()
{
    m_decomposeParDictSections.append({
        "Top-Level",
        "Domain decomposition for parallel execution. Selects method and number of subdomains.",
        {},
        "numberOfSubdomains 4;\nmethod          scotch;\n"
    });
    m_decomposeParDictSections.last().params = {
        {"numberOfSubdomains", "label", "4", "Number of processors / MPI ranks."},
        {"method", "keyword", "scotch", "Decomposition method: scotch / hierarchical / simple / metis / manual / multiLevel / none."},
    };

    m_decomposeParDictSections.append({
        "hierarchical Coefficients",
        "Hierarchical (xyz) decomposition. Good for structured meshes.\n"
        "Splits domain along x, then y, then z recursively.",
        {},
        "method hierarchical;\nhierarchicalCoeffs\n{\n    n (2 2 1);\n}\n"
    });
    m_decomposeParDictSections.last().params = {
        {"n", "vector label", "(2 2 1)", "Number of subdomains in (x y z) directions. Product = numberOfSubdomains."},
        {"order", "keyword", "xyz", "Splitting order: xyz / xzy / yxz / ..."},
        {"delta", "scalar", "0.001", "Perturbation for point location (avoids degenerate splits)."},
    };

    m_decomposeParDictSections.append({
        "simple / scotch Coefficients",
        "Geometric or graph-based decomposition. scotch requires Scotch library at compile time.\n"
        "simple = geometric split along principal axes.",
        {},
        "method simple;\nsimpleCoeffs\n{\n    n (2 2 1);\n}\n"
    });
    m_decomposeParDictSections.last().params = {
        {"n", "vector label", "(2 2 1)", "Number of subdomains for simple. Optional for scotch."},
        {"delta", "scalar", "0.001", "Perturbation amount."},
    };

    m_decomposeParDictSections.append({
        "manual Coefficients",
        "Manual assignment of cells to processors via cellSet files.",
        {},
        "method manual;\nmanualCoeffs\n{\n    dataFile \"cellDecomposition\";\n}\n"
    });
    m_decomposeParDictSections.last().params = {
        {"dataFile", "fileName", "cellDecomposition", "File containing cell-to-processor mapping."},
    };

    m_decomposeParDictSections.append({
        "Distributed / Multi-Level",
        "Advanced options for distributed runs and constrained decomposition.",
        {},
        "distributed     no;\nroots           ();\n"
    });
    m_decomposeParDictSections.last().params = {
        {"distributed", "keyword", "no", "Distribute mesh across machines: yes / no."},
        {"roots", "word list", "()", "Root directories for distributed decomposition."},
        {"preservePatches", "word list", "()", "Patches to keep intact during decomposition."},
        {"preserveFaceZones", "word list", "()", "faceZones to keep on same processor."},
    };
}

// ════════════════════════════════════════════════════════════════════
// refineMeshDict data
// ════════════════════════════════════════════════════════════════════

void DictPanel::initRefineMeshDictData()
{
    m_refineMeshDictSections.append({
        "Top-Level",
        "Mesh refinement settings using cell sets. Refines cells in a specified set.",
        {},
        "set refineCells;\ncoordinateSystem global;\nuseHexTopology  true;\ngeometricCut    false;\nwriteMesh       false;\n"
    });
    m_refineMeshDictSections.last().params = {
        {"set", "word", "refineCells", "Name of the cellSet to refine."},
        {"coordinateSystem", "keyword", "global", "Coordinate system: global / patchLocal."},
        {"useHexTopology", "keyword", "true", "Split hexes 2x2x2 through edge midpoints."},
        {"geometricCut", "keyword", "false", "Purely geometric cut. Incompatible with useHexTopology."},
        {"writeMesh", "keyword", "false", "Write intermediate meshes."},
    };

    m_refineMeshDictSections.append({
        "globalCoeffs",
        "Global coordinate system (same for all cells). Normal = tan1 × tan2.",
        {},
        "globalCoeffs\n{\n    tan1 (1 0 0);\n    tan2 (0 1 0);\n}\n"
    });
    m_refineMeshDictSections.last().params = {
        {"tan1", "vector", "(1 0 0)", "First tangential direction."},
        {"tan2", "vector", "(0 1 0)", "Second tangential direction. Normal = tan1 × tan2."},
    };

    m_refineMeshDictSections.append({
        "patchLocalCoeffs",
        "Patch-local coordinate system (varies per cell). Normal = face normal of patch's first face.",
        {},
        "patchLocalCoeffs\n{\n    patch patchName;\n    tan1 (1 0 0);\n    tan2 (0 1 0);\n}\n"
    });
    m_refineMeshDictSections.last().params = {
        {"patch", "word", "patchName", "Patch name for normal direction reference."},
        {"tan1", "vector", "(1 0 0)", "First tangential direction."},
        {"tan2", "vector", "(0 1 0)", "Second tangential direction."},
    };

    m_refineMeshDictSections.append({
        "directions",
        "List of directions to apply refinement. Choose from: tan1, tan2, normal.",
        {},
        "directions\n(\n    tan1\n    tan2\n    normal\n);\n"
    });
    m_refineMeshDictSections.last().params = {
        {"directions", "word list", "(tan1 tan2 normal)", "Refinement directions. Any combination of tan1/tan2/normal."},
    };
}

// ── transportProperties ─────────────────────────────────────────
void DictPanel::initTransportPropertiesData() {
    m_transportSections.clear();
    m_transportSections.append({"Transport Model", "Rheology model selection.", {},
        "transportModel  Newtonian;\n\nnu              [0 2 -1 0 0 0 0] 1e-06;\n"});
    m_transportSections.last().params = {
        {"transportModel", "word", "Newtonian", "Rheology model: Newtonian / CrossPowerLaw / BirdCarreau / HerschelBulkley / powerLaw"},
        {"nu", "dimensionedScalar", "nu [0 2 -1 0 0 0 0] 1e-06", "Kinematic viscosity [m²/s]"},
        {"nuInf", "scalar", "0", "Infinite-shear viscosity (CrossPowerLaw/BirdCarreau)"},
        {"kappa", "scalar", "1", "Consistency index [Pa·s^n] (powerLaw/BirdCarreau)"},
        {"n", "scalar", "1", "Power-law exponent (n<1: shear-thinning, n>1: shear-thickening)"},
        {"tauYield", "scalar", "0", "Yield stress [Pa] (HerschelBulkley)"},
        {"K", "scalar", "0.001", "Consistency index (CrossPowerLaw)"},
        {"m", "scalar", "1", "Cross-power index (CrossPowerLaw)"},
        {"a", "scalar", "1", "Time constant (BirdCarreau)"},
        {"b", "scalar", "0.5", "Transition exponent (BirdCarreau)"},
    };
}

// ── thermophysicalProperties ─────────────────────────────────────
void DictPanel::initThermophysicalData() {
    m_thermoSections.clear();
    m_thermoSections.append({"Thermo Type", "Thermo-physical model selection.", {},
        "thermoType\n{\n    type            heRhoThermo;\n    mixture         pureMixture;\n"
        "    transport       const;\n    thermo          hConst;\n"
        "    equationOfState perfectGas;\n    specie          specie;\n    energy          sensibleEnthalpy;\n}\n"});
    m_thermoSections.last().params = {
        {"type", "word", "heRhoThermo", "Thermo type: heRhoThermo / hePsiThermo / fluidThermo"},
        {"mixture", "word", "pureMixture", "Mixture type: pureMixture / reactingMixture / multiComponentMixture"},
        {"transport", "word", "const", "Transport: const / sutherland / polynomial / tabulated"},
        {"thermo", "word", "hConst", "Thermodynamics: hConst / eConst / janaf / hPolynomial"},
        {"equationOfState", "word", "perfectGas", "EOS: perfectGas / icoPolynomial / PengRobinsonGas / rPolynomial"},
        {"specie", "word", "specie", "Specie type"},
        {"energy", "word", "sensibleEnthalpy", "Energy form: sensibleEnthalpy / sensibleInternalEnergy / absoluteEnthalpy"},
    };
    m_thermoSections.append({"Gas Properties", "Thermodynamic properties for gas phase.", {},
        "mixture\n{\n    specie\n    {\n        molWeight       28.96;\n    }\n"
        "    thermodynamics\n    {\n        Hf              0;\n        Sf              0;\n"
        "        Cp              1005;\n        Cv              718;\n    }\n"
        "    transport\n    {\n        mu              1.84e-05;\n        Pr              0.71;\n    }\n}\n"});
    m_thermoSections.last().params = {
        {"molWeight", "scalar", "28.96", "Molecular weight [g/mol]"},
        {"Hf", "scalar", "0", "Heat of formation [J/kg]"},
        {"Sf", "scalar", "0", "Standard entropy [J/kg·K]"},
        {"Cp", "scalar", "1005", "Specific heat at constant pressure [J/kg·K]"},
        {"Cv", "scalar", "718", "Specific heat at constant volume [J/kg·K]"},
        {"mu", "scalar", "1.84e-05", "Dynamic viscosity [Pa·s]"},
        {"Pr", "scalar", "0.71", "Prandtl number"},
        {"Tlow", "scalar", "200", "Lower temperature bound [K]"},
        {"Thigh", "scalar", "5000", "Upper temperature bound [K]"},
        {"As", "scalar", "1.4792e-06", "Sutherland coefficient [kg/m·s·√K]"},
        {"Ts", "scalar", "116", "Sutherland temperature [K]"},
    };
}

// ── radiationProperties ─────────────────────────────────────────
void DictPanel::initRadiationData() {
    m_radiationSections.clear();
    m_radiationSections.append({"Radiation Model", "Radiative heat transfer model.", {},
        "radiation       on;\n\nradiationModel  fvDOM;\n\nfvDOMCoeffs\n{\n"
        "    nPhi        4;\n    nTheta      0;\n    convergence 1e-3;\n"
        "    maxIter     4;\n}\n"});
    m_radiationSections.last().params = {
        {"radiation", "Switch", "on", "Enable/disable radiation"},
        {"radiationModel", "word", "fvDOM", "Model: fvDOM / P1 / P1Iso / viewFactor / opaqueSolid"},
        {"nPhi", "label", "4", "Azimuthal discretisation (fvDOM)"},
        {"nTheta", "label", "0", "Polar discretisation (fvDOM)"},
        {"convergence", "scalar", "1e-3", "Convergence tolerance (fvDOM)"},
        {"maxIter", "label", "4", "Max iterations (fvDOM)"},
        {"absorptionEmissionModel", "word", "none", "Absorption/emission model"},
        {"scatterModel", "word", "none", "Scattering model"},
        {"sootModel", "word", "none", "Soot model"},
    };
}

// ── combustionProperties ─────────────────────────────────────────
void DictPanel::initCombustionData() {
    m_combustionSections.clear();
    m_combustionSections.append({"Combustion Model", "Reaction and combustion model.", {},
        "combustionModel  PaSR;\n\nPaSRCoeffs\n{\n    Cmix        0.05;\n"
        "    kappa       0.5;\n}\n"});
    m_combustionSections.last().params = {
        {"combustionModel", "word", "laminar", "laminar / PaSR / EDC / eddyDissipation / diffusion / infinitelyFastChemistry"},
        {"Cmix", "scalar", "0.05", "Mixing constant (PaSR)"},
        {"kappa", "scalar", "0.5", "Reaction zone fraction (PaSR)"},
        {"Ctau", "scalar", "3.0", "Time scale constant (EDC)"},
        {"active", "Switch", "true", "Enable combustion"},
    };
}

// ── sampleDict ───────────────────────────────────────────────────
void DictPanel::initSampleDictData() {
    m_sampleDictSections.clear();
    m_sampleDictSections.append({"Sets", "Lines for sampling.", {},
        "sets\n(\n    centreLine\n    {\n        type    uniform;\n"
        "        axis    x;\n        start   (0 0 0.05);\n        end     (1 0 0.05);\n        nPoints 100;\n    }\n);\n"});
    m_sampleDictSections.last().params = {
        {"type", "word", "uniform", "midPoint / midPointAndFace / uniform / face / cloud"},
        {"axis", "word", "x", "Axis for line sampling"},
        {"start", "vector", "(0 0 0)", "Start point"},
        {"end", "vector", "(1 0 0)", "End point"},
        {"nPoints", "label", "100", "Number of sample points"},
        {"interpolationScheme", "word", "cell", "cell / cellPoint / cellPointFace"},
        {"setFormat", "word", "raw", "raw / vtk / csv / ensight / json"},
    };
    m_sampleDictSections.append({"Surfaces", "Surfaces for sampling.", {},
        "surfaces\n(\n    yNormal\n    {\n        type        plane;\n"
        "        planeType   pointAndNormal;\n        pointAndNormalDict { basePoint (0 0 0); normal (0 1 0); }\n"
        "        interpolate true;\n    }\n);\n"});
    m_sampleDictSections.last().params = {
        {"type", "word", "plane", "plane / isoSurface / cuttingPlane / patch"},
        {"basePoint", "vector", "(0 0 0)", "Base point for plane"},
        {"normal", "vector", "(0 1 0)", "Normal for plane"},
        {"interpolate", "Switch", "true", "Interpolate values"},
        {"isoValue", "scalar", "0.5", "Iso value (isoSurface)"},
    };
}

// ── setFieldsDict ───────────────────────────────────────────────
void DictPanel::initSetFieldsData() {
    m_setFieldsSections.clear();
    m_setFieldsSections.append({"Default Values", "Default field configuration.", {},
        "defaultFieldValues\n(\n    volScalarFieldValue alpha.water 0\n"
        "    volVectorFieldValue U (0 0 0)\n);\n"});
    m_setFieldsSections.last().params = {
        {"defaultFieldValues", "list", "()", "List of volScalarFieldValue / volVectorFieldValue"},
    };
    m_setFieldsSections.append({"Regions", "Geometric regions for field assignment.", {},
        "regions\n(\n    boxToCell\n    {\n        box (0 0 -1) (1 1 1);\n"
        "        fieldValues\n        (\n            volScalarFieldValue alpha.water 1\n"
        "        );\n    }\n);\n"});
    m_setFieldsSections.last().params = {
        {"box", "vector vector", "((0 0 0) (1 1 1))", "Bounding box (minX minY minZ) (maxX maxY maxZ)"},
        {"centre", "vector", "(0 0 0)", "Centre for sphereToCell"},
        {"radius", "scalar", "0.5", "Radius for sphereToCell"},
        {"fieldValues", "list", "()", "Field values within the region"},
    };
}

// ── forces / forceCoeffs ─────────────────────────────────────────
void DictPanel::initForcesData() {
    m_forcesSections.clear();
    m_forcesSections.append({"Forces", "Force and moment monitoring function object.", {},
        "forces\n{\n    type            forces;\n    libs            (\"libforces.so\");\n"
        "    patches         (\"walls\");\n    rho             rhoInf;\n    rhoInf          1.2;\n"
        "    CofR            (0 0 0);\n    writeControl    timeStep;\n    writeInterval   1;\n}\n"});
    m_forcesSections.last().params = {
        {"type", "word", "forces", "forces / forceCoeffs"},
        {"patches", "word list", "(walls)", "List of wall patch names"},
        {"rho", "word", "rhoInf", "Density field name"},
        {"rhoInf", "scalar", "1.2", "Freestream density [kg/m³]"},
        {"CofR", "vector", "(0 0 0)", "Centre of rotation for moment"},
        {"writeControl", "word", "timeStep", "Output control"},
        {"writeInterval", "label", "1", "Output interval"},
    };
    m_forcesSections.append({"Force Coefficients", "Aerodynamic coefficient monitoring.", {},
        "forceCoeffs\n{\n    type            forceCoeffs;\n    libs            (\"libforces.so\");\n"
        "    patches         (\"walls\");\n    rho             rhoInf;\n    rhoInf          1.2;\n"
        "    liftDir         (0 0 1);\n    dragDir         (1 0 0);\n    pitchAxis       (0 1 0);\n"
        "    magUInf         10;\n    lRef            1;\n    Aref            1;\n}\n"});
    m_forcesSections.last().params = {
        {"liftDir", "vector", "(0 0 1)", "Lift direction"},
        {"dragDir", "vector", "(1 0 0)", "Drag direction"},
        {"pitchAxis", "vector", "(0 1 0)", "Pitch axis for moment coefficient"},
        {"magUInf", "scalar", "10", "Freestream velocity magnitude [m/s]"},
        {"lRef", "scalar", "1", "Reference length [m]"},
        {"Aref", "scalar", "1", "Reference area [m²]"},
    };
}

// ── fvConstraints (v2112+) ─────────────────────────────────────
void DictPanel::initFvConstraintsData() {
    m_fvConstraintsSections.clear();
    m_fvConstraintsSections.append({"Field Constraints", "Field value constraints.", {},
        "limitPressure\n{\n    type        limitPressure;\n"
        "    min         10000;\n    max         500000;\n}\n"});
    m_fvConstraintsSections.last().params = {
        {"type", "word", "limitPressure", "limitPressure / limitTemperature / limitVelocity / fixedTemperatureConstraint"},
        {"min", "scalar", "0", "Minimum allowed value"},
        {"max", "scalar", "1e20", "Maximum allowed value"},
        {"phase", "word", "", "Phase name (multiphase)"},
    };
    m_fvConstraintsSections.append({"Mean Velocity", "Mean velocity constraint.", {},
        "meanVelocityForce\n{\n    type            meanVelocityForce;\n"
        "    selectionMode   all;\n    U               U;\n"
        "    velocity        (10 0 0);\n    relaxation      0.5;\n}\n"});
    m_fvConstraintsSections.last().params = {
        {"type", "word", "meanVelocityForce", "meanVelocityForce"},
        {"selectionMode", "word", "all", "all / cellZone / cellSet"},
        {"U", "word", "U", "Velocity field name"},
        {"velocity", "vector", "(10 0 0)", "Target mean velocity [m/s]"},
        {"relaxation", "scalar", "0.5", "Relaxation factor (0–1)"},
    };
}
