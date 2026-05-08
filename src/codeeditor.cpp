#include "codeeditor.h"
#include "linenumberarea.h"
#include "ofhighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QFont>
#include <QRegularExpression>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QKeyEvent>
#include <QStringListModel>

// ── Keyword sets ────────────────────────────────────────────────
static QStringList cppKeywords() { return {
    "class","struct","enum","union","namespace","using","typedef",
    "template","typename","const","constexpr","static","extern","inline","virtual","override","final",
    "public","private","protected","friend",
    "if","else","switch","case","default","break","continue","return","goto",
    "for","while","do","try","catch","throw",
    "new","delete","sizeof","typeid","dynamic_cast","static_cast","const_cast","reinterpret_cast",
    "void","bool","char","short","int","long","float","double","auto","decltype","nullptr","true","false",
    "include","define","ifdef","ifndef","endif","pragma","error","undef",
    "volVectorField","volScalarField","volTensorField","volSymmTensorField",
    "surfaceScalarField","surfaceVectorField","surfaceTensorField",
    "pointScalarField","pointVectorField","pointTensorField",
    "dimensionedScalar","dimensionedVector","dimensionedTensor",
    "IOdictionary","IOobject","IOField","autoPtr","tmp","PtrList",
    "fvMesh","polyMesh","fvPatch","polyPatch","fvPatchField","fvsPatchField",
    "Foam","runTime","mesh","time","argList","Info","Warning","FatalError",
};}

static QStringList pyKeywords() { return {
    "False","None","True","and","as","assert","async","await","break",
    "class","continue","def","del","elif","else","except","finally",
    "for","from","global","if","import","in","is","lambda","nonlocal",
    "not","or","pass","raise","return","try","while","with","yield",
    "self","cls","__init__","__name__","__main__","__file__",
    "print","range","len","list","dict","set","tuple","str","int","float","bool",
    "open","read","write","append","close","with",
    "import","from","numpy","matplotlib","pandas",
    "def","return","if","else","elif","for","while","break","continue",
};}

static QStringList ofKeywords() { return {
    // ── Dictionary structure ──
    "FoamFile","version","format","class","object","location",
    "boundaryField","internalField","dimensions","uniform","nonuniform",
    "type","value","gradient","refValue","refGradient","valueFraction",
    "inletValue","outletValue","freestreamValue","phi","rho","rhoInlet",
    "flowRate","massFlowRate","volumetricFlowRate","extrapolateProfile",
    // ── Basic BCs ──
    "fixedValue","uniformFixedValue","zeroGradient","fixedGradient",
    "mixed","calculated","scaledFixedValue","uniformFixedGradient",
    "fixedMean","prghPressure","fixedFluxPressure","fixedPressureCompressibleDensity",
    "fixedFluxExtrapolatedPressure",
    // ── Wall BCs ──
    "noSlip","slip","partialSlip","movingWallVelocity","rotatingWallVelocity",
    "translatingWallVelocity","fixedNormalSlip","activeBaffleVelocity",
    // ── Wall functions ──
    "kqRWallFunction","epsilonWallFunction","omegaWallFunction",
    "nutkWallFunction","nutUSpaldingWallFunction","nutkRoughWallFunction",
    "nutURoughWallFunction","nutLowReWallFunction","nutUWallFunction",
    "nutUBlendedWallFunction","nutUTabulatedWallFunction",
    "kLowReWallFunction","alphatWallFunction",
    // ── Inlet BCs ──
    "flowRateInletVelocity","pressureInletOutletVelocity","pressureInletVelocity",
    "pressureDirectedInletVelocity","pressureNormalInletOutletVelocity",
    "surfaceNormalFixedValue","cylindricalInletVelocity",
    "swirlFlowRateInletVelocity","turbulentIntensityKineticEnergyInlet",
    "turbulentInlet","turbulentDFSEMInlet","turbulentDigitalFilterInlet",
    "fixedProfile","interstitialInletVelocity","swirlFanVelocity",
    "pressureDirectedInletOutletVelocity","pressureInletOutletParSlipVelocity",
    "pressureInletUniformVelocity","pressurePIDControlInletVelocity",
    "rotatingPressureInletOutletVelocity","variableHeightFlowRateInletVelocity",
    "uniformNormalFixedValue",
    // ── Outlet BCs ──
    "inletOutlet","outletInlet","advective","waveTransmissive",
    "freestream","freestreamVelocity","freestreamPressure",
    "supersonicFreestream","outletMappedUniformInlet",
    "flowRateOutletVelocity","matchedFlowRateOutletVelocity",
    "outletPhaseMeanVelocity","outletMachNumberPressure",
    "acousticWaveTransmissive","fixedMeanOutletInlet",
    "fluxCorrectedVelocity","uniformInletOutlet",
    // ── Pressure BCs ──
    "totalPressure","uniformTotalPressure","prghTotalPressure",
    "fanPressure","uniformDensityHydrostaticPressure",
    "prghTotalHydrostaticPressure","plenumPressure",
    "rotatingTotalPressure","syringePressure","waveSurfacePressure",
    "variableHeightFlowRate","phaseHydrostaticPressure",
    "prghPermeableAlphaTotalPressure",
    // ── Temperature ──
    "totalTemperature","inletOutletTotalTemperature",
    // ── Mapped / Coupled ──
    "mappedFixedValue","timeVaryingMappedFixedValue","mappedMixed",
    "mappedFlowRate","mappedFixedInternalValue","mappedVelocityFluxFixedValue",
    // ── Coded ──
    "codedFixedValue","codedMixed",
    // ── Jump / Special ──
    "fixedJump","fixedJumpAMI","uniformJump","uniformJumpAMI",
    "interfaceCompression",
    // ── Constraint ──
    "empty","symmetry","symmetryPlane","wedge","cyclic","cyclicAMI",
    "cyclicACMI","cyclicSlip","processor","processorCyclic","patch","wall",
    // ── Schemes ──
    "ddtSchemes","gradSchemes","divSchemes","laplacianSchemes",
    "interpolationSchemes","snGradSchemes","fluxRequired",
    "Euler","backward","steadyState","CrankNicolson","localEuler",
    "Gauss","linear","leastSquares","corrected","orthogonal","limited",
    "limitedLinear","linearUpwind","upwind","vanLeer","QUICK","midPoint",
    "cellLimited","cellMDLimited","pointCellsLeastSquares",
    // ── Solvers ──
    "solvers","relaxationFactors","PIMPLE","SIMPLE","PISO",
    "residualControl","nOuterCorrectors","nCorrectors","momentumPredictor",
    "nNonOrthogonalCorrectors","pRefCell","pRefValue","pRefPoint",
    "tolerance","relTol","preconditioner","smoother",
    "PCG","PBiCG","PBiCGStab","GAMG","smoothSolver",
    "DIC","DILU","diagonal","DILUGaussSeidel","GaussSeidel",
    // ── Mesh / blockMesh ──
    "convertToMeters","vertices","blocks","edges","boundary","faces",
    "neighbour","owner","points","arc","spline","polyLine","simpleGrading",
    "hex","wedge","symmetryPlane","emptyPatch","wallPatch",
    // ── controlDict ──
    "application","startFrom","startTime","stopAt","endTime","deltaT",
    "writeControl","writeInterval","purgeWrite","writeFormat","writePrecision",
    "writeCompression","timeFormat","timePrecision","runTimeModifiable",
    "adjustTimeStep","maxCo","maxDeltaT","functions","libs",
    "timeStep","clockTime","cpuTime","runTime","adjustableRunTime",
    // ── decomposeParDict ──
    "numberOfSubdomains","method","simple","hierarchical","scotch","metis",
    "manual","coeffs","n","preservePatches",
    // ── snappyHexMesh ──
    "castellatedMesh","snap","addLayers","geometry","refinementSurfaces",
    "resolveFeatureAngle","features","refinementRegions","locationInMesh",
    "maxGlobalCells","maxLocalCells","minRefinementCells","nCellsBetweenLevels",
    "allowFreeStandingZoneFaces","nSmoothPatch","tolerance","nSolveIter",
    "nRelaxIter","nSurfaceLayers","expansionRatio","finalLayerThickness",
    "minThickness","featureAngle","slipFeatureAngle","maxNonOrtho",
    "maxBoundarySkewness","maxInternalSkewness","maxConcave","minVol",
    "minTetQuality","minArea","minTwist","mergeTolerance",
    // ── Field names ──
    "U","p","p_rgh","k","epsilon","omega","nut","alphat","T","alpha",
    "phi","rho","thermo:mu","thermo:psi","thermo:alpha","Cp","Cv",
    "nuTilda","R","tau","vorticity","Q","yPlus","wallShearStress",
    // ── Turbulence model names ──
    "laminar","kEpsilon","kOmega","kOmegaSST","kOmegaSSTLM",
    "RNGkEpsilon","realizableKE","LaunderSharmaKE","kOmega2006",
    "v2f","LRR","SSG","EBRSM",
    "Smagorinsky","WALE","dynamicKEqn","kEqn","dynamicLagrangian",
    "DeardorffDiffStress","SpalartAllmaras",
    // ── Turbulence parameters ──
    "Cmu","C1","C2","C3","Ceps2","sigmak","sigmaEps","sigmaOmega",
    "alphaK1","alphaOmega1","alphaOmega2","beta1","beta2","betaStar",
    "gamma1","gamma2","Ce","Ck","Bk","kappa","E","Prt","Prt",
    "turbulence","printCoeffs","RAS","LES",
    // ── Common OpenFOAM objects ──
    "runTime","mesh","time","argList","Info","Warning","FatalError",
    "IOdictionary","IOobject","IOField","autoPtr","tmp","PtrList",
    "Switch","word","scalar","vector","tensor","symmTensor","sphericalTensor",
    "label","HashTable","dictionary","Foam","endl","nl","tab",
};}

// ── Constructor ─────────────────────────────────────────────────
CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);
    m_highlighter = new OFHighlighter(document());

    QFont font("Consolas", 11);
    font.setStyleHint(QFont::Monospace);
    setFont(font);
    setTabStopDistance(40);

    setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    setupCompleter();

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

// ── Auto-completion ─────────────────────────────────────────────
void CodeEditor::setupCompleter()
{
    m_completer = new QCompleter(this);
    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setMaxVisibleItems(10);
    m_completer->popup()->setStyleSheet(
        "QAbstractItemView { font-family: Consolas; font-size: 12px; "
        "padding: 2px; border: 1px solid #aaa; background: white; }"
        "QAbstractItemView::item { padding: 3px 8px; }"
        "QAbstractItemView::item:selected { background: #0078D7; color: white; }");

    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
}

void CodeEditor::setAutoCompletion(bool enabled)
{
    m_autoComplete = enabled;
    if (!enabled)
        m_completer->popup()->hide();
}

QString CodeEditor::wordUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CodeEditor::insertCompletion(const QString &text)
{
    QTextCursor tc = textCursor();
    int extra = text.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(text.right(extra));
    setTextCursor(tc);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    // Let completer handle its own keys
    if (m_completer && m_completer->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Tab:
        case Qt::Key_Escape:
            e->ignore();
            return;
        default:
            break;
        }
    }

    QPlainTextEdit::keyPressEvent(e);

    if (!m_autoComplete || !m_completer) return;

    QString prefix = wordUnderCursor();
    if (prefix.length() < 2) {
        m_completer->popup()->hide();
        return;
    }

    // Build keyword list based on language
    if (m_completer->completionPrefix() != prefix) {
        QStringList words;
        switch (m_language) {
        case FileLanguage::Python:
            words = pyKeywords(); break;
        case FileLanguage::Cpp:
        case FileLanguage::CppHeader:
        case FileLanguage::C:
        case FileLanguage::OpenFOAM:
        case FileLanguage::CMake:
            words = cppKeywords() + ofKeywords(); break;
        default:
            words = ofKeywords() + cppKeywords(); break;
        }
        words.removeDuplicates();
        m_completer->setModel(new QStringListModel(words, m_completer));
        m_completer->setCompletionPrefix(prefix);
    } else {
        m_completer->setCompletionPrefix(prefix);
    }

    if (m_completer->completionCount() > 0) {
        QRect cr = cursorRect();
        cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                     + m_completer->popup()->verticalScrollBar()->sizeHint().width() + 4);
        m_completer->complete(cr);
    } else {
        m_completer->popup()->hide();
    }
}

void CodeEditor::setLanguage(FileLanguage lang)
{
    if (m_language == lang) return;
    m_language = lang;
    m_highlighter->setLanguage(lang);
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extras;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(185);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extras.append(selection);
    }

    setExtraSelections(extras);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#F0F0F0"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#999999"));
            painter.drawText(0, top, m_lineNumberArea->width() - 4,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool CodeEditor::maybeSave()
{
    if (!document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, "OpenFOAM GUI",
                             "The file has been modified.\n"
                             "Do you want to save your changes?",
                             QMessageBox::Save | QMessageBox::Discard
                             | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        if (m_filePath.isEmpty())
            return false;
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void CodeEditor::toggleComment()
{
    // Determine comment prefix based on language
    QString commentPrefix;
    switch (m_language) {
    case FileLanguage::OpenFOAM:
    case FileLanguage::Cpp:
    case FileLanguage::CppHeader:
    case FileLanguage::C:
    case FileLanguage::JSON:
    case FileLanguage::CMake:
    case FileLanguage::XML:
        commentPrefix = "//";
        break;
    case FileLanguage::Bash:
    case FileLanguage::Python:
        commentPrefix = "#";
        break;
    default:
        commentPrefix = "//";
        break;
    }

    QTextCursor cursor = textCursor();
    int startPos, endPos;

    if (cursor.hasSelection()) {
        startPos = cursor.selectionStart();
        endPos   = cursor.selectionEnd();
    } else {
        // No selection → operate on current line
        cursor.movePosition(QTextCursor::StartOfBlock);
        startPos = cursor.position();
        cursor.movePosition(QTextCursor::EndOfBlock);
        endPos = cursor.position();
    }

    // Adjust start to beginning of its line
    QTextCursor startCur(document());
    startCur.setPosition(startPos);
    startCur.movePosition(QTextCursor::StartOfBlock);
    startPos = startCur.position();

    // Adjust end to end of its line (or keep exact if mid-line)
    QTextCursor endCur(document());
    endCur.setPosition(endPos);

    // Check if ALL non-empty lines in selection are already commented
    QTextCursor checkCur(document());
    checkCur.setPosition(startPos);
    bool allCommented = true;

    while (checkCur.position() < endPos) {
        checkCur.movePosition(QTextCursor::StartOfBlock);
        QString line = checkCur.block().text();
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty() && !trimmed.startsWith(commentPrefix)) {
            allCommented = false;
            break;
        }
        checkCur.movePosition(QTextCursor::NextBlock);
        if (checkCur.position() == checkCur.block().position()
            && !checkCur.movePosition(QTextCursor::NextBlock))
            break;
        if (checkCur.position() >= endPos) break;
    }

    // Operate line by line
    QTextCursor opCur(document());
    opCur.setPosition(startPos);
    opCur.beginEditBlock();

    while (opCur.position() < endPos || opCur.block().position() == startPos) {
        opCur.movePosition(QTextCursor::StartOfBlock);
        int blockStart = opCur.position();
        QString line = opCur.block().text();
        QString trimmed = line.trimmed();

        if (trimmed.isEmpty()) {
            // skip empty lines
            if (!opCur.movePosition(QTextCursor::NextBlock)) break;
            if (opCur.position() >= endPos && opCur.block().position() != startPos) break;
            continue;
        }

        // Find leading whitespace
        int leadingSpaces = 0;
        while (leadingSpaces < line.length() && line[leadingSpaces].isSpace())
            leadingSpaces++;

        if (allCommented) {
            // Remove comment prefix
            int commentStart = line.indexOf(commentPrefix, leadingSpaces);
            if (commentStart >= 0) {
                opCur.setPosition(blockStart + commentStart);
                opCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                                   commentPrefix.length());
                opCur.removeSelectedText();

                // Remove trailing space after comment prefix
                opCur.setPosition(blockStart + commentStart);
                QTextCursor peekCur(document());
                peekCur.setPosition(blockStart + commentStart);
                peekCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
                if (peekCur.selectedText() == " ") {
                    peekCur.removeSelectedText();
                }
            }
        } else {
            // Add comment prefix after leading whitespace
            opCur.setPosition(blockStart + leadingSpaces);
            opCur.insertText(commentPrefix + " ");
        }

        if (!opCur.movePosition(QTextCursor::NextBlock)) break;
        if (opCur.position() >= endPos && opCur.block().position() != startPos) break;
    }

    opCur.endEditBlock();

    // Restore a reasonable selection
    QTextCursor restoreCur(document());
    restoreCur.setPosition(startPos);
    setTextCursor(restoreCur);
}
