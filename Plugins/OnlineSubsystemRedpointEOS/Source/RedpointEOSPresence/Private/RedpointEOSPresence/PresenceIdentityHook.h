// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1867622895, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Identity;

class FPresenceIdentityHook : public IIdentityHook, public TSharedFromThis<FPresenceIdentityHook>
{
public:
    FPresenceIdentityHook() = default;
    UE_NONCOPYABLE(FPresenceIdentityHook);
    virtual ~FPresenceIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("PresenceIdentityHook");
    }

    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete) override;
    virtual void OnStopSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStopSystemComplete &InOnComplete) override;

    virtual void OnGetAdditionalAuthenticationAttributeKeys(
        const FIdentityHookContext &InContext,
        const TSharedRef<const FIdentityUser> &InUser,
        TSet<FString> &OutKeys) override;
    virtual bool OnGetAdditionalAuthenticationAttributeValue(
        const FIdentityHookContext &InContext,
        const TSharedRef<const FIdentityUser> &InUser,
        const FString &Key,
        FString &OutValue) override;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(1867622895, Redpoint::EOS::Presence, FPresenceIdentityHook)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()