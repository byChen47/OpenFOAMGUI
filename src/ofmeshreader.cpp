#include "ofmeshreader.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QtMath>

OFMeshReader::OFMeshReader() {}

// ── Main entry: read polyMesh directory ──
bool OFMeshReader::readMesh(const QString &casePath, OFMeshData &mesh)
{
    QString polyMeshDir = QDir(casePath).filePath("constant/polyMesh");
    if (!QFileInfo::exists(polyMeshDir)) {
        m_error = "constant/polyMesh/ not found in " + casePath;
        return false;
    }

    if (!readPoints(polyMeshDir + "/points", mesh.vertices)) return false;
    if (!readFaces(polyMeshDir + "/faces", mesh.faces)) return false;
    if (!readBoundary(polyMeshDir + "/boundary", mesh.faces,
                      mesh.faceColors, mesh.patches)) return false;

    computeBounds(mesh);
    return true;
}

// ── Read points file ──
bool OFMeshReader::readPoints(const QString &filePath, QVector<QVector3D> &points)
{
    QFile f(filePath);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        m_error = "Cannot open: " + filePath; return false;
    }
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    QRegularExpression ptRe(R"(\(\s*([-\d.e+]+)\s+([-\d.e+]+)\s+([-\d.e+]+)\s*\))");
    auto it = ptRe.globalMatch(content);
    while (it.hasNext()) {
        auto m = it.next();
        points.append(QVector3D(
            m.captured(1).toDouble(),
            m.captured(2).toDouble(),
            m.captured(3).toDouble()));
    }
    if (points.isEmpty()) {
        m_error = "No points found in: " + filePath; return false;
    }
    return true;
}

// ── Read faces file ──
bool OFMeshReader::readFaces(const QString &filePath,
                              QVector<QVector<uint>> &faces)
{
    QFile f(filePath);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        m_error = "Cannot open: " + filePath; return false;
    }
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    // Match: 4(0 1 5 4) or 3(1 2 6)
    QRegularExpression faceRe(R"((\d+)\s*\(([^)]+)\))");
    auto it = faceRe.globalMatch(content);
    while (it.hasNext()) {
        auto m = it.next();
        int nVerts = m.captured(1).toInt();
        QStringList idxStrs = m.captured(2).trimmed().split(QRegularExpression("\\s+"));
        QVector<uint> face;
        for (int i = 0; i < nVerts && i < idxStrs.size(); ++i)
            face.append(idxStrs[i].toUInt());
        if (!face.isEmpty()) faces.append(face);
    }
    return true;
}

// ── Read boundary file ──
bool OFMeshReader::readBoundary(const QString &filePath,
                                 QVector<QVector<uint>> &faces,
                                 QVector<QVector3D> &colors,
                                 QMap<QString, QVector<uint>> &patches)
{
    QFile f(filePath);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        // Boundary file is optional — assign default color if missing
        for (int i = 0; i < faces.size(); ++i)
            colors.append(QVector3D(0.7f, 0.7f, 0.7f));
        return true;
    }
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    // Parse patches: patchName { type xxx; nFaces n; startFace s; }
    QRegularExpression patchRe(
        R"((\w+)\s*\{(.*?)\n\s*\})",
        QRegularExpression::DotMatchesEverythingOption);
    auto it = patchRe.globalMatch(content);

    struct PatchInfo {
        QString name, type;
        int nFaces = 0, startFace = 0;
    };
    QVector<PatchInfo> pinfos;

    while (it.hasNext()) {
        auto pm = it.next();
        PatchInfo pi;
        pi.name = pm.captured(1);
        QString body = pm.captured(2);

        QRegularExpression typeRe(R"(type\s+(\S+)\s*;)");
        auto tm = typeRe.match(body);
        if (tm.hasMatch()) pi.type = tm.captured(1);

        QRegularExpression nfRe(R"(nFaces\s+(\d+)\s*;)");
        auto nm = nfRe.match(body);
        if (nm.hasMatch()) pi.nFaces = nm.captured(1).toInt();

        QRegularExpression sfRe(R"(startFace\s+(\d+)\s*;)");
        auto sm = sfRe.match(body);
        if (sm.hasMatch()) pi.startFace = sm.captured(1).toInt();

        pinfos.append(pi);
    }

    // Assign colors to each face based on patch
    colors.resize(faces.size());
    for (auto &c : colors) c = QVector3D(0.7f, 0.7f, 0.7f); // default gray

    for (const auto &pi : pinfos) {
        QVector<uint> patchFaces;
        for (int i = 0; i < pi.nFaces; ++i) {
            int fi = pi.startFace + i;
            if (fi < faces.size()) {
                colors[fi] = patchColor(pi.type);
                patchFaces.append(fi);
            }
        }
        patches[pi.name] = patchFaces;
    }

    return true;
}

// ── Color mapping for known patch types ──
QVector3D OFMeshReader::patchColor(const QString &patchType)
{
    QString t = patchType.toLower();
    if (t == "wall")      return QVector3D(0.7f, 0.7f, 0.7f); // gray
    if (t == "patch")     return QVector3D(0.3f, 0.6f, 1.0f); // blue
    if (t.contains("inlet"))  return QVector3D(0.2f, 0.8f, 0.2f); // green
    if (t.contains("outlet")) return QVector3D(1.0f, 0.5f, 0.0f); // orange
    if (t == "symmetry" || t == "symmetryplane")
                           return QVector3D(1.0f, 1.0f, 0.0f); // yellow
    if (t == "empty")      return QVector3D(0.5f, 0.5f, 1.0f); // light blue
    if (t == "wedge")      return QVector3D(1.0f, 0.0f, 1.0f); // magenta
    if (t == "cyclic" || t == "cyclicami")
                           return QVector3D(0.0f, 0.8f, 0.8f); // cyan
    return QVector3D(qHash(t) % 200 / 255.0f + 0.2f,
                     qHash(t + "g") % 200 / 255.0f + 0.2f,
                     qHash(t + "b") % 200 / 255.0f + 0.2f);
}

// ── Read field file (e.g. U, p, k, nut) ──
bool OFMeshReader::readField(const QString &fieldPath, OFMeshData &mesh)
{
    QFile f(fieldPath);
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        m_error = "Cannot open field: " + fieldPath; return false;
    }
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    mesh.fieldName = QFileInfo(fieldPath).fileName();
    mesh.cellValues.clear();

    // Try nonuniform list first
    QRegularExpression nonunif(R"(nonuniform\s+List\w*\s+(\d+)\s*\(([^)]*)\))");
    QRegularExpression unif(R"(uniform\s+\(?\s*([-\d.e+]+))");
    QRegularExpression unifVec(R"(uniform\s+\(([-\d.e+\s]+)\))");

    auto nm = nonunif.match(content);
    if (nm.hasMatch()) {
        QString vals = nm.captured(2);
        QTextStream ts(&vals);
        double v;
        while (!ts.atEnd()) {
            ts >> v;
            if (!ts.status()) break;
            mesh.cellValues.append(v);
        }
        return true;
    }

    auto um = unif.match(content);
    if (um.hasMatch()) {
        double v = um.captured(1).toDouble();
        mesh.cellValues.append(v); // single uniform value
        return true;
    }

    auto uvm = unifVec.match(content);
    if (uvm.hasMatch()) {
        QStringList parts = uvm.captured(1).split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        for (const auto &p : parts)
            mesh.cellValues.append(p.toDouble());
        return true;
    }

    m_error = "Could not parse field data in: " + fieldPath;
    return false;
}

// ── Compute bounding box ──
void OFMeshReader::computeBounds(OFMeshData &mesh)
{
    if (mesh.vertices.isEmpty()) return;
    float xmin = mesh.vertices[0].x(), xmax = xmin;
    float ymin = mesh.vertices[0].y(), ymax = ymin;
    float zmin = mesh.vertices[0].z(), zmax = zmin;

    for (const auto &v : mesh.vertices) {
        xmin = qMin(xmin, v.x()); xmax = qMax(xmax, v.x());
        ymin = qMin(ymin, v.y()); ymax = qMax(ymax, v.y());
        zmin = qMin(zmin, v.z()); zmax = qMax(zmax, v.z());
    }
    mesh.bounds[0] = xmin; mesh.bounds[1] = xmax;
    mesh.bounds[2] = ymin; mesh.bounds[3] = ymax;
    mesh.bounds[4] = zmin; mesh.bounds[5] = zmax;
}
