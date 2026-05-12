#include <cstdio>
#include "../src/ofparser.h"
#include "../src/languagedetector.h"

int main()
{
    int failed = 0;
    auto check = [&](bool cond, const char *name) {
        if (!cond) { printf("FAIL: %s\n", name); failed++; }
        else printf("PASS: %s\n", name);
    };

    // ═══════════ OFParser ═══════════
    check(OFParser::fileDescription("U") == "Velocity (volVectorField)", "ofparser: U");
    check(OFParser::fileDescription("p") == "Pressure (volScalarField)", "ofparser: p");
    check(OFParser::fileDescription("blockMeshDict") == "blockMesh mesh generator input", "ofparser: blockMeshDict");
    check(OFParser::fileDescription("nonexistent").isEmpty(), "ofparser: unknown → empty");

    auto h = OFParser::parseHeader(
        "FoamFile\n{\n    version     2.0;\n    format      ascii;\n"
        "    class       volScalarField;\n    object      p;\n}\n");
    check(h.version == "2.0", "ofparser: version");
    check(h.format == "ascii", "ofparser: format");
    check(h.foamClass == "volScalarField", "ofparser: class");
    check(h.object == "p", "ofparser: object");
    check(OFParser::parseHeader("no header").version.isEmpty(), "ofparser: missing header");

    // ═══════════ LanguageDetector ═══════════
    // Extension-based (use full paths for proper detection)
    check(LanguageDetector::detectByExtension("src/main.cpp") == FileLanguage::Cpp, "lang: .cpp");
    check(LanguageDetector::detectByExtension("src/header.h") == FileLanguage::CppHeader, "lang: .h");
    check(LanguageDetector::detectByExtension("script.py") == FileLanguage::Python, "lang: .py");
    check(LanguageDetector::detectByExtension("run.sh") == FileLanguage::Bash, "lang: .sh");

    // Name-based (extensionless)
    check(LanguageDetector::detect("Allrun", "") == FileLanguage::Bash, "lang: Allrun");
    check(LanguageDetector::detect("Allclean", "") == FileLanguage::Bash, "lang: Allclean");

    // Content-based OF detection with path context
    QString ofContent = "FoamFile\n{\n    version 2.0;\n    class volScalarField;\n}\n";
    check(LanguageDetector::detectByContent(ofContent) == FileLanguage::OpenFOAM, "lang: FoamFile content");

    // OF file detection via content (extensionless files with FoamFile header)
    check(LanguageDetector::detect("blockMeshDict", ofContent) == FileLanguage::OpenFOAM, "lang: blockMeshDict+content");
    check(LanguageDetector::detect("U", ofContent) == FileLanguage::OpenFOAM, "lang: U+content");

    printf("\n=== %d/19 tests passed ===\n", 19 - failed, 19);
    return failed ? 1 : 0;
}
