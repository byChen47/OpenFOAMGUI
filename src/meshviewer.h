#ifndef MESHVIEWER_H
#define MESHVIEWER_H
#include <QWidget>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
class QQuickWidget;
class MeshViewer : public QWidget {
    Q_OBJECT
public:
    explicit MeshViewer(QWidget *p=nullptr); ~MeshViewer();
    bool loadSTL(const QString &fp); bool loadOBJ(const QString &fp);
    bool loadSTEP(const QString &fp); bool loadOpenFOAMCase(const QString &cp);
    void clear();
    void setViewFront(); void setViewBack(); void setViewTop(); void setViewBottom();
    void setViewLeft(); void setViewRight(); void setViewIsometric();
    void resetCamera(); void setWireframe(bool on);
signals:
    void meshLoaded(const QString &fp); void loadError(const QString &e);
private slots:
    void onViewChanged(int i);
private:
    void setupUI(); void updateCameraView(const QString &v);
    QQuickWidget *m_qw;
    QComboBox *m_vc; QPushButton *m_wb,*m_rb; QLabel *m_il;
    QVector<QVector3D> m_vv; QVector<QVector<uint>> m_ff;
    bool m_wm=false; QString m_cf;
};
#endif
