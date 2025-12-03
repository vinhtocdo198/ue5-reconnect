// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayerProvider.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSConfig/DynamicConfigLayers/EngineInterfaceDynamicConfigLayer.h"
#include "RedpointEOSConfig/DynamicConfigLayers/NetDriverDynamicConfigLayer.h"
#include "RedpointEOSConfig/DynamicConfigLayers/OnlineSubsystemDynamicConfigLayer.h"
#include "RedpointEOSConfig/DynamicConfigLayers/RedirectDynamicConfigLayer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2755057129, Redpoint::EOS::Config::DynamicConfigLayers)
{

TArray<TSharedRef<IDynamicConfigLayer>> FDynamicConfigLayerProvider::GetDynamicConfigLayersToLoad()
{
    TArray<TSharedRef<IDynamicConfigLayer>> Layers;
    Layers.Add(MakeShared<FEngineInterfaceDynamicConfigLayer>());
    Layers.Add(MakeShared<FNetDriverDynamicConfigLayer>());
    Layers.Add(MakeShared<FOnlineSubsystemDynamicConfigLayer>());
    Layers.Add(MakeShared<FRedirectDynamicConfigLayer>());
    return Layers;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()