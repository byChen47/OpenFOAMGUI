#ifndef TURBULENCEMODELDATABASE_H
#define TURBULENCEMODELDATABASE_H

#include <QString>
#include <QStringList>
#include <QVector>

enum class TurbModelCategory {
    RAS,            // Reynolds-Averaged Navier-Stokes
    LES,            // Large Eddy Simulation
    DES,            // Detached Eddy Simulation
    ReynoldsStress, // RSTM
    Laminar         // Laminar / non-Newtonian
};

struct TurbParam {
    QString name;
    QString type;
    QString defaultValue;
    QString description;
};

struct TurbModelInfo {
    QString name;
    QString foamClass;
    TurbModelCategory category;
    QString description;
    QString formula;           // Key mathematical formula (LaTeX-compatible)
    QString references;
    QVector<TurbParam> params;
};

class TurbulenceModelDatabase
{
public:
    static TurbulenceModelDatabase* instance();

    QVector<TurbModelInfo> modelsByCategory(TurbModelCategory cat) const;
    QVector<TurbModelInfo> allModels() const { return m_models; }
    TurbModelInfo modelInfo(const QString &name) const;

    static QString categoryName(TurbModelCategory cat);
    static QString generateConfig(const TurbModelInfo &info);

private:
    TurbulenceModelDatabase();
    void initDatabase();
    QVector<TurbModelInfo> m_models;
};

#endif // TURBULENCEMODELDATABASE_H
