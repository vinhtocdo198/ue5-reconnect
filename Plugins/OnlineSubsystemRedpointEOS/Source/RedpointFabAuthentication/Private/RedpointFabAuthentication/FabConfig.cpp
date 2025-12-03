// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFabAuthentication/FabConfig.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1798880739, Redpoint::Fab::Authentication)
{

static struct FFabReservedOptions
{
    int32_t ApiVersion;
    const char *BackendEnvironment;
} FabReservedOptions = {1, "GameDev"};

FFabConfig::FFabConfig(
    const FString &InProductId,
    const FString &InSandboxId,
    const FString &InDeploymentId,
    const FString &InClientId,
    const FString &InClientSecret,
    const FString &InEncryptionKey,
    bool bInIsGamedev)
    : ProductId(InProductId)
    , SandboxId(InSandboxId)
    , DeploymentId(InDeploymentId)
    , ClientId(InClientId)
    , ClientSecret(InClientSecret)
    , EncryptionKey(InEncryptionKey)
    , bIsGamedev(bInIsGamedev)
    , ReservedOptions(bInIsGamedev ? &FabReservedOptions : nullptr)
{
}

FString FFabConfig::GetDelegatedSubsystemsString() const
{
    return TEXT("");
}

FString FFabConfig::GetFreeEditionLicenseKey() const
{
    return TEXT("");
}

FString FFabConfig::GetEncryptionKey() const
{
    return this->EncryptionKey;
}

FString FFabConfig::GetProductName() const
{
    return TEXT("");
}
FString FFabConfig::GetProductVersion() const
{
    return TEXT("");
}

FString FFabConfig::GetProductId() const
{
    return this->ProductId;
}

FString FFabConfig::GetSandboxId() const
{
    return this->SandboxId;
}

FString FFabConfig::GetDeploymentId() const
{
    return this->DeploymentId;
}

FString FFabConfig::GetClientId() const
{
    return this->ClientId;
}

FString FFabConfig::GetClientSecret() const
{
    return this->ClientSecret;
}

FString FFabConfig::GetDedicatedServerClientId() const
{
    return TEXT("");
}

FString FFabConfig::GetDedicatedServerClientSecret() const
{
    return TEXT("");
}

FString FFabConfig::GetDeveloperToolAddress() const
{
    return TEXT("");
}

FString FFabConfig::GetDeveloperToolDefaultCredentialName() const
{
    return TEXT("");
}

FString FFabConfig::GetWidgetClass(FString WidgetName, FString DefaultValue) const
{
    return TEXT("");
}

EEOSApiVersion FFabConfig::GetApiVersion() const
{
    return static_cast<EEOSApiVersion>(0);
}

FName FFabConfig::GetAuthenticationGraph() const
{
    return TEXT("Default");
}

FName FFabConfig::GetEditorAuthenticationGraph() const
{
    return TEXT("Default");
}

FName FFabConfig::GetCrossPlatformAccountProvider() const
{
    return TEXT("EpicGames");
}

bool FFabConfig::GetRequireCrossPlatformAccount() const
{
    return true;
}

EPresenceAdvertisementType FFabConfig::GetPresenceAdvertisementType() const
{
    return EPresenceAdvertisementType::None;
}

bool FFabConfig::IsAutomatedTesting() const
{
    return false;
}

bool FFabConfig::GetRequireEpicGamesLauncher() const
{
    return false;
}

bool FFabConfig::GetNeverShowEpicGamesOverlay() const
{
    return true;
}

FString FFabConfig::GetSimpleFirstPartyLoginUrl() const
{
    return TEXT("");
}

EPartyJoinabilityConstraint FFabConfig::GetPartyJoinabilityConstraint() const
{
    return EPartyJoinabilityConstraint::IgnoreInvitesIfAlreadyInParty;
}

bool FFabConfig::GetEnableAutomaticEncryptionOnTrustedDedicatedServers() const
{
    return false;
}

EDedicatedServersDistributionMode FFabConfig::GetDedicatedServerDistributionMode() const
{
    return EDedicatedServersDistributionMode::DevelopersOnly;
}

FString FFabConfig::GetDedicatedServerPublicKey() const
{
    return TEXT("");
}

FString FFabConfig::GetDedicatedServerPrivateKey() const
{
    return TEXT("");
}

bool FFabConfig::GetEnableAntiCheat() const
{
    return false;
}

FString FFabConfig::GetTrustedClientPublicKey() const
{
    return TEXT("");
}

FString FFabConfig::GetTrustedClientPrivateKey() const
{
    return TEXT("");
}

bool FFabConfig::GetEnableVoiceChatEchoInParties() const
{
    return false;
}

bool FFabConfig::GetEnableVoiceChatPlatformAECByDefault() const
{
    return false;
}

TArray<TTuple<FString, EStatTypingRule>> FFabConfig::GetStatTypingRules() const
{
    return TArray<TTuple<FString, EStatTypingRule>>();
}

bool FFabConfig::GetAcceptStatWriteRequestsFromServers() const
{
    return false;
}

ENetworkAuthenticationMode FFabConfig::GetNetworkAuthenticationMode() const
{
    return ENetworkAuthenticationMode::Off;
}

TArray<EOS_EAuthScopeFlags> FFabConfig::GetEpicGamesScopes() const
{
    return TArray<EOS_EAuthScopeFlags>();
}

EOS_EAuthScopeFlags FFabConfig::GetSupportedEpicGamesScopes() const
{
    return EOS_EAuthScopeFlags::EOS_AS_NoFlags;
}

bool FFabConfig::GetRequireOnlinePlayUserPrivilege() const
{
    return false;
}

void *FFabConfig::GetReservedOptions() const
{
    return this->ReservedOptions;
}

}

REDPOINT_EOS_CODE_GUARD_END()