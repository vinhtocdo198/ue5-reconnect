// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2853190323, Redpoint::EOS::Config::DynamicConfigLayers)
{
using namespace ::Redpoint::EOS::Config::DynamicConfigLayers;

class FOnlineSubsystemDynamicConfigLayer : public IDynamicConfigLayer
{
public:
    FOnlineSubsystemDynamicConfigLayer() = default;
    UE_NONCOPYABLE(FOnlineSubsystemDynamicConfigLayer);
    virtual ~FOnlineSubsystemDynamicConfigLayer() override = default;

    virtual EDynamicConfigLayerApplyTo GetLayerAppliesTo() const override;
    virtual FString GetLayerName() const override;
    virtual FString GetLayerIniContent() const override;
};

}

namespace Redpoint::EOS::Config::DynamicConfigLayers
{
REDPOINT_EOS_FILE_NS_EXPORT(2853190323, Redpoint::EOS::Config::DynamicConfigLayers, FOnlineSubsystemDynamicConfigLayer)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()