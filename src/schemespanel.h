#ifndef SCHEMESPANEL_H
#define SCHEMESPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QButtonGroup>
#include <QToolButton>
#include <QStackedWidget>
#include <QComboBox>
#include <QTreeWidget>

class CodeEditor;

struct SchemeOption {
    QString name;
    QString description;
    QString recommendation; // e.g. "2nd order, recommended"
};

struct SchemeCategory {
    QString name;           // e.g. "ddtSchemes", "gradSchemes"
    QString description;
    QString defaultScheme;
    QVector<SchemeOption> options;
};

class SchemesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SchemesPanel(QWidget *parent = nullptr);

    void loadFile(const QString &filePath, const QString &content);
    void setEditor(CodeEditor *editor) { m_editor = editor; }
    void clear();

signals:
    void insertSnippet(const QString &snippet);

private slots:
    void onFileTypeTabChanged(int index);
    void onSchemeCategoryChanged(QListWidgetItem *item);
    void onOptionClicked(QTreeWidgetItem *item, int col);
    void onApplySnippet();
    void onRefreshPreview();

private:
    void setupUI();
    void initSchemeData();
    void showSchemeCategory(const SchemeCategory &cat);
    void populateFileTypeTabs(bool isFvSchemes);
    void setupSolversView();
    QString buildFullConfig() const;
    QString buildSingleCategorySnippet(const SchemeCategory &cat, const QString &chosenOption) const;

    // UI
    QLabel        *m_headerLabel;
    QLabel        *m_pathLabel;
    QWidget       *m_tabBar;
    QButtonGroup  *m_tabGroup;
    QMap<int, QToolButton*> m_tabButtons;
    QListWidget   *m_categoryList;
    QLabel        *m_catDescLabel;
    QTreeWidget   *m_optionTree;
    QTextEdit     *m_previewEdit;
    QPushButton   *m_previewBtn;
    QPushButton   *m_applyBtn;
    QLabel        *m_recommendLabel;

    // Data
    QVector<SchemeCategory> m_schemeCategories;
    QVector<SchemeCategory> m_solverCategories;
    QString     m_filePath;
    QString     m_currentContent;
    QString     m_fileType;  // "fvSchemes" or "fvSolution"
    SchemeCategory m_currentCategory;
    CodeEditor *m_editor = nullptr;

    // User-modified scheme choices (section -> scheme name)
    QMap<QString, QString> m_userChoices;
};

#endif // SCHEMESPANEL_H
