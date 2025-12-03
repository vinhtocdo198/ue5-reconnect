// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/ForwardDecls.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3892400660, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;

class REDPOINTEOSPLATFORM_API IRuntimePlatformUserService
{
public:
    IRuntimePlatformUserService() = default;
    UE_NONCOPYABLE(IRuntimePlatformUserService);
    virtual ~IRuntimePlatformUserService() = default;

    /**
     * The delegate raised when a call to GetExternalUserByUniqueNetId completes.
     *
     * @note: This delegate does not provide the original FUniqueNetId, since some online subsystems are poorly
     * implemented and do not always allocate their FUniqueNetId values on the heap.
     */
    typedef TDelegate<void(const FError &Error, const FExternalUserInfoPtr &UserInfo)>
        FOnGetExternalUserByUniqueNetIdComplete;

    /**
     * Attempt to resolve an FUniqueNetId into an FExternalUserInfo object, if possible.
     */
    virtual void GetExternalUserByUniqueNetId(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FUniqueNetId &TargetUserId,
        const FOnGetExternalUserByUniqueNetIdComplete &OnComplete) = 0;

    /**
     * The delegate raised when a call to GetExternalUserByAccountId completes.
     */
    typedef TDelegate<
        void(UE::Online::FAccountId TargetUserId, const FError &Error, const FExternalUserInfoPtr &UserInfo)>
        FOnGetExternalUserByAccountIdComplete;

    /**
     * Attempt to resolve an UE::Online::FAccountId into an FExternalUserInfo object, if possible.
     */
    virtual void GetExternalUserByAccountId(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        UE::Online::FAccountId TargetUserId,
        const FOnGetExternalUserByAccountIdComplete &OnComplete) = 0;
};

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(3892400660, Redpoint::EOS::Platform::Services, IRuntimePlatformUserService)
}

REDPOINT_EOS_CODE_GUARD_END()