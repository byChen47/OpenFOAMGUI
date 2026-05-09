#ifndef CASEBROWSER_H
#define CASEBROWSER_H

#include <QWidget>
#include <QTreeWidget>
#include <QLineEdit>
#include <QLabel>
#include <QFileInfo>

class CaseBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit CaseBrowser(QWidget *parent = nullptr);

    void openCase(const QString &casePath);
    void closeCase(const QString &casePath);
    void closeAllCases();
    QStringList casePaths() const { return m_cases; }
    int caseCount() const { return m_cases.size(); }

    // Returns the case path that contains the given file path
    QString caseForFile(const QString &filePath) const;

    void refresh();

    void newFile();
    void newFolder();
    void deleteSelected();
    QString selectedPath() const;
    QTreeWidget* tree() const { return m_tree; }

signals:
    void fileSelected(const QString &filePath);
    void caseOpened(const QString &casePath);
    void caseClosed(const QString &casePath);
    void caseActivated(const QString &casePath);
    void filesystemChanged();

private slots:
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemExpanded(QTreeWidgetItem *item);
    void onFilterTextChanged(const QString &text);
    void onCustomContextMenu(const QPoint &pos);

private:
    void populateCaseUnder(QTreeWidgetItem *caseRoot, const QString &casePath);
    bool isTimeDirectory(const QString &name);
    QTreeWidgetItem* createFileItem(const QFileInfo &fi, QTreeWidgetItem *parent);
    QTreeWidgetItem* findItemByPath(const QString &path);

    QTreeWidget   *m_tree;
    QLineEdit     *m_filterEdit;
    QLabel        *m_caseLabel;
    QStringList    m_cases;
};

#endif // CASEBROWSER_H
