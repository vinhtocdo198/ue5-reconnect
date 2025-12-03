// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4216267164, Redpoint::EOS::Tests::TestUtilities)
{

class REDPOINTEOSTESTS_API FEOSConfigEASLogin : public Redpoint::EOS::Config::IConfig
{
protected:
    virtual FString GetDelegatedSubsystemsString() const override
    {
        return TEXT("");
    }

public:
    FEOSConfigEASLogin() = default;
    UE_NONCOPYABLE(FEOSConfigEASLogin);
    virtual ~FEOSConfigEASLogin() override = default;

    virtual FString GetFreeEditionLicenseKey() const override
    {
        // Get this from the environment so that we're not embedding a valid license key
        // in the test binary (this will only be used on CI/CD anyway).
        return FPlatformMisc::GetEnvironmentVariable(TEXT("ONLINE_SUBSYSTEM_EOS_FREE_EDITION_LICENSE_KEY"));
    }
    virtual FString GetEncryptionKey() const override
    {
        return TEXT("4dc4ad8a46823586f4044225d6cf1b1e7ee32b3d7dff1c63b6ad5807671c4a3f");
    }
    virtual FString GetProductName() const override
    {
        return TEXT("Online Subsystem EOS Automated Tests");
    }
    virtual FString GetProductVersion() const override
    {
        return TEXT("0.0.0");
    }
    virtual FString GetProductId() const override
    {
        return TEXT("620b983080ae4bedbe11b2c49154b55d");
    }
    virtual FString GetSandboxId() const override
    {
        return TEXT("d03930678c1a43148620f9d891750763");
    }
    virtual FString GetDeploymentId() const override
    {
        return TEXT("c7055aea5c644e7d8f5898464b7a5305");
    }
    virtual FString GetClientId() const override
    {
        return TEXT("xyza7891af30Orhot3aht6Zb22EBIHsg");
    }
    virtual FString GetClientSecret() const override
    {
        return TEXT("3whfGHQUpg7P6Sy6hupm2DK2PqK1Sn2UFixzW6xbkiY");
    }
    virtual FString GetDedicatedServerClientId() const override
    {
        return TEXT("xyza7891tEstBjIqzJrii9y9gs4JMkqm");
    }
    virtual FString GetDedicatedServerClientSecret() const override
    {
        return TEXT("X7O/93GLCg9Hg44u+gX3bvLrgGtvIvK+Uzx6j5bi+Lo");
    }
    virtual FString GetDeveloperToolAddress() const override
    {
        return TEXT("localhost:6300");
    }
    virtual FString GetDeveloperToolDefaultCredentialName() const override
    {
        return TEXT("AutomatedTests");
    }
    virtual FString GetWidgetClass(FString WidgetName, FString DefaultValue) const override
    {
        return DefaultValue;
    }
    virtual EEOSApiVersion GetApiVersion() const override
    {
        return (EEOSApiVersion)0;
    }
    virtual FName GetAuthenticationGraph() const override
    {
        return FName(TEXT("AutomatedTesting"));
    }
    virtual FName GetEditorAuthenticationGraph() const override
    {
        return FName(TEXT("AutomatedTesting"));
    }
    virtual FName GetCrossPlatformAccountProvider() const override
    {
        return FName(TEXT("EpicGames"));
    }
    virtual bool GetRequireCrossPlatformAccount() const override
    {
        return true;
    }
    virtual EPresenceAdvertisementType GetPresenceAdvertisementType() const override
    {
        return EPresenceAdvertisementType::None;
    }
    virtual bool IsAutomatedTesting() const override
    {
        return true;
    }
    virtual bool GetRequireEpicGamesLauncher() const override
    {
        return false;
    }
    virtual bool GetNeverShowEpicGamesOverlay() const override
    {
        return true;
    }
    virtual FString GetSimpleFirstPartyLoginUrl() const override
    {
        return TEXT("");
    }
    virtual EPartyJoinabilityConstraint GetPartyJoinabilityConstraint() const override
    {
        return EPartyJoinabilityConstraint::AllowPlayersInMultipleParties;
    }
    virtual FString GetDedicatedServerPublicKey() const override
    {
        return TEXT("OaucWUPjiik6aos1W79NbSFt5TCPC7ejzRnB4OIbY20=");
    }
    virtual FString GetDedicatedServerPrivateKey() const override
    {
        return TEXT("z6Gu5K0vGXb6jPSyqkFk7PKjNPBo3WCqgkHflCjOyG85q5xZQ+OKKTpqizVbv01tIW3lMI8Lt6PNGcHg4htjbQ==");
    }
    virtual bool GetEnableAutomaticEncryptionOnTrustedDedicatedServers() const override
    {
        return true;
    }
    virtual EDedicatedServersDistributionMode GetDedicatedServerDistributionMode() const override
    {
        return EDedicatedServersDistributionMode::DevelopersAndPlayers;
    }
    virtual bool GetEnableAntiCheat() const override
    {
        return false;
    }
    virtual FString GetTrustedClientPublicKey() const override
    {
        return TEXT("");
    }
    virtual FString GetTrustedClientPrivateKey() const override
    {
        return TEXT("");
    }
    virtual bool GetEnableVoiceChatEchoInParties() const override
    {
        return false;
    }
    virtual bool GetEnableVoiceChatPlatformAECByDefault() const override
    {
        return false;
    }
    virtual TArray<TTuple<FString, EStatTypingRule>> GetStatTypingRules() const override
    {
        // Rules for the stat tests.
        return TArray<TTuple<FString, EStatTypingRule>>{
            TTuple<FString, EStatTypingRule>(TEXT("TestFloatTrunc"), EStatTypingRule::FloatTruncated),
            TTuple<FString, EStatTypingRule>(TEXT("TestFloatEnc"), EStatTypingRule::FloatEncoded),
            TTuple<FString, EStatTypingRule>(TEXT("Te*Double*"), EStatTypingRule::DoubleEncoded)};
    }
    virtual bool GetAcceptStatWriteRequestsFromServers() const override
    {
        return true;
    }
    virtual ENetworkAuthenticationMode GetNetworkAuthenticationMode() const override
    {
        return ENetworkAuthenticationMode::IDToken;
    }
    virtual TArray<EOS_EAuthScopeFlags> GetEpicGamesScopes() const override
    {
        return TArray<EOS_EAuthScopeFlags>{this->GetSupportedEpicGamesScopes()};
    }
    virtual EOS_EAuthScopeFlags GetSupportedEpicGamesScopes() const override
    {
        return EOS_EAuthScopeFlags::EOS_AS_BasicProfile | EOS_EAuthScopeFlags::EOS_AS_FriendsList |
               EOS_EAuthScopeFlags::EOS_AS_Presence | EOS_EAuthScopeFlags::EOS_AS_Country;
    }
    virtual bool GetRequireOnlinePlayUserPrivilege() const override
    {
        return false;
    }
};

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(4216267164, Redpoint::EOS::Tests::TestUtilities, FEOSConfigEASLogin)
}

REDPOINT_EOS_CODE_GUARD_END()