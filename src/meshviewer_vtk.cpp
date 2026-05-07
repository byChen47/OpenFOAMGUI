#include "meshviewer.h"
#include "ofmeshreader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QtMath>

// ── VTK core ──
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkCompositePolyDataMapper.h>

// ── VTK readers ──
#include <vtkOpenFOAMReader.h>
#include <vtkSTLReader.h>
#include <vtkOBJReader.h>

// ================================================================
MeshViewer::MeshViewer(QWidget *parent) : QWidget(parent)
{
    setupUI();
    setupVTKPipeline();
}

MeshViewer::~MeshViewer()
{
    removeAllActors();
}

void MeshViewer::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(2);

    // ── Toolbar ──
    auto *bar = new QHBoxLayout();
    bar->setContentsMargins(4, 4, 4, 2);
    bar->addWidget(new QLabel("View:"));

    m_viewCombo = new QComboBox();
    m_viewCombo->addItems({"Free", "Front", "Back", "Top",
                           "Bottom", "Left", "Right", "Isometric"});
    bar->addWidget(m_viewCombo);

    m_wireframeBtn = new QPushButton("Wireframe");
    m_wireframeBtn->setCheckable(true); m_wireframeBtn->setFixedHeight(26);
    bar->addWidget(m_wireframeBtn);

    m_surfaceBtn = new QPushButton("Surface");
    m_surfaceBtn->setCheckable(true); m_surfaceBtn->setChecked(true);
    m_surfaceBtn->setFixedHeight(26);
    bar->addWidget(m_surfaceBtn);

    m_surfaceEdgeBtn = new QPushButton("Edges");
    m_surfaceEdgeBtn->setCheckable(true); m_surfaceEdgeBtn->setFixedHeight(26);
    bar->addWidget(m_surfaceEdgeBtn);

    m_resetBtn = new QPushButton("Reset");
    m_resetBtn->setFixedHeight(26);
    bar->addWidget(m_resetBtn);

    m_infoLabel = new QLabel("No mesh loaded");
    m_infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    bar->addWidget(m_infoLabel);
    bar->addStretch();
    root->addLayout(bar);

    // ── VTK viewport ──
    m_vtkWidget = new QVTKOpenGLNativeWidget();
    m_vtkWidget->setMinimumSize(400, 200);
    root->addWidget(m_vtkWidget, 1);

    // ── Connections ──
    connect(m_viewCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MeshViewer::onViewChanged);
    connect(m_wireframeBtn, &QPushButton::toggled, this, &MeshViewer::setWireframe);
    connect(m_surfaceBtn,    &QPushButton::toggled, this, &MeshViewer::setSurface);
    connect(m_surfaceEdgeBtn,&QPushButton::toggled, this, &MeshViewer::setSurfaceWithEdges);
    connect(m_resetBtn,      &QPushButton::clicked,  this, &MeshViewer::resetCamera);
}

void MeshViewer::setupVTKPipeline()
{
    m_renderWindow = vtkGenericOpenGLRenderWindow::New();
    m_vtkWidget->setRenderWindow(m_renderWindow);

    m_renderer = vtkRenderer::New();
    m_renderer->SetBackground(0.17, 0.17, 0.17);
    m_renderer->SetBackground2(0.33, 0.33, 0.33);
    m_renderer->SetGradientBackground(true);
    m_renderWindow->AddRenderer(m_renderer);
}

void MeshViewer::addActor(vtkActor *actor)
{
    m_renderer->AddActor(actor);
    m_actors.append(actor);
}

void MeshViewer::removeAllActors()
{
    for (auto *a : m_actors) {
        m_renderer->RemoveActor(a);
        a->Delete();
    }
    m_actors.clear();
}

void MeshViewer::setCameraPos(double x, double y, double z,
                               double fx, double fy, double fz)
{
    vtkCamera *cam = m_renderer->GetActiveCamera();
    cam->SetPosition(x, y, z);
    cam->SetFocalPoint(fx, fy, fz);
    cam->SetViewUp(0, 0, 1);
    m_renderer->ResetCameraClippingRange();
    m_renderWindow->Render();
}

// ── Camera views ──
void MeshViewer::onViewChanged(int index)
{
    double cx = (m_bounds[0] + m_bounds[1]) / 2.0;
    double cy = (m_bounds[2] + m_bounds[3]) / 2.0;
    double cz = (m_bounds[4] + m_bounds[5]) / 2.0;
    double d  = qMax(qMax(m_bounds[1]-m_bounds[0],
                           m_bounds[3]-m_bounds[2]),
                     m_bounds[5]-m_bounds[4]) * 1.5;
    if (d < 1.0) d = 500.0;
    if (index == 1) setCameraPos(cx, cy, cz+d, cx, cy, cz);
    if (index == 2) setCameraPos(cx, cy, cz-d, cx, cy, cz);
    if (index == 3) setCameraPos(cx, cy+d, cz, cx, cy, cz);
    if (index == 4) setCameraPos(cx, cy-d, cz, cx, cy, cz);
    if (index == 5) setCameraPos(cx-d, cy, cz, cx, cy, cz);
    if (index == 6) setCameraPos(cx+d, cy, cz, cx, cy, cz);
    if (index == 7) setCameraPos(cx+d, cy+d, cz+d, cx, cy, cz);
}
void MeshViewer::setViewFront()     { onViewChanged(1); }
void MeshViewer::setViewBack()      { onViewChanged(2); }
void MeshViewer::setViewTop()       { onViewChanged(3); }
void MeshViewer::setViewBottom()    { onViewChanged(4); }
void MeshViewer::setViewLeft()      { onViewChanged(5); }
void MeshViewer::setViewRight()     { onViewChanged(6); }
void MeshViewer::setViewIsometric() { onViewChanged(7); }
void MeshViewer::resetCamera()      { m_renderer->ResetCamera(); m_renderWindow->Render(); m_viewCombo->setCurrentIndex(0); }

// ── Display modes ──
void MeshViewer::setWireframe(bool on)
{
    if (on) { m_surfaceBtn->setChecked(false); m_surfaceEdgeBtn->setChecked(false); }
    for (auto *a : m_actors) a->GetProperty()->SetRepresentation(
        on ? VTK_WIREFRAME : VTK_SURFACE);
    m_renderWindow->Render();
}
void MeshViewer::setSurface(bool on)
{
    if (on) { m_wireframeBtn->setChecked(false); m_surfaceEdgeBtn->setChecked(false); }
    for (auto *a : m_actors) {
        a->GetProperty()->SetRepresentation(VTK_SURFACE);
        a->GetProperty()->EdgeVisibilityOff();
    }
    m_renderWindow->Render();
}
void MeshViewer::setSurfaceWithEdges(bool on)
{
    if (on) { m_wireframeBtn->setChecked(false); m_surfaceBtn->setChecked(false); }
    for (auto *a : m_actors) {
        a->GetProperty()->SetRepresentation(VTK_SURFACE);
        a->GetProperty()->SetEdgeVisibility(on);
        a->GetProperty()->SetEdgeColor(0.1, 0.1, 0.1);
    }
    m_renderWindow->Render();
}

// ── STL ──
bool MeshViewer::loadSTL(const QString &filePath)
{
    removeAllActors();
    vtkNew<vtkSTLReader> r;
    r->SetFileName(filePath.toUtf8().constData());
    r->Update();
    if (!r->GetOutput() || r->GetOutput()->GetNumberOfPoints() == 0) {
        emit loadError("Failed to read STL: " + filePath); return false;
    }
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(r->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    a->GetProperty()->SetColor(0.29, 0.56, 0.85);
    a->GetProperty()->SetSpecular(0.3); a->GetProperty()->SetSpecularPower(20);
    addActor(a);
    double *b = a->GetBounds();
    for (int i=0;i<6;++i) m_bounds[i]=b[i];
    m_currentFile = filePath;
    resetCamera();
    m_infoLabel->setText(QFileInfo(filePath).fileName());
    emit meshLoaded(filePath);
    return true;
}

// ── OBJ ──
bool MeshViewer::loadOBJ(const QString &filePath)
{
    removeAllActors();
    vtkNew<vtkOBJReader> r;
    r->SetFileName(filePath.toUtf8().constData());
    r->Update();
    if (!r->GetOutput() || r->GetOutput()->GetNumberOfPoints() == 0) {
        emit loadError("Failed to read OBJ: " + filePath); return false;
    }
    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(r->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    a->GetProperty()->SetColor(0.29, 0.56, 0.85);
    addActor(a);
    double *b = a->GetBounds();
    for (int i=0;i<6;++i) m_bounds[i]=b[i];
    m_currentFile = filePath;
    resetCamera();
    m_infoLabel->setText(QFileInfo(filePath).fileName());
    emit meshLoaded(filePath);
    return true;
}

// ── STEP / IGES (needs OpenCASCADE — fallback message) ──
bool MeshViewer::loadSTEP(const QString &filePath)
{
    Q_UNUSED(filePath);
    emit loadError(
        "STEP/IGES loading requires VTK built with OpenCASCADE.\n"
        "Convert the file to STL format first, or rebuild VTK with\n"
        "-DVTK_MODULE_ENABLE_VTK_IOChemistry=YES (includes OpenCASCADE).");
    return false;
}

// ── OpenFOAM case ──
bool MeshViewer::loadOpenFOAMCase(const QString &casePath)
{
    removeAllActors();

    // 1. Try direct polyMesh parsing (fast, always works)
    OFMeshReader ofReader;
    OFMeshData ofMesh;
    if (ofReader.readMesh(casePath, ofMesh)) {
        vtkNew<vtkPolyData> pd;
        vtkNew<vtkPoints> pts;
        for (const auto &v : ofMesh.vertices)
            pts->InsertNextPoint(v.x(), v.y(), v.z());
        vtkNew<vtkCellArray> polys;
        for (const auto &f : ofMesh.faces) {
            polys->InsertNextCell(f.size());
            for (uint idx : f) polys->InsertCellPoint(idx);
        }
        pd->SetPoints(pts);
        pd->SetPolys(polys);

        vtkNew<vtkPolyDataMapper> m;
        m->SetInputData(pd);
        vtkNew<vtkActor> a;
        a->SetMapper(m);
        a->GetProperty()->SetColor(0.29, 0.56, 0.85);
        a->GetProperty()->SetSpecular(0.3);
        addActor(a);

        double *b = a->GetBounds();
        for (int i=0;i<6;++i) m_bounds[i]=b[i];
        m_currentFile = casePath;
        resetCamera();
        m_infoLabel->setText(QString("%1 (%2 verts, %3 faces)")
            .arg(QFileInfo(casePath).fileName())
            .arg(ofMesh.vertices.size()).arg(ofMesh.faces.size()));
        emit meshLoaded(casePath);
        return true;
    }

    // 2. Fallback: VTK's native OpenFOAM reader via .foam file
    QString foamFile = QDir(casePath).filePath(
        QDir(casePath).dirName() + ".foam");
    if (!QFileInfo::exists(foamFile)) {
        QFile f(foamFile); f.open(QFile::WriteOnly); f.close();
    }

    vtkNew<vtkOpenFOAMReader> foamReader;
    foamReader->SetFileName(foamFile.toUtf8().constData());
    foamReader->Update();

    if (foamReader->GetOutput() == nullptr) {
        emit loadError("Both polyMesh and VTK reader failed for: " + casePath);
        return false;
    }

    vtkNew<vtkCompositeDataGeometryFilter> gf;
    gf->SetInputConnection(foamReader->GetOutputPort());
    gf->Update();

    vtkNew<vtkPolyDataMapper> m;
    m->SetInputConnection(gf->GetOutputPort());
    vtkNew<vtkActor> a;
    a->SetMapper(m);
    a->GetProperty()->SetColor(0.29, 0.56, 0.85);
    a->GetProperty()->SetSpecular(0.3);
    addActor(a);

    double *b = a->GetBounds();
    for (int i=0;i<6;++i) m_bounds[i]=b[i];
    m_currentFile = casePath;
    resetCamera();
    m_infoLabel->setText(QString("%1 (VTK OF reader)").arg(QFileInfo(casePath).fileName()));
    emit meshLoaded(casePath);
    return true;
}

void MeshViewer::clear()
{
    removeAllActors();
    m_currentFile.clear();
    m_infoLabel->setText("No mesh loaded");
    m_renderWindow->Render();
}
