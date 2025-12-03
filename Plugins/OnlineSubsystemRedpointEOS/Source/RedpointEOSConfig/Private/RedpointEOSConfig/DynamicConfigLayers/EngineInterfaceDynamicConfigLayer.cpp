// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DynamicConfigLayers/EngineInterfaceDynamicConfigLayer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(1992884390, Redpoint::EOS::Config::DynamicConfigLayers)
{

EDynamicConfigLayerApplyTo FEngineInterfaceDynamicConfigLayer::GetLayerAppliesTo() const
{
    return EDynamicConfigLayerApplyTo::Engine;
}

FString FEngineInterfaceDynamicConfigLayer::GetLayerName() const
{
    return TEXT("EngineInterface");
}

FString FEngineInterfaceDynamicConfigLayer::GetLayerIniContent() const
{
    return TEXT("[/Script/Engine.OnlineEngineInterface]\n\
ClassName=/Script/RedpointEOSUtils.OnlineEngineInterfaceEOS\n\
");
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()