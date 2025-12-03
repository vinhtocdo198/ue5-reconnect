// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1415206665, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

class FVoiceManagerIdentityHook : public IIdentityHook
{
public:
    FVoiceManagerIdentityHook() = default;
    UE_NONCOPYABLE(FVoiceManagerIdentityHook);
    ~FVoiceManagerIdentityHook() = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("VoiceManagerIdentityHook");
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

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1415206665, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity, FVoiceManagerIdentityHook)
}

#endif