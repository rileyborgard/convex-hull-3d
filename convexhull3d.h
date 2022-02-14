#ifndef CONVEXHULL3D_H
#define CONVEXHULL3D_H

#include <QVector3D>
#include <vector>
#include "mesh.h"

// A face is represented by the indices of its three points a, b, c.
// It also stores an outward-facing normal vector
struct Face {
    int a, b, c;
    QVector3D normal;
    Face(int a, int b, int c, QVector3D normal) : a(a), b(b), c(c), normal(normal) {}
};

class ConvexHull3D
{
public:
    static Mesh* compute(std::vector<QVector3D> points);
};

#endif // CONVEXHULL3D_H
