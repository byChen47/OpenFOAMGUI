#ifndef OFMESHREADER_H
#define OFMESHREADER_H

#include <QString>
#include <QVector>
#include <QVector3D>
#include <QMap>

struct OFMeshData {
    QVector<QVector3D>     vertices;     // Point coordinates
    QVector<QVector<uint>> faces;        // Face connectivity (indices into vertices)
    QVector<QVector3D>     faceColors;   // Color per face (based on patch)
    QMap<QString, QVector<uint>> patches; // Patch name → face indices
    double bounds[6];                     // xmin, xmax, ymin, ymax, zmin, zmax
};

class OFMeshReader
{
public:
    OFMeshReader();

    // Parse OpenFOAM polyMesh directory
    bool readMesh(const QString &casePath, OFMeshData &mesh);

    // Parse a single field for cell/point data coloring
    bool readField(const QString &fieldPath, OFMeshData &mesh);

    QString lastError() const { return m_error; }

private:
    bool readPoints(const QString &filePath, QVector<QVector3D> &points);
    bool readFaces(const QString &filePath, QVector<QVector<uint>> &faces);
    bool readBoundary(const QString &filePath,
                      QVector<QVector<uint>> &faces,
                      QVector<QVector3D> &colors,
                      QMap<QString, QVector<uint>> &patches);

    static QVector3D patchColor(const QString &patchType);
    void computeBounds(OFMeshData &mesh);

    QString m_error;
};

#endif // OFMESHREADER_H
