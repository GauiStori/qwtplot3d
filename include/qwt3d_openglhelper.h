#ifndef __openglhelper_2003_06_06_15_49__
#define __openglhelper_2003_06_06_15_49__

#include "qglobal.h"
#if QT_VERSION < 0x040000
#include <qgl.h>
#elif QT_VERSION < 0x050400
#include <QtOpenGL/qgl.h>
#elif QT_VERSION < 0x060000 && !defined(HAVE_GLES)
#include <QtOpenGL/qgl.h>
#else
#include <QOpenGLWidget>
#define updateGL update
#define QGLWidget QOpenGLWidget
#endif

#ifdef HAVE_GLES
#include <GLES/gl.h>
#include <GLES/glext.h>
#ifndef GLdouble
typedef double GLdouble;
#endif

#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif

void qwt3dGlBegin(GLenum mode);
void qwt3dGlEnd();
void qwt3dGlVertex3d(GLdouble x, GLdouble y, GLdouble z);
void qwt3dGlVertex3dv(const GLdouble *v);
void qwt3dGlNormal3d(GLdouble x, GLdouble y, GLdouble z);
void qwt3dGlNormal3dv(const GLdouble *v);
void qwt3dGlColor3d(GLdouble r, GLdouble g, GLdouble b);
void qwt3dGlColor4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a);
void qwt3dGlColor4dv(const GLdouble *rgba);
void qwt3dGlGetDoublev(GLenum pname, GLdouble *params);
void qwt3dGlOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                  GLdouble nearValue, GLdouble farValue);
void qwt3dGlFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                    GLdouble nearValue, GLdouble farValue);
void qwt3dGlPolygonMode(GLenum face, GLenum mode);
void qwt3dGlLineStipple(GLint factor, GLushort pattern);
void qwt3dGlRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
void qwt3dGlDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
                       const void *pixels);
void qwt3dGlCylinder(GLdouble baseRadius, GLdouble topRadius, GLdouble height,
                     GLint slices);
void qwt3dGlDisk(GLdouble innerRadius, GLdouble outerRadius, GLint slices);
int glesProject(GLdouble objX, GLdouble objY, GLdouble objZ,
                const GLdouble *modelview, const GLdouble *projection,
                const GLint *viewport, GLdouble *winX, GLdouble *winY, GLdouble *winZ);
int glesUnProject(GLdouble winX, GLdouble winY, GLdouble winZ,
                  const GLdouble *modelview, const GLdouble *projection,
                  const GLint *viewport, GLdouble *objX, GLdouble *objY, GLdouble *objZ);
const GLubyte *glesErrorString(GLenum error);

#define glBegin qwt3dGlBegin
#define glEnd qwt3dGlEnd
#define glVertex3d qwt3dGlVertex3d
#define glVertex3dv qwt3dGlVertex3dv
#define glNormal3d qwt3dGlNormal3d
#define glNormal3dv qwt3dGlNormal3dv
#define glColor3d qwt3dGlColor3d
#define glColor4d qwt3dGlColor4d
#define glColor4dv qwt3dGlColor4dv
#define glGetDoublev qwt3dGlGetDoublev
#define glOrtho qwt3dGlOrtho
#define glFrustum qwt3dGlFrustum
#define glPolygonMode qwt3dGlPolygonMode
#define glLineStipple qwt3dGlLineStipple
#define glRasterPos3d qwt3dGlRasterPos3d
#define glDrawPixels qwt3dGlDrawPixels
#else
#include <GL/glu.h>
#endif

namespace Qwt3D {

#ifndef QWT3D_NOT_FOR_DOXYGEN

class GLStateBewarer
{
public:
    GLStateBewarer(GLenum what, bool on, bool persist = false)
    {
        state_ = what;
        stateval_ = glIsEnabled(what);
        if (on)
            turnOn(persist);
        else
            turnOff(persist);
    }

    ~GLStateBewarer()
    {
        if (stateval_)
            glEnable(state_);
        else
            glDisable(state_);
    }

    void turnOn(bool persist = false)
    {
        glEnable(state_);
        if (persist)
            stateval_ = true;
    }

    void turnOff(bool persist = false)
    {
        glDisable(state_);
        if (persist)
            stateval_ = false;
    }

private:
    GLenum state_;
    bool stateval_;
};

inline const GLubyte *gl_error()
{
    GLenum errcode;
    const GLubyte* err = 0;

    if ((errcode = glGetError()) != GL_NO_ERROR)
    {
#ifdef HAVE_GLES
        err = glesErrorString(errcode);
#else
        err = gluErrorString(errcode);
#endif
    }
    return err;
}

inline void SaveGlDeleteLists(GLuint &lstidx, GLsizei range)
{
#ifdef HAVE_GLES
    (void) range;
#else
    if (glIsList(lstidx))
        glDeleteLists(lstidx, range);
#endif
    lstidx = 0;
}

//! get OpenGL transformation matrices
/**
	Don't rely on (use) this in display lists !
	\param modelMatrix should be a GLdouble[16]
	\param projMatrix should be a GLdouble[16]
	\param viewport should be a GLint[4]
*/
inline void getMatrices(GLdouble *modelMatrix, GLdouble *projMatrix, GLint *viewport)
{
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
}

//! simplified glut routine (glUnProject): windows coordinates_p --> object coordinates_p
/**
	Don't rely on (use) this in display lists !
*/
inline bool ViewPort2World(
    double &objx, double &objy, double &objz, double winx, double winy, double winz)
{
    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];

	getMatrices(modelMatrix, projMatrix, viewport);
#ifdef HAVE_GLES
    int res = glesUnProject(winx, winy, winz, modelMatrix, projMatrix, viewport, &objx, &objy, &objz);
#else
    int res = gluUnProject(winx, winy, winz, modelMatrix, projMatrix, viewport, &objx, &objy, &objz);
#endif
    return (res == GL_FALSE) ? false : true;
}

//! simplified glut routine (glProject): object coordinates_p --> windows coordinates_p
/**
    Don't rely on (use) this in display lists !
*/
inline bool World2ViewPort(
    double &winx, double &winy, double &winz, double objx, double objy, double objz)
{
    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];

    getMatrices(modelMatrix, projMatrix, viewport);
#ifdef HAVE_GLES
    int res = glesProject(objx, objy, objz, modelMatrix, projMatrix, viewport, &winx, &winy, &winz);
#else
    int res = gluProject(objx, objy, objz, modelMatrix, projMatrix, viewport, &winx, &winy, &winz);
#endif

    return (res == GL_FALSE) ? false : true;
}

#endif // QWT3D_NOT_FOR_DOXYGEN

} // namespace Qwt3D

#endif
