// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/Config.h"

namespace Redpoint::EOS::Config
{

bool ApiVersionIsAtLeast(EEOSApiVersion InConfigVersion, EEOSApiVersion InTargetVersion)
{
    return InConfigVersion <= InTargetVersion;
}

bool ApiVersionIsAtLeast(const IConfig &InConfig, EEOSApiVersion InTargetVersion)
{
    return ApiVersionIsAtLeast(InConfig.GetApiVersion(), InTargetVersion);
}

void *IConfig::GetReservedOptions() const
{
    return nullptr;
}

FString IConfig::GetAuthServiceOverride() const
{
    return TEXT("");
}

bool IConfig::HasSynthetics() const
{
    return !this->GetDelegatedSubsystemsString().IsEmpty();
}

bool IConfig::GetUseFallbackAddressesInDevelopment() const
{
    return true;
}

float IConfig::GetFallbackAddressTimeoutSecondsInDevelopment() const
{
    return 2.0f;
}

bool IConfig::GetFrameworkDefaultEnsureAlwaysInParty() const
{
    return false;
}

int32 IConfig::GetFrameworkDefaultPartyMaximumSize() const
{
    return 4;
}

bool IConfig::GetFrameworkDefaultEnableVoiceChatInParty() const
{
    return false;
}

TArray<FString> IConfig::GetPrecachePlayerDataStorageFiles() const
{
    return TArray<FString>();
}

}