// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/Exec.h"
#include "RedpointEOSAPI/Connect/NotifyAuthExpiration.h"
#include "RedpointEOSAPI/Connect/NotifyLoginStatusChanged.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphNodeResult.h"
#include "RedpointEOSAuth/AuthenticationGraphOnLogout.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Exec/ExecWithAutoComplete.h"
#include "RedpointEOSIdentity/IdentitySystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1586771502, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Identity::Requests;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS;

class FIdentitySystemGameImpl : public FIdentitySystemImpl, public FExecWithAutoComplete
{
private:
    TMap<FIdentityUserSlot, TSharedPtr<FExclusiveAsyncOperationQueue>> OperationQueue;
    TMap<TSharedRef<FIdentityUser>, FAuthenticationGraphOnLogout> Logouts;
    TSet<TSharedRef<FIdentityUser>> ExpectedLogouts;
    FEventHandlePtr AuthExpirationHandle;
    FEventHandlePtr LoginStatusChangedHandle;

private:
    TSharedRef<FExclusiveAsyncOperationQueue> GetOperationQueue(FIdentityUserSlot UserSlot);
    void OnAuthExpiration(const API::Connect::FNotifyAuthExpiration::Result &Result);
    void OnLoginStatusChanged(const API::Connect::FNotifyLoginStatusChanged::Result &Result);

    void HandleLoginExpiry(const FIdentityUserRef &AffectedUser);
    void OnLoginExpiryRefreshComplete(
        bool bWasSuccessful,
        TSharedRef<FAuthenticationGraphRefreshEOSCredentialsInfo> Info,
        FIdentityUserRef AffectedUser);

public:
    FIdentitySystemGameImpl(
        const FPlatformHandle &InPlatformHandle,
        const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FIdentitySystemGameImpl);
    virtual ~FIdentitySystemGameImpl() override = default;

protected:
    virtual void PerformLogin(const FLoginRequest &Request, const IIdentitySystem::FOnLoginComplete &OnComplete)
        override;

private:
    void OnAuthenticationGraphComplete(
        EAuthenticationGraphNodeResult Result,
        const FAuthenticationGraphOnLogout &OnLogout,
        TSharedRef<FAuthenticationGraphState> State,
        TSharedRef<const FAuthenticationGraph> Graph,
        IIdentitySystem::FOnLoginComplete OnComplete);

protected:
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

    virtual void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &) override;
    virtual bool Exec_Dev(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar) override;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1586771502, Redpoint::EOS::Identity, FIdentitySystemGameImpl)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()