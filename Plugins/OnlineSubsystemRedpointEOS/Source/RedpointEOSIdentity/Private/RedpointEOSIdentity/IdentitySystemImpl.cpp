// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentitySystemImpl.h"

#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSIdentity/IdentityProcessGlobalUserRegistry.h"
#include "RedpointEOSIdentity/IdentityUser.h"
#include "RedpointEOSIdentity/Logging.h"
#include "RedpointEOSIdentity/Stats.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2503918595, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Services;

FIdentitySystemImpl::FIdentitySystemImpl(
    const FPlatformHandle &InPlatformHandle,
    const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment)
    : PlatformHandle(InPlatformHandle)
    , PlatformInstanceEnvironment(InPlatformInstanceEnvironment)
    , OnUserSlotChangedDelegate()
    , CurrentUsers()
{
}

void FIdentitySystemImpl::ExecuteLoginInQueue(
    const FLoginRequest &Request,
    const IIdentitySystem::FOnLoginComplete &OnComplete)
{
    FIdentityHookContext Context(this->AsShared(), this->PlatformHandle);

    auto PreTaskGraph = MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Identity_Login));
    auto PostTaskGraph = MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Identity_Login));
    FAsyncTaskPtr PrePreviousTask;
    FAsyncTaskPtr PostPreviousTask;
    auto HolderLoginError = MakeShared<TSharedPtr<FError>>();
    auto HolderIdentityUser = MakeShared<FIdentityUserPtr>();
    auto HolderDidLink = MakeShared<bool>();

    // Add PreLogin hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, Request, Hook](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPreLogin(
                    Context,
                    Request.UserSlot,
                    IIdentityHook::FOnPreLoginComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PreLogin hook failures prevent login.
                        OnTaskComplete.ExecuteIfBound(Error);
                    }));
            },
            PrePreviousTask);
    }

    // Perform login.
    PrePreviousTask = PreTaskGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, Request, HolderIdentityUser, HolderLoginError, HolderDidLink](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->PerformLogin(
                    Request,
                    IIdentitySystem::FOnLoginComplete::CreateSPLambda(
                        this,
                        [this, Request, HolderIdentityUser, HolderLoginError, HolderDidLink, OnTaskComplete](
                            FError ErrorCode,
                            FIdentityUserPtr NewUser) {
                            if (ErrorCode.WasSuccessful())
                            {
                                checkf(
                                    NewUser->GetUserSlot() == Request.UserSlot,
                                    TEXT("Expected PerformLogin to return new user with same slot as request!"));
                                *HolderDidLink = this->CurrentUsers.Contains(Request.UserSlot) &&
                                                 this->CurrentUsers[Request.UserSlot].IsValid();
                                this->CurrentUsers.Add(NewUser->GetUserSlot(), NewUser);
                                *HolderIdentityUser = NewUser;
                            }
                            OnTaskComplete.ExecuteIfBound(ErrorCode);
                        }));
            }),
        PrePreviousTask);

    // Regardless of whether login succeeds, or whether one of the PreLogin tasks fails, we have to go into the
    // PostTaskGraph to fire events.
    PreTaskGraph->Finally([PostTaskGraph, HolderLoginError](const FError &Error) {
        *HolderLoginError = MakeShared<FError>(Error);
        PostTaskGraph->Start();
    });

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Add runtime platform integration OnLocalUserSignInBeforeSystemStart hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Request, RuntimeIntegration, HolderIdentityUser](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // We only run OnLocalUserSignInBeforeSystemStart if login was successful.
                if (HolderIdentityUser->IsValid())
                {
                    auto CallContext = ConstructRuntimePlatformServiceCallContext(
                        Context.PlatformHandle,
                        HolderIdentityUser->ToSharedRef()->GetUserSlot());
                    RuntimeIntegration->OnLocalUserSignInBeforeSystemStart(
                        CallContext,
                        FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
                }
                else
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }
            },
            PostPreviousTask);
    }

#endif

    // Add StartSystem hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Request, Hook, HolderIdentityUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // We only run StartSystem if login was successful.
                if (HolderIdentityUser->IsValid())
                {
                    Hook->OnStartSystem(
                        Context,
                        HolderIdentityUser->ToSharedRef(),
                        IIdentityHook::FOnStartSystemComplete::CreateLambda([OnTaskComplete](FError Error) {
                            // @note: StartSystem hook failures get logged, but don't prevent us from
                            // calling other hooks or firing events.
                            if (!Error.WasSuccessful())
                            {
                                if (GIsAutomationTesting)
                                {
                                    UE_LOG(
                                        LogRedpointEOSIdentity,
                                        Verbose,
                                        TEXT("StartSystem: %s"),
                                        *Error.ToLogString());
                                }
                                else
                                {
                                    UE_LOG(
                                        LogRedpointEOSIdentity,
                                        Error,
                                        TEXT("StartSystem: %s"),
                                        *Error.ToLogString());
                                }
                            }
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
                }
                else
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }
            },
            PostPreviousTask);
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Add runtime platform integration OnLocalUserSignInAfterSystemStart hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Request, RuntimeIntegration, HolderIdentityUser](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // We only run OnLocalUserSignInAfterSystemStart if login was successful.
                if (HolderIdentityUser->IsValid())
                {
                    auto CallContext = ConstructRuntimePlatformServiceCallContext(
                        Context.PlatformHandle,
                        HolderIdentityUser->ToSharedRef()->GetUserSlot());
                    RuntimeIntegration->OnLocalUserSignInAfterSystemStart(
                        CallContext,
                        FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
                }
                else
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }
            },
            PostPreviousTask);
    }

#endif

    // Add PostLoginBeforeEvents hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Request, Hook, HolderIdentityUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPostLoginBeforeEvents(
                    Context,
                    Request.UserSlot,
                    *HolderIdentityUser,
                    IIdentityHook::FOnPostLoginBeforeEventsComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PostLoginBeforeEvent hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Verbose,
                                    TEXT("PostLoginBeforeEvent: %s"),
                                    *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Error,
                                    TEXT("PostLoginBeforeEvent: %s"),
                                    *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PostPreviousTask);
    }

    // Fire events.
    PostPreviousTask = PostTaskGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, Request, HolderLoginError, HolderIdentityUser, HolderDidLink](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->PerformLoginEvents(Request, **HolderLoginError, *HolderIdentityUser);
                if (HolderIdentityUser->IsValid())
                {
                    if (*HolderDidLink)
                    {
                        this->OnUserSlotChanged().Broadcast(
                            Request.UserSlot,
                            EIdentityUserSlotState::SignedIn,
                            (*HolderIdentityUser)->GetUserId(),
                            EIdentityUserSlotState::SignedIn,
                            (*HolderIdentityUser)->GetUserId(),
                            EIdentityUserSlotChangeReason::ExplicitLink);
                    }
                    else
                    {
                        this->OnUserSlotChanged().Broadcast(
                            Request.UserSlot,
                            EIdentityUserSlotState::NotSignedIn,
                            FIdentityUserId(),
                            EIdentityUserSlotState::SignedIn,
                            (*HolderIdentityUser)->GetUserId(),
                            EIdentityUserSlotChangeReason::ExplicitLogin);
                    }
                }
                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        PostPreviousTask);

    // Add PostLoginBeforeEvents hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Request, Hook, HolderIdentityUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPostLoginAfterEvents(
                    Context,
                    Request.UserSlot,
                    *HolderIdentityUser,
                    IIdentityHook::FOnPostLoginAfterEventsComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PostLoginAfterEvent hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Verbose,
                                    TEXT("PostLoginAfterEvent: %s"),
                                    *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Error,
                                    TEXT("PostLoginAfterEvent: %s"),
                                    *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PostPreviousTask);
    }

    // Fire callback.
    PostTaskGraph->FinallyDelegate(FAsyncTaskGraph::FOnExecuteFinally::CreateSPLambda(
        this,
        [HolderLoginError, HolderIdentityUser, OnComplete](const FError &Error) {
            OnComplete.ExecuteIfBound(**HolderLoginError, *HolderIdentityUser);
        }));

    // Start the operation.
    PreTaskGraph->Start();
}

void FIdentitySystemImpl::ExecuteLogoutInQueue(
    const FLogoutRequest &Request,
    const IIdentitySystem::FOnLogoutComplete &OnComplete)
{

    if (!this->CurrentUsers.Contains(Request.UserSlot))
    {
        OnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(TEXT("User %d is not signed in."), Request.UserSlot)));
        return;
    }

    FIdentityHookContext Context(this->AsShared(), this->PlatformHandle);

    auto PreTaskGraph = MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Identity_Logout));
    auto PostTaskGraph = MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Identity_Logout));
    FAsyncTaskPtr PrePreviousTask;
    FAsyncTaskPtr PostPreviousTask;
    auto HolderLogoutError = MakeShared<TSharedPtr<FError>>();
    auto CurrentUser = this->CurrentUsers[Request.UserSlot].ToSharedRef();
    checkf(
        CurrentUser->GetUserSlot() == Request.UserSlot,
        TEXT("User slot is inconsistent with FIdentitySystemImpl state."));

    // Add PreLogout hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, Hook, CurrentUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPreLogout(
                    Context,
                    CurrentUser,
                    IIdentityHook::FOnPreLogoutComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PreLogout hook failures prevent logout.
                        OnTaskComplete.ExecuteIfBound(Error);
                    }));
            },
            PrePreviousTask);
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Add runtime platform integration OnLocalUserSignOutBeforeSystemStop hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, CurrentUser, RuntimeIntegration](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Calling OnLocalUserSignOutBeforeSystemStop."));
                auto CallContext =
                    ConstructRuntimePlatformServiceCallContext(Context.PlatformHandle, CurrentUser->GetUserSlot());
                RuntimeIntegration->OnLocalUserSignOutBeforeSystemStop(
                    CallContext,
                    FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#endif

    // Add StopSystem hooks, in reverse order.
    auto &Hooks = FIdentityHookRegistry::Get();
    for (int HookIndex = Hooks.Num() - 1; HookIndex >= 0; HookIndex--)
    {
        const auto &Hook = Hooks[HookIndex];
        PrePreviousTask = PreTaskGraph->Add(
            [Context, Hook, CurrentUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnStopSystem(
                    Context,
                    CurrentUser,
                    IIdentityHook::FOnStopSystemComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: StopSystem hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(LogRedpointEOSIdentity, Verbose, TEXT("StopSystem: %s"), *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(LogRedpointEOSIdentity, Error, TEXT("StopSystem: %s"), *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Add runtime platform integration OnLocalUserSignOutAfterSystemStop hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, CurrentUser, RuntimeIntegration](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                auto CallContext =
                    ConstructRuntimePlatformServiceCallContext(Context.PlatformHandle, CurrentUser->GetUserSlot());
                RuntimeIntegration->OnLocalUserSignOutAfterSystemStop(
                    CallContext,
                    FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#endif

    // Perform logout.
    PrePreviousTask = PreTaskGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, Request, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->PerformLogout(
                    Request,
                    CurrentUser,
                    IIdentitySystem::FOnLogoutComplete::CreateSPLambda(
                        this,
                        [this, Request, CurrentUser, HolderLogoutError, OnTaskComplete](FError ErrorCode) {
                            if (ErrorCode.WasSuccessful())
                            {
                                this->CurrentUsers.Remove(Request.UserSlot);
                                FIdentityProcessGlobalUserRegistry::Unregister(
                                    CurrentUser->GetProcessGlobalUserHandle());
                            }
                            OnTaskComplete.ExecuteIfBound(ErrorCode);
                        }));
            }),
        PrePreviousTask);

    // Regardless of whether logout succeeds, or whether one of the PreLogout tasks fails, we have to go into the
    // PostTaskGraph to fire events.
    PreTaskGraph->Finally([PostTaskGraph, HolderLogoutError](const FError &Error) {
        *HolderLogoutError = MakeShared<FError>(Error);
        PostTaskGraph->Start();
    });

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // If the logout operation fails, we need to call runtime platform integration OnLocalUserSignInBeforeSystemStart
    // hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, RuntimeIntegration, CurrentUser, HolderLogoutError](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // We only run OnLocalUserSignInBeforeSystemStart if logout failed.
                if (!(*HolderLogoutError)->WasSuccessful())
                {
                    auto CallContext =
                        ConstructRuntimePlatformServiceCallContext(Context.PlatformHandle, CurrentUser->GetUserSlot());
                    RuntimeIntegration->OnLocalUserSignInBeforeSystemStart(
                        CallContext,
                        FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
                }
                else
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }
            },
            PostPreviousTask);
    }

#endif

    // If the logout operation fails, we need to call StartSystem to get background systems running again.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Hook, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                if (!(*HolderLogoutError)->WasSuccessful())
                {
                    Hook->OnStartSystem(
                        Context,
                        CurrentUser,
                        IIdentityHook::FOnStartSystemComplete::CreateLambda([OnTaskComplete](FError Error) {
                            // @note: StartSystem hook failures get logged, but don't prevent us from
                            // calling other hooks or firing events.
                            if (!Error.WasSuccessful())
                            {
                                if (GIsAutomationTesting)
                                {
                                    UE_LOG(
                                        LogRedpointEOSIdentity,
                                        Verbose,
                                        TEXT("StartSystem: %s"),
                                        *Error.ToLogString());
                                }
                                else
                                {
                                    UE_LOG(
                                        LogRedpointEOSIdentity,
                                        Error,
                                        TEXT("StartSystem: %s"),
                                        *Error.ToLogString());
                                }
                            }
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
                }
                else
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }
            },
            PostPreviousTask);
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // If the logout operation fails, we need to call runtime platform integration OnLocalUserSignInAfterSystemStart
    // hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, RuntimeIntegration, CurrentUser, HolderLogoutError](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                // We only run OnLocalUserSignInAfterSystemStart if login was successful.
                if (!(*HolderLogoutError)->WasSuccessful())
                {
                    auto CallContext =
                        ConstructRuntimePlatformServiceCallContext(Context.PlatformHandle, CurrentUser->GetUserSlot());
                    RuntimeIntegration->OnLocalUserSignInAfterSystemStart(
                        CallContext,
                        FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
                }
                else
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }
            },
            PostPreviousTask);
    }

#endif

    // Add PostLogoutBeforeEvents hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Hook, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPostLogoutBeforeEvents(
                    Context,
                    CurrentUser,
                    (*HolderLogoutError)->WasSuccessful(),
                    IIdentityHook::FOnPostLogoutBeforeEventsComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PostLogoutBeforeEvent hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Verbose,
                                    TEXT("PostLogoutBeforeEvent: %s"),
                                    *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Error,
                                    TEXT("PostLogoutBeforeEvent: %s"),
                                    *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PostPreviousTask);
    }

    // Fire events.
    PostPreviousTask = PostTaskGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, Request, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->PerformLogoutEvents(Request, CurrentUser, **HolderLogoutError);
                if ((*HolderLogoutError)->WasSuccessful())
                {
                    this->OnUserSlotChanged().Broadcast(
                        Request.UserSlot,
                        EIdentityUserSlotState::SignedIn,
                        CurrentUser->GetUserId(),
                        EIdentityUserSlotState::NotSignedIn,
                        FIdentityUserId(),
                        EIdentityUserSlotChangeReason::ExplicitLogout);
                }
                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        PostPreviousTask);

    // Add PostLoginBeforeEvents hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Hook, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPostLogoutAfterEvents(
                    Context,
                    CurrentUser,
                    (*HolderLogoutError)->WasSuccessful(),
                    IIdentityHook::FOnPostLogoutAfterEventsComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PostLogoutAfterEvent hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Verbose,
                                    TEXT("PostLogoutAfterEvent: %s"),
                                    *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Error,
                                    TEXT("PostLogoutAfterEvent: %s"),
                                    *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PostPreviousTask);
    }

    // Fire callback.
    PostTaskGraph->FinallyDelegate(
        FAsyncTaskGraph::FOnExecuteFinally::CreateSPLambda(this, [HolderLogoutError, OnComplete](const FError &Error) {
            OnComplete.ExecuteIfBound(**HolderLogoutError);
        }));

    // Start the operation.
    PreTaskGraph->Start();
}

void FIdentitySystemImpl::ExecuteLogoutFromSDKEventInQueue(
    const FIdentityUserRef &CurrentUser,
    const IIdentitySystem::FOnLogoutComplete &OnComplete)
{

    bool bUserFound = false;
    FIdentityUserSlot UserSlot = 0;
    for (const auto &User : this->CurrentUsers)
    {
        if (User.Value == CurrentUser)
        {
            UserSlot = User.Value->GetUserSlot();
            bUserFound = true;
        }
    }
    if (!bUserFound)
    {
        OnComplete.ExecuteIfBound(Errors::NotFound(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The user passed to ExecuteLogoutFromSDKEventInQueue is no longer tracked and may already have been "
                 "signed out.")));
    }
    FLogoutRequest Request(UserSlot);

    FIdentityHookContext Context(this->AsShared(), this->PlatformHandle);

    auto PreTaskGraph = MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Identity_UnexpectedLogout));
    auto PostTaskGraph = MakeShared<FAsyncTaskGraph>(GET_STATID(STAT_RedpointEOS_Identity_UnexpectedLogout));
    FAsyncTaskPtr PrePreviousTask;
    FAsyncTaskPtr PostPreviousTask;
    auto HolderLogoutError = MakeShared<TSharedPtr<FError>>();

    // Add PreUnexpectedLogout hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, Hook, CurrentUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPreUnexpectedLogout(
                    Context,
                    CurrentUser,
                    IIdentityHook::FOnPreUnexpectedLogoutComplete::CreateLambda([OnTaskComplete]() {
                        // @note: Unexpected logouts can not fail.
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Add runtime platform integration OnLocalUserSignOutBeforeSystemStop hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, CurrentUser, RuntimeIntegration](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Calling OnLocalUserSignOutBeforeSystemStop."));
                auto CallContext =
                    ConstructRuntimePlatformServiceCallContext(Context.PlatformHandle, CurrentUser->GetUserSlot());
                RuntimeIntegration->OnLocalUserSignOutBeforeSystemStop(
                    CallContext,
                    FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#endif

    // Add StopSystem hooks, in reverse order.
    auto &Hooks = FIdentityHookRegistry::Get();
    for (int HookIndex = Hooks.Num() - 1; HookIndex >= 0; HookIndex--)
    {
        const auto &Hook = Hooks[HookIndex];
        PrePreviousTask = PreTaskGraph->Add(
            [Context, Hook, CurrentUser](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnStopSystem(
                    Context,
                    CurrentUser,
                    IIdentityHook::FOnStopSystemComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: StopSystem hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(LogRedpointEOSIdentity, Verbose, TEXT("StopSystem: %s"), *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(LogRedpointEOSIdentity, Error, TEXT("StopSystem: %s"), *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

    // Add runtime platform integration OnLocalUserSignOutAfterSystemStop hooks.
    for (const auto &RuntimeIntegration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        PrePreviousTask = PreTaskGraph->Add(
            [Context, CurrentUser, RuntimeIntegration](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                auto CallContext =
                    ConstructRuntimePlatformServiceCallContext(Context.PlatformHandle, CurrentUser->GetUserSlot());
                RuntimeIntegration->OnLocalUserSignOutAfterSystemStop(
                    CallContext,
                    FSimpleDelegate::CreateLambda([OnTaskComplete]() {
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PrePreviousTask);
    }

#endif

    // Perform logout.
    PrePreviousTask = PreTaskGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, Request, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->PerformLogout(
                    Request,
                    CurrentUser,
                    IIdentitySystem::FOnLogoutComplete::CreateSPLambda(
                        this,
                        [this, Request, CurrentUser, HolderLogoutError, OnTaskComplete](FError ErrorCode) {
                            // With an unexpected logout, it doesn't matter if the actual logout operation fails,
                            // because the SDK already logged us out. Log the error if logout fails, but then proceed as
                            // if it succeeded.
                            if (!ErrorCode.WasSuccessful())
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Warning,
                                    TEXT("PerformLogout call failed after unexpected logout: %s"),
                                    *ErrorCode.ToLogString());
                            }
                            this->CurrentUsers.Remove(Request.UserSlot);
                            FIdentityProcessGlobalUserRegistry::Unregister(CurrentUser->GetProcessGlobalUserHandle());
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        PrePreviousTask);

    // Regardless of whether logout succeeds, or whether one of the PreLogout tasks fails, we have to go into the
    // PostTaskGraph to fire events.
    PreTaskGraph->Finally([PostTaskGraph, HolderLogoutError](const FError &Error) {
        *HolderLogoutError = MakeShared<FError>(Error);
        PostTaskGraph->Start();
    });

    // Add PostLogoutBeforeEvents hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Hook, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPostLogoutBeforeEvents(
                    Context,
                    CurrentUser,
                    (*HolderLogoutError)->WasSuccessful(),
                    IIdentityHook::FOnPostLogoutBeforeEventsComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PostLogoutBeforeEvent hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Verbose,
                                    TEXT("PostLogoutBeforeEvent: %s"),
                                    *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Error,
                                    TEXT("PostLogoutBeforeEvent: %s"),
                                    *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PostPreviousTask);
    }

    // Fire events.
    PostPreviousTask = PostTaskGraph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, Request, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                this->PerformLogoutEvents(Request, CurrentUser, **HolderLogoutError);
                if ((*HolderLogoutError)->WasSuccessful())
                {
                    this->OnUserSlotChanged().Broadcast(
                        Request.UserSlot,
                        EIdentityUserSlotState::SignedIn,
                        CurrentUser->GetUserId(),
                        EIdentityUserSlotState::NotSignedIn,
                        FIdentityUserId(),
                        EIdentityUserSlotChangeReason::ExplicitLogout);
                }
                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        PostPreviousTask);

    // Add PostLoginBeforeEvents hooks.
    for (const auto &Hook : FIdentityHookRegistry::Get())
    {
        PostPreviousTask = PostTaskGraph->Add(
            [Context, Hook, CurrentUser, HolderLogoutError](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Hook->OnPostLogoutAfterEvents(
                    Context,
                    CurrentUser,
                    (*HolderLogoutError)->WasSuccessful(),
                    IIdentityHook::FOnPostLogoutAfterEventsComplete::CreateLambda([OnTaskComplete](FError Error) {
                        // @note: PostLogoutAfterEvent hook failures get logged, but don't prevent us from
                        // calling other hooks or firing events.
                        if (!Error.WasSuccessful())
                        {
                            if (GIsAutomationTesting)
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Verbose,
                                    TEXT("PostLogoutAfterEvent: %s"),
                                    *Error.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSIdentity,
                                    Error,
                                    TEXT("PostLogoutAfterEvent: %s"),
                                    *Error.ToLogString());
                            }
                        }
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                    }));
            },
            PostPreviousTask);
    }

    // Fire callback.
    PostTaskGraph->FinallyDelegate(
        FAsyncTaskGraph::FOnExecuteFinally::CreateSPLambda(this, [HolderLogoutError, OnComplete](const FError &Error) {
            OnComplete.ExecuteIfBound(**HolderLogoutError);
        }));

    // Start the operation.
    PreTaskGraph->Start();
}

FIdentityUserPtr FIdentitySystemImpl::GetUser(const FIdentityUserId &UserId) const
{
    for (const auto &KV : this->CurrentUsers)
    {
        if (KV.Value->GetUserId() == UserId)
        {
            return KV.Value;
        }
    }
    return nullptr;
}

FIdentityUserPtr FIdentitySystemImpl::GetUser(const FIdentityUserSlot &UserSlot) const
{
    return this->CurrentUsers.FindRef(UserSlot);
}

TArray<FIdentityUserRef> FIdentitySystemImpl::GetUsers() const
{
    TArray<FIdentityUserRef> Results;
    for (const auto &KV : this->CurrentUsers)
    {
        Results.Add(KV.Value.ToSharedRef());
    }
    return Results;
}

IIdentitySystem::FOnUserSlotChanged &FIdentitySystemImpl::OnUserSlotChanged()
{
    return this->OnUserSlotChangedDelegate;
}

}

REDPOINT_EOS_CODE_GUARD_END()