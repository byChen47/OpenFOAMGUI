#include "bcpanel.h"
#include "codeeditor.h"
#include "ofparser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QRegularExpression>
#include <QFileInfo>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QTimer>
#include <QFont>
#include <QMenu>
#include <QFrame>
#include <QHeaderView>
#include <QScrollBar>

BCPanel::BCPanel(QWidget *parent) : QWidget(parent) { setupUI(); }

// ════════════════════════════════════════════════════════════════════
// UI Construction
// ════════════════════════════════════════════════════════════════════

void BCPanel::setupUI()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── 1. HEADER ──
    auto *hdr = new QFrame();
    hdr->setObjectName("bcHeader");
    hdr->setStyleSheet("QFrame#bcHeader { background: #E8F0F8; border-bottom: 2px solid #0078D7; padding: 6px 10px; }");
    auto *hl = new QHBoxLayout(hdr);
    hl->setContentsMargins(8, 6, 8, 6);
    hl->setSpacing(8);

    m_fieldIconLabel = new QLabel();
    m_fieldIconLabel->setFixedSize(26, 26);
    m_fieldIconLabel->setAlignment(Qt::AlignCenter);
    m_fieldIconLabel->setStyleSheet("background: #0078D7; color: white; border-radius: 13px; font-weight: bold; font-size: 13px;");
    hl->addWidget(m_fieldIconLabel);

    m_fieldInfoLabel = new QLabel("No field file selected");
    m_fieldInfoLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #333;");
    m_fieldInfoLabel->setWordWrap(true);
    hl->addWidget(m_fieldInfoLabel, 1);
    root->addWidget(hdr);

    m_fieldPathLabel = new QLabel();
    m_fieldPathLabel->setStyleSheet("color: #888; font-size: 10px; padding: 2px 10px; background: #FAFAFA;");
    m_fieldPathLabel->setWordWrap(true);
    root->addWidget(m_fieldPathLabel);

    // ── 2. CATEGORY TABS ──
    m_tabBar = new QWidget();
    m_tabBar->setObjectName("tabBar");
    m_tabBar->setStyleSheet("QWidget#tabBar { background: #F5F5F5; border-bottom: 1px solid #DDD; }");
    auto *tbl = new QHBoxLayout(m_tabBar);
    tbl->setContentsMargins(4, 4, 4, 4);
    tbl->setSpacing(2);
    m_tabGroup = new QButtonGroup(this);
    m_tabGroup->setExclusive(true);
    buildCategoryTabs();
    root->addWidget(m_tabBar);

    // ── 3. SEARCH ──
    auto *sr = new QWidget();
    sr->setStyleSheet("background: #FAFAFA; padding: 3px 6px;");
    auto *srl = new QHBoxLayout(sr);
    srl->setContentsMargins(4, 2, 4, 2);
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Filter BC types...");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setStyleSheet("QLineEdit { padding: 4px 8px; border: 1px solid #CCC; border-radius: 3px; background: white; font-size: 12px; } QLineEdit:focus { border-color: #0078D7; }");
    srl->addWidget(m_searchEdit);
    m_typeCountLabel = new QLabel();
    m_typeCountLabel->setStyleSheet("color: #888; font-size: 11px; padding: 0 4px;");
    srl->addWidget(m_typeCountLabel);
    root->addWidget(sr);

    // ── 4. MAIN SPLIT: BC list || Detail ──
    auto *ms = new QSplitter(Qt::Horizontal);
    ms->setChildrenCollapsible(false);

    // 4a. BC type list
    auto *lp = new QWidget();
    auto *lpl = new QVBoxLayout(lp);
    lpl->setContentsMargins(4, 2, 2, 2);
    lpl->setSpacing(2);
    m_bcTypeList = new QListWidget();
    m_bcTypeList->setAlternatingRowColors(true);
    m_bcTypeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_bcTypeList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_bcTypeList->setStyleSheet(
        "QListWidget { font-size: 12px; border: 1px solid #DDD; border-radius: 3px; background: white; }"
        "QListWidget::item { padding: 4px 8px; border-bottom: 1px solid #F0F0F0; }"
        "QListWidget::item:selected { background: #0078D7; color: white; }"
        "QListWidget::item:hover:!selected { background: #E5F0FA; }");
    lpl->addWidget(m_bcTypeList);
    ms->addWidget(lp);

    // 4b. Detail panel
    auto *rp = new QWidget();
    auto *rpl = new QVBoxLayout(rp);
    rpl->setContentsMargins(6, 4, 6, 4);
    rpl->setSpacing(4);

    // BC name + description
    m_bcNameLabel = new QLabel("Select a BC type from the list");
    m_bcNameLabel->setStyleSheet("font-weight: bold; font-size: 15px; color: #0078D7; padding: 2px 0;");
    m_bcNameLabel->setWordWrap(true);
    rpl->addWidget(m_bcNameLabel);

    m_bcDescLabel = new QLabel();
    m_bcDescLabel->setWordWrap(true);
    m_bcDescLabel->setStyleSheet("color: #555; font-size: 11px; padding: 4px 6px; background: #FFFFF5; border: 1px solid #F5E6C8; border-radius: 3px;");
    rpl->addWidget(m_bcDescLabel);

    // Patch browser
    auto *phdr = new QLabel("Patches in file:");
    phdr->setStyleSheet("font-weight: bold; color: #555; font-size: 11px; margin-top: 4px;");
    rpl->addWidget(phdr);
    m_patchTree = new QTreeWidget();
    m_patchTree->setHeaderLabels({"Patch", "Current Type"});
    m_patchTree->setRootIsDecorated(false);
    m_patchTree->setAlternatingRowColors(true);
    m_patchTree->setMaximumHeight(100);
    m_patchTree->setStyleSheet(
        "QTreeWidget { font-size: 11px; border: 1px solid #DDD; border-radius: 3px; }"
        "QTreeWidget::item { padding: 2px 4px; }"
        "QTreeWidget::item:selected { background: #0078D7; color: white; }"
        "QHeaderView::section { background: #EEE; padding: 3px 6px; font-size: 10px; font-weight: bold; border: none; }");
    rpl->addWidget(m_patchTree);

    // ═══ RTM TABLE: Property | Description | Type | Required | Default ═══
    auto *rtmHdr = new QLabel("Properties (RTM)");
    rtmHdr->setStyleSheet("font-weight: bold; color: #333; font-size: 12px; margin-top: 6px;");
    rpl->addWidget(rtmHdr);

    m_rtmTable = new QTableWidget(0, 5);
    m_rtmTable->setHorizontalHeaderLabels({"Property", "Description", "Type", "Required", "Default"});
    m_rtmTable->setAlternatingRowColors(true);
    m_rtmTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_rtmTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    m_rtmTable->verticalHeader()->setVisible(false);
    m_rtmTable->setShowGrid(true);
    m_rtmTable->setStyleSheet(
        "QTableWidget { font-size: 11px; border: 1px solid #DDD; gridline-color: #EEE; }"
        "QTableWidget::item { padding: 2px 4px; }"
        "QHeaderView::section { background: #EEE; padding: 4px 6px; font-size: 10px; font-weight: bold; border: none; border-bottom: 2px solid #CCC; }"
        "QTableWidget QLineEdit { font-size: 11px; padding: 1px 4px; border: 1px solid #0078D7; border-radius: 2px; }");
    m_rtmTable->horizontalHeader()->setStretchLastSection(true);
    m_rtmTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_rtmTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_rtmTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_rtmTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_rtmTable->setMinimumHeight(120);
    rpl->addWidget(m_rtmTable, 2);

    // ═══ EXAMPLE / Preview ═══
    auto *exHdr = new QLabel("Usage Example");
    exHdr->setStyleSheet("font-weight: bold; color: #333; font-size: 12px; margin-top: 4px;");
    rpl->addWidget(exHdr);

    m_exampleEdit = new QTextEdit();
    m_exampleEdit->setReadOnly(true);
    m_exampleEdit->setFont(QFont("Consolas", 10));
    m_exampleEdit->setMaximumHeight(140);
    m_exampleEdit->setStyleSheet(
        "QTextEdit { background: #1E1E1E; color: #DCDCDC; border: 1px solid #333; border-radius: 3px; padding: 8px; }");
    rpl->addWidget(m_exampleEdit);

    // Action buttons
    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    m_previewBtn = new QPushButton("Refresh Preview");
    m_previewBtn->setStyleSheet(
        "QPushButton { padding: 5px 14px; border: 1px solid #CCC; border-radius: 3px; background: #F5F5F5; font-size: 12px; }"
        "QPushButton:hover { background: #E8E8E8; }");
    m_applyBtn = new QPushButton("Apply to Editor");
    m_applyBtn->setEnabled(false);
    m_applyBtn->setStyleSheet(
        "QPushButton { padding: 5px 18px; border: none; border-radius: 3px; background: #0078D7; color: white; font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background: #005A9E; }"
        "QPushButton:disabled { background: #CCC; color: #888; }");
    btnRow->addWidget(m_previewBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_applyBtn);
    rpl->addLayout(btnRow);

    ms->addWidget(rp);
    ms->setStretchFactor(0, 35);
    ms->setStretchFactor(1, 65);
    root->addWidget(ms, 1);

    // ── CONNECTIONS ──
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BCPanel::onSearchTextChanged);
    connect(m_bcTypeList, &QListWidget::currentItemChanged, this, &BCPanel::onBCTypeSelected);
    connect(m_bcTypeList, &QListWidget::itemDoubleClicked, this, &BCPanel::onBCTypeDoubleClicked);
    connect(m_bcTypeList, &QListWidget::customContextMenuRequested, this, &BCPanel::onBCListContextMenu);
    connect(m_patchTree, &QTreeWidget::itemClicked, this, &BCPanel::onPatchClicked);
    connect(m_previewBtn, &QPushButton::clicked, this, &BCPanel::onPreviewBC);
    connect(m_applyBtn, &QPushButton::clicked, this, &BCPanel::onApplyBC);
    connect(m_rtmTable, &QTableWidget::cellChanged, this, &BCPanel::onParamCellChanged);
#if QT_CONFIG(shortcut)
    connect(m_tabGroup, &QButtonGroup::idClicked, this, &BCPanel::onCategoryButtonClicked);
#endif
}

// ════════════════════════════════════════════════════════════════════
// Category tabs
// ════════════════════════════════════════════════════════════════════

void BCPanel::buildCategoryTabs()
{
    struct { int id; QString label; } cats[] = {
        {-1, "All"}, {0, "Basic"}, {1, "Wall"}, {2, "Inlet"},
        {3, "Outlet"}, {4, "Pressure"}, {5, "Mapped"}, {6, "Constraint"}, {7, "Coded"},
    };
    for (auto *btn : m_tabButtons) delete btn;
    m_tabButtons.clear();
    for (auto &c : cats) {
        auto *btn = new QToolButton();
        btn->setText(c.label);
        btn->setCheckable(true);
        btn->setAutoRaise(true);
        btn->setStyleSheet(
            "QToolButton { padding: 4px 12px; border: 1px solid transparent; border-radius: 10px; font-size: 11px; font-weight: bold; color: #555; }"
            "QToolButton:hover { background: #E5F0FA; color: #0078D7; }"
            "QToolButton:checked { background: #0078D7; color: white; }");
        m_tabGroup->addButton(btn, c.id);
        m_tabButtons[c.id] = btn;
        static_cast<QHBoxLayout*>(m_tabBar->layout())->addWidget(btn);
    }
    if (m_tabButtons.contains(-1)) m_tabButtons[-1]->setChecked(true);
}

void BCPanel::onCategoryButtonClicked(int catId)
{
    m_activeCategory = catId;
    if (catId == -1) { populateBCTypes(m_currentTypes); return; }
    BCCategory targetCat;
    switch (catId) {
    case 0: targetCat = BCCategory::Basic; break;
    case 1: targetCat = BCCategory::Wall; break;
    case 2: targetCat = BCCategory::Inlet; break;
    case 3: targetCat = BCCategory::Outlet; break;
    case 4: targetCat = BCCategory::Special; break;
    case 5: targetCat = BCCategory::Mapped; break;
    case 6: targetCat = BCCategory::Constraint; break;
    case 7: targetCat = BCCategory::Coded; break;
    default: return;
    }
    QString s = m_searchEdit->text().toLower();
    QVector<BCTypeInfo> f;
    for (auto &t : m_currentTypes) {
        if (t.category != targetCat) continue;
        if (!s.isEmpty() && !t.name.toLower().contains(s) && !t.description.toLower().contains(s)) continue;
        f.append(t);
    }
    populateBCTypes(f);
}

void BCPanel::onSearchTextChanged(const QString &text)
{
    if (m_activeCategory == -1) {
        QString s = text.toLower();
        QVector<BCTypeInfo> f;
        for (auto &t : m_currentTypes) {
            if (!s.isEmpty() && !t.name.toLower().contains(s) && !t.description.toLower().contains(s)) continue;
            f.append(t);
        }
        populateBCTypes(f);
    } else {
        onCategoryButtonClicked(m_activeCategory);
    }
}

// ════════════════════════════════════════════════════════════════════
// Load field file
// ════════════════════════════════════════════════════════════════════

void BCPanel::loadFieldFile(const QString &filePath, const QString &content)
{
    m_filePath = filePath;
    m_currentContent = content;
    auto hdr = OFParser::parseHeader(content);
    m_fieldClass = hdr.foamClass;
    m_fieldName  = hdr.object;
    QFileInfo fi(filePath);

    if (hdr.foamClass.contains("Vector")) {
        m_fieldIconLabel->setText("V");
        m_fieldIconLabel->setStyleSheet("background: #0078D7; color: white; border-radius: 13px; font-weight: bold; font-size: 12px;");
    } else if (hdr.foamClass.contains("Scalar")) {
        m_fieldIconLabel->setText("S");
        m_fieldIconLabel->setStyleSheet("background: #388E3C; color: white; border-radius: 13px; font-weight: bold; font-size: 12px;");
    } else {
        m_fieldIconLabel->setText("?");
        m_fieldIconLabel->setStyleSheet("background: #999; color: white; border-radius: 13px; font-weight: bold; font-size: 12px;");
    }

    FieldCategory cat = BCTypeDatabase::categoryFromClass(hdr.foamClass);
    m_fieldInfoLabel->setText(
        QString("<b>%1</b> &nbsp;—&nbsp; %2 &nbsp;|&nbsp; %3 &nbsp;|&nbsp; object: %4")
        .arg(fi.fileName(), hdr.foamClass, BCTypeDatabase::categoryString(cat), hdr.object));
    m_fieldPathLabel->setText(filePath);

    m_currentTypes = BCTypeDatabase::instance()->typesForField(hdr.foamClass, m_fieldName);
    if (m_tabButtons.contains(-1)) m_tabButtons[-1]->setChecked(true);
    m_activeCategory = -1;
    populateBCTypes(m_currentTypes);
    parsePatches(content);

    m_bcNameLabel->setText("Select a BC type from the list");
    m_bcDescLabel->clear();
    m_applyBtn->setEnabled(false);
    m_exampleEdit->clear();
    m_rtmTable->setRowCount(0);
}

void BCPanel::clear()
{
    m_fieldIconLabel->setText("?");
    m_fieldInfoLabel->setText("No field file selected");
    m_fieldPathLabel->clear();
    m_bcTypeList->clear();
    m_patchTree->clear();
    m_bcNameLabel->setText("Select a BC type from the list");
    m_bcDescLabel->clear();
    m_exampleEdit->clear();
    m_applyBtn->setEnabled(false);
    m_typeCountLabel->clear();
    m_rtmTable->setRowCount(0);
}

// ════════════════════════════════════════════════════════════════════
// BC type list
// ════════════════════════════════════════════════════════════════════

void BCPanel::populateBCTypes(const QVector<BCTypeInfo> &types)
{
    m_bcTypeList->clear();
    BCCategory lastCat = (BCCategory)(-1);
    for (auto &t : types) {
        if (t.category != lastCat) {
            lastCat = t.category;
            QString cn;
            switch (t.category) {
            case BCCategory::Basic: cn="── Basic ──"; break;
            case BCCategory::Wall: cn="── Wall ──"; break;
            case BCCategory::Inlet: cn="── Inlet ──"; break;
            case BCCategory::Outlet: cn="── Outlet ──"; break;
            case BCCategory::Mapped: cn="── Mapped / Coupled ──"; break;
            case BCCategory::Coded: cn="── Coded (User) ──"; break;
            case BCCategory::Special: cn="── Pressure / Special ──"; break;
            case BCCategory::Constraint: cn="── Constraint / Patch ──"; break;
            default: continue;
            }
            auto *hi = new QListWidgetItem(cn);
            hi->setFlags(Qt::NoItemFlags);
            hi->setForeground(QColor("#999"));
            QFont f = hi->font(); f.setBold(true); f.setPointSize(9); hi->setFont(f);
            m_bcTypeList->addItem(hi);
        }
        auto *item = new QListWidgetItem(t.name);
        item->setData(Qt::UserRole, t.name);
        item->setToolTip(t.description);
        QColor tag;
        switch (t.category) {
        case BCCategory::Basic: tag=QColor("#0078D7"); break;
        case BCCategory::Wall: tag=QColor("#D73A0F"); break;
        case BCCategory::Inlet: tag=QColor("#388E3C"); break;
        case BCCategory::Outlet: tag=QColor("#E67E22"); break;
        case BCCategory::Mapped: tag=QColor("#8E44AD"); break;
        case BCCategory::Coded: tag=QColor("#2C3E50"); break;
        case BCCategory::Special: tag=QColor("#16A085"); break;
        case BCCategory::Constraint: tag=QColor("#95A5A6"); break;
        default: break;
        }
        if (tag.isValid()) { QPixmap px(8,8); px.fill(tag); item->setIcon(QIcon(px)); }
        m_bcTypeList->addItem(item);
    }
    m_typeCountLabel->setText(QString("%1 types").arg(types.size()));
}

void BCPanel::onBCTypeSelected(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    m_selectedBC = BCTypeDatabase::instance()->typeInfo(item->data(Qt::UserRole).toString());
    showBCDetails(m_selectedBC);
}

void BCPanel::onBCTypeDoubleClicked(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    onBCTypeSelected(item);
    if (!m_selectedPatchName.isEmpty()) onApplyBC();
}

// ════════════════════════════════════════════════════════════════════
// RTM TABLE — matching official doc.openfoam.com format
// ════════════════════════════════════════════════════════════════════

void BCPanel::showBCDetails(const BCTypeInfo &info)
{
    m_bcNameLabel->setText(info.name);
    m_bcDescLabel->setText(info.description);
    m_applyBtn->setEnabled(true);

    // Disconnect signal while rebuilding table
    disconnect(m_rtmTable, &QTableWidget::cellChanged, this, &BCPanel::onParamCellChanged);

    m_rtmTable->setRowCount(0);

    // Helper: add a row to the RTM table
    auto addRow = [&](const QString &prop, const QString &desc,
                      const QString &type, const QString &req, const QString &defVal,
                      bool editable = false) {
        int r = m_rtmTable->rowCount();
        m_rtmTable->insertRow(r);

        auto *p0 = new QTableWidgetItem(prop);
        p0->setFont(QFont("Consolas", 10));
        p0->setForeground(QColor("#0000CC"));
        if (prop == "type")
            p0->setBackground(QColor("#F0F0FF"));
        p0->setFlags(p0->flags() & ~Qt::ItemIsEditable);
        m_rtmTable->setItem(r, 0, p0);

        auto *p1 = new QTableWidgetItem(desc);
        p1->setFlags(p1->flags() & ~Qt::ItemIsEditable);
        m_rtmTable->setItem(r, 1, p1);

        auto *p2 = new QTableWidgetItem(type);
        p2->setForeground(QColor("#888"));
        p2->setFlags(p2->flags() & ~Qt::ItemIsEditable);
        m_rtmTable->setItem(r, 2, p2);

        auto *p3 = new QTableWidgetItem(req);
        p3->setTextAlignment(Qt::AlignCenter);
        if (req.toLower() == "yes" || req.toLower() == "required")
            p3->setForeground(QColor("#D73A0F"));
        else
            p3->setForeground(QColor("#388E3C"));
        p3->setFlags(p3->flags() & ~Qt::ItemIsEditable);
        m_rtmTable->setItem(r, 3, p3);

        // Default value cell — this IS editable for the user
        auto *p4 = new QTableWidgetItem(defVal);
        p4->setFont(QFont("Consolas", 10));
        if (!editable) {
            p4->setForeground(QColor("#888"));
            p4->setFlags(p4->flags() & ~Qt::ItemIsEditable);
        } else {
            p4->setForeground(QColor("#333"));
            p4->setData(Qt::UserRole, prop);  // store the property name
        }
        m_rtmTable->setItem(r, 4, p4);
    };

    // Row 1: type (always required, not editable)
    addRow("type", QString("Type name: %1").arg(info.name), "word", "yes", info.name);

    // Required params (editable)
    for (auto &p : info.requiredParams)
        addRow(p.name, p.description, p.type, "yes", p.defaultValue, true);

    // Optional params (editable)
    for (auto &p : info.optionalParams)
        addRow(p.name, p.description, p.type, "no", p.defaultValue, true);

    // Resize
    m_rtmTable->resizeColumnsToContents();
    if (m_rtmTable->columnWidth(1) > 400)
        m_rtmTable->setColumnWidth(1, 400);

    connect(m_rtmTable, &QTableWidget::cellChanged, this, &BCPanel::onParamCellChanged);

    // Generate example
    onPreviewBC();
}

void BCPanel::onParamCellChanged(int /*row*/, int /*col*/)
{
    // Update the preview when user edits a default value
    onPreviewBC();
}

QString BCPanel::collectParamValues() const
{
    // Collect user-edited values from the RTM table
    QStringList params;
    for (int r = 0; r < m_rtmTable->rowCount(); ++r) {
        auto *item = m_rtmTable->item(r, 0);
        auto *defItem = m_rtmTable->item(r, 4);
        if (!item || !defItem) continue;
        QString prop = item->text();
        if (prop == "type") continue; // type is handled separately
        QString val = defItem->text();
        // If user edited it, use the new value
        params.append(QString("        %1      %2;").arg(prop, -15).arg(val));
    }
    return params.join("\n");
}

// ════════════════════════════════════════════════════════════════════
// Patches
// ════════════════════════════════════════════════════════════════════

void BCPanel::parsePatches(const QString &content)
{
    m_patchTree->clear();
    // Depth-based: find boundaryField { ... } matching braces
    int bfIdx = content.indexOf("boundaryField");
    if (bfIdx < 0) return;
    int braceIdx = content.indexOf('{', bfIdx);
    if (braceIdx < 0) return;
    int depth = 1, pos = braceIdx + 1;
    while (pos < content.size() && depth > 0) {
        if (content[pos] == '{') depth++;
        else if (content[pos] == '}') depth--;
        pos++;
    }
    if (depth != 0) return;
    QString bfBody = content.mid(braceIdx + 1, pos - braceIdx - 2);

    QRegularExpression pr(R"((\w+)\s*\{(.*?)\n\s*\})", QRegularExpression::DotMatchesEverythingOption);
    auto it = pr.globalMatch(bfBody);
    while (it.hasNext()) {
        auto pm = it.next();
        QString pn = pm.captured(1);
        QRegularExpression tr(R"(type\s+(\S+)\s*;)");
        auto tm = tr.match(pm.captured(2));
        QString tp = tm.hasMatch() ? tm.captured(1) : "?";
        auto *item = new QTreeWidgetItem(m_patchTree);
        item->setText(0, pn);
        item->setText(1, tp);
        item->setData(0, Qt::UserRole, pn);
        if (tp == "zeroGradient" || tp == "empty" || tp == "symmetry" || tp == "wedge")
            item->setForeground(1, QColor("#388E3C"));
        else if (tp.contains("fixedValue") || tp == "uniformFixedValue")
            item->setForeground(1, QColor("#0078D7"));
        else if (tp == "noSlip" || tp == "slip")
            item->setForeground(1, QColor("#D73A0F"));
        else if (tp.contains("Inlet") || tp.contains("FlowRate"))
            item->setForeground(1, QColor("#E67E22"));
        else if (tp == "inletOutlet" || tp == "outletInlet")
            item->setForeground(1, QColor("#8E44AD"));
        else if (tp == "calculated")
            item->setForeground(1, QColor("#95A5A6"));
    }
    if (m_patchTree->topLevelItemCount() > 0) {
        m_patchTree->setCurrentItem(m_patchTree->topLevelItem(0));
        m_selectedPatchName = m_patchTree->topLevelItem(0)->data(0, Qt::UserRole).toString();
    }
}

void BCPanel::onPatchClicked(QTreeWidgetItem *item, int /*col*/)
{
    if (!item) return;
    m_selectedPatchName = item->data(0, Qt::UserRole).toString();
    selectBCInList(item->text(1));
    if (item->text(1) == "?") suggestBCForPatch(m_selectedPatchName);
}

void BCPanel::selectBCInList(const QString &name)
{
    for (int i = 0; i < m_bcTypeList->count(); ++i) {
        auto *li = m_bcTypeList->item(i);
        if (!(li->flags() & Qt::ItemIsSelectable)) continue;
        if (li->data(Qt::UserRole).toString() == name) {
            m_bcTypeList->setCurrentItem(li);
            m_bcTypeList->scrollToItem(li, QAbstractItemView::PositionAtCenter);
            return;
        }
    }
}

void BCPanel::suggestBCForPatch(const QString &pn)
{
    QString fn = m_fieldName.toLower();
    QString pnl = pn.toLower();
    bool isIn = pnl.contains("inlet")||pnl.contains("in")||pnl.contains("entry");
    bool isOut= pnl.contains("outlet")||pnl.contains("out")||pnl.contains("exit");
    bool isW  = pnl.contains("wall")||pnl.contains("plate")||pnl.contains("bottom")||pnl.contains("top")||pnl.contains("side");
    bool isAt = pnl.contains("atmos")||pnl.contains("open");
    bool isSy = pnl.contains("sym")||pnl.contains("plane");
    QString sug;
    if (fn=="u"&&isW) sug="noSlip";
    if (fn=="u"&&isIn) sug="flowRateInletVelocity";
    if (fn=="u"&&isOut) sug="pressureInletOutletVelocity";
    if (fn=="u"&&isAt) sug="pressureInletOutletVelocity";
    if ((fn=="p"||fn=="p_rgh")&&isW) sug="fixedFluxPressure";
    if ((fn=="p"||fn=="p_rgh")&&isIn) sug="totalPressure";
    if ((fn=="p"||fn=="p_rgh")&&isOut) sug="zeroGradient";
    if ((fn=="p"||fn=="p_rgh")&&isAt) sug="totalPressure";
    if (fn.startsWith("alpha.")&&isW) sug="zeroGradient";
    if (fn.startsWith("alpha.")&&isOut) sug="inletOutlet";
    if (fn.startsWith("alpha.")&&isAt) sug="inletOutlet";
    if (fn=="k"&&isW) sug="kqRWallFunction";
    if (fn=="epsilon"&&isW) sug="epsilonWallFunction";
    if (fn=="omega"&&isW) sug="omegaWallFunction";
    if (fn=="nut"&&isW) sug="nutkWallFunction";
    if ((fn=="k"||fn=="epsilon"||fn=="omega")&&isIn) sug=(fn=="k")?"turbulentIntensityKineticEnergyInlet":"fixedValue";
    if ((fn=="k"||fn=="epsilon"||fn=="omega")&&isOut) sug="inletOutlet";
    if (fn=="t"&&isW) sug="zeroGradient";
    if (fn=="t"&&isOut) sug="inletOutlet";
    if (isSy) sug="symmetry";
    if (!sug.isEmpty()) selectBCInList(sug);
}

// ════════════════════════════════════════════════════════════════════
// Preview + Apply
// ════════════════════════════════════════════════════════════════════

void BCPanel::onPreviewBC()
{
    if (m_selectedBC.name.isEmpty()) return;

    QString patch = m_selectedPatchName.isEmpty() ? "<patchName>" : m_selectedPatchName;

    // Build example in official format
    QString ex;
    ex += QString("    %1\n    {\n").arg(patch);
    ex += QString("        type            %1;\n").arg(m_selectedBC.name);

    for (int r = 0; r < m_rtmTable->rowCount(); ++r) {
        auto *propItem = m_rtmTable->item(r, 0);
        auto *defItem  = m_rtmTable->item(r, 4);
        auto *reqItem  = m_rtmTable->item(r, 3);
        if (!propItem || !defItem) continue;
        QString prop = propItem->text();
        if (prop == "type") continue;
        QString val = defItem->text();
        bool required = reqItem && reqItem->text().toLower() == "yes";
        if (!required)
            ex += QString("        // %1      %2;  (optional)\n").arg(prop, -15).arg(val);
        else
            ex += QString("        %1      %2;\n").arg(prop, -15).arg(val);
    }

    ex += "    }\n";
    m_exampleEdit->setPlainText(ex);
}

void BCPanel::onApplyBC()
{
    if (m_selectedBC.name.isEmpty() || !m_editor) return;

    if (m_selectedPatchName.isEmpty() && m_patchTree->topLevelItemCount() > 0)
        m_selectedPatchName = m_patchTree->topLevelItem(0)->data(0, Qt::UserRole).toString();

    onPreviewBC();
    QString snip = m_exampleEdit->toPlainText();

    QTextCursor c = m_editor->textCursor();
    c.insertText(snip);
    m_editor->setTextCursor(c);
    QApplication::clipboard()->setText(snip);

    m_applyBtn->setText("✓ Applied!");
    QTimer::singleShot(1500, this, [this]() { m_applyBtn->setText("Apply to Editor"); });
}

void BCPanel::onBCListContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_bcTypeList->itemAt(pos);
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;

    // Select the item under cursor
    m_bcTypeList->setCurrentItem(item);
    onBCTypeSelected(item);

    QMenu menu;
    // 1) Value only (type name)
    QAction *valAct = menu.addAction(
        QString("Insert \"%1\" (type name only)").arg(m_selectedBC.name));
    connect(valAct, &QAction::triggered, [this]() {
        if (!m_editor) return;
        QTextCursor c = m_editor->textCursor();
        c.insertText(m_selectedBC.name);
        m_editor->setTextCursor(c);
    });

    menu.addSeparator();

    // 2) Full snippet
    QAction *insertAct = menu.addAction("Insert BC Snippet");
    insertAct->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(insertAct, &QAction::triggered, this, &BCPanel::onInsertToEditor);

    if (!m_selectedPatchName.isEmpty()) {
        QAction *insertPatchAct = menu.addAction(
            QString("Insert \"%1\" for patch \"%2\"").arg(m_selectedBC.name, m_selectedPatchName));
        connect(insertPatchAct, &QAction::triggered, this, &BCPanel::onInsertToEditor);
    }

    menu.exec(m_bcTypeList->mapToGlobal(pos));
}

void BCPanel::onInsertToEditor()
{
    if (m_selectedBC.name.isEmpty() || !m_editor) return;
    if (m_selectedPatchName.isEmpty() && m_patchTree->topLevelItemCount() > 0)
        m_selectedPatchName = m_patchTree->topLevelItem(0)->data(0, Qt::UserRole).toString();

    onPreviewBC();
    QString snip = m_exampleEdit->toPlainText();
    QTextCursor c = m_editor->textCursor();
    c.insertText(snip);
    m_editor->setTextCursor(c);
    QApplication::clipboard()->setText(snip);

    m_applyBtn->setText("✓ Inserted!");
    QTimer::singleShot(1500, this, [this]() { m_applyBtn->setText("Apply to Editor"); });
}
