// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2755057129, Redpoint::EOS::Config::DynamicConfigLayers)
{
using namespace ::Redpoint::EOS::Config::DynamicConfigLayers;

class FDynamicConfigLayerProvider
{
public:
    static TArray<TSharedRef<IDynamicConfigLayer>> GetDynamicConfigLayersToLoad();
};

}

namespace Redpoint::EOS::Config::DynamicConfigLayers
{
REDPOINT_EOS_FILE_NS_EXPORT(2755057129, Redpoint::EOS::Config::DynamicConfigLayers, FDynamicConfigLayerProvider)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()