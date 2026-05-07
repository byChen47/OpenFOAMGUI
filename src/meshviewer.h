#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#include <QWidget>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QPoint>
#include <QOpenGLWidget>
class QComboBox;
class QPushButton;
class QLabel;

class GLViewer : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit GLViewer(QWidget *p = nullptr);
    void setMeshData(const QVector<float> &verts, const QVector<unsigned> &idx,
                     int nVerts);
    void clearMesh();
    void setCameraPos(const QVector3D &pos, const QVector3D &target);
    void setDisplayMode(int mode); // 0=surface, 1=wireframe, 2=surface+edges
    void resetView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    void initShaders();
    void uploadMesh();
    void updateProjection();

    QOpenGLShaderProgram *m_prog = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer m_ibo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;
    int m_attribPos = 0, m_attribNorm = 1;

    QVector<float> m_vertices;
    QVector<unsigned> m_indices;
    int m_numVerts = 0;
    bool m_meshDirty = false;

    // Camera
    QVector3D m_camPos{0, 0, 5};
    QVector3D m_camTarget{0, 0, 0};
    QVector3D m_camUp{0, 1, 0};
    QMatrix4x4 m_proj;

    // Mouse
    QPoint m_lastMouse;
    bool m_dragging = false;

    // Display
    int m_displayMode = 0; // 0=surface, 1=wireframe, 2=surface+edges

    friend class MeshViewer;
};

class MeshViewer : public QWidget
{
    Q_OBJECT
public:
    explicit MeshViewer(QWidget *p = nullptr);
    bool loadSTL(const QString &fp);
    bool loadOBJ(const QString &fp);
    bool loadSTEP(const QString &) { emit loadError("STEP not supported"); return false; }
    bool loadOpenFOAMCase(const QString &cp);
    void clear();

    void setViewFront(); void setViewBack(); void setViewTop(); void setViewBottom();
    void setViewLeft(); void setViewRight(); void setViewIsometric();
    void resetView();
    void setWireframe(bool on);
    void setSurface(bool on);
    void setSurfaceWithEdges(bool on);

signals:
    void meshLoaded(const QString &fp);
    void loadError(const QString &e);

private slots:
    void onViewChanged(int i);

private:
    void setupUI();
    void computeBounds();
    void genViewPos(int viewIdx, QVector3D &pos, QVector3D &target);

    GLViewer *m_gl;
    QComboBox *m_vc;
    QPushButton *m_wb, *m_sb, *m_seb, *m_rb;
    QLabel *m_il;
    QVector<float> m_verts;
    QVector<unsigned> m_idx;
    float m_bounds[6]{};
    QString m_cf;
};

#endif
