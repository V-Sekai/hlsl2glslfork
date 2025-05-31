#include "unit_tests_common.h"
#include <iostream>
#include <fstream>
using namespace ::testing;

namespace {

class Hlsl2GlslTest : public ::testing::Test
{
public:
    ETargetVersion targetVersion { ETargetGLSL_ES_100 };
    std::array<ShHandle, EShLangCount> compilerHandles {};

    void SetUp() override;
    void TearDown() override;

    [[nodiscard]] std::pair<bool, std::string> compileShader(EShLanguage type, const std::string& shaderSrc) const;
};

void Hlsl2GlslTest::SetUp()
{
    int success = Hlsl2Glsl_Initialize();
    if (!success)
    {
        throw std::runtime_error { "failed to initialize HLSL2GLSL" };
    }

    for (size_t i=0;i<compilerHandles.size();++i) {
        compilerHandles[i] = Hlsl2Glsl_ConstructCompiler(static_cast<EShLanguage>(i));
    }
}

void Hlsl2GlslTest::TearDown()
{
    for (auto& hnd : compilerHandles) {
        Hlsl2Glsl_DestructCompiler(hnd);
        hnd = nullptr;
    }

    Hlsl2Glsl_Shutdown();
}

bool C_DECL file_Hlsl2Glsl_IncludeOpenFunc(bool isSystem, const char* fname, const char* parentfname, const char* parent, std::string& output, void* data) {
    std::ifstream vt(fname);
    if (!vt) {
        std::string fullfname = std::string("CGIncludes/") + fname;
        vt = std::ifstream(fullfname.c_str());
    }
    std::stringstream vbuffer;
    vbuffer << vt.rdbuf();
    output = vbuffer.str();
    return true;
}

void C_DECL file_Hlsl2Glsl_IncludeCloseFunc(const char* file, void* data) {
}


std::pair<bool, std::string> Hlsl2GlslTest::compileShader(EShLanguage type, const std::string& shaderSrc) const
{
    Hlsl2Glsl_ParseCallbacks parseCallb;
    parseCallb.includeOpenCallback = &file_Hlsl2Glsl_IncludeOpenFunc;
    parseCallb.includeCloseCallback = &file_Hlsl2Glsl_IncludeCloseFunc;
    parseCallb.data = nullptr;

    unsigned options = 0;
    int parseOk = Hlsl2Glsl_Parse (compilerHandles[type], shaderSrc.c_str(),
        targetVersion, &parseCallb, options);
    std::string infoLog = Hlsl2Glsl_GetInfoLog(compilerHandles[type]);
    if (parseOk) {
        int translateOk = Hlsl2Glsl_Translate (compilerHandles[type], "main", targetVersion, options);
        infoLog = Hlsl2Glsl_GetInfoLog(compilerHandles[type]);
        if (translateOk) {
            std::string text = GetCompiledShaderText(compilerHandles[type]);
            return std::make_pair(true, text);
        }
    }
    return std::make_pair(false, infoLog);
}

// NOLINTNEXTLINE
TEST_F(Hlsl2GlslTest, VertexShaderES3)
{
    targetVersion = ETargetGLSL_ES_300;
    std::ifstream vt("vert.hlsl");
    std::stringstream vbuffer;
    vbuffer << vt.rdbuf();
    std::string vshaderSrc = vbuffer.str();
    std::pair<bool, std::string> res = compileShader(VERTEX_SHADER, vshaderSrc);
    std::cout << "// vert " << res.first << std::endl << res.second << std::endl;
}

// NOLINTNEXTLINE
TEST_F(Hlsl2GlslTest, FragmentShaderES3)
{
    targetVersion = ETargetGLSL_ES_300;
    std::ifstream vt("frag.hlsl");
    std::stringstream vbuffer;
    vbuffer << vt.rdbuf();
    std::string vshaderSrc = vbuffer.str();
    std::pair<bool, std::string> res = compileShader(FRAGMENT_SHADER, vshaderSrc);
    std::cout << "// frag " << res.first << std::endl << res.second << std::endl;
}

} // namespace
