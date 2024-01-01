/********************************************************************
    created:   2003/09/09
    filename:  main.cpp
	
    author:    Micha Bieber	
*********************************************************************/

#include "mesh2mainwindow.h"
#include <qapplication.h>

int main(int argc, char **argv)
{
#if QT_VERSION < 0x060000 // FIXME
    QApplication::setColorSpec(QApplication::CustomColor);
#endif
    QApplication app(argc, argv);

#if QT_VERSION < 0x060000 // FIXME
    if (!QGLFormat::hasOpenGL()) {
        qWarning("This system has no OpenGL support. Exiting.");
        return -1;
    }
#endif

    Mesh2MainWindow mainwindow;

#if QT_VERSION < 0x040000
    app.setMainWidget(&mainwindow);
#endif

    mainwindow.resize(1024, 768);
    mainwindow.show();

    return app.exec();
}
