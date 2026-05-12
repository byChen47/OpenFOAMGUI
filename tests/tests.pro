QT       += core
CONFIG   += c++17 console

TARGET    = OpenFOAMGUI_tests
TEMPLATE  = app

INCLUDEPATH += ../src

SOURCES += \
    test_main.cpp \
    ../src/ofparser.cpp \
    ../src/languagedetector.cpp
