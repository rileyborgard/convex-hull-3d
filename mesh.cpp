#include "mesh.h"

Mesh::Mesh() {

}

void Mesh::addTriangle(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3) {
    QVector3D normal = QVector3D::normal(v2 - v1, v3 - v1);
    addVertex(v1);  addVertex(normal);
    addVertex(v2);  addVertex(normal);
    addVertex(v3);  addVertex(normal);
}

void Mesh::addVertex(const QVector3D &v)
{
    data.push_back(v.x());
    data.push_back(v.y());
    data.push_back(v.z());
}
