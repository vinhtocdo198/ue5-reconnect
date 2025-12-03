// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/VoiceChatConfigurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(433308794, Redpoint::EOS::Editor::Config::Configurators)
{

void FVoiceChatConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->EnableVoiceChatEchoInParties = false;
    Instance->EnableVoiceChatPlatformAECByDefault = false;
}

void FVoiceChatConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Reader.GetBool(TEXT("EnableVoiceChatEchoInParties"), Instance->EnableVoiceChatEchoInParties);
    Reader.GetBool(TEXT("EnableVoiceChatPlatformAECByDefault"), Instance->EnableVoiceChatPlatformAECByDefault);
}

void FVoiceChatConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetBool(TEXT("EnableVoiceChatEchoInParties"), Instance->EnableVoiceChatEchoInParties);
    Writer.SetBool(TEXT("EnableVoiceChatPlatformAECByDefault"), Instance->EnableVoiceChatPlatformAECByDefault);
}

}

REDPOINT_EOS_CODE_GUARD_END()