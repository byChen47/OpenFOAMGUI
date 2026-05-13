#ifndef SNAPPYPANEL_H
#define SNAPPYPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QMap>

struct ShmParam {
    QString name;
    QString type;
    QString defaultValue;
    QString description;
};

struct ShmSection {
    QString name;
    QString description;
    QVector<ShmParam> params;
    QString sampleBlock;  // example full block
};

class CodeEditor;

class SnappyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SnappyPanel(QWidget *parent = nullptr);

    void loadFile(const QString &filePath, const QString &content);
    void setEditor(CodeEditor *editor) { m_editor = editor; }
    void clear();
    void onCalcBL(); // BL calculator (public for toolbar access)

signals:
    void insertSnippet(const QString &snippet);

private slots:
    void onSectionChanged(QListWidgetItem *item);
    void onParamContextMenu(const QPoint &pos);
    void onSectionContextMenu(const QPoint &pos);
    void onParamEdited(QTreeWidgetItem *item, int column);
    void updateSampleBlock();

private:
    void setupUI();
    void initData();
    void showSection(const ShmSection &section);

    QLabel       *m_headerLabel;
    QLabel       *m_pathLabel;
    QListWidget  *m_sectionList;
    QLabel       *m_sectionDesc;
    QTreeWidget  *m_paramTree;
    QTextEdit    *m_previewEdit;

    QVector<ShmSection> m_sections;
    ShmSection    m_currentSection;
    QMap<QString, QString> m_userValues; // paramName → user-edited value
    QString       m_filePath;
    CodeEditor   *m_editor = nullptr;
};

#endif // SNAPPYPANEL_H
