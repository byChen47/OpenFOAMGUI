#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QIcon>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Ensure Qt finds plugins when running inside Enigma Virtual Box
    QCoreApplication::addLibraryPath(
        QCoreApplication::applicationDirPath());
    QCoreApplication::addLibraryPath(
        QCoreApplication::applicationDirPath() + "/platforms");

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
