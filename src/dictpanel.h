#ifndef DICTPANEL_H
#define DICTPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextEdit>
#include <QTreeWidget>
#include <QMap>

struct DictParam {
    QString name;
    QString type;
    QString defaultValue;
    QString description;
};

struct DictSection {
    QString name;
    QString description;
    QVector<DictParam> params;
    QString sampleBlock;
};

class CodeEditor;

class DictPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DictPanel(QWidget *parent = nullptr);

    void loadFile(const QString &filePath, const QString &content);
    void setEditor(CodeEditor *editor) { m_editor = editor; }
    void clear();

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
    void initBlockMeshData();
    void initTopoSetData();
    void initDynamicMeshData();
    void initControlDictData();
    void initDecomposeParDictData();
    void initRefineMeshDictData();
    void initTransportPropertiesData();
    void initThermophysicalData();
    void initRadiationData();
    void initCombustionData();
    void initSampleDictData();
    void initSetFieldsData();
    void initForcesData();
    void initFvConstraintsData();
    void initSurfaceFeatureExtractData();
    void initMapFieldsData();
    void initCreatePatchData();
    void initExtrudeMeshData();
    void initPostProcessData();
    void initWavePropertiesData();
    void initWaves2FoamData();
    void showSection(const DictSection &section);

    QLabel       *m_headerLabel;
    QLabel       *m_pathLabel;
    QListWidget  *m_sectionList;
    QLabel       *m_sectionDesc;
    QTreeWidget  *m_paramTree;
    QTextEdit    *m_previewEdit;

    QVector<DictSection> m_blockMeshSections;
    QVector<DictSection> m_topoSetSections;
    QVector<DictSection> m_dynamicMeshSections;
    QVector<DictSection> m_controlDictSections;
    QVector<DictSection> m_decomposeParDictSections;
    QVector<DictSection> m_refineMeshDictSections;
    QVector<DictSection> m_transportSections;
    QVector<DictSection> m_thermoSections;
    QVector<DictSection> m_radiationSections;
    QVector<DictSection> m_combustionSections;
    QVector<DictSection> m_sampleDictSections;
    QVector<DictSection> m_setFieldsSections;
    QVector<DictSection> m_forcesSections;
    QVector<DictSection> m_fvConstraintsSections;
    QVector<DictSection> m_surfaceFeatureSections;
    QVector<DictSection> m_mapFieldsSections;
    QVector<DictSection> m_createPatchSections;
    QVector<DictSection> m_extrudeMeshSections;
    QVector<DictSection> m_postProcessSections;
    QVector<DictSection> m_wavePropertiesSections;
    QVector<DictSection> m_waves2FoamSections;
    QVector<DictSection> *m_currentSections = nullptr;
    DictSection    m_currentSection;
    QMap<QString, QString> m_userValues;
    QString        m_filePath;
    QString        m_fileType;
    CodeEditor    *m_editor = nullptr;
};

#endif // DICTPANEL_H
