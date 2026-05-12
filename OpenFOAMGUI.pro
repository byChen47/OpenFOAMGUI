QT       += core gui widgets svg svgwidgets

CONFIG += c++17 console

# Hide console window: build normally, then flip PE subsystem
QMAKE_POST_LINK = $$quote(objcopy --subsystem=windows:6.0 $(DESTDIR_TARGET) $(DESTDIR_TARGET)_tmp && mv -f $(DESTDIR_TARGET)_tmp $(DESTDIR_TARGET))

TARGET = OpenFOAMGUI
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/fileviewer.cpp \
    src/casebrowser.cpp \
    src/codeeditor.cpp \
    src/ofhighlighter.cpp \
    src/ofparser.cpp \
    src/linenumberarea.cpp \
    src/languagedetector.cpp \
    src/bctypedatabase.cpp \
    src/bcpanel.cpp \
    src/turbulencemodeldatabase.cpp \
    src/turbulencepanel.cpp \
    src/schemespanel.cpp \
    src/snappypanel.cpp \
    src/dictpanel.cpp \
    src/ofmeshreader.cpp

HEADERS += \
    src/mainwindow.h \
    src/fileviewer.h \
    src/ofmeshreader.h \
    src/casebrowser.h \
    src/codeeditor.h \
    src/ofhighlighter.h \
    src/ofparser.h \
    src/linenumberarea.h \
    src/languagedetector.h \
    src/bctypedatabase.h \
    src/bcpanel.h \
    src/turbulencemodeldatabase.h \
    src/turbulencepanel.h \
    src/schemespanel.h \
    src/snappypanel.h \
    src/dictpanel.h

RESOURCES += \
    resources.qrc

RC_ICONS = src/bychen.ico

# ── Test target ──
check.target = check
check.commands = \
    cd tests && \
    $(QMAKE) tests.pro -o Makefile.Tests && \
    $(MAKE) -f Makefile.Tests && \
    release/OpenFOAMGUI_tests.exe
check.depends = all
QMAKE_EXTRA_TARGETS += check
