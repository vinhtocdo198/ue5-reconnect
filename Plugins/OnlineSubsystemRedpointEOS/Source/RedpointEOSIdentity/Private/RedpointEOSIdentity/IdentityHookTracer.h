// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !NO_LOGGING

namespace REDPOINT_EOS_FILE_NS_ID(1332459060, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class FIdentityHookTracer : public IIdentityHook
{
private:
    TSharedRef<IIdentityHook> Hook;

public:
    FIdentityHookTracer(const TSharedRef<IIdentityHook> &InHook);
    UE_NONCOPYABLE(FIdentityHookTracer);
    virtual ~FIdentityHookTracer() override = default;

    virtual FString GetHookDebugName() const override;

    virtual void OnPreLogin(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FOnPreLoginComplete &InOnComplete) override;
    virtual void OnPostLoginBeforeEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserPtr &InUserIfLoginSuccessful,
        const FOnPostLoginBeforeEventsComplete &InOnComplete) override;
    virtual void OnPostLoginAfterEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserPtr &InUserIfLoginSuccessful,
        const FOnPostLoginAfterEventsComplete &InOnComplete) override;

    virtual void OnPreLogout(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        const FOnPreLogoutComplete &InOnComplete) override;
    virtual void OnPreUnexpectedLogout(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        const FOnPreUnexpectedLogoutComplete &InOnComplete) override;
    virtual void OnPostLogoutBeforeEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        bool bLogoutWasSuccessful,
        const FOnPostLogoutBeforeEventsComplete &InOnComplete) override;
    virtual void OnPostLogoutAfterEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        bool bLogoutWasSuccessful,
        const FOnPostLogoutAfterEventsComplete &InOnComplete) override;

    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete) override;
    virtual void OnStopSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStopSystemComplete &InOnComplete) override;

    virtual void OnPostCredentialRefresh(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        bool bWasSuccessful) override;

    virtual void OnGetAdditionalAuthenticationAttributeKeys(
        const FIdentityHookContext &InContext,
        const TSharedRef<const FIdentityUser> &InUser,
        TSet<FString> &OutKeys) override;
    virtual bool OnGetAdditionalAuthenticationAttributeValue(
        const FIdentityHookContext &InContext,
        const TSharedRef<const FIdentityUser> &InUser,
        const FString &Key,
        FString &OutValue) override;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    virtual TSharedPtr<Redpoint::EOS::Auth::IAuthenticationGraphUserInterface> OnAuthenticationUserInterfaceRequired(
        const FIdentityHookContext &InContext) override;
#endif
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1332459060, Redpoint::EOS::Identity, FIdentityHookTracer)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()