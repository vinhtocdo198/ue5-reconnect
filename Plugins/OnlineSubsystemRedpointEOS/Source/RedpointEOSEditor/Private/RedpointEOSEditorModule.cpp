// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorModule.h"

#include "../Private/OnlinePIESettings.h"
#include "./Checks/EOSCheckEngine.h"
#include "CoreGlobals.h"
#include "DesktopPlatformModule.h"
#include "DetailCustomizations.h"
#include "Editor.h"
#include "EditorDirectories.h"
#include "HAL/FileManager.h"
#include "IDesktopPlatform.h"
#include "ISettingsModule.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"
#include "LevelEditor.h"
#include "Misc/FeedbackContext.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "ObjectEditorUtils.h"
#include "OnlineSubsystemEOSEditorCommands.h"
#include "OnlineSubsystemEOSEditorConfig.h"
#include "OnlineSubsystemEOSEditorConfigDetails.h"
#include "OnlineSubsystemEOSEditorPrefs.h"
#include "OnlineSubsystemEOSEditorStyle.h"
#include "OnlineSubsystemRedpointEOS/Public/OnlineSubsystemRedpointEOSModule.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSEditor/DevAuthToolLauncher.h"
#include "Settings/EditorExperimentalSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include <regex>
#include <string>
#if REDPOINT_EOS_IS_FREE_EDITION
#include "./SlateWindows/LicenseAgreementWindow.h"
#include "./SlateWindows/UpgradeWindow.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IMainFrameModule.h"
#endif
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "OnlineAccountStoredCredentials.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSSharedModule.h"
#endif

DEFINE_LOG_CATEGORY(LogRedpointEOSEditor);

static const FName OnlineSubsystemEOSLicenseAgreementTabName("License Agreement");

#define LOCTEXT_NAMESPACE "OnlineSubsystemEOSEditorModule"

FRedpointEOSEditorModule::FRedpointEOSEditorModule()
    : SettingsInstance(nullptr)
    , CheckEngine(nullptr)
    , TickerHandle()
    , BlueprintFilterer()
#if REDPOINT_EOS_IS_FREE_EDITION
    , LicenseAgreementWindow(MakeShared<FLicenseAgreementWindow>())
    , UpgradePromptWindow(MakeShared<FUpgradeWindow>())
#endif
    , PluginCommands(nullptr)
#if REDPOINT_EOS_UE_5_5_OR_LATER
    , FabPlatformNextInstanceId(1)
    , FabPlatformHandles()
#endif
{
}

void FRedpointEOSEditorModule::StartupModule()
{
    checkf(!this->CheckEngine.IsValid(), TEXT("FRedpointEOSEditorModule::StartupModule already called."));

    FModuleManager &ModuleManager = FModuleManager::Get();

#if REDPOINT_EOS_UE_5_5_OR_LATER
    ModuleManager.LoadModuleChecked<FRedpointEOSSharedModule>("RedpointEOSShared").OnPlatformCreateRequested =
        TDelegate<EOS_HPlatform(EOS_Platform_Options & PlatformOptions)>::CreateLambda(
            [this](EOS_Platform_Options &PlatformOptions) -> EOS_HPlatform {
                using namespace ::Redpoint::EOS::Core;
                auto Handle = IInstancePool::Pool().CreateWithPlatform(
                    FName(*FString::Printf(TEXT("Fab%d"), this->FabPlatformNextInstanceId++)),
                    PlatformOptions);
                this->FabPlatformHandles.Add(Handle);
                return Handle->Instance()->Handle();
            });
#endif

#if REDPOINT_EOS_UE_5_5_OR_LATER
    if (ModuleManager.GetModule("OnlineSubsystemEOS") != nullptr)
    {
        FText ConflictWarningTitle = LOCTEXT("ConflictTitle", "Error: Conflicting plugins detected");

        FMessageDialog::Open(
            EAppMsgType::Ok,
            LOCTEXT(
                "ConflictDescription",
                "This project has the Epic \"Online Subsystem EOS\" plugin enabled. This plugin conflicts with the "
                "Redpoint EOS Online Framework plugin, and it must be disabled from the Plugins window.\n\nThe "
                "Redpoint EOS Online Framework plugin will not work until you disable this plugin and "
                "restart the editor."),
            ConflictWarningTitle);
    }
#else
    if (ModuleManager.GetModule("OnlineSubsystemEOS") != nullptr || ModuleManager.GetModule("EOSShared") != nullptr)
    {
        FText ConflictWarningTitle = LOCTEXT("ConflictTitle", "Error: Conflicting plugins detected");

        FMessageDialog::Open(
            EAppMsgType::Ok,
            LOCTEXT(
                "ConflictDescription",
                "This project has the Epic \"Online Subsystem EOS\" or \"EOS Shared\" plugin enabled. These plugins "
                "conflict with the Redpoint EOS Online Framework plugin, and they must be disabled from the Plugins "
                "window.\n\nThe Redpoint EOS Online Framework plugin will not work until you disable these plugins and "
                "restart the editor."),
#if REDPOINT_EOS_UE_5_3_OR_LATER
            ConflictWarningTitle
#else
            &ConflictWarningTitle
#endif
        );
    }
#endif

    // Create check engine.
    this->CheckEngine = MakeShared<FEOSCheckEngine>();
    auto RuntimeModule = (FOnlineSubsystemRedpointEOSModule *)ModuleManager.GetModule("OnlineSubsystemRedpointEOS");
    if (RuntimeModule != nullptr)
    {
        Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().AddLambda(
            [this](const FString &Context, const FString &SignalId) {
                this->ProcessCustomSignal(Context, SignalId);
            });
    }
    Redpoint::EOS::Core::FModule::GetModuleChecked().SetLogHandler(
        Redpoint::EOS::Core::FOnLogForwardedForEditor::CreateLambda(
            [this](int32_t Level, const FString &Category, const FString &Message) {
                this->ProcessLogMessage((EOS_ELogLevel)Level, Category, Message);
            }));

    // Set up editor styles & setup window.
    SettingsInstance = MakeShared<FOnlineSubsystemEOSEditorConfig>();
    SettingsInstance->RegisterSettings();
    FOnlineSubsystemEOSEditorStyle::Initialize();
    FOnlineSubsystemEOSEditorStyle::ReloadTextures();
    FOnlineSubsystemEOSEditorCommands::Register();

    // Map commands to actions.
    PluginCommands = MakeShareable(new FUICommandList);
    PluginCommands->MapAction(
        FOnlineSubsystemEOSEditorCommands::Get().ViewDocumentation,
        FExecuteAction::CreateRaw(this, &FRedpointEOSEditorModule::ViewDocumentation),
        FCanExecuteAction());
#if REDPOINT_EOS_IS_FREE_EDITION
    PluginCommands->MapAction(
        FOnlineSubsystemEOSEditorCommands::Get().ViewLicenseAgreementInBrowser,
        FExecuteAction::CreateRaw(this, &FRedpointEOSEditorModule::ViewLicenseAgreementInBrowser),
        FCanExecuteAction());
    PluginCommands->MapAction(
        FOnlineSubsystemEOSEditorCommands::Get().UpgradeToPaid,
        FExecuteAction::CreateRaw(this, &FRedpointEOSEditorModule::UpgradeToPaid),
        FCanExecuteAction());
#else
    PluginCommands->MapAction(
        FOnlineSubsystemEOSEditorCommands::Get().AccessSupport,
        FExecuteAction::CreateRaw(this, &FRedpointEOSEditorModule::AccessSupport),
        FCanExecuteAction());
#endif
    PluginCommands->MapAction(
        FOnlineSubsystemEOSEditorCommands::Get().ViewWebsite,
        FExecuteAction::CreateRaw(this, &FRedpointEOSEditorModule::ViewWebsite),
        FCanExecuteAction());

    // When the toolbar menus startup, let us register our menu items.
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRedpointEOSEditorModule::RegisterMenus));

#if PLATFORM_WINDOWS || PLATFORM_MAC
    // Synchronise logins now so that we start the Developer Authentication Tool if needed.
    FRedpointEOSEditorModule::ResyncLoginsIfEnabled();

    FEditorDelegates::PreBeginPIE.AddLambda([](const bool bIsSimulating) {
        // Synchronise before PIE starts every time.
        FRedpointEOSEditorModule::ResyncLoginsIfEnabled();
    });
#endif

#if REDPOINT_EOS_IS_FREE_EDITION
    // Show the license agreement.
    if (!HasAcceptedLicenseAgreement())
    {
        IMainFrameModule &MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
        if (MainFrame.IsWindowInitialized())
        {
            this->LicenseAgreementWindow->Open();
        }
        else
        {
            // NOLINTNEXTLINE(performance-unnecessary-value-param)
            MainFrame.OnMainFrameCreationFinished().AddLambda([this](TSharedPtr<SWindow>, bool) {
                this->LicenseAgreementWindow->Open();
            });
        }
    }

    // Fetch latest version number.
    auto HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(TEXT("https://licensing.redpoint.games/api/product-version/eos-online-subsystem-free"));
    HttpRequest->SetVerb(TEXT("GET"));
    HttpRequest->SetHeader(TEXT("Content-Length"), TEXT("0"));
    HttpRequest->OnProcessRequestComplete().BindLambda(
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        [this](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
            if (HttpResponse.IsValid())
            {
                FString VersionNumberList = HttpResponse->GetContentAsString();

#if PLATFORM_WINDOWS
                FString VersionPath = FString(FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"))) /
                                      ".eos-free-edition-latest-version";
#else
                FString VersionPath =
                    FString(FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"))) / ".eos-free-edition-latest-version";
#endif
                FFileHelper::SaveStringToFile(VersionNumberList.TrimStartAndEnd(), *VersionPath);

#if defined(REDPOINT_EOS_BUILD_VERSION_NAME)
                FString CurrentVersion = FString(REDPOINT_EOS_BUILD_VERSION_NAME);
                TArray<FString> AllowedVersions;
                VersionNumberList.TrimStartAndEnd().ParseIntoArrayLines(AllowedVersions, true);
                if (AllowedVersions.Num() > 0)
                {
                    bool bAllowedVersionFound = false;
                    for (auto AllowedVersion : AllowedVersions)
                    {
                        if (AllowedVersion == CurrentVersion)
                        {
                            bAllowedVersionFound = true;
                            break;
                        }
                    }

                    if (!bAllowedVersionFound)
                    {
                        this->UpgradePromptWindow->Open();
                    }
                }
#endif
            }
        });
    HttpRequest->ProcessRequest();
#endif

    // Add custom detail for the EOS Online Framework project settings page.
    FPropertyEditorModule &PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        "OnlineSubsystemEOSEditorConfig",
        FOnGetDetailCustomizationInstance::CreateStatic(&FOnlineSubsystemEOSEditorConfigDetails::MakeInstance));

    // Register ticker.
    this->TickerHandle =
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FRedpointEOSEditorModule::Tick));

    // Register class filter to filter out "Online Subsystem Blueprints" types from the editor if desired.
    this->BlueprintFilterer =
        ::Redpoint::EOS::Core::Utils::MakeSharedWithEvents<::Redpoint::EOS::Editor::FBlueprintFilterer>();
}

void FRedpointEOSEditorModule::ShutdownModule()
{
    // Unregister previously registered class filters if possible.
    this->BlueprintFilterer.Reset();

    FTSTicker::GetCoreTicker().RemoveTicker(this->TickerHandle);
    this->TickerHandle.Reset();
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FOnlineSubsystemEOSEditorStyle::Shutdown();
    FOnlineSubsystemEOSEditorCommands::Unregister();
    SettingsInstance->UnregisterSettings();
}

void FRedpointEOSEditorModule::PreUnloadCallback()
{
#if REDPOINT_EOS_UE_5_5_OR_LATER
    // This ensures that in-flight calls Fab is making to the EOS SDK are cancelled prior to the EOS SDK being unloaded.
    this->FabPlatformHandles.Empty();
#endif
}

bool FRedpointEOSEditorModule::Tick(float DeltaSeconds)
{
    this->CheckEngine->Tick(DeltaSeconds);
    return true;
}

void FRedpointEOSEditorModule::ProcessLogMessage(
    EOS_ELogLevel InLogLevel,
    const FString &Category,
    const FString &Message)
{
    this->CheckEngine->ProcessLogMessage(InLogLevel, Category, Message);
}

void FRedpointEOSEditorModule::ProcessCustomSignal(const FString &Context, const FString &SignalId)
{
    this->CheckEngine->ProcessCustomSignal(Context, SignalId);
}

void FRedpointEOSEditorModule::ViewDocumentation()
{
    FPlatformProcess::LaunchURL(TEXT("https://docs.redpoint.games/eos-online-subsystem/docs/"), nullptr, nullptr);
}

void FRedpointEOSEditorModule::ViewWebsite()
{
    FPlatformProcess::LaunchURL(TEXT("https://docs.redpoint.games/eos-online-subsystem/"), nullptr, nullptr);
}

#if REDPOINT_EOS_IS_FREE_EDITION
void FRedpointEOSEditorModule::ViewLicenseAgreementInBrowser()
{
    if (!HasAcceptedLicenseAgreement())
    {
        // User hasn't accepted in editor yet, so they need to be able to click the I Accept button instead.
        this->LicenseAgreementWindow->Open();
        return;
    }

    FPlatformProcess::LaunchURL(TEXT("https://redpoint.games/eos-online-subsystem-free-eula/"), nullptr, nullptr);
}

void FRedpointEOSEditorModule::UpgradeToPaid()
{
    FPlatformProcess::LaunchURL(TEXT("https://unreal.tools/eos"), nullptr, nullptr);
}

bool FRedpointEOSEditorModule::HasAcceptedLicenseAgreement()
{
#if PLATFORM_WINDOWS
    FString FlagPath = FString(FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"))) / ".eos-free-edition-agreed";
#else
    FString FlagPath = FString(FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"))) / ".eos-free-edition-agreed";
#endif
    return FPaths::FileExists(FlagPath);
}
#else
void FRedpointEOSEditorModule::AccessSupport()
{
    FPlatformProcess::LaunchURL(
        TEXT("https://docs.redpoint.games/eos-online-subsystem/docs/accessing_support"),
        nullptr,
        nullptr);
}
#endif

TSharedRef<SWidget> FRedpointEOSEditorModule::GenerateOnlineSettingsMenu(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FUICommandList> InCommandList)
{
    FToolMenuContext MenuContext(InCommandList);

    return UToolMenus::Get()->GenerateWidget(
        "LevelEditor.LevelEditorToolBar.LevelToolbarOnlineSubsystemEOS",
        MenuContext);
}

void FRedpointEOSEditorModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    FToolMenuOwnerScoped OwnerScoped(this);

    {
        UToolMenu *Menu =
            UToolMenus::Get()->RegisterMenu("LevelEditor.LevelEditorToolBar.LevelToolbarOnlineSubsystemEOS");

        struct Local
        {
            static void OpenSettings(FName ContainerName, FName CategoryName, FName SectionName)
            {
                FModuleManager::LoadModuleChecked<ISettingsModule>("Settings")
                    .ShowViewer(ContainerName, CategoryName, SectionName);
            }
        };

        {
            FToolMenuSection &Section =
                Menu->AddSection("OnlineSection", LOCTEXT("OnlineSection", "Epic Online Services"));

            Section.AddMenuEntry(
                "OnlineSettings",
                LOCTEXT("OnlineSettingsMenuLabel", "Settings..."),
                LOCTEXT("OnlineSettingsMenuToolTip", "Configure the Redpoint EOS Online Framework plugin."),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateStatic(
                    &Local::OpenSettings,
                    FName("Project"),
                    FName("Game"),
                    FName("Epic Online Services"))));
        }

#if PLATFORM_WINDOWS || PLATFORM_MAC
        {
            FToolMenuSection &Section =
                Menu->AddSection("AuthenticationSection", LOCTEXT("AuthenticationSection", "Authentication"));
            Section.AddEntry(FToolMenuEntry::InitMenuEntry(
                "StartDevAuthTool",
                LOCTEXT("StartDevAuthTool", "Start Developer Authentication Tool"),
                LOCTEXT(
                    "StartDevAuthToolTooltip",
                    "Launches the Developer Authentication Tool, which can be used to automatically sign each "
                    "play-in-editor instance into a different Epic Games account."),
                FSlateIcon(),
                FUIAction(
                    FExecuteAction::CreateLambda([]() {
                        using namespace ::Redpoint::EOS::Editor;
                        FDevAuthToolLauncher::Get().Launch(true);
                    }),
                    FCanExecuteAction::CreateLambda([]() {
                        using namespace ::Redpoint::EOS::Editor;
                        return !FDevAuthToolLauncher::Get().IsDevAuthToolRunning();
                    }),
                    FGetActionCheckState(),
                    FIsActionButtonVisible()),
                EUserInterfaceActionType::Button,
                "StartDevAuthToolEAS"));
            Section.AddEntry(FToolMenuEntry::InitMenuEntry(
                "ToggleAutoLogin",
                LOCTEXT("AutoLogin", "Login before play-in-editor"),
                LOCTEXT(
                    "AutoLoginTooltip",
                    "If enabled, the editor will authenticate each play-in-editor instance against the Developer "
                    "Authentication Tool before play starts. This is recommended if you are testing anything other "
                    "than your login screen."),
                FSlateIcon(),
                FUIAction(
                    FExecuteAction::CreateLambda([]() {
                        IOnlineSubsystemUtils *Utils = Online::GetUtils();
                        FRedpointEOSEditorModule::SetLoginsEnabled(!Utils->IsOnlinePIEEnabled());
                        FRedpointEOSEditorModule::ResyncLoginsIfEnabled();
                    }),
                    FCanExecuteAction::CreateLambda([]() {
                        using namespace ::Redpoint::EOS::Editor;
                        return FDevAuthToolLauncher::Get().IsDevAuthToolRunning();
                    }),
                    FGetActionCheckState::CreateLambda([]() {
                        using namespace ::Redpoint::EOS::Editor;
                        IOnlineSubsystemUtils *Utils = Online::GetUtils();
                        return (Utils->IsOnlinePIEEnabled() && FDevAuthToolLauncher::Get().IsDevAuthToolRunning())
                                   ? ECheckBoxState::Checked
                                   : ECheckBoxState::Unchecked;
                    }),
                    FIsActionButtonVisible()),
                EUserInterfaceActionType::ToggleButton,
                "ToggleAutoLoginEAS"));
        }
#endif

        {
            FToolMenuSection &Section =
                Menu->AddSection("PluginSection", LOCTEXT("PluginSection", "Redpoint EOS Online Framework Plugin"));

            Section.AddMenuEntry(FOnlineSubsystemEOSEditorCommands::Get().ViewDocumentation);
#if REDPOINT_EOS_IS_FREE_EDITION
            Section.AddMenuEntry(FOnlineSubsystemEOSEditorCommands::Get().ViewLicenseAgreementInBrowser);
            Section.AddMenuEntry(FOnlineSubsystemEOSEditorCommands::Get().UpgradeToPaid);
#else
            Section.AddMenuEntry(FOnlineSubsystemEOSEditorCommands::Get().AccessSupport);
#endif
            Section.AddMenuEntry(FOnlineSubsystemEOSEditorCommands::Get().ViewWebsite);
        }
    }

    {
        UToolMenu *Toolbar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
        {
            FToolMenuSection &SettingsSection = Toolbar->AddSection("EOSSettings");
            {
                FUIAction Act;
                Act.IsActionVisibleDelegate = FIsActionButtonVisible::CreateLambda([]() {
                    const UOnlineSubsystemEOSEditorPrefs *Prefs = GetDefault<UOnlineSubsystemEOSEditorPrefs>();
                    return !Prefs || !Prefs->bHideDropdownInEditorToolbar;
                });
                FToolMenuEntry ComboButton = FToolMenuEntry::InitComboButton(
                    "LevelToolbarOnlineSubsystemEOS",
                    Act,
                    FOnGetContent::CreateStatic(
                        &FRedpointEOSEditorModule::GenerateOnlineSettingsMenu,
                        PluginCommands.ToSharedRef()),
                    LOCTEXT("SettingsCombo", "Epic Online Services"),
                    LOCTEXT("SettingsCombo_ToolTip", "Manage Epic Online Services."),
                    FSlateIcon(FOnlineSubsystemEOSEditorStyle::GetStyleSetName(), "LevelEditor.OnlineSettings"),
                    false,
                    "LevelToolbarOnlineSubsystemEOS");
                ComboButton.StyleNameOverride = "CalloutToolbar";
                SettingsSection.AddEntry(ComboButton);
            }
        }
    }
}

#if PLATFORM_WINDOWS || PLATFORM_MAC
void FRedpointEOSEditorModule::SetLoginsEnabled(bool bEnabled)
{
    IOnlineSubsystemUtils *Utils = Online::GetUtils();
    Utils->SetShouldTryOnlinePIE(true);

    UE_LOG(
        LogRedpointEOSEditor,
        Verbose,
        TEXT("Setting PIE logins enabled to %s"),
        bEnabled ? TEXT("true") : TEXT("false"));

#if REDPOINT_EOS_UE_5_1_OR_LATER
    UClass *PIESettingsClass =
        FindObject<UClass>(FTopLevelAssetPath(TEXT("/Script/OnlineSubsystemUtils"), TEXT("OnlinePIESettings")));
#else
    UClass *PIESettingsClass = FindObject<UClass>(ANY_PACKAGE, TEXT("OnlinePIESettings"));
#endif
    UObject *PIESettings = PIESettingsClass->GetDefaultObject(true);
    PIESettings->LoadConfig();

    FProperty *Prop = PIESettingsClass->FindPropertyByName("bOnlinePIEEnabled");
    bool *SourceAddr = Prop->ContainerPtrToValuePtr<bool>(PIESettings);
    *SourceAddr = bEnabled;

    PIESettings->SaveConfig();
}

void FRedpointEOSEditorModule::ResyncLoginsIfEnabled()
{
    using namespace ::Redpoint::EOS::Editor;

    const ULevelEditorPlaySettings *PlayInSettings = GetDefault<ULevelEditorPlaySettings>();
    const UEditorExperimentalSettings *ExperimentalSettings = GetDefault<UEditorExperimentalSettings>();
    int32 PlayNumberOfClients(0);
    if (!PlayInSettings->GetPlayNumberOfClients(PlayNumberOfClients))
    {
        return;
    }

    IOnlineSubsystemUtils *Utils = Online::GetUtils();
    if (Utils->IsOnlinePIEEnabled())
    {
        if (!FDevAuthToolLauncher::Get().IsDevAuthToolRunning())
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Verbose,
                TEXT("Launching the Developer Authentication Tool because it was not running"));

            using namespace ::Redpoint::EOS::Editor;
            FDevAuthToolLauncher::Get().Launch(false);
        }

        UE_LOG(
            LogRedpointEOSEditor,
            Verbose,
            TEXT("Synchronising automatic login credentials because PIE automatic login is enabled"));

#if REDPOINT_EOS_UE_5_1_OR_LATER
        UClass *PIESettingsClass =
            FindObject<UClass>(FTopLevelAssetPath(TEXT("/Script/OnlineSubsystemUtils"), TEXT("OnlinePIESettings")));
#else
        UClass *PIESettingsClass = FindObject<UClass>(ANY_PACKAGE, TEXT("OnlinePIESettings"));
#endif
        UObject *PIESettings = PIESettingsClass->GetDefaultObject(true);
        PIESettings->LoadConfig();

        FProperty *Prop = PIESettingsClass->FindPropertyByName("Logins");
#if REDPOINT_EOS_UE_5_5_OR_LATER
        TArray<FOnlineAccountStoredCredentials> *Arr =
            Prop->ContainerPtrToValuePtr<TArray<FOnlineAccountStoredCredentials>>(PIESettings);
#else
        TArray<FPIELoginSettingsInternal> *Arr =
            Prop->ContainerPtrToValuePtr<TArray<FPIELoginSettingsInternal>>(PIESettings);
#endif

        if (ExperimentalSettings->bAllowLateJoinInPIE)
        {
            // Add additional logins so that late join can work.
            PlayNumberOfClients = FMath::Max(PlayNumberOfClients * 2, 8);
        }

#if REDPOINT_EOS_UE_5_5_OR_LATER
        using FCredentials = FOnlineAccountStoredCredentials;
#else
        using FCredentials = FPIELoginSettingsInternal;
#endif

        while (Arr->Num() < PlayNumberOfClients)
        {
            Arr->Add(FCredentials());
        }
        for (int i = 0; i < PlayNumberOfClients; i++)
        {
            auto &Settings = (*Arr)[i];
            Settings.Id = TEXT("DEV_TOOL_AUTO_LOGIN");
            Settings.Token = TEXT("DEV_TOOL_AUTO_LOGIN");
            Settings.Type = TEXT("DEV_TOOL_AUTO_LOGIN");
#if REDPOINT_EOS_UE_5_5_OR_LATER
            Settings.Encrypt();
#endif
        }
        while (Arr->Num() > PlayNumberOfClients)
        {
            Arr->RemoveAt(Arr->Num() - 1);
        }

        PIESettings->SaveConfig();
    }
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedpointEOSEditorModule, RedpointEOSEditor)