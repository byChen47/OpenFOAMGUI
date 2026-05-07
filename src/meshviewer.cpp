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
#include <QTextStream>
#include <QUrl>
#include <QQuickWidget>
#include <QQuickItem>
#include <QtMath>

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
            clipNear: 0.1; clipFar: 50000
        }
        environment: SceneEnvironment {
            clearColor: "#2b2b2b"
            backgroundMode: SceneEnvironment.Color
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }
        DirectionalLight { eulerRotation.x: -30; eulerRotation.y: 30 }
        DirectionalLight { eulerRotation.x: 30; eulerRotation.y: -45; brightness: 0.4 }
        Model {
            id: meshModel
            visible: true
            scale: Qt.vector3d(1, 1, 1)
            materials: DefaultMaterial { diffuseColor: "#4a90d9"; specularAmount: 0.3 }
        }
    }
    function setView(v) {
        var d = 500;
        switch(v) {
        case "front": cam.position=Qt.vector3d(0,0,d); break;
        case "back":  cam.position=Qt.vector3d(0,0,-d); break;
        case "top":   cam.position=Qt.vector3d(0,d,0.01); break;
        case "bottom":cam.position=Qt.vector3d(0,-d,0.01); break;
        case "left":  cam.position=Qt.vector3d(-d,0,0); break;
        case "right": cam.position=Qt.vector3d(d,0,0); break;
        case "iso":   cam.position=Qt.vector3d(d*0.577,d*0.577,d*0.577); break;
        }
        cam.lookAt(Qt.vector3d(0,0,0));
    }
    function loadMesh(urlStr) { meshModel.source = urlStr; }
}
)";

static QString writeTempOBJ(const QVector<QVector3D> &verts,
                             const QVector<QVector<uint>> &faces)
{
    QString p = QDir::temp().filePath("ofgui_mesh.obj");
    QFile f(p);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return QString();
    QTextStream o(&f);
    o << "# OF mesh\n";
    for (auto &v : verts) o << QString("v %1 %2 %3\n").arg(v.x()).arg(v.y()).arg(v.z());
    for (auto &fc : faces) { o << "f"; for (uint i : fc) o << " " << (i+1); o << "\n"; }
    f.close();
    return QUrl::fromLocalFile(p).toString();
}

MeshViewer::MeshViewer(QWidget *parent) : QWidget(parent) { setupUI(); }
MeshViewer::~MeshViewer() {}

void MeshViewer::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0); root->setSpacing(2);
    auto *bar = new QHBoxLayout(); bar->setContentsMargins(4,4,4,2);
    bar->addWidget(new QLabel("View:"));
    m_viewCombo = new QComboBox();
    m_viewCombo->addItems({"Free","Front","Back","Top","Bottom","Left","Right","Isometric"});
    bar->addWidget(m_viewCombo);
    m_wireframeBtn = new QPushButton("Wireframe"); m_wireframeBtn->setCheckable(true); m_wireframeBtn->setFixedHeight(26);
    bar->addWidget(m_wireframeBtn);
    m_resetBtn = new QPushButton("Reset"); m_resetBtn->setFixedHeight(26);
    bar->addWidget(m_resetBtn);
    m_infoLabel = new QLabel("No mesh loaded"); m_infoLabel->setStyleSheet("color:#888;font-size:11px;");
    bar->addWidget(m_infoLabel); bar->addStretch();
    root->addLayout(bar);
    m_quickWidget = new QQuickWidget();
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setContent(QUrl("data:text/plain;base64,"+QString::fromUtf8(qml3D).toUtf8().toBase64()),nullptr,nullptr);
    root->addWidget(m_quickWidget,1);
    connect(m_viewCombo,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MeshViewer::onViewChanged);
    connect(m_wireframeBtn,&QPushButton::toggled,this,&MeshViewer::setWireframe);
    connect(m_resetBtn,&QPushButton::clicked,this,&MeshViewer::resetCamera);
}

void MeshViewer::onViewChanged(int i) {
    static const char *v[]={"","front","back","top","bottom","left","right","iso"};
    if (i>0&&i<9) updateCameraView(v[i]);
}
void MeshViewer::resetCamera() { m_viewCombo->setCurrentIndex(0); }
void MeshViewer::updateCameraView(const QString &v) {
    QQuickItem *item = m_quickWidget->rootObject();
    if (item) QMetaObject::invokeMethod(item,"setView",Q_ARG(QVariant,v));
}

bool MeshViewer::loadSTL(const QString &fp) {
    QQuickItem *item = m_quickWidget->rootObject();
    if (!item) return false;
    QMetaObject::invokeMethod(item,"loadMesh",Q_ARG(QVariant,QUrl::fromLocalFile(fp).toString()));
    m_currentFile = fp; m_vertices.clear(); m_faces.clear();
    m_infoLabel->setText(QFileInfo(fp).fileName()); emit meshLoaded(fp); return true;
}
bool MeshViewer::loadOBJ(const QString &fp) { return loadSTL(fp); }

bool MeshViewer::loadSTEP(const QString &fp) {
    Q_UNUSED(fp);
    emit loadError("STEP/IGES requires VTK. Convert to STL first, or rebuild with VTK enabled.");
    return false;
}

bool MeshViewer::loadOpenFOAMCase(const QString &casePath) {
    OFMeshReader r; OFMeshData m;
    if (!r.readMesh(casePath,m)) { emit loadError(r.lastError()); return false; }
    m_vertices = m.vertices; m_faces = m.faces;
    QString obj = writeTempOBJ(m_vertices,m_faces);
    if (obj.isEmpty()) { emit loadError("Failed to write temp mesh"); return false; }
    QQuickItem *item = m_quickWidget->rootObject();
    if (item) QMetaObject::invokeMethod(item,"loadMesh",Q_ARG(QVariant,obj));
    m_currentFile = casePath;
    m_infoLabel->setText(QString("%1 (%2v, %3f)").arg(QFileInfo(casePath).fileName()).arg(m.vertices.size()).arg(m.faces.size()));
    emit meshLoaded(casePath); return true;
}

void MeshViewer::clear() { m_currentFile.clear(); m_infoLabel->setText("No mesh loaded"); }
void MeshViewer::setViewFront(){updateCameraView("front");} void MeshViewer::setViewBack(){updateCameraView("back");}
void MeshViewer::setViewTop(){updateCameraView("top");} void MeshViewer::setViewBottom(){updateCameraView("bottom");}
void MeshViewer::setViewLeft(){updateCameraView("left");} void MeshViewer::setViewRight(){updateCameraView("right");}
void MeshViewer::setViewIsometric(){updateCameraView("iso");}
void MeshViewer::setWireframe(bool on){m_wireframeMode=on; m_wireframeBtn->setText(on?"Solid":"Wireframe");}
