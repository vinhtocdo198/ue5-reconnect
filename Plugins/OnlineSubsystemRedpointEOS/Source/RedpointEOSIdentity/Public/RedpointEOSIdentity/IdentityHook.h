// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/AuthenticationGraphUserInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityHookContext.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSIdentity/IdentityUserSlot.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4071255846, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSIDENTITY_API IIdentityHook
{
public:
    IIdentityHook() = default;
    UE_NONCOPYABLE(IIdentityHook);
    virtual ~IIdentityHook() = default;

    virtual FString GetHookDebugName() const = 0;

    typedef TDelegate<void(FError ErrorCode)> FOnPreLoginComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnPostLoginBeforeEventsComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnPostLoginAfterEventsComplete;

    typedef TDelegate<void(FError ErrorCode)> FOnPreLogoutComplete;
    typedef TDelegate<void()> FOnPreUnexpectedLogoutComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnPostLogoutBeforeEventsComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnPostLogoutAfterEventsComplete;

    typedef TDelegate<void(FError ErrorCode)> FOnStartSystemComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnStopSystemComplete;

    /**
     * After IIdentitySystem::Login is called, but before authentication starts. If you return an error in the
     * OnComplete callback, the IIdentitySystem::Login call fails with that error. You can use this to prevent login
     * from starting based on the current game state.
     */
    virtual void OnPreLogin(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FOnPreLoginComplete &InOnComplete);

    /**
     * After the authentication process completes (not necessarily successfully) as part of IIdentitySystem::Login, but
     * before the IIdentitySystem::OnUserSlotChanged event is fired. This allows you to set up any internal state before
     * code relying on IIdentitySystem::OnUserSlotChanged event notifications runs. Internally the plugin uses this to
     * update the FUniqueNetId of local player states and player controllers, so that any blueprint code that runs in
     * response to login completing will see valid player IDs on the local player controllers. If you return an error in
     * the OnComplete callback, the IIdentitySystem::Login call will log the error but otherwise continue as it normally
     * would.
     */
    virtual void OnPostLoginBeforeEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserPtr &InUserIfLoginSuccessful,
        const FOnPostLoginBeforeEventsComplete &InOnComplete);

    /**
     * After the authentication process completes (not necessarily successfully) as part of IIdentitySystem::Login,
     * after the IIdentitySystem::OnUserSlotChanged event is fired. This otherwise has the same semantics as
     * OnPostLoginBeforeEvents.
     */
    virtual void OnPostLoginAfterEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserSlot &InUserSlot,
        const FIdentityUserPtr &InUserIfLoginSuccessful,
        const FOnPostLoginAfterEventsComplete &InOnComplete);

    /**
     * After IIdentitySystem::Logout is called, but before the user is signed out. If you return an error in the
     * OnComplete callback, the IIdentitySystem::Logout call fails with that error and the user remains signed in. You
     * can use this to prevent logout from starting based on the current game state.
     */
    virtual void OnPreLogout(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        const FOnPreLogoutComplete &InOnComplete);

    /**
     * Called when the EOS SDK notifies us that the user has been unexpectedly signed out (typically due to an automatic
     * credential refresh failing). This occurs before the user is internally signed out in the plugin, but you will not
     * be able to call any EOS SDK functions at this point, as the user has already been signed out of Epic Online
     * Services. Any error returned from the OnComplete callback is ignored, as unexpected logouts can not be cancelled.
     * You can use this callback to notify the local player that they need to sign in again or otherwise return them to
     * the main menu.
     */
    virtual void OnPreUnexpectedLogout(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        const FOnPreUnexpectedLogoutComplete &InOnComplete);

    /**
     * This is the logout version of OnPostLoginBeforeEvents and it has the same semantics except that they fire in
     * response to the IIdentitySystem::Logout call instead of IIdentitySystem::Login.
     */
    virtual void OnPostLogoutBeforeEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        bool bLogoutWasSuccessful,
        const FOnPostLogoutBeforeEventsComplete &InOnComplete);

    /**
     * This is the logout version of OnPostLoginAfterEvents and it has the same semantics except that they fire in
     * response to the IIdentitySystem::Logout call instead of IIdentitySystem::Login.
     */
    virtual void OnPostLogoutAfterEvents(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUserPriorToLogout,
        bool bLogoutWasSuccessful,
        const FOnPostLogoutAfterEventsComplete &InOnComplete);

    /**
     * After the user is successfully signed in, or after the logout attempt fails. You can use this hook to start any
     * background systems that need to track local users or perform periodic operations on them. If you return an error
     * in the OnComplete callback, the identity system will log the error but otherwise continue as it normally would.
     */
    virtual void OnStartSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStartSystemComplete &InOnComplete);

    /**
     * After IIdentitySystem::Logout is called and pre-logout hooks have run, but before the user is actually signed
     * out. You should use this if implementing OnStartSystem and cease any API calls that use the local user after this
     * hook runs. If you return an error in the OnComplete callback, the identity system will log the error but
     * otherwise continue as it normally would.
     */
    virtual void OnStopSystem(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        const FOnStopSystemComplete &InOnComplete);

    /**
     * Called when the identity system automatically refreshes credentials for a local player in response to an upcoming
     * credential expiry. For example, when playing on Steam, the EOS SDK requires the plugin to periodically refresh
     * authentication with the EOS backend by providing a new Steam session ticket, and this call is invoked once that
     * credential refresh process completes (not necessarily successfully). If bWasSuccessful was true, the
     * authentication attributes of the FIdentityUser and the token available on IOnlineExternalCredentials may have new
     * values.
     */
    virtual void OnPostCredentialRefresh(
        const FIdentityHookContext &InContext,
        const FIdentityUserRef &InUser,
        bool bWasSuccessful);

    /**
     * Called by the identity system to populate authentication attribute keys on FIdentityUser. You can use this to
     * dynamically provide authentication attributes for local users so they're visible in other systems.
     */
    virtual void OnGetAdditionalAuthenticationAttributeKeys(
        const FIdentityHookContext &InContext,
        const TSharedRef<const FIdentityUser> &InUser,
        TSet<FString> &OutKeys);

    /**
     * Called by the identity system to retrieve the value for authentication attribute keys on FIdentityUser that were
     * dynamically added through OnGetAdditionalAuthenticationAttributeKeys.
     */
    virtual bool OnGetAdditionalAuthenticationAttributeValue(
        const FIdentityHookContext &InContext,
        const TSharedRef<const FIdentityUser> &InUser,
        const FString &Key,
        FString &OutValue);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    /**
     * Called by the identity system to get the IAuthenticationGraphUserInterface implementation, which is used by the
     * RedpointEOSAuth module to display UI widgets on screen or in XR/VR when the login process has an interactive
     * step. You should not need to override this in a hook as the plugin already provides a hook that implements it.
     */
    virtual TSharedPtr<Redpoint::EOS::Auth::IAuthenticationGraphUserInterface> OnAuthenticationUserInterfaceRequired(
        const FIdentityHookContext &InContext);
#endif
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(4071255846, Redpoint::EOS::Identity, IIdentityHook)
}

REDPOINT_EOS_CODE_GUARD_END()