#include "meshviewer.h"
#include "ofmeshreader.h"

#include <QVBoxLayout><QHBoxLayout><QComboBox><QPushButton><QLabel>
#include <QFileInfo><QDir><QFile><QTextStream><QtMath>
#include <QOpenGLWidget>
#include <QMouseEvent><QWheelEvent>
#include <cmath>
#include <cstring>

// Minimal GLU replacements (MinGW doesn't have GLU)
static void gluPerspective(double fovY, double aspect, double zNear, double zFar) {
    double fH = tan(fovY/360.0*M_PI)*zNear, fW = fH*aspect;
    glFrustum(-fW,fW,-fH,fH,zNear,zFar);
}
static void gluLookAt(double ex,double ey,double ez,double cx,double cy,double cz,double ux,double uy,double uz) {
    QMatrix4x4 m; m.lookAt(QVector3D(ex,ey,ez),QVector3D(cx,cy,cz),QVector3D(ux,uy,uz));
    GLfloat mat[16]; memcpy(mat, m.constData(), sizeof(mat));
    glMultMatrixf(mat);
}

// ================================================================
// ASCII STL reader
// ================================================================
bool readASCII_STL(const QString &fp, QVector<QVector3D> &vv, QVector<QVector<uint>> &ff)
{
    QFile f(fp);
    if (!f.open(QFile::ReadOnly | QFile::Text)) return false;
    QTextStream in(&f);
    QString line;
    QVector3D norm;
    int state = 0; // 0=looking for facet, 1=looking for vertex, 2=done 3 verts
    int vi = 0;
    QVector<uint> face;
    QHash<QString, uint> unique; // deduplicate vertices
    auto addV = [&](const QVector3D &v) {
        QString k = QString("%1,%2,%3").arg(v.x(),0,'f',6).arg(v.y(),0,'f',6).arg(v.z(),0,'f',6);
        if (unique.contains(k)) { face.append(unique[k]); return; }
        uint idx = vv.size();
        vv.append(v);
        unique[k] = idx;
        face.append(idx);
    };
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.startsWith("facet normal")) {
            auto parts = line.split(QRegularExpression("\\s+"));
            if (parts.size() >= 5) norm = QVector3D(parts[2].toFloat(), parts[3].toFloat(), parts[4].toFloat());
            state = 1; vi = 0; face.clear();
        } else if (state == 1 && line.startsWith("vertex")) {
            auto parts = line.split(QRegularExpression("\\s+"));
            if (parts.size() >= 4) addV(QVector3D(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
            vi++;
            if (vi >= 3) { ff.append(face); state = 0; }
        }
    }
    f.close();
    return !vv.isEmpty();
}

// ================================================================
// OpenGL widget subclass
// ================================================================
class GLWidget : public QOpenGLWidget {
public:
    MeshViewer *owner = nullptr;
    void initializeGL() override {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glClearColor(0.17f, 0.17f, 0.17f, 1.0f);
    }
    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, double(w)/qMax(h,1), 0.1, 100000.0);
        glMatrixMode(GL_MODELVIEW);
    }
    void paintGL() override {
        if (!owner || owner->m_vv.isEmpty()) { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); return; }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Camera
        const auto &cp = owner->m_camPos;
        const auto &ct = owner->m_camTarget;
        const auto &cu = owner->m_camUp;
        gluLookAt(cp.x(), cp.y(), cp.z(), ct.x(), ct.y(), ct.z(), cu.x(), cu.y(), cu.z());
        glScalef(owner->m_zoom, owner->m_zoom, owner->m_zoom);

        // Light
        GLfloat lp[] = {1,1,1,0};
        glLightfv(GL_LIGHT0, GL_POSITION, lp);

        // Display mode
        if (owner->m_displayMode == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glColor3f(0.29f, 0.56f, 0.85f);
        GLfloat spec[] = {0.3f,0.3f,0.3f,1};
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialf(GL_FRONT, GL_SHININESS, 20);

        const auto &vv = owner->m_vv;
        const auto &ff = owner->m_ff;
        glBegin(GL_TRIANGLES);
        for (const auto &f : ff) {
            if (f.size() < 3) continue;
            QVector3D a = vv[f[0]], b = vv[f[1]], c = vv[f[2]];
            QVector3D n = QVector3D::crossProduct(b-a, c-a).normalized();
            glNormal3f(n.x(), n.y(), n.z());
            for (uint idx : f) glVertex3f(vv[idx].x(), vv[idx].y(), vv[idx].z());
            // Triangulate quads
            if (f.size() >= 4) {
                QVector3D d = vv[f[3]];
                QVector3D n2 = QVector3D::crossProduct(c-a, d-c).normalized();
                glNormal3f(n2.x(), n2.y(), n2.z());
                glVertex3f(vv[f[0]].x(), vv[f[0]].y(), vv[f[0]].z());
                glVertex3f(vv[f[2]].x(), vv[f[2]].y(), vv[f[2]].z());
                glVertex3f(vv[f[3]].x(), vv[f[3]].y(), vv[f[3]].z());
            }
        }
        glEnd();

        // Edge overlay
        if (owner->m_displayMode == 2 && !ff.isEmpty()) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_LIGHTING);
            glColor3f(0.1f,0.1f,0.1f);
            glLineWidth(1.0f);
            glBegin(GL_TRIANGLES);
            for (const auto &f : ff) {
                if (f.size() < 3) continue;
                for (uint idx : f) glVertex3f(vv[idx].x(), vv[idx].y(), vv[idx].z());
                if (f.size() >= 4) { glVertex3f(vv[f[0]].x(),vv[f[0]].y(),vv[f[0]].z()); glVertex3f(vv[f[2]].x(),vv[f[2]].y(),vv[f[2]].z()); glVertex3f(vv[f[3]].x(),vv[f[3]].y(),vv[f[3]].z()); }
            }
            glEnd();
            glEnable(GL_LIGHTING);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    void mousePressEvent(QMouseEvent *e) override { if(owner){owner->m_lastMouse=e->pos();owner->m_mousePressed=true;owner->m_buttons=e->buttons();} }
    void mouseMoveEvent(QMouseEvent *e) override {
        if(!owner || !owner->m_mousePressed) return;
        QPoint d = e->pos() - owner->m_lastMouse;
        owner->m_lastMouse = e->pos();
        if(owner->m_buttons & Qt::LeftButton) {
            float ax = d.y()*0.5f, ay = d.x()*0.5f;
            QVector3D dir = owner->m_camTarget - owner->m_camPos;
            float len = dir.length();
            dir.normalize();
            QVector3D right = QVector3D::crossProduct(dir, owner->m_camUp).normalized();
            QMatrix4x4 rot;
            rot.rotate(ax, right);
            rot.rotate(ay, owner->m_camUp);
            QVector4D r4 = rot.map(QVector4D(dir, 0.0f));
            owner->m_camPos = owner->m_camTarget - QVector3D(r4.x(), r4.y(), r4.z()) * len;
        } else if(owner->m_buttons & Qt::MiddleButton) {
            QVector3D dir = owner->m_camTarget - owner->m_camPos;
            float len = dir.length(); dir.normalize();
            QVector3D right = QVector3D::crossProduct(dir, owner->m_camUp).normalized();
            QVector3D up = QVector3D::crossProduct(right, dir).normalized();
            QVector3D pan = right*(float)(-d.x())*len*0.001f + up*(float)(d.y())*len*0.001f;
            owner->m_camPos += pan; owner->m_camTarget += pan;
        }
        update();
    }
    void mouseReleaseEvent(QMouseEvent *) override { if(owner) owner->m_mousePressed = false; }
    void wheelEvent(QWheelEvent *e) override {
        if(!owner) return;
        owner->m_zoom *= (e->angleDelta().y()>0) ? 1.1f : 0.9f;
        owner->m_zoom = qBound(0.01f, owner->m_zoom, 100.0f);
        update();
    }
};

// ================================================================
MeshViewer::MeshViewer(QWidget *p) : QWidget(p) { setupUI(); }
MeshViewer::~MeshViewer() {}

void MeshViewer::setupUI()
{
    auto *r = new QVBoxLayout(this); r->setContentsMargins(0,0,0,0); r->setSpacing(2);
    auto *b = new QHBoxLayout(); b->setContentsMargins(4,4,4,2);
    b->addWidget(new QLabel("View:"));
    m_vc = new QComboBox(); m_vc->addItems({"Free","Front","Back","Top","Bottom","Left","Right","Iso"}); b->addWidget(m_vc);
    m_wb = new QPushButton("Wireframe"); m_wb->setCheckable(true); m_wb->setFixedHeight(26); b->addWidget(m_wb);
    m_sb = new QPushButton("Surface"); m_sb->setCheckable(true); m_sb->setChecked(true); m_sb->setFixedHeight(26); b->addWidget(m_sb);
    m_seb = new QPushButton("Edges"); m_seb->setCheckable(true); m_seb->setFixedHeight(26); b->addWidget(m_seb);
    m_rb = new QPushButton("Reset"); m_rb->setFixedHeight(26); b->addWidget(m_rb);
    m_il = new QLabel("No mesh"); m_il->setStyleSheet("color:#888;font-size:11px;"); b->addWidget(m_il); b->addStretch(); r->addLayout(b);

    auto *glw = new GLWidget(); glw->owner = this; m_gl = glw; m_gl->setMinimumSize(400,200); r->addWidget(m_gl,1);

    connect(m_vc,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MeshViewer::onViewChanged);
    connect(m_wb,&QPushButton::toggled,[this](bool on){if(on){m_sb->setChecked(false);m_seb->setChecked(false);m_displayMode=1;updateGL();}});
    connect(m_sb,&QPushButton::toggled,[this](bool on){if(on){m_wb->setChecked(false);m_seb->setChecked(false);m_displayMode=0;updateGL();}});
    connect(m_seb,&QPushButton::toggled,[this](bool on){if(on){m_wb->setChecked(false);m_sb->setChecked(false);m_displayMode=2;updateGL();}});
    connect(m_rb,&QPushButton::clicked,this,&MeshViewer::resetCamera);
}

void MeshViewer::updateGL() { m_gl->update(); }
void MeshViewer::resetCamera() { m_camPos={0,0,500}; m_camTarget={0,0,0}; m_camUp={0,1,0}; m_zoom=1.0f; m_vc->setCurrentIndex(0); m_gl->update(); }

void MeshViewer::setCameraPos(const QVector3D &cp, const QVector3D &ct) {
    m_camPos=cp; m_camTarget=ct; m_camUp=(qAbs(cp.y()-ct.y())>qAbs(cp.z()-ct.z()))?QVector3D(0,0,1):QVector3D(0,1,0);
    m_gl->update();
}

void MeshViewer::onViewChanged(int i) {
    double cx=(m_bounds[0]+m_bounds[1])/2, cy=(m_bounds[2]+m_bounds[3])/2, cz=(m_bounds[4]+m_bounds[5])/2;
    double d=qMax(qMax(m_bounds[1]-m_bounds[0],m_bounds[3]-m_bounds[2]),m_bounds[5]-m_bounds[4])*1.5;
    if(d<1)d=500;
    switch(i){case 1:setCameraPos({(float)cx,(float)cy,(float)(cz+d)},{(float)cx,(float)cy,(float)cz});break;
    case 2:setCameraPos({(float)cx,(float)cy,(float)(cz-d)},{(float)cx,(float)cy,(float)cz});break;
    case 3:setCameraPos({(float)cx,(float)(cy+d),(float)cz},{(float)cx,(float)cy,(float)cz});break;
    case 4:setCameraPos({(float)cx,(float)(cy-d),(float)cz},{(float)cx,(float)cy,(float)cz});break;
    case 5:setCameraPos({(float)(cx-d),(float)cy,(float)cz},{(float)cx,(float)cy,(float)cz});break;
    case 6:setCameraPos({(float)(cx+d),(float)cy,(float)cz},{(float)cx,(float)cy,(float)cz});break;
    case 7:setCameraPos({(float)(cx+d),(float)(cy+d),(float)(cz+d)},{(float)cx,(float)cy,(float)cz});break;}
}

void MeshViewer::setViewFront(){onViewChanged(1);} void MeshViewer::setViewBack(){onViewChanged(2);}
void MeshViewer::setViewTop(){onViewChanged(3);} void MeshViewer::setViewBottom(){onViewChanged(4);}
void MeshViewer::setViewLeft(){onViewChanged(5);} void MeshViewer::setViewRight(){onViewChanged(6);}
void MeshViewer::setViewIsometric(){onViewChanged(7);}
void MeshViewer::setWireframe(bool on){if(on){m_sb->setChecked(false);m_seb->setChecked(false);m_displayMode=1;}else{m_sb->setChecked(true);m_displayMode=0;}updateGL();}
void MeshViewer::setSurface(bool on){if(on){m_wb->setChecked(false);m_seb->setChecked(false);m_displayMode=0;}updateGL();}
void MeshViewer::setSurfaceWithEdges(bool on){if(on){m_wb->setChecked(false);m_sb->setChecked(false);m_displayMode=2;}updateGL();}

bool MeshViewer::loadSTL(const QString &fp) {
    m_vv.clear(); m_ff.clear();
    if (!readASCII_STL(fp, m_vv, m_ff)) {
        emit loadError("Failed to read STL: "+fp); return false;
    }
    for (const auto &v : m_vv) {
        m_bounds[0]=qMin(m_bounds[0],v.x()); m_bounds[1]=qMax(m_bounds[1],v.x());
        m_bounds[2]=qMin(m_bounds[2],v.y()); m_bounds[3]=qMax(m_bounds[3],v.y());
        m_bounds[4]=qMin(m_bounds[4],v.z()); m_bounds[5]=qMax(m_bounds[5],v.z());
    }
    m_cf=fp; resetCamera(); m_il->setText(QFileInfo(fp).fileName()); emit meshLoaded(fp); return true;
}

bool MeshViewer::loadOBJ(const QString &fp) {
    // Simple OBJ: v and f lines only
    m_vv.clear(); m_ff.clear();
    QFile f(fp); if(!f.open(QFile::ReadOnly|QFile::Text)){emit loadError("Cannot open: "+fp); return false;}
    QTextStream in(&f); QString line;
    while(in.readLineInto(&line)) {
        line=line.trimmed();
        if(line.startsWith("v ")){auto p=line.split(QRegularExpression("\\s+"));if(p.size()>=4)m_vv.append({p[1].toFloat(),p[2].toFloat(),p[3].toFloat()});}
        else if(line.startsWith("f ")){auto p=line.split(QRegularExpression("\\s+"));QVector<uint> fc;for(int i=1;i<p.size()&&!p[i].isEmpty();++i){bool ok;int idx=p[i].split("//").first().split("/").first().toInt(&ok);if(ok&&idx!=0)fc.append(idx>0?idx-1:m_vv.size()+idx);}if(!fc.isEmpty())m_ff.append(fc);}
    }
    f.close();
    if(m_vv.isEmpty()){emit loadError("No vertices in OBJ");return false;}
    for(int i=0;i<6;++i)m_bounds[i]=(i%2==0)?1e9f:-1e9f;
    for(auto&v:m_vv){m_bounds[0]=qMin(m_bounds[0],v.x());m_bounds[1]=qMax(m_bounds[1],v.x());m_bounds[2]=qMin(m_bounds[2],v.y());m_bounds[3]=qMax(m_bounds[3],v.y());m_bounds[4]=qMin(m_bounds[4],v.z());m_bounds[5]=qMax(m_bounds[5],v.z());}
    m_cf=fp; resetCamera(); m_il->setText(QFileInfo(fp).fileName()); emit meshLoaded(fp); return true;
}

bool MeshViewer::loadOpenFOAMCase(const QString &cp) {
    OFMeshReader r; OFMeshData m;
    if(!r.readMesh(cp,m)){emit loadError(r.lastError());return false;}
    m_vv=m.vertices; m_ff=m.faces;
    for(int i=0;i<6;++i)m_bounds[i]=(i%2==0)?1e9f:-1e9f;
    for(auto&v:m_vv){m_bounds[0]=qMin(m_bounds[0],v.x());m_bounds[1]=qMax(m_bounds[1],v.x());m_bounds[2]=qMin(m_bounds[2],v.y());m_bounds[3]=qMax(m_bounds[3],v.y());m_bounds[4]=qMin(m_bounds[4],v.z());m_bounds[5]=qMax(m_bounds[5],v.z());}
    m_cf=cp; resetCamera(); m_il->setText(QString("%1 (%2v,%3f)").arg(QFileInfo(cp).fileName()).arg(m_vv.size()).arg(m_ff.size()));
    emit meshLoaded(cp); return true;
}

void MeshViewer::clear() { m_vv.clear(); m_ff.clear(); m_cf.clear(); m_il->setText("No mesh"); m_gl->update(); }
