// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"

#include "RedpointEOSAPI/Auth/GetLoggedInAccounts.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Utils/SetArray.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3204027507, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

void FAuthenticationHelpersEpicGames::Login(
    Redpoint::EOS::API::FPlatformHandle PlatformHandle,
    const FString &Id,
    const FString &Token,
    EOS_ELoginCredentialType Type,
    EOS_EExternalCredentialType ExternalType,
    bool bPermitInteractive,
    const Redpoint::EOS::Config::IConfig &InConfig,
    const FLoginCompletionDelegate &OnComplete)
{
    using namespace ::Redpoint::EOS::API::Auth;
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<EOS_EpicAccountId> ExistingUsersResult;
    EOS_EResult ExistingUsersResultCode;
    FGetLoggedInAccounts::Execute(PlatformHandle, ExistingUsersResultCode, ExistingUsersResult);
    auto ExistingUsers = ArrayToSet(ExistingUsersResult);

    EOS_EAuthScopeFlags ScopeFlags = EOS_EAuthScopeFlags::EOS_AS_NoFlags;
    for (const auto &Flag : InConfig.GetEpicGamesScopes())
    {
        ScopeFlags |= Flag;
    }
    if (ScopeFlags != InConfig.GetSupportedEpicGamesScopes())
    {
        UE_LOG(
            LogRedpointEOSAuth,
            Warning,
            TEXT("Non-standard Epic Games scopes set in configuration; there is NO SUPPORT for using a non-standard "
                 "set of scopes!"));
    }

    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("%s: Request started"), ANSI_TO_TCHAR(__FUNCTION__));

    FLogin::Execute(
        PlatformHandle,
        FLogin::Options{Id, Token, Type, ExternalType, ScopeFlags, !bPermitInteractive},
        FLogin::CompletionDelegate::CreateLambda([OnComplete, ExistingUsers](const FLogin::Result &Result) {
            UE_LOG(
                LogRedpointEOSAuth,
                Verbose,
                TEXT("%s: Request finished (result code %s)"),
                ANSI_TO_TCHAR(__FUNCTION__),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));

            OnComplete.ExecuteIfBound(
                Result,
                Result.ResultCode == EOS_EResult::EOS_Success && !ExistingUsers.Contains(Result.LocalUserId));
        }));
}

void FAuthenticationHelpersEpicGames::LinkAccount(
    Redpoint::EOS::API::FPlatformHandle PlatformHandle,
    EOS_ContinuanceToken ContinuanceToken,
    EOS_ELinkAccountFlags LinkAccountFlags,
    EOS_EpicAccountId LocalUserId,
    const FLinkAccountCompletionDelegate &OnComplete)
{
    using namespace ::Redpoint::EOS::API::Auth;
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<EOS_EpicAccountId> ExistingUsersResult;
    EOS_EResult ExistingUsersResultCode;
    FGetLoggedInAccounts::Execute(PlatformHandle, ExistingUsersResultCode, ExistingUsersResult);
    auto ExistingUsers = ArrayToSet(ExistingUsersResult);

    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("%s: Request started"), ANSI_TO_TCHAR(__FUNCTION__));

    FLinkAccount::Execute(
        PlatformHandle,
        FLinkAccount::Options{ContinuanceToken, LinkAccountFlags, LocalUserId},
        FLinkAccount::CompletionDelegate::CreateLambda([OnComplete, ExistingUsers](const FLinkAccount::Result &Result) {
            UE_LOG(
                LogRedpointEOSAuth,
                Verbose,
                TEXT("%s: Request finished (result code %s)"),
                ANSI_TO_TCHAR(__FUNCTION__),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));

            OnComplete.ExecuteIfBound(
                Result,
                Result.ResultCode == EOS_EResult::EOS_Success && !ExistingUsers.Contains(Result.LocalUserId));
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()