// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Services/NullRuntimePlatformAvatarService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(10947481, Redpoint::EOS::Platform::Services)
{

void FNullRuntimePlatformAvatarService::GetAvatar(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const IRuntimePlatformAvatarService::FOnGetAvatarComplete &OnComplete)
{
    OnComplete.ExecuteIfBound(
        UserInfo,
        Redpoint::EOS::API::Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("This platform does not support this operation.")),
        nullptr);
}

void FNullRuntimePlatformAvatarService::GetAvatarUrl(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FExternalUserInfoRef &UserInfo,
    const IRuntimePlatformAvatarService::FOnGetAvatarUrlComplete &OnComplete)
{
    OnComplete.ExecuteIfBound(
        UserInfo,
        Redpoint::EOS::API::Errors::NotImplemented(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("This platform does not support this operation.")),
        TEXT(""));
}

TSharedRef<IRuntimePlatformAvatarService> NullAvatarService = MakeShared<FNullRuntimePlatformAvatarService>();

}

REDPOINT_EOS_CODE_GUARD_END()