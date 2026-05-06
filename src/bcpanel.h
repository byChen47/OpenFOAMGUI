#ifndef BCPANEL_H
#define BCPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTreeWidget>
#include <QGroupBox>
#include <QTableWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QStackedWidget>

#include "bctypedatabase.h"

class CodeEditor;

class BCPanel : public QWidget
{
    Q_OBJECT

public:
    explicit BCPanel(QWidget *parent = nullptr);

    void loadFieldFile(const QString &filePath, const QString &content);
    void setEditor(CodeEditor *editor) { m_editor = editor; }
    void clear();

signals:
    void insertSnippet(const QString &snippet, int lineNumber);

private slots:
    void onCategoryButtonClicked(int catId);
    void onBCTypeSelected(QListWidgetItem *item);
    void onBCTypeDoubleClicked(QListWidgetItem *item);
    void onSearchTextChanged(const QString &text);
    void onPatchClicked(QTreeWidgetItem *item, int column);
    void onApplyBC();
    void onPreviewBC();
    void onParamCellChanged(int row, int col);
    void onBCListContextMenu(const QPoint &pos);
    void onInsertToEditor();

private:
    void setupUI();
    void populateBCTypes(const QVector<BCTypeInfo> &types);
    void showBCDetails(const BCTypeInfo &info);
    void parsePatches(const QString &content);
    void selectBCInList(const QString &bcTypeName);
    void suggestBCForPatch(const QString &patchName);
    void buildCategoryTabs();
    QString collectParamValues() const;

    // ── UI: Header ──
    QLabel      *m_fieldIconLabel;
    QLabel      *m_fieldInfoLabel;
    QLabel      *m_fieldPathLabel;

    // ── UI: Category tabs ──
    QWidget      *m_tabBar;
    QButtonGroup *m_tabGroup;
    QMap<int, QToolButton*> m_tabButtons;

    // ── UI: Search ──
    QLineEdit   *m_searchEdit;

    // ── UI: BC type list ──
    QListWidget *m_bcTypeList;
    QLabel      *m_typeCountLabel;

    // ── UI: Patch list ──
    QTreeWidget *m_patchTree;

    // ── UI: BC detail (OFFICIAL RTM FORMAT) ──
    QLabel      *m_bcNameLabel;
    QLabel      *m_bcDescLabel;

    // RTM table: Property | Description | Type | Required | Default
    QTableWidget *m_rtmTable;

    // Example code block
    QTextEdit   *m_exampleEdit;

    // Action buttons
    QPushButton *m_previewBtn;
    QPushButton *m_applyBtn;

    // ── State ──
    QString      m_fieldName;
    QString      m_fieldClass;
    QString      m_filePath;
    QString      m_currentContent;
    BCTypeInfo   m_selectedBC;
    QString      m_selectedPatchName;
    CodeEditor  *m_editor = nullptr;
    QVector<BCTypeInfo> m_currentTypes;
    int          m_activeCategory = -1;
    bool         m_updatingTable = false;  // prevent re-entrant signals
};

#endif // BCPANEL_H
