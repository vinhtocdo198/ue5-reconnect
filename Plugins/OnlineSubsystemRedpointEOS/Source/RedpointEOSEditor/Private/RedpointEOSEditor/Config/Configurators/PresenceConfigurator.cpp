// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/PresenceConfigurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1623824531, Redpoint::EOS::Editor::Config::Configurators)
{

void FPresenceConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->PresenceAdvertises = EPresenceAdvertisementType::Party;
}

void FPresenceConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Reader.GetEnum<EPresenceAdvertisementType>(
        TEXT("PresenceAdvertises"),
        TEXT("EPresenceAdvertisementType"),
        Instance->PresenceAdvertises);
}

void FPresenceConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetEnum<EPresenceAdvertisementType>(
        TEXT("PresenceAdvertises"),
        TEXT("EPresenceAdvertisementType"),
        Instance->PresenceAdvertises);
}

}

REDPOINT_EOS_CODE_GUARD_END()