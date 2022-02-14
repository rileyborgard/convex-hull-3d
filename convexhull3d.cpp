
#include "convexhull3d.h"

Mesh* ConvexHull3D::compute(std::vector<QVector3D> points) {
    int n = points.size();

    const double EPS = 1e-6;

    // First, order the points so that the first 4 create a non-degenerate tetrahedron
    std::vector<int> ve;
    ve.push_back(0);
    for (int i = 1; i < n && ve.size() < 4; i++) {
        if ((ve.size() == 1 && points[ve[0]].distanceToPoint(points[i]) > EPS) ||
                (ve.size() == 2 && QVector3D::crossProduct(points[ve[1]] - points[ve[0]], points[i] - points[ve[0]]).length() > EPS) ||
                (ve.size() == 3 && std::abs(QVector3D::dotProduct(points[i] - points[ve[0]], QVector3D::crossProduct(points[ve[1]] - points[ve[0]], points[ve[2]] - points[ve[0]]))) > EPS)) {
            ve.push_back(i);
        }
    }
    Q_ASSERT(ve.size() == 4);
    std::vector<QVector3D> myPoints;
    for (int i : ve) {
        myPoints.push_back(points[i]);
    }
    std::reverse(ve.begin(), ve.end());
    for(int i : ve) {
        points.erase(points.begin() + i);
    }
    points.insert(points.begin(), myPoints.begin(), myPoints.end());

    std::vector<Face> faces;

    // Consider an edge (a->b) dead if it is not a CCW edge of some current face
    // If an edge is alive but not its reverse, this is an exposed edge.
    // We should add new faces on the exposed edges.
    std::vector<std::vector<bool>> dead(n, std::vector<bool>(n, true));

    auto addFace = [&](int a, int b, int c) {
        faces.push_back(Face(a, b, c, QVector3D::normal(points[b] - points[a], points[c] - points[a])));
        dead[a][b] = dead[b][c] = dead[c][a] = false;
    };

    // Initialize the convex hull of the first 3 points as a
    // triangular disk with two faces of opposite orientation
    addFace(0, 1, 2);
    addFace(0, 2, 1);

    for (int i = 3; i < n; i++) {
        // faces2 will be the list of faces invisible to the added points[i]
        std::vector<Face> faces2;
        for(Face &f : faces) {
            if(QVector3D::dotProduct(points[i] - points[f.a], f.normal) > EPS) {
                // this face is visible to the new point, so mark its edges as dead
                dead[f.a][f.b] = dead[f.b][f.c] = dead[f.c][f.a] = true;
            }else {
                faces2.push_back(f);
            }
        }
        // Add a new face for each exposed edge.
        // Only check edges of alive faces for being exposed.
        faces.clear();
        for(Face &F : faces2) {
            int arr[3] = {F.a, F.b, F.c};
            for (int j = 0; j < 3; j++) {
                int a = arr[j], b = arr[(j + 1) % 3];
                if (dead[b][a]) {
                    addFace(b, a, i);
                }
            }
        }
        faces.insert(faces.end(), faces2.begin(), faces2.end());
    }

    // convert list of faces to a mesh object
    Mesh *mesh = new Mesh;
    for (Face &f : faces) {
        mesh->addTriangle(points[f.a], points[f.b], points[f.c]);
    }
    return mesh;
}
