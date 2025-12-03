// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4076165956, Redpoint::EOS::Identity::Hooks)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class FLocalPlayerIdentityHook : public IIdentityHook
{
public:
    FLocalPlayerIdentityHook() = default;
    UE_NONCOPYABLE(FLocalPlayerIdentityHook);
    virtual ~FLocalPlayerIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("LocalPlayerIdentityHook");
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

namespace Redpoint::EOS::Identity::Hooks
{
REDPOINT_EOS_FILE_NS_EXPORT(4076165956, Redpoint::EOS::Identity::Hooks, FLocalPlayerIdentityHook)
}

REDPOINT_EOS_CODE_GUARD_END()