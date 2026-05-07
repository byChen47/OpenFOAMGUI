QT       += core gui widgets svgwidgets openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += console

mingw {
    QMAKE_LFLAGS_CONSOLE = -Wl,-subsystem,windows -mthreads
}

LIBS += -lopengl32

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
    src/meshviewer.cpp \
    src/ofmeshreader.cpp

HEADERS += \
    src/mainwindow.h \
    src/fileviewer.h \
    src/meshviewer.h \
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
