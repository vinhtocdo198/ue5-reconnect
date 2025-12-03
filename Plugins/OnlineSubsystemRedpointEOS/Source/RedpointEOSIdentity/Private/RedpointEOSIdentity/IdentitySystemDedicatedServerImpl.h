// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentitySystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1327002834, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Identity::Requests;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API;

class FIdentitySystemDedicatedServerImpl : public FIdentitySystemImpl
{
private:
    TSharedRef<FExclusiveAsyncOperationQueue> OperationQueue;

public:
    FIdentitySystemDedicatedServerImpl(
        const FPlatformHandle &InPlatformHandle,
        const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FIdentitySystemDedicatedServerImpl);
    virtual ~FIdentitySystemDedicatedServerImpl() override = default;

protected:
    virtual void PerformLogin(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete)
        override;
    virtual void PerformLoginEvents(
        const FLoginRequest &Request,
        const FError &Error,
        const FIdentityUserPtr &IdentityUser) override;
    virtual void PerformLogout(
        const FLogoutRequest &Request,
        const FIdentityUserRef &OldIdentityUser,
        const IIdentitySystem::FOnLogoutComplete &OnComplete) override;
    virtual void PerformLogoutEvents(
        const FLogoutRequest &Request,
        const FIdentityUserRef &OldIdentityUser,
        const FError &Error) override;

public:
    virtual void Login(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete) override;
    virtual void Logout(const FLogoutRequest &Request, const IIdentitySystem::FOnLogoutComplete &OnComplete) override;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1327002834, Redpoint::EOS::Identity, FIdentitySystemDedicatedServerImpl)
}

REDPOINT_EOS_CODE_GUARD_END()