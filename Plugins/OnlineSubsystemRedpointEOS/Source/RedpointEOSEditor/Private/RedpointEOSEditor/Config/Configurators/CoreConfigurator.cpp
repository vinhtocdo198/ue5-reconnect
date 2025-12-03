// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/CoreConfigurator.h"

#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#include "RedpointEOSEditor/Config/PlatformHelpers.h"
#include "RedpointEOSEditorModule.h"
#include "XmlFile.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1426337698, Redpoint::EOS::Editor::Config::Configurators)
{

void FCoreConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->ProductName = TEXT("Product Name Not Set");
    Instance->ProductVersion = TEXT("0.0.0");
    Instance->RequireEpicGamesLauncher = false;
    Instance->ApiVersion = (EEOSApiVersion)0;
}

void FCoreConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Reader.GetString(TEXT("ProductName"), Instance->ProductName);
    Reader.GetString(TEXT("ProductVersion"), Instance->ProductVersion);
    Reader.GetString(TEXT("ProductId"), Instance->ProductId);
    Reader.GetString(TEXT("SandboxId"), Instance->SandboxId);
    Reader.GetString(TEXT("DeploymentId"), Instance->DeploymentId);
    Reader.GetString(TEXT("ClientId"), Instance->ClientId);
    Reader.GetString(TEXT("ClientSecret"), Instance->ClientSecret);
    Reader.GetEnum<EEOSApiVersion>(TEXT("ApiVersion"), TEXT("EEOSApiVersion"), Instance->ApiVersion);
#if REDPOINT_EOS_IS_FREE_EDITION
    if (Instance->GetClass() == UOnlineSubsystemEOSEditorConfigFreeEdition::StaticClass())
    {
        Reader.GetString(
            TEXT("FreeEditionLicenseKey"),
            Cast<UOnlineSubsystemEOSEditorConfigFreeEdition>(Instance)->FreeEditionLicenseKey);
    }
#endif

    Reader.GetBool(TEXT("RequireEpicGamesLauncher"), Instance->RequireEpicGamesLauncher);
    Reader.GetBool(TEXT("NeverShowEpicGamesOverlay"), Instance->NeverShowEpicGamesOverlay);
}

bool FCoreConfigurator::Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
        // Check that the default subsystem is correct.
        FString Value;
        GConfig->GetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), Value, GEngineIni);
        if (Value != TEXT("RedpointEOS"))
        {
#if REDPOINT_EOS_UE_5_5_OR_LATER
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Detected Redpoint EOS Online Framework is not set as the default online subsystem. This is a "
                     "bug, as the dynamic layer config system should be forcing to be the active online subsystem at "
                     "startup."));
#else
            UE_LOG(
                LogRedpointEOSEditor,
                Warning,
                TEXT("Detected Redpoint EOS Online Framework is not set as the default online subsystem. This "
                     "configuration setting will be automatically updated."));
            Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
                TEXT("Configuration"),
                TEXT("NotConfiguredForEOS"));
#endif
            return true;
        }

        // Check that EOSShared is configured with "has project binary".
        bool bHasProjectBinary;
        GConfig->GetBool(TEXT("EOSSDK"), TEXT("bHasProjectBinary"), bHasProjectBinary, GEngineIni);
        if (!bHasProjectBinary)
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Warning,
                TEXT(
                    "The EOSShared module is configured to copy an older version of the EOS SDK out during build. This "
                    "configuration setting will be automatically updated."));
            Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
                TEXT("Configuration"),
                TEXT("OldEOSSDKCopiedByBuildSystem"));
            return true;
        }

        // Check that iOS has entitlements configured for modern Xcode projects.
        FString PremadeIOSEntitlements;
        FString ExpectedConfiguration = TEXT("(FilePath=\"/Game/Build/IOS/Resources/SignInWithApple.entitlements\")");
        GConfig->GetString(
            TEXT("/Script/MacTargetPlatform.XcodeProjectSettings"),
            TEXT("PremadeIOSEntitlements"),
            PremadeIOSEntitlements,
            GEngineIni);
        if (PremadeIOSEntitlements != ExpectedConfiguration)
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Warning,
                TEXT("Detected iOS entitlements configuration is out-of-date for modern Xcode build system. This "
                     "configuration setting will be automatically updated."));
            return true;
        }

        // @note: In Unreal Engine 5.5 and later, we set the networking drivers via a dynamic config layer, so the INI
        // files should be empty.
#if !REDPOINT_EOS_UE_5_5_OR_LATER
        // Check that the net driver definitions are correct, and if they aren't
        // force a full save.
        TArray<FString> ExpectedNetDriverDefinitions;
        ExpectedNetDriverDefinitions.Add(
            "(DefName=\"GameNetDriver\",DriverClassName=\"/Script/"
            "RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/"
            "OnlineSubsystemUtils.IpNetDriver\")");
        ExpectedNetDriverDefinitions.Add(
            "(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/"
            "RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/"
            "OnlineSubsystemUtils.IpNetDriver\")");
        ExpectedNetDriverDefinitions.Add(
            "(DefName=\"DemoNetDriver\",DriverClassName=\"/Script/"
            "Engine.DemoNetDriver\",DriverClassNameFallback=\"/Script/Engine.DemoNetDriver\")");
        TArray<FString> ActualNetDriverDefinitions;
        GConfig->GetArray(
            TEXT("/Script/Engine.Engine"),
            TEXT("NetDriverDefinitions"),
            ActualNetDriverDefinitions,
            GEngineIni);
        if (ActualNetDriverDefinitions.Num() != ExpectedNetDriverDefinitions.Num())
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Warning,
                TEXT("Detected engine network drivers are not set to use the Redpoint EOS Online Framework network "
                     "drivers. This configuration setting will be automatically updated."));
            return true;
        }
        for (const auto &Expected : ExpectedNetDriverDefinitions)
        {
            if (!ActualNetDriverDefinitions.Contains(Expected))
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Warning,
                    TEXT("Detected engine network drivers are not set to use the Redpoint EOS Online Framework network "
                         "drivers. This configuration setting will be automatically updated."));
                return true;
            }
        }
        ActualNetDriverDefinitions.Empty();
        GConfig->GetArray(
            TEXT("/Script/Engine.GameEngine"),
            TEXT("NetDriverDefinitions"),
            ActualNetDriverDefinitions,
            GEngineIni);
        if (ActualNetDriverDefinitions.Num() != ExpectedNetDriverDefinitions.Num())
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Warning,
                TEXT("Detected engine network drivers are not set to use the Redpoint EOS Online Framework network "
                     "drivers. This configuration setting will be automatically updated."));
            return true;
        }
        for (const auto &Expected : ExpectedNetDriverDefinitions)
        {
            if (!ActualNetDriverDefinitions.Contains(Expected))
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Warning,
                    TEXT("Detected engine network drivers are not set to use the Redpoint EOS Online Framework network "
                         "drivers. This configuration setting will be automatically updated."));
                return true;
            }
        }
#endif
    }

    return false;
}

static void WriteXmlNodeHierarchy(const FXmlNode &Node, const FString &Indent, FString &Output)
{
    // Write the tag
    Output += Indent + FString::Printf(TEXT("<%s"), *Node.GetTag());
    for (const FXmlAttribute &Attribute : Node.GetAttributes())
    {
        FString EscapedValue = Attribute.GetValue();
        EscapedValue.ReplaceInline(TEXT("&"), TEXT("&amp;"), ESearchCase::CaseSensitive);
        EscapedValue.ReplaceInline(TEXT("\""), TEXT("&quot;"), ESearchCase::CaseSensitive);
        EscapedValue.ReplaceInline(TEXT("'"), TEXT("&apos;"), ESearchCase::CaseSensitive);
        EscapedValue.ReplaceInline(TEXT("<"), TEXT("&lt;"), ESearchCase::CaseSensitive);
        EscapedValue.ReplaceInline(TEXT(">"), TEXT("&gt;"), ESearchCase::CaseSensitive);
        Output += FString::Printf(TEXT(" %s=\"%s\""), *Attribute.GetTag(), *EscapedValue);
    }

    // Write the node contents
    const FXmlNode *FirstChildNode = Node.GetFirstChildNode();
    if (FirstChildNode == nullptr)
    {
        const FString &Content = Node.GetContent();
        if (Content.Len() == 0)
        {
            Output += TEXT(" />") LINE_TERMINATOR;
        }
        else
        {
            Output += TEXT(">") + Content + FString::Printf(TEXT("</%s>"), *Node.GetTag()) + LINE_TERMINATOR;
        }
    }
    else
    {
        Output += TEXT(">") LINE_TERMINATOR;
        for (const FXmlNode *ChildNode = FirstChildNode; ChildNode != nullptr; ChildNode = ChildNode->GetNextNode())
        {
            WriteXmlNodeHierarchy(*ChildNode, Indent + TEXT("\t"), Output);
        }
        Output += Indent + FString::Printf(TEXT("</%s>"), *Node.GetTag()) + LINE_TERMINATOR;
    }
}

void FCoreConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
#if !REDPOINT_EOS_UE_5_5_OR_LATER
        // Set up network driver array.
        TArray<FString> NetDriverDefinitions;
        NetDriverDefinitions.Add("(DefName=\"GameNetDriver\",DriverClassName=\"/Script/"
                                 "RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/"
                                 "OnlineSubsystemUtils.IpNetDriver\")");
        NetDriverDefinitions.Add("(DefName=\"BeaconNetDriver\",DriverClassName=\"/Script/"
                                 "RedpointEOSNetworking.RedpointEOSNetDriver\",DriverClassNameFallback=\"/Script/"
                                 "OnlineSubsystemUtils.IpNetDriver\")");
        NetDriverDefinitions.Add("(DefName=\"DemoNetDriver\",DriverClassName=\"/Script/"
                                 "Engine.DemoNetDriver\",DriverClassNameFallback=\"/Script/Engine.DemoNetDriver\")");
#endif

        // Prevent Epic's implementation of the EOS SDK from copying old binaries out.
        Writer.SetBool(TEXT("bHasProjectBinary"), true, TEXT("EOSSDK"), EConfigurationFileType::Engine);

        // Core settings required for EOS and networking.
#if !REDPOINT_EOS_UE_5_5_OR_LATER
        Writer.SetString(
            TEXT("DefaultPlatformService"),
            TEXT("RedpointEOS"),
            TEXT("OnlineSubsystem"),
            EConfigurationFileType::Engine);
        Writer.ReplaceArray(
            TEXT("NetDriverDefinitions"),
            NetDriverDefinitions,
            TEXT("/Script/Engine.Engine"),
            EConfigurationFileType::Engine);
        Writer.ReplaceArray(
            TEXT("NetDriverDefinitions"),
            NetDriverDefinitions,
            TEXT("/Script/Engine.GameEngine"),
            EConfigurationFileType::Engine);
        for (const auto &Platform : GetAllPlatformNames())
        {
            Writer.SetString(
                TEXT("DefaultPlatformService"),
                TEXT("RedpointEOS"),
                TEXT("OnlineSubsystem"),
                EConfigurationFileType::Engine,
                Platform);
            Writer.ReplaceArray(
                TEXT("NetDriverDefinitions"),
                NetDriverDefinitions,
                TEXT("/Script/Engine.Engine"),
                EConfigurationFileType::Engine,
                Platform);
            Writer.ReplaceArray(
                TEXT("NetDriverDefinitions"),
                NetDriverDefinitions,
                TEXT("/Script/Engine.GameEngine"),
                EConfigurationFileType::Engine,
                Platform);
        }
#endif

        // IOS required settings.
        Writer.SetString(
            TEXT("MinimumiOSVersion"),
            TEXT("IOS_13"),
            TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"),
            EConfigurationFileType::Engine,
            FName(TEXT("IOS")));
        Writer.SetBool(
            TEXT("bEnabled"),
            false,
            TEXT("OnlineSubsystemApple"),
            EConfigurationFileType::Engine,
            FName(TEXT("IOS")));

        // For legacy Xcode build scenarios on iOS:
        // - Turn on the "bEnableSignInWithAppleSupport" which causes the entitlement to be written out.
        Writer.SetBool(
            TEXT("bEnableSignInWithAppleSupport"),
            true,
            TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"),
            EConfigurationFileType::Engine,
            FName(TEXT("IOS")));

        // For modern Xcode build scenarios on iOS:
        // - Set the premade iOS entitlements so we can request the "Sign in with Apple" entitlement.
        Writer.SetString(
            TEXT("PremadeIOSEntitlements"),
            TEXT("(FilePath=\"/Game/Build/IOS/Resources/SignInWithApple.entitlements\")"),
            TEXT("/Script/MacTargetPlatform.XcodeProjectSettings"),
            EConfigurationFileType::Engine);
        // - Set the contents of SignInWithApple.entitlements.
        {
            TMap<FString, FString> Entitlements;
            Entitlements.Add(TEXT("com.apple.developer.applesignin"), TEXT("Default"));

            TSharedRef<FXmlFile> XmlFile = MakeShared<FXmlFile>(
                TEXT("<plist version=\"1.0\"><dict></dict></plist>"),
                EConstructMethod::ConstructFromBuffer);
            FXmlNode *PlistNode = XmlFile->GetRootNode();
            FXmlNode *DictNode = PlistNode->FindChildNode(TEXT("dict"));
            for (const auto &KV : Entitlements)
            {
                DictNode->AppendChildNode(TEXT("key"), KV.Key);
                if (KV.Value == TEXT("true") || KV.Value == TEXT("false"))
                {
                    DictNode->AppendChildNode(KV.Value);
                }
                else
                {
                    DictNode->AppendChildNode(TEXT("string"), KV.Value);
                }
            }
            auto EntitlementsDirectory = FPaths::ProjectDir() / TEXT("Build") / TEXT("IOS") / TEXT("Resources");
            auto EntitlementsFile = EntitlementsDirectory / TEXT("SignInWithApple.entitlements");

            FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*EntitlementsDirectory);

            FString Xml = TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
                LINE_TERMINATOR TEXT("<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
                                     "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">") LINE_TERMINATOR;
            WriteXmlNodeHierarchy(*XmlFile->GetRootNode(), TEXT(""), Xml);

            FString ExistingContent;
            if (FPaths::FileExists(EntitlementsFile))
            {
                FFileHelper::LoadFileToString(ExistingContent, *EntitlementsFile);
            }
            if (ExistingContent != Xml)
            {
                FFileHelper::SaveStringToFile(
                    Xml,
                    *EntitlementsFile,
                    FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
            }
        }

        // Steam networking settings.
        Writer.SetBool(
            TEXT("bUseSteamNetworking"),
            false,
            TEXT("OnlineSubsystemSteam"),
            EConfigurationFileType::Engine,
            TEXT("Windows"));
        Writer.SetBool(
            TEXT("bUseSteamNetworking"),
            false,
            TEXT("OnlineSubsystemSteam"),
            EConfigurationFileType::Engine,
            TEXT("Mac"));
        Writer.SetBool(
            TEXT("bUseSteamNetworking"),
            false,
            TEXT("OnlineSubsystemSteam"),
            EConfigurationFileType::Engine,
            TEXT("Linux"));
    }

    // Settings configured through Project Settings.
    Writer.SetString(TEXT("ProductName"), Instance->ProductName);
    Writer.SetString(TEXT("ProductVersion"), Instance->ProductVersion);
    Writer.SetString(TEXT("ProductId"), Instance->ProductId);
    Writer.SetString(TEXT("SandboxId"), Instance->SandboxId);
    Writer.SetString(TEXT("DeploymentId"), Instance->DeploymentId);
    Writer.SetString(TEXT("ClientId"), Instance->ClientId);
    Writer.SetString(TEXT("ClientSecret"), Instance->ClientSecret);
    Writer.SetEnum<EEOSApiVersion>(TEXT("ApiVersion"), TEXT("EEOSApiVersion"), Instance->ApiVersion);
#if REDPOINT_EOS_IS_FREE_EDITION
    if (Instance->GetClass() == UOnlineSubsystemEOSEditorConfigFreeEdition::StaticClass())
    {
        Writer.SetString(
            TEXT("FreeEditionLicenseKey"),
            Cast<UOnlineSubsystemEOSEditorConfigFreeEdition>(Instance)->FreeEditionLicenseKey);
    }
#endif

    // Epic Games Store settings.
    Writer.SetBool(TEXT("RequireEpicGamesLauncher"), Instance->RequireEpicGamesLauncher);
    Writer.SetBool(TEXT("NeverShowEpicGamesOverlay"), Instance->NeverShowEpicGamesOverlay);
}

}

REDPOINT_EOS_CODE_GUARD_END()