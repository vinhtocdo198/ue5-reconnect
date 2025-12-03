// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/Texture.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPlatform/Services/ForwardDecls.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1957691047, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::UserCache;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Platform::Services;

class REDPOINTEOSPLATFORM_API IRuntimePlatformAvatarService
{
public:
    IRuntimePlatformAvatarService() = default;
    UE_NONCOPYABLE(IRuntimePlatformAvatarService);
    virtual ~IRuntimePlatformAvatarService() = default;

    /**
     * The delegate raised when an avatar texture fetch operation is complete.
     */
    typedef TDelegate<
        void(const FExternalUserInfoRef &UserInfo, const FError &Error, const TSoftObjectPtr<UTexture> &AvatarTexture)>
        FOnGetAvatarComplete;

    /**
     * Attempt to retrieve the avatar texture for the specified user.
     */
    virtual void GetAvatar(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const FOnGetAvatarComplete &OnComplete) = 0;

    /**
     * The delegate raised when an avatar URL fetch operation is complete.
     */
    typedef TDelegate<void(const FExternalUserInfoRef &UserInfo, const FError &Error, const FString &AvatarUrl)>
        FOnGetAvatarUrlComplete;

    /**
     * Attempt to retrieve the avatar URL for the specified user.
     */
    virtual void GetAvatarUrl(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FExternalUserInfoRef &UserInfo,
        const FOnGetAvatarUrlComplete &OnComplete) = 0;
};

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(1957691047, Redpoint::EOS::Platform::Services, IRuntimePlatformAvatarService)
}

REDPOINT_EOS_CODE_GUARD_END()