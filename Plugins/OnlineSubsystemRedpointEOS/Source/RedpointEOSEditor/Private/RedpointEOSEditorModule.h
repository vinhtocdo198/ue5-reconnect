// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "CoreGlobals.h"
#include "Input/Reply.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/BlueprintFilterer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSEditor, All, All);

class FRedpointEOSEditorModule : public IModuleInterface
{
public:
    FRedpointEOSEditorModule();

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    virtual void PreUnloadCallback() override;

    bool Tick(float DeltaSeconds);

    void ProcessLogMessage(EOS_ELogLevel InLogLevel, const FString &Category, const FString &Message);
    void ProcessCustomSignal(const FString &Context, const FString &SignalId);

private:
    TSharedPtr<class FOnlineSubsystemEOSEditorConfig> SettingsInstance;
    TSharedPtr<class FEOSCheckEngine> CheckEngine;
    FTSTicker::FDelegateHandle TickerHandle;
    TSharedPtr<::Redpoint::EOS::Editor::FBlueprintFilterer> BlueprintFilterer;

    void ViewDocumentation();
    void ViewWebsite();

#if REDPOINT_EOS_IS_FREE_EDITION
    TSharedRef<class FLicenseAgreementWindow> LicenseAgreementWindow;
    TSharedRef<class FUpgradeWindow> UpgradePromptWindow;

    void ViewLicenseAgreementInBrowser();
    void UpgradeToPaid();
    static bool HasAcceptedLicenseAgreement();
#else
    void AccessSupport();
#endif

    static TSharedRef<SWidget> GenerateOnlineSettingsMenu(TSharedRef<class FUICommandList> InCommandList);

    void RegisterMenus();

#if PLATFORM_WINDOWS || PLATFORM_MAC
    static void SetLoginsEnabled(bool bEnabled);
    static void ResyncLoginsIfEnabled();
#endif

    TSharedPtr<class FUICommandList> PluginCommands;

#if REDPOINT_EOS_UE_5_5_OR_LATER
    int FabPlatformNextInstanceId;
    TArray<::Redpoint::EOS::API::FPlatformRefCountedHandle> FabPlatformHandles;
#endif
};