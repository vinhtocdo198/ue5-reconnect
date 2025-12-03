// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1798880739, Redpoint::Fab::Authentication)
{
using namespace ::Redpoint::EOS::Config;

class FFabConfig : public IConfig
{
private:
    FString ProductId;
    FString SandboxId;
    FString DeploymentId;
    FString ClientId;
    FString ClientSecret;
    FString EncryptionKey;
    bool bIsGamedev;
    void *ReservedOptions;

public:
    FFabConfig(
        const FString &InProductId,
        const FString &InSandboxId,
        const FString &InDeploymentId,
        const FString &InClientId,
        const FString &InClientSecret,
        const FString &InEncryptionKey,
        bool bInIsGameDev);
    UE_NONCOPYABLE(FFabConfig);
    virtual ~FFabConfig() override = default;

    virtual FString GetDelegatedSubsystemsString() const override;
    virtual FString GetFreeEditionLicenseKey() const override;
    virtual FString GetEncryptionKey() const override;
    virtual FString GetProductName() const override;
    virtual FString GetProductVersion() const override;
    virtual FString GetProductId() const override;
    virtual FString GetSandboxId() const override;
    virtual FString GetDeploymentId() const override;
    virtual FString GetClientId() const override;
    virtual FString GetClientSecret() const override;
    virtual FString GetDedicatedServerClientId() const override;
    virtual FString GetDedicatedServerClientSecret() const override;
    virtual FString GetDeveloperToolAddress() const override;
    virtual FString GetDeveloperToolDefaultCredentialName() const override;
    virtual FString GetWidgetClass(FString WidgetName, FString DefaultValue) const override;
    virtual EEOSApiVersion GetApiVersion() const override;
    virtual FName GetAuthenticationGraph() const override;
    virtual FName GetEditorAuthenticationGraph() const override;
    virtual FName GetCrossPlatformAccountProvider() const override;
    virtual bool GetRequireCrossPlatformAccount() const override;
    virtual EPresenceAdvertisementType GetPresenceAdvertisementType() const override;
    virtual bool IsAutomatedTesting() const override;
    virtual bool GetRequireEpicGamesLauncher() const override;
    virtual bool GetNeverShowEpicGamesOverlay() const override;
    virtual FString GetSimpleFirstPartyLoginUrl() const override;
    virtual EPartyJoinabilityConstraint GetPartyJoinabilityConstraint() const override;
    virtual bool GetEnableAutomaticEncryptionOnTrustedDedicatedServers() const override;
    virtual EDedicatedServersDistributionMode GetDedicatedServerDistributionMode() const override;
    virtual FString GetDedicatedServerPublicKey() const override;
    virtual FString GetDedicatedServerPrivateKey() const override;
    virtual bool GetEnableAntiCheat() const override;
    virtual FString GetTrustedClientPublicKey() const override;
    virtual FString GetTrustedClientPrivateKey() const override;
    virtual bool GetEnableVoiceChatEchoInParties() const override;
    virtual bool GetEnableVoiceChatPlatformAECByDefault() const override;
    virtual TArray<TTuple<FString, EStatTypingRule>> GetStatTypingRules() const override;
    virtual bool GetAcceptStatWriteRequestsFromServers() const override;
    virtual ENetworkAuthenticationMode GetNetworkAuthenticationMode() const override;
    virtual TArray<EOS_EAuthScopeFlags> GetEpicGamesScopes() const override;
    virtual EOS_EAuthScopeFlags GetSupportedEpicGamesScopes() const override;
    virtual bool GetRequireOnlinePlayUserPrivilege() const override;
    virtual void *GetReservedOptions() const override;
};

}

namespace Redpoint::Fab::Authentication
{
REDPOINT_EOS_FILE_NS_EXPORT(1798880739, Redpoint::Fab::Authentication, FFabConfig);
}

REDPOINT_EOS_CODE_GUARD_END()