#ifndef TURBULENCEPANEL_H
#define TURBULENCEPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QButtonGroup>
#include <QToolButton>

#include "turbulencemodeldatabase.h"

class CodeEditor;

class TurbulencePanel : public QWidget
{
    Q_OBJECT

public:
    explicit TurbulencePanel(QWidget *parent = nullptr);

    void loadTurbulenceFile(const QString &filePath, const QString &content);
    void setEditor(CodeEditor *editor) { m_editor = editor; }
    void clear();

signals:
    void insertSnippet(const QString &snippet);

private slots:
    void onCategoryButtonClicked(int catId);
    void onModelSelected(QListWidgetItem *item);
    void onModelDoubleClicked(QListWidgetItem *item);
    void onApplyConfig();
    void onPreviewConfig();

private:
    void setupUI();
    void buildCategoryTabs();
    void populateModels(const QVector<TurbModelInfo> &models);
    void showModelDetails(const TurbModelInfo &info);
    void parseCurrentConfig(const QString &content);

    // UI
    QLabel       *m_headerLabel;
    QLabel       *m_pathLabel;
    QWidget      *m_tabBar;
    QButtonGroup *m_tabGroup;
    QMap<int, QToolButton*> m_tabButtons;
    QListWidget  *m_modelList;
    QLabel       *m_countLabel;
    QLabel       *m_modelNameLabel;
    QLabel       *m_modelRefLabel;
    QLabel       *m_formulaLabel;
    QTableWidget *m_paramTable;
    QTextEdit    *m_previewEdit;
    QPushButton  *m_previewBtn;
    QPushButton  *m_applyBtn;

    // State
    QString      m_filePath;
    QString      m_currentContent;
    QString      m_currentSimType; // RAS, LES, laminar
    QString      m_currentModelName;
    TurbModelInfo m_selectedModel;
    CodeEditor  *m_editor = nullptr;
};

#endif // TURBULENCEPANEL_H
