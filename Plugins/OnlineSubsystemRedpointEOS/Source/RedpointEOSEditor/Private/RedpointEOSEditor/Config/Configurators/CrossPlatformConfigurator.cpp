// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/CrossPlatformConfigurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3521751065, Redpoint::EOS::Editor::Config::Configurators)
{

void FCrossPlatformConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->PartyJoinabilityConstraint = EPartyJoinabilityConstraint::AllowPlayersInMultipleParties;
}

void FCrossPlatformConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    FString DelegatedSubsystems;
    if (Reader.GetString(TEXT("DelegatedSubsystems"), DelegatedSubsystems))
    {
        DelegatedSubsystems.ParseIntoArray(Instance->DelegatedSubsystems, TEXT(","), true);
    }

    Reader.GetEnum<EPartyJoinabilityConstraint>(
        TEXT("PartyJoinabilityConstraint"),
        TEXT("EPartyJoinabilityConstraint"),
        Instance->PartyJoinabilityConstraint);
}

void FCrossPlatformConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetString(TEXT("DelegatedSubsystems"), FString::Join(Instance->DelegatedSubsystems, TEXT(",")));
    Writer.SetEnum<EPartyJoinabilityConstraint>(
        TEXT("PartyJoinabilityConstraint"),
        TEXT("EPartyJoinabilityConstraint"),
        Instance->PartyJoinabilityConstraint);
}

}

REDPOINT_EOS_CODE_GUARD_END()