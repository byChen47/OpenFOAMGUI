#include "mainwindow.h"
#include "casebrowser.h"
#include "codeeditor.h"
#include "ofparser.h"
#include "languagedetector.h"
#include "bcpanel.h"
#include "turbulencepanel.h"
#include "schemespanel.h"
#include "snappypanel.h"
#include "dictpanel.h"
#include "fileviewer.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QInputDialog>
#include <QFileDialog>
#include <QRegularExpression>
#include <QSet>
#include <QPainter>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>

#include <QApplication>
#include <QTabBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>

// ────────────────────────────────────────────────────────────────────
// Construction
// ────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("OpenFOAM GUI — CFD Case Manager");
    // Show icons in menus (disabled by default on Windows)
    menuBar()->setNativeMenuBar(false);
    setMinimumSize(1000, 650);

    createActions();
    createMenus();
    createToolBar();
    createStatusBar();
    createDockWidgets();
    setupConnections();
    loadSettings();

    // Central widget: tabbed editor
    m_tabWidget = new QTabWidget();
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #C0C0C0; }"
        "QTabBar::tab { padding: 6px 16px; min-width: 80px; }"
        "QTabBar::tab:selected { background: #FFFFFF; border-bottom: 2px solid #0078D7; }");
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onCloseTab);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);

    // Right-click context menu on tabs
    m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabWidget->tabBar(), &QTabBar::customContextMenuRequested,
            this, &MainWindow::onTabContextMenu);

    // Welcome screen — closable landing tab
    auto *welcomeLabel = new QLabel();
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setWordWrap(true);
    welcomeLabel->setStyleSheet("QLabel { color: #606060; font-size: 16px; padding: 40px; }");
    welcomeLabel->setText(
        "<h2>Welcome to OpenFOAM GUI</h2>"
        "<p>A CFD case manager for OpenFOAM v2206</p>"
        "<p>Create by chen at 2026.05.09</p>"
        "<br>"
        "<p><b>Case → Open Case</b> to open an OpenFOAM case directory</p>"
        "<p>or drag &amp; drop a case folder onto the browser panel.</p>");
    m_tabWidget->addTab(welcomeLabel, "Welcome");
}

MainWindow::~MainWindow()
{
    saveSettings();
}

// ────────────────────────────────────────────────────────────────────
// Actions
// ────────────────────────────────────────────────────────────────────

// Helper: create a clean custom icon with a colored background + letter
static QIcon makeIcon(const QColor &bg, const QString &letter, const QColor &fg = Qt::white)
{
    QPixmap pm(24, 24);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(bg);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(1, 1, 22, 22, 4, 4);
    painter.setPen(fg);
    QFont f("Segoe UI", 11, QFont::Bold);
    painter.setFont(f);
    painter.drawText(QRect(0, 0, 24, 24), Qt::AlignCenter, letter);
    painter.end();
    return QIcon(pm);
}

void MainWindow::createActions()
{
    QStyle *s = style();

    m_openCaseAction = new QAction(s->standardIcon(QStyle::SP_DirOpenIcon),
                                   "&Open Case...", this);
    m_openCaseAction->setShortcut(QKeySequence("Ctrl+O"));
    m_openCaseAction->setStatusTip("Open an OpenFOAM case directory");

    m_closeCaseAction = new QAction(s->standardIcon(QStyle::SP_DirClosedIcon),
                                    "&Close Case", this);
    m_closeCaseAction->setShortcut(QKeySequence("Ctrl+Shift+W"));
    m_closeCaseAction->setStatusTip("Close the selected case");

    m_saveAction = new QAction(s->standardIcon(QStyle::SP_DialogSaveButton),
                               "&Save", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("Save the current file");

    m_saveAllAction = new QAction("Save A&ll", this);
    m_saveAllAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    m_saveAllAction->setStatusTip("Save all open files");

    m_saveAsAction = new QAction("Save &As...", this);
    m_saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
    m_saveAsAction->setStatusTip("Save the current file with a new name");

    m_closeTabAction = new QAction("&Close Tab", this);
    m_closeTabAction->setShortcut(QKeySequence("Ctrl+W"));
    m_closeTabAction->setStatusTip("Close the current editor tab");

    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    m_exitAction->setStatusTip("Exit OpenFOAM GUI");

    m_undoAction = new QAction(s->standardIcon(QStyle::SP_ArrowBack),
                               "&Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = new QAction(s->standardIcon(QStyle::SP_ArrowForward),
                               "&Redo", this);
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_findAction = new QAction(s->standardIcon(QStyle::SP_FileDialogContentsView),
                               "&Find...", this);
    m_findAction->setShortcut(QKeySequence::Find);
    m_findAction->setStatusTip("Find text in the current file");

    m_commentAction = new QAction("&Comment / Uncomment", this);
    m_commentAction->setShortcut(QKeySequence("Ctrl+/"));
    m_commentAction->setStatusTip("Toggle line comments (// or # based on file language)");

    m_acCppAction = new QAction("C++ Auto Completion", this);
    m_acCppAction->setCheckable(true);
    m_acCppAction->setChecked(true);
    m_acCppAction->setStatusTip("Enable auto-completion for C++ keywords and STL");

    m_acPythonAction = new QAction("Python Auto Completion", this);
    m_acPythonAction->setCheckable(true);
    m_acPythonAction->setChecked(true);
    m_acPythonAction->setStatusTip("Enable auto-completion for Python keywords");

    m_acOFAction = new QAction("OpenFOAM Auto Completion", this);
    m_acOFAction->setCheckable(true);
    m_acOFAction->setChecked(true);
    m_acOFAction->setStatusTip("Enable auto-completion for OpenFOAM BCs and keywords");

    m_newFileAction = new QAction(s->standardIcon(QStyle::SP_FileIcon),
                                  "New &File", this);
    m_newFileAction->setShortcut(QKeySequence("Ctrl+N"));
    m_newFileAction->setStatusTip("Create a new file in the selected directory");

    m_newFolderAction = new QAction(s->standardIcon(QStyle::SP_DirIcon),
                                    "New F&older", this);
    m_newFolderAction->setShortcut(QKeySequence("Ctrl+Shift+N"));
    m_newFolderAction->setStatusTip("Create a new folder in the selected directory");

    m_deleteAction = new QAction(s->standardIcon(QStyle::SP_TrashIcon),
                                 "&Delete", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setStatusTip("Delete the selected file or folder");

    m_cleanTimeAction = new QAction(s->standardIcon(QStyle::SP_DialogResetButton),
                                    "Clean &Time Dirs", this);
    m_cleanTimeAction->setStatusTip("Delete all time directories except 0/ from all opened cases");

    m_paraviewAction = new QAction(makeIcon(QColor("#1A73E8"), "PV"),
                                    "&ParaView", this);
    m_paraviewAction->setStatusTip("Open the current case in ParaView for post-processing");

    m_paraviewConfigAction = new QAction("ParaView &Path...", this);
    m_paraviewConfigAction->setStatusTip("Configure the ParaView executable path");

    m_pythonConfigAction = new QAction("Python &Path...", this);
    m_pythonConfigAction->setStatusTip("Configure the Python executable path");

    m_cppAction = new QAction(makeIcon(QColor("#00599C"), "C++"),
                              "Run &C++", this);
    m_cppAction->setShortcut(QKeySequence("Ctrl+Shift+C"));
    m_cppAction->setStatusTip("Compile and run the current C++ file");

    m_cppConfigAction = new QAction("C++ Compiler &Path...", this);
    m_cppConfigAction->setStatusTip("Configure the C++ compiler path (g++)");

    m_refreshAction = new QAction(s->standardIcon(QStyle::SP_BrowserReload),
                                  "&Refresh Case", this);
    m_refreshAction->setShortcut(QKeySequence("F5"));
    m_refreshAction->setStatusTip("Refresh the case browser");

    m_bcPanelAction = new QAction(s->standardIcon(QStyle::SP_FileDialogDetailedView),
                                  "&Boundary Conditions", this);
    m_bcPanelAction->setShortcut(QKeySequence("Ctrl+B"));
    m_bcPanelAction->setStatusTip("Toggle the Boundary Conditions configuration panel");
    m_bcPanelAction->setCheckable(true);
    m_bcPanelAction->setChecked(true);

    m_pythonAction = new QAction(makeIcon(QColor("#3776AB"), "Py"),
                                 "Run &Python", this);
    m_pythonAction->setShortcut(QKeySequence("Ctrl+Shift+P"));
    m_pythonAction->setStatusTip("Run the current Python file with system Python");

    m_terminalAction = new QAction(makeIcon(QColor("#444444"), ">_"),
                                   "&Terminal", this);
    m_terminalAction->setShortcut(QKeySequence("Ctrl+`"));
    m_terminalAction->setStatusTip("Open system terminal in the current directory");

    m_syncBoundariesAction = new QAction(makeIcon(QColor("#D73A0F"), "⇄"),
                                         "Sync &Boundaries", this);
    m_syncBoundariesAction->setStatusTip("Sync blockMeshDict boundary names to all 0/ field files boundaryField");

    m_aboutAction = new QAction("&About OpenFOAM GUI", this);
    m_aboutAction->setStatusTip("About this application");

    for (int i = 0; i < MaxRecentCases; ++i) {
        m_recentCaseActions[i] = new QAction(this);
        m_recentCaseActions[i]->setVisible(false);
    }
}

// ────────────────────────────────────────────────────────────────────
// Menus
// ────────────────────────────────────────────────────────────────────

void MainWindow::createMenus()
{
    // File menu — file-level operations only
    m_fileMenu = menuBar()->addMenu("&File");
    m_recentCasesMenu = m_fileMenu->addMenu("&Recent Cases");
    for (int i = 0; i < MaxRecentCases; ++i)
        m_recentCasesMenu->addAction(m_recentCaseActions[i]);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);
    m_fileMenu->addAction(m_saveAllAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_newFileAction);
    m_fileMenu->addAction(m_newFolderAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_closeTabAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // Edit menu — text editing + utility operations
    m_editMenu = menuBar()->addMenu("&Edit");
    m_editMenu->addAction(m_undoAction);
    m_editMenu->addAction(m_redoAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_findAction);
    m_editMenu->addAction(m_commentAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_acCppAction);
    m_editMenu->addAction(m_acPythonAction);
    m_editMenu->addAction(m_acOFAction);

    // View menu
    m_viewMenu = menuBar()->addMenu("&View");
    // Toolbar visibility toggles
    auto addViewToggle = [&](const QString &name, QAction *target, bool shown) {
        auto *act = m_viewMenu->addAction(name);
        act->setCheckable(true);
        act->setChecked(shown);
        if (!shown) {
            // Not in toolbar yet — add/remove on toggle
            connect(act, &QAction::toggled, [this, target](bool on) {
                if (on) m_mainToolBar->addAction(target);
                else    m_mainToolBar->removeAction(target);
            });
        } else {
            // Already in toolbar — just show/hide
            connect(act, &QAction::toggled, [target](bool on) { target->setVisible(on); });
        }
    };
    addViewToggle("Show New File",         m_newFileAction, false);
    addViewToggle("Show New Folder",       m_newFolderAction, false);
    addViewToggle("Show BC Panel",         m_bcPanelAction, true);
    addViewToggle("Show Terminal",         m_terminalAction, true);
    addViewToggle("Show Run Python",       m_pythonAction, true);
    addViewToggle("Show Run C++",          m_cppAction, true);
    addViewToggle("Show Sync Boundaries",  m_syncBoundariesAction, true);
    addViewToggle("Show ParaView",         m_paraviewAction, true);
    m_viewMenu->addSeparator();
    QAction *resetLayout = m_viewMenu->addAction("Reset Default &Layout");
    connect(resetLayout, &QAction::triggered, [this]() {
        // Restore toolbar buttons to their default positions
        m_mainToolBar->clear();
        m_mainToolBar->addAction(m_openCaseAction);
        m_mainToolBar->addAction(m_closeCaseAction);
        m_mainToolBar->addSeparator();
        m_mainToolBar->addAction(m_saveAction);
        m_mainToolBar->addSeparator();
        m_mainToolBar->addAction(m_deleteAction);
        m_mainToolBar->addSeparator();
        m_mainToolBar->addAction(m_bcPanelAction);
        m_mainToolBar->addAction(m_terminalAction);
        m_mainToolBar->addAction(m_pythonAction);
        m_mainToolBar->addAction(m_cppAction);
        m_mainToolBar->addAction(m_syncBoundariesAction);
        m_mainToolBar->addAction(m_paraviewAction);
        // Re-apply View toggle state (if New File/Folder were checked, add them)
        statusBar()->showMessage("Layout reset to default.", 3000);
    });
    m_viewMenu->addSeparator();

    // Case menu — case-level operations (open, close, create, clean, sync, tools)
    m_caseMenu = menuBar()->addMenu("&Case");
    m_caseMenu->addAction(m_openCaseAction);
    m_caseMenu->addAction(m_closeCaseAction);
    m_caseMenu->addSeparator();
    m_caseMenu->addAction(m_newFileAction);
    m_caseMenu->addAction(m_newFolderAction);
    m_caseMenu->addAction(m_deleteAction);
    m_caseMenu->addAction(m_cleanTimeAction);
    m_caseMenu->addSeparator();
    m_caseMenu->addAction(m_pythonAction);
    m_caseMenu->addAction(m_cppAction);
    m_caseMenu->addAction(m_paraviewAction);
    m_caseMenu->addAction(m_paraviewConfigAction);
    m_caseMenu->addAction(m_pythonConfigAction);
    m_caseMenu->addAction(m_cppConfigAction);
    m_caseMenu->addSeparator();
    m_caseMenu->addAction(m_refreshAction);

    // Help menu
    m_helpMenu = menuBar()->addMenu("&Help");
    m_helpMenu->addAction(m_aboutAction);

    updateRecentCasesMenu();
}

// ────────────────────────────────────────────────────────────────────
// Toolbar
// ────────────────────────────────────────────────────────────────────

void MainWindow::createToolBar()
{
    m_mainToolBar = addToolBar("Main Toolbar");
    m_mainToolBar->setMovable(true);
    m_mainToolBar->setFloatable(true);
    m_mainToolBar->setObjectName("MainToolBar");
    m_mainToolBar->setIconSize(QSize(20, 20));
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mainToolBar->setContextMenuPolicy(Qt::CustomContextMenu);

    // Give every action a unique objectName — required for drag-to-reorder
    m_openCaseAction->setObjectName("tbOpenCase");
    m_closeCaseAction->setObjectName("tbCloseCase");
    m_saveAction->setObjectName("tbSave");
    m_deleteAction->setObjectName("tbDelete");
    m_bcPanelAction->setObjectName("tbBCPanel");
    m_terminalAction->setObjectName("tbTerminal");
    m_pythonAction->setObjectName("tbRunPython");
    m_cppAction->setObjectName("tbRunCpp");
    m_syncBoundariesAction->setObjectName("tbSyncBoundaries");
    m_paraviewAction->setObjectName("tbParaView");
    m_newFileAction->setObjectName("tbNewFile");
    m_newFolderAction->setObjectName("tbNewFolder");

    m_mainToolBar->addAction(m_openCaseAction);
    m_mainToolBar->addAction(m_closeCaseAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_saveAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_deleteAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_bcPanelAction);
    m_mainToolBar->addAction(m_terminalAction);
    m_mainToolBar->addAction(m_pythonAction);
    m_mainToolBar->addAction(m_cppAction);
    m_mainToolBar->addAction(m_syncBoundariesAction);
    m_mainToolBar->addAction(m_paraviewAction);

    // Right-click toolbar → Customize for drag-to-reorder
    connect(m_mainToolBar, &QToolBar::customContextMenuRequested, [this](const QPoint &) {
        QMenu menu;
        QAction *customize = menu.addAction("Customize Toolbar...");
        connect(customize, &QAction::triggered, [this]() {
            // Open Qt's built-in toolbar editor for drag-to-reorder
            QMetaObject::invokeMethod(this, "toggleViewAction", Qt::DirectConnection);
        });
        menu.exec(QCursor::pos());
    });
}

// ────────────────────────────────────────────────────────────────────
// Status Bar
// ────────────────────────────────────────────────────────────────────

void MainWindow::createStatusBar()
{
    m_statusFileType  = new QLabel("Ready");
    m_statusCursorPos = new QLabel("");
    m_statusEncoding = new QLabel("UTF-8");

    m_statusFileType->setMinimumWidth(300);
    m_statusCursorPos->setMinimumWidth(160);
    m_statusCursorPos->setAlignment(Qt::AlignRight);
    m_statusEncoding->setMinimumWidth(80);
    m_statusEncoding->setAlignment(Qt::AlignCenter);

    statusBar()->addWidget(m_statusFileType, 1);
    statusBar()->addWidget(m_statusCursorPos);
    statusBar()->addPermanentWidget(m_statusEncoding);
}

// ────────────────────────────────────────────────────────────────────
// Dock Widgets
// ────────────────────────────────────────────────────────────────────

void MainWindow::createDockWidgets()
{
    m_caseBrowser = new CaseBrowser();

    m_caseBrowserDock = new QDockWidget("Case Browser", this);
    m_caseBrowserDock->setWidget(m_caseBrowser);
    m_caseBrowserDock->setFeatures(QDockWidget::DockWidgetMovable
                                   | QDockWidget::DockWidgetFloatable);
    m_caseBrowserDock->setMinimumWidth(260);
    addDockWidget(Qt::LeftDockWidgetArea, m_caseBrowserDock);

    // Right-side panel: BC Panel + Turbulence Panel + Schemes Panel (stacked)
    m_bcPanel = new BCPanel();
    m_turbulencePanel = new TurbulencePanel();
    m_schemesPanel = new SchemesPanel();
    m_rightPanelStack = new QStackedWidget();
    m_rightPanelStack->addWidget(m_bcPanel);          // index 0
    m_rightPanelStack->addWidget(m_turbulencePanel);  // index 1
    m_rightPanelStack->addWidget(m_schemesPanel);     // index 2
    m_snappyPanel = new SnappyPanel();
    m_rightPanelStack->addWidget(m_snappyPanel);       // index 3
    m_dictPanel = new DictPanel();
    m_rightPanelStack->addWidget(m_dictPanel);          // index 4
    m_rightPanelStack->setCurrentIndex(0);

    m_bcPanelDock = new QDockWidget("Boundary Conditions", this);
    m_bcPanelDock->setWidget(m_rightPanelStack);
    m_bcPanelDock->setFeatures(QDockWidget::DockWidgetMovable
                               | QDockWidget::DockWidgetFloatable);
    m_bcPanelDock->setMinimumWidth(320);
    addDockWidget(Qt::RightDockWidgetArea, m_bcPanelDock);

    m_viewMenu->addAction(m_caseBrowserDock->toggleViewAction());
    m_viewMenu->addAction(m_bcPanelDock->toggleViewAction());

    // Keep toolbar button in sync with dock close/open
    connect(m_bcPanelDock, &QDockWidget::visibilityChanged, [this](bool visible) {
        m_bcPanelAction->setChecked(visible);
    });
}

// ────────────────────────────────────────────────────────────────────
// Connections
// ────────────────────────────────────────────────────────────────────

void MainWindow::setupConnections()
{
    connect(m_openCaseAction, &QAction::triggered,
            this, &MainWindow::onOpenCase);
    connect(m_closeCaseAction, &QAction::triggered,
            this, &MainWindow::onCloseCase);
    connect(m_saveAction, &QAction::triggered,
            this, &MainWindow::onSaveFile);
    connect(m_saveAllAction, &QAction::triggered,
            this, &MainWindow::onSaveAllFiles);
    connect(m_saveAsAction, &QAction::triggered,
            this, &MainWindow::onSaveFileAs);
    connect(m_closeTabAction, &QAction::triggered, [this]() {
        int idx = m_tabWidget->currentIndex();
        if (idx >= 0)
            onCloseTab(idx);
    });
    connect(m_exitAction, &QAction::triggered, this, &QMainWindow::close);

    connect(m_undoAction, &QAction::triggered, [this]() {
        if (auto *e = currentEditor()) e->undo();
        else statusBar()->showMessage("No file is open to undo.", 3000);
    });
    connect(m_redoAction, &QAction::triggered, [this]() {
        if (auto *e = currentEditor()) e->redo();
        else statusBar()->showMessage("No file is open to redo.", 3000);
    });
    connect(m_findAction, &QAction::triggered, this, &MainWindow::onFindText);
    connect(m_commentAction, &QAction::triggered, [this]() {
        if (auto *e = currentEditor()) e->toggleComment();
        else statusBar()->showMessage("No file is open to comment.", 3000);
    });
    connect(m_acCppAction, &QAction::toggled, [this](bool on) {
        for (int i = 0; i < m_tabWidget->count(); ++i)
            if (auto *e = qobject_cast<CodeEditor*>(m_tabWidget->widget(i)))
                e->setAutoCompleteCpp(on);
    });
    connect(m_acPythonAction, &QAction::toggled, [this](bool on) {
        for (int i = 0; i < m_tabWidget->count(); ++i)
            if (auto *e = qobject_cast<CodeEditor*>(m_tabWidget->widget(i)))
                e->setAutoCompletePython(on);
    });
    connect(m_acOFAction, &QAction::toggled, [this](bool on) {
        for (int i = 0; i < m_tabWidget->count(); ++i)
            if (auto *e = qobject_cast<CodeEditor*>(m_tabWidget->widget(i)))
                e->setAutoCompleteOpenFOAM(on);
    });
    connect(m_newFileAction, &QAction::triggered, this, &MainWindow::onNewFile);
    connect(m_newFolderAction, &QAction::triggered, this, &MainWindow::onNewFolder);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    connect(m_cleanTimeAction, &QAction::triggered, this, &MainWindow::onCleanTimeDirs);
    connect(m_syncBoundariesAction, &QAction::triggered, this, &MainWindow::onSyncBoundaries);
    connect(m_pythonAction, &QAction::triggered, this, &MainWindow::onRunPython);
    connect(m_cppAction, &QAction::triggered, this, &MainWindow::onRunCpp);
    connect(m_paraviewAction, &QAction::triggered, this, &MainWindow::onParaView);
    connect(m_paraviewConfigAction, &QAction::triggered, this, &MainWindow::onConfigureParaView);
    connect(m_pythonConfigAction, &QAction::triggered, this, &MainWindow::onConfigurePython);
    connect(m_cppConfigAction, &QAction::triggered, this, &MainWindow::onConfigureCpp);
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::onRefreshCase);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);

    // BC panel toggle: clicking the toolbar button shows/hides the dock
    connect(m_bcPanelAction, &QAction::toggled, [this](bool checked) {
        m_bcPanelDock->setVisible(checked);
    });

    // Terminal: launch system terminal
    connect(m_terminalAction, &QAction::triggered,
            this, &MainWindow::onOpenTerminal);

    connect(m_caseBrowser, &CaseBrowser::fileSelected,
            this, &MainWindow::onFileSelected);
    connect(m_caseBrowser, &CaseBrowser::caseOpened, [this](const QString &path) {
        addRecentCase(path);
        updateWindowTitle();
    });
    connect(m_caseBrowser, &CaseBrowser::caseClosed, [this](const QString &) {
        updateWindowTitle();
    });

    for (int i = 0; i < MaxRecentCases; ++i) {
        connect(m_recentCaseActions[i], &QAction::triggered,
                this, &MainWindow::onOpenRecentCase);
    }
}

// ────────────────────────────────────────────────────────────────────
// Slots — file / case operations
// ────────────────────────────────────────────────────────────────────

void MainWindow::onOpenCase()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Open OpenFOAM Case Directory", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty())
        return;

    // Validate: the directory (or a subdirectory) should contain
    // at least one of {0, 0.orig, constant, system}
    QDir caseDir(dir);
    auto entries = caseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    bool isValid = false;
    QStringList requiredDirs = {"constant", "system"};

    // Check top-level
    for (const auto &entry : entries) {
        if (entry == "constant" || entry == "system") {
            isValid = true;
            break;
        }
        bool isTime = false;
        entry.toDouble(&isTime);
        if (isTime || entry == "0" || entry.startsWith("0.")) {
            isValid = true;
            break;
        }
    }

    // Also check one level down (e.g., tutorials/.../damBreak/damBreak/)
    if (!isValid) {
        for (const auto &entry : entries) {
            QDir sub(caseDir.filePath(entry));
            auto subEntries = sub.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const auto &se : subEntries) {
                if (se == "constant" || se == "system") {
                    isValid = true;
                    break;
                }
                bool isTime = false;
                se.toDouble(&isTime);
                if (isTime || se == "0" || se.startsWith("0.")) {
                    isValid = true;
                    break;
                }
            }
            if (isValid) break;
        }
    }

    if (!isValid) {
        QMessageBox::warning(this, "Invalid Case",
                             "The selected directory does not appear to be a valid "
                             "OpenFOAM case.\n\nAn OpenFOAM case should contain "
                             "at least a \"constant/\" or \"system/\" directory.");
        return;
    }

    m_caseBrowser->openCase(dir);
    statusBar()->showMessage("Case opened: " + dir, 4000);
}

void MainWindow::onCloseCase()
{
    QStringList cases = m_caseBrowser->casePaths();
    if (cases.isEmpty()) return;

    // If only one case, close it directly
    if (cases.size() == 1) {
        m_caseBrowser->closeCase(cases.first());
        statusBar()->showMessage("Case closed", 3000);
        return;
    }

    // Multiple cases: find which case the selected tree item belongs to
    QTreeWidget *tree = m_caseBrowser->tree();
    if (!tree || !tree->currentItem()) {
        statusBar()->showMessage("No case selected to close.", 3000);
        return;
    }

    // Walk up from current item to the nearest caseroot
    QString selectedPath;
    auto *item = tree->currentItem();
    while (item) {
        QString t = item->data(0, Qt::UserRole + 1).toString();
        if (t == "caseroot") {
            selectedPath = item->data(0, Qt::UserRole).toString();
            break;
        }
        if (t == "subcase") {
            // Subcase: find the parent caseroot above it
            auto *parent = item->parent();
            while (parent) {
                if (parent->data(0, Qt::UserRole + 1).toString() == "caseroot") {
                    selectedPath = parent->data(0, Qt::UserRole).toString();
                    break;
                }
                parent = parent->parent();
            }
            break;
        }
        item = item->parent();
    }

    if (!selectedPath.isEmpty()) {
        m_caseBrowser->closeCase(selectedPath);
        statusBar()->showMessage("Case closed: " + selectedPath, 3000);
    } else {
        statusBar()->showMessage("Could not identify case to close.", 3000);
    }
}

void MainWindow::onOpenRecentCase()
{
    auto *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;
    QString path = action->data().toString();
    if (!path.isEmpty() && QDir(path).exists()) {
        m_caseBrowser->openCase(path);
    } else {
        QMessageBox::information(this, "Case Not Found",
                                 "The case directory no longer exists:\n" + path);
        m_recentCases.removeAll(path);
        updateRecentCasesMenu();
    }
}

void MainWindow::onFileSelected(const QString &filePath)
{
    openFileInTab(filePath);
}

bool MainWindow::openFileInTab(const QString &filePath)
{
    QFileInfo fi(filePath);
    QString ext = fi.suffix().toLower();

    // ── Image files — native display with zoom ──
    if (FileViewer::isImageFile(ext)) {
        for (int i = 0; i < m_tabWidget->count(); ++i) {
            if (m_tabWidget->tabToolTip(i) == filePath) {
                m_tabWidget->setCurrentIndex(i);
                return true;
            }
        }
        bool loaded;
        QString error;
        QWidget *viewer = FileViewer::createImageViewer(filePath, loaded, error);
        if (!viewer) {
            statusBar()->showMessage(error, 6000);
            return false;
        }
        int idx = m_tabWidget->addTab(viewer, fi.fileName());
        m_tabWidget->setCurrentWidget(viewer);
        m_tabWidget->setTabToolTip(idx, filePath);
        statusBar()->showMessage("Opened: " + fi.fileName(), 3000);
        return true;
    }

    // ── PDF / Office — system default app ──
    if (FileViewer::isOfficeFile(ext)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        statusBar()->showMessage(
            QString("Opened %1 externally: %2")
                .arg(FileViewer::officeTypeName(ext), fi.fileName()), 4000);
        return true;
    }

    // ════════════════════════════════════════════════════════
    //  Text files — use the code editor
    // ════════════════════════════════════════════════════════

    // Check if already open
    CodeEditor *existing = editorForFile(filePath);
    if (existing) {
        int idx = m_tabWidget->indexOf(existing);
        if (idx >= 0) {
            m_tabWidget->setCurrentIndex(idx);
            return true;
        }
    }

    // Read file
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Cannot Open File",
                             "Failed to open:\n" + filePath + "\n\n"
                             + file.errorString());
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Create editor
    auto *editor = new CodeEditor();
    editor->setPlainText(content);
    editor->setFileName(filePath);
    editor->document()->setModified(false);

    // Auto-detect language and apply syntax highlighting
    FileLanguage lang = LanguageDetector::detect(filePath, content);
    editor->setLanguage(lang);

    // Tab title: file name only
    m_tabWidget->addTab(editor, fi.fileName());
    m_tabWidget->setCurrentWidget(editor);
    m_tabWidget->setTabToolTip(m_tabWidget->currentIndex(), filePath);

    // Connect editor modification signal
    connect(editor, &CodeEditor::modificationChanged, [this, editor](bool changed) {
        int idx = m_tabWidget->indexOf(editor);
        if (idx >= 0) {
            QString title = m_tabWidget->tabText(idx);
            if (changed && !title.endsWith(" *"))
                m_tabWidget->setTabText(idx, title + " *");
            else if (!changed && title.endsWith(" *"))
                m_tabWidget->setTabText(idx, title.chopped(2));
        }
    });

    connect(editor, &CodeEditor::cursorPositionChanged, [this, editor]() {
        if (editor == currentEditor())
            updateStatusBarForEditor(editor);
    });

    updateStatusBarForEditor(editor);
    statusBar()->showMessage("Opened: " + fi.fileName(), 3000);

    // Load into BC panel or turbulence panel based on file type
    m_bcPanel->setEditor(editor);
    m_turbulencePanel->setEditor(editor);
    m_schemesPanel->setEditor(editor);
    m_snappyPanel->setEditor(editor);
    m_dictPanel->setEditor(editor);

    QFileInfo fi2(filePath);
    if (fi2.fileName() == "turbulenceProperties" && lang == FileLanguage::OpenFOAM) {
        m_turbulencePanel->loadTurbulenceFile(filePath, content);
        m_rightPanelStack->setCurrentIndex(1);
        m_bcPanelDock->setWindowTitle("Turbulence Model");
    } else if ((fi2.fileName() == "fvSchemes" || fi2.fileName() == "fvSolution")
               && lang == FileLanguage::OpenFOAM) {
        m_schemesPanel->loadFile(filePath, content);
        m_rightPanelStack->setCurrentIndex(2);
        m_bcPanelDock->setWindowTitle("Discretisation & Solvers");
    } else if (fi2.fileName() == "snappyHexMeshDict"
               && lang == FileLanguage::OpenFOAM) {
        m_snappyPanel->loadFile(filePath, content);
        m_rightPanelStack->setCurrentIndex(3);
        m_bcPanelDock->setWindowTitle("snappyHexMesh");
    } else if ((fi2.fileName() == "blockMeshDict" || fi2.fileName() == "topoSetDict"
                || fi2.fileName() == "dynamicMeshDict" || fi2.fileName() == "controlDict"
                || fi2.fileName() == "decomposeParDict" || fi2.fileName() == "refineMeshDict"
                || fi2.fileName() == "transportProperties"
                || fi2.fileName() == "thermophysicalProperties"
                || fi2.fileName() == "radiationProperties"
                || fi2.fileName() == "combustionProperties"
                || fi2.fileName() == "setFieldsDict" || fi2.fileName() == "sampleDict"
                || fi2.fileName() == "surfaceFeatureExtractDict"
                || fi2.fileName() == "mapFieldsDict" || fi2.fileName() == "createPatchDict"
                || fi2.fileName() == "extrudeMeshDict" || fi2.fileName() == "forces"
                || fi2.fileName() == "forceCoeffs" || fi2.fileName() == "fvConstraints"
                || fi2.fileName() == "mirrorMeshDict" || fi2.fileName() == "renumberMeshDict"
                || fi2.fileName() == "transformPointsDict"
                || fi2.fileName() == "waveProperties"
                || fi2.fileName() == "waveProperties.input")
               && lang == FileLanguage::OpenFOAM) {
        m_dictPanel->loadFile(filePath, content);
        m_rightPanelStack->setCurrentIndex(4);
        m_bcPanelDock->setWindowTitle(fi2.fileName());
    } else if (lang == FileLanguage::OpenFOAM) {
        m_bcPanel->loadFieldFile(filePath, content);
        m_rightPanelStack->setCurrentIndex(0);
        m_bcPanelDock->setWindowTitle("Boundary Conditions");
    } else {
        m_bcPanel->clear();
        m_turbulencePanel->clear();
        m_schemesPanel->clear();
        m_snappyPanel->clear();
        m_dictPanel->clear();
    }

    return true;
}

void MainWindow::onSaveFile()
{
    CodeEditor *editor = currentEditor();
    if (!editor) {
        statusBar()->showMessage("No file is open to save.", 3000);
        return;
    }

    if (editor->fileName().isEmpty()) {
        onSaveFileAs();
        return;
    }

    saveEditor(editor);
}

void MainWindow::onSaveAllFiles()
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (editor && editor->document()->isModified())
            saveEditor(editor);
    }
}

void MainWindow::onSaveFileAs()
{
    CodeEditor *editor = currentEditor();
    if (!editor) {
        statusBar()->showMessage("No file is open to save.", 3000);
        return;
    }

    QFileInfo fi(editor->fileName());
    QStringList cases = m_caseBrowser->casePaths();
    QString suggestedDir = fi.exists() ? fi.absolutePath()
                                       : (cases.isEmpty() ? QDir::currentPath() : cases.first());

    QString filePath = QFileDialog::getSaveFileName(
        this, "Save OpenFOAM File As", suggestedDir + "/" + fi.fileName(),
        "OpenFOAM Files (*);;All Files (*.*)");

    if (filePath.isEmpty())
        return;

    editor->setFileName(filePath);
    saveEditor(editor);

    // Update tab title
    int idx = m_tabWidget->currentIndex();
    if (idx >= 0) {
        QFileInfo nfi(filePath);
        m_tabWidget->setTabText(idx, nfi.fileName());
        m_tabWidget->setTabToolTip(idx, filePath);
    }
}

bool MainWindow::saveEditor(CodeEditor *editor)
{
    if (!editor || editor->fileName().isEmpty())
        return false;

    QFile file(editor->fileName());
    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not write to:\n" + editor->fileName()
                             + "\n\n" + file.errorString());
        return false;
    }

    QTextStream out(&file);
    out << editor->toPlainText();
    file.close();

    editor->document()->setModified(false);
    statusBar()->showMessage("Saved: " + QFileInfo(editor->fileName()).fileName(), 3000);
    return true;
}

void MainWindow::onTabContextMenu(const QPoint &pos)
{
    int idx = m_tabWidget->tabBar()->tabAt(pos);
    if (idx < 0) return;

    // Don't show context menu on welcome tab
    if (qobject_cast<QLabel*>(m_tabWidget->widget(idx))) return;

    QMenu menu;

    QAction *closeCurrent = menu.addAction("Close Current Tab");
    closeCurrent->setShortcut(QKeySequence("Ctrl+W"));
    connect(closeCurrent, &QAction::triggered, [this, idx]() { onCloseTab(idx); });

    menu.addSeparator();

    QAction *closeOthers = menu.addAction("Close Other Tabs");
    connect(closeOthers, &QAction::triggered, [this, idx]() {
        // Close all non-welcome tabs except the current one
        QVector<int> toClose;
        for (int i = m_tabWidget->count() - 1; i >= 0; --i) {
            if (i != idx && !qobject_cast<QLabel*>(m_tabWidget->widget(i)))
                toClose.append(i);
        }
        for (int i : toClose)
            onCloseTab(i);
    });

    QAction *closeAll = menu.addAction("Close All Tabs");
    connect(closeAll, &QAction::triggered, [this]() {
        // Close all non-welcome tabs (editors, images, PDF cards, etc.)
        for (int i = m_tabWidget->count() - 1; i >= 0; --i) {
            if (!qobject_cast<QLabel*>(m_tabWidget->widget(i)))
                onCloseTab(i);
        }
    });

    menu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
}

void MainWindow::onCloseTab(int index)
{
    if (index < 0)
        return;

    auto *widget = m_tabWidget->widget(index);

    // Editor tab: check for unsaved changes
    auto *editor = qobject_cast<CodeEditor*>(widget);
    if (editor && editor->document()->isModified()) {
        QMessageBox::StandardButton ret =
            QMessageBox::warning(this, "Unsaved Changes",
                                 "The file has been modified.\n"
                                 "Do you want to save your changes?",
                                 QMessageBox::Save | QMessageBox::Discard
                                 | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            saveEditor(editor);
        else if (ret == QMessageBox::Cancel)
            return;
    }

    // Welcome tab (QLabel) or editor tab — close it
    m_tabWidget->removeTab(index);
    // Note: no auto-recreate — empty tab area is fine, user can use File → Open Case
}

void MainWindow::onTabChanged(int index)
{
    auto *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(index));
    updateStatusBarForEditor(editor);

    if (editor && !editor->fileName().isEmpty()) {
        m_bcPanel->setEditor(editor);
        m_turbulencePanel->setEditor(editor);
        m_schemesPanel->setEditor(editor);
        m_snappyPanel->setEditor(editor);
        m_dictPanel->setEditor(editor);

        QFileInfo fi(editor->fileName());
        if (fi.fileName() == "turbulenceProperties"
            && editor->language() == FileLanguage::OpenFOAM) {
            m_turbulencePanel->loadTurbulenceFile(editor->fileName(), editor->toPlainText());
            m_rightPanelStack->setCurrentIndex(1);
            m_bcPanelDock->setWindowTitle("Turbulence Model");
        } else if ((fi.fileName() == "fvSchemes" || fi.fileName() == "fvSolution")
                   && editor->language() == FileLanguage::OpenFOAM) {
            m_schemesPanel->loadFile(editor->fileName(), editor->toPlainText());
            m_rightPanelStack->setCurrentIndex(2);
            m_bcPanelDock->setWindowTitle("Discretisation & Solvers");
        } else if (fi.fileName() == "snappyHexMeshDict"
                   && editor->language() == FileLanguage::OpenFOAM) {
            m_snappyPanel->loadFile(editor->fileName(), editor->toPlainText());
            m_rightPanelStack->setCurrentIndex(3);
            m_bcPanelDock->setWindowTitle("snappyHexMesh");
        } else if ((fi.fileName() == "blockMeshDict" || fi.fileName() == "topoSetDict"
                    || fi.fileName() == "dynamicMeshDict" || fi.fileName() == "controlDict"
                    || fi.fileName() == "decomposeParDict" || fi.fileName() == "refineMeshDict"
                    || fi.fileName() == "transportProperties"
                    || fi.fileName() == "thermophysicalProperties"
                    || fi.fileName() == "radiationProperties"
                    || fi.fileName() == "combustionProperties"
                    || fi.fileName() == "setFieldsDict" || fi.fileName() == "sampleDict"
                    || fi.fileName() == "surfaceFeatureExtractDict"
                    || fi.fileName() == "mapFieldsDict" || fi.fileName() == "createPatchDict"
                    || fi.fileName() == "extrudeMeshDict" || fi.fileName() == "forces"
                    || fi.fileName() == "forceCoeffs" || fi.fileName() == "fvConstraints"
                    || fi.fileName() == "mirrorMeshDict" || fi.fileName() == "renumberMeshDict"
                    || fi.fileName() == "transformPointsDict")
                   && editor->language() == FileLanguage::OpenFOAM) {
            m_dictPanel->loadFile(editor->fileName(), editor->toPlainText());
            m_rightPanelStack->setCurrentIndex(4);
            m_bcPanelDock->setWindowTitle(fi.fileName());
        } else if (editor->language() == FileLanguage::OpenFOAM) {
            m_bcPanel->loadFieldFile(editor->fileName(), editor->toPlainText());
            m_rightPanelStack->setCurrentIndex(0);
            m_bcPanelDock->setWindowTitle("Boundary Conditions");
        } else {
            m_bcPanel->clear();
            m_turbulencePanel->clear();
            m_schemesPanel->clear();
        m_snappyPanel->clear();
        m_dictPanel->clear();
        }
    } else {
        m_bcPanel->clear();
        m_turbulencePanel->clear();
        m_schemesPanel->clear();
        m_snappyPanel->clear();
        m_dictPanel->clear();
    }
}

void MainWindow::onFindText()
{
    auto *editor = currentEditor();
    if (!editor) {
        statusBar()->showMessage("No file is open to search.", 3000);
        return;
    }

    // Use a proper find dialog
    bool ok;
    QString searchText = QInputDialog::getText(this, "Find",
        "Search for:", QLineEdit::Normal,
        editor->textCursor().selectedText(), &ok);

    if (!ok || searchText.isEmpty()) return;

    // Find from current cursor position
    QTextCursor cursor = editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.setPosition(cursor.selectionEnd());
        editor->setTextCursor(cursor);
    }

    bool found = editor->find(searchText);
    if (!found) {
        // Wrap around
        QTextCursor startCur = editor->textCursor();
        startCur.movePosition(QTextCursor::Start);
        editor->setTextCursor(startCur);
        found = editor->find(searchText);
    }

    if (!found) {
        statusBar()->showMessage("Text not found: \"" + searchText + "\"", 4000);
    } else {
        statusBar()->showMessage("Found: \"" + searchText + "\"", 3000);
        // Press F3 to find next
    }
}

void MainWindow::onNewFile()
{
    m_caseBrowser->newFile();
}

void MainWindow::onNewFolder()
{
    m_caseBrowser->newFolder();
}

void MainWindow::onDeleteSelected()
{
    m_caseBrowser->deleteSelected();
}

void MainWindow::onCleanTimeDirs()
{
    QStringList cases = m_caseBrowser->casePaths();
    if (cases.isEmpty()) {
        statusBar()->showMessage("No case opened.", 3000);
        return;
    }

    // Collect all time directories to delete
    QStringList toDelete;
    for (const auto &casePath : cases) {
        QDir caseDir(casePath);
        auto entries = caseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry : entries) {
            // Is this a time directory? (starts with a digit, but NOT "0" or "0.orig")
            bool isTime = false;
            entry.toDouble(&isTime);
            if ((isTime || entry.startsWith("0.")) && entry != "0" && entry != "0.orig") {
                toDelete.append(caseDir.filePath(entry));
            }
        }
    }

    if (toDelete.isEmpty()) {
        statusBar()->showMessage("No time directories to clean.", 3000);
        return;
    }

    QStringList names;
    for (const auto &d : toDelete) names.append(QDir(d).dirName());
    QString msg = QString("Delete %1 time director%2?\n\n%3\n\n"
        "This will remove all simulation results except 0/.\nThis cannot be undone.")
        .arg(toDelete.size())
        .arg(toDelete.size() > 1 ? "ies" : "y")
        .arg(names.join(", "));

    auto ret = QMessageBox::warning(this, "Clean Time Directories", msg,
        QMessageBox::Ok | QMessageBox::Cancel);
    if (ret != QMessageBox::Ok) return;

    int deleted = 0;
    for (const auto &d : toDelete) {
        if (QDir(d).removeRecursively()) deleted++;
    }

    m_caseBrowser->refresh();
    statusBar()->showMessage(QString("Deleted %1 time directories.").arg(deleted), 5000);
}

// ────────────────────────────────────────────────────────────────────
// Sync blockMeshDict boundary names → 0/ field files boundaryField
// ────────────────────────────────────────────────────────────────────

void MainWindow::onSyncBoundaries()
{
    QStringList cases = m_caseBrowser->casePaths();
    if (cases.isEmpty()) {
        statusBar()->showMessage("No case opened. Open a case first.", 4000);
        return;
    }

    int totalUpdated = 0;
    QStringList updatedFiles;
    for (const auto &casePath : cases)
        totalUpdated += syncBoundariesForCase(casePath, &updatedFiles);

    // Reload synced files that are currently open in editor tabs
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (!editor || editor->fileName().isEmpty()) continue;

        QString canonicalEditor = QFileInfo(editor->fileName()).canonicalFilePath();
        for (const auto &f : updatedFiles) {
            if (QFileInfo(f).canonicalFilePath() == canonicalEditor) {
                QFile file(editor->fileName());
                if (file.open(QFile::ReadOnly | QFile::Text)) {
                    editor->setPlainText(QString::fromUtf8(file.readAll()));
                    file.close();
                    editor->document()->setModified(false);
                }
                break;
            }
        }
    }

    m_caseBrowser->refresh();
    statusBar()->showMessage(
        QString("Boundary sync complete. %1 file(s) updated across %2 case(s).")
            .arg(totalUpdated).arg(cases.size()), 5000);
}

int MainWindow::syncBoundariesForCase(const QString &casePath, QStringList *updatedFiles)
{
    // ── 1. Find blockMeshDict ──
    QString bmdPath;
    QStringList bmdCandidates = {
        QDir(casePath).filePath("system/blockMeshDict"),
        QDir(casePath).filePath("constant/polyMesh/blockMeshDict"),
    };
    for (const auto &c : bmdCandidates) {
        if (QFileInfo::exists(c)) { bmdPath = c; break; }
    }
    if (bmdPath.isEmpty()) {
        statusBar()->showMessage(
            "blockMeshDict not found in system/ or constant/polyMesh/.", 4000);
        return 0;
    }

    // ── 2. Parse boundary names from blockMeshDict ──
    QFile bmdFile(bmdPath);
    if (!bmdFile.open(QFile::ReadOnly | QFile::Text)) return 0;
    QString bmdContent = QString::fromUtf8(bmdFile.readAll());
    bmdFile.close();

    // Extract the "boundary" block: boundary ( ... );
    QRegularExpression bndRe(
        R"(boundary\s*\()",
        QRegularExpression::DotMatchesEverythingOption);
    auto bndStart = bndRe.match(bmdContent);
    if (!bndStart.hasMatch()) {
        statusBar()->showMessage("No 'boundary' block found in blockMeshDict.", 4000);
        return 0;
    }

    // Find matching closing ) by counting nesting depth
    int startPos = bndStart.capturedEnd();
    int depth = 1;
    int endPos = -1;
    for (int i = startPos; i < bmdContent.size(); ++i) {
        if (bmdContent[i] == '(') depth++;
        else if (bmdContent[i] == ')') {
            depth--;
            if (depth == 0) { endPos = i; break; }
        }
    }
    if (endPos < 0) {
        statusBar()->showMessage("Could not parse boundary block in blockMeshDict.", 4000);
        return 0;
    }
    QString bndBody = bmdContent.mid(startPos, endPos - startPos);

    // Parse each patch: patchName { ... }
    struct PatchInfo { QString name; QString type; };
    QVector<PatchInfo> patches;
    QRegularExpression patchRe(
        R"((\w+)\s*\{(.*?)\n\s*\})",
        QRegularExpression::DotMatchesEverythingOption);
    auto it = patchRe.globalMatch(bndBody);
    while (it.hasNext()) {
        auto pm = it.next();
        PatchInfo pi;
        pi.name = pm.captured(1);
        QRegularExpression typeRe(R"(type\s+(\S+)\s*;)");
        auto tm = typeRe.match(pm.captured(2));
        pi.type = tm.hasMatch() ? tm.captured(1) : "patch";
        patches.append(pi);
    }

    if (patches.isEmpty()) {
        statusBar()->showMessage("No patches found in blockMeshDict boundary.", 4000);
        return 0;
    }

    // ── 3. Find the first time directory (0/ or 0.orig/) ──
    QString timeDir;
    QDir caseDir(casePath);
    auto entries = caseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto &e : entries) {
        if (e == "0" || e == "0.orig" || e.startsWith("0.")) {
            timeDir = caseDir.filePath(e);
            break;
        }
    }
    if (timeDir.isEmpty()) {
        statusBar()->showMessage("No 0/ or 0.orig/ time directory found.", 4000);
        return 0;
    }

    // ── 4. For each field file in the time directory, sync boundaryField ──
    QDir td(timeDir);
    auto fieldFiles = td.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    int updatedCount = 0;

    for (const auto &fi : fieldFiles) {
        QString fn = fi.fileName();
        // Skip non-field files
        if (fn == "uniform" || fn.endsWith(".foam") || fn.endsWith(".orig"))
            continue;

        QFile fieldFile(fi.absoluteFilePath());
        if (!fieldFile.open(QFile::ReadOnly | QFile::Text)) continue;
        QString content = QString::fromUtf8(fieldFile.readAll());
        fieldFile.close();

        // Detect field type from FoamFile header
        QString foamClass;
        QRegularExpression classRe(R"(class\s+(\S+)\s*;)");
        auto cm = classRe.match(content);
        if (cm.hasMatch()) foamClass = cm.captured(1);

        // Parse boundaryField: find { and matching } by depth
        QRegularExpression bfStartRe(
            R"(boundaryField\s*\{)");
        auto bfStart = bfStartRe.match(content);
        if (!bfStart.hasMatch()) continue;

        int bfOpenPos = bfStart.capturedEnd() - 1; // position of {
        int bfDepth = 1;
        int bfClosePos = -1;
        for (int i = bfOpenPos + 1; i < content.size(); ++i) {
            if (content[i] == '{') bfDepth++;
            else if (content[i] == '}') {
                bfDepth--;
                if (bfDepth == 0) { bfClosePos = i; break; }
            }
        }
        if (bfClosePos < 0) continue;

        // Content between { and }
        QString bfContent = content.mid(bfOpenPos + 1, bfClosePos - bfOpenPos - 1);

        // Collect existing patch names in boundaryField
        QSet<QString> existingPatches;
        QRegularExpression existRe(R"((\w+)\s*\{(?!\s*\|))");
        auto eit = existRe.globalMatch(bfContent);
        while (eit.hasNext()) {
            auto em = eit.next();
            existingPatches.insert(em.captured(1));
        }

        // Build default BC snippet for each missing patch
        QStringList newEntries;
        for (const auto &p : patches) {
            if (existingPatches.contains(p.name)) continue; // already present

            QString bcType;
            if (p.type == "empty") {
                bcType = "empty";
            } else if (p.type == "symmetry" || p.type == "symmetryPlane") {
                bcType = "symmetry";
            } else if (p.type == "wedge") {
                bcType = "wedge";
            } else if (p.type == "cyclic" || p.type == "cyclicAMI") {
                bcType = "cyclic";
            } else if (p.type == "wall") {
                QString fnLower = fn.toLower();
                if (fnLower == "u" || fnLower == "v")
                    bcType = "noSlip";
                else if (fnLower == "p" || fnLower == "p_rgh")
                    bcType = "fixedFluxPressure";
                else if (fnLower == "k")
                    bcType = "kqRWallFunction";
                else if (fnLower == "epsilon")
                    bcType = "epsilonWallFunction";
                else if (fnLower == "omega")
                    bcType = "omegaWallFunction";
                else if (fnLower == "nut" || fnLower == "alphat")
                    bcType = "nutkWallFunction";
                else if (fnLower == "t" || foamClass.contains("Scalar"))
                    bcType = "zeroGradient";
                else
                    bcType = "zeroGradient";
            } else {
                bcType = "zeroGradient";
            }

            QString entry = QString(
                "        %1\n        {\n"
                "            type            %2;\n"
                "        }\n").arg(p.name, bcType);
            newEntries.append(entry);
        }

        if (newEntries.isEmpty()) continue; // all patches already present

        // Insert new entries right before the closing }
        QString insertion = newEntries.join("");
        content = content.left(bfClosePos) + insertion + content.mid(bfClosePos);

        // Write back
        if (fieldFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
            QTextStream out(&fieldFile);
            out << content;
            fieldFile.close();
            updatedCount++;
            if (updatedFiles)
                updatedFiles->append(fi.absoluteFilePath());
        }
    }

    if (updatedCount > 0) {
        statusBar()->showMessage(
            QString("Synced %1 field file(s) in %2 — added %3 missing patch(es).")
                .arg(updatedCount)
                .arg(QDir(timeDir).dirName())
                .arg(patches.size()), 5000);
    } else {
        statusBar()->showMessage("All field files are already in sync.", 3000);
    }
    return updatedCount;
}

// ── Unified terminal-style output dialog (VSCode light by default) ──
static void showTerminalOutput(QWidget *parent, const QString &title,
    const QString &headerOk, const QString &headerErr,
    const QString &output, const QString &info, int exitCode)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.resize(750, 550);
    dlg.setMinimumSize(500, 350);

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Title bar (VSCode-style) ──
    auto *titleBar = new QWidget();
    titleBar->setStyleSheet(exitCode == 0
        ? "background: #0078D7;"
        : "background: #C72E2E;");
    auto *tb = new QHBoxLayout(titleBar);
    tb->setContentsMargins(14, 9, 14, 9);
    auto *statusIcon = new QLabel("●");
    statusIcon->setStyleSheet("color: white; font-size: 11px;");
    tb->addWidget(statusIcon);
    auto *titleLbl = new QLabel(exitCode == 0 ? headerOk : headerErr);
    titleLbl->setStyleSheet("color: white; font-size: 13px; font-weight: 600; font-family: 'Segoe UI';");
    tb->addWidget(titleLbl, 1);
    auto *exitLabel = new QLabel(QString("Exit: %1").arg(exitCode));
    exitLabel->setStyleSheet("color: rgba(255,255,255,0.75); font-size: 11px; "
        "font-family: 'Cascadia Code', 'Consolas', monospace;");
    tb->addWidget(exitLabel);
    root->addWidget(titleBar);

    // ── Output area (VSCode light theme by default) ──
    auto *te = new QTextEdit();
    te->setReadOnly(true);
    QFont monoFont("Cascadia Code", 10);
    monoFont.setStyleHint(QFont::Monospace);
    if (!QFontInfo(monoFont).exactMatch()) {
        monoFont = QFont("Consolas", 10);
        monoFont.setStyleHint(QFont::Monospace);
    }
    te->setFont(monoFont);

    bool darkMode = false;
    auto applyTheme = [te](bool dark) {
        if (dark) {
            te->setStyleSheet(
                "QTextEdit { background: #0D0D0D; color: #D4D4D4; border: none; "
                "padding: 14px; selection-background-color: #264F78; }"
                "QScrollBar:vertical { background: #1E1E1E; width: 10px; margin: 0; }"
                "QScrollBar::handle:vertical { background: #555; border-radius: 5px; min-height: 30px; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");
        } else {
            te->setStyleSheet(
                "QTextEdit { background: #FFFFFF; color: #1E1E1E; border: none; "
                "padding: 14px; selection-background-color: #ADD6FF; }"
                "QScrollBar:vertical { background: #F5F5F5; width: 10px; margin: 0; }"
                "QScrollBar::handle:vertical { background: #C1C1C1; border-radius: 5px; min-height: 30px; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");
        }
    };
    applyTheme(false); // default: VSCode light
    te->setPlainText(output.isEmpty() ? "(no output)" : output);
    root->addWidget(te, 1);

    // ── Footer (VSCode status bar style) ──
    auto *footer = new QWidget();
    footer->setStyleSheet(exitCode == 0
        ? "background: #0078D7; border-top: none;"
        : "background: #C72E2E; border-top: none;");
    auto *fb = new QHBoxLayout(footer);
    fb->setContentsMargins(14, 5, 14, 5);
    auto *infoLbl = new QLabel(info);
    infoLbl->setStyleSheet("color: rgba(255,255,255,0.85); font-size: 11px; "
        "font-family: 'Cascadia Code', 'Consolas', monospace;");
    fb->addWidget(infoLbl, 1);

    // ── Theme toggle button ──
    auto *themeBtn = new QPushButton("☽ Dark");
    themeBtn->setStyleSheet(
        "QPushButton { padding: 3px 10px; background: rgba(255,255,255,0.15); color: white; "
        "border: 1px solid rgba(255,255,255,0.3); border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }");
    QObject::connect(themeBtn, &QPushButton::clicked, [te, themeBtn, applyTheme, &darkMode]() {
        darkMode = !darkMode;
        applyTheme(darkMode);
        themeBtn->setText(darkMode ? "☀ Light" : "☽ Dark");
    });
    fb->addWidget(themeBtn);

    auto *copyBtn = new QPushButton("Copy");
    copyBtn->setStyleSheet(
        "QPushButton { padding: 3px 12px; background: rgba(255,255,255,0.15); color: white; "
        "border: 1px solid rgba(255,255,255,0.3); border-radius: 3px; font-size: 11px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.25); }");
    QObject::connect(copyBtn, &QPushButton::clicked, [te]() {
        QApplication::clipboard()->setText(te->toPlainText());
    });
    fb->addWidget(copyBtn);

    auto *closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet(
        "QPushButton { padding: 3px 14px; background: rgba(255,255,255,0.2); color: white; "
        "border: none; border-radius: 3px; font-size: 11px; font-weight: 600; }"
        "QPushButton:hover { background: rgba(255,255,255,0.35); }");
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    fb->addWidget(closeBtn);
    root->addWidget(footer);

    dlg.exec();
}

// ── Run Python ──────────────────────────────────────────────────
void MainWindow::onRunPython()
{
    // Find Python executable: user-configured path first
    QString pythonPath = m_pythonPath;
    if (!pythonPath.isEmpty() && !QFileInfo::exists(pythonPath))
        pythonPath.clear();

    if (pythonPath.isEmpty())
        pythonPath = QStandardPaths::findExecutable("python");
    if (pythonPath.isEmpty())
        pythonPath = QStandardPaths::findExecutable("python3");
    if (pythonPath.isEmpty()) {
        // Check common Windows paths
        QStringList pyPaths = {
            "C:/Python312/python.exe", "C:/Python311/python.exe",
            "C:/Python310/python.exe", "C:/Python39/python.exe",
        };
        QString home = qgetenv("USERNAME");
        pyPaths << "C:/Users/" + home + "/AppData/Local/Programs/Python/Python312/python.exe";
        pyPaths << "C:/Users/" + home + "/AppData/Local/Programs/Python/Python311/python.exe";
        for (const auto &p : pyPaths)
            if (QFileInfo::exists(p)) { pythonPath = p; break; }
    }

    // If the current file is a .py, run it. Otherwise ask user.
    CodeEditor *editor = currentEditor();
    QString scriptPath;

    if (editor && editor->fileName().endsWith(".py", Qt::CaseInsensitive)) {
        scriptPath = editor->fileName();
        // Save before running
        if (editor->document()->isModified())
            saveEditor(editor);
    } else {
        // Let user pick a Python file
        scriptPath = QFileDialog::getOpenFileName(this, "Select Python Script",
            QString(), "Python Files (*.py);;All Files (*.*)");
    }

    if (scriptPath.isEmpty() || pythonPath.isEmpty()) {
        if (pythonPath.isEmpty()) {
            QMessageBox::information(this, "Python Not Found",
                "Python is not installed or not in PATH.\n\n"
                "Install Python from https://www.python.org/downloads/\n"
                "and ensure it is added to your system PATH.");
        }
        return;
    }

    // Run the script and capture output
    QProcess proc;
    proc.setWorkingDirectory(QFileInfo(scriptPath).absolutePath());
    proc.start(pythonPath, {scriptPath});

    if (!proc.waitForStarted(5000)) {
        QMessageBox::warning(this, "Python Error",
            "Failed to start Python.\nPath: " + pythonPath);
        return;
    }

    statusBar()->showMessage("Running: " + scriptPath, 3000);

    // Wait up to 60 seconds for completion
    if (!proc.waitForFinished(60000)) {
        proc.kill();
        QMessageBox::warning(this, "Python Timeout",
            "The script took too long and was terminated.");
        return;
    }

    QString output = QString::fromUtf8(proc.readAllStandardOutput());
    QString errors = QString::fromUtf8(proc.readAllStandardError());
    int exitCode = proc.exitCode();

    // Show output dialog
    QString result;
    if (!output.isEmpty())
        result += output;
    if (!errors.isEmpty()) {
        if (!result.isEmpty()) result += "\n\n";
        result += "--- STDERR ---\n" + errors;
    }
    if (result.isEmpty())
        result = "(no output)";

    showTerminalOutput(this, "Python Output",
        "Python Script Completed", "Python Script Error",
        result,
        QFileInfo(scriptPath).fileName() + "   |   " + pythonPath,
        exitCode);

    statusBar()->showMessage(
        QString("Python exit code: %1").arg(exitCode), 5000);
}

void MainWindow::onConfigurePython()
{
    QString path = QFileDialog::getOpenFileName(this, "Select Python Executable",
        m_pythonPath.isEmpty() ? "C:/" : QFileInfo(m_pythonPath).absolutePath(),
#ifdef Q_OS_WIN
        "Python (python.exe);;All Files (*.*)"
#else
        "Python (python python3);;All Files (*)"
#endif
    );
    if (!path.isEmpty()) {
        m_pythonPath = path;
        saveSettings();
        statusBar()->showMessage("Python path set: " + path, 5000);
    }
}

// ── Configure C++ compiler path ─────────────────────────────────
void MainWindow::onConfigureCpp()
{
    QString path = QFileDialog::getOpenFileName(this, "Select C++ Compiler (g++)",
        m_cppCompilerPath.isEmpty() ? "D:/3.Wpsandother/mingw64/bin" : QFileInfo(m_cppCompilerPath).absolutePath(),
        "g++ (g++.exe);;All Files (*.*)");
    if (!path.isEmpty()) {
        m_cppCompilerPath = path;
        saveSettings();
        statusBar()->showMessage("C++ compiler path set: " + path, 5000);
    }
}

// ── Run C++ ─────────────────────────────────────────────────────
void MainWindow::onRunCpp()
{
    QString compilerPath = m_cppCompilerPath;
    if (compilerPath.isEmpty() || !QFileInfo::exists(compilerPath)) {
        // Auto-detect g++
        compilerPath = QStandardPaths::findExecutable("g++");
        if (compilerPath.isEmpty())
            compilerPath = QStandardPaths::findExecutable("c++");
        // Common MinGW/MSYS2 paths
        QStringList paths = {
            "D:/3.Wpsandother/mingw64/bin/g++.exe",
            "C:/msys64/mingw64/bin/g++.exe",
            "C:/mingw64/bin/g++.exe",
        };
        for (const auto &p : paths)
            if (QFileInfo::exists(p)) { compilerPath = p; break; }
    }

    if (compilerPath.isEmpty()) {
        QMessageBox::information(this, "Compiler Not Found",
            "g++ compiler not found.\n\n"
            "Configure the path via: Case → C++ Compiler Path...\n"
            "Example: D:/mingw64/bin/g++.exe");
        return;
    }

    CodeEditor *editor = currentEditor();
    QString sourcePath;

    if (editor && (editor->fileName().endsWith(".cpp", Qt::CaseInsensitive)
                   || editor->fileName().endsWith(".cxx", Qt::CaseInsensitive)
                   || editor->fileName().endsWith(".cc", Qt::CaseInsensitive)
                   || editor->fileName().endsWith(".c", Qt::CaseInsensitive))) {
        sourcePath = editor->fileName();
        if (editor->document()->isModified())
            saveEditor(editor);
    } else {
        sourcePath = QFileDialog::getOpenFileName(this, "Select C++ Source File",
            QString(), "C++ Files (*.cpp *.cxx *.cc *.c);;All Files (*.*)");
    }

    if (sourcePath.isEmpty()) return;

    QFileInfo srcInfo(sourcePath);
    QString exePath = srcInfo.absolutePath() + "/" + srcInfo.completeBaseName() + ".exe";

    // Compile
    statusBar()->showMessage("Compiling: " + srcInfo.fileName(), 3000);

    QProcess compile;
    compile.setWorkingDirectory(srcInfo.absolutePath());
    compile.start(compilerPath, {"-std=c++17", "-O2", "-o", exePath, sourcePath});

    if (!compile.waitForFinished(30000)) {
        QMessageBox::warning(this, "Compilation Timeout", "Compilation timed out.");
        return;
    }

    QString compileErr = QString::fromUtf8(compile.readAllStandardError());
    if (compile.exitCode() != 0) {
        QMessageBox::warning(this, "Compilation Failed",
            "Compilation failed with exit code " + QString::number(compile.exitCode()) + "\n\n" + compileErr);
        statusBar()->showMessage("Compilation failed.", 5000);
        return;
    }

    statusBar()->showMessage("Compiled OK. Running...", 3000);

    // Run
    QProcess run;
    run.setWorkingDirectory(srcInfo.absolutePath());
    run.start(exePath, {});

    if (!run.waitForStarted(5000)) {
        QMessageBox::warning(this, "Run Failed", "Failed to start: " + exePath);
        return;
    }

    if (!run.waitForFinished(30000)) {
        run.kill();
        QMessageBox::warning(this, "Execution Timeout", "Program took too long and was terminated.");
        return;
    }

    QString output = QString::fromUtf8(run.readAllStandardOutput());
    QString errors = QString::fromUtf8(run.readAllStandardError());
    int exitCode = run.exitCode();

    QString result;
    if (!output.isEmpty()) result += output;
    if (!errors.isEmpty()) { if (!result.isEmpty()) result += "\n\n"; result += "--- STDERR ---\n" + errors; }
    if (result.isEmpty()) result = "(no output)";

    showTerminalOutput(this, "C++ Output",
        "C++ Program Completed", "C++ Program Error",
        result,
        srcInfo.fileName() + "   |   " + compilerPath,
        exitCode);

    statusBar()->showMessage(QString("C++ exit code: %1").arg(exitCode), 5000);
}

void MainWindow::onParaView()
{
    QStringList cases = m_caseBrowser->casePaths();
    if (cases.isEmpty()) {
        statusBar()->showMessage("No case opened. Open a case first.", 4000);
        return;
    }

    QString caseDir = cases.first();

    // Create .foam dummy file
    QString foamFile = QDir(caseDir).filePath(
        QDir(caseDir).dirName() + ".foam");
    QFile f(foamFile);
    if (!f.exists()) {
        (void)f.open(QFile::WriteOnly);
        f.close();
    }

    // Try user-configured path first, then auto-detect
    QString paraviewPath = m_paraviewPath;
    if (!paraviewPath.isEmpty() && !QFileInfo::exists(paraviewPath))
        paraviewPath.clear();

    if (paraviewPath.isEmpty()) {
#ifdef Q_OS_WIN
        QStringList searchPaths = {
            "paraview.exe",
            "C:/Program Files/ParaView 5.13/bin/paraview.exe",
            "C:/Program Files/ParaView 5.12/bin/paraview.exe",
            "C:/Program Files/ParaView 5.11/bin/paraview.exe",
            "C:/Program Files/ParaView 5.10/bin/paraview.exe",
            "C:/Program Files/ParaView/bin/paraview.exe",
        };
        for (const auto &p : searchPaths) {
            if (QFileInfo::exists(p)) { paraviewPath = p; break; }
        }
#else
        for (const auto &p : {"paraview","/usr/bin/paraview","/usr/local/bin/paraview"}) {
            if (QFileInfo::exists(p)) { paraviewPath = p; break; }
        }
#endif
        if (paraviewPath.isEmpty())
            paraviewPath = QStandardPaths::findExecutable("paraview");
    }

    // ── ParaView NOT found ──
    if (paraviewPath.isEmpty()) {
        QMessageBox::information(this, "ParaView Not Found",
            "ParaView is not installed on this computer.\n\n"
            "Download ParaView (free, open-source):\n"
            "https://www.paraview.org/download/\n\n"
            "After installation, set the path via:\n"
            "Case → ParaView Path...\n\n"
            "The .foam file has been created at:\n" + foamFile);
        statusBar()->showMessage(
            "ParaView not found — download at paraview.org", 8000);
        return;
    }

    // ── ParaView found — launch ──
    statusBar()->showMessage(
        "Launching: " + paraviewPath, 5000);

    bool launched = QProcess::startDetached(paraviewPath, {
        "--data=" + QDir::toNativeSeparators(foamFile)
    });

    if (launched) {
        statusBar()->showMessage(
            "ParaView launched: " + paraviewPath, 8000);
    } else {
        QMessageBox::warning(this, "ParaView Launch Failed",
            "ParaView was found but could not be launched.\n\n"
            "Path: " + paraviewPath + "\n\n"
            "Try launching it manually, or select a different path via:\n"
            "Case → ParaView Path...\n\n"
            ".foam file: " + foamFile);
        statusBar()->showMessage("ParaView launch failed — check path.", 8000);
    }
}

void MainWindow::onConfigureParaView()
{
    QString current = m_paraviewPath;
    QString path = QFileDialog::getOpenFileName(this, "Select ParaView Executable",
        current.isEmpty() ? "C:/Program Files" : QFileInfo(current).absolutePath(),
#ifdef Q_OS_WIN
        "ParaView (paraview.exe);;All Files (*.*)"
#else
        "ParaView (paraview);;All Files (*)"
#endif
    );
    if (!path.isEmpty()) {
        m_paraviewPath = path;
        saveSettings();
        statusBar()->showMessage("ParaView path set: " + path, 5000);
    }
}

void MainWindow::onRefreshCase()
{
    m_caseBrowser->refresh();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About OpenFOAM GUI",
                       "<h3>OpenFOAM GUI v2.0.2  byCHen</h3>"
                       "<p>A CFD case manager for OpenFOAM v2012 to v2512 </p>"
                       "<p>Open, browse, edit, and save OpenFOAM case files "
                       "with syntax highlighting and case structure awareness.</p>"
                       "<hr>"
                       "<p><b>Features:</b></p>"
                       "<ul>"
                       "<li>Case directory browser (0/, constant/, system/)</li>"
                       "<li>Syntax highlighting for OpenFOAM dictionaries</li>"
                       "<li>Line numbers and current line highlighting</li>"
                       "<li>Multi-tab editing</li>"
                       "<li>File type detection and tooltips</li>"
                       "<li>Recent cases history</li>"
                       "</ul>");
}

void MainWindow::onOpenTerminal()
{
    // Determine working directory: case path if open, otherwise project dir
    QStringList cases = m_caseBrowser->casePaths();
    QString workDir = cases.isEmpty() ? QDir::currentPath() : cases.first();

#ifdef Q_OS_WIN
    // Try Windows Terminal first, fall back to cmd.exe
    QString wtPath = QDir("C:/Windows/System32").filePath("cmd.exe");

    // Try Windows Terminal (wt.exe), fall back to cmd.exe
    QString wtExe = QStandardPaths::findExecutable("wt.exe");
    if (wtExe.isEmpty()) {
        // Check default WindowsApps location
        QString userLocal = qgetenv("LOCALAPPDATA");
        QString wtPath = userLocal + "/Microsoft/WindowsApps/wt.exe";
        if (QFileInfo::exists(wtPath)) wtExe = wtPath;
    }

    if (!wtExe.isEmpty()) {
        QProcess::startDetached(wtExe, {"-d", workDir});
        statusBar()->showMessage("Windows Terminal opened in: " + workDir, 4000);
    } else {
        QString cmdPath = QStandardPaths::findExecutable("cmd.exe");
        if (cmdPath.isEmpty()) cmdPath = "C:/Windows/System32/cmd.exe";
        QProcess::startDetached(cmdPath, {"/k", "cd", "/d", workDir});
        statusBar()->showMessage("Command Prompt opened in: " + workDir, 4000);
    }
#elif defined(Q_OS_MACOS)
    QStringList args = {"-a", "Terminal", workDir};
    QProcess::startDetached("open", args);
    statusBar()->showMessage("Terminal opened in: " + workDir, 4000);
#else
    // Linux: try common terminal emulators
    QStringList terms = {
        "gnome-terminal", "konsole", "xfce4-terminal",
        "lxterminal", "x-terminal-emulator", "xterm"
    };
    bool launched = false;
    for (const auto &term : terms) {
        if (launched) break;
        QString exe = QStandardPaths::findExecutable(term);
        if (!exe.isEmpty()) {
            QStringList args;
            if (term == "gnome-terminal") args << "--working-directory" << workDir;
            else if (term == "konsole")   args << "--workdir" << workDir;
            else if (term == "xfce4-terminal") args << "--working-directory" << workDir;
            launched = QProcess::startDetached(exe, args);
        }
    }
    if (!launched) {
        QProcess::startDetached("xterm", {"-e", "cd", workDir, "&&", "bash"});
    }
    statusBar()->showMessage("Terminal opened in: " + workDir, 4000);
#endif
}

// ────────────────────────────────────────────────────────────────────
// Helpers
// ────────────────────────────────────────────────────────────────────

CodeEditor* MainWindow::currentEditor()
{
    return qobject_cast<CodeEditor*>(m_tabWidget->currentWidget());
}

CodeEditor* MainWindow::editorForFile(const QString &filePath)
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (editor && editor->fileName() == filePath)
            return editor;
    }
    return nullptr;
}

void MainWindow::updateWindowTitle()
{
    QStringList cases = m_caseBrowser->casePaths();
    if (cases.isEmpty()) {
        setWindowTitle("OpenFOAM GUI — CFD Case Manager");
    } else if (cases.size() == 1) {
        QFileInfo fi(cases.first());
        setWindowTitle(QString("%1 — OpenFOAM GUI").arg(fi.fileName()));
    } else {
        setWindowTitle(QString("%1 cases — OpenFOAM GUI").arg(cases.size()));
    }
}

void MainWindow::updateStatusBarForEditor(CodeEditor *editor)
{
    if (!editor) {
        m_statusFileType->setText("Ready");
        m_statusCursorPos->setText("");
        return;
    }

    // Language
    QString langStr = LanguageDetector::languageName(editor->language());

    // Show file type info
    QFileInfo fi(editor->fileName());
    QString typeInfo = QString("%1  [%2]").arg(fi.fileName(), langStr);

    QString desc = OFParser::fileDescription(fi.fileName());
    if (!desc.isEmpty())
        typeInfo += "  <" + desc + ">";

    // Also parse header if present
    QString content = editor->toPlainText();
    if (OFParser::isOpenFOAMFile(content)) {
        auto header = OFParser::parseHeader(content);
        if (!header.foamClass.isEmpty()) {
            typeInfo += "  (class: " + header.foamClass;
            if (!header.object.isEmpty())
                typeInfo += ", object: " + header.object;
            typeInfo += ")";
        }
    }

    m_statusFileType->setText(typeInfo);

    // Cursor position
    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col  = cursor.columnNumber() + 1;
    m_statusCursorPos->setText(QString("Ln %1, Col %2").arg(line).arg(col));
}

void MainWindow::addRecentCase(const QString &path)
{
    m_recentCases.removeAll(path);
    m_recentCases.prepend(path);
    while (m_recentCases.size() > MaxRecentCases)
        m_recentCases.removeLast();
    updateRecentCasesMenu();
}

void MainWindow::updateRecentCasesMenu()
{
    for (int i = 0; i < MaxRecentCases; ++i) {
        if (i < m_recentCases.size()) {
            QFileInfo fi(m_recentCases[i]);
            QString text = QString("%1 | %2").arg(i + 1).arg(fi.fileName());
            m_recentCaseActions[i]->setText(text);
            m_recentCaseActions[i]->setData(m_recentCases[i]);
            m_recentCaseActions[i]->setToolTip(m_recentCases[i]);
            m_recentCaseActions[i]->setVisible(true);
        } else {
            m_recentCaseActions[i]->setVisible(false);
        }
    }
}

// ────────────────────────────────────────────────────────────────────
// Settings persistence
// ────────────────────────────────────────────────────────────────────

void MainWindow::loadSettings()
{
    QSettings settings;
    m_recentCases = settings.value("recentCases").toStringList();
    m_paraviewPath = settings.value("paraviewPath").toString();
    m_pythonPath       = settings.value("pythonPath").toString();
    m_cppCompilerPath  = settings.value("cppCompilerPath").toString();
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    updateRecentCasesMenu();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("recentCases", m_recentCases);
    settings.setValue("paraviewPath", m_paraviewPath);
    settings.setValue("pythonPath",       m_pythonPath);
    settings.setValue("cppCompilerPath",  m_cppCompilerPath);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

// ────────────────────────────────────────────────────────────────────
// Close event
// ────────────────────────────────────────────────────────────────────

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check for unsaved tabs
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (editor && editor->document()->isModified()) {
            // Switch to the tab and ask
            m_tabWidget->setCurrentIndex(i);
            QMessageBox::StandardButton ret =
                QMessageBox::warning(this, "Unsaved Changes",
                                     QString("'%1' has been modified.\n"
                                             "Do you want to save your changes?")
                                     .arg(QFileInfo(editor->fileName()).fileName()),
                                     QMessageBox::Save | QMessageBox::Discard
                                     | QMessageBox::Cancel);
            if (ret == QMessageBox::Save)
                saveEditor(editor);
            else if (ret == QMessageBox::Cancel) {
                event->ignore();
                return;
            }
        }
    }

    saveSettings();
    event->accept();
}
