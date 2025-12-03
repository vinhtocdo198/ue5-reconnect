// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/CallbackValidator.h"

#include "Misc/CoreMisc.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Memory.h"
#if WITH_GAMEPLAY_DEBUGGER_CORE
#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

struct FCallbackInfo
{
public:
    FString CallbackName;
    TOptional<EOS_EResult> FirstResultCode;
};

struct FEventInfo
{
public:
    FString EventName;
};

static TMap<void *, FCallbackInfo> GRecentCallbacks;
static TArray<void *> GRecentCallbackPtrsByAge;
static TMap<void *, FCallbackInfo> GLiveCallbacks;

static TMap<FString, int> GLiveCallbackCount;

static TMap<void *, FEventInfo> GLiveEvents;
static TMap<EOS_NotificationId, void *> GLiveEventPtrsByNotificationId;
static TMap<void *, FEventInfo> GRecentEvents;
static TArray<void *> GRecentEventPtrsByAge;

void FCallbackValidator::TrackCallback(void *HeapState, const FString &CallName)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    GLiveCallbacks.Add(HeapState, FCallbackInfo{CallName, TOptional<EOS_EResult>()});
    GLiveCallbackCount.FindOrAdd(CallName)++;
}

void FCallbackValidator::TrackCallback(void *HeapState)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    GLiveCallbacks.Add(HeapState, FCallbackInfo{TEXT("(unavailable)"), TOptional<EOS_EResult>()});
    GLiveCallbackCount.FindOrAdd(TEXT("(unavailable)"))++;
}

bool FCallbackValidator::VerifyCallback(void *HeapState, const TOptional<EOS_EResult> &ResultCode)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    auto *CallbackState = GLiveCallbacks.Find(HeapState);
    if (CallbackState != nullptr)
    {
        // This is a live, valid callback.
        if (ResultCode.IsSet())
        {
            CallbackState->FirstResultCode = ResultCode;
        }
        GRecentCallbacks.Add(HeapState, *CallbackState);
        GRecentCallbackPtrsByAge.Add(HeapState);
        GLiveCallbackCount.FindOrAdd(CallbackState->CallbackName)--;
        GLiveCallbacks.Remove(HeapState);

        if (GRecentCallbackPtrsByAge.Num() > 1000)
        {
            GRecentCallbacks.Remove(GRecentCallbackPtrsByAge[0]);
            GRecentCallbackPtrsByAge.RemoveAt(0);
        }

        return true;
    }
    else
    {
        // The EOS SDK has made a double call.
        auto *RecentCallbackState = GRecentCallbacks.Find(HeapState);
        if (RecentCallbackState != nullptr)
        {
            checkf(
                false,
                TEXT("The EOS SDK incorrectly invoked the callback of the asynchronous SDK function '%s' more than "
                     "once. This is a bug in the EOS SDK and should be reported to Epic Games on "
                     "https://eoshelp.epicgames.com/; Redpoint Games is unable to fix this bug as it is in the EOS SDK "
                     "itself. For more detailed information, attach a debugger. The result code that the EOS SDK "
                     "returned the first time it invoked the callback was: %s"),
                *RecentCallbackState->CallbackName,
                RecentCallbackState->FirstResultCode.IsSet()
                    ? (*ConvertError(RecentCallbackState->FirstResultCode.GetValue()).ToLogString())
                    : TEXT("(unavailable)"));
        }
        else
        {
            checkf(
                false,
                TEXT("The EOS SDK incorrectly invoked the callback of the asynchronous SDK function more than once. "
                     "This is a bug in the EOS SDK and should be reported to Epic Games on "
                     "https://eoshelp.epicgames.com/; Redpoint Games is unable to fix this bug as it is in the EOS SDK "
                     "itself. For more detailed information, attach a debugger. The name of the callback and the "
                     "original result code are unavailable as the call has dropped off the 'recent callbacks' list "
                     "(limited to 1000 calls)."));
        }

        return false;
    }
}

bool FCallbackValidator::VerifyCallbackWithoutRelease(void *HeapState, const TOptional<EOS_EResult> &ResultCode)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    auto *CallbackState = GLiveCallbacks.Find(HeapState);
    if (CallbackState != nullptr)
    {
        // This is a live, valid callback.
        if (ResultCode.IsSet())
        {
            CallbackState->FirstResultCode = ResultCode;
        }
        return true;
    }
    else
    {
        // The EOS SDK has made a double call.
        auto *RecentCallbackState = GRecentCallbacks.Find(HeapState);
        if (RecentCallbackState != nullptr)
        {
            checkf(
                false,
                TEXT("The EOS SDK incorrectly invoked the callback of the asynchronous SDK function '%s' more than "
                     "once. This is a bug in the EOS SDK and should be reported to Epic Games on "
                     "https://eoshelp.epicgames.com/; Redpoint Games is unable to fix this bug as it is in the EOS SDK "
                     "itself. For more detailed information, attach a debugger. The result code that the EOS SDK "
                     "returned the first time it invoked the callback was: %s"),
                *RecentCallbackState->CallbackName,
                RecentCallbackState->FirstResultCode.IsSet()
                    ? (*ConvertError(RecentCallbackState->FirstResultCode.GetValue()).ToLogString())
                    : TEXT("(unavailable)"));
        }
        else
        {
            checkf(
                false,
                TEXT("The EOS SDK incorrectly invoked the callback of the asynchronous SDK function more than once. "
                     "This is a bug in the EOS SDK and should be reported to Epic Games on "
                     "https://eoshelp.epicgames.com/; Redpoint Games is unable to fix this bug as it is in the EOS SDK "
                     "itself. For more detailed information, attach a debugger. The name of the callback and the "
                     "original result code are unavailable as the call has dropped off the 'recent callbacks' list "
                     "(limited to 1000 calls)."));
        }

        return false;
    }
}

void FCallbackValidator::TrackEvent(void *HeapState, const FString &EventName, EOS_NotificationId NotificationId)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    GLiveEvents.Add(HeapState, FEventInfo{EventName});
    GLiveEventPtrsByNotificationId.Add(NotificationId, HeapState);
}

bool FCallbackValidator::VerifyEvent(void *HeapState)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    auto *EventState = GLiveEvents.Find(HeapState);
    if (EventState != nullptr)
    {
        // This is a live, valid event.
        return true;
    }
    else
    {
        // The EOS SDK has called an event after it was unregistered!
        auto *RecentEventState = GRecentEvents.Find(HeapState);
        if (RecentEventState != nullptr)
        {
            checkf(
                false,
                TEXT("The EOS SDK incorrectly invoked the callback for event '%s' after it was unregistered. This is a "
                     "bug in the EOS SDK and should be reported to Epic Games on https://eoshelp.epicgames.com/; "
                     "Redpoint Games is unable to fix this bug as it is in the EOS SDK itself. For more detailed "
                     "information, attach a debugger."),
                *RecentEventState->EventName);
        }
        else
        {
            checkf(
                false,
                TEXT("The EOS SDK incorrectly invoked the callback an event after it was unregistered. This is a "
                     "bug in the EOS SDK and should be reported to Epic Games on https://eoshelp.epicgames.com/; "
                     "Redpoint Games is unable to fix this bug as it is in the EOS SDK itself. For more detailed "
                     "information, attach a debugger. The name of the event is unavailable as the event has dropped "
                     "off the 'recent events' list (limited to 1000 event registrations)."));
        }

        return false;
    }
}

void FCallbackValidator::UntrackEvent(EOS_NotificationId NotificationId)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    auto *EventPtrPtr = GLiveEventPtrsByNotificationId.Find(NotificationId);
    if (EventPtrPtr == nullptr)
    {
        return;
    }
    auto HeapState = *EventPtrPtr;

    auto *EventState = GLiveEvents.Find(HeapState);
    checkf(EventState != nullptr, TEXT("EventPtr should return findable value in FCallbackValidator::UntrackEvent."));

    GRecentEvents.Add(HeapState, *EventState);
    GRecentEventPtrsByAge.Add(HeapState);
    GLiveEvents.Remove(HeapState);
    GLiveEventPtrsByNotificationId.Remove(NotificationId);

    if (GRecentEventPtrsByAge.Num() > 1000)
    {
        GRecentEvents.Remove(GRecentEventPtrsByAge[0]);
        GRecentEventPtrsByAge.RemoveAt(0);
    }
}

#if WITH_GAMEPLAY_DEBUGGER_CORE

class FLiveCallGameplayDebuggerCategory : public FGameplayDebuggerCategory
{
public:
    FLiveCallGameplayDebuggerCategory()
        : FGameplayDebuggerCategory()
    {
        REDPOINT_EOS_USE_LLM_TAG(API);

        bShowOnlyWithDebugActor = false;
        bAllowLocalDataCollection = true;
    }

    virtual void DrawData(APlayerController *OwnerPC, FGameplayDebuggerCanvasContext &CanvasContext) override
    {
        REDPOINT_EOS_USE_LLM_TAG(API);

        GLiveCallbackCount.ValueSort([](int A, int B) {
            return !(A < B);
        });

        for (const auto &KV : GLiveCallbackCount)
        {
            if (KV.Value > 0)
            {
                CanvasContext.Printf(TEXT("%d: %s"), KV.Value, *KV.Key);
            }
        }
    }
};

static TSharedRef<FLiveCallGameplayDebuggerCategory> GLiveCallGameplayDebuggerInstance =
    MakeShared<FLiveCallGameplayDebuggerCategory>();

void RegisterLiveCallGameplayDebuggerInstance();
void UnregisterLiveCallGameplayDebuggerInstance();

void RegisterLiveCallGameplayDebuggerInstance()
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    IGameplayDebugger &GameplayDebuggerModule = IGameplayDebugger::Get();
    GameplayDebuggerModule.RegisterCategory(
        "EOSLiveCalls",
        IGameplayDebugger::FOnGetCategory::CreateLambda([]() {
            return GLiveCallGameplayDebuggerInstance;
        }),
        EGameplayDebuggerCategoryState::EnabledInGame);
    GameplayDebuggerModule.NotifyCategoriesChanged();
}

void UnregisterLiveCallGameplayDebuggerInstance()
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    if (IGameplayDebugger::IsAvailable())
    {
        IGameplayDebugger &GameplayDebuggerModule = IGameplayDebugger::Get();
        GameplayDebuggerModule.UnregisterCategory("EOSLiveCalls");
    }
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()