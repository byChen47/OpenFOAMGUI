#include "fileviewer.h"

#include <QApplication>
#include <QCoreApplication>
#include <QObject>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSvgWidget>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>

namespace FileViewer {

// ────────────────────────────────────────────────────────────────
//  Ghostscript detection
// ────────────────────────────────────────────────────────────────

QString findGhostscript()
{
    // 0. Bundled with the app
    QString localGs = QCoreApplication::applicationDirPath()
                      + "/ghostscript/bin/gswin64c.exe";
    if (QFileInfo::exists(localGs)) return localGs;

    // 1. PATH and well-known install locations
    QStringList paths = {
        "gswin64c", "gswin32c", "gs",
        "C:/Program Files/gs/gs10.04.0/bin/gswin64c.exe",
        "C:/Program Files/gs/gs10.03.1/bin/gswin64c.exe",
        "C:/Program Files/gs/gs10.03.0/bin/gswin64c.exe",
        "C:/Program Files/gs/gs9.56.1/bin/gswin64c.exe",
        "C:/Program Files/gs/gs9.55.0/bin/gswin64c.exe",
    };
    for (const auto &drive : {"C:", "D:", "E:"}) {
        for (int y = 2020; y <= 2027; ++y)
            paths.append(QString("%1/texlive/%2/tlpkg/tlgs/bin/gswin64c.exe")
                         .arg(drive).arg(y));
    }
    for (const auto &p : paths) {
        if (QFileInfo::exists(p)) return p;
    }

    // 2. Recursive search under LaTeX install dirs
    QStringList searchRoots;
    for (const auto &drive : {"C:/", "D:/", "E:/"}) {
        QDir root(drive);
        auto dirs = root.entryList({"*tex*", "*latex*", "*LaTex*", "*TeX*", "*texlive*"},
                                   QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &d : dirs)
            searchRoots.append(drive + d);
    }
    for (const auto &root : searchRoots) {
        QDirIterator it(root, {"gswin64c.exe"}, QDir::Files,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString candidate = it.filePath();
            if (candidate.contains("tlgs") || candidate.contains("ghostscript"))
                return candidate;
        }
    }

    // 3. PATH fallback
    QString f = QStandardPaths::findExecutable("gswin64c");
    if (!f.isEmpty()) return f;
    return QStandardPaths::findExecutable("gs");
}

// ────────────────────────────────────────────────────────────────
//  File type helpers
// ────────────────────────────────────────────────────────────────

bool isImageFile(const QString &ext)
{
    static QStringList exts = {
        "png", "jpg", "jpeg", "bmp", "gif", "webp", "ico",
        "svg", "eps", "epsf", "ps"
    };
    return exts.contains(ext);
}

bool isOfficeFile(const QString &ext)
{
    static QStringList exts = {
        "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx"
    };
    return exts.contains(ext);
}

QString officeTypeName(const QString &ext)
{
    if (ext == "pdf") return "PDF";
    if (ext == "doc" || ext == "docx") return "Word";
    if (ext == "xls" || ext == "xlsx") return "Excel";
    if (ext == "ppt" || ext == "pptx") return "PowerPoint";
    return "Document";
}

// ────────────────────────────────────────────────────────────────
//  Image viewer widget factory
// ────────────────────────────────────────────────────────────────

QWidget *createImageViewer(const QString &filePath,
                           bool &loaded, QString &errorMsg)
{
    QFileInfo fi(filePath);
    QString ext = fi.suffix().toLower();
    bool isEps = (ext == "eps" || ext == "epsf" || ext == "ps");
    bool isSvg = (ext == "svg");
    loaded = false;
    QPixmap basePixmap;

    auto *scrollArea = new QScrollArea();
    scrollArea->setAlignment(Qt::AlignCenter);

    if (isSvg) {
        auto *svgWidget = new QSvgWidget(filePath);
        svgWidget->setMinimumSize(100, 100);
        scrollArea->setWidget(svgWidget);
        loaded = true;
    } else if (isEps) {
        basePixmap = QPixmap(filePath);
        if (!basePixmap.isNull()) {
            loaded = true;
        } else {
            QString gsExe = findGhostscript();
            if (!gsExe.isEmpty()) {
                QString tmpPng = QDir::temp().filePath(
                    QString("ofgui_eps_%1.png")
                        .arg(fi.completeBaseName().replace(
                            QRegularExpression("[^a-zA-Z0-9_]"), "_")));
                QProcess gsProc;
                gsProc.start(gsExe, {"-dSAFER","-dBATCH","-dNOPAUSE",
                    "-sDEVICE=png16m","-r150",
                    "-sOutputFile=" + tmpPng, filePath});
                if (gsProc.waitForFinished(30000) && gsProc.exitCode() == 0
                    && QFileInfo::exists(tmpPng)) {
                    basePixmap = QPixmap(tmpPng);
                    if (!basePixmap.isNull()) loaded = true;
                }
            }
        }
    } else {
        basePixmap = QPixmap(filePath);
        if (!basePixmap.isNull()) loaded = true;
    }

    if (!loaded) {
        delete scrollArea;
        errorMsg = "Cannot render: " + fi.fileName()
                   + (isEps ? " (Ghostscript not found for EPS)" : "");
        return nullptr;
    }

    // Build viewer with zoom controls
    QWidget *container = new QWidget();
    auto *cl = new QVBoxLayout(container);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(2);

    // Zoom toolbar
    auto *zBar = new QHBoxLayout();
    zBar->setContentsMargins(4, 2, 4, 2);
    auto *zOut = new QPushButton(QString::fromUtf8("\xe2\x88\x92"));
    zOut->setFixedSize(28, 28); zOut->setToolTip("Zoom Out");
    auto *zIn  = new QPushButton("+");
    zIn->setFixedSize(28, 28); zIn->setToolTip("Zoom In");
    auto *zFit = new QPushButton("Fit");
    zFit->setFixedHeight(28); zFit->setToolTip("Fit to Window");
    auto *zOne = new QPushButton("1:1");
    zOne->setFixedHeight(28); zOne->setToolTip("Original Size");
    auto *zLab = new QLabel("100%");
    zLab->setFixedWidth(50); zLab->setAlignment(Qt::AlignCenter);
    zLab->setStyleSheet("font-size: 11px; color: #555;");
    zBar->addWidget(zOut); zBar->addWidget(zLab); zBar->addWidget(zIn);
    zBar->addWidget(zFit); zBar->addWidget(zOne); zBar->addStretch();
    cl->addLayout(zBar);

    cl->addWidget(scrollArea, 1);
    scrollArea->setWidgetResizable(isSvg);

    // Zoom logic for raster images
    if (!isSvg && !basePixmap.isNull()) {
        auto *imLab = new QLabel();
        imLab->setPixmap(basePixmap);
        imLab->setAlignment(Qt::AlignCenter);
        imLab->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        scrollArea->setWidget(imLab);

        double *zf = new double(1.0);
        auto applyZoom = [imLab, zLab](double f, QPixmap base) {
            QSize ns = base.size() * f;
            imLab->setPixmap(base.scaled(ns, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imLab->resize(ns);
            zLab->setText(QString("%1%").arg((int)(f * 100)));
        };
        auto fitWin = [imLab, zLab, scrollArea](QPixmap base) {
            QSize vs = scrollArea->viewport()->size() - QSize(4, 4);
            double f = qMin((double)vs.width() / base.width(),
                            (double)vs.height() / base.height());
            imLab->setPixmap(base.scaled(base.size() * f,
                            Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imLab->resize(base.size() * f);
            zLab->setText("Fit");
        };
        QPixmap baseCopy = basePixmap;
        QObject::connect(zIn, &QPushButton::clicked, [zf, applyZoom, baseCopy]() {
            *zf = qMin(*zf * 1.25, 10.0); applyZoom(*zf, baseCopy); });
        QObject::connect(zOut, &QPushButton::clicked, [zf, applyZoom, baseCopy]() {
            *zf = qMax(*zf / 1.25, 0.05); applyZoom(*zf, baseCopy); });
        QObject::connect(zFit, &QPushButton::clicked, [zf, fitWin, baseCopy]() {
            *zf = 0; fitWin(baseCopy); });
        QObject::connect(zOne, &QPushButton::clicked, [zf, applyZoom, baseCopy]() {
            *zf = 1.0; applyZoom(*zf, baseCopy); });
        QTimer::singleShot(100, scrollArea, [fitWin, baseCopy, scrollArea]() {
            fitWin(baseCopy); });
    }

    loaded = true;
    return container;
}

} // namespace FileViewer
