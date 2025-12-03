// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentitySystemDedicatedServerImpl.h"

#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSIdentity/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1327002834, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Auth;

FIdentitySystemDedicatedServerImpl::FIdentitySystemDedicatedServerImpl(
    const FPlatformHandle &InPlatformHandle,
    const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment)
    : FIdentitySystemImpl(InPlatformHandle, InPlatformInstanceEnvironment)
    , OperationQueue(MakeShared<FExclusiveAsyncOperationQueue>())
{
}

void FIdentitySystemDedicatedServerImpl::RegisterEvents()
{
}

void FIdentitySystemDedicatedServerImpl::PerformLogin(
    const FLoginRequest &Request,
    const IIdentitySystem::FOnLoginComplete &OnComplete)
{
    using namespace ::Redpoint::EOS::Core::Id;

    auto User = FIdentityUser::Create(
        this->AsShared(),
        this->PlatformHandle,
        Request.UserSlot,
        GetDedicatedServerAccountId(),
        NAME_None,
        FAuthenticationGraphRefreshEOSCredentials(),
        nullptr,
        nullptr,
        TMap<FString, FString>(),
        nullptr);

    OnComplete.ExecuteIfBound(Errors::Success(), User);
}

void FIdentitySystemDedicatedServerImpl::PerformLoginEvents(
    const FLoginRequest &Request,
    const FError &Error,
    const FIdentityUserPtr &IdentityUser)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (Error.WasSuccessful())
    {
        UE_LOG(LogRedpointEOSIdentity, Verbose, TEXT("DedicatedServer: Performed Login on dedicated server."));
        UE_LOG(
            LogRedpointEOSIdentity,
            Verbose,
            TEXT("Local user %d is now signed in as %s"),
            Request.UserSlot,
            *GetUserIdDebugString(IdentityUser->GetUserId()));
    }
}

void FIdentitySystemDedicatedServerImpl::PerformLogout(
    const FLogoutRequest &Request,
    const FIdentityUserRef &OldIdentityUser,
    const IIdentitySystem::FOnLogoutComplete &OnComplete)
{
    OnComplete.ExecuteIfBound(Errors::Success());
}

void FIdentitySystemDedicatedServerImpl::PerformLogoutEvents(
    const FLogoutRequest &Request,
    const FIdentityUserRef &OldIdentityUser,
    const FError &Error)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (Error.WasSuccessful())
    {
        UE_LOG(LogRedpointEOSIdentity, Verbose, TEXT("DedicatedServer: Performed Logout on dedicated server."));
        UE_LOG(LogRedpointEOSIdentity, Verbose, TEXT("Local user %d is now signed out"), Request.UserSlot);
    }
}

void FIdentitySystemDedicatedServerImpl::Login(
    const FLoginRequest &Request,
    const IIdentitySystem::FOnLoginComplete &OnComplete)
{
    if (Request.UserSlot != 0)
    {
        OnComplete.ExecuteIfBound(
            Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("You can only sign into local user 0 when running as a dedicated server.")),
            nullptr);
        return;
    }

    this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
        this,
        [this, Request, OnComplete](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
            auto ExistingDedicatedServerUser = this->GetUser(Request.UserSlot);
            if (ExistingDedicatedServerUser.IsValid())
            {
                // @note: Dedicated servers don't permit linking flows.
                OnComplete.ExecuteIfBound(
                    Errors::InvalidParameters(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("User %d is already signed in."), Request.UserSlot)),
                    ExistingDedicatedServerUser);
                return;
            }

            this->ExecuteLoginInQueue(
                Request,
                IIdentitySystem::FOnLoginComplete::CreateSPLambda(
                    this,
                    [OnComplete, InOnDone](FError ErrorCode, FIdentityUserPtr NewUser) {
                        OnComplete.ExecuteIfBound(ErrorCode, NewUser);
                        InOnDone.ExecuteIfBound();
                    }));

            FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
        }));
}

void FIdentitySystemDedicatedServerImpl::Logout(
    const FLogoutRequest &Request,
    const IIdentitySystem::FOnLogoutComplete &OnComplete)
{
    if (Request.UserSlot != 0)
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("You can only sign out of local user 0 when running as a dedicated server.")));
        return;
    }

    this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSPLambda(
        this,
        [this, Request, OnComplete](const FExclusiveAsyncOperationQueue::FOnDone &InOnDone) {
            this->ExecuteLogoutInQueue(
                Request,
                IIdentitySystem::FOnLogoutComplete::CreateSPLambda(this, [OnComplete, InOnDone](FError ErrorCode) {
                    OnComplete.ExecuteIfBound(ErrorCode);
                    InOnDone.ExecuteIfBound();
                }));
        }));
}

}

REDPOINT_EOS_CODE_GUARD_END()