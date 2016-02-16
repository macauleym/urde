#include "CElementGenShaders.hpp"
#include "CElementGen.hpp"
#include "CGenDescription.hpp"

namespace pshag
{

static const char* VS_GLSL_TEX =
"#version 330\n"
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 colorIn;\n"
"layout(location=5) in vec2 uvsIn[4];\n"
"\n"
"uniform ParticleUniform\n"
"{\n"
"    mat4 mvp;\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    vtf.uv = uvsIn[gl_VertexID];\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"    gl_Position = vec4(posIn[gl_VertexID].x, posIn[gl_VertexID].z, 0.0, 1.0);\n"
"}\n";

static const char* FS_GLSL_TEX =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"uniform sampler2D texs[1];\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(texs[0], vtf.uv);\n"
"    colorOut = vtf.color;\n"
"}\n";

static const char* FS_GLSL_TEX_REDTOALPHA =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"uniform sampler2D texs[1];\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(texs[0], vtf.uv);\n"
"    colorOut.a = colorOut.r;\n"
"    colorOut = vec4(1.0,1.0,1.0,1.0);\n"
"}\n";

static const char* VS_GLSL_INDTEX =
"#version 330\n"
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 colorIn;\n"
"layout(location=5) in vec4 uvsInTexrTind[4];\n"
"layout(location=9) in vec2 uvsInScene[4];\n"
"\n"
"uniform ParticleUniform\n"
"{\n"
"    mat4 mvp;\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uvTexr;\n"
"    vec2 uvScene;\n"
"    vec2 uvTind;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    vtf.uvTexr = uvsInTexrTind[gl_VertexID].xy;\n"
"    vtf.uvScene = uvsInScene[gl_VertexID];\n"
"    vtf.uvTind = uvsInTexrTind[gl_VertexID].zw;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"}\n";

static const char* FS_GLSL_INDTEX =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uvTexr;\n"
"    vec2 uvScene;\n"
"    vec2 uvTind;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"uniform sampler2D texs[3];\n"
"void main()\n"
"{\n"
"    vec2 tindTexel = texture(texs[2], vtf.uvTind).xy;\n"
"    vec4 sceneTexel = texture(texs[1], vtf.uvScene + tindTexel);\n"
"    vec4 texrTexel = texture(texs[0], vtf.uvTexr);\n"
"    colorOut = vtf.color * sceneTexel + texrTexel;\n"
"    colorOut.a = vtf.color.a * texrTexel.a;"
"    colorOut = vec4(1.0,1.0,1.0,1.0);\n"
"}\n";

static const char* FS_GLSL_CINDTEX =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uvTexr;\n"
"    vec2 uvScene;\n"
"    vec2 uvTind;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"uniform sampler2D texs[3];\n"
"void main()\n"
"{\n"
"    vec2 tindTexel = texture(texs[2], vtf.uvTind).xy;\n"
"    vec4 sceneTexel = texture(texs[1], vtf.uvScene + tindTexel);\n"
"    colorOut = vtf.color * sceneTexel * texture(texs[0], vtf.uvTexr);\n"
"    colorOut = vec4(1.0,1.0,1.0,1.0);\n"
"}\n";

static const char* VS_GLSL_NOTEX =
"#version 330\n"
"layout(location=0) in vec4 posIn[4];\n"
"layout(location=4) in vec4 colorIn;\n"
"\n"
"uniform ParticleUniform\n"
"{\n"
"    mat4 mvp;\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    gl_Position = mvp * posIn[gl_VertexID];\n"
"}\n";

static const char* FS_GLSL_NOTEX =
"#version 330\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"    colorOut = vec4(1.0,1.0,1.0,1.0);\n"
"}\n";

struct MetalDataBindingFactory : CElementGenShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(CElementGen& gen,
                                boo::IShaderPipeline* regPipeline,
                                boo::IShaderPipeline* redToAlphaPipeline)
    {
        CGenDescription* desc = gen.GetDesc();

        boo::IVertexFormat* vtxFmt = nullptr;
        CUVElement* texr = desc->x54_TEXR.get();
        CUVElement* tind = desc->x58_TIND.get();
        int texCount = 0;
        boo::ITexture* textures[3];

        if (texr)
        {
            textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
            texCount = 1;
            if (tind)
            {
                textures[1] = CGraphics::g_SpareTexture;
                textures[2] = tind->GetValueTexture(0).GetObj()->GetBooTexture();
                texCount = 3;

                const boo::VertexElementDescriptor TexFmtIndTex[] =
                {
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 4},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 5},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 6},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 7}
                };
                vtxFmt = CGraphics::g_BooFactory->newVertexFormat(13, TexFmtIndTex);
            }
            else
            {
                const boo::VertexElementDescriptor TexFmtTex[] =
                {
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {gen.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3}
                };
                vtxFmt = CGraphics::g_BooFactory->newVertexFormat(9, TexFmtTex);
            }
        }
        else
        {
            const boo::VertexElementDescriptor TexFmtNoTex[] =
            {
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                {gen.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
            };
            vtxFmt = CGraphics::g_BooFactory->newVertexFormat(5, TexFmtNoTex);
        }

        boo::IGraphicsBuffer* uniforms[] = {gen.m_uniformBuf};

        if (regPipeline)
            gen.m_normalDataBind = CGraphics::g_BooFactory->newShaderDataBinding(regPipeline, vtxFmt, nullptr,
                                                                                 gen.m_instBuf, nullptr, 1, uniforms,
                                                                                 texCount, textures);
        if (redToAlphaPipeline)
            gen.m_redToAlphaDataBind = CGraphics::g_BooFactory->newShaderDataBinding(redToAlphaPipeline, vtxFmt, nullptr,
                                                                                     gen.m_instBuf, nullptr, 1, uniforms,
                                                                                     texCount, textures);
    }
};

CElementGenShaders::IDataBindingFactory* CElementGenShaders::Initialize(boo::GLDataFactory& factory)
{
    static const char* UniNames[] = {"ParticleUniform"};

    m_texZTestZWrite = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 true, true, false);
    m_texNoZTestZWrite = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   false, true, false);
    m_texZTestNoZWrite = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   true, false, false);
    m_texNoZTestNoZWrite = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     false, false, false);

    m_texAdditiveZTest = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                   true, false, false);
    m_texAdditiveNoZTest = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     false, false, false);

    m_texRedToAlphaZTest = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, "texs", 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     true, false, false);
    m_texRedToAlphaNoZTest = factory.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX_REDTOALPHA, 1, "texs", 1, UniNames,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       false, false, false);

    m_indTexZWrite = factory.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, "texs", 1, UniNames,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               true, true, false);
    m_indTexNoZWrite = factory.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, "texs", 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 true, false, false);
    m_indTexAdditive = factory.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_INDTEX, 3, "texs", 1, UniNames,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 true, true, false);

    m_cindTexZWrite = factory.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, "texs", 1, UniNames,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                true, true, false);
    m_cindTexNoZWrite = factory.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, "texs", 1, UniNames,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                  true, false, false);
    m_cindTexAdditive = factory.newShaderPipeline(VS_GLSL_INDTEX, FS_GLSL_CINDTEX, 3, "texs", 1, UniNames,
                                                  boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                  true, true, false);

    m_noTexZTestZWrite = factory.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                   boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                   true, true, false);
    m_noTexNoZTestZWrite = factory.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     false, true, false);
    m_noTexZTestNoZWrite = factory.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                     true, false, false);
    m_noTexNoZTestNoZWrite = factory.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                       false, false, false);

    m_noTexAdditiveZTest = factory.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                     boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                     true, false, false);
    m_noTexAdditiveNoZTest = factory.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                       false, false, false);

    return new struct MetalDataBindingFactory;
}

}