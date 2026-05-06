#include "languagedetector.h"
#include <QFileInfo>
#include <QRegularExpression>

// ────────────────────────────────────────────────────────────────────
// Extension → Language mapping
// ────────────────────────────────────────────────────────────────────

static const struct {
    const char **exts;
    int count;
    FileLanguage lang;
} extMap[] = {
    // C++ source
    { (const char*[]){"C", "cpp", "cxx", "cc", "c++"}, 5, FileLanguage::Cpp },
    // C++ header
    { (const char*[]){"H", "h", "hpp", "hxx", "hh", "h++"}, 6, FileLanguage::CppHeader },
    // C source
    { (const char*[]){"c"}, 1, FileLanguage::C },
    // Bash
    { (const char*[]){"sh", "bash", "zsh", "ksh"}, 4, FileLanguage::Bash },
    // Python
    { (const char*[]){"py", "pyw", "pyx"}, 3, FileLanguage::Python },
    // CMake
    { (const char*[]){"cmake"}, 1, FileLanguage::CMake },
    // JSON
    { (const char*[]){"json", "jsonc"}, 2, FileLanguage::JSON },
    // XML
    { (const char*[]){"xml", "svg", "html", "htm"}, 4, FileLanguage::XML },
};

// Special filenames (no extension) that imply a specific language
static const struct {
    const char *name;
    FileLanguage lang;
} nameMap[] = {
    { "Allrun",      FileLanguage::Bash },
    { "Allclean",    FileLanguage::Bash },
    { "Allwmake",    FileLanguage::Bash },
    { "Alltest",     FileLanguage::Bash },
    { "Allcollect",  FileLanguage::Bash },
    { "CMakeLists",  FileLanguage::CMake },
    { "Makefile",    FileLanguage::Bash },    // Makefile -> Bash highlighting (close enough)
    { "GNUmakefile", FileLanguage::Bash },
    { "makefile",    FileLanguage::Bash },
    { "Dockerfile",  FileLanguage::Bash },
    { ".bashrc",     FileLanguage::Bash },
    { ".profile",    FileLanguage::Bash },
};

FileLanguage LanguageDetector::detectByExtension(const QString &filePath)
{
    QFileInfo fi(filePath);
    QString suffix = fi.suffix();

    // Extension-based
    if (!suffix.isEmpty()) {
        QString lower = suffix.toLower();
        for (const auto &m : extMap) {
            for (int i = 0; i < m.count; ++i) {
                if (lower == QString::fromLatin1(m.exts[i]).toLower())
                    return m.lang;
            }
        }
        // Additional common extensions
        if (lower == "stl" || lower == "obj" || lower == "msh"
            || lower == "gmsh" || lower == "foam" || lower == "csv"
            || lower == "dat" || lower == "txt" || lower == "log"
            || lower == "out" || lower == "gz" || lower == "tar"
            || lower == "png" || lower == "jpg" || lower == "svg")
            return FileLanguage::PlainText;
    }

    // Name-based (no extension or known script names)
    QString base = fi.completeBaseName();
    // Handle hidden files (e.g., .bashrc, .profile)
    QString fileName = fi.fileName();
    for (const auto &m : nameMap) {
        QString n = QString::fromLatin1(m.name);
        if (base == n || fileName == n)
            return m.lang;
    }

    // Files like "Allrun.pre", "Allrun.post" etc
    if (base.startsWith("Allrun") || base.startsWith("Allclean")
        || base.startsWith("Allwmake"))
        return FileLanguage::Bash;

    return FileLanguage::Unknown;
}

// ────────────────────────────────────────────────────────────────────
// Content-based detection (used for extensionless files)
// ────────────────────────────────────────────────────────────────────

bool LanguageDetector::isOpenFOAMContent(const QString &content)
{
    if (content.contains("FoamFile"))
        return true;
    // Look for OpenFOAM header comment block
    if (content.contains("OpenFOAM: The Open Source CFD Toolbox"))
        return true;
    if (content.contains("=========") && content.contains("Field"))
        return true;
    return false;
}

bool LanguageDetector::isCppContent(const QString &content)
{
    // C++ #include, #define, using namespace, etc.
    if (content.contains(QRegularExpression(R"(#include\s*[<\"])")))
        return true;
    if (content.contains(QRegularExpression(R"(\bnamespace\s+\w+)")))
        return true;
    if (content.contains(QRegularExpression(R"(\bstd::\w+)")))
        return true;
    if (content.contains(QRegularExpression(R"(\b(class|struct|enum)\s+\w+)")))
        return true;
    return false;
}

bool LanguageDetector::isBashContent(const QString &content)
{
    // Shebang
    if (content.startsWith("#!/"))
        return true;
    // Common shell patterns
    if (content.contains(QRegularExpression(R"(^\s*(if|then|else|elif|fi|for|while|do|done|case|esac)\b)",
                                            QRegularExpression::MultilineOption)))
        return true;
    return false;
}

bool LanguageDetector::isPythonContent(const QString &content)
{
    if (content.startsWith("#!/") && content.contains("python"))
        return true;
    if (content.contains(QRegularExpression(R"(^\s*(import|from)\s+\w+)",
                                            QRegularExpression::MultilineOption)))
        return true;
    if (content.contains(QRegularExpression(R"(^\s*def\s+\w+\s*\()",
                                            QRegularExpression::MultilineOption)))
        return true;
    if (content.contains(QRegularExpression(R"(^\s*class\s+\w+\s*[:(])",
                                            QRegularExpression::MultilineOption)))
        return true;
    return false;
}

FileLanguage LanguageDetector::detectByContent(const QString &content,
                                                const QString &filePath)
{
    // Use file path context for better detection
    if (!filePath.isEmpty()) {
        // If file is in 0/, 0.orig/, constant/, system/ — likely OpenFOAM
        QString path = filePath;
        path.replace('\\', '/');
        QStringList parts = path.split('/');
        for (int i = 0; i < parts.size(); ++i) {
            const auto &p = parts[i];
            if (p == "0" || p.startsWith("0.") || p == "constant" || p == "system") {
                // Check if next part is a file (i+1 is the filename)
                if (!content.isEmpty() && isOpenFOAMContent(content))
                    return FileLanguage::OpenFOAM;
                if (content.isEmpty())
                    return FileLanguage::OpenFOAM; // heuristic
                break;
            }
            // Check if part is a time directory (number)
            bool isNum = false;
            p.toDouble(&isNum);
            if (isNum) {
                if (!content.isEmpty() && isOpenFOAMContent(content))
                    return FileLanguage::OpenFOAM;
                if (content.isEmpty())
                    return FileLanguage::OpenFOAM;
                break;
            }
        }
    }

    if (content.isEmpty())
        return FileLanguage::Unknown;

    // Check in priority order
    if (isOpenFOAMContent(content))
        return FileLanguage::OpenFOAM;
    if (isBashContent(content))
        return FileLanguage::Bash;
    if (isPythonContent(content))
        return FileLanguage::Python;
    if (isCppContent(content))
        return FileLanguage::Cpp;

    return FileLanguage::Unknown;
}

// ────────────────────────────────────────────────────────────────────
// Full detection
// ────────────────────────────────────────────────────────────────────

FileLanguage LanguageDetector::detect(const QString &filePath,
                                       const QString &contentPreview)
{
    // 1. Try extension/name first
    FileLanguage extLang = detectByExtension(filePath);

    // 2. If unambiguous from extension, return it
    if (extLang != FileLanguage::Unknown)
        return extLang;

    // 3. Extensionless — use content
    if (!contentPreview.isEmpty())
        return detectByContent(contentPreview, filePath);

    // 4. No content available — use file path heuristics
    return detectByContent(QString(), filePath);
}

// ────────────────────────────────────────────────────────────────────
// Display names
// ────────────────────────────────────────────────────────────────────

QString LanguageDetector::languageName(FileLanguage lang)
{
    switch (lang) {
    case FileLanguage::OpenFOAM:   return "OpenFOAM";
    case FileLanguage::Cpp:        return "C++";
    case FileLanguage::CppHeader:  return "C++ Header";
    case FileLanguage::C:          return "C";
    case FileLanguage::Bash:       return "Bash";
    case FileLanguage::Python:     return "Python";
    case FileLanguage::CMake:      return "CMake";
    case FileLanguage::JSON:       return "JSON";
    case FileLanguage::XML:        return "XML";
    case FileLanguage::PlainText:  return "Plain Text";
    case FileLanguage::Unknown:    return "Unknown";
    }
    return "Unknown";
}

QStringList LanguageDetector::extensions(FileLanguage lang)
{
    switch (lang) {
    case FileLanguage::Cpp:
        return {"C", "cpp", "cxx", "cc", "c++"};
    case FileLanguage::CppHeader:
        return {"H", "h", "hpp", "hxx", "hh"};
    case FileLanguage::C:
        return {"c"};
    case FileLanguage::Bash:
        return {"sh", "bash", "zsh"};
    case FileLanguage::Python:
        return {"py", "pyw", "pyx"};
    case FileLanguage::CMake:
        return {"cmake"};
    case FileLanguage::JSON:
        return {"json"};
    default:
        return {};
    }
}
