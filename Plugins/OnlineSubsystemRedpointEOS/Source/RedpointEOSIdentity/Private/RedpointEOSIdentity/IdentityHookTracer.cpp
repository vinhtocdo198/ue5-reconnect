// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentityHookTracer.h"

#include "Containers/Ticker.h"
#include "RedpointEOSIdentity/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !NO_LOGGING

namespace REDPOINT_EOS_FILE_NS_ID(1332459060, Redpoint::EOS::Identity)
{

#define REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(FunctionName, ...)                                                       \
    auto DebugName = Hook->GetHookDebugName();                                                                         \
    UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook begin: %s::%s"), *DebugName, TEXT(#FunctionName));          \
    auto TimeoutHandle = MakeShared<FTSTicker::FDelegateHandle>();                                                     \
    auto Handled = MakeShared<bool>();                                                                                 \
    *TimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(                                                             \
        FTickerDelegate::CreateLambda([InOnComplete, TimeoutHandle, Handled, DebugName](float) -> bool {               \
            if (*Handled)                                                                                              \
            {                                                                                                          \
                return false;                                                                                          \
            }                                                                                                          \
            *Handled = true;                                                                                           \
            UE_LOG(                                                                                                    \
                LogRedpointEOSIdentity,                                                                                \
                Error,                                                                                                 \
                TEXT("Hook timed out: %s::%s: %s"),                                                                    \
                *DebugName,                                                                                            \
                TEXT(#FunctionName),                                                                                   \
                *Errors::TimedOut().ToLogString());                                                                    \
            InOnComplete.ExecuteIfBound(Errors::TimedOut());                                                           \
            return false;                                                                                              \
        }),                                                                                                            \
        10.0f);                                                                                                        \
    this->Hook->FunctionName(                                                                                          \
        __VA_ARGS__,                                                                                                   \
        F##FunctionName##Complete::CreateLambda([InOnComplete, TimeoutHandle, Handled, DebugName](FError ErrorCode) {  \
            if (*Handled)                                                                                              \
            {                                                                                                          \
                return;                                                                                                \
            }                                                                                                          \
            *Handled = true;                                                                                           \
            FTSTicker::GetCoreTicker().RemoveTicker(*TimeoutHandle);                                                   \
            UE_LOG(                                                                                                    \
                LogRedpointEOSIdentity,                                                                                \
                VeryVerbose,                                                                                           \
                TEXT("Hook end: %s::%s: %s"),                                                                          \
                *DebugName,                                                                                            \
                TEXT(#FunctionName),                                                                                   \
                *ErrorCode.ToLogString());                                                                             \
            InOnComplete.ExecuteIfBound(ErrorCode);                                                                    \
        }));
#define REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_NO_ERROR(FunctionName, ...)                                              \
    auto DebugName = Hook->GetHookDebugName();                                                                         \
    UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook begin: %s::%s"), *DebugName, TEXT(#FunctionName));          \
    this->Hook->FunctionName(                                                                                          \
        __VA_ARGS__,                                                                                                   \
        F##FunctionName##Complete::CreateLambda([InOnComplete, DebugName]() {                                          \
            UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook end: %s::%s"), *DebugName, TEXT(#FunctionName));    \
            InOnComplete.ExecuteIfBound();                                                                             \
        }));
#define REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_SYNC(FunctionName, ...)                                                  \
    auto DebugName = Hook->GetHookDebugName();                                                                         \
    UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook begin: %s::%s"), *DebugName, TEXT(#FunctionName));          \
    this->Hook->FunctionName(__VA_ARGS__);                                                                             \
    UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook end: %s::%s"), *DebugName, TEXT(#FunctionName));
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#define REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_RETURN(FunctionName, ...)                                                \
    auto DebugName = Hook->GetHookDebugName();                                                                         \
    UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook begin: %s::%s"), *DebugName, TEXT(#FunctionName));          \
    auto Result = this->Hook->FunctionName(__VA_ARGS__);                                                               \
    UE_LOG(LogRedpointEOSIdentity, VeryVerbose, TEXT("Hook end: %s::%s"), *DebugName, TEXT(#FunctionName));            \
    return Result;
#endif

FIdentityHookTracer::FIdentityHookTracer(const TSharedRef<IIdentityHook> &InHook)
    : Hook(InHook)
{
}

FString FIdentityHookTracer::GetHookDebugName() const
{
    return this->Hook->GetHookDebugName();
}

void FIdentityHookTracer::OnPreLogin(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FOnPreLoginComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(OnPreLogin, InContext, InUserSlot);
}

void FIdentityHookTracer::OnPostLoginBeforeEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserPtr &InUserIfLoginSuccessful,
    const FOnPostLoginBeforeEventsComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(OnPostLoginBeforeEvents, InContext, InUserSlot, InUserIfLoginSuccessful);
}

void FIdentityHookTracer::OnPostLoginAfterEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserPtr &InUserIfLoginSuccessful,
    const FOnPostLoginAfterEventsComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(OnPostLoginAfterEvents, InContext, InUserSlot, InUserIfLoginSuccessful);
}

void FIdentityHookTracer::OnPreLogout(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    const FOnPreLogoutComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(OnPreLogout, InContext, InUserPriorToLogout);
}

void FIdentityHookTracer::OnPreUnexpectedLogout(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    const FOnPreUnexpectedLogoutComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_NO_ERROR(OnPreUnexpectedLogout, InContext, InUserPriorToLogout);
}

void FIdentityHookTracer::OnPostLogoutBeforeEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    bool bLogoutWasSuccessful,
    const FOnPostLogoutBeforeEventsComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(
        OnPostLogoutBeforeEvents,
        InContext,
        InUserPriorToLogout,
        bLogoutWasSuccessful);
}

void FIdentityHookTracer::OnPostLogoutAfterEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUserPriorToLogout,
    bool bLogoutWasSuccessful,
    const FOnPostLogoutAfterEventsComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(
        OnPostLogoutAfterEvents,
        InContext,
        InUserPriorToLogout,
        bLogoutWasSuccessful);
}

void FIdentityHookTracer::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(OnStartSystem, InContext, InUser);
}

void FIdentityHookTracer::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD(OnStopSystem, InContext, InUser);
}

void FIdentityHookTracer::OnPostCredentialRefresh(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    bool bWasSuccessful)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_SYNC(OnPostCredentialRefresh, InContext, InUser, bWasSuccessful);
}

void FIdentityHookTracer::OnGetAdditionalAuthenticationAttributeKeys(
    const FIdentityHookContext &InContext,
    const TSharedRef<const FIdentityUser> &InUser,
    TSet<FString> &OutKeys)
{
    // @note: We don't trace this function call because it can be extremely frequent.
    this->Hook->OnGetAdditionalAuthenticationAttributeKeys(InContext, InUser, OutKeys);
}

bool FIdentityHookTracer::OnGetAdditionalAuthenticationAttributeValue(
    const FIdentityHookContext &InContext,
    const TSharedRef<const FIdentityUser> &InUser,
    const FString &Key,
    FString &OutValue)
{
    // @note: We don't trace this function call because it can be extremely frequent.
    return this->Hook->OnGetAdditionalAuthenticationAttributeValue(InContext, InUser, Key, OutValue);
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedPtr<Redpoint::EOS::Auth::IAuthenticationGraphUserInterface> FIdentityHookTracer::
    OnAuthenticationUserInterfaceRequired(const FIdentityHookContext &InContext)
{
    REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_RETURN(OnAuthenticationUserInterfaceRequired, InContext);
}

#endif

#undef REDPOINT_IDENTITY_HOOK_TRACER_FORWARD
#undef REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_NO_ERROR
#undef REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_SYNC
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#undef REDPOINT_IDENTITY_HOOK_TRACER_FORWARD_RETURN
#endif

}

#endif

REDPOINT_EOS_CODE_GUARD_END()