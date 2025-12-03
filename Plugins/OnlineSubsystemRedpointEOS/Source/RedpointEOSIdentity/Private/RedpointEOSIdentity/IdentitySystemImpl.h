// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/ExclusiveAsyncOperationQueue.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSIdentity/IdentityUser.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2503918595, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Identity::Requests;
using namespace ::Redpoint::EOS::Core::Utils;

class FIdentitySystemImpl : public IIdentitySystem,
                            public IHasEventRegistration,
                            public TSharedFromThis<FIdentitySystemImpl>
{
protected:
    const FPlatformHandle PlatformHandle;
    const FPlatformInstanceEnvironmentRef PlatformInstanceEnvironment;

private:
    IIdentitySystem::FOnUserSlotChanged OnUserSlotChangedDelegate;
    TMap<FIdentityUserSlot, FIdentityUserPtr> CurrentUsers;

protected:
    void ExecuteLoginInQueue(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete);

    typedef TDelegate<void(const FError &Error, const FIdentityUserPtr &IdentityUser)> FOnLoginComplete;
    virtual void PerformLogin(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete) = 0;
    virtual void PerformLoginEvents(
        const FLoginRequest &Request,
        const FError &Error,
        const FIdentityUserPtr &IdentityUser) = 0;

    void ExecuteLogoutInQueue(const FLogoutRequest &Request, const IIdentitySystem::FOnLogoutComplete &OnComplete);

    typedef TDelegate<void(const FError &Error, const FIdentityUserPtr &IdentityUser)> FOnLogoutComplete;
    virtual void PerformLogout(
        const FLogoutRequest &Request,
        const FIdentityUserRef &OldIdentityUser,
        const IIdentitySystem::FOnLogoutComplete &OnComplete) = 0;
    virtual void PerformLogoutEvents(
        const FLogoutRequest &Request,
        const FIdentityUserRef &OldIdentityUser,
        const FError &Error) = 0;

    void ExecuteLogoutFromSDKEventInQueue(
        const FIdentityUserRef &AffectedUser,
        const IIdentitySystem::FOnLogoutComplete &OnComplete);

public:
    FIdentitySystemImpl(
        const FPlatformHandle &InPlatformHandle,
        const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment);
    UE_NONCOPYABLE(FIdentitySystemImpl);
    virtual ~FIdentitySystemImpl() override = default;

    FIdentityUserPtr GetUser(const FIdentityUserId &UserId) const override;
    FIdentityUserPtr GetUser(const FIdentityUserSlot &UserSlot) const override;
    TArray<FIdentityUserRef> GetUsers() const override;
    IIdentitySystem::FOnUserSlotChanged &OnUserSlotChanged() override;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(2503918595, Redpoint::EOS::Identity, FIdentitySystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()