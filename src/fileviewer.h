#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QTabWidget>

class QLabel;

namespace FileViewer {

// Locate Ghostscript executable (bundled, PATH, TeX Live, recursive)
QString findGhostscript();

// Check file extension helpers
bool isImageFile(const QString &ext);
bool isOfficeFile(const QString &ext);
QString officeTypeName(const QString &ext);

// Create an image viewer widget with zoom controls
// Returns container widget, sets loaded=true/false, sets error if failed
QWidget *createImageViewer(const QString &filePath,
                           bool &loaded, QString &errorMsg);

} // namespace FileViewer

#endif // FILEVIEWER_H
