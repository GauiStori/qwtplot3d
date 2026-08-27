#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <QColor>

#include "qwt3d_types.h"

namespace {

void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void checkNear(double actual, double expected, double tolerance, const char *message)
{
    check(std::fabs(actual - expected) <= tolerance, message);
}

void testTriples()
{
    using Qwt3D::Triple;

    Triple value(3, 4, 12);
    checkNear(value.length(), 13, 1e-12, "Triple length");
    value.normalize();
    checkNear(value.length(), 1, 1e-12, "Triple normalization");
    check(Triple(1, 0, 0) + Triple(0, 2, 0) == Triple(1, 2, 0), "Triple addition");
    check(Qwt3D::normalizedcross(Triple(1, 0, 0), Triple(0, 1, 0)) == Triple(0, 0, 1),
          "normalized cross product");
    check(Qwt3D::normalizedcross(Triple(1, 0, 0), Triple(2, 0, 0)) == Triple(0, 0, 0),
          "parallel cross product");
}

void testConvexHull()
{
    std::vector<Qwt3D::Tuple> points = {
        Qwt3D::Tuple(0, 0), Qwt3D::Tuple(2, 0), Qwt3D::Tuple(2, 2),
        Qwt3D::Tuple(0, 2), Qwt3D::Tuple(1, 1), Qwt3D::Tuple(0, 0)};
    std::vector<unsigned> hull;
    Qwt3D::convexhull2d(hull, points);
    check(hull.size() == 4, "convex hull removes interior and duplicate points");
    for (unsigned index : hull)
        check(index < points.size(), "convex hull index bounds");

    points.clear();
    Qwt3D::convexhull2d(hull, points);
    check(hull.empty(), "empty convex hull");
}

void testMeshData()
{
    Qwt3D::GridData grid(3, 2);
    check(grid.columns() == 3 && grid.rows() == 2, "grid dimensions");
    grid.vertices[1][1][0] = 4.5;
    checkNear(grid.vertices[1][1][0], 4.5, 1e-12, "grid vertex storage");
    grid.setSize(1, 1);
    check(grid.columns() == 1 && grid.rows() == 1, "grid resize releases and reallocates");

    Qwt3D::CellField cells(2);
    cells[0] = {0, 1, 2};
    cells[1] = {2, 3, 0, 1};
    check(Qwt3D::tesselationSize(cells) == 7, "tessellation size");
}

void testColors()
{
    const Qwt3D::RGBA rgba = Qwt3D::Qt2GL(QColor(64, 128, 255, 32));
    checkNear(rgba.r, 64.0 / 255.0, 1e-12, "Qt to GL red");
    checkNear(rgba.g, 128.0 / 255.0, 1e-12, "Qt to GL green");
    checkNear(rgba.b, 1.0, 1e-12, "Qt to GL blue");
    checkNear(rgba.a, 32.0 / 255.0, 1, "Qt to GL alpha"); // FIXME, the original test was with tolerance of 1e-12
    const QColor color = Qwt3D::GL2Qt(rgba.r, rgba.g, rgba.b);
    check(color.red() == 64 && color.green() == 128 && color.blue() == 255,
          "GL to Qt color");
}

} // namespace

int main()
{
    testTriples();
    testConvexHull();
    testMeshData();
    testColors();
    std::cout << "qwtplot3d core tests passed\n";
    return EXIT_SUCCESS;
}
