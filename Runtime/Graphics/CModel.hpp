#ifndef __URDE_CMODEL_HPP__
#define __URDE_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "CFactoryMgr.hpp"
#include "CToken.hpp"
#include "zeus/CAABox.hpp"
#include "DNACommon/CMDL.hpp"
#include "DNAMP1/CMDLMaterials.hpp"
#include "Shaders/CModelShaders.hpp"

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{
class IObjectStore;
class CTexture;
class CLight;
class CSkinRules;
class CPoseAsTransforms;
class CModel;

struct CModelFlags
{
    u8 x0_blendMode = 0; /* >6: additive, >4: blend, else opaque */
    u8 x1_matSetIdx = 0;
    EExtendedShader m_extendedShader = EExtendedShader::Flat;
    u16 x2_flags = 0; /* Flags */
    zeus::CColor x4_color; /* Set into kcolor slot specified by material */
    zeus::CColor addColor = zeus::CColor::skClear;
    zeus::CAABox mbShadowBox;

    CModelFlags() = default;
    CModelFlags(u8 blendMode, u8 shadIdx, u16 flags, const zeus::CColor& col)
    : x0_blendMode(blendMode), x1_matSetIdx(shadIdx), x2_flags(flags), x4_color(col)
    {
        if (blendMode > 6)
            m_extendedShader = EExtendedShader::ForcedAdditive;
        else if (blendMode > 4)
            m_extendedShader = EExtendedShader::ForcedAlpha;
        else
            m_extendedShader = EExtendedShader::Lighting;
    }

    /* Flags
        0x1: depth equal
        0x2: depth update
        0x4: render without texture lock
        0x8: depth greater
        0x10: depth non-inclusive
     */
};

/* urde addition: doesn't require hacky stashing of
 * pointers within loaded CMDL buffer */
struct CBooSurface
{
    DataSpec::DNACMDL::SurfaceHeader_2 m_data;
    size_t selfIdx;
    class CBooModel* m_parent = nullptr;
    CBooSurface* m_next = nullptr;

    zeus::CAABox GetBounds() const
    {
        if (!m_data.aabbSz)
            return zeus::CAABox(m_data.centroid, m_data.centroid);
        else
            return zeus::CAABox(m_data.aabb[0], m_data.aabb[1]);
    }
};

class CBooModel
{
    friend class CModel;
    friend class CGameArea;
    friend class CBooRenderer;
    friend class CMetroidModelInstance;
    friend class CSkinnedModel;
public:
    using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;
    using UVAnimation = DataSpec::DNAMP1::MaterialSet::Material::UVAnimation;
    struct SShader
    {
        std::vector<TCachedToken<CTexture>> x0_textures;
        std::vector<std::shared_ptr<hecl::Runtime::ShaderPipelines>> m_shaders;
        MaterialSet m_matSet;
        int m_matSetIdx;
        SShader(int idx) : m_matSetIdx(idx) {}
        void UnlockTextures();
        std::shared_ptr<hecl::Runtime::ShaderPipelines>
        BuildShader(const hecl::HMDLMeta& meta, const MaterialSet::Material& mat);
        void BuildShaders(const hecl::HMDLMeta& meta,
                          std::vector<std::shared_ptr<hecl::Runtime::ShaderPipelines>>& shaders);
        void BuildShaders(const hecl::HMDLMeta& meta) { BuildShaders(meta, m_shaders); }
    };

    enum class ESurfaceSelection
    {
        UnsortedOnly,
        SortedOnly,
        All
    };

private:
    CBooModel* m_next = nullptr;
    CBooModel* m_prev = nullptr;
    size_t m_uniUpdateCount = 0;
    TToken<CModel> m_modelTok;
    CModel* m_model;
    std::vector<CBooSurface>* x0_surfaces;
    const MaterialSet* x4_matSet;
    int m_matSetIdx = -1;
    const std::vector<std::shared_ptr<hecl::Runtime::ShaderPipelines>>* m_pipelines;
    std::vector<TCachedToken<CTexture>> x1c_textures;
    zeus::CAABox x20_aabb;
    CBooSurface* x38_firstUnsortedSurface = nullptr;
    CBooSurface* x3c_firstSortedSurface = nullptr;
    bool x40_24_texturesLoaded : 1;
    bool x40_25_modelVisible : 1;
    u8 x41_mask;
    u32 x44_areaInstanceIdx = -1;

    struct UVAnimationBuffer
    {
        static void ProcessAnimation(u8*& bufOut, const UVAnimation& anim);
        static void PadOutBuffer(u8*& bufStart, u8*& bufOut);
        static void Update(u8*& bufOut, const MaterialSet* matSet, const CModelFlags& flags);
    };

    CModelShaders::LightingUniform m_lightingData;

    /* urde addition: boo! */
    size_t m_uniformDataSize = 0;
    struct ModelInstance
    {
        boo::GraphicsDataToken m_gfxToken;
        boo::IGraphicsBufferD* m_uniformBuffer;
        std::vector<std::vector<boo::IShaderDataBinding*>> m_shaderDataBindings;
        boo::IVertexFormat* m_dynamicVtxFmt = nullptr;
        boo::IGraphicsBufferD* m_dynamicVbo = nullptr;

        boo::IGraphicsBuffer* GetBooVBO(const CBooModel& model, boo::IGraphicsDataFactory::Context& ctx);
        boo::IVertexFormat* GetBooVtxFmt(const CBooModel& model, boo::IGraphicsDataFactory::Context& ctx);
    };
    std::vector<ModelInstance> m_instances;

    boo::IVertexFormat* m_staticVtxFmt = nullptr;
    boo::IGraphicsBufferS* m_staticVbo = nullptr;
    boo::IGraphicsBufferS* m_staticIbo = nullptr;

    boo::ITexture* m_txtrOverrides[8] = {};

    boo::ITexture* m_lastDrawnShadowMap = nullptr;

    ModelInstance* PushNewModelInstance();
    void DrawAlphaSurfaces(const CModelFlags& flags) const;
    void DrawNormalSurfaces(const CModelFlags& flags) const;
    void DrawSurfaces(const CModelFlags& flags) const;
    void DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const;
    void WarmupDrawSurfaces() const;
    void WarmupDrawSurface(const CBooSurface& surf) const;

    static zeus::CVector3f g_PlayerPosition;
    static float g_ModSeconds;
    static float g_TransformedTime;
    static float g_TransformedTime2;
    static CBooModel* g_LastModelCached;

    static bool g_DummyTextures;

public:
    ~CBooModel();
    CBooModel(TToken<CModel>& token, CModel* parent, std::vector<CBooSurface>* surfaces, SShader& shader,
              boo::IVertexFormat* vtxFmt, boo::IGraphicsBufferS* vbo, boo::IGraphicsBufferS* ibo,
              const zeus::CAABox& aabb, u8 renderMask,
              int numInsts, boo::ITexture* txtrOverrides[8]);

    static void MakeTexturesFromMats(const MaterialSet& matSet,
                                     std::vector<TCachedToken<CTexture>>& toksOut,
                                     IObjectStore& store);
    void MakeTexturesFromMats(std::vector<TCachedToken<CTexture>>& toksOut,
                              IObjectStore& store);

    bool IsOpaque() const {return x3c_firstSortedSurface == nullptr;}
    void ActivateLights(const std::vector<CLight>& lights);
    void DisableAllLights();
    void RemapMaterialData(SShader& shader);
    void RemapMaterialData(SShader& shader,
                           const std::vector<std::shared_ptr<hecl::Runtime::ShaderPipelines>>& pipelines);
    bool TryLockTextures() const;
    void UnlockTextures() const;
    void SyncLoadTextures() const;
    void Touch(int shaderIdx) const;
    void VerifyCurrentShader(int shaderIdx);
    boo::IGraphicsBufferD* UpdateUniformData(const CModelFlags& flags,
                                             const CSkinRules* cskr,
                                             const CPoseAsTransforms* pose) const;
    void DrawAlpha(const CModelFlags& flags,
                   const CSkinRules* cskr,
                   const CPoseAsTransforms* pose) const;
    void DrawNormal(const CModelFlags& flags,
                    const CSkinRules* cskr,
                    const CPoseAsTransforms* pose) const;
    void Draw(const CModelFlags& flags,
              const CSkinRules* cskr,
              const CPoseAsTransforms* pose) const;
    void DrawFlat(ESurfaceSelection sel, EExtendedShader extendedIdx) const;

    void LockParent() { m_modelTok.Lock(); }
    void UnlockParent() { m_modelTok.Unlock(); }


    const MaterialSet::Material& GetMaterialByIndex(int idx) const
    {
        return x4_matSet->materials.at(idx);
    }

    void ClearUniformCounter() { m_uniUpdateCount = 0; }
    static void ClearModelUniformCounters();

    static bool g_DrawingOccluders;
    static void SetDrawingOccluders(bool occ) {g_DrawingOccluders = occ;}

    static void SetNewPlayerPositionAndTime(const zeus::CVector3f& pos);

    static zeus::CVector3f g_ReflectViewPos;
    static void KillCachedViewDepState();
    static void EnsureViewDepStateCached(const CBooModel& model, const CBooSurface* surf,
                                         zeus::CMatrix4f* mtxsOut, float& alphaOut);

    static boo::ITexture* g_shadowMap;
    static zeus::CTransform g_shadowTexXf;
    static void EnableShadowMaps(boo::ITexture* map, const zeus::CTransform& texXf);
    static void DisableShadowMaps();

    static void SetDummyTextures(bool b) { g_DummyTextures = true; }
};

class CModel
{
    friend class CBooModel;
    //std::unique_ptr<u8[]> x0_data;
    //u32 x4_dataLen;
    TToken<CModel> m_selfToken; /* DO NOT LOCK! */
    zeus::CAABox m_aabb;
    u32 m_flags;
    std::vector<CBooSurface> x8_surfaces;
    std::vector<CBooModel::SShader> x18_matSets;
    std::unique_ptr<CBooModel> x28_modelInst;
    CModel* x30_next = nullptr;
    CModel* x34_prev = nullptr;
    int x38_lastFrame;

    /* urde addition: boo! */
    boo::GraphicsDataToken m_gfxToken;
    boo::IVertexFormat* m_staticVtxFmt = nullptr;
    boo::IGraphicsBufferS* m_staticVbo = nullptr;
    hecl::HMDLMeta m_hmdlMeta;
    std::unique_ptr<uint8_t[]> m_dynamicVertexData;
    boo::IGraphicsBufferS* m_ibo = nullptr;

public:
    using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;

    CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store, CObjectReference* selfRef);
    void DrawSortedParts(const CModelFlags& flags) const;
    void DrawUnsortedParts(const CModelFlags& flags) const;
    void Draw(const CModelFlags& flags) const;
    bool IsLoaded(int shaderIdx) const;
    void Touch(int shaderIdx) { x28_modelInst->Touch(shaderIdx); }

    const zeus::CAABox& GetAABB() const {return m_aabb;}
    CBooModel& GetInstance() {return *x28_modelInst;}
    const CBooModel& GetInstance() const {return *x28_modelInst;}
    std::unique_ptr<CBooModel> MakeNewInstance(int shaderIdx, int subInsts,
                                               boo::ITexture* txtrOverrides[8] = nullptr,
                                               bool lockParent = true);
    void UpdateLastFrame() const { const_cast<CModel&>(*this).x38_lastFrame = CGraphics::GetFrameCounter(); }

    size_t GetPoolVertexOffset(size_t idx) const;
    zeus::CVector3f GetPoolVertex(size_t idx) const;
    size_t GetPoolNormalOffset(size_t idx) const;
    zeus::CVector3f GetPoolNormal(size_t idx) const;
    void ApplyVerticesCPU(boo::IGraphicsBufferD* vertBuf,
                          const std::vector<std::pair<zeus::CVector3f, zeus::CVector3f>>& vn) const;

    void _WarmupShaders();
    static void WarmupShaders(const SObjectTag& cmdlTag);
};

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms,
                               CObjectReference* selfRef);

}

#endif // __URDE_CMODEL_HPP__
