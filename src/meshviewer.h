#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class QOpenGLWidget;
class QOpenGLFunctions;

class GLWidget;
class MeshViewer : public QWidget
{
    Q_OBJECT
    friend class GLWidget;
public:
    explicit MeshViewer(QWidget *p = nullptr);
    ~MeshViewer();

    bool loadSTL(const QString &fp);
    bool loadOBJ(const QString &fp);
    bool loadSTEP(const QString &) { emit loadError("STEP/IGES not supported yet"); return false; }
    bool loadOpenFOAMCase(const QString &cp);
    void clear();
    void setCameraPos(const QVector3D &cp, const QVector3D &ct);

    void setViewFront(); void setViewBack(); void setViewTop(); void setViewBottom();
    void setViewLeft(); void setViewRight(); void setViewIsometric();
    void resetCamera();
    void setWireframe(bool on);
    void setSurface(bool on);
    void setSurfaceWithEdges(bool on);

signals:
    void meshLoaded(const QString &fp);
    void loadError(const QString &e);

private slots:
    void onViewChanged(int i);
    void updateGL();

private:
    void setupUI();

    QOpenGLWidget *m_gl;
    QComboBox *m_vc;
    QPushButton *m_wb, *m_sb, *m_seb, *m_rb;
    QLabel *m_il;

    // Mesh data
    QVector<QVector3D> m_vv;
    QVector<QVector<uint>> m_ff;

    // Camera
    QVector3D m_camPos{0, 0, 500};
    QVector3D m_camTarget{0, 0, 0};
    QVector3D m_camUp{0, 1, 0};
    float m_zoom = 1.0f;

    // Mouse
    QPoint m_lastMouse;
    bool m_mousePressed = false;
    Qt::MouseButtons m_buttons;

    // Display mode: 0=surface, 1=wireframe, 2=surface+edges
    int m_displayMode = 0;

    // Bounds
    float m_bounds[6]{};
    QString m_cf;
};

// Helper: simple ASCII STL reader
bool readASCII_STL(const QString &fp, QVector<QVector3D> &vv, QVector<QVector<uint>> &ff);

#endif
