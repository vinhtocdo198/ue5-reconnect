// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/ChainEASResultToEOSNode.h"

#include "RedpointEOSAPI/Auth/CopyUserAuthToken.h"
#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPersistentEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2590565940, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::API::Auth;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::Nodes;

class FEpicExternalCredentials : public IOnlineExternalCredentials
{
private:
    FPlatformHandle PlatformHandle;
    FCopyUserAuthToken::Result Token;
    FName NativeSubsystem;
    TMap<FString, FString> Attributes;
    TMap<FString, FString> ComputeAttributesFromToken(const FCopyUserAuthToken::Result &InToken);

public:
    UE_NONCOPYABLE(FEpicExternalCredentials);
    FEpicExternalCredentials(
        FPlatformHandle InPlatformHandle,
        FName InNativeSubsystem,
        const FCopyUserAuthToken::Result &InToken);
    virtual ~FEpicExternalCredentials() override = default;
    virtual FText GetProviderDisplayName() const override;
    virtual FString GetType() const override;
    virtual FString GetId() const override;
    virtual FString GetToken() const override;
    virtual TMap<FString, FString> GetAuthAttributes() const override;
    virtual FName GetNativeSubsystemName() const override;
    virtual void Refresh(
        TSoftObjectPtr<UWorld> InWorld,
        int32 LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete OnComplete) override;
    EOS_EpicAccountId GetEpicAccountId() const;
};

FEpicExternalCredentials::FEpicExternalCredentials(
    FPlatformHandle InPlatformHandle,
    FName InNativeSubsystem,
    const FCopyUserAuthToken::Result &InToken)
    : PlatformHandle(InPlatformHandle)
    , Token(InToken)
    , NativeSubsystem(InNativeSubsystem)
{
    this->Attributes = this->ComputeAttributesFromToken(InToken);
}

FText FEpicExternalCredentials::GetProviderDisplayName() const
{
    return NSLOCTEXT("OnlineRedpointRedpointEOS", "EpicGames", "Epic Games");
}

FString FEpicExternalCredentials::GetType() const
{
    return TEXT("EPIC");
}

FString FEpicExternalCredentials::GetId() const
{
    return TEXT("");
}

FString FEpicExternalCredentials::GetToken() const
{
    return this->Token.AccessToken;
}

TMap<FString, FString> FEpicExternalCredentials::ComputeAttributesFromToken(const FCopyUserAuthToken::Result &InToken)
{
    TMap<FString, FString> Attrs;
    Attrs.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("epic"));
    Attrs.Add(TEXT("epic.accessToken"), this->Token.AccessToken);
    FString AccountIdStr;
    verify(EOSString_EpicAccountId::ToString(this->Token.AccountId, AccountIdStr) == EOS_EResult::EOS_Success);
    Attrs.Add(TEXT("epic.accountId"), AccountIdStr);
    Attrs.Add(TEXT("epic.app"), this->Token.App);
    switch (this->Token.AuthType)
    {
    case EOS_EAuthTokenType::EOS_ATT_Client:
        Attrs.Add(TEXT("epic.authTokenType"), TEXT("client"));
        break;
    case EOS_EAuthTokenType::EOS_ATT_User:
        Attrs.Add(TEXT("epic.authTokenType"), TEXT("user"));
        break;
    default:
        Attrs.Add(TEXT("epic.authTokenType"), TEXT("unknown"));
        break;
    }
    Attrs.Add(TEXT("epic.clientId"), this->Token.ClientId);
    Attrs.Add(TEXT("epic.expiresAt"), this->Token.ExpiresAt);
    Attrs.Add(TEXT("epic.refreshExpiresAt"), this->Token.RefreshExpiresAt);
    Attrs.Add(TEXT("epic.refreshToken"), this->Token.RefreshToken);
    return Attrs;
}

TMap<FString, FString> FEpicExternalCredentials::GetAuthAttributes() const
{
    return this->Attributes;
}

FName FEpicExternalCredentials::GetNativeSubsystemName() const
{
    return this->NativeSubsystem;
}

EOS_EpicAccountId FEpicExternalCredentials::GetEpicAccountId() const
{
    return this->Token.AccountId;
}

void FEpicExternalCredentials::Refresh(
    TSoftObjectPtr<UWorld> InWorld,
    int32 LocalUserNum,
    FOnlineExternalCredentialsRefreshComplete OnComplete)
{
    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("Copying updated refresh token from EOS Auth..."));

    EOS_EResult CopyResultCode;
    FCopyUserAuthToken::Result CopyResult;
    FCopyUserAuthToken::Execute(
        this->PlatformHandle,
        FCopyUserAuthToken::Options{this->Token.AccountId},
        CopyResultCode,
        CopyResult);
    if (CopyResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSAuth,
            Error,
            TEXT("Failed to copy auth token during refresh, got result: %s"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(CopyResultCode)));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    this->Token = CopyResult;
    this->Attributes = this->ComputeAttributesFromToken(this->Token);

    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("Successfully obtained updated EAS token for user"));

    OnComplete.ExecuteIfBound(true);
}

void FChainEASResultToEOSNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    
    EOS_EpicAccountId AuthenticatedEpicAccountId = State->GetAuthenticatedEpicAccountId();

    check(EOSString_EpicAccountId::IsValid(AuthenticatedEpicAccountId));

    {
        FString EpicAccountIdStr;
        if (EOSString_EpicAccountId::ToString(AuthenticatedEpicAccountId, EpicAccountIdStr) == EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSAuth,
                Verbose,
                TEXT("ChainEASResultToEOSNode: Retrieving auth token for account: %s"),
                *EpicAccountIdStr);
        }
    }

    EOS_EResult CopyResultCode;
    FCopyUserAuthToken::Result CopyResult;
    FCopyUserAuthToken::Execute(
        State->PlatformHandle,
        FCopyUserAuthToken::Options{AuthenticatedEpicAccountId},
        CopyResultCode,
        CopyResult);
    if (CopyResultCode != EOS_EResult::EOS_Success)
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("ChainEASResultToEOSNode: Failed to copy user auth token from Epic account ID: %s"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(CopyResultCode))));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    // Store the refresh token in metadata so it can be stored by future authentication nodes.
    State->Metadata.Add(EOS_METADATA_EAS_REFRESH_TOKEN, CopyResult.RefreshToken);

    // Some platforms don't support linking Epic Games accounts so instead we store the refresh token with
    // the external credentials to avoid prompting the user to sign into Epic each time they open the game.
    for (const auto &ExternalCredential : State->AvailableExternalCredentials)
    {
        if (ExternalCredential->GetProviderSupportedLinkType(EPIC_GAMES_ACCOUNT_ID) ==
            ESupportedLinkType::PersistentAuth)
        {
            ExternalCredential->StoreData(TEXT("EASRefreshToken"), CopyResult.RefreshToken);
        }
    }

    // Get the native subsystem. If we authenticated into EAS with e.g. a Steam credential, then the
    // native subsystem should be Steam instead of Epic Games.
    FName NativeSubsystem = FName(TEXT("RedpointEAS"));
    if (State->Metadata.Contains(EOS_METADATA_EAS_NATIVE_SUBSYSTEM))
    {
        NativeSubsystem = State->Metadata[EOS_METADATA_EAS_NATIVE_SUBSYSTEM].GetValue<FName>();
    }

    // Create the Epic "external" credentials which we'll use to create the candidate. This means refreshing gets
    // handled for us, and our external credentials can be used for client-server authentication.
    TSharedRef<FEpicExternalCredentials> EpicCredentials =
        MakeShared<FEpicExternalCredentials>(State->PlatformHandle, NativeSubsystem, CopyResult);

    // Authenticate.
    FAuthenticationHelpersConnect::Login(
        State->PlatformHandle,
        EpicCredentials->GetId(),
        EpicCredentials->GetToken(),
        EpicCredentials->GetUserDisplayName(),
        FExternalCredentialType::FromString(EpicCredentials->GetType()),
        FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone, EpicCredentials](
                const Redpoint::EOS::API::Connect::FLogin::Result &Data,
                bool bThisCallIsResponsibleForLogout) {
                if (bThisCallIsResponsibleForLogout)
                {
                    State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                }
                if (Data.ResultCode == EOS_EResult::EOS_Success || Data.ResultCode == EOS_EResult::EOS_InvalidUser)
                {
                    State->AddEOSConnectCandidateFromExternalCredentials(
                        Data,
                        EpicCredentials,
                        EAuthenticationGraphEOSCandidateType::CrossPlatform,
                        MakeShared<FEpicGamesCrossPlatformAccountId>(EpicCredentials->GetEpicAccountId()));
                }
                else
                {
                    State->ErrorMessages.Add(FString::Printf(
                        TEXT("Epic Account Services failed to authenticate with EOS Connect: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                }

                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()