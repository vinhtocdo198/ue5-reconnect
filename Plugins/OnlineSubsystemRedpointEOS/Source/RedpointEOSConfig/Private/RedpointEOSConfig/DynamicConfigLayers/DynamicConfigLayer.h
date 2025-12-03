// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(3564025432, Redpoint::EOS::Config::DynamicConfigLayers)
{

enum EDynamicConfigLayerApplyTo
{
    // Apply on top of DefaultEngine.ini hierarchy.
    Engine,
};

class IDynamicConfigLayer
{
public:
    IDynamicConfigLayer() = default;
    UE_NONCOPYABLE(IDynamicConfigLayer);
    virtual ~IDynamicConfigLayer() = default;

    virtual EDynamicConfigLayerApplyTo GetLayerAppliesTo() const = 0;
    virtual FString GetLayerName() const = 0;
    virtual FString GetLayerIniContent() const = 0;
};

}

namespace Redpoint::EOS::Config::DynamicConfigLayers
{
REDPOINT_EOS_FILE_NS_EXPORT(3564025432, Redpoint::EOS::Config::DynamicConfigLayers, EDynamicConfigLayerApplyTo)
REDPOINT_EOS_FILE_NS_EXPORT(3564025432, Redpoint::EOS::Config::DynamicConfigLayers, IDynamicConfigLayer)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()