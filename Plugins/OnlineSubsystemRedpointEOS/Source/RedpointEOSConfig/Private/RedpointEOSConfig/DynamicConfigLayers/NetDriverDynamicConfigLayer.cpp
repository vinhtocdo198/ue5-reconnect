// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DynamicConfigLayers/NetDriverDynamicConfigLayer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2214528975, Redpoint::EOS::Config::DynamicConfigLayers)
{

EDynamicConfigLayerApplyTo FNetDriverDynamicConfigLayer::GetLayerAppliesTo() const
{
    return EDynamicConfigLayerApplyTo::Engine;
}

FString FNetDriverDynamicConfigLayer::GetLayerName() const
{
    return TEXT("NetDriver");
}

FString FNetDriverDynamicConfigLayer::GetLayerIniContent() const
{
    return TEXT("[/Script/Engine.Engine]\n\
!NetDriverDefinitions=ClearArray\n\
+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")\n\
+NetDriverDefinitions=(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")\n\
+NetDriverDefinitions=(DefName=\"DemoNetDriver\",DriverClassName=\"/Script/Engine.DemoNetDriver\",DriverClassNameFallback=\"/Script/Engine.DemoNetDriver\")\n\
\n\
[/Script/Engine.GameEngine]\n\
!NetDriverDefinitions=ClearArray\n\
+NetDriverDefinitions=(DefName=\"GameNetDriver\",DriverClassName=\"/Script/RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")\n\
+NetDriverDefinitions=(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/OnlineSubsystemUtils.IpNetDriver\")\n\
+NetDriverDefinitions=(DefName=\"DemoNetDriver\",DriverClassName=\"/Script/Engine.DemoNetDriver\",DriverClassNameFallback=\"/Script/Engine.DemoNetDriver\")\n\
");
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()