#ifndef MESH_H
#define MESH_H

#include <qopengl.h>
#include <vector>
#include <QVector3D>

class Mesh {
public:
    Mesh();

    const GLfloat *constData() const { return data.data(); }
    int vertexCount() const { return data.size() / 6; }

    void addTriangle(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3);

private:
    void addVertex(const QVector3D &v);

    std::vector<GLfloat> data;
};

#endif // MESH_H
