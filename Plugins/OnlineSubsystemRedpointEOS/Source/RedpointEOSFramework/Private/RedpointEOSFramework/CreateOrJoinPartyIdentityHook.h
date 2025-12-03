// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSIdentity/IdentityHook.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2146425322, Redpoint::EOS::Framework)
{
using namespace ::Redpoint::EOS::Identity;

class FCreateOrJoinPartyIdentityHook : public IIdentityHook, public TSharedFromThis<FCreateOrJoinPartyIdentityHook>
{
public:
    FCreateOrJoinPartyIdentityHook() = default;
    UE_NONCOPYABLE(FCreateOrJoinPartyIdentityHook);
    virtual ~FCreateOrJoinPartyIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("CreateOrJoinPartyIdentityHook");
    }

    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete) override;
};

}

namespace Redpoint::EOS::Framework
{
REDPOINT_EOS_FILE_NS_EXPORT(2146425322, Redpoint::EOS::Framework, FCreateOrJoinPartyIdentityHook)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()