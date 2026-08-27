#include "qwt3d_openglhelper.h"

#ifdef HAVE_GLES

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

struct ImmediateVertex
{
    GLfloat position[3];
    GLfloat normal[3];
    GLfloat color[4];
};

GLenum primitiveMode = GL_POINTS;
std::vector<ImmediateVertex> vertices;
GLfloat currentNormal[3] = {0.0f, 0.0f, 1.0f};
GLfloat currentColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLdouble rasterPosition[3] = {0.0, 0.0, 0.0};

void drawVertices(GLenum mode, const std::vector<ImmediateVertex> &data)
{
    if (data.empty())
        return;

    const GLboolean vertexArray = glIsEnabled(GL_VERTEX_ARRAY);
    const GLboolean normalArray = glIsEnabled(GL_NORMAL_ARRAY);
    const GLboolean colorArray = glIsEnabled(GL_COLOR_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(ImmediateVertex), data[0].position);
    glNormalPointer(GL_FLOAT, sizeof(ImmediateVertex), data[0].normal);
    glColorPointer(4, GL_FLOAT, sizeof(ImmediateVertex), data[0].color);
    glDrawArrays(mode, 0, static_cast<GLsizei>(data.size()));

    if (!colorArray)
        glDisableClientState(GL_COLOR_ARRAY);
    if (!normalArray)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (!vertexArray)
        glDisableClientState(GL_VERTEX_ARRAY);
}

void multiplyMatrices(const GLdouble a[16], const GLdouble b[16], GLdouble result[16])
{
    for (int column = 0; column != 4; ++column) {
        for (int row = 0; row != 4; ++row) {
            result[column * 4 + row] = 0.0;
            for (int k = 0; k != 4; ++k)
                result[column * 4 + row] += a[k * 4 + row] * b[column * 4 + k];
        }
    }
}

void multiplyVector(const GLdouble matrix[16], const GLdouble input[4], GLdouble output[4])
{
    for (int row = 0; row != 4; ++row) {
        output[row] = 0.0;
        for (int column = 0; column != 4; ++column)
            output[row] += matrix[column * 4 + row] * input[column];
    }
}

bool invertMatrix(const GLdouble m[16], GLdouble inverse[16])
{
    GLdouble augmented[4][8];
    for (int row = 0; row != 4; ++row) {
        for (int column = 0; column != 4; ++column)
            augmented[row][column] = m[column * 4 + row];
        for (int column = 0; column != 4; ++column)
            augmented[row][column + 4] = row == column ? 1.0 : 0.0;
    }

    for (int column = 0; column != 4; ++column) {
        int pivot = column;
        for (int row = column + 1; row != 4; ++row) {
            if (std::fabs(augmented[row][column]) > std::fabs(augmented[pivot][column]))
                pivot = row;
        }
        if (std::fabs(augmented[pivot][column]) < 1e-15)
            return false;
        if (pivot != column)
            for (int k = 0; k != 8; ++k)
                std::swap(augmented[pivot][k], augmented[column][k]);

        const GLdouble divisor = augmented[column][column];
        for (int k = 0; k != 8; ++k)
            augmented[column][k] /= divisor;
        for (int row = 0; row != 4; ++row) {
            if (row == column)
                continue;
            const GLdouble factor = augmented[row][column];
            for (int k = 0; k != 8; ++k)
                augmented[row][k] -= factor * augmented[column][k];
        }
    }

    for (int row = 0; row != 4; ++row)
        for (int column = 0; column != 4; ++column)
            inverse[column * 4 + row] = augmented[row][column + 4];
    return true;
}

} // namespace

void qwt3dGlBegin(GLenum mode)
{
    primitiveMode = mode;
    vertices.clear();
    glGetFloatv(GL_CURRENT_COLOR, currentColor);
    glGetFloatv(GL_CURRENT_NORMAL, currentNormal);
}

void qwt3dGlEnd()
{
    if (primitiveMode == GL_POLYGON) {
        drawVertices(GL_TRIANGLE_FAN, vertices);
    } else if (primitiveMode == GL_QUADS) {
        std::vector<ImmediateVertex> triangles;
        triangles.reserve(vertices.size() / 4 * 6);
        for (std::size_t i = 0; i + 3 < vertices.size(); i += 4) {
            triangles.push_back(vertices[i]);
            triangles.push_back(vertices[i + 1]);
            triangles.push_back(vertices[i + 2]);
            triangles.push_back(vertices[i]);
            triangles.push_back(vertices[i + 2]);
            triangles.push_back(vertices[i + 3]);
        }
        drawVertices(GL_TRIANGLES, triangles);
    } else {
        drawVertices(primitiveMode, vertices);
    }
    vertices.clear();
}

void qwt3dGlVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    ImmediateVertex vertex;
    vertex.position[0] = static_cast<GLfloat>(x);
    vertex.position[1] = static_cast<GLfloat>(y);
    vertex.position[2] = static_cast<GLfloat>(z);
    std::copy(currentNormal, currentNormal + 3, vertex.normal);
    std::copy(currentColor, currentColor + 4, vertex.color);
    vertices.push_back(vertex);
}

void qwt3dGlVertex3dv(const GLdouble *v)
{
    qwt3dGlVertex3d(v[0], v[1], v[2]);
}

void qwt3dGlNormal3d(GLdouble x, GLdouble y, GLdouble z)
{
    currentNormal[0] = static_cast<GLfloat>(x);
    currentNormal[1] = static_cast<GLfloat>(y);
    currentNormal[2] = static_cast<GLfloat>(z);
    glNormal3f(currentNormal[0], currentNormal[1], currentNormal[2]);
}

void qwt3dGlNormal3dv(const GLdouble *v)
{
    qwt3dGlNormal3d(v[0], v[1], v[2]);
}

void qwt3dGlColor3d(GLdouble r, GLdouble g, GLdouble b)
{
    qwt3dGlColor4d(r, g, b, 1.0);
}

void qwt3dGlColor4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a)
{
    currentColor[0] = static_cast<GLfloat>(r);
    currentColor[1] = static_cast<GLfloat>(g);
    currentColor[2] = static_cast<GLfloat>(b);
    currentColor[3] = static_cast<GLfloat>(a);
    glColor4f(currentColor[0], currentColor[1], currentColor[2], currentColor[3]);
}

void qwt3dGlColor4dv(const GLdouble *rgba)
{
    qwt3dGlColor4d(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void qwt3dGlGetDoublev(GLenum pname, GLdouble *params)
{
    GLfloat values[16] = {0.0f};
    glGetFloatv(pname, values);
    int count = 1;
    if (pname == GL_MODELVIEW_MATRIX || pname == GL_PROJECTION_MATRIX)
        count = 16;
    else if (pname == GL_CURRENT_COLOR || pname == GL_COLOR_CLEAR_VALUE)
        count = 4;
    for (int i = 0; i != count; ++i)
        params[i] = values[i];
}

void qwt3dGlOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                  GLdouble nearValue, GLdouble farValue)
{
    glOrthof(static_cast<GLfloat>(left), static_cast<GLfloat>(right),
             static_cast<GLfloat>(bottom), static_cast<GLfloat>(top),
             static_cast<GLfloat>(nearValue), static_cast<GLfloat>(farValue));
}

void qwt3dGlFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
                    GLdouble nearValue, GLdouble farValue)
{
    glFrustumf(static_cast<GLfloat>(left), static_cast<GLfloat>(right),
               static_cast<GLfloat>(bottom), static_cast<GLfloat>(top),
               static_cast<GLfloat>(nearValue), static_cast<GLfloat>(farValue));
}

void qwt3dGlPolygonMode(GLenum, GLenum)
{
}

void qwt3dGlLineStipple(GLint, GLushort)
{
}

void qwt3dGlRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    rasterPosition[0] = x;
    rasterPosition[1] = y;
    rasterPosition[2] = z;
}

void qwt3dGlDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type,
                       const void *pixels)
{
    if (!pixels || width <= 0 || height <= 0 || format != GL_RGBA || type != GL_UNSIGNED_BYTE)
        return;

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    qwt3dGlGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    qwt3dGlGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLdouble windowX, windowY, windowZ;
    if (!glesProject(rasterPosition[0], rasterPosition[1], rasterPosition[2], modelview,
                     projection, viewport, &windowX, &windowY, &windowZ))
        return;

    GLdouble corners[4][3];
    const GLdouble screen[4][2] = {
        {windowX, windowY},
        {windowX + width, windowY},
        {windowX, windowY + height},
        {windowX + width, windowY + height}
    };
    for (int i = 0; i != 4; ++i) {
        if (!glesUnProject(screen[i][0], screen[i][1], windowZ, modelview, projection,
                           viewport, &corners[i][0], &corners[i][1], &corners[i][2]))
            return;
    }

    GLsizei textureWidth = 1;
    GLsizei textureHeight = 1;
    while (textureWidth < width)
        textureWidth *= 2;
    while (textureHeight < height)
        textureHeight *= 2;
    std::vector<GLubyte> texturePixels(textureWidth * textureHeight * 4, 0);
    const GLubyte *source = static_cast<const GLubyte *>(pixels);
    for (GLsizei row = 0; row != height; ++row)
        std::copy(source + row * width * 4, source + (row + 1) * width * 4,
                  texturePixels.begin() + row * textureWidth * 4);

    const GLfloat positions[12] = {
        static_cast<GLfloat>(corners[0][0]), static_cast<GLfloat>(corners[0][1]), static_cast<GLfloat>(corners[0][2]),
        static_cast<GLfloat>(corners[1][0]), static_cast<GLfloat>(corners[1][1]), static_cast<GLfloat>(corners[1][2]),
        static_cast<GLfloat>(corners[2][0]), static_cast<GLfloat>(corners[2][1]), static_cast<GLfloat>(corners[2][2]),
        static_cast<GLfloat>(corners[3][0]), static_cast<GLfloat>(corners[3][1]), static_cast<GLfloat>(corners[3][2])
    };
    const GLfloat maxS = static_cast<GLfloat>(width) / textureWidth;
    const GLfloat maxT = static_cast<GLfloat>(height) / textureHeight;
    const GLfloat texcoords[8] = {0.0f, 0.0f, maxS, 0.0f, 0.0f, maxT, maxS, maxT};

    const GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);
    const GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    const GLboolean vertexArray = glIsEnabled(GL_VERTEX_ARRAY);
    const GLboolean textureArray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
    const GLboolean colorArray = glIsEnabled(GL_COLOR_ARRAY);
    GLint savedTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &savedTexture);
    GLfloat savedColor[4];
    glGetFloatv(GL_CURRENT_COLOR, savedColor);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, &texturePixels[0]);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, positions);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (colorArray)
        glEnableClientState(GL_COLOR_ARRAY);
    if (!textureArray)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    if (!vertexArray)
        glDisableClientState(GL_VERTEX_ARRAY);
    if (lightingEnabled)
        glEnable(GL_LIGHTING);
    if (!textureEnabled)
        glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(savedTexture));
    glColor4f(savedColor[0], savedColor[1], savedColor[2], savedColor[3]);
    std::copy(savedColor, savedColor + 4, currentColor);
}

void qwt3dGlCylinder(GLdouble baseRadius, GLdouble topRadius, GLdouble height, GLint slices)
{
    slices = std::max(slices, 3);
    qwt3dGlBegin(GL_TRIANGLE_STRIP);
    for (GLint i = 0; i <= slices; ++i) {
        const GLdouble angle = 2.0 * 3.14159265358979323846 * i / slices;
        const GLdouble cosine = std::cos(angle);
        const GLdouble sine = std::sin(angle);
        qwt3dGlNormal3d(cosine, sine, 0.0);
        qwt3dGlVertex3d(baseRadius * cosine, baseRadius * sine, 0.0);
        qwt3dGlVertex3d(topRadius * cosine, topRadius * sine, height);
    }
    qwt3dGlEnd();
}

void qwt3dGlDisk(GLdouble innerRadius, GLdouble outerRadius, GLint slices)
{
    slices = std::max(slices, 3);
    qwt3dGlBegin(GL_TRIANGLE_STRIP);
    qwt3dGlNormal3d(0.0, 0.0, 1.0);
    for (GLint i = 0; i <= slices; ++i) {
        const GLdouble angle = 2.0 * 3.14159265358979323846 * i / slices;
        const GLdouble cosine = std::cos(angle);
        const GLdouble sine = std::sin(angle);
        qwt3dGlVertex3d(outerRadius * cosine, outerRadius * sine, 0.0);
        qwt3dGlVertex3d(innerRadius * cosine, innerRadius * sine, 0.0);
    }
    qwt3dGlEnd();
}

int glesProject(GLdouble objX, GLdouble objY, GLdouble objZ,
                const GLdouble *modelview, const GLdouble *projection,
                const GLint *viewport, GLdouble *winX, GLdouble *winY, GLdouble *winZ)
{
    const GLdouble input[4] = {objX, objY, objZ, 1.0};
    GLdouble eye[4];
    GLdouble clip[4];
    multiplyVector(modelview, input, eye);
    multiplyVector(projection, eye, clip);
    if (clip[3] == 0.0)
        return GL_FALSE;
    const GLdouble inverseW = 1.0 / clip[3];
    *winX = viewport[0] + (clip[0] * inverseW + 1.0) * viewport[2] / 2.0;
    *winY = viewport[1] + (clip[1] * inverseW + 1.0) * viewport[3] / 2.0;
    *winZ = (clip[2] * inverseW + 1.0) / 2.0;
    return GL_TRUE;
}

int glesUnProject(GLdouble winX, GLdouble winY, GLdouble winZ,
                  const GLdouble *modelview, const GLdouble *projection,
                  const GLint *viewport, GLdouble *objX, GLdouble *objY, GLdouble *objZ)
{
    GLdouble combined[16];
    GLdouble inverse[16];
    multiplyMatrices(projection, modelview, combined);
    if (!invertMatrix(combined, inverse) || viewport[2] == 0 || viewport[3] == 0)
        return GL_FALSE;

    const GLdouble input[4] = {
        (winX - viewport[0]) * 2.0 / viewport[2] - 1.0,
        (winY - viewport[1]) * 2.0 / viewport[3] - 1.0,
        2.0 * winZ - 1.0,
        1.0
    };
    GLdouble output[4];
    multiplyVector(inverse, input, output);
    if (output[3] == 0.0)
        return GL_FALSE;
    *objX = output[0] / output[3];
    *objY = output[1] / output[3];
    *objZ = output[2] / output[3];
    return GL_TRUE;
}

const GLubyte *glesErrorString(GLenum error)
{
    switch (error) {
    case GL_NO_ERROR: return reinterpret_cast<const GLubyte *>("no error");
    case GL_INVALID_ENUM: return reinterpret_cast<const GLubyte *>("invalid enum");
    case GL_INVALID_VALUE: return reinterpret_cast<const GLubyte *>("invalid value");
    case GL_INVALID_OPERATION: return reinterpret_cast<const GLubyte *>("invalid operation");
    case GL_OUT_OF_MEMORY: return reinterpret_cast<const GLubyte *>("out of memory");
    default: return reinterpret_cast<const GLubyte *>("unknown GLES error");
    }
}

#endif // HAVE_GLES
