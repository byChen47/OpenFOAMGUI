#include "ofparser.h"
#include <QRegularExpression>
#include <QFileInfo>

FoamFileHeader OFParser::parseHeader(const QString &content)
{
    FoamFileHeader header;

    QRegularExpression re(R"(FoamFile\s*\{(.*?)\})",
                          QRegularExpression::DotMatchesEverythingOption);
    auto match = re.match(content);
    if (!match.hasMatch())
        return header;

    QString block = match.captured(1);

    auto extract = [&](const QString &key) -> QString {
        QRegularExpression kRe(QString("%1\\s+(.+?);").arg(key));
        auto m = kRe.match(block);
        return m.hasMatch() ? m.captured(1).trimmed() : QString();
    };

    header.version  = extract("version");
    header.format   = extract("format");
    header.foamClass = extract("class");
    header.object   = extract("object");
    header.location = extract("location");
    header.note     = extract("note");

    return header;
}

bool OFParser::isOpenFOAMFile(const QString &content)
{
    return content.contains("FoamFile");
}

QString OFParser::guessFileCategory(const FoamFileHeader &header,
                                    const QString &relativePath)
{
    // Use relative path to infer time/folder context
    QStringList parts = relativePath.split('/', Qt::SkipEmptyParts);

    // Check if file belongs to a time directory (starts with digit)
    for (const auto &part : parts) {
        bool isTime = false;
        double d = part.toDouble(&isTime);
        Q_UNUSED(d)
        if (isTime) {
            if (header.foamClass.contains("ScalarField")
                || header.foamClass.contains("VectorField")
                || header.foamClass.contains("TensorField")) {
                return QString("Field data (time directory)");
            }
        }
    }

    if (relativePath.startsWith("0") || relativePath.startsWith("0.")) {
        return QString("Initial / boundary conditions (0/)");
    }
    if (relativePath.startsWith("constant")) {
        if (relativePath.contains("polyMesh"))
            return QString("Mesh definition (constant/polyMesh)");
        return QString("Physical properties (constant/)");
    }
    if (relativePath.startsWith("system")) {
        return QString("Simulation control (system/)");
    }

    return QString("OpenFOAM file");
}

QString OFParser::fileDescription(const QString &fileName)
{
    static QMap<QString, QString> desc;

    if (desc.isEmpty()) {
        // 0/ folder — field files
        desc["U"]          = "Velocity (volVectorField)";
        desc["p"]          = "Pressure (volScalarField)";
        desc["p_rgh"]      = "Pressure without hydrostatic component (volScalarField)";
        desc["k"]          = "Turbulent kinetic energy (volScalarField)";
        desc["epsilon"]    = "Turbulence dissipation rate (volScalarField)";
        desc["omega"]      = "Specific dissipation rate (volScalarField)";
        desc["nut"]        = "Turbulent viscosity (volScalarField)";
        desc["nuTilda"]    = "Modified turbulent viscosity — Spalart-Allmaras (volScalarField)";
        desc["alphat"]     = "Turbulent thermal diffusivity (volScalarField)";
        desc["alpha.water"]= "Volume fraction of water (volScalarField)";
        desc["alpha.air"]  = "Volume fraction of air (volScalarField)";
        desc["T"]          = "Temperature (volScalarField)";
        desc["rho"]        = "Density (volScalarField)";
        desc["thermo:rho"] = "Density (fluidThermo)";
        desc["h"]          = "Enthalpy (volScalarField)";
        desc["gh"]         = "Potential (gh)";
        desc["ghf"]        = "Potential at cell faces";
        // constant/ folder
        desc["transportProperties"] = "Transport model & fluid properties";
        desc["turbulenceProperties"]= "Turbulence model selection";
        desc["g"]          = "Gravitational acceleration (uniformDimensionedVectorField)";
        desc["thermophysicalProperties"] = "Thermophysical model & mixture properties";
        desc["phaseProperties"]  = "Multiphase phase definitions";
        desc["dynamicMeshDict"]  = "Dynamic mesh configuration";
        desc["MRFProperties"]    = "Multiple Reference Frame properties";
        desc["radiationProperties"] = "Radiation model properties";
        // system/ folder
        desc["controlDict"]    = "Time control, I/O settings";
        desc["fvSchemes"]      = "Finite volume discretisation schemes";
        desc["fvSolution"]     = "Linear solver & algorithm settings";
        desc["fvConstraints"]  = "Finite volume constraints";
        desc["fvModels"]       = "Finite volume models";
        desc["blockMeshDict"]  = "blockMesh mesh generator input";
        desc["snappyHexMeshDict"] = "snappyHexMesh mesher configuration";
        desc["decomposeParDict"]  = "Domain decomposition for parallel runs";
        desc["setFieldsDict"]     = "setFields initial field initialisation";
        desc["topoSetDict"]       = "topoSet cell/face/point set definitions";
        desc["surfaceFeatureExtractDict"] = "Surface feature extraction";
        desc["mapFieldsDict"]    = "Field mapping parameters";
        desc["forces"]           = "Force function object settings";
        desc["probes"]           = "Probe location definitions";
        desc["sampleDict"]       = "Sample/set extraction configuration";
        desc["residuals"]        = "Residual computation settings";
    }

    QFileInfo fi(fileName);
    QString base = fi.completeBaseName();
    return desc.value(base, QString());
}
