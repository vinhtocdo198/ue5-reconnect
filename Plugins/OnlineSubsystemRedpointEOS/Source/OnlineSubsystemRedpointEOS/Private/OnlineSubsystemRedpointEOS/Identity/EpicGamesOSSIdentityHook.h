// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4052375527, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

class FEpicGamesOSSIdentityHook : public IIdentityHook
{
public:
    FEpicGamesOSSIdentityHook() = default;
    UE_NONCOPYABLE(FEpicGamesOSSIdentityHook);
    ~FEpicGamesOSSIdentityHook() = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("EpicGamesOSSIdentityHook");
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
REDPOINT_EOS_FILE_NS_EXPORT(4052375527, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity, FEpicGamesOSSIdentityHook)
}

#endif