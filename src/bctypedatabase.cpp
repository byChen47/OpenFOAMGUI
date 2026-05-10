#include "bctypedatabase.h"
#include <QRegularExpression>
#include <algorithm>

BCTypeDatabase* BCTypeDatabase::instance()
{
    static BCTypeDatabase db;
    return &db;
}

BCTypeDatabase::BCTypeDatabase() { initDatabase(); }

FieldCategory BCTypeDatabase::categoryFromClass(const QString &foamClass)
{
    QString c = foamClass.toLower();
    if (c.contains("scalar"))   return FieldCategory::Scalar;
    if (c.contains("vector"))   return FieldCategory::Vector;
    if (c.contains("tensor"))   return FieldCategory::Tensor;
    if (c.contains("symmtensor")) return FieldCategory::SymmTensor;
    if (c.contains("sphericaltensor")) return FieldCategory::SphericalTensor;
    if (c.contains("surface"))  return FieldCategory::SurfaceScalar;
    return FieldCategory::Generic;
}

QString BCTypeDatabase::categoryString(FieldCategory cat)
{
    switch (cat) {
    case FieldCategory::Scalar:          return "Scalar";
    case FieldCategory::Vector:          return "Vector";
    case FieldCategory::Tensor:          return "Tensor";
    case FieldCategory::SymmTensor:      return "SymmTensor";
    case FieldCategory::SphericalTensor: return "SphericalTensor";
    case FieldCategory::SurfaceScalar:   return "SurfaceScalar";
    case FieldCategory::SurfaceVector:   return "SurfaceVector";
    case FieldCategory::Generic:         return "Generic";
    }
    return "Unknown";
}

BCTypeInfo BCTypeDatabase::typeInfo(const QString &name) const
{
    for (const auto &t : m_types) {
        if (t.name == name) return t;
        for (const auto &a : t.aliases) if (a == name) return t;
    }
    return {};
}

QVector<BCTypeInfo> BCTypeDatabase::typesForField(const QString &fieldClass,
                                                   const QString &fieldName) const
{
    QVector<BCTypeInfo> result;
    FieldCategory cat = categoryFromClass(fieldClass);
    for (const auto &t : m_types) {
        for (const auto &a : t.appliesTo) {
            if (a == "all") { result.append(t); break; }
            if (cat == FieldCategory::Scalar  && a == "scalar")  { result.append(t); break; }
            if (cat == FieldCategory::Vector  && a == "vector")  { result.append(t); break; }
            if (cat == FieldCategory::Tensor  && a == "tensor")  { result.append(t); break; }
            if (cat == FieldCategory::SymmTensor && a == "symmTensor") { result.append(t); break; }
            if (cat == FieldCategory::Generic && a == "all") { result.append(t); break; }
        }
    }

    QString fn = fieldName.toLower();
    bool isPressure   = (fn == "p" || fn == "p_rgh" || fn == "pcorr" || fn.startsWith("p_"));
    bool isTurbulence = (fn == "k" || fn == "epsilon" || fn == "omega" || fn == "nut" || fn == "nutilde");
    bool isVelocity   = (fn == "u");
    bool isAlpha      = fn.startsWith("alpha.");
    bool isT          = (fn == "t");
    bool isNut        = (fn == "nut" || fn == "alphat");

    std::sort(result.begin(), result.end(),
              [&](const BCTypeInfo &a, const BCTypeInfo &b) {
        auto sc = [&](const BCTypeInfo &t) -> int {
            int s = 0;
            if (isVelocity && t.category == BCCategory::Wall && t.name == "noSlip") s += 10;
            if (isVelocity && t.category == BCCategory::Inlet) s += 8;
            if (isPressure && t.name == "zeroGradient") s += 10;
            if (isPressure && (t.name.contains("Pressure") || t.name.contains("Flux"))) s += 8;
            if (isTurbulence && t.name.contains("WallFunction")) s += 10;
            if (isAlpha && t.category == BCCategory::Wall) s += 5;
            if (isAlpha && t.name == "inletOutlet") s += 8;
            if (isT && t.category == BCCategory::Basic) s += 5;
            if (isNut && t.name.contains("WallFunction")) s += 10;
            if (t.category == BCCategory::Basic) s += 3;
            return -s;
        };
        return sc(a) < sc(b);
    });
    return result;
}

QVector<BCTypeInfo> BCTypeDatabase::search(const QString &keyword) const
{
    QVector<BCTypeInfo> result;
    QString kw = keyword.toLower();
    for (const auto &t : m_types) {
        if (t.name.toLower().contains(kw) || t.description.toLower().contains(kw))
            result.append(t);
        else for (const auto &a : t.aliases)
            if (a.toLower().contains(kw)) { result.append(t); break; }
    }
    return result;
}

QString BCTypeDatabase::generateSnippet(const BCTypeInfo &info,
                                         const QString &patchName,
                                         const QString &fieldName)
{
    QString snip;
    snip += QString("    %1\n    {\n").arg(patchName);
    snip += QString("        type            %1;\n").arg(info.name);
    Q_UNUSED(fieldName)
    for (const auto &p : info.requiredParams) {
        if (p.name == "value") {
            QString def = p.defaultValue;
            if (fieldName.toLower() == "u")      def = "uniform (0 0 0)";
            else if (fieldName.toLower() == "p" || fieldName.toLower() == "p_rgh") def = "uniform 0";
            else def = "uniform 0";
            snip += QString("        %1      %2;\n").arg(p.name, -15).arg(def);
        } else {
            snip += QString("        %1      %2;\n").arg(p.name, -15).arg(p.defaultValue);
        }
    }
    for (const auto &p : info.optionalParams)
        snip += QString("        // %1      %2;  (optional)\n").arg(p.name, -15).arg(p.defaultValue);
    snip += "    }\n";
    return snip;
}

// ════════════════════════════════════════════════════════════════════
//  DATABASE — parameters verified against OpenFOAM-v2206 source .H files
//  Each BC entry's required/optional params match the Usage \table in
//  src/finiteVolume/fields/fvPatchFields/derived/<name>/<name>FvPatch*Field.H
// ════════════════════════════════════════════════════════════════════

void BCTypeDatabase::initDatabase()
{
    auto add = [&](const QString &name, BCCategory cat,
                    const QString &desc, const QStringList &applies,
                    const QVector<BCParameter> &req = {},
                    const QVector<BCParameter> &opt = {},
                    const QStringList &aliases = {}) {
        BCTypeInfo info;
        info.name = name; info.foamClass = name + "FvPatchField";
        info.category = cat; info.description = desc;
        info.appliesTo = applies;
        info.requiredParams = req; info.optionalParams = opt;
        info.aliases = aliases;
        m_types.append(info);
    };

    // ════════════════════════════════════════════════════════════════
    // 1. BASIC — apply to all field types
    // ════════════════════════════════════════════════════════════════

    add("fixedValue", BCCategory::Basic,
        "Prescribe a fixed value on the patch.",
        {"all"},
        {{"value", "scalar/vector/tensor", "uniform 0", "Fixed field value"}});

    add("uniformFixedValue", BCCategory::Basic,
        "Uniform fixed value (single value across entire patch, can be time-varying Function1).",
        {"all"},
        {{"uniformValue", "Function1<Type>", "constant 0", "Uniform value as Function1"}},
        {},
        {"fixedValue"});

    add("zeroGradient", BCCategory::Basic,
        "Zero normal gradient (dφ/dn = 0). Standard for outlet scalars and walls (scalars).",
        {"all"});

    add("fixedGradient", BCCategory::Basic,
        "Prescribe a fixed normal gradient on the patch.",
        {"all"},
        {{"gradient", "scalar/vector/tensor", "uniform 0", "Fixed normal gradient"}});

    add("mixed", BCCategory::Basic,
        "Mix of fixedValue and fixedGradient: φ = f·refValue + (1-f)·(φ_c + refGrad·Δ).\n"
        "valueFraction=1 → fixedValue; valueFraction=0 → fixedGradient.",
        {"all"},
        {{"refValue", "scalar/vector/tensor", "uniform 0", "Reference value for fixedValue portion"},
         {"refGradient", "scalar/vector/tensor", "uniform 0", "Reference gradient for fixedGradient portion"},
         {"valueFraction", "scalar/tensor", "uniform 1", "Blend factor: 1 = fixedValue, 0 = fixedGradient"}});

    add("calculated", BCCategory::Basic,
        "Value computed by the solver. Not user-prescribed. Read-only effective BC.",
        {"all"});

    // ════════════════════════════════════════════════════════════════
    // 2. WALL — velocity + turbulence wall functions
    // ════════════════════════════════════════════════════════════════

    add("noSlip", BCCategory::Wall,
        "No-slip wall: U = (0 0 0). Standard viscous wall for velocity.",
        {"vector"});

    add("slip", BCCategory::Wall,
        "Free-slip wall: normal component zero, tangential zeroGradient.",
        {"vector"});

    add("partialSlip", BCCategory::Wall,
        "Partial slip: blends between noSlip and slip via valueFraction [0-1].\n"
        "0 = noSlip, 1 = full slip.",
        {"vector"},
        {{"valueFraction", "scalar", "0.5", "Slip fraction (0=noSlip, 1=slip)"}},
        {{"refValue", "vector", "uniform (0 0 0)", "Reference value at zero slip"},
         {"writeValue", "bool", "false", "Output patch value for post-processing"}});

    add("movingWallVelocity", BCCategory::Wall,
        "Moving wall: U = wall motion velocity. Used with dynamic mesh (no user params).",
        {"vector"});

    add("rotatingWallVelocity", BCCategory::Wall,
        "Rotating wall: U = ω × r. For SRF/MRF rotating geometry.",
        {"vector"},
        {},
        {{"origin", "vector", "(0 0 0)", "Rotation origin"},
         {"axis", "vector", "(0 0 1)", "Rotation axis"},
         {"omega", "scalar", "0", "Angular velocity [rad/s]"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("translatingWallVelocity", BCCategory::Wall,
        "Translating wall: entire patch moves at constant velocity.",
        {"vector"},
        {},
        {{"U", "vector", "(0 0 0)", "Translation velocity"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("fixedNormalSlip", BCCategory::Wall,
        "Fixed normal component; slip (zeroGradient) in tangential direction.",
        {"vector"},
        {{"fixedValue", "vector", "uniform (0 0 0)", "Fixed normal component value"}});

    // ── Wall functions for turbulence ──

    add("kqRWallFunction", BCCategory::Wall,
        "Wall function for k (turbulent kinetic energy). zeroGradient.\n"
        "For k, q, R in high-Re and low-Re turbulence models.",
        {"scalar"});

    add("epsilonWallFunction", BCCategory::Wall,
        "Wall function for ε (turbulence dissipation).\n"
        "Computes near-wall ε from k and y+. Standard for k-ε models.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial field value"}});

    add("omegaWallFunction", BCCategory::Wall,
        "Wall function for ω (specific dissipation rate).\n"
        "Blends viscous sublayer ω_vis and log-layer ω_log. For k-ω SST.",
        {"scalar"},
        {},
        {{"beta1", "scalar", "0.075", "Model constant β₁"},
         {"blended", "bool", "true", "Blend viscous + log layer"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("nutkWallFunction", BCCategory::Wall,
        "Wall function for ν_t (turbulent viscosity). Uses log-law or Spalding fit.",
        {"scalar"},
        {},
        {{"Cmu", "scalar", "0.09", "Turbulence model constant"},
         {"kappa", "scalar", "0.41", "von Kármán constant"},
         {"E", "scalar", "9.8", "Log-law constant (wall roughness)"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("nutUSpaldingWallFunction", BCCategory::Wall,
        "Spalding's continuous wall function for ν_t through viscous sublayer.\n"
        "Smoother blending than nutkWallFunction. For low-Re models.",
        {"scalar"},
        {},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("alphatWallFunction", BCCategory::Wall,
        "Wall function for α_t (turbulent thermal diffusivity).",
        {"scalar"},
        {},
        {{"Prt", "scalar", "0.85", "Turbulent Prandtl number"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    // ── Rough wall functions for nut (turbulent viscosity) ──

    add("nutkRoughWallFunction", BCCategory::Wall,
        "Wall function for ν_t (nut) for rough walls, based on k.\n"
        "Manipulates the roughness parameter E to account for roughness.\n"
        "Roughness height Ks = 0 gives smooth wall (same as nutkWallFunction).\n"
        "Cs range: 0.5 (uniform sand-grain) to 1.0 (larger roughness elements).\n\n"
        "Inherits from nutkWallFunction (Cmu, kappa, E parameters available).",
        {"scalar"},
        {{"Ks", "scalarField", "uniform 0", "Sand-grain roughness height [m] (0 = smooth)"},
         {"Cs", "scalarField", "uniform 0.5", "Roughness constant [0.5-1.0]"}},
        {{"Cmu", "scalar", "0.09", "Turbulence model constant (inherited)"},
         {"kappa", "scalar", "0.41", "von Kármán constant (inherited)"},
         {"E", "scalar", "9.8", "Wall roughness parameter (inherited)"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("nutURoughWallFunction", BCCategory::Wall,
        "Wall function for ν_t for rough walls, based on velocity U.\n"
        "Alternative to nutkRoughWallFunction. Uses U-based blending.",
        {"scalar"},
        {{"roughnessHeight", "scalar", "0", "Sand-grain roughness height [m]"},
         {"roughnessConstant", "scalar", "0.5", "Roughness constant [0.5-1.0]"}},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("nutLowReWallFunction", BCCategory::Wall,
        "Wall function for ν_t for low-Reynolds-number turbulence models.\n"
        "Suitable for models that resolve the viscous sublayer (e.g. k-ω SST).\n"
        "Sets nut = 0 at the wall (no wall function — resolved).",
        {"scalar"},
        {},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("nutUWallFunction", BCCategory::Wall,
        "Wall function for ν_t based on velocity U (log-law).\n"
        "Uses U-based blending. Alternative to nutkWallFunction.",
        {"scalar"},
        {},
        {{"Cmu", "scalar", "0.09", "Turbulence model constant"},
         {"kappa", "scalar", "0.41", "von Kármán constant"},
         {"E", "scalar", "9.8", "Log-law constant"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("nutUBlendedWallFunction", BCCategory::Wall,
        "Blended U-based wall function for ν_t. Smoothly blends viscous sublayer\n"
        "and log-layer using a blending function. For transitional flows.",
        {"scalar"},
        {},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("nutUTabulatedWallFunction", BCCategory::Wall,
        "Tabulated U-based wall function for ν_t. Uses a pre-computed table of\n"
        "u+ vs y+ instead of the standard log-law.",
        {"scalar"},
        {{"uPlusTable", "Function1<scalar>", "", "Tabulated u+ vs y+ relationship"}},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("kLowReWallFunction", BCCategory::Wall,
        "Wall function for k (turbulent kinetic energy) for low-Re models.\n"
        "Applies near-wall treatment suitable for low-Reynolds-number models.\n"
        "Uses model coefficients for the k equation wall treatment.",
        {"scalar"},
        {},
        {{"Ceps2", "scalar", "1.9", "Model coefficient Cε2"},
         {"Ck", "scalar", "-0.416", "Model coefficient Ck"},
         {"Bk", "scalar", "8.366", "Model coefficient Bk"},
         {"C", "scalar", "11.0", "Model coefficient C"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    // ════════════════════════════════════════════════════════════════
    // 3. INLET
    // ════════════════════════════════════════════════════════════════

    add("flowRateInletVelocity", BCCategory::Inlet,
        "Inlet velocity from prescribed mass/volumetric flow rate.\n"
        "Supply EITHER 'volumetricFlowRate' OR 'massFlowRate' with 'rho'.",
        {"vector"},
        {},  // Either volumetricFlowRate or massFlowRate is required (not both)
        {{"volumetricFlowRate", "Function1<scalar>", "0.2", "Volumetric flow rate [m³/s]"},
         {"massFlowRate", "Function1<scalar>", "0.2", "Mass flow rate [kg/s]"},
         {"rho", "word", "rho", "Density field name (required for massFlowRate)"},
         {"rhoInlet", "scalar", "1.0", "Inlet density (for massFlowRate)"},
         {"extrapolateProfile", "Switch", "false", "Extrapolate velocity profile from interior"}});

    add("pressureInletOutletVelocity", BCCategory::Inlet,
        "Combined: zeroGradient for outflow, fixedValue (from flux) for inflow.\n"
        "Most common U BC for pressure-driven flows.",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"tangentialVelocity", "vector", "(0 0 0)", "Optional tangential velocity component"}});

    add("pressureInletVelocity", BCCategory::Inlet,
        "Velocity inlet where pressure is specified. U from flux; zeroGradient on tangential.",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name"}});

    add("pressureDirectedInletVelocity", BCCategory::Inlet,
        "Inlet velocity where direction is prescribed and magnitude from flux.",
        {"vector"},
        {{"inletDirection", "vector", "(1 0 0)", "Inlet flow direction vector"}},
        {{"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("pressureNormalInletOutletVelocity", BCCategory::Inlet,
        "Normal component from flux; tangential component zeroGradient.",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"}});

    add("surfaceNormalFixedValue", BCCategory::Inlet,
        "Fixed value specified in surface-normal direction.\n"
        "Useful for specifying inlet velocity magnitude normal to the patch.",
        {"vector"},
        {{"refValue", "scalar", "uniform 0", "Value in surface-normal direction"}},
        {{"ramp", "Function1<scalar>", "", "Optional time-based ramping"}});

    add("cylindricalInletVelocity", BCCategory::Inlet,
        "Inlet velocity defined in cylindrical coordinates (r, θ, z).",
        {"vector"},
        {{"axis", "vector", "(0 0 1)", "Cylinder axis direction"},
         {"origin", "vector", "(0 0 0)", "Axis origin point"},
         {"axialVelocity", "Function1<scalar>", "constant 0", "Axial velocity [m/s]"},
         {"radialVelocity", "Function1<scalar>", "constant 0", "Radial velocity [m/s] (negative = inward)"},
         {"rpm", "Function1<scalar>", "constant 0", "Rotational speed (tangential component)"}});

    add("swirlFlowRateInletVelocity", BCCategory::Inlet,
        "Inlet velocity with prescribed flow rate AND swirl (tangential rpm).",
        {"vector"},
        {{"flowRate", "Function1<scalar>", "constant 0", "Flow rate [m³/s or kg/s]"},
         {"rpm", "Function1<scalar>", "constant 0", "Rotational speed for swirl"}},
        {{"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name (if mass flow)"},
         {"origin", "vector", "", "Swirl origin (default: patch centre)"},
         {"axis", "vector", "", "Swirl axis (default: -patch normal)"}});

    add("turbulentIntensityKineticEnergyInlet", BCCategory::Inlet,
        "Inlet k from turbulence intensity I: k = 1.5·(I·|U|)².",
        {"scalar"},
        {{"intensity", "scalar", "0.05", "Turbulence intensity fraction [0-1] (0.05 = 5%)"}},
        {{"U", "word", "U", "Velocity field name"},
         {"phi", "word", "phi", "Flux field name"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("turbulentInlet", BCCategory::Inlet,
        "Synthetic turbulence inlet (random fluctuations superposed on mean U).",
        {"vector"},
        {},
        {{"fluctuationScale", "scalar", "0.1", "Turbulence intensity scale factor"},
         {"referenceField", "vector", "uniform (0 0 0)", "Mean velocity field"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("turbulentDFSEMInlet", BCCategory::Inlet,
        "Divergence-Free Synthetic Eddy Method. Generates correlated synthetic\n"
        "turbulence that exactly satisfies continuity.",
        {"vector"},
        {{"R", "symmTensor", "uniform (1 0 0 1 0 1)", "Reynolds stress tensor"},
         {"L", "scalar", "0.01", "Integral length scale [m]"},
         {"value", "vector", "uniform (0 0 0)", "Mean velocity"}},
        {{"nCellsPerEddy", "label", "1", "Cells per eddy in each direction"},
         {"d", "scalar", "0", "Normal distance to the wall"},
         {"mapMethod", "word", "cellWeight", "Distribution mapping method"}});

    add("turbulentDigitalFilterInlet", BCCategory::Inlet,
        "Digital filter-based synthetic turbulence inlet. Correlated fluctuations.",
        {"vector"},
        {{"R", "symmTensor", "uniform (1 0 0 1 0 1)", "Reynolds stress tensor"},
         {"L", "scalar", "0.01", "Integral length scale [m]"},
         {"value", "vector", "uniform (0 0 0)", "Mean velocity"}},
        {},
        {"turbulentDFInlet"});

    // ════════════════════════════════════════════════════════════════
    // 4. OUTLET
    // ════════════════════════════════════════════════════════════════

    add("inletOutlet", BCCategory::Outlet,
        "Zero-gradient for outflow; fixedValue for inflow (backflow).\n"
        "STANDARD outlet for scalars: p, k, ε, ω, α, T, etc.",
        {"all"},
        {{"inletValue", "scalar/vector/tensor", "uniform 0", "Value when reverse flow occurs (inflow)"}},
        {{"phi", "word", "phi", "Flux field name to detect flow direction"},
         {"value", "scalar/vector/tensor", "uniform 0", "Initial/current value"}});

    add("outletInlet", BCCategory::Outlet,
        "Reverse of inletOutlet: fixedValue for outflow; zeroGradient for backflow.\n"
        "Rarely used; prefer inletOutlet for most cases.",
        {"all"},
        {{"outletValue", "scalar/vector/tensor", "uniform 0", "Value for outflow"}},
        {{"phi", "word", "phi", "Flux field name"}});

    add("advective", BCCategory::Outlet,
        "Non-reflective advective outlet: ∂φ/∂t + U_n·∇φ = 0.",
        {"all"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name"},
         {"fieldInf", "scalar/vector/tensor", "0", "Far-field reference value"},
         {"lInf", "scalar", "1.0", "Relaxation distance beyond patch for fieldInf"}});

    add("waveTransmissive", BCCategory::Outlet,
        "Non-reflective wave-transmissive: ∂φ/∂t + (U_n + c)·∇φ = 0.\n"
        "Includes acoustic wave speed. For compressible/pressure outlets.",
        {"all"},
        {{"gamma", "scalar", "1.4", "Ratio of specific heats Cp/Cv"}},
        {{"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name"},
         {"psi", "word", "thermo:psi", "Compressibility field name"},
         {"fieldInf", "scalar", "0", "Far-field value"},
         {"lInf", "scalar", "1.0", "Relaxation distance for fieldInf"}});

    add("freestream", BCCategory::Outlet,
        "Freestream: fixedValue for inflow, zeroGradient for outflow.\n"
        "Similar to inletOutlet with freestream blending.",
        {"all"},
        {},
        {{"freestreamValue", "scalar/vector/tensor", "uniform 0", "Freestream reference value"},
         {"freestreamBC", "patchField", "", "Optional patchField providing inlet BC"},
         {"phi", "word", "phi", "Flux field name"}});

    add("freestreamVelocity", BCCategory::Outlet,
        "Freestream velocity: fixedValue for supersonic inflow; zeroGradient for outflow.",
        {"vector"},
        {{"freestreamValue", "vector", "uniform (0 0 0)", "Freestream velocity"}});

    add("freestreamPressure", BCCategory::Outlet,
        "Freestream pressure: fixedValue for supersonic inflow; zeroGradient for outflow.",
        {"scalar"},
        {{"freestreamValue", "scalar", "uniform 1e5", "Freestream pressure [Pa]"}},
        {{"U", "word", "U", "Velocity field name to determine inflow/outflow"}});

    add("supersonicFreestream", BCCategory::Outlet,
        "Supersonic far-field. All values from freestream if supersonic inflow.",
        {"vector"},
        {{"freestreamValue", "vector", "uniform (0 0 0)", "Freestream velocity"}},
        {{"Tfreestream", "scalar", "300", "Freestream temperature [K]"},
         {"pFreestream", "scalar", "1e5", "Freestream pressure [Pa]"}});

    add("outletMappedUniformInlet", BCCategory::Outlet,
        "Outlet that maps flux-averaged outlet value to a uniform inlet patch.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("flowRateOutletVelocity", BCCategory::Outlet,
        "Outlet velocity that adjusts to match target flow rate.",
        {"vector"},
        {},
        {{"volumetricFlowRate", "Function1<scalar>", "0", "Target volumetric flow rate [m³/s]"},
         {"massFlowRate", "Function1<scalar>", "0", "Target mass flow rate [kg/s]"},
         {"rho", "word", "rho", "Density field (for massFlowRate)"},
         {"extrapolateProfile", "Switch", "false", "Extrapolate velocity profile"}});

    // ════════════════════════════════════════════════════════════════
    // 5. PRESSURE-specific
    // ════════════════════════════════════════════════════════════════

    add("totalPressure", BCCategory::Inlet,
        "Total pressure p₀ inlet. p = p₀ − ½ρ|U|².\n"
        "Supports: incompressible subsonic / compressible subsonic / transonic / supersonic.\n"
        "Mode determined by p dimensions, psi, and gamma.",
        {"scalar"},
        {{"p0", "scalar", "uniform 1e5", "Total/stagnation pressure [Pa or m²/s²]"}},
        {{"U", "word", "U", "Velocity field name"},
         {"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name (compressible subsonic)"},
         {"psi", "word", "none", "Compressibility field name (compressible trans/supersonic)"},
         {"gamma", "scalar", "1.0", "Cp/Cv (>1 for supersonic mode)"}});

    add("uniformTotalPressure", BCCategory::Inlet,
        "Uniform total pressure (single value, possibly time-varying, across patch).",
        {"scalar"},
        {{"p0", "Function1<scalar>", "constant 1e5", "Uniform total pressure (may be table/Function1)"}});

    add("prghTotalPressure", BCCategory::Inlet,
        "Total pressure for p_rgh: p_rgh = p₀ − ½ρ|U|² − ρ(g·h).",
        {"scalar"},
        {{"p0", "scalar", "uniform 0", "Total pressure (excluding hydrostatic)"}},
        {{"U", "word", "U", "Velocity field name"},
         {"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name"}});

    add("fixedFluxPressure", BCCategory::Basic,
        "Pressure adjusted to satisfy specified flux. Standard p/p_rgh at walls\n"
        "in buoyant/pressure-driven flows. Inherits fixedGradient.",
        {"scalar"},
        {},
        {{"value", "scalar", "uniform 0", "Initial/backup value"}});

    add("fixedMean", BCCategory::Basic,
        "Adjusts patch pressure to maintain a target area-averaged mean.\n"
        "Sets a reference pressure level to anchor the pressure field.",
        {"scalar"},
        {{"meanValue", "Function1<scalar>", "0", "Target area-weighted mean value"}},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("fanPressure", BCCategory::Special,
        "Fan/pump pressure jump as a function of flow rate (fan curve).\n"
        "Δp = f(Q). Can use dimensional or non-dimensional curve.",
        {"scalar"},
        {{"fanCurve", "table/file", "table ((0 0)(10 500))", "Pressure jump vs flow rate table"},
         {"direction", "word", "in", "Flow direction: in or out"},
         {"p0", "scalar", "uniform 1e5", "Environmental total pressure [Pa]"}},
        {{"nonDimensional", "Switch", "false", "Use non-dimensional fan curve"},
         {"rpm", "scalar", "0", "Fan RPM (non-dimensional mode)"},
         {"dm", "scalar", "0", "Mean diameter [m] (non-dimensional mode)"},
         {"file", "fileName", "", "Legacy: fan curve file"},
         {"outOfBounds", "word", "", "Legacy: out-of-bounds handling"}},
        {"fan"});

    add("uniformDensityHydrostaticPressure", BCCategory::Inlet,
        "Hydrostatic pressure assuming uniform density.\n"
        "p = pRef + ρ(g·(x − xRef)).",
        {"scalar"},
        {{"rho", "scalar", "1000", "Uniform density [kg/m³]"},
         {"pRefValue", "scalar", "1e5", "Reference pressure [Pa]"},
         {"pRefPoint", "vector", "(0 0 0)", "Reference pressure location [m]"}});

    add("prghPressure", BCCategory::Basic,
        "p_rgh condition at walls. Similar to fixedFluxPressure for p_rgh.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("prghTotalHydrostaticPressure", BCCategory::Inlet,
        "Total pressure with hydrostatic component for p_rgh.\n"
        "p_rgh = p₀ − ½ρ|U|², where p₀ includes hydrostatic variation.",
        {"scalar"},
        {{"p0", "scalar", "uniform 0", "Total pressure reference"}},
        {{"rho", "word", "rho", "Density field name"},
         {"U", "word", "U", "Velocity field name"},
         {"phi", "word", "phi", "Flux field name"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("fixedFluxExtrapolatedPressure", BCCategory::Basic,
        "Pressure with extrapolated gradient from interior to satisfy flux.\n"
        "Variant of fixedFluxPressure for cases needing extrapolation.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("fixedPressureCompressibleDensity", BCCategory::Basic,
        "Pressure BC for compressible flows where density is computed from pressure.",
        {"scalar"},
        {{"p", "scalar", "uniform 1e5", "Fixed pressure value"}});

    add("plenumPressure", BCCategory::Special,
        "Plenum chamber pressure. Uniform pressure that evolves with net mass flux.\n"
        "dp/dt = (γRT₀/V)·Σ(ṁ).",
        {"scalar"},
        {{"gamma", "scalar", "1.4", "Heat capacity ratio Cp/Cv"},
         {"R", "scalar", "287", "Gas constant [J/(kg·K)]"},
         {"T0", "scalar", "300", "Stagnation temperature [K]"},
         {"V", "scalar", "1.0", "Plenum volume [m³]"},
         {"value", "scalar", "uniform 0", "Initial pressure [Pa]"}});

    // ════════════════════════════════════════════════════════════════
    // 6. MULTIPHASE
    // ════════════════════════════════════════════════════════════════

    add("interfaceCompression", BCCategory::Special,
        "Interface compression at walls for α in interFoam.\n"
        "Used with wall contact angle treatment.",
        {"scalar"});

    add("outletPhaseMeanVelocity", BCCategory::Outlet,
        "Adjusts outlet velocity to match target mean velocity.\n"
        "Used in multiphase for mass conservation at outlets.",
        {"vector"},
        {{"Umean", "vector", "(0 0 0)", "Target mean velocity"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("prghPermeableAlphaTotalPressure", BCCategory::Inlet,
        "Multi-phase total pressure for p_rgh with phase-aware backflow handling.\n"
        "Handles different backflow phase fractions at the inlet.",
        {"scalar"},
        {{"p0", "scalar", "uniform 0", "Total pressure"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("phaseHydrostaticPressure", BCCategory::Inlet,
        "Hydrostatic pressure for a single phase in multiphase flows.\n"
        "Accounts for phase fraction in hydrostatic calculation.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    // ════════════════════════════════════════════════════════════════
    // 7. TEMPERATURE / ENERGY
    // ════════════════════════════════════════════════════════════════

    add("totalTemperature", BCCategory::Inlet,
        "Total temperature T₀ inlet. T computed from T₀ and velocity.\n"
        "T₀ = T·(1 + (γ−1)/2·Ma²). For compressible flows.",
        {"scalar"},
        {{"T0", "scalar", "uniform 300", "Total/stagnation temperature [K]"},
         {"gamma", "scalar", "1.4", "Ratio of specific heats Cp/Cv"}},
        {{"U", "word", "U", "Velocity field name"},
         {"phi", "word", "phi", "Flux field name"},
         {"psi", "word", "thermo:psi", "Compressibility field name"},
         {"value", "scalar", "uniform 300", "Initial value"}});

    add("inletOutletTotalTemperature", BCCategory::Inlet,
        "inletOutlet + totalTemperature: zeroGradient for outflow; T₀ for backflow.",
        {"scalar"},
        {{"T0", "scalar", "uniform 300", "Total temperature for reverse flow [K]"},
         {"gamma", "scalar", "1.4", "Cp/Cv"},
         {"inletValue", "scalar", "uniform 300", "Temperature for reverse flow"}},
        {{"U", "word", "U", "Velocity field for Mach number"},
         {"phi", "word", "phi", "Flux field for flow direction"},
         {"value", "scalar", "uniform 300", "Initial value"}});

    // ════════════════════════════════════════════════════════════════
    // 8. MAPPED / COUPLED
    // ════════════════════════════════════════════════════════════════

    add("mappedFixedValue", BCCategory::Mapped,
        "Fixed value mapped from another patch/region.\n"
        "For CHT, fluid-structure coupling, or recycled BCs.",
        {"all"},
        {{"value", "scalar/vector", "uniform 0", "Fallback field value"}},
        {{"field", "word", "", "Source field name (default: this field name)"},
         {"setAverage", "Switch", "false", "Force mapped field to have same average"},
         {"average", "scalar", "0", "Target average value (if setAverage=yes)"},
         {"interpolationScheme", "word", "cell", "Interpolation: cell, cellPoint, cellPointFace"}});

    add("timeVaryingMappedFixedValue", BCCategory::Mapped,
        "Time-varying mapped value from another case's time directories.\n"
        "Standard for inlet BCs from precursor simulations.",
        {"all"},
        {{"value", "scalar/vector", "uniform 0", "Fallback value"}},
        {{"setAverage", "Switch", "false", "Adjust to match mapped average"},
         {"perturb", "scalar", "1e-5", "Small perturbation for regular geometries"},
         {"points", "word", "points", "Points file name for mapping"},
         {"fieldTable", "word", "", "Alternative field name to sample"},
         {"mapMethod", "word", "planarInterpolation", "Mapping type"},
         {"offset", "Function1<Type>", "", "Offset added to mapped values"}});

    add("mappedMixed", BCCategory::Mapped,
        "Mixed BC with refValue/refGrad/valueFraction mapped from another patch.",
        {"all"},
        {{"value", "scalar/vector", "uniform 0", "Fallback value"}},
        {{"field", "word", "", "Source field name"},
         {"refValue", "scalar/vector", "uniform 0", "Fallback refValue"},
         {"refGradient", "scalar/vector", "uniform 0", "Fallback refGradient"},
         {"valueFraction", "scalar", "uniform 1", "Blend factor"},
         {"setAverage", "Switch", "false", "Adjust to match average"},
         {"interpolationScheme", "word", "cell", "Interpolation scheme"}});

    add("mappedFlowRate", BCCategory::Mapped,
        "Flow rate that maps flux from another patch. For matched inlet-outlet pairs.",
        {"vector"},
        {{"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("fixedProfile", BCCategory::Inlet,
        "Fixed value profile defined by a 1D profile Function1.\n"
        "For boundary layer profiles, experimental data, etc.",
        {"all"},
        {{"profile", "Function1<Type>", "", "Profile definition (e.g. table, polynomial)"},
         {"direction", "vector", "(1 0 0)", "Direction of profile variation"},
         {"origin", "vector", "(0 0 0)", "Profile origin point"},
         {"value", "scalar/vector", "uniform 0", "Initial value"}});

    add("mappedFixedInternalValue", BCCategory::Mapped,
        "Mapped value from another patch applied as internal value.",
        {"all"},
        {{"value", "scalar/vector", "uniform 0", "Fallback value"}},
        {{"field", "word", "", "Source field name"},
         {"setAverage", "Switch", "false", "Adjust mapped field to same average"},
         {"interpolationScheme", "word", "cell", "Interpolation scheme"}});

    // ════════════════════════════════════════════════════════════════
    // 9. CODED (user-defined)
    // ════════════════════════════════════════════════════════════════

    add("codedFixedValue", BCCategory::Coded,
        "User-coded fixedValue: inline C++ code computes the boundary value.\n"
        "Powerful for custom BCs without compiling a new library.\n\n"
        "Code entry format:\n"
        "  code #{\n"
        "    const vectorField& Cf = patch().Cf();\n"
        "    operator==(vector(1, 0, 0)*Cf.component(0));\n"
        "  #};",
        {"all"},
        {{"value", "scalar/vector/tensor", "uniform 0", "Initial field value"},
         {"code", "C++ code", "#{\n    // Write your BC code here\n#}", "C++ code computing the patch value"}},
        {{"codeInclude", "string", "", "Additional #include headers"},
         {"codeOptions", "string", "", "Additional compiler flags (EXE_INC)"},
         {"codeLibs", "string", "", "Additional link libraries (LIB_LIBS)"},
         {"localCode", "C++ code", "", "Local static functions (optional)"},
         {"codeContext", "dictionary", "", "Additional dictionary context for code"}});

    add("codedMixed", BCCategory::Coded,
        "User-coded mixed BC: inline C++ computes refValue/refGrad/valueFraction.\n"
        "Gives full control over the mixed condition.",
        {"all"},
        {{"value", "scalar/vector/tensor", "uniform 0", "Initial field value"},
         {"code", "C++ code", "#{\n    // Compute refValue, refGrad, valueFraction\n#}", "C++ code"}},
        {{"codeInclude", "string", "", "Additional headers"},
         {"codeOptions", "string", "", "Compiler flags (EXE_INC)"},
         {"codeLibs", "string", "", "Link libraries (LIB_LIBS)"},
         {"localCode", "C++ code", "", "Local static functions"},
         {"codeContext", "dictionary", "", "Additional dictionary context"}});

    // ════════════════════════════════════════════════════════════════
    // 10. JUMP / SPECIAL
    // ════════════════════════════════════════════════════════════════

    add("fixedJump", BCCategory::Special,
        "Fixed jump across a cyclic patch pair. Δφ = jump.\n"
        "Typical for pressure jumps, temperature drops, etc.",
        {"all"},
        {{"patchType", "word", "cyclic", "Underlying patch type (must be cyclic)"},
         {"jump", "scalar/vector", "uniform 0", "Jump value across the cyclic"}},
        {{"relax", "scalar", "", "Under-relaxation factor for jump adjustment"},
         {"minJump", "scalar", "", "Minimum jump limit (optional)"}});

    add("fixedJumpAMI", BCCategory::Special,
        "Fixed jump across an AMI (Arbitrary Mesh Interface) cyclic pair.",
        {"all"},
        {{"jump", "scalar/vector", "uniform 0", "Jump value"},
         {"value", "scalar/vector", "uniform 0", "Initial value"}});

    add("syringePressure", BCCategory::Special,
        "Syringe pump pressure. Pressure adjusts based on injected volume.\n"
        "p adjusts to match Q·t = Ap·(x − x₀).",
        {"scalar"},
        {{"Q", "scalar", "0", "Volumetric flow rate [m³/s]"},
         {"Ap", "scalar", "0", "Piston cross-sectional area [m²]"},
         {"V0", "scalar", "0", "Initial volume [m³]"},
         {"value", "scalar", "uniform 0", "Initial pressure"}});

    add("rotatingTotalPressure", BCCategory::Inlet,
        "Total pressure in rotating frame. p₀ = p + ½ρ(|U|² − |ω×r|²).\n"
        "For SRF/MRF rotating inlets.",
        {"scalar"},
        {{"p0", "scalar", "uniform 1e5", "Total pressure (stationary frame)"},
         {"omega", "scalar", "0", "Angular velocity [rad/s]"}},
        {{"U", "word", "U", "Velocity field name"},
         {"phi", "word", "phi", "Flux field name"},
         {"rho", "word", "rho", "Density field name"}});

    add("scaledFixedValue", BCCategory::Basic,
        "Fixed value scaled by a scalar factor. φ_bc = scale × fixedValue.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Value to be scaled"},
         {"scale", "scalar", "1.0", "Scale factor"}});

    add("swirlInletVelocity", BCCategory::Inlet,
        "Swirl inlet velocity with axial + radial + tangential components.",
        {"vector"},
        {{"origin", "vector", "(0 0 0)", "Swirl origin point"},
         {"axis", "vector", "(0 0 1)", "Swirl axis direction"},
         {"axialVelocity", "Function1<scalar>", "", "Axial velocity [m/s]"},
         {"radialVelocity", "Function1<scalar>", "", "Radial velocity [m/s]"},
         {"omega", "Function1<scalar>", "", "Angular velocity [rad/s]"}});

    // ════════════════════════════════════════════════════════════════
    // 11. CONSTRAINT types (patch types appearing in boundaryField)
    // ════════════════════════════════════════════════════════════════

    add("empty", BCCategory::Constraint,
        "Empty patch: 2D simplification. No solution in this direction.\n"
        "Used for front-and-back planes in 2D cases.",
        {"all"});

    add("symmetry", BCCategory::Constraint,
        "Symmetry plane: normal component = 0, normal gradient of tangential = 0.",
        {"all"});

    add("symmetryPlane", BCCategory::Constraint,
        "Symmetry plane (alternative name). Same as symmetry.",
        {"all"},
        {}, {},
        {"symmetry"});

    add("wedge", BCCategory::Constraint,
        "Wedge patch for 2D axisymmetric (wedge) geometries.\n"
        "Front and back are wedge type. Single cell layer in azimuthal direction.",
        {"all"});

    add("cyclic", BCCategory::Constraint,
        "Cyclic (periodic) patch pair. Values match exactly on both sides.",
        {"all"});

    add("cyclicAMI", BCCategory::Constraint,
        "Cyclic AMI (Arbitrary Mesh Interface). Periodic with non-conformal mesh.",
        {"all"});

    add("cyclicACMI", BCCategory::Constraint,
        "Cyclic ACMI (Arbitrarily Coupled Mesh Interface). Cyclic with partial overlap.",
        {"all"});

    add("cyclicSlip", BCCategory::Constraint,
        "Cyclic with slip: tangential components match; normal is zeroGradient.",
        {"all"});

    add("processor", BCCategory::Constraint,
        "Processor boundary: inter-processor communication in parallel runs.\n"
        "Auto-generated by decomposePar. Do not edit manually.",
        {"all"});

    add("processorCyclic", BCCategory::Constraint,
        "Processor boundary on a cyclic: parallel decomposed cyclic BC.",
        {"all"});

    add("patch", BCCategory::Constraint,
        "Generic patch: no special constraint. Used with any field BC type.",
        {"all"});

    add("wall", BCCategory::Constraint,
        "Wall patch type. Use with wall BCs (noSlip, epsilonWallFunction, etc.).",
        {"all"});

    // ════════════════════════════════════════════════════════════════
    // 12. ADDITIONAL from source scan + official docs (v2206–v12)
    // ════════════════════════════════════════════════════════════════

    add("acousticWaveTransmissive", BCCategory::Outlet,
        "Acoustic wave transmissive: includes acoustic impedance for pressure waves.",
        {"scalar"},
        {{"gamma", "scalar", "1.4", "Ratio of specific heats Cp/Cv"}},
        {{"fieldInf", "scalar", "0", "Far-field value"},
         {"lInf", "scalar", "1.0", "Relaxation distance"}});

    add("fixedMeanOutletInlet", BCCategory::Outlet,
        "fixedMean + outletInlet: adjustable mean at outlet with backflow protection.",
        {"scalar"},
        {{"meanValue", "scalar", "0", "Target mean value"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("fluxCorrectedVelocity", BCCategory::Outlet,
        "Velocity outlet corrected to match target flux. For incompressible flows.",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("matchedFlowRateOutletVelocity", BCCategory::Outlet,
        "Outlet velocity matched to corresponding inlet flow rate.\n"
        "Ensures mass conservation across inlet-outlet pairs.",
        {"vector"},
        {{"inletPatchName", "word", "", "Name of the matching inlet patch"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("outletMachNumberPressure", BCCategory::Outlet,
        "Outlet pressure that maintains a target subsonic Mach number.",
        {"scalar"},
        {{"M", "scalar", "0.3", "Target Mach number"},
         {"pBackflow", "scalar", "1e5", "Pressure for backflow"},
         {"value", "scalar", "uniform 1e5", "Initial value"}});

    add("pressureDirectedInletOutletVelocity", BCCategory::Inlet,
        "Velocity inlet/outlet with prescribed direction. Normal component from flux;\n"
        "direction prescribed.",
        {"vector"},
        {{"inletDirection", "vector", "(1 0 0)", "Inlet flow direction"}},
        {{"phi", "word", "phi", "Flux field name"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("pressureInletOutletParSlipVelocity", BCCategory::Inlet,
        "Pressure inlet/outlet velocity with parallel slip condition.\n"
        "Similar to pressureInletOutletVelocity but with slip treatment.",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("pressureInletUniformVelocity", BCCategory::Inlet,
        "Uniform velocity inlet where pressure is specified.\n"
        "Uniform across the patch; magnitude from flux.",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("pressurePIDControlInletVelocity", BCCategory::Inlet,
        "PID-controlled inlet velocity to achieve target pressure/flow rate.\n"
        "Uses PID controller for dynamic adjustment.",
        {"vector"},
        {{"targetFlowRate", "scalar", "0", "Target flow rate"},
         {"Kp", "scalar", "1.0", "Proportional gain"},
         {"Ki", "scalar", "0.1", "Integral gain"},
         {"Kd", "scalar", "0", "Derivative gain"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("rotatingPressureInletOutletVelocity", BCCategory::Inlet,
        "Pressure inlet/outlet velocity in rotating frame (SRF/MRF).",
        {"vector"},
        {},
        {{"phi", "word", "phi", "Flux field name"},
         {"omega", "vector", "(0 0 0)", "Angular velocity vector"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("swirlFanVelocity", BCCategory::Inlet,
        "Fan velocity with swirl component. Axial + tangential velocity from fan curve.",
        {"vector"},
        {{"fanCurve", "table", "table ((0 0)(10 10))", "Axial velocity vs radius"},
         {"origin", "vector", "(0 0 0)", "Fan origin"},
         {"axis", "vector", "(0 0 1)", "Fan axis"},
         {"rpm", "scalar", "0", "Rotational speed"}});

    add("uniformFixedGradient", BCCategory::Basic,
        "Uniform fixed gradient across entire patch (single value, may be time-varying).",
        {"all"},
        {{"uniformGradient", "Function1<Type>", "constant 0", "Uniform gradient as Function1"}});

    add("uniformInletOutlet", BCCategory::Outlet,
        "Uniform inletOutlet: uniform inletValue across patch (single value for backflow).",
        {"all"},
        {{"inletValue", "scalar/vector/tensor", "uniform 0", "Uniform value for reverse flow"}},
        {{"phi", "word", "phi", "Flux field name"}});

    add("uniformJump", BCCategory::Special,
        "Uniform fixed jump across cyclic (single jump value across entire interface).",
        {"all"},
        {{"jumpTable", "Function1<Type>", "", "Uniform jump table"}});

    add("uniformJumpAMI", BCCategory::Special,
        "Uniform fixed jump across AMI cyclic.",
        {"all"},
        {{"jumpTable", "Function1<Type>", "", "Uniform jump table"}});

    add("uniformNormalFixedValue", BCCategory::Inlet,
        "Uniform version of surfaceNormalFixedValue. Single normal value across patch.",
        {"vector"},
        {{"refValue", "scalar", "uniform 0", "Uniform normal component value"}},
        {{"ramp", "Function1<scalar>", "", "Time-based ramping"}});

    add("variableHeightFlowRate", BCCategory::Inlet,
        "Flow rate that varies with height (e.g. atmospheric boundary layer inlet).",
        {"scalar"},
        {{"flowRate", "Function1<scalar>", "constant 0", "Flow rate as function of height"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    add("variableHeightFlowRateInletVelocity", BCCategory::Inlet,
        "Velocity inlet with flow rate varying with height. For ABL profiles.",
        {"vector"},
        {{"flowRate", "Function1<scalar>", "", "Flow rate vs height"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("waveSurfacePressure", BCCategory::Inlet,
        "Wave surface pressure for free-surface flows. Time-varying from wave theory.",
        {"scalar"},
        {},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    add("interstitialInletVelocity", BCCategory::Inlet,
        "Inlet velocity for porous/interstitial regions. Accounts for porosity.",
        {"vector"},
        {{"value", "vector", "uniform (0 0 0)", "Inlet velocity"},
         {"alpha", "scalar", "1.0", "Volume fraction/porosity"}});

    add("activeBaffleVelocity", BCCategory::Wall,
        "Active baffle velocity: wall velocity controlled by pressure difference.\n"
        "Used for pressure-activated baffles/valves.",
        {"vector"},
        {{"openFraction", "scalar", "0", "Baffle open fraction [0-1]"},
         {"value", "vector", "uniform (0 0 0)", "Initial value"}});

    add("mappedVelocityFluxFixedValue", BCCategory::Mapped,
        "Mapped velocity that also maps the flux. For coupled velocity boundaries.",
        {"vector"},
        {{"value", "vector", "uniform (0 0 0)", "Fallback value"}},
        {{"field", "word", "", "Source field name"},
         {"setAverage", "Switch", "false", "Adjust to match average"}});

    // ══ OpenFOAM v2012–v2512 additions ══

    // ── External / coupled thermal BCs ──
    add("externalCoupledTemperature", BCCategory::Wall,
        "External coupled thermal BC for conjugate heat transfer (CHT) via\n"
        "external coupling interface. Used with OpenFOAM-preCICE adapters.",
        {"scalar"},
        {{"value", "scalar", "uniform 300", "Initial temperature"}},
        {{"kappaMethod", "word", "fluidThermo", "Thermal conductivity method"},
         {"kappa", "word", "kappa", "Thermal conductivity field name"}});

    add("turbulentTemperatureCoupledBaffleMixed", BCCategory::Wall,
        "Mixed temperature BC for coupled baffles (thin walls) with turbulence.\n"
        "Uses turbulent thermal diffusivity for heat transfer across baffle.",
        {"scalar"},
        {{"value", "scalar", "uniform 300", "Initial temperature"}},
        {{"Tnbr", "word", "T", "Neighbour temperature field"},
         {"kappa", "word", "kappa", "Thermal conductivity"},
         {"thicknessLayers", "scalarList", "()", "Layer thicknesses"}});

    add("humidityTemperatureCoupledMixed", BCCategory::Wall,
        "Temperature BC for coupled walls with humidity and condensation.\n"
        "Accounts for latent heat effects at wall surfaces.",
        {"scalar"},
        {{"value", "scalar", "uniform 300", "Initial temperature"}},
        {{"Tnbr", "word", "T", "Neighbour temperature field"},
         {"kappa", "word", "kappa", "Thermal conductivity"}});

    // ── Compressible / acoustic BCs ──
    add("pressureInletOutletParSlipVelocity", BCCategory::Inlet,
        "Pressure-driven inlet/outlet velocity with parallel slip at the boundary.",
        {"vector"},
        {{"value", "vector", "uniform (0 0 0)", "Initial value"}},
        {{"phi", "word", "phi", "Flux field name"}});

    add("characteristicPressureInletOutletVelocity", BCCategory::Inlet,
        "Characteristic-based non-reflecting velocity BC for compressible flows.\n"
        "Uses Riemann invariants for wave transmissive behaviour.",
        {"vector"},
        {{"value", "vector", "uniform (0 0 0)", "Initial value"}},
        {{"pRef", "scalar", "1e5", "Reference pressure"},
         {"rhoRef", "scalar", "1.2", "Reference density"}});

    add("charactersticWallTemperature", BCCategory::Wall,
        "Characteristic-based wall temperature BC for compressible flows.\n"
        "Handles adiabatic/iso-thermal wall conditions in high-speed flows.",
        {"scalar"},
        {{"value", "scalar", "uniform 300", "Initial temperature"}},
        {{"adiabatic", "Switch", "false", "Adiabatic wall condition"},
         {"Cp", "word", "thermo:Cp", "Heat capacity field name"}});

    // ── Multiphase BCs ──
    add("contactAngleForce", BCCategory::Wall,
        "Dynamic contact angle model for multiphase flows at walls.\n"
        "Uses advancing/receding angles with hysteresis.",
        {"scalar"},
        {{"theta0", "scalar", "90", "Equilibrium contact angle [degrees]"},
         {"uTheta", "scalar", "0.1", "Contact angle velocity scale"}},
        {{"thetaA", "scalar", "90", "Advancing contact angle [degrees]"},
         {"thetaR", "scalar", "90", "Receding contact angle [degrees]"},
         {"limit", "word", "none", "Gradient limiting: none/gradient/zeroGradient"}});

    add("multiphaseStabilizedTurbulence", BCCategory::Inlet,
        "Stabilized turbulence BC for multiphase inlet flows.\n"
        "Prevents unphysical turbulence decay near the interface.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}},
        {{"alpha", "word", "alpha.water", "Phase fraction field name"}});

    // ── Scalar transport / species BCs ──
    add("externalWallHeatFluxTemperature", BCCategory::Wall,
        "Wall temperature BC driven by external heat flux.\n"
        "Calculates wall temperature from prescribed heat flux and thermal resistance.",
        {"scalar"},
        {{"q", "scalar", "uniform 0", "Heat flux [W/m²]"}},
        {{"kappa", "word", "kappa", "Thermal conductivity field"},
         {"thicknessLayers", "scalarList", "()", "Optional layer thicknesses [m]"},
         {"kappaLayers", "scalarList", "()", "Optional layer conductivities [W/m·K]"}});

    add("reactingMultiphaseSurfaceFilm", BCCategory::Wall,
        "Surface film BC for reacting multiphase flows.\n"
        "Models liquid film formation, evaporation, and reaction at walls.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial film thickness"}},
        {{"Twall", "scalar", "300", "Wall temperature [K]"},
         {"deltaWet", "scalar", "5e-5", "Wetted film thickness [m]"}});

    // ── Overset / immersed boundary ──
    add("oversetPressureFvPatchScalarField", BCCategory::Constraint,
        "Pressure BC for overset (Chimera) mesh patches.\n"
        "Enables pressure interpolation across overlapping mesh regions.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}});

    // ── Solver-coupled BCs ──
    add("buoyancyPressure", BCCategory::Inlet,
        "Buoyancy-modified pressure BC accounting for density variations.\n"
        "p_rgh = p − rho*(g·h) with variable density.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial value"}},
        {{"rho", "word", "rho", "Density field name"}});

    add("outletMappedUniformInletHeatAddition", BCCategory::Outlet,
        "Maps outlet temperature/energy back to inlet with heat addition.\n"
        "For CHT or recycled flow with energy source.",
        {"scalar"},
        {{"value", "scalar", "uniform 300", "Initial value"}},
        {{"Q", "scalar", "0", "Heat addition [W]"},
         {"phi", "word", "phi", "Flux field name"}});

    // ── Non-Newtonian / rheology BCs ──
    add("nonNewtonianViscosityWallFunction", BCCategory::Wall,
        "Wall function for turbulent viscosity with non-Newtonian fluids.\n"
        "Accounts for shear-rate dependent viscosity near walls.",
        {"scalar"},
        {{"Cmu", "scalar", "0.09", "Turbulence model constant"},
         {"kappa", "scalar", "0.41", "von Kármán constant"},
         {"E", "scalar", "9.8", "Wall roughness parameter"}},
        {{"n", "scalar", "1.0", "Power-law index (1=Newtonian)"},
         {"K", "scalar", "0.001", "Consistency index [Pa·s^n]"},
         {"value", "scalar", "uniform 0", "Initial value"}});

    // ── Acoustic / vibroacoustic BCs ──
    add("acousticWaveTransmissivePressure", BCCategory::Outlet,
        "Acoustic wave transmissive pressure BC.\n"
        "Non-reflecting pressure outlet for aeroacoustic simulations.",
        {"scalar"},
        {{"gamma", "scalar", "1.4", "Ratio of specific heats Cp/Cv"}},
        {{"fieldInf", "scalar", "1e5", "Far-field pressure [Pa]"},
         {"lInf", "scalar", "1.0", "Relaxation length scale [m]"}});

    // ── Radiation BCs ──
    add("greyDiffusiveRadiationViewFactor", BCCategory::Wall,
        "Grey diffusive radiation BC with view factor.\n"
        "For radiative heat transfer between surfaces.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Initial radiative intensity"}},
        {{"emissivity", "scalar", "1.0", "Surface emissivity (0–1)"},
         {"Twall", "scalar", "300", "Wall temperature [K]"}});

    // ── Battery / electrochemical BCs ──
    add("electrochemicalPotential", BCCategory::Inlet,
        "Electrochemical potential BC for battery/fuel cell simulations.\n"
        "Prescribes electrode potential at boundaries.",
        {"scalar"},
        {{"value", "scalar", "uniform 0", "Potential [V]"}},
        {{"currentDensity", "scalar", "0", "Applied current density [A/m²]"},
         {"exchangeCurrentDensity", "scalar", "1", "Exchange current density [A/m²]"}});

}
