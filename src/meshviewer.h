#ifndef MESHVIEWER_H
#define MESHVIEWER_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QVector3D>

class QQuickWidget;
class QComboBox;
class QPushButton;
class QLabel;

class MeshViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MeshViewer(QWidget *parent = nullptr);
    ~MeshViewer();

    bool loadSTL(const QString &filePath);
    bool loadOBJ(const QString &filePath);
    bool loadOpenFOAMCase(const QString &casePath);
    void clear();

    // Preset views
    void setViewFront();
    void setViewBack();
    void setViewTop();
    void setViewBottom();
    void setViewLeft();
    void setViewRight();
    void setViewIsometric();

    void setWireframe(bool enabled);

signals:
    void meshLoaded(const QString &filePath);
    void loadError(const QString &error);

private slots:
    void onViewChanged(int index);
    void onResetView();

private:
    void setupUI();
    void loadMeshToScene(const QVector<QVector3D> &vertices,
                         const QVector<QVector<uint>> &faces,
                         const QVector<QVector3D> &faceColors);
    QWidget *create3DView();
    void updateCameraView(const QString &view);

    QQuickWidget *m_quickWidget;
    QComboBox    *m_viewCombo;
    QPushButton  *m_wireframeBtn;
    QPushButton  *m_resetBtn;
    QLabel       *m_infoLabel;

    QVector<QVector3D>     m_vertices;
    QVector<QVector<uint>> m_faces;
    QVector<QVector3D>     m_faceColors;
    bool                    m_wireframeMode = false;
    QString                 m_currentFile;
};

#endif // MESHVIEWER_H
