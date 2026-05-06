#include "turbulencemodeldatabase.h"

TurbulenceModelDatabase* TurbulenceModelDatabase::instance()
{
    static TurbulenceModelDatabase db;
    return &db;
}

TurbulenceModelDatabase::TurbulenceModelDatabase() { initDatabase(); }

QString TurbulenceModelDatabase::categoryName(TurbModelCategory cat)
{
    switch (cat) {
    case TurbModelCategory::RAS:            return "RAS (RANS)";
    case TurbModelCategory::LES:            return "LES";
    case TurbModelCategory::DES:            return "DES (Hybrid)";
    case TurbModelCategory::ReynoldsStress: return "Reynolds Stress (RSTM)";
    case TurbModelCategory::Laminar:        return "Laminar / Non-Newtonian";
    }
    return "Unknown";
}

TurbModelInfo TurbulenceModelDatabase::modelInfo(const QString &name) const
{
    for (const auto &m : m_models)
        if (m.name == name) return m;
    return {};
}

QVector<TurbModelInfo> TurbulenceModelDatabase::modelsByCategory(TurbModelCategory cat) const
{
    QVector<TurbModelInfo> result;
    for (const auto &m : m_models)
        if (m.category == cat) result.append(m);
    return result;
}

QString TurbulenceModelDatabase::generateConfig(const TurbModelInfo &info)
{
    QString s;
    s += "simulationType  " + categoryName(info.category).split(' ').first() + ";\n\n";
    s += categoryName(info.category).split(' ').first() + "\n{\n";
    QString modelKeyword = (info.category == TurbModelCategory::ReynoldsStress) ? "RASModel" : "model";
    s += "    " + modelKeyword + "        " + info.name + ";\n";
    s += "    turbulence      on;\n";
    s += "    printCoeffs     on;\n";
    if (!info.params.isEmpty()) {
        s += "\n    " + info.name + "Coeffs\n    {\n";
        for (const auto &p : info.params) {
            s += QString("        %1      %2;\n").arg(p.name, -16).arg(p.defaultValue);
        }
        s += "    }\n";
    }
    s += "}\n";
    return s;
}

void TurbulenceModelDatabase::initDatabase()
{
    // ════════════════════════════════════════════════════════════════
    // RAS — Reynolds-Averaged Navier-Stokes models
    // ════════════════════════════════════════════════════════════════

    m_models.append({
        "kEpsilon", "RASModels::kEpsilon", TurbModelCategory::RAS,
        "Standard k-epsilon model. Two-equation linear eddy-viscosity model. "
        "Most widely used RANS model for industrial flows. Robust and well-validated.",
        "nu_t = C_mu * k^2 / epsilon\n"
        "Dk/Dt = P - epsilon + div[(nu + nu_t/sigma_k) grad k]\n"
        "Depsilon/Dt = C1*epsilon/k*P - C2*epsilon^2/k + div[(nu + nu_t/sigma_e) grad epsilon]",
        "Launder & Spalding (1972, 1974)",
        {{"Cmu", "scalar", "0.09", "Turbulent viscosity constant"},
         {"C1", "scalar", "1.44", "epsilon production constant"},
         {"C2", "scalar", "1.92", "epsilon dissipation constant"},
         {"C3", "scalar", "-0.33", "RDT compression term coefficient"},
         {"sigmak", "scalar", "1.0", "Turbulent Prandtl number for k"},
         {"sigmaEps", "scalar", "1.3", "Turbulent Prandtl number for epsilon"}}
    });

    m_models.append({
        "realizableKE", "RASModels::realizableKE", TurbModelCategory::RAS,
        "Realizable k-epsilon model. Enforces realizability constraints (positive normal "
        "stresses, Schwarz inequality). Better for flows with strong streamline curvature, "
        "vortices, and rotation. Improved C_mu formulation.",
        "nu_t = C_mu * k^2 / epsilon  (C_mu is NOT constant)\n"
        "C_mu = 1 / (A0 + As * U* * k / epsilon)\n"
        "U* = sqrt(S_ij*S_ij + Omega_ij*Omega_ij)",
        "Shih et al. (1994, 1995)",
        {{"A0", "scalar", "4.0", "Model constant A0"},
         {"C2", "scalar", "1.9", "epsilon dissipation constant"},
         {"sigmak", "scalar", "1.0", "Turbulent Prandtl number for k"},
         {"sigmaEps", "scalar", "1.2", "Turbulent Prandtl number for epsilon"}}
    });

    m_models.append({
        "RNGkEpsilon", "RASModels::RNGkEpsilon", TurbModelCategory::RAS,
        "RNG k-epsilon model. Derived using Renormalization Group theory. "
        "Includes analytically derived constants and an additional strain-dependent "
        "term in the epsilon equation. Better for low-Re and transitional flows.",
        "nu_t = C_mu * k^2 / epsilon\n"
        "Additional term R in epsilon equation:\n"
        "R = C_mu * eta^3 * (1 - eta/eta0) / (1 + beta*eta^3) * epsilon^2/k\n"
        "eta = S * k / epsilon",
        "Yakhot et al. (1992)",
        {{"Cmu", "scalar", "0.0845", "Turbulent viscosity constant"},
         {"C1", "scalar", "1.42", "epsilon production constant"},
         {"C2", "scalar", "1.68", "epsilon dissipation constant"},
         {"C3", "scalar", "-0.33", "RDT compression term coefficient"},
         {"sigmak", "scalar", "0.71942", "Turbulent Prandtl number for k"},
         {"sigmaEps", "scalar", "0.71942", "Turbulent Prandtl number for epsilon"},
         {"eta0", "scalar", "4.38", "RNG parameter eta0"},
         {"beta", "scalar", "0.012", "RNG parameter beta"}}
    });

    m_models.append({
        "kOmega", "RASModels::kOmega", TurbModelCategory::RAS,
        "Standard Wilcox k-omega model. Better near-wall treatment than k-epsilon "
        "without wall functions. Sensitive to freestream omega values.",
        "nu_t = k / omega\n"
        "Dk/Dt = P - beta* * k * omega + div[(nu + sigma_k * nu_t) grad k]\n"
        "Domega/Dt = alpha * omega/k * P - beta * omega^2 + div[(nu + sigma_omega * nu_t) grad omega]",
        "Wilcox (1998)",
        {{"Cmu", "scalar", "0.09", "beta* = turbulent viscosity constant"},
         {"alpha", "scalar", "0.52", "Production coefficient"},
         {"beta", "scalar", "0.072", "Destruction coefficient"},
         {"alphak", "scalar", "0.5", "Diffusion coefficient for k"},
         {"alphaOmega", "scalar", "0.5", "Diffusion coefficient for omega"}}
    });

    m_models.append({
        "kOmegaSST", "RASModels::kOmegaSST", TurbModelCategory::RAS,
        "Menter k-omega SST (Shear Stress Transport). Blends k-omega (near wall) "
        "with k-epsilon (far field). Includes a limiter for eddy viscosity to account "
        "for adverse pressure gradients. Industry standard for external aerodynamics.",
        "nu_t = a1 * k / max(a1 * omega, F2 * S)\n"
        "Blending function F1: k-omega near wall -> k-epsilon in freestream\n"
        "Blending function F2: limits nu_t in adverse pressure gradients",
        "Menter (1994); Menter, Kuntz & Langtry (2003)",
        {{"alphaK1", "scalar", "0.85", "k diffusion coefficient (inner)"},
         {"alphaK2", "scalar", "1.0", "k diffusion coefficient (outer)"},
         {"alphaOmega1", "scalar", "0.5", "omega diffusion (inner)"},
         {"alphaOmega2", "scalar", "0.856", "omega diffusion (outer)"},
         {"beta1", "scalar", "0.075", "Destruction coefficient (inner)"},
         {"beta2", "scalar", "0.0828", "Destruction coefficient (outer)"},
         {"betaStar", "scalar", "0.09", "C_mu equivalent"},
         {"gamma1", "scalar", "0.55556", "Production coefficient (inner) = 5/9"},
         {"gamma2", "scalar", "0.44", "Production coefficient (outer)"},
         {"a1", "scalar", "0.31", "Shear stress transport constant"},
         {"b1", "scalar", "1.0", "Roughness constant"},
         {"c1", "scalar", "10.0", "Roughness constant"}}
    });

    m_models.append({
        "SpalartAllmaras", "RASModels::SpalartAllmaras", TurbModelCategory::RAS,
        "Spalart-Allmaras one-equation model. Solves a transport equation for the "
        "modified turbulent viscosity nuTilda. Designed for aerospace/external "
        "aerodynamics. Computationally efficient (1 equation vs 2).",
        "D(nuTilda)/Dt = Cb1*(1-ft2)*S_tilda*nuTilda\n"
        "  - (Cw1*fw - Cb1/kappa^2*ft2)*(nuTilda/d)^2\n"
        "  + 1/sigma * div[(nu + nuTilda) grad nuTilda]\n"
        "  + Cb2/sigma * |grad nuTilda|^2\n"
        "nu_t = nuTilda * fv1,  chi = nuTilda/nu\n"
        "fv1 = chi^3/(chi^3 + Cv1^3)",
        "Spalart & Allmaras (1994)",
        {{"sigmaNut", "scalar", "0.66666", "Diffusion coefficient = 2/3"},
         {"kappa", "scalar", "0.41", "von Karman constant"},
         {"Cb1", "scalar", "0.1355", "Production coefficient"},
         {"Cb2", "scalar", "0.622", "Non-conservative diffusion coefficient"},
         {"Cw2", "scalar", "0.3", "Wall destruction coefficient"},
         {"Cw3", "scalar", "2.0", "Wall destruction exponent"},
         {"Cv1", "scalar", "7.1", "Viscous damping constant"},
         {"Cs", "scalar", "0.3", "ft2 trip constant"}}
    });

    m_models.append({
        "LaunderSharmaKE", "RASModels::LaunderSharmaKE", TurbModelCategory::RAS,
        "Launder-Sharma low-Reynolds-number k-epsilon model. Resolves the viscous "
        "sublayer using damping functions f_mu and f2. Suitable for low-Re flows "
        "where wall functions are inadequate.",
        "nu_t = C_mu * f_mu * k^2 / epsilon_tilde\n"
        "epsilon = epsilon_tilde + D  (D = 2*nu*(grad sqrt(k))^2)\n"
        "f_mu = exp[-3.4/(1+Re_t/50)^2]\n"
        "f2 = 1 - 0.3*exp(-Re_t^2),  Re_t = k^2/(nu*epsilon_tilde)",
        "Launder & Sharma (1974)",
        {{"Cmu", "scalar", "0.09", "Turbulent viscosity constant"},
         {"C1", "scalar", "1.44", "Production constant"},
         {"C2", "scalar", "1.92", "Dissipation constant"},
         {"C3", "scalar", "-0.33", "RDT compression coefficient"},
         {"alphah", "scalar", "1.0", "Enthalpy Prandtl number (compressible)"},
         {"alphahk", "scalar", "1.0", "k Prandtl number (compressible)"},
         {"alphaEps", "scalar", "0.76923", "epsilon diffusion coefficient"}}
    });

    m_models.append({
        "kOmegaSSTSAS", "RASModels::kOmegaSSTSAS", TurbModelCategory::RAS,
        "k-omega SST SAS (Scale-Adaptive Simulation). URANS model that dynamically "
        "adjusts to resolve turbulent structures in unsteady regions while maintaining "
        "RANS behavior in stable regions. Adds Q_SAS source term to omega equation.",
        "Based on kOmegaSST with additional source term:\n"
        "Q_SAS = max[rho*zeta2*kappa*S^2*(L/L_vK)^2 - C*2*rho*k/sigmaPhi*max(...), 0]\n"
        "L = sqrt(k)/(C_mu^(1/4)*omega),  L_vK = von Karman length scale",
        "Menter & Egorov (2010)",
        {{"Cs", "scalar", "0.11", "SAS coefficient"},
         {"kappa", "scalar", "0.41", "von Karman constant"},
         {"zeta2", "scalar", "3.51", "SAS coefficient zeta2"},
         {"sigmaPhi", "scalar", "0.66667", "SAS coefficient = 2/3"},
         {"C", "scalar", "2.0", "SAS coefficient C"}}
    });

    m_models.append({
        "kOmegaSSTLM", "RASModels::kOmegaSSTLM", TurbModelCategory::RAS,
        "Langtry-Menter 4-equation transitional SST model. Adds transport equations "
        "for intermittency (gamma) and transition momentum thickness Reynolds number "
        "(ReTheta_t). Predicts laminar-to-turbulent transition.",
        "Based on kOmegaSST with coupled transition equations:\n"
        "D(gamma)/Dt = P_gamma - E_gamma + div[(nu + nu_t/sigma_gamma) grad gamma]\n"
        "D(ReTheta_t)/Dt = P_theta_t + div[sigma_theta_t*(nu+nu_t) grad ReTheta_t]\n"
        "Effective production: P_k_eff = gamma_eff * P_k",
        "Langtry & Menter (2009)",
        {{"ca1", "scalar", "2.0", "Transition constant ca1"},
         {"ca2", "scalar", "0.06", "Transition constant ca2"},
         {"ce1", "scalar", "1.0", "Transition constant ce1"},
         {"ce2", "scalar", "50.0", "Transition constant ce2"},
         {"cThetat", "scalar", "0.03", "Transition constant cTheta_t"},
         {"sigmaThetat", "scalar", "2.0", "Diffusion for ReTheta_t"}}
    });

    // ════════════════════════════════════════════════════════════════
    // LES — Large Eddy Simulation models
    // ════════════════════════════════════════════════════════════════

    m_models.append({
        "Smagorinsky", "LESModels::Smagorinsky", TurbModelCategory::LES,
        "Classic Smagorinsky SGS model. Eddy viscosity from local equilibrium "
        "assumption. Simple with one model constant Cs. Overly dissipative "
        "near walls without damping.",
        "tau_ij^d = -2 * nu_sgs * S_ij^d\n"
        "nu_sgs = Ck * sqrt(k_sgs) * Delta\n"
        "k_sgs solved from equilibrium: D_ij:B_ij + Ce*k_sgs^(3/2)/Delta = 0",
        "Smagorinsky (1963)",
        {{"Ck", "scalar", "0.094", "k-equation coefficient Ck (= Cs^2 ~ 0.02^2 equivalent)"},
         {"Ce", "scalar", "1.048", "Dissipation coefficient Ce"}}
    });

    m_models.append({
        "WALE", "LESModels::WALE", TurbModelCategory::LES,
        "WALE (Wall-Adapting Local Eddy-viscosity). Cubic wall-asymptotic behavior "
        "(nu_sgs ~ y^3) without dynamic procedure or damping functions. "
        "Handles laminar-turbulent transition better than Smagorinsky.",
        "nu_sgs = (Cw * Delta)^2 * (S_ij^d * S_ij^d)^(3/2) /\n"
        "        [(S_ij * S_ij)^(5/2) + (S_ij^d * S_ij^d)^(5/4)]\n"
        "S_ij^d = 1/2*(gradU^2 + gradU^(2T)) - 1/3*trace(gradU^2)*I",
        "Nicoud & Ducros (1999)",
        {{"Ck", "scalar", "0.094", "SGS kinetic energy coefficient"},
         {"Ce", "scalar", "1.048", "Dissipation coefficient"},
         {"Cw", "scalar", "0.325", "WALE model coefficient"}}
    });

    m_models.append({
        "kEqn", "LESModels::kEqn", TurbModelCategory::LES,
        "One-equation eddy-viscosity LES model. Solves transport equation for "
        "sub-grid kinetic energy k_sgs. Accounts for history and non-local "
        "effects better than algebraic models.",
        "nu_sgs = Ck * Delta * sqrt(k_sgs)\n"
        "D(k_sgs)/Dt = -tau_ij*S_ij - Ce*k_sgs^(3/2)/Delta\n"
        "              + div[(nu + nu_sgs) grad k_sgs]",
        "Yoshizawa (1986)",
        {{"Ck", "scalar", "0.094", "Energy production coefficient"},
         {"Ce", "scalar", "1.048", "Dissipation coefficient"}}
    });

    m_models.append({
        "dynamicKEqn", "LESModels::dynamicKEqn", TurbModelCategory::LES,
        "Dynamic one-equation model. Model coefficients Ck and Ce determined "
        "dynamically from the resolved scales using a test filter. No user-specified "
        "coefficients needed. Adapts to local flow conditions.",
        "nu_sgs = Ck_dyn * Delta * sqrt(k_sgs)\n"
        "Ck and Ce computed dynamically via Germano identity:\n"
        "L_ij = T_ij - tau_ij_tilde = U_i*U_j_tilde - U_i_tilde*U_j_tilde",
        "Kim & Menon (1995)",
        {}  // dynamic — no user coefficients
    });

    m_models.append({
        "dynamicLagrangian", "LESModels::dynamicLagrangian", TurbModelCategory::LES,
        "Dynamic Lagrangian LES model. Uses Lagrangian averaging of the dynamic "
        "coefficient along pathlines for improved stability and reduced spatial "
        "variation compared to standard dynamic procedure.",
        "Coefficients averaged along fluid particle trajectories:\n"
        "C(x,t) = <J_LM>/<J_MM>  (Lagrangian average)",
        "Meneveau et al. (1996)",
        {}  // dynamic
    });

    // ════════════════════════════════════════════════════════════════
    // DES — Detached Eddy Simulation (Hybrid RANS-LES)
    // ════════════════════════════════════════════════════════════════

    m_models.append({
        "SpalartAllmarasDES", "DESModels::SpalartAllmarasDES", TurbModelCategory::DES,
        "Spalart-Allmaras DES. Switches from S-A RANS in attached boundary layers "
        "to Smagorinsky-like LES in separated regions based on a length scale criterion.",
        "d_tilde = min(d, C_DES * Delta)\n"
        "RANS mode (near wall): d_tilde = d (wall distance)\n"
        "LES mode (separated): d_tilde = C_DES * Delta",
        "Spalart et al. (1997)",
        {{"CDES", "scalar", "0.65", "DES length scale constant"}}
    });

    m_models.append({
        "SpalartAllmarasDDES", "DESModels::SpalartAllmarasDDES", TurbModelCategory::DES,
        "Delayed DES (DDES). Prevents premature switching to LES in thick boundary "
        "layers. Uses a shielding function to protect the RANS layer.",
        "d_tilde = d - f_d * max(0, d - C_DES * Delta)\n"
        "f_d = 1 - tanh[(8*r_d)^3]  (shielding function)\n"
        "r_d = (nu_t + nu)/(kappa^2*d^2*sqrt(U_i_j*U_i_j))",
        "Spalart et al. (2006)",
        {{"CDES", "scalar", "0.65", "DDES length scale constant"}}
    });

    m_models.append({
        "SpalartAllmarasIDDES", "DESModels::SpalartAllmarasIDDES", TurbModelCategory::DES,
        "Improved DDES (IDDES). Combines DDES with Wall-Modeled LES (WMLES) "
        "capabilities. Adapts between DDES and WMLES depending on grid and flow.",
        "DDES mode: same as SpalartAllmarasDDES\n"
        "WMLES mode: activates when turbulent content is present in the boundary layer",
        "Shur et al. (2008)",
        {{"CDES", "scalar", "0.65", "IDDES length scale constant"}}
    });

    // ════════════════════════════════════════════════════════════════
    // Reynolds Stress (RSTM)
    // ════════════════════════════════════════════════════════════════

    m_models.append({
        "LRR", "RASModels::LRR", TurbModelCategory::ReynoldsStress,
        "Launder-Reece-Rodi Reynolds Stress model. Solves transport equations for "
        "all 6 Reynolds stress components. Captures anisotropy, curvature, and "
        "secondary flows better than eddy-viscosity models.",
        "D(u_i*u_j)/Dt = P_ij + Phi_ij - epsilon_ij + D_ij\n"
        "Phi_ij (pressure-strain) — LRR-IP model:\n"
        "Phi_ij = -C1*epsilon/k*(u_i*u_j - 2/3*k*delta_ij)\n"
        "        - C2*(P_ij - 2/3*P*delta_ij)",
        "Launder, Reece & Rodi (1975)",
        {{"C1", "scalar", "1.8", "Slow pressure-strain coefficient"},
         {"C2", "scalar", "0.6", "Rapid pressure-strain coefficient"},
         {"C1Ref", "scalar", "0.5", "Wall-reflection coefficient"},
         {"C2Ref", "scalar", "0.3", "Wall-reflection coefficient"},
         {"Cmu", "scalar", "0.09", "Used for turbulent diffusion"},
         {"Cs", "scalar", "0.22", "Diffusion coefficient"}}
    });

    m_models.append({
        "SSG", "RASModels::SSG", TurbModelCategory::ReynoldsStress,
        "Speziale-Sarkar-Gatski RSTM. Uses quadratic pressure-strain correlation. "
        "More accurate than LRR for swirling flows and flows with strong "
        "streamline curvature.",
        "Phi_ij (SSG pressure-strain):\n"
        "Phi_ij = -(C1*epsilon + C1**P)*b_ij + C2*epsilon*(b_ik*b_jk - 1/3*b_mn*b_mn)\n"
        "        + (C3 - C3**sqrt(b_kl*b_kl))*k*S_ij + C4*k*(b_ik*S_jk + b_jk*S_ik\n"
        "        - 2/3*b_mn*S_mn*delta_ij) + C5*k*(b_ik*W_jk + b_jk*W_ik)",
        "Speziale, Sarkar & Gatski (1991)",
        {{"C1", "scalar", "3.4", "Linear slow coefficient"},
         {"C1s", "scalar", "1.8", "C1* — nonlinear slow coefficient"},
         {"C2", "scalar", "4.2", "Nonlinear return-to-isotropy"},
         {"C3", "scalar", "0.8", "Linear rapid coefficient"},
         {"C3s", "scalar", "1.3", "C3* — nonlinear rapid coefficient"},
         {"C4", "scalar", "1.25", "Rapid coefficient C4"},
         {"C5", "scalar", "0.4", "Rapid coefficient C5"},
         {"Ceps1", "scalar", "1.44", "epsilon production"},
         {"Ceps2", "scalar", "1.92", "epsilon destruction"},
         {"Cmu", "scalar", "0.09", "Used for turbulent diffusion"}}
    });

    // ════════════════════════════════════════════════════════════════
    // Laminar / non-Newtonian
    // ════════════════════════════════════════════════════════════════

    m_models.append({
        "Stokes", "laminarModels::Stokes", TurbModelCategory::Laminar,
        "Stokes (Newtonian) laminar model. No turbulence modelling. "
        "Uses Newtonian stress-strain relationship. Appropriate for "
        "creeping flows (Re << 1) or verification cases.",
        "tau = 2 * mu * S_ij - 2/3 * mu * div(U) * I\n"
        "S_ij = 1/2 * (grad U + grad U^T)",
        "Newton (1687); Stokes (1845)",
        {}
    });

    m_models.append({
        "generalizedNewtonian", "laminarModels::generalizedNewtonian", TurbModelCategory::Laminar,
        "Generalized Newtonian model. Viscosity depends on shear rate. "
        "Supports: power-law, Bird-Carreau, Cross, Herschel-Bulkley models.",
        "nu = nu(gamma_dot)\n"
        "Power-law: nu = k * gamma_dot^(n-1)\n"
        "Bird-Carreau: nu = nu_inf + (nu_0-nu_inf)*(1+(lambda*gamma_dot)^a)^((n-1)/a)",
        "Various",
        {}
    });
}
