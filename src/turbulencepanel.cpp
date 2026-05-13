#include "turbulencepanel.h"
#include "codeeditor.h"
#include "ofparser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileInfo>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QFont>
#include <QFrame>
#include <QHeaderView>
#include <QRegularExpression>
#include <QMessageBox>
#include <QMenu>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QTextEdit>

TurbulencePanel::TurbulencePanel(QWidget *parent) : QWidget(parent) { setupUI(); }

void TurbulencePanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Header ──
    auto *hdr = new QFrame();
    hdr->setStyleSheet("QFrame#turbHeader { background: #F0F4E8; border-bottom: 2px solid #388E3C; padding: 6px 10px; }");
    hdr->setObjectName("turbHeader");
    auto *hl = new QHBoxLayout(hdr);
    hl->setContentsMargins(8, 6, 8, 6);
    auto *ico = new QLabel("T");
    ico->setFixedSize(26, 26);
    ico->setAlignment(Qt::AlignCenter);
    ico->setStyleSheet("background: #388E3C; color: white; border-radius: 13px; font-weight: bold; font-size: 13px;");
    hl->addWidget(ico);
    m_headerLabel = new QLabel("No turbulence file selected");
    m_headerLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #333;");
    m_headerLabel->setWordWrap(true);
    hl->addWidget(m_headerLabel, 1);
    root->addWidget(hdr);

    m_pathLabel = new QLabel();
    m_pathLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px 10px; background: #FAFAFA;");
    m_pathLabel->setWordWrap(true);
    root->addWidget(m_pathLabel);

    // ── Category tabs ──
    m_tabBar = new QWidget();
    m_tabBar->setStyleSheet("QWidget#tabBar { background: #F5F5F5; border-bottom: 1px solid #DDD; }");
    m_tabBar->setObjectName("tabBar");
    auto *tl = new QHBoxLayout(m_tabBar);
    tl->setContentsMargins(4, 4, 4, 4); tl->setSpacing(2);
    m_tabGroup = new QButtonGroup(this);
    m_tabGroup->setExclusive(true);
    buildCategoryTabs();
    root->addWidget(m_tabBar);

    // ── Main split ──
    auto *ms = new QSplitter(Qt::Horizontal);
    ms->setChildrenCollapsible(false);

    // Left: model list
    auto *lp = new QWidget();
    auto *ll = new QVBoxLayout(lp);
    ll->setContentsMargins(4, 2, 2, 2); ll->setSpacing(2);
    m_modelList = new QListWidget();
    m_modelList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_modelList->setStyleSheet(
        "QListWidget { font-size: 12px; border: 1px solid #DDD; border-radius: 3px; background: white; }"
        "QListWidget::item { padding: 4px 8px; }"
        "QListWidget::item:selected { background: #388E3C; color: white; }"
        "QListWidget::item:hover:!selected { background: #E9F5EA; }");
    ll->addWidget(m_modelList);
    m_countLabel = new QLabel();
    m_countLabel->setStyleSheet("color: #888; font-size: 10px; padding: 0 4px;");
    ll->addWidget(m_countLabel);
    ms->addWidget(lp);

    // Right: detail
    auto *rp = new QWidget();
    auto *rl = new QVBoxLayout(rp);
    rl->setContentsMargins(6, 4, 6, 4); rl->setSpacing(4);

    m_modelNameLabel = new QLabel("Select a turbulence model");
    m_modelNameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #388E3C;");
    m_modelNameLabel->setWordWrap(true);
    rl->addWidget(m_modelNameLabel);

    m_modelRefLabel = new QLabel();
    m_modelRefLabel->setWordWrap(true);
    m_modelRefLabel->setStyleSheet("color: #666; font-size: 10px; font-style: italic; padding: 2px 4px;");
    rl->addWidget(m_modelRefLabel);

    auto *fHdr = new QLabel("Key Equations:");
    fHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(fHdr);

    m_formulaLabel = new QLabel();
    m_formulaLabel->setWordWrap(true);
    m_formulaLabel->setStyleSheet(
        "color: #333; font-size: 10px; padding: 6px 8px; background: #FFFEF5; "
        "border: 1px solid #E8E0C0; border-radius: 3px; font-family: Consolas, monospace;");
    rl->addWidget(m_formulaLabel);

    auto *pHdr = new QLabel("Model Coefficients:");
    pHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(pHdr);

    m_paramTable = new QTableWidget(0, 4);
    m_paramTable->setHorizontalHeaderLabels({"Coefficient", "Description", "Type", "Default"});
    m_paramTable->setAlternatingRowColors(true);
    m_paramTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_paramTable->verticalHeader()->setVisible(false);
    m_paramTable->setMinimumHeight(100);
    m_paramTable->setStyleSheet(
        "QTableWidget { font-size: 11px; border: 1px solid #DDD; gridline-color: #EEE; }"
        "QTableWidget::item { padding: 2px 4px; }"
        "QHeaderView::section { background: #EEE; padding: 3px 6px; font-size: 10px; font-weight: bold; border: none; }");
    m_paramTable->horizontalHeader()->setStretchLastSection(true);
    m_paramTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_paramTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    rl->addWidget(m_paramTable, 2);

    auto *exHdr = new QLabel("Configuration Preview:");
    exHdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rl->addWidget(exHdr);

    m_previewEdit = new QTextEdit();
    m_previewEdit->setReadOnly(true);
    m_previewEdit->setFont(QFont("Consolas", 10));
    m_previewEdit->setMaximumHeight(120);
    m_previewEdit->setStyleSheet(
        "QTextEdit { background: #1E1E1E; color: #DCDCDC; border: 1px solid #333; "
        "border-radius: 3px; padding: 6px; }");
    rl->addWidget(m_previewEdit);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    m_previewBtn = new QPushButton("Refresh Preview");
    m_previewBtn->setStyleSheet("QPushButton { padding: 5px 14px; border: 1px solid #CCC; border-radius: 3px; background: #F5F5F5; font-size: 12px; } QPushButton:hover { background: #E8E8E8; }");
    m_applyBtn = new QPushButton("Apply to Editor");
    m_applyBtn->setEnabled(false);
    m_applyBtn->setStyleSheet("QPushButton { padding: 5px 18px; border: none; border-radius: 3px; background: #388E3C; color: white; font-weight: bold; font-size: 12px; } QPushButton:hover { background: #2E7D32; } QPushButton:disabled { background: #CCC; color: #888; }");
    btnRow->addWidget(m_previewBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_applyBtn);
    rl->addLayout(btnRow);

    ms->addWidget(rp);
    ms->setStretchFactor(0, 30);
    ms->setStretchFactor(1, 70);
    root->addWidget(ms, 1);

    // Connections
    connect(m_modelList, &QListWidget::currentItemChanged, this, &TurbulencePanel::onModelSelected);
    connect(m_modelList, &QListWidget::itemDoubleClicked, this, &TurbulencePanel::onModelDoubleClicked);
    connect(m_modelList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = m_modelList->itemAt(pos);
        if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
        m_modelList->setCurrentItem(item);
        onModelSelected(item);
        QMenu menu;
        // 1) Value only (model name)
        QAction *valAct = menu.addAction(
            QString("Insert \"%1\" (model name only)").arg(m_selectedModel.name));
        connect(valAct, &QAction::triggered, [this]() {
            if (!m_editor) return;
            QTextCursor c = m_editor->textCursor();
            c.insertText(m_selectedModel.name);
            m_editor->setTextCursor(c);
        });

        menu.addSeparator();

        // 2) Full config
        QAction *act = menu.addAction("Insert Turbulence Config");
        connect(act, &QAction::triggered, [this]() {
            if (!m_editor || m_selectedModel.name.isEmpty()) return;
            onPreviewConfig();
            QTextCursor c = m_editor->textCursor();
            c.insertText(m_previewEdit->toPlainText());
            m_editor->setTextCursor(c);
            m_applyBtn->setText("✓ Inserted!");
            QTimer::singleShot(1500, this, [this]() { m_applyBtn->setText("Apply to Editor"); });
        });
        menu.exec(m_modelList->mapToGlobal(pos));
    });
    connect(m_previewBtn, &QPushButton::clicked, this, &TurbulencePanel::onPreviewConfig);
    connect(m_applyBtn, &QPushButton::clicked, this, &TurbulencePanel::onApplyConfig);
#if QT_CONFIG(shortcut)
    connect(m_tabGroup, &QButtonGroup::idClicked, this, &TurbulencePanel::onCategoryButtonClicked);
#endif
}

void TurbulencePanel::buildCategoryTabs()
{
    struct { int id; QString lb; } cats[] = {
        {-1,"All"}, {0,"RAS"}, {1,"LES"}, {2,"DES"}, {3,"RSTM"}, {4,"Laminar"},
    };
    for (auto *b : m_tabButtons) delete b;
    m_tabButtons.clear();
    for (auto &c : cats) {
        auto *btn = new QToolButton();
        btn->setText(c.lb); btn->setCheckable(true); btn->setAutoRaise(true);
        btn->setStyleSheet(
            "QToolButton { padding: 4px 12px; border-radius: 10px; font-size: 11px; font-weight: bold; color: #555; }"
            "QToolButton:hover { background: #E9F5EA; color: #388E3C; }"
            "QToolButton:checked { background: #388E3C; color: white; }");
        m_tabGroup->addButton(btn, c.id);
        m_tabButtons[c.id] = btn;
        static_cast<QHBoxLayout*>(m_tabBar->layout())->addWidget(btn);
    }
    if (m_tabButtons.contains(-1)) m_tabButtons[-1]->setChecked(true);
}

void TurbulencePanel::onCategoryButtonClicked(int id)
{
    if (id == -1) {
        populateModels(TurbulenceModelDatabase::instance()->allModels());
        return;
    }
    TurbModelCategory cat;
    switch (id) { case 0:cat=TurbModelCategory::RAS;break; case 1:cat=TurbModelCategory::LES;break; case 2:cat=TurbModelCategory::DES;break; case 3:cat=TurbModelCategory::ReynoldsStress;break; case 4:cat=TurbModelCategory::Laminar;break; default:return; }
    populateModels(TurbulenceModelDatabase::instance()->modelsByCategory(cat));
}

// ────────────────────────────────────────────────────────────────────
// Load file
// ────────────────────────────────────────────────────────────────────

void TurbulencePanel::loadTurbulenceFile(const QString &filePath, const QString &content)
{
    m_filePath = filePath;
    m_currentContent = content;
    QFileInfo fi(filePath);

    m_headerLabel->setText(QString("<b>%1</b> — turbulenceProperties").arg(fi.fileName()));
    m_pathLabel->setText(filePath);

    parseCurrentConfig(content);

    auto *db = TurbulenceModelDatabase::instance();
    populateModels(db->allModels());

    // Try to select the current model
    if (!m_currentModelName.isEmpty())
        for (int i = 0; i < m_modelList->count(); ++i) {
            auto *item = m_modelList->item(i);
            if (item && (item->flags() & Qt::ItemIsSelectable)
                && item->data(Qt::UserRole).toString() == m_currentModelName) {
                m_modelList->setCurrentItem(item);
                break;
            }
        }

    if (m_tabButtons.contains(-1)) m_tabButtons[-1]->setChecked(true);
}

void TurbulencePanel::clear()
{
    m_headerLabel->setText("No turbulence file selected");
    m_pathLabel->clear();
    m_modelList->clear();
    m_modelNameLabel->setText("Select a turbulence model");
    m_modelRefLabel->clear();
    m_formulaLabel->clear();
    m_paramTable->setRowCount(0);
    m_previewEdit->clear();
    m_applyBtn->setEnabled(false);
    m_countLabel->clear();
}

void TurbulencePanel::parseCurrentConfig(const QString &content)
{
    // Extract simulationType
    QRegularExpression simRe(R"(simulationType\s+(\S+)\s*;)");
    auto sm = simRe.match(content);
    m_currentSimType = sm.hasMatch() ? sm.captured(1).toLower() : "ras";

    // Extract model name from model/RASModel/LESModel entry
    QRegularExpression modelRe(R"((model|RASModel|LESModel)\s+(\S+)\s*;)");
    auto mm = modelRe.match(content);
    m_currentModelName = mm.hasMatch() ? mm.captured(2) : QString();

    // Parse model coefficients sub-dict (e.g. kEpsilonCoeffs { ... })
    if (!m_currentModelName.isEmpty()) {
        QString coeffsKey = m_currentModelName + "Coeffs";
        QRegularExpression coeffRe(
            coeffsKey + R"(\s*\{([^}]*)\})",
            QRegularExpression::DotMatchesEverythingOption);
        auto cm = coeffRe.match(content);
        if (cm.hasMatch()) {
            QString body = cm.captured(1);
            QRegularExpression kvRe(R"((\w+)\s+([-\d.e+]+)\s*;)");
            auto it = kvRe.globalMatch(body);
            m_parsedCoeffs.clear();
            while (it.hasNext()) {
                auto m = it.next();
                m_parsedCoeffs[m.captured(1)] = m.captured(2);
            }
        }
    }
}

// ────────────────────────────────────────────────────────────────────
// Model list
// ────────────────────────────────────────────────────────────────────

void TurbulencePanel::populateModels(const QVector<TurbModelInfo> &models)
{
    m_modelList->clear();
    TurbModelCategory lastCat = (TurbModelCategory)(-1);
    for (const auto &m : models) {
        if (m.category != lastCat) {
            lastCat = m.category;
            auto *hi = new QListWidgetItem("── " + TurbulenceModelDatabase::categoryName(m.category) + " ──");
            hi->setFlags(Qt::NoItemFlags);
            hi->setForeground(QColor("#999"));
            QFont f = hi->font(); f.setBold(true); f.setPointSize(9); hi->setFont(f);
            m_modelList->addItem(hi);
        }
        auto *item = new QListWidgetItem(m.name);
        item->setData(Qt::UserRole, m.name);
        item->setToolTip(m.description);
        QColor tag;
        switch (m.category) {
        case TurbModelCategory::RAS: tag=QColor("#388E3C"); break;
        case TurbModelCategory::LES: tag=QColor("#8E44AD"); break;
        case TurbModelCategory::DES: tag=QColor("#E67E22"); break;
        case TurbModelCategory::ReynoldsStress: tag=QColor("#D73A0F"); break;
        case TurbModelCategory::Laminar: tag=QColor("#95A5A6"); break;
        }
        QPixmap px(8,8); px.fill(tag); item->setIcon(QIcon(px));
        m_modelList->addItem(item);
    }
    m_countLabel->setText(QString("%1 models").arg(models.size()));
}

void TurbulencePanel::onModelSelected(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    m_selectedModel = TurbulenceModelDatabase::instance()->modelInfo(item->data(Qt::UserRole).toString());
    showModelDetails(m_selectedModel);
}

void TurbulencePanel::onModelDoubleClicked(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    onModelSelected(item);
    onApplyConfig();
}

// ────────────────────────────────────────────────────────────────────
// Model detail
// ────────────────────────────────────────────────────────────────────

void TurbulencePanel::showModelDetails(const TurbModelInfo &info)
{
    m_modelNameLabel->setText(info.name);
    m_modelRefLabel->setText(info.references.isEmpty() ? "" : "Reference: " + info.references);
    m_formulaLabel->setText(info.formula.isEmpty() ? "(No formula available)" : info.formula);
    m_applyBtn->setEnabled(true);

    m_paramTable->setRowCount(0);
    for (const auto &p : info.params) {
        int r = m_paramTable->rowCount();
        m_paramTable->insertRow(r);
        auto *c0 = new QTableWidgetItem(p.name);
        c0->setFont(QFont("Consolas", 10));
        c0->setForeground(QColor("#0000CC"));
        c0->setFlags(c0->flags() & ~Qt::ItemIsEditable);
        m_paramTable->setItem(r, 0, c0);

        auto *c1 = new QTableWidgetItem(p.description);
        c1->setFlags(c1->flags() & ~Qt::ItemIsEditable);
        m_paramTable->setItem(r, 1, c1);

        auto *c2 = new QTableWidgetItem(p.type);
        c2->setForeground(QColor("#888"));
        c2->setFlags(c2->flags() & ~Qt::ItemIsEditable);
        m_paramTable->setItem(r, 2, c2);

        auto *c3 = new QTableWidgetItem(p.defaultValue);
        c3->setFont(QFont("Consolas", 10));
        c3->setForeground(QColor("#333"));
        m_paramTable->setItem(r, 3, c3);
    }
    if (info.params.isEmpty()) {
        m_paramTable->insertRow(0);
        m_paramTable->setItem(0, 0, new QTableWidgetItem("(none)"));
        m_paramTable->setItem(0, 1, new QTableWidgetItem("No user-specified coefficients (dynamic or built-in)"));
    }

    onPreviewConfig();
}

void TurbulencePanel::onPreviewConfig()
{
    if (m_selectedModel.name.isEmpty()) return;
    m_previewEdit->setPlainText(TurbulenceModelDatabase::generateConfig(m_selectedModel));
}

void TurbulencePanel::onApplyConfig()
{
    if (m_selectedModel.name.isEmpty() || !m_editor) return;
    onPreviewConfig();
    QString snip = m_previewEdit->toPlainText();
    QTextCursor c = m_editor->textCursor();
    c.insertText(snip);
    m_editor->setTextCursor(c);
    QApplication::clipboard()->setText(snip);
    m_applyBtn->setText("✓ Applied!");
    QTimer::singleShot(1500, this, [this]() { m_applyBtn->setText("Apply to Editor"); });
}

// ── Turbulence Inlet Parameter Calculator ──
void TurbulencePanel::onCalcTurb()
{
    auto *dlg = new QDialog(nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle("Turbulence Inlet Calculator — 0/ field values");
    dlg->resize(560, 680);
    auto *l = new QVBoxLayout(dlg);

    auto *form = new QFormLayout();

    auto *U = new QDoubleSpinBox(); U->setRange(0.1, 500); U->setValue(10);
    U->setDecimals(2); U->setSuffix(" m/s");
    form->addRow("Freestream velocity U:", U);

    auto *I_turb = new QDoubleSpinBox(); I_turb->setRange(0.1, 30); I_turb->setValue(5);
    I_turb->setDecimals(1); I_turb->setSuffix(" % (1-5% low, 5-20% medium, >20% high)");
    form->addRow("Turbulence intensity I:", I_turb);

    auto *L_char = new QDoubleSpinBox(); L_char->setRange(0.001, 100); L_char->setValue(1);
    L_char->setDecimals(3); L_char->setSuffix(" m (hydraulic diameter / chord)");
    form->addRow("Characteristic length L:", L_char);

    auto *nu = new QDoubleSpinBox(); nu->setRange(1e-7, 1e-3); nu->setValue(1.5e-5);
    nu->setDecimals(8); nu->setSuffix(" m²/s");
    nu->setSingleStep(1e-6);
    form->addRow("Kinematic viscosity ν:", nu);

    auto *result = new QTextEdit();
    result->setReadOnly(true);
    result->setFont(QFont("Consolas", 9));
    result->setStyleSheet("QTextEdit { background:#F5FFF5; border:1px solid #C0E0C0; "
                          "border-radius:3px; padding:8px; color:#333; }");
    result->setMinimumHeight(200);
    l->addLayout(form);
    l->addWidget(result);

    auto calc = [=]() -> QString {
        double vel = U->value();
        double I_val = I_turb->value() / 100.0;
        double L = L_char->value();
        double nuVal = nu->value();

        // ── Common parameters ──
        double uprime = vel * I_val;
        double k = 1.5 * uprime * uprime;
        double Lt = 0.07 * L;
        static const double Cmu = 0.09;
        static const double Cmu34 = 0.1643, Cmu14 = 0.5477;
        double epsilon = Cmu34 * qPow(k, 1.5) / Lt;
        double omega = qSqrt(k) / (Cmu14 * Lt);
        double nut = Cmu * k * k / epsilon;

        // ── SA ──
        double nuTilda = 5.0 * nuVal;

        // ── LES subgrid ──
        double Cs = 0.168; // Smagorinsky constant
        double Delta = qPow(L * L * L / 1e6, 1.0/3.0); // grid filter scale estimate
        double nuSgs_smag = Cs * Cs * Delta * Delta * qSqrt(2.0) * vel / L;
        double kSgs = nuSgs_smag * nuSgs_smag / (Cmu * Delta * Delta);

        // ── RSTM ──
        double R_ii = 2.0 / 3.0 * k;

        // ── v2f ──
        double v2_val = 2.0 / 3.0 * k; // isotropic assumption

        return QString(
            "Input: U=%1 m/s  I=%2%  L=%3 m  ν=%4  u'=%5 m/s  L_t=%6 m\n"
            "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
            "╔══ kEpsilon Family ═════════════════════════════╗\n"
            "║ Standard kEpsilon                              ║\n"
            "║   k       = %7  m²/s²                          ║\n"
            "║   epsilon = %8  m²/s³                          ║\n"
            "║   nut     = %9  m²/s                           ║\n"
            "║ RNG kEpsilon — same k, ε as standard            ║\n"
            "║ realizableKE — same k, ε (Cμ = f(mean strain))  ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ kOmega Family ═══════════════════════════════╗\n"
            "║ Standard kOmega / kOmegaSST / kOmega2006        ║\n"
            "║   k     = %7  m²/s²                             ║\n"
            "║   omega = %10  1/s                              ║\n"
            "║   nut   = %9  m²/s                              ║\n"
            "║ kOmegaSSTSAS — same k, ω as SST                 ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ SpalartAllmaras Family ══════════════════════╗\n"
            "║ SA / SA-DES / SA-DDES                            ║\n"
            "║   nuTilda ≈ ").arg(vel,0,'f',2).arg(I_val*100,0,'f',1).arg(L,0,'g',3).arg(nuVal,0,'g',4)
         .arg(uprime,0,'g',4).arg(Lt,0,'g',4)
         .arg(k,0,'g',4).arg(epsilon,0,'g',4).arg(nut,0,'g',4)
         .arg(omega,0,'g',4) +
            QString("%1  m²/s                              ║\n"
            "║   nut = nuTilda × fv1                           ║\n"
            "║   fv1 = χ³/(χ³ + Cv1³), χ = nuTilda/ν          ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ v2f (Durbin) ════════════════════════════════╗\n"
            "║   k  = %2  m²/s²                                ║\n"
            "║   ε  = %3  m²/s³                                ║\n"
            "║   v2 = %4  m²/s²  (isotropic: 2/3·k)           ║\n"
            "║   f  = 0  (equilibrium at inlet)                ║\n"
            "║   nut = %5  m²/s                                ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ RSTM (LRR / SSG) ════════════════════════════╗\n"
            "║ Reynolds stresses: R_ij = 2/3·k·δ_ij at inlet   ║\n"
            "║   R_xx=R_yy=R_zz = %6  m²/s²                    ║\n"
            "║   R_xy=R_xz=R_yz = 0                            ║\n"
            "║   epsilon = %3  m²/s³                           ║\n"
            "║   (use omega for SSG/LRR-ω variant)              ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ LES ═════════════════════════════════════════╗\n"
            "║ Smagorinsky: nuSgs ≈ %7  m²/s                   ║\n"
            "║ WALE: nuSgs similar order of magnitude           ║\n"
            "║ kEqn: kSgs ≈ %8  m²/s² (estimate)              ║\n"
            "║ dynamicKEqn / dynamicLagrangian: auto-calibrated ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ DES / DDES ══════════════════════════════════╗\n"
            "║ Same inlet as parent RANS model:                 ║\n"
            "║ SA-DES/DDES → use SA nuTilda above               ║\n"
            "║ kOmegaSSTDES/DDES → use k/ω above                ║\n"
            "║ CDES switching is internal (no extra BC needed)   ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "╔══ Transition Models ═══════════════════════════╗\n"
            "║ kOmegaSSTLM / gammaReTheta:                      ║\n"
            "║   k, ω — same as SST above                       ║\n"
            "║   gammaInt  = 1  (fully turbulent inlet)         ║\n"
            "║   ReThetat  = f(Tu, λ_θ) empirical correlation   ║\n"
            "╚═════════════════════════════════════════════════╝\n\n"
            "=== All 0/ field file templates ===\n"
            "// 0/k\ninternalField   uniform %2;\n\n"
            "// 0/epsilon\ninternalField   uniform %3;\n\n"
            "// 0/omega\ninternalField   uniform %9;\n\n"
            "// 0/nut\ninternalField   uniform %5;\n\n"
            "// 0/nuTilda\ninternalField   uniform %1;\n\n"
            "// 0/v2  (v2f only)\ninternalField   uniform %4;\n\n"
            "// 0/R  symmTensor (RSTM only)\n"
            "internalField   uniform (%6 0 0 %6 0 %6);\n\n"
            "// 0/nuSgs  (LES only, if modelling SGS viscosity)\n"
            "internalField   uniform %7;\n"
            ).arg(nuTilda,0,'g',4).arg(k,0,'g',4).arg(epsilon,0,'g',4)
             .arg(v2_val,0,'g',4).arg(nut,0,'g',4).arg(R_ii,0,'g',4)
             .arg(nuSgs_smag,0,'g',4).arg(kSgs,0,'g',4).arg(omega,0,'g',4);
    };

    result->setPlainText(calc());
    connect(U, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { result->setPlainText(calc()); });
    connect(I_turb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { result->setPlainText(calc()); });
    connect(L_char, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { result->setPlainText(calc()); });
    connect(nu, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=]() { result->setPlainText(calc()); });

    // ── Theory button ──
    auto *theoryBtn = new QPushButton("📖 Show Derivation & Physics...");
    theoryBtn->setStyleSheet("QPushButton { text-align:left; border:1px solid #C0E0C0; "
                             "background:#F5FFF5; color:#388E3C; font-size:10px; "
                             "font-weight:bold; padding:3px 8px; border-radius:3px; }"
                             "QPushButton:hover { background:#E0FFE0; }");
    l->addWidget(theoryBtn);

    connect(theoryBtn, &QPushButton::clicked, []() {
        auto *td = new QDialog(nullptr);
        td->setWindowTitle("Turbulence Inlet Parameters — Derivation & Physics");
        td->resize(700, 550);
        td->setMinimumSize(500, 400);
        td->setAttribute(Qt::WA_DeleteOnClose);
        auto *tl = new QVBoxLayout(td);
        tl->setContentsMargins(0, 0, 0, 0);
        auto *te = new QTextEdit();
        te->setReadOnly(true);
        te->setFont(QFont("Segoe UI", 10));
        te->setStyleSheet("QTextEdit { background:#FFF; border:1px solid #E0E0E0; "
                          "border-radius:4px; padding:12px; color:#333; }");
        te->setHtml(
            "<h2 style='color:#388E3C;'>Turbulence Inlet Parameters — y+ → 0/ fields</h2>"
            "<p><i>Standard turbulence model boundary conditions for RANS CFD</i></p>"

            "<h3 style='color:#555;'>Step 1 — Turbulence Intensity</h3>"
            "<p><b>I = u' / U</b></p>"
            "<p>Defined as the ratio of RMS velocity fluctuation to mean velocity. "
            "Empirically estimated from flow type:</p>"
            "<table border='1' cellpadding='3' cellspacing='0' style='border-collapse:collapse; font-size:9pt;'>"
            "<tr style='background:#E0FFE0;'><th>Flow Type</th><th>I (%)</th></tr>"
            "<tr><td>Low-turbulence wind tunnel</td><td>< 1%</td></tr>"
            "<tr><td>External flow (clean inlet)</td><td>1–5%</td></tr>"
            "<tr><td>Pipe flow (fully developed)</td><td>5–10%</td></tr>"
            "<tr><td>Complex geometry / wakes</td><td>10–20%</td></tr>"
            "<tr><td>High-intensity (combustion, cyclone)</td><td>> 20%</td></tr>"
            "</table>"

            "<h3 style='color:#555;'>Step 2 — Turbulent Kinetic Energy k</h3>"
            "<p><b>k = 3/2 · (U·I)² = 3/2 · u'²</b></p>"
            "<p><b>Derivation:</b> By definition, <i>k ≡ ½(u'² + v'² + w'²)</i>. "
            "Assuming isotropic turbulence (<i>u'² ≈ v'² ≈ w'²</i>): "
            "<i>k = ½ × 3u'² = 1.5 × u'² = 1.5 × (U·I)²</i>.</p>"
            "<p>Units: m²/s². Typical values: 0.01 (low-turbulence) to 1.0 (high-turbulence).</p>"

            "<h3 style='color:#555;'>Step 3 — Turbulence Length Scale L_t</h3>"
            "<p><b>L_t = 0.07 · L_char</b></p>"
            "<p>Empirical relation for internal flows (pipe/channel). "
            "L_char is hydraulic diameter for internal flow, or chord length for external flow. "
            "For external aerodynamics, use: L_t ≈ boundary layer thickness δ_99.</p>"
            "<p>Alternative: L_t = C_mu^(3/4) · k^(3/2) / epsilon (if epsilon is known).</p>"

            "<h3 style='color:#555;'>Step 4 — Dissipation Rate ε (kEpsilon)</h3>"
            "<p><b>ε = C_μ^(3/4) · k^(3/2) / L_t</b></p>"
            "<p>From Kolmogorov's energy cascade: the large-eddy turnover time is k/ε. "
            "The length scale L_t relates k and ε: L_t ∝ k^(3/2)/ε. "
            "The constant C_μ^(3/4) ≈ 0.164 is fixed by the standard k-ε model calibration.</p>"
            "<p>C_μ = 0.09 was determined by Launder & Spalding (1974) to satisfy "
            "the log-law in equilibrium boundary layers: u_τ² = C_μ^(1/2) · k.</p>"
            "<p>Units: m²/s³.</p>"

            "<h3 style='color:#555;'>Step 5 — Specific Dissipation ω (kOmega/kOmegaSST)</h3>"
            "<p><b>ω = √k / (C_μ^(1/4) · L_t)</b></p>"
            "<p>By definition: ω ≡ ε / (C_μ · k). Substituting ε from Step 4: "
            "ω = C_μ^(3/4)·k^(3/2)/L_t / (C_μ·k) = k^(1/2) / (C_μ^(1/4)·L_t).</p>"
            "<p>Physically, ω represents the rate of dissipation per unit turbulent kinetic energy. "
            "It is the inverse of the turbulent time scale. Units: 1/s.</p>"

            "<h3 style='color:#555;'>Step 6 — Eddy Viscosity ν_t</h3>"
            "<p><b>ν_t = C_μ · k² / ε</b></p>"
            "<p>From dimensional analysis: ν_t ∝ k²/ε (Prandtl-Kolmogorov relation). "
            "C_μ ≈ 0.09 ensures consistency with the log-law: ν_t = u_τ·κ·y in the log region.</p>"
            "<p>Units: m²/s (same as kinematic viscosity). Typical ν_t/ν = 10–1000.</p>"

            "<h3 style='color:#555;'>Step 7 — Spalart-Allmaras ν̃</h3>"
            "<p><b>ν̃ ≈ 3~5 × ν</b></p>"
            "<p>SA model uses a transport equation for the modified eddy viscosity ν̃. "
            "For fully turbulent inlet: ν̃ = 3~5 × ν_laminar is a common engineering estimate "
            "(Spalart & Allmaras, 1992). More precise: ν̃ = √(1.5) · U · I · L_t.</p>"

            "<hr>"
            "<h3 style='color:#555;'>Summary Table</h3>"
            "<table border='1' cellpadding='4' cellspacing='0' style='border-collapse:collapse; font-size:9pt;'>"
            "<tr style='background:#E0FFE0;'>"
            "<th>Field</th><th>Model</th><th>Formula</th><th>Dependency</th></tr>"
            "<tr><td>k</td><td>All RANS</td><td>3/2·(U·I)²</td><td>U, I</td></tr>"
            "<tr><td>ε</td><td>kEpsilon</td><td>C_μ^(3/4)·k^(3/2)/L_t</td><td>k, L_t, C_μ</td></tr>"
            "<tr><td>ω</td><td>kOmega/SST</td><td>√k/(C_μ^(1/4)·L_t)</td><td>k, L_t, C_μ</td></tr>"
            "<tr><td>ν_t</td><td>kEpsilon/kOmega</td><td>C_μ·k²/ε</td><td>k, ε, C_μ</td></tr>"
            "<tr><td>ν̃</td><td>SA</td><td>3~5·ν</td><td>ν</td></tr>"
            "</table>"

            "<hr>"
            "<p style='color:#888; font-size:9pt;'><b>References:</b><br>"
            "Launder, B.E. & Spalding, D.B. (1974). <i>Computer Methods in Applied Mechanics "
            "and Engineering</i>, 3(2), 269–289.<br>"
            "Wilcox, D.C. (2006). <i>Turbulence Modeling for CFD</i>, 3rd ed., DCW Industries.<br>"
            "Spalart, P.R. & Allmaras, S.R. (1992). AIAA Paper 92-0439.<br>"
            "Menter, F.R. (1994). <i>AIAA Journal</i>, 32(8), 1598–1605.</p>"
        );
        tl->addWidget(te, 1);
        auto *closeBtn = new QPushButton("Close");
        closeBtn->setStyleSheet("QPushButton { padding:6px 24px; }");
        tl->addWidget(closeBtn);
        connect(closeBtn, &QPushButton::clicked, td, &QDialog::accept);
        td->show();
    });

    auto *btns = new QHBoxLayout();
    btns->addStretch();
    auto *close = new QPushButton("Close");
    auto *insertK = new QPushButton("Insert k");
    auto *insertEps = new QPushButton("Insert epsilon");
    auto *insertOmega = new QPushButton("Insert omega");
    insertK->setStyleSheet("QPushButton { background:#388E3C; color:white; padding:4px 10px; "
                           "border:none; border-radius:3px; font-weight:bold; font-size:10px; }");
    insertEps->setStyleSheet(insertK->styleSheet());
    insertOmega->setStyleSheet(insertK->styleSheet());
    btns->addWidget(insertK); btns->addWidget(insertEps);
    btns->addWidget(insertOmega); btns->addWidget(close);
    l->addLayout(btns);

    connect(close, &QPushButton::clicked, dlg, &QDialog::close);
    connect(insertK, &QPushButton::clicked, dlg, [this, result, dlg]() {
        QString txt = result->toPlainText();
        int idx = txt.indexOf("internalField   uniform");
        if (m_editor && idx >= 0) {
            m_editor->textCursor().insertText(txt.mid(idx, txt.indexOf('\n', idx + 30) - idx + 1));
        }
    });
    connect(insertEps, &QPushButton::clicked, dlg, [this, result, dlg]() {
        QString txt = result->toPlainText();
        int idx = txt.indexOf("// 0/epsilon");
        if (m_editor && idx >= 0) {
            int s = txt.indexOf("uniform", idx);
            m_editor->textCursor().insertText(txt.mid(s, txt.indexOf('\n', s) - s + 1));
        }
    });
    connect(insertOmega, &QPushButton::clicked, dlg, [this, result, dlg]() {
        QString txt = result->toPlainText();
        int idx = txt.indexOf("// 0/omega");
        if (m_editor && idx >= 0) {
            int s = txt.indexOf("uniform", idx);
            m_editor->textCursor().insertText(txt.mid(s, txt.indexOf('\n', s) - s + 1));
        }
    });

    dlg->show();
}
