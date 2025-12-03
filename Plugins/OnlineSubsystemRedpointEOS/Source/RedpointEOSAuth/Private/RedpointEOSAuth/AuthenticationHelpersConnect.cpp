// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"

#include "RedpointEOSAPI/Connect/GetLoggedInUsers.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Utils/SetArray.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2599832381, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Auth;

void FAuthenticationHelpersConnect::Login(
    Redpoint::EOS::API::FPlatformHandle PlatformHandle,
    const FString &Id,
    const FString &Token,
    const FString &LocalDisplayName,
    EOS_EExternalCredentialType Type,
    const FLoginCompletionDelegate &OnComplete)
{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Core::Utils;

    TArray<EOS_ProductUserId> ExistingUsersResult;
    EOS_EResult ExistingUsersResultCode;
    FGetLoggedInUsers::Execute(PlatformHandle, ExistingUsersResultCode, ExistingUsersResult);
    auto ExistingUsers = ArrayToSet(ExistingUsersResult);

    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("%s: Request started"), ANSI_TO_TCHAR(__FUNCTION__));

    FLogin::Execute(
        PlatformHandle,
        FLogin::Options{
            Id,
            Token,
            LocalDisplayName,
            Type,
        },
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

}

#endif

REDPOINT_EOS_CODE_GUARD_END()