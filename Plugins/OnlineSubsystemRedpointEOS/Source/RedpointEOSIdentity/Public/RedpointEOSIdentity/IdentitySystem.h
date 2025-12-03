// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityProcessGlobalUserHandle.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSIdentity/IdentityUserSlot.h"
#include "RedpointEOSIdentity/IdentityUserSlotChangeReason.h"
#include "RedpointEOSIdentity/IdentityUserSlotState.h"
#include "RedpointEOSIdentity/Requests/LoginRequest.h"
#include "RedpointEOSIdentity/Requests/LogoutRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(526824123, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity::Requests;

class REDPOINTEOSIDENTITY_API IIdentitySystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IIdentitySystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IIdentitySystem() = default;
    UE_NONCOPYABLE(IIdentitySystem);
    virtual ~IIdentitySystem() override = default;

    typedef TDelegate<void(FError ErrorCode, FIdentityUserPtr NewUser)> FOnLoginComplete;
    typedef TDelegate<void(FError ErrorCode)> FOnLogoutComplete;

    typedef TMulticastDelegate<void(
        FIdentityUserSlot UserSlot,
        EIdentityUserSlotState OldUserSlotState,
        FIdentityUserId OldUserId,
        EIdentityUserSlotState NewUserSlotState,
        FIdentityUserId NewUserId,
        EIdentityUserSlotChangeReason ChangeReason)>
        FOnUserSlotChanged;

    /**
     * Log a user into a user slot.
     */
    virtual void Login(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete) = 0;

    /**
     * Log a user out of a user slot.
     */
    virtual void Logout(const FLogoutRequest &Request, const IIdentitySystem::FOnLogoutComplete &OnComplete) = 0;

    /**
     * Return a currently signed in user by their user ID.
     */
    virtual FIdentityUserPtr GetUser(const FIdentityUserId &UserId) const = 0;

    /**
     * Return a currently signed in user by the user slot.
     */
    virtual FIdentityUserPtr GetUser(const FIdentityUserSlot &UserSlot) const = 0;

    /**
     * Return all of the currently signed in users.
     */
    virtual TArray<FIdentityUserRef> GetUsers() const = 0;

    /**
     * The event which fires when a user slot changes state.
     */
    virtual FOnUserSlotChanged &OnUserSlotChanged() = 0;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(526824123, Redpoint::EOS::Identity, IIdentitySystem)
}

REDPOINT_EOS_CODE_GUARD_END()