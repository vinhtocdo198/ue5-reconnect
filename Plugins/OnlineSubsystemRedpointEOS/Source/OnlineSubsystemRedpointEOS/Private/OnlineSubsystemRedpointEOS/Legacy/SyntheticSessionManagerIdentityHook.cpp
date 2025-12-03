// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Legacy/SyntheticSessionManagerIdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "OnlineSubsystemRedpointEOS/Legacy/SyntheticSessionManager.h"
#include "RedpointEOSAPI/Error.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3131657263, Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy;

void FSyntheticSessionManagerIdentityHook::OnStartSystem(
    const FIdentityHookContext &Context,
    const FIdentityUserRef &User,
    const FOnStartSystemComplete &OnComplete)
{
    Context.PlatformHandle->GetSystem<FSyntheticSessionManager>()->OnStartSystem(
        User,
        FSimpleDelegate::CreateLambda([OnComplete]() {
            OnComplete.ExecuteIfBound(Errors::Success());
        }));
}

void FSyntheticSessionManagerIdentityHook::OnStopSystem(
    const FIdentityHookContext &Context,
    const FIdentityUserRef &User,
    const FOnStopSystemComplete &OnComplete)
{
    Context.PlatformHandle->GetSystem<FSyntheticSessionManager>()->OnStopSystem(
        User,
        FSimpleDelegate::CreateLambda([OnComplete]() {
            OnComplete.ExecuteIfBound(Errors::Success());
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()