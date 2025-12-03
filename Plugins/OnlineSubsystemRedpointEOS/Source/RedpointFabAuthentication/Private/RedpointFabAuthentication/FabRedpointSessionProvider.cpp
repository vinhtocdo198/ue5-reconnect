// Copyright June Rhodes. All Rights Reserved.

#include "FabRedpointSessionProvider.h"

#include "RedpointEOSAPI/Auth/CopyUserAuthToken.h"
#include "RedpointEOSAPI/Auth/DeletePersistentAuth.h"
#include "RedpointEOSAPI/Auth/GetLoggedInAccounts.h"
#include "RedpointEOSAPI/Auth/Login.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointFabAuthentication/FabConfig.h"
#include "RedpointFabAuthentication/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3493870958, Redpoint::Fab::Authentication)
{
using namespace ::Redpoint::Fab::Authentication;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Auth;

void FFabRedpointSessionProvider::Init(
    const FString &ProductId,
    const FString &SandboxId,
    const FString &DeploymentId,
    const FString &ClientId,
    const FString &ClientSecret,
    const FString &EncryptionKey,
    bool bIsGamedev)
{
    this->PlatformRefHandle = IInstancePool::Pool().CreateWithConfig(
        FName("Fab"),
        MakeShared<FFabConfig>(ProductId, SandboxId, DeploymentId, ClientId, ClientSecret, EncryptionKey, bIsGamedev));

    FLogin::Execute(
        this->PlatformRefHandle->Instance(),
        FLogin::Options{
            TEXT(""),
            TEXT(""),
            EOS_ELoginCredentialType::EOS_LCT_PersistentAuth,
            EOS_EExternalCredentialType::EOS_ECT_EPIC,
            EOS_EAuthScopeFlags::EOS_AS_BasicProfile,
            false},
        FLogin::CompletionDelegate::CreateSPLambda(
            this,
            [this](const FLogin::Result &PersistentResult) {
                if (PersistentResult.ResultCode == EOS_EResult::EOS_Success)
                {
                    this->OnLoggedIn().Broadcast(this->GetAuthToken());
                    return;
                }

                UE_LOG(
                    LogRedpointFabAuthentication,
                    Error,
                    TEXT("%s"),
                    *ConvertError(
                         ANSI_TO_TCHAR(__FUNCTION__),
                         TEXT("Failed to perform persistent authentication for Fab."),
                         PersistentResult.ResultCode)
                         .ToLogString());
                FLogin::Execute(
                    this->PlatformRefHandle->Instance(),
                    FLogin::Options{
                        TEXT(""),
                        TEXT(""),
                        EOS_ELoginCredentialType::EOS_LCT_AccountPortal,
                        EOS_EExternalCredentialType::EOS_ECT_EPIC,
                        EOS_EAuthScopeFlags::EOS_AS_BasicProfile,
                        false},
                    FLogin::CompletionDelegate::CreateSPLambda(
                        this,
                        [this](const FLogin::Result &PortalResult) {
                            if (PortalResult.ResultCode == EOS_EResult::EOS_Success)
                            {
                                this->OnLoggedIn().Broadcast(this->GetAuthToken());
                                return;
                            }

                            UE_LOG(
                                LogRedpointFabAuthentication,
                                Error,
                                TEXT("%s"),
                                *ConvertError(
                                     ANSI_TO_TCHAR(__FUNCTION__),
                                     TEXT("Failed to perform portal authentication for Fab."),
                                     PortalResult.ResultCode)
                                     .ToLogString());
                        }));
            }));
}

void FFabRedpointSessionProvider::Shutdown()
{
    this->PlatformRefHandle.Reset();
}

void FFabRedpointSessionProvider::Login()
{
    if (!this->PlatformRefHandle.IsValid())
    {
        return;
    }

    FLogin::Execute(
        this->PlatformRefHandle->Instance(),
        FLogin::Options{
            TEXT(""),
            TEXT(""),
            EOS_ELoginCredentialType::EOS_LCT_AccountPortal,
            EOS_EExternalCredentialType::EOS_ECT_EPIC,
            EOS_EAuthScopeFlags::EOS_AS_BasicProfile,
            false},
        FLogin::CompletionDelegate::CreateSPLambda(this, [this](const FLogin::Result &PortalResult) {
            if (PortalResult.ResultCode == EOS_EResult::EOS_Success)
            {
                this->OnLoggedIn().Broadcast(this->GetAuthToken());
                return;
            }

            UE_LOG(
                LogRedpointFabAuthentication,
                Error,
                TEXT("%s"),
                *ConvertError(
                     ANSI_TO_TCHAR(__FUNCTION__),
                     TEXT("Failed to perform portal authentication for Fab."),
                     PortalResult.ResultCode)
                     .ToLogString());
        }));
}

FOnFabLoggedIn &FFabRedpointSessionProvider::OnLoggedIn()
{
    return this->LoggedInDelegate;
}

void FFabRedpointSessionProvider::Logout()
{
    if (!this->PlatformRefHandle.IsValid())
    {
        return;
    }

    FDeletePersistentAuth::Execute(
        this->PlatformRefHandle->Instance(),
        FDeletePersistentAuth::Options{TEXT("")},
        FDeletePersistentAuth::CompletionDelegate::CreateSPLambda(
            this,
            [](const FDeletePersistentAuth::Result &Result) {
                if (Result.ResultCode == EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointFabAuthentication,
                        Verbose,
                        TEXT("Successfully deleted persistent authentication for Fab."));
                }
                else
                {
                    UE_LOG(
                        LogRedpointFabAuthentication,
                        Error,
                        TEXT("%s"),
                        *ConvertError(
                             ANSI_TO_TCHAR(__FUNCTION__),
                             TEXT("Failed to delete persistent authentication for Fab."),
                             Result.ResultCode)
                             .ToLogString());
                }
            }));
}

FString FFabRedpointSessionProvider::GetAuthToken()
{
    if (!this->PlatformRefHandle.IsValid())
    {
        return TEXT("");
    }

    TArray<EOS_EpicAccountId> Accounts;
    EOS_EResult AccountsResultCode;
    FGetLoggedInAccounts::Execute(this->PlatformRefHandle->Instance(), AccountsResultCode, Accounts);
    if (Accounts.Num() == 0)
    {
        UE_LOG(
            LogRedpointFabAuthentication,
            Error,
            TEXT("%s"),
            *Errors::InvalidUser(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Unable to provide auth token for Fab as there is no user logged in."))
                 .ToLogString());
        return TEXT("");
    }

    EOS_EResult ResultCode;
    FCopyUserAuthToken::Result Result;
    FCopyUserAuthToken::Execute(
        this->PlatformRefHandle->Instance(),
        FCopyUserAuthToken::Options{Accounts[0]},
        ResultCode,
        Result);
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointFabAuthentication,
            Error,
            TEXT("%s"),
            *ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Unable to copy auth token for Fab."), ResultCode)
                 .ToLogString());
        return TEXT("");
    }

    return Result.AccessToken;
}

FString FFabRedpointSessionProvider::GetRefreshToken()
{
    if (!this->PlatformRefHandle.IsValid())
    {
        return TEXT("");
    }

    TArray<EOS_EpicAccountId> Accounts;
    EOS_EResult AccountsResultCode;
    FGetLoggedInAccounts::Execute(this->PlatformRefHandle->Instance(), AccountsResultCode, Accounts);
    if (Accounts.Num() == 0)
    {
        UE_LOG(
            LogRedpointFabAuthentication,
            Error,
            TEXT("%s"),
            *Errors::InvalidUser(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Unable to provide auth token for Fab as there is no user logged in."))
                 .ToLogString());
        return TEXT("");
    }

    EOS_EResult ResultCode;
    FCopyUserAuthToken::Result Result;
    FCopyUserAuthToken::Execute(
        this->PlatformRefHandle->Instance(),
        FCopyUserAuthToken::Options{Accounts[0]},
        ResultCode,
        Result);
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointFabAuthentication,
            Error,
            TEXT("%s"),
            *ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Unable to copy auth token for Fab."), ResultCode)
                 .ToLogString());
        return TEXT("");
    }

    return Result.RefreshToken;
}

}

REDPOINT_EOS_CODE_GUARD_END()