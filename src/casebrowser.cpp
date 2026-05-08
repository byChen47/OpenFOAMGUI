#include "casebrowser.h"
#include "ofparser.h"

#include <QHeaderView>
#include <QFileInfo>
#include <QDirIterator>
#include <QRegularExpression>
#include <QFileIconProvider>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <functional>

CaseBrowser::CaseBrowser(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_caseLabel = new QLabel("No case opened");
    m_caseLabel->setWordWrap(true);
    m_caseLabel->setStyleSheet(
        "QLabel { background: #E8E8E8; padding: 6px; font-weight: bold; "
        "border-bottom: 1px solid #CCC; }");
    layout->addWidget(m_caseLabel);

    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Filter files...");
    m_filterEdit->setClearButtonEnabled(true);
    layout->addWidget(m_filterEdit);

    m_tree = new QTreeWidget();
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(true);
    m_tree->setAnimated(true);
    m_tree->setIndentation(16);
    m_tree->setIconSize(QSize(16, 16));
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setStyleSheet(
        "QTreeWidget { font-size: 13px; }"
        "QTreeWidget::item { padding: 2px 0; }");

    layout->addWidget(m_tree);

    connect(m_tree, &QTreeWidget::itemDoubleClicked,
            this, &CaseBrowser::onItemDoubleClicked);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &CaseBrowser::onFilterTextChanged);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &CaseBrowser::onCustomContextMenu);
}

// ────────────────────────────────────────────────────────────────────
// Case management
// ────────────────────────────────────────────────────────────────────

void CaseBrowser::openCase(const QString &casePath)
{
    // Check for duplicates
    for (const auto &c : m_cases) {
        if (QDir(c).canonicalPath() == QDir(casePath).canonicalPath()) {
            // Already open — select it
            for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
                auto *item = m_tree->topLevelItem(i);
                if (item->data(0, Qt::UserRole).toString() == c) {
                    m_tree->setCurrentItem(item);
                    return;
                }
            }
            return;
        }
    }

    m_cases.append(casePath);

    QFileInfo fi(casePath);
    auto *caseRoot = new QTreeWidgetItem(m_tree);
    caseRoot->setText(0, fi.fileName());
    caseRoot->setIcon(0, style()->standardIcon(QStyle::SP_DirOpenIcon));
    caseRoot->setData(0, Qt::UserRole, casePath);
    caseRoot->setData(0, Qt::UserRole + 1, "caseroot");
    caseRoot->setToolTip(0, casePath);

    // Bold for case root items
    QFont f = caseRoot->font(0);
    f.setBold(true);
    caseRoot->setFont(0, f);

    populateCaseUnder(caseRoot, casePath);
    caseRoot->setExpanded(true);

    // Update label
    if (m_cases.size() == 1)
        m_caseLabel->setText("Case: " + fi.fileName());
    else
        m_caseLabel->setText(QString("%1 cases opened").arg(m_cases.size()));
    m_caseLabel->setToolTip(m_cases.join("\n"));

    m_tree->setCurrentItem(caseRoot);
    emit caseOpened(casePath);
}

// Recursively search the entire tree for an item whose UserRole path matches
QTreeWidgetItem* CaseBrowser::findItemByPath(const QString &path)
{
    std::function<QTreeWidgetItem*(QTreeWidgetItem*)> search =
        [&](QTreeWidgetItem *parent) -> QTreeWidgetItem* {
            int count = parent ? parent->childCount()
                               : m_tree->topLevelItemCount();
            for (int i = 0; i < count; ++i) {
                QTreeWidgetItem *item = parent ? parent->child(i)
                                               : m_tree->topLevelItem(i);
                if (item->data(0, Qt::UserRole).toString() == path)
                    return item;
                if (item->childCount() > 0) {
                    QTreeWidgetItem *found = search(item);
                    if (found) return found;
                }
            }
            return nullptr;
        };
    return search(nullptr);
}

void CaseBrowser::closeCase(const QString &casePath)
{
    // Recursively find the tree item (works at any nesting depth)
    QTreeWidgetItem *found = findItemByPath(casePath);

    if (!found) {
        // Path not found in tree — still remove from case list if present
        if (m_cases.removeAll(casePath) > 0)
            emit caseClosed(casePath);
        return;
    }

    QString type = found->data(0, Qt::UserRole + 1).toString();
    QString caserootPath = casePath;

    if (type == "subcase") {
        // Find the parent caseroot and close the entire case
        QTreeWidgetItem *parent = found->parent();
        while (parent) {
            if (parent->data(0, Qt::UserRole + 1).toString() == "caseroot") {
                caserootPath = parent->data(0, Qt::UserRole).toString();
                break;
            }
            parent = parent->parent();
        }
    }

    // Remove the top-level caseroot item (deletes all children)
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_tree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toString() == caserootPath) {
            delete m_tree->takeTopLevelItem(i);
            break;
        }
    }

    m_cases.removeAll(caserootPath);

    // Update label
    if (m_cases.isEmpty()) {
        m_caseLabel->setText("No case opened");
        m_caseLabel->setToolTip("");
    } else if (m_cases.size() == 1) {
        QFileInfo fi(m_cases.first());
        m_caseLabel->setText("Case: " + fi.fileName());
        m_caseLabel->setToolTip(m_cases.first());
    } else {
        m_caseLabel->setText(QString("%1 cases opened").arg(m_cases.size()));
        m_caseLabel->setToolTip(m_cases.join("\n"));
    }

    emit caseClosed(caserootPath);
}

void CaseBrowser::closeAllCases()
{
    while (!m_cases.isEmpty())
        closeCase(m_cases.first());
}

void CaseBrowser::refresh()
{
    QStringList cases = m_cases;
    closeAllCases();
    for (const auto &c : cases)
        openCase(c);
}

QString CaseBrowser::caseForFile(const QString &filePath) const
{
    for (const auto &c : m_cases) {
        QDir caseDir(c);
        QString canonicalFile = QDir(filePath).canonicalPath();
        QString canonicalCase = caseDir.canonicalPath();
        if (canonicalFile.startsWith(canonicalCase))
            return c;
    }
    return QString();
}

// ────────────────────────────────────────────────────────────────────
// Tree population for a single case
// ────────────────────────────────────────────────────────────────────

bool CaseBrowser::isTimeDirectory(const QString &name)
{
    if (name == "0" || name.startsWith("0."))
        return true;
    bool ok = false;
    name.toDouble(&ok);
    return ok;
}

void CaseBrowser::populateCaseUnder(QTreeWidgetItem *caseRoot,
                                     const QString &casePath)
{
    QDir caseDir(casePath);
    if (!caseDir.exists()) return;

    QStringList entries = caseDir.entryList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name);

    QStringList timeDirs, hasConstant, hasSystem, otherFiles, subDirs;

    for (const auto &entry : entries) {
        QFileInfo fi(caseDir.filePath(entry));
        if (fi.isDir()) {
            if (isTimeDirectory(entry))       timeDirs.append(entry);
            else if (entry == "constant")     hasConstant.append(entry);
            else if (entry == "system")       hasSystem.append(entry);
            else                              subDirs.append(entry);
        } else {
            otherFiles.append(entry);
        }
    }

    // Always populate standard dirs if present
    // Time dirs
    for (const auto &td : timeDirs) {
        auto *tdItem = new QTreeWidgetItem(caseRoot);
        tdItem->setText(0, td);
        tdItem->setIcon(0, style()->standardIcon(QStyle::SP_DirOpenIcon));
        QString tdPath = caseDir.filePath(td);
        tdItem->setData(0, Qt::UserRole, tdPath);
        tdItem->setData(0, Qt::UserRole + 1, "timedir");
        QDir tDir(tdPath);
        auto files = tDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const auto &f : files)
            createFileItem(f, tdItem);
    }

    // constant/
    if (!hasConstant.isEmpty()) {
        auto *ci = new QTreeWidgetItem(caseRoot);
        ci->setText(0, "constant");
        ci->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        ci->setData(0, Qt::UserRole, caseDir.filePath("constant"));
        ci->setData(0, Qt::UserRole + 1, "constantdir");
        QDir constDir(caseDir.filePath("constant"));
        auto cFiles = constDir.entryInfoList(
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);
        for (const auto &cf : cFiles) {
            if (cf.isDir()) {
                auto *sub = new QTreeWidgetItem(ci);
                sub->setText(0, cf.fileName());
                sub->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
                sub->setData(0, Qt::UserRole, cf.absoluteFilePath());
                sub->setData(0, Qt::UserRole + 1, "subdir");
                QDir sd(cf.absoluteFilePath());
                for (const auto &sf : sd.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
                    createFileItem(sf, sub);
            } else {
                createFileItem(cf, ci);
            }
        }
    }

    // system/
    if (!hasSystem.isEmpty()) {
        auto *si = new QTreeWidgetItem(caseRoot);
        si->setText(0, "system");
        si->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        si->setData(0, Qt::UserRole, caseDir.filePath("system"));
        si->setData(0, Qt::UserRole + 1, "systemdir");
        QDir sysDir(caseDir.filePath("system"));
        for (const auto &sf : sysDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
            createFileItem(sf, si);
    }

    // Other files at top level
    for (const auto &of : otherFiles)
        createFileItem(QFileInfo(caseDir.filePath(of)), caseRoot);

    // Handle sub-directories: EACH is checked independently
    // This supports overset mesh cases with multiple component dirs
    // (e.g. background/, overset/, component1/, component2/)
    for (const auto &sd : subDirs) {
        QDir sdDir(caseDir.filePath(sd));
        auto sdE = sdDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        bool isSubCase = false;
        for (const auto &sde : sdE) {
            if (sde == "constant" || sde == "system" || isTimeDirectory(sde)) {
                isSubCase = true;
                break;
            }
        }

        if (isSubCase) {
            // This sub-directory has OpenFOAM structure → treat as sub-case
            auto *subItem = new QTreeWidgetItem(caseRoot);
            subItem->setText(0, sd);
            subItem->setIcon(0, style()->standardIcon(QStyle::SP_DirOpenIcon));
            subItem->setData(0, Qt::UserRole, sdDir.absolutePath());
            subItem->setData(0, Qt::UserRole + 1, "subcase");
            QFont f = subItem->font(0);
            f.setBold(true);
            subItem->setFont(0, f);
            populateCaseUnder(subItem, sdDir.absolutePath());
        } else {
            // Regular directory — add as a browseable folder
            auto *odi = new QTreeWidgetItem(caseRoot);
            odi->setText(0, sd);
            odi->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            odi->setData(0, Qt::UserRole, caseDir.filePath(sd));
            odi->setData(0, Qt::UserRole + 1, "subdir");

            // Populate shallow contents of this dir
            QDir subD(caseDir.filePath(sd));
            auto sf = subD.entryInfoList(
                QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::DirsFirst | QDir::Name);
            for (const auto &sff : sf) {
                if (sff.isDir()) {
                    auto *dItem = new QTreeWidgetItem(odi);
                    dItem->setText(0, sff.fileName());
                    dItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
                    dItem->setData(0, Qt::UserRole, sff.absoluteFilePath());
                    dItem->setData(0, Qt::UserRole + 1, "subdir");
                } else {
                    createFileItem(sff, odi);
                }
            }
        }
    }
}

QTreeWidgetItem* CaseBrowser::createFileItem(const QFileInfo &fi,
                                              QTreeWidgetItem *parent)
{
    auto *item = new QTreeWidgetItem(parent ? parent : static_cast<QTreeWidgetItem*>(nullptr));
    if (!parent)
        m_tree->addTopLevelItem(item);

    item->setText(0, fi.fileName());
    item->setData(0, Qt::UserRole, fi.absoluteFilePath());
    item->setData(0, Qt::UserRole + 1, "file");

    QString desc = OFParser::fileDescription(fi.fileName());
    if (!desc.isEmpty())
        item->setToolTip(0, desc);

    QString base = fi.completeBaseName();
    if (base == "controlDict" || base == "fvSchemes" || base == "fvSolution"
        || base == "decomposeParDict" || base == "fvConstraints" || base == "fvModels")
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    else if (base == "U" || base == "p" || base == "p_rgh" || base == "k"
             || base == "epsilon" || base == "omega" || base == "nut"
             || base.startsWith("alpha.") || base == "T")
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileDialogContentsView));
    else
        item->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));

    return item;
}

// ────────────────────────────────────────────────────────────────────
// Interaction
// ────────────────────────────────────────────────────────────────────

void CaseBrowser::onItemDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
    QString type = item->data(0, Qt::UserRole + 1).toString();
    QString path = item->data(0, Qt::UserRole).toString();

    if (type == "file") {
        emit fileSelected(path);
    } else if (type == "caseroot" || type == "subcase") {
        emit caseActivated(path);
    }
}

void CaseBrowser::onCustomContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    if (!item) return;

    QString type = item->data(0, Qt::UserRole + 1).toString();
    QString path = item->data(0, Qt::UserRole).toString();

    QMenu menu;
    if (type == "caseroot" || type == "subcase") {
        QAction *closeAct = menu.addAction("Close Case");
        connect(closeAct, &QAction::triggered, [this, path]() { closeCase(path); });
        QAction *refreshAct = menu.addAction("Refresh Case");
        connect(refreshAct, &QAction::triggered, [this, path]() { closeCase(path); openCase(path); });
        menu.addSeparator();
    }

    // File/folder creation — available for any directory or case root
    if (type == "caseroot" || type == "subcase" || type == "timedir"
        || type == "constantdir" || type == "systemdir" || type == "subdir") {
        QAction *newFileAct = menu.addAction("New File...");
        connect(newFileAct, &QAction::triggered, [this, path]() {
            bool ok;
            QString name = QInputDialog::getText(this, "New File",
                "File name:", QLineEdit::Normal, "", &ok);
            if (ok && !name.isEmpty()) {
                QFile f(QDir(path).filePath(name));
                if (f.open(QFile::WriteOnly)) { f.close(); refresh(); emit filesystemChanged(); }
            }
        });
        QAction *newFolderAct = menu.addAction("New Folder...");
        connect(newFolderAct, &QAction::triggered, [this, path]() {
            bool ok;
            QString name = QInputDialog::getText(this, "New Folder",
                "Folder name:", QLineEdit::Normal, "", &ok);
            if (ok && !name.isEmpty()) {
                QDir(path).mkdir(name);
                refresh();
                emit filesystemChanged();
            }
        });
        menu.addSeparator();
    }

    // Delete — for files and all non-case-root dirs
    if (type == "file" || type == "subdir" || type == "timedir"
        || type == "constantdir" || type == "systemdir") {
        QAction *delAct = menu.addAction("Delete");
        delAct->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
        connect(delAct, &QAction::triggered, [this, path, type]() {
            QString msg = (type == "file")
                ? QString("Delete file:\n%1\n\nThis cannot be undone.").arg(path)
                : QString("Delete directory:\n%1\n\nAll contents will be removed. This cannot be undone.").arg(path);
            auto ret = QMessageBox::warning(this, "Confirm Delete", msg,
                QMessageBox::Ok | QMessageBox::Cancel);
            if (ret == QMessageBox::Ok) {
                if (type == "file") QFile::remove(path);
                else { QDir(path).removeRecursively(); }
                refresh();
                emit filesystemChanged();
            }
        });
    }

    // Open file
    if (type == "file") {
        QAction *openAct = menu.addAction("Open File");
        connect(openAct, &QAction::triggered, [this, path]() { emit fileSelected(path); });

        QAction *openWithAct = menu.addAction("Open With...");
        connect(openWithAct, &QAction::triggered, [this, path]() {
            QString exe = QFileDialog::getOpenFileName(
                this, "Choose a program to open this file",
                "C:/Windows/System32", "Executables (*.exe);;All Files (*.*)");
            if (!exe.isEmpty())
                QProcess::startDetached(exe, {QDir::toNativeSeparators(path)});
        });
    }

    if (!menu.isEmpty())
        menu.exec(m_tree->mapToGlobal(pos));
}

void CaseBrowser::newFile()
{
    QString dir = selectedPath();
    if (dir.isEmpty()) return;
    bool ok;
    QString name = QInputDialog::getText(this, "New File", "File name:",
        QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QString filePath = QDir(dir).filePath(name);
        QFile f(filePath);
        if (f.open(QFile::WriteOnly)) {
            f.close();
            // Insert item directly without full tree rebuild
            QFileInfo fi(filePath);
            QTreeWidgetItem *parent = m_tree->currentItem();
            if (parent && parent->data(0, Qt::UserRole + 1).toString() == "file")
                parent = parent->parent();
            createFileItem(fi, parent);
            emit filesystemChanged();
        }
    }
}

void CaseBrowser::newFolder()
{
    QString dir = selectedPath();
    if (dir.isEmpty()) return;
    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Folder name:",
        QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QString folderPath = QDir(dir).filePath(name);
        QDir(dir).mkdir(name);
        // Insert folder directly without full tree rebuild
        QTreeWidgetItem *parent = m_tree->currentItem();
        if (parent && parent->data(0, Qt::UserRole + 1).toString() == "file")
            parent = parent->parent();
        auto *item = new QTreeWidgetItem(parent);
        item->setText(0, name);
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        item->setData(0, Qt::UserRole, folderPath);
        item->setData(0, Qt::UserRole + 1, "subdir");
        emit filesystemChanged();
    }
}

void CaseBrowser::deleteSelected()
{
    QString path = selectedPath();
    if (path.isEmpty()) return;
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item) return;
    QString type = item->data(0, Qt::UserRole + 1).toString();
    if (type != "file" && type != "subdir" && type != "timedir"
        && type != "constantdir" && type != "systemdir") return;

    QString msg = (type == "file")
        ? QString("Delete file:\n%1\n\nThis cannot be undone.").arg(path)
        : QString("Delete directory:\n%1\n\nAll contents removed. Cannot be undone.").arg(path);
    if (QMessageBox::warning(this, "Confirm Delete", msg, QMessageBox::Ok | QMessageBox::Cancel)
        == QMessageBox::Ok) {
        if (type == "file") QFile::remove(path);
        else QDir(path).removeRecursively();
        // Remove tree item directly — no full rebuild needed
        delete item;
        emit filesystemChanged();
    }
}

QString CaseBrowser::selectedPath() const
{
    QTreeWidgetItem *item = m_tree->currentItem();
    if (!item) return QString();
    QString type = item->data(0, Qt::UserRole + 1).toString();
    if (type == "file") {
        QFileInfo fi(item->data(0, Qt::UserRole).toString());
        return fi.absolutePath();
    }
    return item->data(0, Qt::UserRole).toString();
}

void CaseBrowser::onFilterTextChanged(const QString &text)
{
    QString lower = text.toLower();

    std::function<void(QTreeWidgetItem*)> filterItem = [&](QTreeWidgetItem *item) {
        bool anyChildVisible = false;
        for (int i = 0; i < item->childCount(); ++i) {
            auto *child = item->child(i);
            filterItem(child);
            if (!child->isHidden())
                anyChildVisible = true;
        }
        if (item->childCount() == 0) {
            bool matches = lower.isEmpty()
                           || item->text(0).toLower().contains(lower);
            item->setHidden(!matches);
        } else {
            item->setHidden(!anyChildVisible);
            if (!lower.isEmpty() && anyChildVisible)
                item->setExpanded(true);
        }
    };

    for (int i = 0; i < m_tree->topLevelItemCount(); ++i)
        filterItem(m_tree->topLevelItem(i));
}
