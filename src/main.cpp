#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QIcon>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Critical: Set plugin path BEFORE any Qt object is created
    // This is required for both Enigma Virtual Box and normal deployment
    QString appDir = QCoreApplication::applicationDirPath();
    qputenv("QT_QPA_PLATFORM_PLUGIN_PATH",
            QDir::toNativeSeparators(appDir + "/platforms").toUtf8());
    qputenv("QT_PLUGIN_PATH",
            QDir::toNativeSeparators(appDir).toUtf8());

    // Replace default library paths with our own
    QStringList paths;
    paths << appDir
          << appDir + "/platforms"
          << appDir + "/styles"
          << appDir + "/imageformats"
          << appDir + "/iconengines";
    QCoreApplication::setLibraryPaths(paths);

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
