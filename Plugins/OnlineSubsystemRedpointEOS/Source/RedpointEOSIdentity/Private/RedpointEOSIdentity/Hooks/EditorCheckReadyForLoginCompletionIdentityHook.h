// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(89100057, Redpoint::EOS::Identity::Hooks)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class FEditorCheckReadyForLoginCompletionIdentityHook
    : public IIdentityHook,
      public TSharedFromThis<FEditorCheckReadyForLoginCompletionIdentityHook>
{
public:
    FEditorCheckReadyForLoginCompletionIdentityHook() = default;
    UE_NONCOPYABLE(FEditorCheckReadyForLoginCompletionIdentityHook);
    virtual ~FEditorCheckReadyForLoginCompletionIdentityHook() override = default;

    virtual FString GetHookDebugName() const override
    {
        return TEXT("EditorCheckReadyForLoginCompletionIdentityHook");
    }

    virtual void OnPostLoginBeforeEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserPtr &InUserIfLoginSuccessful,
        const FOnPostLoginBeforeEventsComplete &InOnComplete) override;

#if WITH_EDITOR
private:
    bool CheckIfReadyToFinishEditorLogin(float DeltaSeconds, FOnPostLoginBeforeEventsComplete InOnComplete);
#endif
};

}

namespace Redpoint::EOS::Identity::Hooks
{
REDPOINT_EOS_FILE_NS_EXPORT(89100057, Redpoint::EOS::Identity::Hooks, FEditorCheckReadyForLoginCompletionIdentityHook)
}

REDPOINT_EOS_CODE_GUARD_END()