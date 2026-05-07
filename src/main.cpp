#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QIcon>
#include <QSurfaceFormat>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // OpenGL 3.3 Compatibility Profile for 3D viewer shaders
    qputenv("QT_OPENGL", "desktop");
    QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setApplicationName("OpenFOAM GUI");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OpenFOAMGUI");
    app.setWindowIcon(QIcon(":/bychen.ico"));

    QSettings::setDefaultFormat(QSettings::IniFormat);

    MainWindow window;
    window.resize(1400, 900);
    window.show();

    return app.exec();
}
