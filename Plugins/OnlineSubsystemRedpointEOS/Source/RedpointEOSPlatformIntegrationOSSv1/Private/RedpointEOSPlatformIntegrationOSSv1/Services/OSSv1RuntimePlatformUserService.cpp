// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformUserService.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"
#include "RedpointEOSPlatformIntegrationOSSv1/WrappedExternalUserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(924768428, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;

FOSSv1RuntimePlatformUserService::FOSSv1RuntimePlatformUserService(
    const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
    const FName &InSubsystemName,
    const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver)
    : ExternalAccountTypeToHandle(InExternalAccountTypeToHandle)
    , SubsystemName(InSubsystemName)
    , ExternalInfoResolver(InExternalInfoResolver)
{
}

void FOSSv1RuntimePlatformUserService::GetExternalUserByUniqueNetId(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FUniqueNetId &TargetUserId,
    const FOnGetExternalUserByUniqueNetIdComplete &OnComplete)
{
    auto LegacyExternalAccountInfo = this->ExternalInfoResolver(TargetUserId);
    if (LegacyExternalAccountInfo.IsSet())
    {
        OnComplete.ExecuteIfBound(
            Errors::Success(),
            MakeShared<FWrappedExternalUserInfo>(
                LegacyExternalAccountInfo->AccountIdType,
                LegacyExternalAccountInfo->AccountId,
                TEXT("")));
    }
    else
    {
        OnComplete.ExecuteIfBound(
            Errors::NotImplemented(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("This runtime platform integration does not handle user IDs of type '%s'."),
                    *TargetUserId.GetType().ToString())),
            nullptr);
    }
}

void FOSSv1RuntimePlatformUserService::GetExternalUserByAccountId(
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

}; // namespace REDPOINT_EOS_FILE_NS_ID(924768428,Redpoint::EOS::Platform::Integration::OSSv1::Services)

REDPOINT_EOS_CODE_GUARD_END()