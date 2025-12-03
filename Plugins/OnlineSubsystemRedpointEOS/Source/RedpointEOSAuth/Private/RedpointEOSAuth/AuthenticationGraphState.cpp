// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/AuthenticationGraphState.h"

#include "Blueprint/GameViewportSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(191830136, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Utils;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

FAuthenticationGraphState::FAuthenticationGraphState(
    const Redpoint::EOS::API::FPlatformHandle &InPlatformHandle,
    const TSharedRef<IAuthenticationGraphUserInterface> &InUserInterface,
    int32 InLocalUserNum,
    FName InWorldContextHandle,
    const TSharedRef<Redpoint::EOS::Config::IConfig> &InConfig)
    : SelectedEOSCandidate()
    , HasSelectedEOSCandidateFlag(false)
    , CurrentWidget(nullptr)
    , bWasMouseCursorShown(false)
    , LastCachedWorld(nullptr)
    , LogoutNodes()
    , bHasObtainedAuthService(false)
    , AuthService()
    , UserInterface(InUserInterface)
    , Config(InConfig)
    , PlatformHandle(InPlatformHandle)
    , LocalUserNum(InLocalUserNum)
    , ExistingUserId()
    , ExistingExternalCredentials()
    , ExistingCrossPlatformAccountId(nullptr)
    , ProvidedCredentials()
    , ErrorMessages()
    , CrossPlatformAccountProvider()
    , AuthenticatedCrossPlatformAccountId(nullptr)
    , AvailableExternalCredentials()
    , AttemptedDeveloperAuthenticationCredentialNames()
    , WorldContextHandle(InWorldContextHandle)
    , EOSCandidates()
    , ResultUserId()
    , ResultExternalCredentials()
    , ResultCrossPlatformAccountId()
    , ResultRefreshCallback()
    , LastSignInChoice()
#if !UE_BUILD_SHIPPING
    , AutomatedTestingEmailAddress()
    , AutomatedTestingPassword()
#endif // #if !UE_BUILD_SHIPPING
    , Metadata()
    , EASExternalContinuanceToken(nullptr)
    , ResultUserAuthAttributes()
    , RegisteredNodes()
{
}

FAuthenticationGraphState::~FAuthenticationGraphState()
{
    // We can only clean up if we're not running in garbage collection, since calling
    // IsValid will fail in this scenario. I've only seen this failure when the game
    // is shutting down and the widget was still present on screen when the user
    // clicks the [X] close button on the window (not during a normal shutdown
    // from Exit Game).
    if (!(IsInGameThread() && IsGarbageCollecting()))
    {
        if (this->CurrentWidget.IsValid())
        {
            this->ClearCurrentUserInterfaceWidget();
        }
    }
}

UWorld *FAuthenticationGraphState::GetWorld()
{
    if (this->LastCachedWorld.IsValid())
    {
        return this->LastCachedWorld.Get();
    }

    this->LastCachedWorld = Redpoint::EOS::Core::Utils::FWorldResolution::GetWorld(this->WorldContextHandle);
    if (!this->LastCachedWorld.IsValid())
    {
        return nullptr;
    }

    return this->LastCachedWorld.Get();
}

/**
 * @note: I don't super like having this inside AuthenticationGraphState, but it is the automatic implementation of the
 * refresh callback for external credentials.
 */
static void RefreshFromExternalCredentials(
    const TSharedRef<FAuthenticationGraphRefreshEOSCredentialsInfo> &Info,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<IOnlineExternalCredentials> ExternalCredentials)
{
    using namespace ::Redpoint::EOS::API::Connect;

    ExternalCredentials->Refresh(
        Info->World,
        Info->LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete::CreateLambda([Info, ExternalCredentials](bool bWasSuccessful) {
            if (!bWasSuccessful)
            {
                UE_LOG(LogRedpointEOSAuth, Error, TEXT("Unable to refresh external credentials!"));
                Info->OnComplete.ExecuteIfBound(false);
                return;
            }

            FAuthenticationHelpersConnect::Login(
                Info->PlatformHandle,
                ExternalCredentials->GetId(),
                ExternalCredentials->GetToken(),
                ExternalCredentials->GetUserDisplayName(),
                FExternalCredentialType::FromString(ExternalCredentials->GetType()),
                FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateLambda(
                    [Info, ExternalCredentials](const FLogin::Result &Data, bool) {
                        // FEOSAuthentication does EOS_Connect_Login, which is all we need to do.
                        if (Data.ResultCode == EOS_EResult::EOS_Success)
                        {
                            // Compute the difference between the existing auth attributes and what is currently in
                            // ExternalCredentials->GetAuthAttributes().
                            TMap<FString, FString> NewAuthAttributes = ExternalCredentials->GetAuthAttributes();
                            Info->SetUserAuthAttributes = NewAuthAttributes;
                            for (const auto &KV : Info->ExistingUserAuthAttributes)
                            {
                                if (!NewAuthAttributes.Contains(KV.Key))
                                {
                                    Info->DeleteUserAuthAttributes.Add(KV.Key);
                                }
                            }
                            Info->OnComplete.ExecuteIfBound(true);
                            UE_LOG(
                                LogRedpointEOSAuth,
                                Verbose,
                                TEXT("Successfully refreshed EOS external credential login for user"))
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSAuth,
                                Error,
                                TEXT("Failed to refresh EOS external credential login for user, got result code: %s"),
                                ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode)));
                            Info->OnComplete.ExecuteIfBound(false);
                        }
                    }));
        }));
}

FAuthenticationGraphEOSCandidate FAuthenticationGraphState::AddEOSConnectCandidateFromExternalCredentials(
    const Redpoint::EOS::API::Connect::FLogin::Result &Data,
    const TSharedRef<IOnlineExternalCredentials> &ExternalCredentials,
    EAuthenticationGraphEOSCandidateType InType,
    TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> InCrossPlatformAccountId)
{
    FAuthenticationGraphEOSCandidate Candidate;
    Candidate.DisplayName = ExternalCredentials->GetProviderDisplayName();
    Candidate.UserAuthAttributes = ExternalCredentials->GetAuthAttributes();
    Candidate.ProductUserId = Data.LocalUserId;
    Candidate.ContinuanceToken = Data.ContinuanceToken;
    Candidate.Type = InType;
    Candidate.AssociatedCrossPlatformAccountId = MoveTemp(InCrossPlatformAccountId);
    Candidate.RefreshCallback =
        FAuthenticationGraphRefreshEOSCredentials::CreateStatic(&RefreshFromExternalCredentials, ExternalCredentials);
    Candidate.ExternalCredentials = ExternalCredentials;
    Candidate.NativeSubsystemName = ExternalCredentials->GetNativeSubsystemName();
    this->EOSCandidates.Add(Candidate);
    return Candidate;
}

FAuthenticationGraphEOSCandidate FAuthenticationGraphState::AddEOSConnectCandidate(
    FText DisplayName,
    TMap<FString, FString> UserAuthAttributes,
    const Redpoint::EOS::API::Connect::FLogin::Result &Data,
    FAuthenticationGraphRefreshEOSCredentials RefreshCallback,
    const FName &InNativeSubsystemName,
    EAuthenticationGraphEOSCandidateType InType,
    TSharedPtr<const CrossPlatform::FCrossPlatformAccountId> InCrossPlatformAccountId)
{
    FAuthenticationGraphEOSCandidate Candidate;
    Candidate.DisplayName = MoveTemp(DisplayName);
    Candidate.UserAuthAttributes = MoveTemp(UserAuthAttributes);
    Candidate.ProductUserId = Data.LocalUserId;
    Candidate.ContinuanceToken = Data.ContinuanceToken;
    Candidate.Type = InType;
    Candidate.AssociatedCrossPlatformAccountId = MoveTemp(InCrossPlatformAccountId);
    Candidate.RefreshCallback = MoveTemp(RefreshCallback);
    Candidate.ExternalCredentials = nullptr;
    Candidate.NativeSubsystemName = InNativeSubsystemName;
    this->EOSCandidates.Add(Candidate);
    return Candidate;
}

void FAuthenticationGraphState::SelectEOSCandidate(const FAuthenticationGraphEOSCandidate &Candidate)
{
    this->SelectedEOSCandidate = Candidate;
    this->HasSelectedEOSCandidateFlag = true;
}

bool FAuthenticationGraphState::HasSelectedEOSCandidate()
{
    return this->HasSelectedEOSCandidateFlag;
}

bool FAuthenticationGraphState::HasCurrentUserInterfaceWidget()
{
    return this->CurrentWidget != nullptr;
}

void FAuthenticationGraphState::SetCurrentUserInterfaceWidget(UUserWidget *InWidget)
{
    check(this->CurrentWidget == nullptr);

    this->CurrentWidget = InWidget;

    if (this->UserInterface->AddWidgetToViewport(this->GetWorld(), InWidget) ==
        IAuthenticationGraphUserInterface::EEventResult::Handled)
    {
        // Defer to the event instead.
        return;
    }

    if (UGameViewportSubsystem *GameViewportSubsystem = UGameViewportSubsystem::Get(GetWorld()))
    {
        FGameViewportWidgetSlot ViewportSlot;
        ViewportSlot.ZOrder = 100000;
        GameViewportSubsystem->AddWidget(InWidget, ViewportSlot);
    }

    // Enable cursor.
    if (GEngine && GEngine->GameViewport)
    {
        auto WorldRef = GEngine->GameViewport->GetWorld();
        if (WorldRef != nullptr)
        {
            auto PC = WorldRef->GetFirstPlayerController();
            if (PC != nullptr)
            {
                this->bWasMouseCursorShown = PC->bShowMouseCursor;

                PC->SetInputMode(FInputModeUIOnly());
                PC->bShowMouseCursor = true;
            }
        }
    }
}

void FAuthenticationGraphState::ClearCurrentUserInterfaceWidget()
{
    if (this->CurrentWidget.IsValid())
    {
        if (this->UserInterface->RemoveWidgetFromViewport(this->GetWorld(), this->CurrentWidget.Get()) ==
            IAuthenticationGraphUserInterface::EEventResult::Handled)
        {
            this->CurrentWidget = nullptr;
            return;
        }

        this->CurrentWidget->RemoveFromParent();
    }

    this->CurrentWidget = nullptr;

    // Restore cursor.
    if (GEngine && GEngine->GameViewport)
    {
        auto WorldRef = GEngine->GameViewport->GetWorld();
        if (WorldRef != nullptr)
        {
            auto PC = WorldRef->GetFirstPlayerController();
            if (PC != nullptr)
            {
                // @todo: We can't restore the InputMode, since there's no GetInputMode function.
                PC->bShowMouseCursor = this->bWasMouseCursorShown;
            }
        }
    }
}

EOS_EpicAccountId FAuthenticationGraphState::GetAuthenticatedEpicAccountId() const
{
    TSharedPtr<const CrossPlatform::FEpicGamesCrossPlatformAccountId> EpicId =
        this->GetAuthenticatedCrossPlatformAccountId<CrossPlatform::FEpicGamesCrossPlatformAccountId>(
            EPIC_GAMES_ACCOUNT_ID);
    if (EpicId.IsValid() && EpicId->HasValidEpicAccountId())
    {
        return EpicId->GetEpicAccountId();
    }
    return nullptr;
}

FAuthenticationGraphEOSCandidate FAuthenticationGraphState::GetSelectedEOSCandidate()
{
    check(this->HasSelectedEOSCandidateFlag);
    return this->SelectedEOSCandidate;
}

void FAuthenticationGraphState::AddLogoutNode(const TSharedRef<FAuthenticationGraphNode> &Node)
{
    this->LogoutNodes.Insert(Node, 0);
}

TSharedPtr<IRuntimePlatformAuthService> FAuthenticationGraphState::GetAuthService()
{
    if (this->bHasObtainedAuthService)
    {
        return this->AuthService;
    }

    this->AuthService = FAuthenticationGraphState::GetAuthServiceStatic(
        this->PlatformHandle,
        this->Config.ToSharedRef(),
        this->GetWorld(),
        this->LocalUserNum);
    this->bHasObtainedAuthService = true;
    return this->AuthService;
}

TSharedPtr<IRuntimePlatformAuthService> FAuthenticationGraphState::GetAuthServiceStatic(
    const FPlatformHandle &InPlatformHandle,
    const TSharedRef<IConfig> &InConfig,
    const TSoftObjectPtr<UWorld> &InWorld,
    int32 InLocalUserNum)
{
    auto AuthServiceOverride = InConfig->GetAuthServiceOverride();
    if (!AuthServiceOverride.IsEmpty())
    {
        for (const auto &Integration :
             FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
        {
            if (Integration->GetAuthService()->GetPlatformName() == FName(*AuthServiceOverride))
            {
                UE_LOG(
                    LogRedpointEOSAuth,
                    Verbose,
                    TEXT("Selected auth service '%s' due to configuration override."),
                    *AuthServiceOverride);
                return Integration->GetAuthService();
            }
        }

        UE_LOG(
            LogRedpointEOSAuth,
            Error,
            TEXT(
                "The configuration specified an auth service override of '%s', but no such auth service is available."),
            *AuthServiceOverride);
        return nullptr;
    }
    else
    {
        auto CallContext = ConstructRuntimePlatformServiceCallContext(InPlatformHandle, InLocalUserNum);

        int32 SelectionScore = 0;
        TSharedPtr<IRuntimePlatformAuthService> AuthService = nullptr;
        UE_LOG(
            LogRedpointEOSAuth,
            Verbose,
            TEXT("There are %d runtime platform integrations available."),
            FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations().Num());
        for (const auto &Integration :
             FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
        {
            auto IntegrationSelectionScore = Integration->GetAuthService()->GetSelectionScore(CallContext);
            UE_LOG(
                LogRedpointEOSAuth,
                Verbose,
                TEXT("Considering authentication service '%s', which has a selection score of %d."),
                *Integration->GetAuthService()->GetPlatformName().ToString(),
                IntegrationSelectionScore);
            if (IntegrationSelectionScore != 0 && IntegrationSelectionScore > SelectionScore)
            {
                UE_LOG(
                    LogRedpointEOSAuth,
                    Verbose,
                    TEXT("Authentication service '%s' is now under consideration."),
                    *Integration->GetAuthService()->GetPlatformName().ToString());
                AuthService = Integration->GetAuthService();
                SelectionScore = IntegrationSelectionScore;
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSAuth,
                    Verbose,
                    TEXT("Skipping authentication service '%s' because it doesn't beat the current selection score of "
                         "%d."),
                    *Integration->GetAuthService()->GetPlatformName().ToString(),
                    SelectionScore);
            }
        }

        if (SelectionScore <= 0)
        {
            UE_LOG(LogRedpointEOSAuth, Error, TEXT("No auth service is available on this platform."));
            return nullptr;
        }

        UE_LOG(
            LogRedpointEOSAuth,
            Verbose,
            TEXT("Detected that auth service named '%s' should be used for authentication."),
            *AuthService->GetPlatformName().ToString());
        return AuthService;
    }
}

const TMap<FString, FString> &FAuthenticationGraphState::GetResultUserAuthAttributes() const
{
    return this->ResultUserAuthAttributes;
}

void FAuthenticationGraphState::AddResultUserAuthAttribute(const FString &Key, const FString &Value)
{
    this->ResultUserAuthAttributes.Add(Key, Value);
}

void FAuthenticationGraphState::AddResultUserAuthAttributes(const TMap<FString, FString> &Attributes)
{
    for (const auto &KV : Attributes)
    {
        this->ResultUserAuthAttributes.Add(KV.Key, KV.Value);
    }
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()