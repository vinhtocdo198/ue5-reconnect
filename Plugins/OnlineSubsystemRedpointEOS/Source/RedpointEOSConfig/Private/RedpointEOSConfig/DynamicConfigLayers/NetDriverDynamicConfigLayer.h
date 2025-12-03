// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2214528975, Redpoint::EOS::Config::DynamicConfigLayers)
{
using namespace ::Redpoint::EOS::Config::DynamicConfigLayers;

class FNetDriverDynamicConfigLayer : public IDynamicConfigLayer
{
public:
    FNetDriverDynamicConfigLayer() = default;
    UE_NONCOPYABLE(FNetDriverDynamicConfigLayer);
    virtual ~FNetDriverDynamicConfigLayer() override = default;

    virtual EDynamicConfigLayerApplyTo GetLayerAppliesTo() const override;
    virtual FString GetLayerName() const override;
    virtual FString GetLayerIniContent() const override;
};

}

namespace Redpoint::EOS::Config::DynamicConfigLayers
{
REDPOINT_EOS_FILE_NS_EXPORT(2214528975, Redpoint::EOS::Config::DynamicConfigLayers, FNetDriverDynamicConfigLayer)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()