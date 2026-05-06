#ifndef LANGUAGEDETECTOR_H
#define LANGUAGEDETECTOR_H

#include <QString>
#include <QStringList>

enum class FileLanguage {
    Unknown,
    OpenFOAM,       // OpenFOAM dictionary / field files (FoamFile header)
    Cpp,            // C++ source (.C, .cpp, .cxx, .cc)
    CppHeader,      // C++ header (.H, .h, .hpp, .hxx)
    C,              // C source (.c)
    Bash,           // Shell scripts (.sh, .bash, Allrun, Allclean)
    Python,         // Python (.py)
    CMake,          // CMakeLists.txt, .cmake
    JSON,           // .json
    XML,            // .xml
    PlainText       // fallback
};

class LanguageDetector
{
public:
    LanguageDetector() = default;

    // Full detection: extension + name + content
    static FileLanguage detect(const QString &filePath,
                               const QString &contentPreview = QString());

    // Extension-only quick check
    static FileLanguage detectByExtension(const QString &filePath);

    // Content-based detection
    static FileLanguage detectByContent(const QString &content,
                                        const QString &filePath = QString());

    // Human-readable language name
    static QString languageName(FileLanguage lang);

    // Get common extensions for a language
    static QStringList extensions(FileLanguage lang);

private:
    static bool isOpenFOAMContent(const QString &content);
    static bool isCppContent(const QString &content);
    static bool isBashContent(const QString &content);
    static bool isPythonContent(const QString &content);
};

#endif // LANGUAGEDETECTOR_H
