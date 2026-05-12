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
