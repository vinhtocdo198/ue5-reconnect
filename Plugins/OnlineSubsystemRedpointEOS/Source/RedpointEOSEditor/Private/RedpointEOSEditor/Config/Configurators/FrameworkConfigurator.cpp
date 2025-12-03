// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/FrameworkConfigurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2400495877, Redpoint::EOS::Editor::Config::Configurators)
{

void FFrameworkConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->EnsureAlwaysInParty = false;
    Instance->PartyMaximumSize = 4;
    Instance->EnableVoiceChatInParty = false;
}

void FFrameworkConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Reader.GetBool(TEXT("FrameworkDefaultEnsureAlwaysInParty"), Instance->EnsureAlwaysInParty);
    Reader.GetInt32(TEXT("FrameworkDefaultPartyMaximumSize"), Instance->PartyMaximumSize);
    Reader.GetBool(TEXT("FrameworkDefaultEnableVoiceChatInParty"), Instance->EnableVoiceChatInParty);
}

void FFrameworkConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetBool(TEXT("FrameworkDefaultEnsureAlwaysInParty"), Instance->EnsureAlwaysInParty);
    Writer.SetInt32(TEXT("FrameworkDefaultPartyMaximumSize"), Instance->PartyMaximumSize);
    Writer.SetBool(TEXT("FrameworkDefaultEnableVoiceChatInParty"), Instance->EnableVoiceChatInParty);
}

}

REDPOINT_EOS_CODE_GUARD_END()