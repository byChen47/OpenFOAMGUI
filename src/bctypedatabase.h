#ifndef BCTYPEDATABASE_H
#define BCTYPEDATABASE_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

// Field category that a BC type applies to
enum class FieldCategory {
    Scalar,       // volScalarField: p, k, epsilon, omega, nut, alpha.*, T, ...
    Vector,       // volVectorField: U, ...
    Tensor,       // volTensorField
    SymmTensor,   // volSymmTensorField
    SphericalTensor,
    SurfaceScalar,
    SurfaceVector,
    Generic       // applies to all
};

// BC usage category for UI organisation
enum class BCCategory {
    Basic,        // fixedValue, zeroGradient, etc.
    Wall,         // noSlip, wall functions, etc.
    Inlet,        // inlet conditions
    Outlet,       // outlet conditions
    Symmetry,     // symmetry, cyclic, etc.
    Mapped,       // mapped / coupled
    Special,      // waveTransmissive, fan, etc.
    Coded,        // codedFixedValue, codedMixed
    Turbulence,   // turbulent inlet, etc.
    Constraint    // empty, wedge, cyclic, processor
};

// A single BC parameter
struct BCParameter {
    QString name;
    QString type;         // "scalar", "vector", "string", "scalarList", "bool"
    QString defaultValue;
    QString description;
    bool required = true;
};

// A boundary condition type entry
struct BCTypeInfo {
    QString name;                // e.g. "fixedValue"
    QString foamClass;           // e.g. "fixedValueFvPatchField"
    BCCategory category;
    QString description;
    QStringList appliesTo;       // field type strings: "scalar", "vector", "tensor", etc.
    QVector<BCParameter> requiredParams;
    QVector<BCParameter> optionalParams;
    QStringList aliases;         // searchable aliases
};

class BCTypeDatabase
{
public:
    BCTypeDatabase();
    static BCTypeDatabase* instance();

    // Query by field type
    QVector<BCTypeInfo> typesForField(const QString &fieldClass,
                                       const QString &fieldName = QString()) const;

    // Get full info for a specific type
    BCTypeInfo typeInfo(const QString &name) const;

    // All types
    QVector<BCTypeInfo> allTypes() const { return m_types; }

    // Search
    QVector<BCTypeInfo> search(const QString &keyword) const;

    // Get field category from FoamFile class string
    static FieldCategory categoryFromClass(const QString &foamClass);
    static QString categoryString(FieldCategory cat);

    // Generate dictionary snippet for a BC type
    static QString generateSnippet(const BCTypeInfo &info,
                                    const QString &patchName,
                                    const QString &fieldName = QString());

private:
    void initDatabase();
    QVector<BCTypeInfo> m_types;
};

#endif // BCTYPEDATABASE_H
