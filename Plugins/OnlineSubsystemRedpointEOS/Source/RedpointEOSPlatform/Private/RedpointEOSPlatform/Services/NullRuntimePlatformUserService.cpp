// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Services/NullRuntimePlatformUserService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(93821324, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::API;

void FNullRuntimePlatformUserService::GetExternalUserByUniqueNetId(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FUniqueNetId &TargetUserId,
    const FOnGetExternalUserByUniqueNetIdComplete &OnComplete)
{
    OnComplete.ExecuteIfBound(
        Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("This platform does not support resolving FUniqueNetId values into FExternalUserInfo values.")),
        nullptr);
}

void FNullRuntimePlatformUserService::GetExternalUserByAccountId(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    UE::Online::FAccountId TargetUserId,
    const FOnGetExternalUserByAccountIdComplete &OnComplete)
{
    OnComplete.ExecuteIfBound(
        TargetUserId,
        Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("This platform does not support resolving UE::Online::FAccountId values into FExternalUserInfo "
                 "values.")),
        nullptr);
}

TSharedRef<IRuntimePlatformUserService> NullUserService = MakeShared<FNullRuntimePlatformUserService>();

}

REDPOINT_EOS_CODE_GUARD_END()