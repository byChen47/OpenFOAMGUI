#ifndef OFPARSER_H
#define OFPARSER_H

#include <QString>
#include <QMap>
#include <QVariant>

struct FoamFileHeader {
    QString version;
    QString format;
    QString foamClass;
    QString object;
    QString location;
    QString note;
};

class OFParser
{
public:
    OFParser() = default;

    static FoamFileHeader parseHeader(const QString &content);
    static bool isOpenFOAMFile(const QString &content);
    static QString guessFileCategory(const FoamFileHeader &header,
                                     const QString &relativePath);

    // Common OpenFOAM file descriptions for tooltips / status bar
    static QString fileDescription(const QString &fileName);
};

#endif // OFPARSER_H
