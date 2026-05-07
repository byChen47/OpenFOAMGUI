#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#include <QWidget>
#include <QString>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVector>

class QVTKOpenGLNativeWidget;
class vtkGenericOpenGLRenderWindow;
class vtkRenderer;
class vtkActor;

class MeshViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MeshViewer(QWidget *parent = nullptr);
    ~MeshViewer();

    bool loadSTL(const QString &filePath);
    bool loadOBJ(const QString &filePath);
    bool loadSTEP(const QString &filePath);
    bool loadOpenFOAMCase(const QString &casePath);
    void clear();

    void setViewFront();
    void setViewBack();
    void setViewTop();
    void setViewBottom();
    void setViewLeft();
    void setViewRight();
    void setViewIsometric();
    void resetCamera();

    void setWireframe(bool enabled);
    void setSurface(bool enabled);
    void setSurfaceWithEdges(bool enabled);

signals:
    void meshLoaded(const QString &filePath);
    void loadError(const QString &error);

private slots:
    void onViewChanged(int index);

private:
    void setupUI();
    void setupVTKPipeline();
    void addActor(vtkActor *actor);
    void removeAllActors();
    void setCameraPos(double x, double y, double z, double fx, double fy, double fz);

    QVTKOpenGLNativeWidget      *m_vtkWidget;
    vtkGenericOpenGLRenderWindow *m_renderWindow;
    vtkRenderer                  *m_renderer;
    QVector<vtkActor*>           m_actors;

    QComboBox   *m_viewCombo;
    QPushButton *m_wireframeBtn;
    QPushButton *m_surfaceBtn;
    QPushButton *m_surfaceEdgeBtn;
    QPushButton *m_resetBtn;
    QLabel      *m_infoLabel;

    QString m_currentFile;
    double  m_bounds[6];
};

#endif // MESHVIEWER_H
