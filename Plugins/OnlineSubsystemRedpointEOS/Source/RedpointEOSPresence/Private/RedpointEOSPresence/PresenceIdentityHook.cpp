// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceIdentityHook.h"

#include "Misc/AutomationTest.h"
#include "RedpointEOSPresence/Logging.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1867622895, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Presence;

void FPresenceIdentityHook::OnStartSystem(
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
            // Skip presence lobbies for unrelated automation tests.
            UE_LOG(
                LogRedpointEOSPresence,
                Verbose,
                TEXT("Skipping presence lobby set up as this is an non-presence automation test."));
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }
    }
#endif

    auto PresenceSystem =
        StaticCastSharedRef<FPresenceSystemImpl>(InContext.PlatformHandle->GetSystem<IPresenceSystem>());

    PresenceSystem->AddLocalUser(
        InUser,
        FPresenceSystemImpl::FOnLocalUserAdded::CreateSPLambda(
            this,
            [InOnComplete](const FError &ErrorCode) {
                // @note: We probably need to update the identity hook system to actually logout the user when an
                // error is returned from OnPostLoginBeforeEvents, since we actually want to prevent sign in in this
                // case.
                InOnComplete.ExecuteIfBound(ErrorCode);
            }));
}

void FPresenceIdentityHook::OnStopSystem(
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
            // Skip presence lobbies for unrelated automation tests.
            UE_LOG(
                LogRedpointEOSPresence,
                Verbose,
                TEXT("Skipping presence lobby set up as this is an non-presence automation test."));
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }
    }
#endif

    auto PresenceSystem =
        StaticCastSharedRef<FPresenceSystemImpl>(InContext.PlatformHandle->GetSystem<IPresenceSystem>());

    PresenceSystem->RemoveLocalUser(
        InUser,
        FPresenceSystemImpl::FOnLocalUserRemoved::CreateSPLambda(this, [InOnComplete]() {
            InOnComplete.ExecuteIfBound(Errors::Success());
        }));
}

void FPresenceIdentityHook::OnGetAdditionalAuthenticationAttributeKeys(
    const FIdentityHookContext &InContext,
    const TSharedRef<const FIdentityUser> &InUser,
    TSet<FString> &OutKeys)
{
    auto PresenceSystem = InContext.PlatformHandle->GetSystem<IPresenceSystem>();
    auto Result = PresenceSystem->GetFriendCode(InUser->GetUserId());
    if (Result.IsSet())
    {
        OutKeys.Add(TEXT("friendCode"));
    }
}

bool FPresenceIdentityHook::OnGetAdditionalAuthenticationAttributeValue(
    const FIdentityHookContext &InContext,
    const TSharedRef<const FIdentityUser> &InUser,
    const FString &Key,
    FString &OutValue)
{
    if (Key == TEXT("friendCode"))
    {
        auto PresenceSystem = InContext.PlatformHandle->GetSystem<IPresenceSystem>();
        auto Result = PresenceSystem->GetFriendCode(InUser->GetUserId());
        OutValue = Result.Get(TEXT(""));
        return true;
    }
    return false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()