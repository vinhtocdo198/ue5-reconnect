// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4071255846, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

void IIdentityHook::OnPreLogin(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FOnPreLoginComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnPostLoginBeforeEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserPtr &InUserIfLoginSuccessful,
    const FOnPostLoginBeforeEventsComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnPostLoginAfterEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserPtr &InUserIfLoginSuccessful,
    const FOnPostLoginAfterEventsComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnPreLogout(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    const FOnPreLogoutComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnPreUnexpectedLogout(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    const FOnPreUnexpectedLogoutComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound();
}

void IIdentityHook::OnPostLogoutBeforeEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    bool bLogoutWasSuccessful,
    const FOnPostLogoutBeforeEventsComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnPostLogoutAfterEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    bool bLogoutWasSuccessful,
    const FOnPostLogoutAfterEventsComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void IIdentityHook::OnPostCredentialRefresh(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    bool bWasSuccessful)
{
}

void IIdentityHook::OnGetAdditionalAuthenticationAttributeKeys(
    const FIdentityHookContext &InContext,
    const TSharedRef<const FIdentityUser> &InUser,
    TSet<FString> &OutKeys)
{
}

bool IIdentityHook::OnGetAdditionalAuthenticationAttributeValue(
    const FIdentityHookContext &InContext,
    const TSharedRef<const FIdentityUser> &InUser,
    const FString &Key,
    FString &OutValue)
{
    return false;
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedPtr<Redpoint::EOS::Auth::IAuthenticationGraphUserInterface> IIdentityHook::OnAuthenticationUserInterfaceRequired(
    const FIdentityHookContext &InContext)
{
    return nullptr;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()