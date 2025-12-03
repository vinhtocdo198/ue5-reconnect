// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3723025591, Redpoint::EOS::Identity::Hooks)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class FEditorDefaultInstanceCheckIdentityHook : public IIdentityHook
{
public:
    FEditorDefaultInstanceCheckIdentityHook() = default;
    UE_NONCOPYABLE(FEditorDefaultInstanceCheckIdentityHook);
    virtual ~FEditorDefaultInstanceCheckIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("EditorDefaultInstanceCheckIdentityHook");
    }

    virtual void OnPreLogin(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FOnPreLoginComplete &InOnComplete) override;
};

}

namespace Redpoint::EOS::Identity::Hooks
{
REDPOINT_EOS_FILE_NS_EXPORT(3723025591, Redpoint::EOS::Identity::Hooks, FEditorDefaultInstanceCheckIdentityHook)
}

REDPOINT_EOS_CODE_GUARD_END()