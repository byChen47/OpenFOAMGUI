#include "ofhighlighter.h"
#include <QRegularExpression>

OFHighlighter::OFHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    setLanguage(FileLanguage::OpenFOAM); // default
}

void OFHighlighter::setLanguage(FileLanguage lang)
{
    if (m_language == lang) return;
    m_language = lang;
    clearRules();
    switch (lang) {
    case FileLanguage::OpenFOAM:   setupOpenFOAM();  break;
    case FileLanguage::Cpp:
    case FileLanguage::CppHeader:
    case FileLanguage::C:          setupCpp();        break;
    case FileLanguage::Bash:       setupBash();       break;
    case FileLanguage::Python:     setupPython();     break;
    case FileLanguage::JSON:       setupJSON();       break;
    case FileLanguage::CMake:      setupCMake();      break;
    case FileLanguage::PlainText:
    case FileLanguage::Unknown:
    default:                       setupPlainText();  break;
    }
    rehighlight();
}

void OFHighlighter::clearRules()
{
    m_rules.clear();
    m_multiRules.clear();
}

// ════════════════════════════════════════════════════════════════════
// OpenFOAM dictionary / field file highlighting
// ════════════════════════════════════════════════════════════════════

void OFHighlighter::setupOpenFOAM()
{
    // ── Formats ──
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#0000CC"));
    keywordFmt.setFontWeight(QFont::Bold);

    QTextCharFormat classFmt;
    classFmt.setForeground(QColor("#A020F0"));
    classFmt.setFontWeight(QFont::Bold);

    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#008000"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#C04000"));

    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#808080"));
    commentFmt.setFontItalic(true);

    QTextCharFormat headerFmt;
    headerFmt.setForeground(QColor("#B8860B"));
    headerFmt.setFontWeight(QFont::Bold);

    QTextCharFormat dimFmt;
    dimFmt.setForeground(QColor("#2E8B57"));

    QTextCharFormat directiveFmt;
    directiveFmt.setForeground(QColor("#00008B"));
    directiveFmt.setFontWeight(QFont::Bold);

    QTextCharFormat bcTypeFmt;
    bcTypeFmt.setForeground(QColor("#A020F0"));
    bcTypeFmt.setFontWeight(QFont::Bold);

    // Helper
    auto add = [&](const QString &pat, const QTextCharFormat &fmt) {
        m_rules.append({QRegularExpression(pat), fmt});
    };

    // ── FoamFile header keys ──
    for (auto &k : {"version", "format", "class", "object", "location", "note"})
        add(QString("\\b%1\\b").arg(k), headerFmt);

    // ── BC types ──
    for (auto &bc : {
        "zeroGradient","fixedValue","fixedGradient","mixed",
        "inletOutlet","outletInlet","totalPressure","totalTemperature",
        "pressureInletOutletVelocity","pressureInletVelocity",
        "fixedFluxPressure","noSlip","slip","empty","symmetry",
        "wedge","cyclic","processor","wall","patch",
        "cyclicAMI","cyclicACMI","calculated","uniformFixedValue",
        "surfaceNormalFixedValue","flowRateInletVelocity","advective",
        "turbulentIntensityKineticEnergyInlet","movingWallVelocity",
        "waveTransmissive","partialSlip","kqRWallFunction",
        "epsilonWallFunction","nutkWallFunction","omegaWallFunction",
        "alphatWallFunction","fixedJump","timeVaryingMappedFixedValue",
        "externalCoupledTemperature","fanPressure","prghTotalPressure",
        "uniformDensityHydrostaticPressure","fixedProfile",
        "codedFixedValue","codedMixed"
    }) add(QString("\\b%1\\b").arg(bc), bcTypeFmt);

    // ── Field / dictionary classes ──
    for (auto &fc : {
        "volScalarField","volVectorField","volTensorField",
        "volSymmTensorField","volSphericalTensorField",
        "surfaceScalarField","surfaceVectorField",
        "pointScalarField","pointVectorField",
        "uniformDimensionedScalarField","uniformDimensionedVectorField",
        "dictionary","dimensionedScalar","dimensionedVector",
        "IOdictionary","IOobject","polyMesh","fvMesh",
        "fvSchemes","fvSolution","fvConstraints","fvModels"
    }) add(QString("\\b%1\\b").arg(fc), classFmt);

    // ── Solvers / preconditioners ──
    for (auto &sk : {
        "PCG","PBiCG","PBiCGStab","GAMG","smoothSolver",
        "DIC","DILU","diagonal","FDIC","symGaussSeidel",
        "GaussSeidel","DILUGaussSeidel","nonBlockingGaussSeidel"
    }) add(QString("\\b%1\\b").arg(sk), directiveFmt);

    // ── Discretisation schemes ──
    for (auto &sn : {
        "Euler","localEuler","CrankNicolson","backward","steadyState",
        "Gauss","linear","upwind","linearUpwind","vanLeer",
        "limitedLinear","SFCD","QUICK","TVD","NVDTVD",
        "orthogonal","corrected","uncorrected","limited",
        "cubicCorrected","linearFit","midPoint","faceCorrected",
        "pointLinear","leastSquares"
    }) add(QString("\\b%1\\b").arg(sn), directiveFmt);

    // ── Dictionary key keywords ──
    for (auto &key : {
        "internalField","boundaryField","dimensions",
        "ddtSchemes","gradSchemes","divSchemes","laplacianSchemes",
        "interpolationSchemes","snGradSchemes","fluxRequired",
        "solvers","relaxationFactors","PIMPLE","PISO","SIMPLE",
        "residualControl","cacheAgglomeration",
        "writeControl","writeInterval","writeFormat","writePrecision",
        "writeCompression","timeFormat","timePrecision",
        "runTimeModifiable","adjustTimeStep",
        "startFrom","startTime","stopAt","endTime","deltaT",
        "application","purgeWrite",
        "nOuterCorrectors","nCorrectors","nNonOrthogonalCorrectors",
        "momentumPredictor","maxCo","maxAlphaCo","maxDeltaT",
        "transportModel","nu","rho","sigma","phases",
        "turbulence","RAS","LES","laminar",
        "nAlphaCorr","nAlphaSubCycles","cAlpha",
        "MULESCorr","nLimiterIter",
        "solver","smoother","preconditioner",
        "tolerance","relTol","minIter","maxIter",
        "default","equations","fields",
        "type","value","inletValue","p0","U","p","k","epsilon",
        "omega","nut","nuTilda","alphat","p_rgh","T","gh","h"
    }) add(QString("\\b%1\\b").arg(QRegularExpression::escape(key)), keywordFmt);

    // ── Dimensions vector ──
    add(R"(\[\s*-?\d+\s+-?\d+\s+-?\d+\s+-?\d+\s+-?\d+\s+-?\d+\s+-?\d+\s*\])", dimFmt);

    // ── Numbers ──
    add(R"(\b(uniform|nonuniform)\s+)?-?\b\d+\.?\d*(?:[eE][+-]?\d+)?\b)", numberFmt);

    // ── Quoted string patterns ──
    add(R"("[^"]*")", stringFmt);

    // ── C++ comments ──
    add("//[^\n]*", commentFmt);
    m_multiRules.append({
        QRegularExpression(R"(/\*)"),
        QRegularExpression(R"(\*/)"),
        commentFmt
    });
}

// ════════════════════════════════════════════════════════════════════
// C / C++ highlighting
// ════════════════════════════════════════════════════════════════════

void OFHighlighter::setupCpp()
{
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#0000CC"));
    keywordFmt.setFontWeight(QFont::Bold);

    QTextCharFormat typeFmt;
    typeFmt.setForeground(QColor("#2B91AF"));

    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#008000"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#C04000"));

    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#808080"));
    commentFmt.setFontItalic(true);

    QTextCharFormat preprocessorFmt;
    preprocessorFmt.setForeground(QColor("#9B4F96"));

    QTextCharFormat includeFmt;
    includeFmt.setForeground(QColor("#A31515"));

    auto add = [&](const QString &pat, const QTextCharFormat &fmt) {
        m_rules.append({QRegularExpression(pat), fmt});
    };

    // ── C++ keywords ──
    for (auto &kw : {
        "if","else","for","while","do","switch","case","default",
        "break","continue","return","goto","try","catch","throw",
        "class","struct","enum","union","typedef","namespace",
        "using","template","typename","public","private","protected",
        "virtual","override","final","const","constexpr","static",
        "inline","explicit","volatile","mutable","friend","operator",
        "new","delete","sizeof","typeid","dynamic_cast","static_cast",
        "const_cast","reinterpret_cast","this","nullptr","true","false",
        "auto","decltype","noexcept","thread_local","consteval",
        "constinit","export","module","import","requires","concept"
    }) add(QString("\\b%1\\b").arg(kw), keywordFmt);

    // ── C++ types ──
    for (auto &tp : {
        "void","bool","char","short","int","long","float","double",
        "unsigned","signed","size_t","ssize_t","ptrdiff_t",
        "int8_t","int16_t","int32_t","int64_t",
        "uint8_t","uint16_t","uint32_t","uint64_t",
        "wchar_t","char16_t","char32_t","string","vector","map",
        "set","pair","shared_ptr","unique_ptr","weak_ptr",
        "Foam","word","label","scalar","vector","tensor",
        "symmTensor","sphericalTensor","tmp","autoPtr","PtrList",
        "HashTable","List","Field","fvMesh","Time","polyMesh",
        "IOobject","dictionary","volScalarField","volVectorField",
        "surfaceScalarField","dimensionedScalar"
    }) add(QString("\\b%1\\b").arg(tp), typeFmt);

    // ── Preprocessor ──
    add(R"(^\s*#\s*(include|define|undef|ifdef|ifndef|if|else|elif|endif|pragma|error|warning|line)\b)",
        preprocessorFmt);
    add(R"(<[^>]+>)", includeFmt);

    // ── Numbers ──
    add(R"(\b0[xX][0-9a-fA-F]+)", numberFmt);
    add(R"(\b\d+\.?\d*(?:[eE][+-]?\d+)?[fFLl]?\b)", numberFmt);

    // ── Strings (single-line double-quoted) ──
    add(R"("[^"\\]*(\\.[^"\\]*)*")", stringFmt);

    // ── Comments ──
    add("//[^\n]*", commentFmt);
    m_multiRules.append({
        QRegularExpression(R"(/\*)"),
        QRegularExpression(R"(\*/)"),
        commentFmt
    });
}

// ════════════════════════════════════════════════════════════════════
// Bash / Shell highlighting
// ════════════════════════════════════════════════════════════════════

void OFHighlighter::setupBash()
{
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#0000CC"));
    keywordFmt.setFontWeight(QFont::Bold);

    QTextCharFormat builtinFmt;
    builtinFmt.setForeground(QColor("#2B91AF"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#C04000"));

    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#808080"));
    commentFmt.setFontItalic(true);

    QTextCharFormat variableFmt;
    variableFmt.setForeground(QColor("#B8860B"));

    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#008000"));

    auto add = [&](const QString &pat, const QTextCharFormat &fmt) {
        m_rules.append({QRegularExpression(pat), fmt});
    };

    // ── Control flow ──
    for (auto &kw : {
        "if","then","else","elif","fi","case","esac",
        "for","while","until","do","done","in","select",
        "function","return","exit","break","continue","declare",
        "local","export","readonly","unset","shift"
    }) add(QString("\\b%1\\b").arg(kw), keywordFmt);

    // ── Built-in commands ──
    for (auto &bi : {
        "echo","printf","cd","pwd","ls","cat","rm","cp","mv",
        "mkdir","rmdir","chmod","chown","test","source","exec",
        "set","eval","alias","type","command","builtin","enable",
        "trap","wait","read","mapfile"
    }) add(QString("\\b%1\\b").arg(bi), builtinFmt);

    // ── Variables ──
    add(R"(\$\{?[A-Za-z_]\w*\}?)", variableFmt);
    add(R"(\$\d+)", variableFmt);
    add(R"(\$[@*#?$!\-])", variableFmt);

    // ── Numbers ──
    add(R"(\b\d+\.?\d*\b)", numberFmt);

    // ── Strings ──
    add(R"("[^"\\]*(\\.[^"\\]*)*")", stringFmt);
    add(R"('[^']*')", stringFmt);

    // ── Shebang ──
    QTextCharFormat shebangFmt;
    shebangFmt.setForeground(QColor("#9B4F96"));
    shebangFmt.setFontWeight(QFont::Bold);
    add(R"(^#!.*)", shebangFmt);

    // ── Comments ──
    add("#[^\n]*", commentFmt);
}

// ════════════════════════════════════════════════════════════════════
// Python highlighting
// ════════════════════════════════════════════════════════════════════

void OFHighlighter::setupPython()
{
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#0000CC"));
    keywordFmt.setFontWeight(QFont::Bold);

    QTextCharFormat builtinFmt;
    builtinFmt.setForeground(QColor("#2B91AF"));

    QTextCharFormat decoratorFmt;
    decoratorFmt.setForeground(QColor("#9B4F96"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#C04000"));

    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#808080"));
    commentFmt.setFontItalic(true);

    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#008000"));

    QTextCharFormat selfFmt;
    selfFmt.setForeground(QColor("#8B0000"));
    selfFmt.setFontItalic(true);

    auto add = [&](const QString &pat, const QTextCharFormat &fmt) {
        m_rules.append({QRegularExpression(pat), fmt});
    };

    // ── Keywords ──
    for (auto &kw : {
        "and","as","assert","async","await","break","class","continue",
        "def","del","elif","else","except","finally","for","from",
        "global","if","import","in","is","lambda","nonlocal","not",
        "or","pass","raise","return","try","while","with","yield",
        "True","False","None"
    }) add(QString("\\b%1\\b").arg(kw), keywordFmt);

    // ── Built-in functions ──
    for (auto &bi : {
        "print","len","range","int","float","str","list","dict","set",
        "tuple","bool","type","isinstance","hasattr","getattr","setattr",
        "open","enumerate","zip","map","filter","sorted","reversed",
        "min","max","sum","abs","round","all","any","iter","next",
        "super","self","cls","Exception","ValueError","TypeError",
        "KeyError","IndexError","RuntimeError","OSError","IOError"
    }) add(QString("\\b%1\\b").arg(bi), builtinFmt);

    // ── Decorators ──
    add(R"(@\w+)", decoratorFmt);

    // ── self / cls ──
    add(R"(\bself\b)", selfFmt);
    add(R"(\bcls\b)", selfFmt);

    // ── Numbers ──
    add(R"(\b\d+\.?\d*(?:[eE][+-]?\d+)?[jJ]?\b)", numberFmt);
    add(R"(\b0[xX][0-9a-fA-F]+)", numberFmt);
    add(R"(\b0[oO][0-7]+)", numberFmt);
    add(R"(\b0[bB][01]+)", numberFmt);

    // ── Strings (single, double, triple) ──
    add(R"("[^"\\]*(\\.[^"\\]*)*")", stringFmt);
    add(R"('[^'\\]*(\\.[^'\\]*)*')", stringFmt);
    add(R"('''[^'\\]*(\\.[^'\\]*)*''')", stringFmt);
    add(R"(\"\"\"[^\"\\]*(\\.[^\"\\]*)*\"\"\")", stringFmt);

    // ── f-strings ──
    add(R"(f"[^"\\]*(\\.[^"\\]*)*")", stringFmt);
    add(R"(f'[^'\\]*(\\.[^'\\]*)*')", stringFmt);

    // ── Comments ──
    add("#[^\n]*", commentFmt);
}

// ════════════════════════════════════════════════════════════════════
// JSON highlighting
// ════════════════════════════════════════════════════════════════════

void OFHighlighter::setupJSON()
{
    QTextCharFormat keyFmt;
    keyFmt.setForeground(QColor("#0000CC"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#C04000"));

    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#008000"));

    QTextCharFormat boolFmt;
    boolFmt.setForeground(QColor("#0000CC"));
    boolFmt.setFontWeight(QFont::Bold);

    auto add = [&](const QString &pat, const QTextCharFormat &fmt) {
        m_rules.append({QRegularExpression(pat), fmt});
    };

    add(R"("[^"\\]*(\\.[^"\\]*)*"\s*:)", keyFmt);
    add(R"("[^"\\]*(\\.[^"\\]*)*")", stringFmt);
    add(R"(\b-?\d+\.?\d*(?:[eE][+-]?\d+)?\b)", numberFmt);
    add(R"(\b(true|false|null)\b)", boolFmt);
}

// ════════════════════════════════════════════════════════════════════
// CMake highlighting
// ════════════════════════════════════════════════════════════════════

void OFHighlighter::setupCMake()
{
    QTextCharFormat cmdFmt;
    cmdFmt.setForeground(QColor("#0000CC"));
    cmdFmt.setFontWeight(QFont::Bold);

    QTextCharFormat varFmt;
    varFmt.setForeground(QColor("#B8860B"));

    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#808080"));
    commentFmt.setFontItalic(true);

    auto add = [&](const QString &pat, const QTextCharFormat &fmt) {
        m_rules.append({QRegularExpression(pat), fmt});
    };

    for (auto &cmd : {
        "cmake_minimum_required","project","set","unset","list",
        "add_executable","add_library","target_link_libraries",
        "target_include_directories","target_compile_definitions",
        "target_compile_options","target_sources","find_package",
        "include","include_directories","link_directories",
        "add_subdirectory","install","file","message","option",
        "if","elseif","else","endif","foreach","endforeach",
        "while","endwhile","macro","endmacro","function","endfunction",
        "string","separate_arguments"
    }) add(QString("\\b%1\\b").arg(cmd), cmdFmt);

    add(R"(\$\{[^}]+\})", varFmt);
    add("#[^\n]*", commentFmt);

    // Bracketed comments for CMake >= 3.0
    m_multiRules.append({
        QRegularExpression(R"(#\[=*\()"),
        QRegularExpression(R"(\)=*])"),
        commentFmt
    });
}

// ────────────────────────────────────────────────────────────────────
// Plain text
// ────────────────────────────────────────────────────────────────────

void OFHighlighter::setupPlainText()
{
    // No highlighting rules needed
}

// ────────────────────────────────────────────────────────────────────
// highlightBlock — dispatches single-line rules + multi-line spans
// ────────────────────────────────────────────────────────────────────

void OFHighlighter::highlightBlock(const QString &text)
{
    if (m_language == FileLanguage::PlainText
        || m_language == FileLanguage::Unknown)
        return;

    // ── Single-line rules ──
    for (const auto &rule : m_rules) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), rule.format);
        }
    }

    // ── Multi-line rules (typically block comments) ──
    setCurrentBlockState(0);
    for (const auto &ml : m_multiRules)
        matchMultiLine(text, ml);
}

bool OFHighlighter::matchMultiLine(const QString &text,
                                    const MultiLineRule &mlRule)
{
    int startIdx = 0;
    if (previousBlockState() != 1) {
        auto sm = mlRule.startPattern.match(text);
        startIdx = sm.hasMatch() ? sm.capturedStart() : -1;
    }

    while (startIdx >= 0) {
        auto em = mlRule.endPattern.match(text, startIdx);
        int endIdx = em.hasMatch() ? em.capturedEnd() : -1;
        int len;
        if (endIdx == -1) {
            setCurrentBlockState(1);
            len = text.length() - startIdx;
        } else {
            len = endIdx - startIdx;
        }
        if (len > 0)
            setFormat(startIdx, len, mlRule.format);
        break;
    }
    return true;
}
