#include "meshviewer.h"
#include "ofmeshreader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QQuickWidget>
#include <QQuickItem>
#include <QQmlContext>
#include <QFileInfo>
#include <QDir>
#include <QtMath>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QVector3D>
#include <QUrl>
#include <QQmlEngine>

// ── QML for the 3D viewport ──
static const char *qml3D = R"(
import QtQuick
import QtQuick3D

Rectangle {
    color: "#2b2b2b"
    View3D {
        id: view3d
        anchors.fill: parent

        camera: PerspectiveCamera {
            id: cam
            position: Qt.vector3d(0, 0, 500)
            clipNear: 0.1
            clipFar: 50000
        }

        environment: SceneEnvironment {
            clearColor: "#2b2b2b"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: 30
        }
        DirectionalLight {
            eulerRotation.x: 30
            eulerRotation.y: -45
            brightness: 0.4
        }

        Model {
            id: meshModel
            visible: true
            scale: Qt.vector3d(1, 1, 1)
            materials: DefaultMaterial {
                diffuseColor: "#4a90d9"
                specularAmount: 0.3
            }
        }
    }

    function setView(v) {
        var d = 500;
        switch(v) {
        case "front":  cam.position = Qt.vector3d(0, 0, d); break;
        case "back":   cam.position = Qt.vector3d(0, 0, -d); break;
        case "top":    cam.position = Qt.vector3d(0, d, 0.01); break;
        case "bottom": cam.position = Qt.vector3d(0, -d, 0.01); break;
        case "left":   cam.position = Qt.vector3d(-d, 0, 0); break;
        case "right":  cam.position = Qt.vector3d(d, 0, 0); break;
        case "iso":    cam.position = Qt.vector3d(d*0.577, d*0.577, d*0.577); break;
        }
        cam.lookAt(Qt.vector3d(0, 0, 0));
    }

    function loadMesh(urlStr) {
        meshModel.source = urlStr;
    }
}
)";

// ── Helper: write mesh as Wavefront OBJ ──
static QString writeTempOBJ(const QVector<QVector3D> &verts,
                             const QVector<QVector<uint>> &faces)
{
    QString tmpPath = QDir::temp().filePath("ofgui_mesh.obj");
    QFile f(tmpPath);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return QString();
    QTextStream out(&f);
    out << "# OpenFOAM GUI mesh\n";
    for (const auto &v : verts)
        out << QString("v %1 %2 %3\n").arg(v.x()).arg(v.y()).arg(v.z());
    for (const auto &face : faces) {
        out << "f";
        for (int i = 0; i < face.size(); ++i)
            out << " " << (face[i] + 1); // OBJ 1-indexed
        out << "\n";
    }
    f.close();
    return QUrl::fromLocalFile(tmpPath).toString();
}

// ════════════════════════════════════════════════════
MeshViewer::MeshViewer(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

MeshViewer::~MeshViewer() {}

void MeshViewer::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(2);

    // ── Toolbar ──
    auto *bar = new QHBoxLayout();
    bar->setContentsMargins(4, 4, 4, 2);

    m_viewCombo = new QComboBox();
    m_viewCombo->addItems({"Free", "Front", "Back", "Top",
                           "Bottom", "Left", "Right", "Isometric"});
    m_viewCombo->setToolTip("Camera view");
    bar->addWidget(new QLabel("View:"));
    bar->addWidget(m_viewCombo);

    m_wireframeBtn = new QPushButton("Wireframe");
    m_wireframeBtn->setCheckable(true);
    m_wireframeBtn->setFixedHeight(26);
    bar->addWidget(m_wireframeBtn);

    m_resetBtn = new QPushButton("Reset");
    m_resetBtn->setFixedHeight(26);
    bar->addWidget(m_resetBtn);

    m_infoLabel = new QLabel("No mesh loaded");
    m_infoLabel->setStyleSheet("color: #888; font-size: 11px;");
    bar->addWidget(m_infoLabel);
    bar->addStretch();
    root->addLayout(bar);

    // ── 3D Viewport ──
    m_quickWidget = new QQuickWidget();
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->engine()->addImportPath("qrc:/");
    m_quickWidget->setContent(QUrl("data:text/plain;base64," +
        QString::fromUtf8(qml3D).toUtf8().toBase64()), nullptr,
        m_quickWidget->rootObject());
    root->addWidget(m_quickWidget, 1);

    // ── Connections ──
    connect(m_viewCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MeshViewer::onViewChanged);
    connect(m_wireframeBtn, &QPushButton::clicked, [this](bool c) {
        setWireframe(c);
    });
    connect(m_resetBtn, &QPushButton::clicked, this, &MeshViewer::onResetView);
}

void MeshViewer::onViewChanged(int index)
{
    static const char *views[] = {"", "front", "back", "top",
                                  "bottom", "left", "right", "iso"};
    if (index > 0 && index < 9)
        updateCameraView(views[index]);
}

void MeshViewer::onResetView()
{
    m_viewCombo->setCurrentIndex(0);
}

void MeshViewer::updateCameraView(const QString &view)
{
    QQuickItem *item = m_quickWidget->rootObject();
    if (item)
        QMetaObject::invokeMethod(item, "setView",
                                  Q_ARG(QVariant, view));
}

// ── Load STL/OBJ file ──
bool MeshViewer::loadSTL(const QString &filePath)
{
    QQuickItem *item = m_quickWidget->rootObject();
    if (!item) return false;
    QMetaObject::invokeMethod(item, "loadMesh",
                              Q_ARG(QVariant, QUrl::fromLocalFile(filePath).toString()));
    m_currentFile = filePath;
    m_vertices.clear();
    m_faces.clear();
    m_infoLabel->setText(QFileInfo(filePath).fileName());
    emit meshLoaded(filePath);
    return true;
}

bool MeshViewer::loadOBJ(const QString &filePath)
{
    return loadSTL(filePath);
}

// ── Load OpenFOAM case mesh ──
bool MeshViewer::loadOpenFOAMCase(const QString &casePath)
{
    OFMeshReader reader;
    OFMeshData mesh;
    if (!reader.readMesh(casePath, mesh)) {
        m_infoLabel->setText("Mesh load failed");
        emit loadError(reader.lastError());
        return false;
    }

    m_vertices = mesh.vertices;
    m_faces = mesh.faces;
    m_faceColors = mesh.faceColors;

    QString objPath = writeTempOBJ(m_vertices, m_faces);
    if (objPath.isEmpty()) {
        emit loadError("Failed to write temp mesh");
        return false;
    }

    QQuickItem *item = m_quickWidget->rootObject();
    if (item)
        QMetaObject::invokeMethod(item, "loadMesh",
                                  Q_ARG(QVariant, objPath));

    m_currentFile = casePath;
    m_infoLabel->setText(QString("%1 (%2 verts, %3 faces)")
        .arg(QFileInfo(casePath).fileName())
        .arg(mesh.vertices.size()).arg(mesh.faces.size()));
    emit meshLoaded(casePath);
    return true;
}

void MeshViewer::clear()
{
    m_vertices.clear();
    m_faces.clear();
    m_currentFile.clear();
    m_infoLabel->setText("No mesh loaded");
    QQuickItem *item = m_quickWidget->rootObject();
    if (item)
        QMetaObject::invokeMethod(item, "loadMesh",
                                  Q_ARG(QVariant, QString()));
}

// ── View presets ──
void MeshViewer::setViewFront()     { updateCameraView("front"); }
void MeshViewer::setViewBack()      { updateCameraView("back"); }
void MeshViewer::setViewTop()       { updateCameraView("top"); }
void MeshViewer::setViewBottom()    { updateCameraView("bottom"); }
void MeshViewer::setViewLeft()      { updateCameraView("left"); }
void MeshViewer::setViewRight()     { updateCameraView("right"); }
void MeshViewer::setViewIsometric() { updateCameraView("iso"); }

void MeshViewer::setWireframe(bool enabled)
{
    m_wireframeMode = enabled;
    m_wireframeBtn->setText(enabled ? "Solid" : "Wireframe");
}
