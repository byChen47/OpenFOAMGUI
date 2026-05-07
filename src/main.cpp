#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QIcon>
#include <QDir>
#include <QSurfaceFormat>
#include <QProcessEnvironment>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Fix: Enigma Virtual Box needs explicit plugin paths BEFORE QApplication
    QString appDir = QCoreApplication::applicationDirPath();
    QCoreApplication::addLibraryPath(appDir);
    QCoreApplication::addLibraryPath(appDir + "/platforms");
    QCoreApplication::addLibraryPath(appDir + "/styles");
    QCoreApplication::addLibraryPath(appDir + "/imageformats");
    QCoreApplication::addLibraryPath(appDir + "/iconengines");
    qputenv("QT_QPA_PLATFORM_PLUGIN_PATH",
            (appDir + "/platforms").toUtf8());
    qputenv("QT_PLUGIN_PATH", appDir.toUtf8());

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
