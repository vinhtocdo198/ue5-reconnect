// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1724827442, Redpoint::EOS::Config::DynamicConfigLayers)
{
using namespace ::Redpoint::EOS::Config::DynamicConfigLayers;

class FRedirectDynamicConfigLayer : public IDynamicConfigLayer
{
public:
    FRedirectDynamicConfigLayer() = default;
    UE_NONCOPYABLE(FRedirectDynamicConfigLayer);
    virtual ~FRedirectDynamicConfigLayer() override = default;

    virtual EDynamicConfigLayerApplyTo GetLayerAppliesTo() const override;
    virtual FString GetLayerName() const override;
    virtual FString GetLayerIniContent() const override;
};

}

namespace Redpoint::EOS::Config::DynamicConfigLayers
{
REDPOINT_EOS_FILE_NS_EXPORT(1724827442, Redpoint::EOS::Config::DynamicConfigLayers, FRedirectDynamicConfigLayer)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()