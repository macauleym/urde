#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct NewCameraShaker : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<bool> active;
  PropertyFlags flags;
  Value<float> duration;
  Value<float> sfxDist;
  struct CameraShakerComponent : BigDNA {
    AT_DECL_DNA
    PropertyFlags flags;
    struct CameraShakePoint : BigDNA {
      AT_DECL_DNA
      PropertyFlags flags;
      Value<float> attackTime;
      Value<float> sustainTime;
      Value<float> duration;
      Value<float> magnitude;
    };
    CameraShakePoint am;
    CameraShakePoint fm;
  } shakerComponents[3];
};
} // namespace DataSpec::DNAMP1
