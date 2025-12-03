// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/FriendIdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Misc/AutomationTest.h"
#include "RedpointEOSFriends/FriendSystem.h"
#include "RedpointEOSFriends/FriendSystemImpl.h"
#include "RedpointEOSFriends/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3376039905, Redpoint::EOS::Friends)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Friends;

void FFriendIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
#if REDPOINT_EOS_UE_5_4_OR_LATER
    if (GIsAutomationTesting)
    {
        auto TestPath = FAutomationTestFramework::Get().GetCurrentTestFullPath();
        if (!TestPath.Contains(TEXT("Presence")) && !TestPath.Contains(TEXT("Identity.LoginAsUser")) &&
            !TestPath.Contains(TEXT("Party.Advertisement")) && !TestPath.Contains(TEXT("Friends")))
        {
            // Skip friends for unrelated automation tests.
            UE_LOG(
                LogRedpointEOSFriends,
                Verbose,
                TEXT("Skipping friends set up as this is an non-presence automation test."));
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }
    }
#endif

    auto FriendSystem = StaticCastSharedRef<FFriendSystemImpl>(InContext.PlatformHandle->GetSystem<IFriendSystem>());

    FriendSystem->AddLocalUser(InUser).Next([InOnComplete](FError ErrorCode) {
        InOnComplete.ExecuteIfBound(ErrorCode);
    });
}

void FFriendIdentityHook::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
#if REDPOINT_EOS_UE_5_4_OR_LATER
    if (GIsAutomationTesting)
    {
        auto TestPath = FAutomationTestFramework::Get().GetCurrentTestFullPath();
        if (!TestPath.Contains(TEXT("Presence")) && !TestPath.Contains(TEXT("Identity.LoginAsUser")) &&
            !TestPath.Contains(TEXT("Party.Advertisement")) && !TestPath.Contains(TEXT("Friends")))
        {
            // Skip friends for unrelated automation tests.
            UE_LOG(
                LogRedpointEOSFriends,
                Verbose,
                TEXT("Skipping friends set up as this is an non-presence automation test."));
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }
    }
#endif

    auto FriendSystem = StaticCastSharedRef<FFriendSystemImpl>(InContext.PlatformHandle->GetSystem<IFriendSystem>());

#if REDPOINT_EOS_UE_5_6_OR_LATER
    FriendSystem->RemoveLocalUser(InUser).Next([InOnComplete]() {
        InOnComplete.ExecuteIfBound(Errors::Success());
    });
#else
    FriendSystem->RemoveLocalUser(InUser).Next([InOnComplete](auto) {
        InOnComplete.ExecuteIfBound(Errors::Success());
    });
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()