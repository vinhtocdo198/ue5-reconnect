// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DynamicConfigLayers/OnlineSubsystemDynamicConfigLayer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2853190323, Redpoint::EOS::Config::DynamicConfigLayers)
{

EDynamicConfigLayerApplyTo FOnlineSubsystemDynamicConfigLayer::GetLayerAppliesTo() const
{
    return EDynamicConfigLayerApplyTo::Engine;
}

FString FOnlineSubsystemDynamicConfigLayer::GetLayerName() const
{
    return TEXT("OnlineSubsystem");
}

FString FOnlineSubsystemDynamicConfigLayer::GetLayerIniContent() const
{
    return TEXT("[OnlineSubsystem]\n\
DefaultPlatformService=RedpointEOS\n\
\n\
[OnlineSubsystemSteam]\n\
bUseSteamNetworking=False\n\
");
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()