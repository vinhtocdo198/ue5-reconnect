// Copyright June Rhodes. All Rights Reserved.

#include "RedpointGameInstance.h"

#include "Blueprint/GameViewportSubsystem.h"
#include "Containers/Ticker.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Level.h"
#include "GameFramework/OnlineSession.h"
#include "HAL/PlatformProcess.h"
#include "LogRedpointEOSFramework.h"
#include "Misc/CoreDelegates.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointStartupScreen.h"
#include "TickTaskManagerInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointGameInstance::URedpointGameInstance(const FObjectInitializer &ObjectInitializer)
{
    this->StartupScreen = FSoftClassPath(TEXT("/Script/RedpointEOSFramework.RedpointDefaultStartupScreen"));
    this->OnlineSessionClass = UOnlineSession::StaticClass();
}

void URedpointGameInstance::StartGameInstance()
{
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Core;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Identity;
    using namespace ::Redpoint::EOS::Identity::Requests;

    if (!this->bEnsureAlwaysSignedIn)
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("bEnsureAlwaysSignedIn is turned off on the game instance; not blocking startup on sign-in process."));
        Super::StartGameInstance();
        return;
    }

    // @note: This only fires in standalone game and *not* the editor.

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        UE_LOG(LogRedpointEOSFramework, Warning, TEXT("Unable to find platform handle on startup!"));
        Super::StartGameInstance();
        return;
    }

    // Display the startup widget if we have one.
    TSubclassOf<UUserWidget> WidgetClass = this->StartupScreen.TryLoadClass<UUserWidget>();
    if (WidgetClass != nullptr && !IsValid(this->StartupScreenWidget))
    {
        this->StartupScreenWidget = NewObject<UUserWidget>(this, WidgetClass, NAME_None, RF_Transactional);
        if (UGameViewportSubsystem *GameViewportSubsystem = UGameViewportSubsystem::Get(GetWorld()))
        {
            FGameViewportWidgetSlot ViewportSlot;
            ViewportSlot.ZOrder = 1000;
            GameViewportSubsystem->AddWidget(this->StartupScreenWidget, ViewportSlot);
            this->StartupScreenWidget->SetFocus();
        }
    }

    // Tell the startup widget that it has been displayed and can now start the login process if it wants to.
    if (IsValid(this->StartupScreenWidget))
    {
        FOnStartupScreenStartLogin OnStartLoginDelegate;
        OnStartLoginDelegate.BindDynamic(this, &URedpointGameInstance::OnStartLogin);
        IRedpointStartupScreen::Execute_OnStartupScreenDisplayed(this->StartupScreenWidget, OnStartLoginDelegate);
    }
    else
    {
        // Just start login if we don't have a widget.
        this->OnStartLogin();
    }
}

void URedpointGameInstance::OnStartLogin()
{
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Core;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Identity;
    using namespace ::Redpoint::EOS::Identity::Requests;

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        UE_LOG(LogRedpointEOSFramework, Warning, TEXT("Unable to find platform handle on startup!"));
        Super::StartGameInstance();
        return;
    }

    // Start the login process for the primary user.
    // @todo: Are there scenarios where we need to sign in other local users? Probably not on startup since adding new
    // controllers is likely always a game decision.
    auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
    IdentitySystem->Login(
        FLoginRequest(0),
        IIdentitySystem::FOnLoginComplete::CreateUObject(this, &URedpointGameInstance::OnFinishLogin));
}

void URedpointGameInstance::OnFinishLogin(
    ::Redpoint::EOS::API::FError ErrorCode,
    ::Redpoint::EOS::Identity::FIdentityUserPtr NewUser)
{
    if (ErrorCode.WasSuccessful())
    {
        // Register the event that we use to hide the startup screen widget once the startup map is ready to begin play.
        this->OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
            this,
            &URedpointGameInstance::OnStartupMapWorldInitialized);

        // Replace the progress message with "Loading..." since we're about to load the startup map.
        if (IsValid(this->StartupScreenWidget))
        {
            IRedpointStartupScreen::Execute_UpdateProgressMessage(
                this->StartupScreenWidget,
                NSLOCTEXT("RedpointEOSFramework", "Loading", "Loading..."));
        }

        // Schedule the game instance to start on the beginning of the frame in a few ticks. This gives the UI an
        // opportunity to update in response to the "Loading..." message.
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateWeakLambda(
                this,
                [this](float) -> bool {
                    this->OnBeginFrameDelegateHandle = FCoreDelegates::OnBeginFrame.AddUObject(
                        this,
                        &URedpointGameInstance::StartGameInstanceOnNextFrame);
                    return false;
                }),
            0.1f);
    }
    else if (!IsValid(this->StartupScreenWidget))
    {
        // Can't proceed or show error message.
        UE_LOG(
            LogRedpointEOSFramework,
            Error,
            TEXT("Login failed '%s', and the startup screen is not valid. Forcing exit."),
            *ErrorCode.ToLogString());
        FPlatformMisc::RequestExit(true);
    }
    else
    {
        FOnStartupScreenStartLogin OnStartLoginDelegate;
        OnStartLoginDelegate.BindDynamic(this, &URedpointGameInstance::OnStartLogin);
        IRedpointStartupScreen::Execute_OnLoginFailed(
            this->StartupScreenWidget,
            ErrorCode.ErrorMessage(),
            OnStartLoginDelegate);
    }
}

void URedpointGameInstance::StartGameInstanceOnNextFrame()
{
    FCoreDelegates::OnBeginFrame.Remove(this->OnBeginFrameDelegateHandle);
    this->OnBeginFrameDelegateHandle.Reset();

    Super::StartGameInstance();
}

void URedpointGameInstance::OnStartupMapWorldInitialized(UWorld *World, const UWorld::InitializationValues)
{
    // The world has been initialized.
    FWorldDelegates::OnPostWorldInitialization.Remove(this->OnPostWorldInitializationHandle);
    this->OnPostWorldInitializationHandle.Reset();

    // Wait for BeginPlay to happen.
    this->OnWorldBeginPlayHandle =
        World->OnWorldBeginPlay.AddUObject(this, &URedpointGameInstance::OnStartupMapWorldBeginPlay, World);
}

void URedpointGameInstance::OnStartupMapWorldBeginPlay(UWorld *World)
{
    // The world is beginning play.
    World->OnWorldBeginPlay.Remove(this->OnWorldBeginPlayHandle);
    this->OnWorldBeginPlayHandle.Reset();

    // Hide the startup screen widget.
    if (IsValid(this->StartupScreenWidget))
    {
        this->StartupScreenWidget->RemoveFromParent();
    }
    this->StartupScreenWidget = nullptr;
}

TSubclassOf<UOnlineSession> URedpointGameInstance::GetOnlineSessionClass()
{
    return this->OnlineSessionClass;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()