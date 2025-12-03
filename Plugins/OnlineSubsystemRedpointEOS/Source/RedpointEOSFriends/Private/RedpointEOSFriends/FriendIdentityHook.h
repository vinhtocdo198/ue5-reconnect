// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSIdentity/IdentityHook.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3376039905, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::Identity;

class FFriendIdentityHook : public IIdentityHook, public TSharedFromThis<FFriendIdentityHook>
{
public:
    FFriendIdentityHook() = default;
    UE_NONCOPYABLE(FFriendIdentityHook);
    virtual ~FFriendIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("FriendIdentityHook");
    }

    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete) override;
    virtual void OnStopSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStopSystemComplete &InOnComplete) override;
};

}

namespace Redpoint::EOS::Friends
{
REDPOINT_EOS_FILE_NS_EXPORT(3376039905, Redpoint::EOS::Friends, FFriendIdentityHook)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()