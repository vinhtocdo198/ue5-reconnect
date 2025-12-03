// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/NetworkingConfigurator.h"

#include "Misc/Base64.h"
#include "RedpointEOSEditor/Config/PlatformHelpers.h"
#include "RedpointEOSEditorModule.h"
#include "RedpointLibHydrogen.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(660091597, Redpoint::EOS::Editor::Config::Configurators)
{
using namespace ::Redpoint::EOS::Editor::Config;

void FNetworkingConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->EnableAntiCheat = false;
    Instance->TrustedPlatforms.Empty();
    Instance->TrustedClientPrivateKey.Empty();
    Instance->TrustedClientPublicKey.Empty();
    Instance->NetworkAuthenticationMode = ENetworkAuthenticationMode::Default;
    Instance->DedicatedServerDistributionMode = EDedicatedServersDistributionMode::DevelopersAndPlayers;
    Instance->EnableAutomaticEncryptionOnTrustedDedicatedServers = true;
}

void FNetworkingConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Reader.GetBool(TEXT("EnableAntiCheat"), Instance->EnableAntiCheat);
    Reader.GetString(TEXT("TrustedClientPublicKey"), Instance->TrustedClientPublicKey);
    Reader.GetString(
        TEXT("TrustedClientPrivateKey"),
        Instance->TrustedClientPrivateKey,
        DefaultEOSSection,
        EConfigurationFileType::TrustedClient);

    // Determine trusted clients based on which INI files have the private key.
    {
        TSet<FName> TrustedPlatformsSet;
        FString PlatformClientPrivateKey;
        for (const auto &PlatformName : GetConfidentialPlatformNames())
        {
            if (Reader.GetString(
                    TEXT("TrustedClientPrivateKey"),
                    PlatformClientPrivateKey,
                    DefaultEOSSection,
                    EConfigurationFileType::Engine,
                    PlatformName))
            {
                if (!PlatformClientPrivateKey.IsEmpty() &&
                    PlatformClientPrivateKey == Instance->TrustedClientPrivateKey)
                {
                    TrustedPlatformsSet.Add(PlatformName);
                }
            }
        }
        Instance->TrustedPlatforms.Empty();
        for (const auto &TrustedPlatform : TrustedPlatformsSet)
        {
            Instance->TrustedPlatforms.Add(TrustedPlatform);
        }
    }

    Reader.GetEnum<ENetworkAuthenticationMode>(
        TEXT("NetworkAuthenticationMode"),
        TEXT("ENetworkAuthenticationMode"),
        Instance->NetworkAuthenticationMode,
        DefaultEOSSection);

    Reader.GetEnum<EDedicatedServersDistributionMode>(
        TEXT("DedicatedServerDistributionMode"),
        TEXT("EDedicatedServersDistributionMode"),
        Instance->DedicatedServerDistributionMode,
        DefaultEOSSection,
        EConfigurationFileType::Editor);

    Reader.GetString(TEXT("DedicatedServerPublicKey"), Instance->DedicatedServerPublicKey);
    Reader.GetBool(
        TEXT("EnableAutomaticEncryptionOnTrustedDedicatedServers"),
        Instance->EnableAutomaticEncryptionOnTrustedDedicatedServers);
    Reader.GetString(
        TEXT("DedicatedServerClientId"),
        Instance->DedicatedServerClientId,
        DefaultEOSSection,
        EConfigurationFileType::Editor);
    Reader.GetString(
        TEXT("DedicatedServerClientSecret"),
        Instance->DedicatedServerClientSecret,
        DefaultEOSSection,
        EConfigurationFileType::Editor);
    Reader.GetString(
        TEXT("DedicatedServerPrivateKey"),
        Instance->DedicatedServerPrivateKey,
        DefaultEOSSection,
        EConfigurationFileType::Editor);
}

bool FNetworkingConfigurator::Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    bool bDidModify = false;

    if (Instance->DedicatedServerPublicKey.IsEmpty() && Instance->DedicatedServerPrivateKey.IsEmpty() &&
        Instance->DedicatedServerDistributionMode != EDedicatedServersDistributionMode::PlayersOnly)
    {
        // Generate a dedicated server public/private key pair.
        hydro_sign_keypair server_key_pair;
        hydro_sign_keygen(&server_key_pair);

        TArray<uint8> PrivateKey(server_key_pair.sk, sizeof(server_key_pair.sk));
        TArray<uint8> PublicKey(server_key_pair.pk, sizeof(server_key_pair.pk));

        Instance->DedicatedServerPrivateKey = FBase64::Encode(PrivateKey);
        Instance->DedicatedServerPublicKey = FBase64::Encode(PublicKey);

        // Mark as modified.
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Automatically generated public/private keypair for dedicated servers. This configuration setting "
                 "will be automatically updated."));
        bDidModify = true;
    }

    if (Instance->EnableAntiCheat &&
        (Instance->TrustedClientPrivateKey.IsEmpty() || Instance->TrustedClientPublicKey.IsEmpty()))
    {
        // Generate a trusted client public/private key pair.
        hydro_sign_keypair client_key_pair;
        hydro_sign_keygen(&client_key_pair);

        TArray<uint8> PrivateKey(client_key_pair.sk, sizeof(client_key_pair.sk));
        TArray<uint8> PublicKey(client_key_pair.pk, sizeof(client_key_pair.pk));

        Instance->TrustedClientPrivateKey = FBase64::Encode(PrivateKey);
        Instance->TrustedClientPublicKey = FBase64::Encode(PublicKey);

        // Mark as modified.
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Automatically generated public/private keypair for trusted clients. This configuration setting "
                 "will be automatically updated."));
        bDidModify = true;
    }

    return bDidModify;
}

void FNetworkingConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetBool(TEXT("EnableAntiCheat"), Instance->EnableAntiCheat);
    Writer.SetString(TEXT("TrustedClientPublicKey"), Instance->TrustedClientPublicKey);
    Writer.SetString(
        TEXT("TrustedClientPrivateKey"),
        Instance->TrustedClientPrivateKey,
        DefaultEOSSection,
        EConfigurationFileType::TrustedClient);

    {
        for (const auto &PlatformName : GetConfidentialPlatformNames())
        {
            if (Instance->TrustedPlatforms.Contains(PlatformName))
            {
                Writer.SetString(
                    TEXT("TrustedClientPrivateKey"),
                    Instance->TrustedClientPrivateKey,
                    DefaultEOSSection,
                    EConfigurationFileType::Engine,
                    PlatformName);
            }
            else
            {
                Writer.Remove(
                    TEXT("TrustedClientPrivateKey"),
                    DefaultEOSSection,
                    EConfigurationFileType::Engine,
                    PlatformName);
            }
        }
    }

    Writer.SetEnum<ENetworkAuthenticationMode>(
        TEXT("NetworkAuthenticationMode"),
        TEXT("ENetworkAuthenticationMode"),
        Instance->NetworkAuthenticationMode,
        DefaultEOSSection);

    Writer.SetEnum<EDedicatedServersDistributionMode>(
        TEXT("DedicatedServerDistributionMode"),
        TEXT("EDedicatedServersDistributionMode"),
        Instance->DedicatedServerDistributionMode,
        DefaultEOSSection,
        EConfigurationFileType::Editor);

    Writer.SetString(TEXT("DedicatedServerPublicKey"), Instance->DedicatedServerPublicKey);
    Writer.SetBool(
        TEXT("EnableAutomaticEncryptionOnTrustedDedicatedServers"),
        Instance->EnableAutomaticEncryptionOnTrustedDedicatedServers);

    bool bTrustedServersEnabled = Instance->NetworkAuthenticationMode != ENetworkAuthenticationMode::Off;

    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
        if (Instance->EnableAutomaticEncryptionOnTrustedDedicatedServers && bTrustedServersEnabled)
        {
            Writer.SetString(
                TEXT("EncryptionComponent"),
                TEXT("AESGCMHandlerComponent"),
                TEXT("PacketHandlerComponents"),
                EConfigurationFileType::Engine);
        }
        else if (!Instance->EnableAutomaticEncryptionOnTrustedDedicatedServers && bTrustedServersEnabled)
        {
            // Leave user override.
        }
        else
        {
            Writer.Remove(TEXT("EncryptionComponent"), TEXT("PacketHandlerComponents"), EConfigurationFileType::Engine);
        }
    }

    Writer.SetString(
        TEXT("DedicatedServerClientId"),
        Instance->DedicatedServerClientId,
        DefaultEOSSection,
        EConfigurationFileType::Editor);
    Writer.SetString(
        TEXT("DedicatedServerClientSecret"),
        Instance->DedicatedServerClientSecret,
        DefaultEOSSection,
        EConfigurationFileType::Editor);
    Writer.SetString(
        TEXT("DedicatedServerPrivateKey"),
        Instance->DedicatedServerPrivateKey,
        DefaultEOSSection,
        EConfigurationFileType::Editor);

    if (Instance->DedicatedServerDistributionMode == EDedicatedServersDistributionMode::DevelopersOnly)
    {
        Writer.SetString(
            TEXT("DedicatedServerClientId"),
            Instance->DedicatedServerClientId,
            DefaultEOSSection,
            EConfigurationFileType::DedicatedServer);
        Writer.SetString(
            TEXT("DedicatedServerClientSecret"),
            Instance->DedicatedServerClientSecret,
            DefaultEOSSection,
            EConfigurationFileType::DedicatedServer);
        Writer.SetString(
            TEXT("DedicatedServerPrivateKey"),
            Instance->DedicatedServerPrivateKey,
            DefaultEOSSection,
            EConfigurationFileType::DedicatedServer);
    }
    else
    {
        Writer.Remove(TEXT("DedicatedServerClientId"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
        Writer.Remove(TEXT("DedicatedServerClientSecret"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
        Writer.Remove(TEXT("DedicatedServerPrivateKey"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
    }

    // Remove legacy values for dedicated servers.
    Writer.Remove(TEXT("ClientId"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
    Writer.Remove(TEXT("ClientSecret"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
    Writer.Remove(TEXT("DedicatedServerActAsClientId"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
    Writer.Remove(TEXT("DedicatedServerActAsClientSecret"), DefaultEOSSection, EConfigurationFileType::DedicatedServer);
}

}

REDPOINT_EOS_CODE_GUARD_END()