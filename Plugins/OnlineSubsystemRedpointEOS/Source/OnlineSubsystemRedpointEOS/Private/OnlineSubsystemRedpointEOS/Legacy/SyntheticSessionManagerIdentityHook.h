// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSIdentity/IdentityHook.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3131657263, Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy)
{
using namespace ::Redpoint::EOS::Identity;

class FSyntheticSessionManagerIdentityHook : public IIdentityHook
{
public:
    FSyntheticSessionManagerIdentityHook() = default;
    UE_NONCOPYABLE(FSyntheticSessionManagerIdentityHook);
    virtual ~FSyntheticSessionManagerIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("SyntheticSessionManagerIdentityHook");
    }

    virtual void OnStartSystem(
        const FIdentityHookContext &Context,
        const FIdentityUserRef &User,
        const FOnStartSystemComplete &OnComplete) override;
    virtual void OnStopSystem(
        const FIdentityHookContext &Context,
        const FIdentityUserRef &User,
        const FOnStopSystemComplete &OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3131657263,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy,
    FSyntheticSessionManagerIdentityHook)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()