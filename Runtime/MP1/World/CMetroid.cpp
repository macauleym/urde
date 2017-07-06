#include "CMetroid.hpp"
#include "World/ScriptLoader.hpp"

namespace urde
{
namespace MP1
{

CMetroidData::CMetroidData(CInputStream& in)
: x0_dVuln1(in), x68_dVuln2(in),
  xd0_(in.readFloatBig()),
  xd4_(in.readFloatBig()),
  xd8_(in.readFloatBig()),
  xdc_(in.readFloatBig()),
  xe0_(in.readFloatBig()),
  xe4_(in.readFloatBig())
{
    xe8_animParms1 = ScriptLoader::LoadAnimationParameters(in);
    xf8_animParms2 = ScriptLoader::LoadAnimationParameters(in);
    x108_animParms3 = ScriptLoader::LoadAnimationParameters(in);
    x118_animParms4 = ScriptLoader::LoadAnimationParameters(in);
    x128_24_ = in.readBool();
}

CMetroid::CMetroid(TUniqueId uid, const std::string& name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                   const CActorParameters& aParms, const CMetroidData& metroidData)
: CPatterned(ECharacter::Metroid, uid, name, flavor, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Three, aParms, true)
{
}

}
}