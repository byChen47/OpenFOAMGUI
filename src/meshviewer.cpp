#include "meshviewer.h"
#include "ofmeshreader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QtMath>
#include <QDir>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QRegularExpression>
#include <QDebug>
#include <cmath>

// ─── Shaders ───────────────────────────────────────────────────
static const char *vertSrc = R"(
#version 330 core
uniform mat4 uMVP;
uniform mat4 uModel;
in vec3 aPos;
in vec3 aNorm;
out vec3 vNorm;
out vec3 vPos;
void main() {
    vec4 wp = uModel * vec4(aPos,1.0);
    vPos = wp.xyz;
    vNorm = mat3(transpose(inverse(uModel))) * aNorm;
    gl_Position = uMVP * vec4(aPos,1.0);
}
)";

static const char *fragSrc = R"(
#version 330 core
in vec3 vNorm;
in vec3 vPos;
out vec4 fragColor;
uniform vec3 uLightDir;
uniform vec3 uColor;
uniform int uWire;
void main() {
    if (uWire == 1) { fragColor = vec4(0.1,0.1,0.1,1.0); return; }
    vec3 N = normalize(vNorm);
    vec3 L = normalize(uLightDir);
    float diff = max(dot(N,L), 0.0);
    float amb = 0.25;
    fragColor = vec4(uColor * (amb + diff), 1.0);
}
)";

// ─── ASCII STL reader ───────────────────────────────────────────
static bool readSTL(const QString &fp, QVector<float> &verts, QVector<unsigned> &idx)
{
    QFile f(fp);
    if (!f.open(QFile::ReadOnly|QFile::Text)) return false;
    QTextStream in(&f); QString line;
    QVector<QVector3D> vv;
    QHash<QString,unsigned> uniq;
    auto addV = [&](const QVector3D &v) {
        QString k = QString("%1,%2,%3").arg(v.x(),0,'f',6).arg(v.y(),0,'f',6).arg(v.z(),0,'f',6);
        if (uniq.contains(k)) { idx.append(uniq[k]); return; }
        unsigned i = vv.size(); vv.append(v); uniq[k]=i;
        verts.append(v.x()); verts.append(v.y()); verts.append(v.z());
        idx.append(i);
    };
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.startsWith("facet normal")) continue;
        if (line.startsWith("vertex")) {
            auto p = line.split(QRegularExpression("\\s+"));
            if (p.size()>=4) addV(QVector3D(p[1].toFloat(),p[2].toFloat(),p[3].toFloat()));
        }
    }
    f.close();
    return !vv.isEmpty();
}

// ─── OBJ reader ─────────────────────────────────────────────────
static bool readOBJ(const QString &fp, QVector<float> &verts, QVector<unsigned> &idx)
{
    QFile f(fp);
    if (!f.open(QFile::ReadOnly|QFile::Text)) return false;
    QTextStream in(&f); QString line;
    QVector<QVector3D> vv;
    while (in.readLineInto(&line)) {
        line = line.trimmed();
        if (line.startsWith("v ")) {
            auto p = line.split(QRegularExpression("\\s+"));
            if (p.size()>=4) vv.append(QVector3D(p[1].toFloat(),p[2].toFloat(),p[3].toFloat()));
        } else if (line.startsWith("f ")) {
            auto p = line.split(QRegularExpression("\\s+"));
            for (int i=1; i<p.size()&&!p[i].isEmpty(); ++i) {
                int vi = p[i].split("//").first().split("/").first().toInt();
                unsigned u = (vi>0) ? vi-1 : vv.size()+vi;
                if (u < (unsigned)vv.size()) idx.append(u);
            }
        }
    }
    f.close();
    if (vv.isEmpty()) return false;
    for (auto &v : vv) { verts.append(v.x()); verts.append(v.y()); verts.append(v.z()); }
    return true;
}

// ─── Convert OF faces to indexed triangle list ──────────────────
static void ofToIndexed(const QVector<QVector3D> &vv, const QVector<QVector<uint>> &ff,
                         QVector<float> &verts, QVector<unsigned> &idx)
{
    for (auto &v : vv) { verts.append(v.x()); verts.append(v.y()); verts.append(v.z()); }
    for (auto &f : ff) {
        if (f.size() < 3) continue;
        // Fan triangulation for polygons
        for (int i = 1; i+1 < f.size(); ++i) {
            idx.append(f[0]); idx.append(f[i]); idx.append(f[i+1]);
        }
    }
}

// ════════════════════════════════════════════════════════════════
//  GLViewer — OpenGL Core Profile rendering
// ════════════════════════════════════════════════════════════════
GLViewer::GLViewer(QWidget *p) : QOpenGLWidget(p) {
    setFocusPolicy(Qt::StrongFocus);
}

void GLViewer::initShaders() {
    m_prog = new QOpenGLShaderProgram(this);
    if (!m_prog->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc))
        qWarning("Vertex shader failed: %s", qPrintable(m_prog->log()));
    if (!m_prog->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc))
        qWarning("Fragment shader failed: %s", qPrintable(m_prog->log()));
    m_prog->bindAttributeLocation("aPos", 0);
    m_prog->bindAttributeLocation("aNorm", 1);
    if (!m_prog->link())
        qWarning("Shader link failed: %s", qPrintable(m_prog->log()));
}

void GLViewer::initializeGL() {
    auto *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0.17f, 0.17f, 0.17f, 1.0f);
    f->glEnable(GL_DEPTH_TEST);
    initShaders();
    m_vao.create();
}

void GLViewer::resizeGL(int w, int h) {
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, float(w)/qMax(h,1), 0.01f, 10000.0f);
}

void GLViewer::uploadMesh() {
    if (m_vertices.isEmpty() || m_indices.isEmpty()) return;
    m_vao.bind();
    m_vbo.create(); m_vbo.bind();
    m_vbo.allocate(m_vertices.constData(), m_vertices.size() * sizeof(float));
    m_ibo.create(); m_ibo.bind();
    m_ibo.allocate(m_indices.constData(), m_indices.size() * sizeof(unsigned));

    auto *f = QOpenGLContext::currentContext()->functions();
    // Position: 3 floats, offset 0
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    // Normal: 3 floats, offset 3*sizeof(float)
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    m_vao.release();
}

void GLViewer::setMeshData(const QVector<float> &verts, const QVector<unsigned> &idx, int nVerts) {
    m_vertices = verts; m_indices = idx; m_numVerts = nVerts;
    m_meshDirty = true;
    update();
}

void GLViewer::clearMesh() { m_vertices.clear(); m_indices.clear(); m_meshDirty=true; update(); }
void GLViewer::setDisplayMode(int m) { m_displayMode = m; update(); }

void GLViewer::setCameraPos(const QVector3D &p, const QVector3D &t) {
    m_camPos = p; m_camTarget = t;
    m_camUp = (qAbs(p.y()-t.y()) > qAbs(p.z()-t.z())) ? QVector3D(0,0,1) : QVector3D(0,1,0);
    update();
}
void GLViewer::resetView() {
    m_camPos = {0,0,5}; m_camTarget={0,0,0}; m_camUp={0,1,0};
    update();
}

void GLViewer::paintGL() {
    auto *f = QOpenGLContext::currentContext()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_meshDirty) { uploadMesh(); m_meshDirty = false; }
    if (m_indices.isEmpty()) return;

    m_prog->bind();
    m_vao.bind();

    // MVP matrix
    QMatrix4x4 view, model, mvp;
    view.lookAt(m_camPos, m_camTarget, m_camUp);
    mvp = m_proj * view * model;

    m_prog->setUniformValue("uMVP", mvp);
    m_prog->setUniformValue("uModel", model);
    m_prog->setUniformValue("uLightDir", QVector3D(0.5f, 1.0f, 0.8f).normalized());
    m_prog->setUniformValue("uColor", QVector3D(0.29f, 0.56f, 0.85f));

    if (m_displayMode == 1) {
        // Wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_prog->setUniformValue("uWire", 1);
        f->glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        // Surface
        m_prog->setUniformValue("uWire", 0);
        f->glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        // Edge overlay
        if (m_displayMode == 2) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);
            m_prog->setUniformValue("uWire", 1);
            f->glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
            glDisable(GL_POLYGON_OFFSET_LINE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    m_vao.release();
    m_prog->release();
}

// ── Mouse interaction ──
void GLViewer::mousePressEvent(QMouseEvent *e) { m_lastMouse = e->pos(); m_dragging = true; }
void GLViewer::mouseMoveEvent(QMouseEvent *e) {
    if (!m_dragging) return;
    QPoint d = e->pos() - m_lastMouse;
    m_lastMouse = e->pos();
    float rx = d.y() * 0.5f, ry = d.x() * 0.5f;
    if (e->buttons() & Qt::LeftButton) {
        QVector3D dir = (m_camTarget - m_camPos).normalized();
        QVector3D right = QVector3D::crossProduct(dir, m_camUp).normalized();
        QMatrix4x4 rot;
        rot.rotate(rx, right);
        rot.rotate(ry, m_camUp);
        QVector4D nd = rot * QVector4D(dir, 0);
        float len = (m_camTarget - m_camPos).length();
        m_camPos = m_camTarget - QVector3D(nd.x(), nd.y(), nd.z()) * len;
    } else if (e->buttons() & Qt::MiddleButton) {
        QVector3D dir = (m_camTarget - m_camPos).normalized();
        QVector3D right = QVector3D::crossProduct(dir, m_camUp).normalized();
        QVector3D up = QVector3D::crossProduct(right, dir).normalized();
        float s = (m_camTarget - m_camPos).length() * 0.001f;
        QVector3D pan = right * (-d.x() * s) + up * (d.y() * s);
        m_camPos += pan; m_camTarget += pan;
    }
    update();
}
void GLViewer::mouseReleaseEvent(QMouseEvent *e) { m_dragging = false; QOpenGLWidget::mouseReleaseEvent(e); }
void GLViewer::wheelEvent(QWheelEvent *e) {
    QVector3D dir = (m_camTarget - m_camPos).normalized();
    float s = (e->angleDelta().y() > 0) ? 0.9f : 1.1f;
    float d = (m_camTarget - m_camPos).length() * s;
    d = qBound(0.1f, d, 5000.0f);
    m_camPos = m_camTarget - dir * d;
    update();
}

// ════════════════════════════════════════════════════════════════
//  MeshViewer — UI wrapper
// ════════════════════════════════════════════════════════════════
MeshViewer::MeshViewer(QWidget *p) : QWidget(p) { setupUI(); }
void MeshViewer::setupUI() {
    auto *r = new QVBoxLayout(this); r->setContentsMargins(0,0,0,0); r->setSpacing(2);
    auto *b = new QHBoxLayout(); b->setContentsMargins(4,4,4,2);
    b->addWidget(new QLabel("View:"));
    m_vc = new QComboBox(); m_vc->addItems({"Free","Front","Back","Top","Bottom","Left","Right","Iso"}); b->addWidget(m_vc);
    m_wb = new QPushButton("Wire"); m_wb->setCheckable(true); m_wb->setFixedHeight(26); b->addWidget(m_wb);
    m_sb = new QPushButton("Surf"); m_sb->setCheckable(true); m_sb->setChecked(true); m_sb->setFixedHeight(26); b->addWidget(m_sb);
    m_seb = new QPushButton("Edge"); m_seb->setCheckable(true); m_seb->setFixedHeight(26); b->addWidget(m_seb);
    m_rb = new QPushButton("Reset"); m_rb->setFixedHeight(26); b->addWidget(m_rb);
    m_il = new QLabel("No mesh"); m_il->setStyleSheet("color:#888;font-size:11px;"); b->addWidget(m_il); b->addStretch(); r->addLayout(b);
    m_gl = new GLViewer(); r->addWidget(m_gl,1);
    connect(m_vc,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&MeshViewer::onViewChanged);
    connect(m_wb,&QPushButton::toggled,[this](bool on){if(on){m_sb->setChecked(false);m_seb->setChecked(false);m_gl->setDisplayMode(1);}});
    connect(m_sb,&QPushButton::toggled,[this](bool on){if(on){m_wb->setChecked(false);m_seb->setChecked(false);m_gl->setDisplayMode(0);}});
    connect(m_seb,&QPushButton::toggled,[this](bool on){if(on){m_wb->setChecked(false);m_sb->setChecked(false);m_gl->setDisplayMode(2);}});
    connect(m_rb,&QPushButton::clicked,[this](){m_gl->resetView();m_vc->setCurrentIndex(0);});
}

void MeshViewer::genViewPos(int i, QVector3D &pos, QVector3D &target) {
    float cx=(m_bounds[0]+m_bounds[1])/2, cy=(m_bounds[2]+m_bounds[3])/2, cz=(m_bounds[4]+m_bounds[5])/2;
    float d=qMax(qMax(m_bounds[1]-m_bounds[0],m_bounds[3]-m_bounds[2]),m_bounds[5]-m_bounds[4])*1.5f;
    if(d<1)d=5;
    target={cx,cy,cz};
    switch(i){case 1:pos={cx,cy,cz+d};break;case 2:pos={cx,cy,cz-d};break;case 3:pos={cx,cy+d,cz};break;
    case 4:pos={cx,cy-d,cz};break;case 5:pos={cx-d,cy,cz};break;case 6:pos={cx+d,cy,cz};break;
    case 7:pos={cx+d,cy+d,cz+d};break;}
}
void MeshViewer::onViewChanged(int i) { QVector3D p,t; if(i>0&&i<8){genViewPos(i,p,t);m_gl->setCameraPos(p,t);} }
void MeshViewer::computeBounds() {
    if(m_verts.isEmpty())return;
    for(int i=0;i<6;++i)m_bounds[i]=(i%2==0)?1e9f:-1e9f;
    for(int i=0;i<m_verts.size();i+=6){float x=m_verts[i],y=m_verts[i+1],z=m_verts[i+2];
        m_bounds[0]=qMin(m_bounds[0],x);m_bounds[1]=qMax(m_bounds[1],x);
        m_bounds[2]=qMin(m_bounds[2],y);m_bounds[3]=qMax(m_bounds[3],y);
        m_bounds[4]=qMin(m_bounds[4],z);m_bounds[5]=qMax(m_bounds[5],z);}
}
void MeshViewer::setViewFront(){onViewChanged(1);}void MeshViewer::setViewBack(){onViewChanged(2);}
void MeshViewer::setViewTop(){onViewChanged(3);}void MeshViewer::setViewBottom(){onViewChanged(4);}
void MeshViewer::setViewLeft(){onViewChanged(5);}void MeshViewer::setViewRight(){onViewChanged(6);}
void MeshViewer::setViewIsometric(){onViewChanged(7);}
void MeshViewer::resetView(){m_gl->resetView();m_vc->setCurrentIndex(0);}
void MeshViewer::setWireframe(bool on){if(on){m_sb->setChecked(false);m_seb->setChecked(false);m_gl->setDisplayMode(1);}}
void MeshViewer::setSurface(bool on){if(on){m_wb->setChecked(false);m_seb->setChecked(false);m_gl->setDisplayMode(0);}}
void MeshViewer::setSurfaceWithEdges(bool on){if(on){m_wb->setChecked(false);m_sb->setChecked(false);m_gl->setDisplayMode(2);}}

bool MeshViewer::loadSTL(const QString &fp) {
    m_verts.clear(); m_idx.clear();
    if (!readSTL(fp, m_verts, m_idx)) { emit loadError("Failed: "+fp); return false; }
    m_gl->setMeshData(m_verts, m_idx, m_verts.size()/6);
    computeBounds(); m_cf=fp; m_il->setText(QFileInfo(fp).fileName()); emit meshLoaded(fp); return true;
}
bool MeshViewer::loadOBJ(const QString &fp) {
    m_verts.clear(); m_idx.clear();
    if (!readOBJ(fp, m_verts, m_idx)) { emit loadError("Failed: "+fp); return false; }
    m_gl->setMeshData(m_verts, m_idx, m_verts.size()/6);
    computeBounds(); m_cf=fp; m_il->setText(QFileInfo(fp).fileName()); emit meshLoaded(fp); return true;
}
bool MeshViewer::loadOpenFOAMCase(const QString &cp) {
    OFMeshReader r; OFMeshData m;
    if (!r.readMesh(cp,m)) { emit loadError(r.lastError()); return false; }
    m_verts.clear(); m_idx.clear();
    ofToIndexed(m.vertices, m.faces, m_verts, m_idx);
    // Compute normals
    QVector<float> vn(m_verts.size(), 0.0f);
    for (int i=0; i+2<m_idx.size(); i+=3) {
        unsigned a=m_idx[i], b=m_idx[i+1], c=m_idx[i+2];
        QVector3D va(m_verts[a*3],m_verts[a*3+1],m_verts[a*3+2]);
        QVector3D vb(m_verts[b*3],m_verts[b*3+1],m_verts[b*3+2]);
        QVector3D vc(m_verts[c*3],m_verts[c*3+1],m_verts[c*3+2]);
        QVector3D n = QVector3D::crossProduct(vb-va, vc-va).normalized();
        for (unsigned idx : {a,b,c}) { vn[idx*3]+=n.x(); vn[idx*3+1]+=n.y(); vn[idx*3+2]+=n.z(); }
    }
    // Interleave: pos(3) + norm(3) for each vertex
    QVector<float> interleaved;
    for (int i=0; i<m_verts.size()/3; ++i) {
        interleaved.append(m_verts[i*3]); interleaved.append(m_verts[i*3+1]); interleaved.append(m_verts[i*3+2]);
        QVector3D n(vn[i*3],vn[i*3+1],vn[i*3+2]); n.normalize();
        interleaved.append(n.x()); interleaved.append(n.y()); interleaved.append(n.z());
    }
    m_verts = interleaved;
    m_gl->setMeshData(m_verts, m_idx, m_verts.size()/6);
    computeBounds(); m_cf=cp;
    m_il->setText(QString("%1 (%2v,%3f)").arg(QFileInfo(cp).fileName()).arg(m_verts.size()/6).arg(m_idx.size()/3));
    emit meshLoaded(cp); return true;
}
void MeshViewer::clear() { m_verts.clear(); m_idx.clear(); m_gl->clearMesh(); m_il->setText("No mesh"); }
