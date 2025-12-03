// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2660729793, Redpoint::EOS::Config::DynamicConfigLayers)
{

class FDynamicConfigLayerLoader
{
private:
    static void LoadDynamicConfigLayers();

public:
    static void RegisterEventHandlerToLoadDynamicConfigLayers();
};

}

namespace Redpoint::EOS::Config::DynamicConfigLayers
{
REDPOINT_EOS_FILE_NS_EXPORT(2660729793, Redpoint::EOS::Config::DynamicConfigLayers, FDynamicConfigLayerLoader)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()