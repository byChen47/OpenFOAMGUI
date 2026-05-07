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

static const char *QML = R"(
import QtQuick; import QtQuick3D
Rectangle { color: "#2b2b2b"
View3D { id: v3d; anchors.fill: parent
camera: PerspectiveCamera { id: cam; position: Qt.vector3d(0,0,500); clipNear: 0.1; clipFar: 50000 }
environment: SceneEnvironment { clearColor: "#2b2b2b"; backgroundMode: SceneEnvironment.Color; antialiasingMode: SceneEnvironment.MSAA; antialiasingQuality: SceneEnvironment.High }
DirectionalLight { eulerRotation.x: -30; eulerRotation.y: 30 }
DirectionalLight { eulerRotation.x: 30; eulerRotation.y: -45; brightness: 0.4 }
Model { id: meshModel; scale: Qt.vector3d(1,1,1); materials: DefaultMaterial { diffuseColor: "#4a90d9"; specularAmount: 0.3 } }
}
function setView(v) { var d = 500;
switch(v) {
case "front": cam.position = Qt.vector3d(0,0,d); break;
case "back": cam.position = Qt.vector3d(0,0,-d); break;
case "top": cam.position = Qt.vector3d(0,d,0.01); break;
case "bottom": cam.position = Qt.vector3d(0,-d,0.01); break;
case "left": cam.position = Qt.vector3d(-d,0,0); break;
case "right": cam.position = Qt.vector3d(d,0,0); break;
case "iso": cam.position = Qt.vector3d(d*0.577,d*0.577,d*0.577); break;
}
cam.lookAt(Qt.vector3d(0,0,0)); }
function loadMesh(u) { meshModel.source = u; }
})";

static QString writeOBJ(const QVector<QVector3D>&vv,const QVector<QVector<uint>>&ff){
    QString p=QDir::temp().filePath("ofgui_mesh.obj");QFile f(p);
    if(!f.open(QFile::WriteOnly|QFile::Text))return{};
    QTextStream o(&f);o<<"# OF\n";
    for(auto&v:vv)o<<QString("v %1 %2 %3\n").arg(v.x()).arg(v.y()).arg(v.z());
    for(auto&fc:ff){o<<"f";for(uint i:fc)o<<" "<<(i+1);o<<"\n";}
    f.close();return QUrl::fromLocalFile(p).toString();
}

MeshViewer::MeshViewer(QWidget*p):QWidget(p){setupUI();}
MeshViewer::~MeshViewer(){}
void MeshViewer::setupUI(){
    auto*r=new QVBoxLayout(this);r->setContentsMargins(0,0,0,0);r->setSpacing(2);
    auto*b=new QHBoxLayout();b->setContentsMargins(4,4,4,2);b->addWidget(new QLabel("View:"));
    m_vc=new QComboBox();m_vc->addItems({"Free","Front","Back","Top","Bottom","Left","Right","Iso"});b->addWidget(m_vc);
    m_wb=new QPushButton("Wireframe");m_wb->setCheckable(true);m_wb->setFixedHeight(26);b->addWidget(m_wb);
    m_rb=new QPushButton("Reset");m_rb->setFixedHeight(26);b->addWidget(m_rb);
    m_il=new QLabel("No mesh");m_il->setStyleSheet("color:#888;font-size:11px;");b->addWidget(m_il);b->addStretch();r->addLayout(b);
    m_qw=new QQuickWidget();m_qw->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_qw->setContent(QUrl("data:text/plain;base64,"+QString::fromUtf8(QML).toUtf8().toBase64()),nullptr,nullptr);r->addWidget(m_qw,1);
    connect(m_vc,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MeshViewer::onViewChanged);
    connect(m_wb,&QPushButton::toggled,this,&MeshViewer::setWireframe);
    connect(m_rb,&QPushButton::clicked,this,&MeshViewer::resetCamera);
}
void MeshViewer::onViewChanged(int i){static const char*v[]={"","front","back","top","bottom","left","right","iso"};if(i>0&&i<9)updateCameraView(v[i]);}
void MeshViewer::resetCamera(){m_vc->setCurrentIndex(0);}
void MeshViewer::updateCameraView(const QString&v){auto*i=m_qw->rootObject();if(i)QMetaObject::invokeMethod(i,"setView",Q_ARG(QVariant,v));}
bool MeshViewer::loadSTL(const QString&fp){auto*i=m_qw->rootObject();if(!i)return false;QMetaObject::invokeMethod(i,"loadMesh",Q_ARG(QVariant,QUrl::fromLocalFile(fp).toString()));m_cf=fp;m_il->setText(QFileInfo(fp).fileName());emit meshLoaded(fp);return true;}
bool MeshViewer::loadOBJ(const QString&fp){return loadSTL(fp);}
bool MeshViewer::loadSTEP(const QString&fp){Q_UNUSED(fp);emit loadError("STEP/IGES needs VTK. Convert to STL first.");return false;}
bool MeshViewer::loadOpenFOAMCase(const QString&cp){OFMeshReader r;OFMeshData m;if(!r.readMesh(cp,m)){emit loadError(r.lastError());return false;}m_vv=m.vertices;m_ff=m.faces;QString o=writeOBJ(m_vv,m_ff);if(o.isEmpty()){emit loadError("Temp OBJ failed");return false;}auto*i=m_qw->rootObject();if(i)QMetaObject::invokeMethod(i,"loadMesh",Q_ARG(QVariant,o));m_cf=cp;m_il->setText(QString("%1 (%2v %3f)").arg(QFileInfo(cp).fileName()).arg(m.vertices.size()).arg(m.faces.size()));emit meshLoaded(cp);return true;}
void MeshViewer::clear(){m_cf.clear();m_il->setText("No mesh");}
void MeshViewer::setViewFront(){updateCameraView("front");}void MeshViewer::setViewBack(){updateCameraView("back");}
void MeshViewer::setViewTop(){updateCameraView("top");}void MeshViewer::setViewBottom(){updateCameraView("bottom");}
void MeshViewer::setViewLeft(){updateCameraView("left");}void MeshViewer::setViewRight(){updateCameraView("right");}
void MeshViewer::setViewIsometric(){updateCameraView("iso");}
void MeshViewer::setWireframe(bool on){m_wm=on;m_wb->setText(on?"Solid":"Wireframe");}
