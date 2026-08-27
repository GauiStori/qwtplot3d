TEMPLATE = app
TARGET = qwtplot3d-core-tests
CONFIG += console testcase c++11
CONFIG -= app_bundle

INCLUDEPATH += ../include
QT += core gui

contains(CONFIG, gles) {
    DEFINES += HAVE_GLES
    QMAKE_LIBS_OPENGL =
    unix:LIBS += -lGLESv1_CM
} else {
    CONFIG += opengl
    unix:LIBS += -lGLU
}

SOURCES += core_tests.cpp ../src/qwt3d_types.cpp
