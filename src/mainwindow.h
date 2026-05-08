#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QLabel>
#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QCloseEvent>
#include <QFileInfo>
#include <QStackedWidget>

class CaseBrowser;
class CodeEditor;
class BCPanel;
class TurbulencePanel;
class SchemesPanel;
class SnappyPanel;
class DictPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onOpenCase();
    void onOpenRecentCase();
    void onFileSelected(const QString &filePath);
    void onSaveFile();
    void onSaveAllFiles();
    void onSaveFileAs();
    void onCloseTab(int index);
    void onTabChanged(int index);
    void onTabContextMenu(const QPoint &pos);
    void onFindText();
    void onNewFile();
    void onNewFolder();
    void onDeleteSelected();
    void onCleanTimeDirs();
    void onParaView();
    void onConfigureParaView();
    void onAbout();
    void onRefreshCase();
    void onOpenTerminal();
    void onCloseCase();
    void onSyncBoundaries();
    void onRunPython();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDockWidgets();
    void setupConnections();

    void loadSettings();
    void saveSettings();
    void addRecentCase(const QString &path);
    void updateRecentCasesMenu();

    bool openFileInTab(const QString &filePath);
    CodeEditor* currentEditor();
    CodeEditor* editorForFile(const QString &filePath);
    void updateWindowTitle();
    void updateStatusBarForEditor(CodeEditor *editor);
    bool saveEditor(CodeEditor *editor);
    int syncBoundariesForCase(const QString &casePath);

    // Main widgets
    CaseBrowser      *m_caseBrowser;
    QTabWidget       *m_tabWidget;
    BCPanel          *m_bcPanel;
    TurbulencePanel  *m_turbulencePanel;
    SchemesPanel     *m_schemesPanel;
    SnappyPanel      *m_snappyPanel;
    DictPanel        *m_dictPanel;
    QStackedWidget   *m_rightPanelStack;

    // Status bar
    QLabel *m_statusFileType;
    QLabel *m_statusCursorPos;
    QLabel *m_statusEncoding;

    // Dock widgets
    QDockWidget *m_caseBrowserDock;
    QDockWidget *m_bcPanelDock;

    // Menus
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_caseMenu;
    QMenu *m_helpMenu;
    QMenu *m_recentCasesMenu;

    // Toolbar
    QToolBar *m_mainToolBar;

    // Actions
    QAction *m_openCaseAction;
    QAction *m_closeCaseAction;
    QAction *m_saveAction;
    QAction *m_saveAllAction;
    QAction *m_saveAsAction;
    QAction *m_closeTabAction;
    QAction *m_exitAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_findAction;
    QAction *m_commentAction;
    QAction *m_newFileAction;
    QAction *m_newFolderAction;
    QAction *m_deleteAction;
    QAction *m_cleanTimeAction;
    QAction *m_pythonAction;
    QAction *m_paraviewAction;
    QAction *m_paraviewConfigAction;
    QAction *m_refreshAction;
    QAction *m_bcPanelAction;
    QAction *m_terminalAction;
    QAction *m_syncBoundariesAction;
    QAction *m_aboutAction;
    QAction *m_recentCaseActions[10];

    // State
    QStringList m_recentCases;
    QString     m_paraviewPath;  // user-configured ParaView binary path
    static const int MaxRecentCases = 10;
};

#endif // MAINWINDOW_H
