// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Services/NullRuntimePlatformAuthService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1384177521, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::API;

FName FNullRuntimePlatformAuthService::GetPlatformName() const
{
    // No platform name.
    return NAME_None;
}

int32 FNullRuntimePlatformAuthService::GetSelectionScore(const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    // Never select.
    return -1000;
}

void FNullRuntimePlatformAuthService::EnsureSignedInOnLocalPlatform(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnEnsureSignedInOnLocalPlatform &OnComplete)
{
    checkf(
        false,
        TEXT("%s: This function should not be called, as GetPlatformName returns NAME_None and GetSelectionScore "
             "returns -1000."),
        ANSI_TO_TCHAR(__FUNCTION__));
    OnComplete.ExecuteIfBound(
        Errors::NotImplemented(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Invalid platform configuration.")));
}

void FNullRuntimePlatformAuthService::IsOnlinePlayPermitted(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FOnIsOnlinePlayPermitted &OnComplete)
{
    checkf(
        false,
        TEXT("%s: This function should not be called, as GetPlatformName returns NAME_None and GetSelectionScore "
             "returns -1000."),
        ANSI_TO_TCHAR(__FUNCTION__));
    OnComplete.ExecuteIfBound(
        Errors::NotImplemented(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Invalid platform configuration.")));
}

void FNullRuntimePlatformAuthService::GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph &OnComplete)
{
    checkf(
        false,
        TEXT("%s: This function should not be called, as GetPlatformName returns NAME_None and GetSelectionScore "
             "returns -1000."),
        ANSI_TO_TCHAR(__FUNCTION__));
    OnComplete.ExecuteIfBound(
        Errors::NotImplemented(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Invalid platform configuration.")));
}

TSharedRef<IRuntimePlatformAuthService> NullAuthService = MakeShared<FNullRuntimePlatformAuthService>();

}

#endif

REDPOINT_EOS_CODE_GUARD_END()