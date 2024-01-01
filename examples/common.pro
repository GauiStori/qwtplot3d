TEMPLATE     = app
CONFIG      += qt warn_on thread debug
UI_DIR = tmp
MOC_DIR      = tmp
OBJECTS_DIR  = tmp
DESTDIR = ../bin
greaterThan(QT_MAJOR_VERSION,5) {
  QT += core gui widgets opengl openglwidgets
}
else {
  QT += opengl
}

#unix:LIBS += -lqwtplot3d -L../../lib
linux-g++:QMAKE_CXXFLAGS += -fno-exceptions

unix{
  unix:LIBS += -lqwtplot3d-qt$$QT_MAJOR_VERSION
  greaterThan(QT_MAJOR_VERSION, 4) {
    INCLUDEPATH    += /usr/include/qwtplot3d
  }
  else {
    INCLUDEPATH    += /usr/include/qwtplot3d-qt4
  }
}

DEPENDPATH	= $$INCLUDEPATH

win32{
  LIBS += ../../lib/qwtplot3d.lib
  TEMPLATE  = vcapp
  DEFINES  += QT_DLL QWT3D_DLL
  RC_FILE = ../icon.rc
  contains (CONFIG, debug)  {
    QMAKE_LFLAGS += /NODEFAULTLIB:msvcrt
    DESTDIR = ../bin/debug
  }
  !contains (CONFIG, release)  {
    DESTDIR = ../bin/release
  }
}

MYVERSION = $$[QMAKE_VERSION] 
ISQT4 = $$find(MYVERSION, ^[2-9])

!isEmpty( ISQT4 ) {
#RESOURCES     = ../images.qrc
QT += opengl
}

isEmpty( ISQT4 ) {
CONFIG += opengl
}
