// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationMeta/MetaSDK.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED
#include "Containers/Ticker.h"
#include "Misc/ConfigCacheIni.h"
#include "OVR_Platform.h"
#include "RedpointEOSPlatformIntegrationMeta/Logging.h"
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#if !REDPOINT_EOS_UE_5_5_OR_LATER
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#endif
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_META_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(1841280286, Redpoint::EOS::Platform::Integration::Meta)
{
using namespace ::Redpoint::EOS::Platform::Integration::Meta;

bool FMetaSDK::bDispatcherRunning = false;
TMap<ovrRequest, TTuple<FMetaSDK::FOnResponseOrTimeout, FDateTime>> FMetaSDK::AwaitingRequests;
FCriticalSection FMetaSDK::DispatcherStartLock;

void FMetaSDK::AwaitRequest(const ovrRequest &Request, const FOnResponseOrTimeout &OnResponseOrTimeout)
{
    if (!FMetaSDK::bDispatcherRunning)
    {
        FScopeLock Lock(&FMetaSDK::DispatcherStartLock);
        if (!FMetaSDK::bDispatcherRunning)
        {
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&FMetaSDK::TickDispatch));
            FMetaSDK::bDispatcherRunning = true;
        }
    }

    FMetaSDK::AwaitingRequests.Add(
        Request,
        TTuple<FOnResponseOrTimeout, FDateTime>(OnResponseOrTimeout, FDateTime::UtcNow()));
}

bool FMetaSDK::TickDispatch(float)
{
    // Process messages.
    ovrMessageHandle Message = nullptr;
    while ((Message = ovr_PopMessage()) != nullptr)
    {
        switch (ovr_Message_GetType(Message))
        {
        case ovrMessage_Notification_ApplicationLifecycle_LaunchIntentChanged: {
            if (!ovr_Message_IsError(Message))
            {
                auto LaunchDetails = ovr_ApplicationLifecycle_GetLaunchDetails();
                auto DeeplinkMessage = ovr_LaunchDetails_GetDeeplinkMessage(LaunchDetails);
                FMetaSDK::OnDeeplinkReceivedDelegate.Broadcast(ANSI_TO_TCHAR(DeeplinkMessage));
            }
            break;
        }
#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_JOIN_INTENT_RECEIVED
        case ovrMessage_Notification_GroupPresence_JoinIntentReceived: {
            if (!ovr_Message_IsError(Message))
            {
                auto JoinIntent = ovr_Message_GetGroupPresenceJoinIntent(Message);
                auto DeeplinkMessage = ovr_GroupPresenceJoinIntent_GetDeeplinkMessage(JoinIntent);
                FMetaSDK::OnDeeplinkReceivedDelegate.Broadcast(ANSI_TO_TCHAR(DeeplinkMessage));
            }
            break;
        }
#endif
        default:
            break;
        }

        auto RequestID = ovr_Message_GetRequestID(Message);
        auto *AwaitedRequest = FMetaSDK::AwaitingRequests.Find(RequestID);
        if (AwaitedRequest != nullptr)
        {
            AwaitedRequest->Get<0>().ExecuteIfBound(Message);
            FMetaSDK::AwaitingRequests.Remove(RequestID);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSPlatformIntegrationMeta,
                Warning,
                TEXT("Received unexpected message with type '0x%08x' from Meta OVR platform library during tick. It "
                     "will be discarded."),
                ovr_Message_GetType(Message));
        }
        ovr_FreeMessage(Message);
    }

    // Any awaited requests that are older than 60 seconds have timed out.
    TArray<ovrRequest> ExpiredRequests;
    FDateTime OldestPermittedStartTime = FDateTime::UtcNow() - FTimespan::FromSeconds(60.0);
    for (const auto &KV : FMetaSDK::AwaitingRequests)
    {
        if (KV.Value.Get<1>() < OldestPermittedStartTime)
        {
            KV.Value.Get<0>().ExecuteIfBound(TOptional<ovrMessageHandle>());
            ExpiredRequests.Add(KV.Key);
        }
    }
    for (const auto &ExpiredRequest : ExpiredRequests)
    {
        FMetaSDK::AwaitingRequests.Remove(ExpiredRequest);
    }

    return true;
}

bool FMetaSDK::bIsInitAttempted = false;
bool FMetaSDK::bIsInitSuccessful = false;
FCriticalSection FMetaSDK::InitLock;

#if PLATFORM_WINDOWS || REDPOINT_EOS_UE_5_5_OR_LATER
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif
// Meta SDK design is silly and provides a ovrPlatformInitializeResult_ToString function, but you can't actually call it
// if the SDK fails to load because the function implementation won't have been loaded into memory. Implement our own
// "ToString" function for this enumeration here.
static const TCHAR *ovrPlatformInitializeResultToString(ovrPlatformInitializeResult_ Result)
{
#define REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(Value)                                                                 \
    case Value:                                                                                                        \
        return TEXT(#Value);
    switch (Result)
    {
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_Success)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_Uninitialized)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_PreLoaded)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_FileInvalid)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_SignatureInvalid)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_UnableToVerify)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_VersionMismatch)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_Unknown)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_InvalidCredentials)
        REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE(ovrPlatformInitialize_NotEntitled)
    default:
        return TEXT("(unknown)");
    }
#undef REDPOINT_EOS_META_SDK_ENUM_STRING_VALUE
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

bool FMetaSDK::InitIfNeeded()
{
    if (FMetaSDK::bIsInitAttempted)
    {
        return FMetaSDK::bIsInitSuccessful;
    }

    FScopeLock Lock(&FMetaSDK::InitLock);
    if (FMetaSDK::bIsInitAttempted)
    {
        return FMetaSDK::bIsInitSuccessful;
    }

    auto MetaAppIdStr = GConfig->GetStr(TEXT("OnlineSubsystemOculus"), TEXT("OculusAppId"), GEngineIni);
#if PLATFORM_ANDROID
    if (MetaAppIdStr.IsEmpty())
    {
        MetaAppIdStr = GConfig->GetStr(TEXT("OnlineSubsystemOculus"), TEXT("MobileAppId"), GEngineIni);
    }
#endif
#if PLATFORM_WINDOWS
    if (MetaAppIdStr.IsEmpty())
    {
        MetaAppIdStr = GConfig->GetStr(TEXT("OnlineSubsystemOculus"), TEXT("RiftAppId"), GEngineIni);
    }
#endif

    if (MetaAppIdStr.IsEmpty())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Verbose,
            TEXT("This application is not configured for Meta Quest (missing app ID in configuration)."));
        FMetaSDK::bIsInitAttempted = true;
        FMetaSDK::bIsInitSuccessful = false;
        return FMetaSDK::bIsInitSuccessful;
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationMeta,
        Verbose,
        TEXT("Initializing Oculus Platform SDK using app ID '%s'..."),
        *MetaAppIdStr);

#if PLATFORM_WINDOWS
    auto PlatformInitializeResult = ovr_PlatformInitializeWindows(TCHAR_TO_ANSI(*MetaAppIdStr));
    FMetaSDK::bIsInitSuccessful = (PlatformInitializeResult == ovrPlatformInitialize_Success);
    if (!FMetaSDK::bIsInitSuccessful)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Warning,
            TEXT("Oculus Platform SDK failed to initialize with result code '%s'."),
            ovrPlatformInitializeResultToString(PlatformInitializeResult));
    }
#else
#if REDPOINT_EOS_UE_5_5_OR_LATER
    // On Unreal Engine 5.5 we do the initialization ourselves.
    JNIEnv *Env = FAndroidApplication::GetJavaEnv();
    if (Env == nullptr)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Error,
            TEXT("No JNI environment available at time of Oculus Platform SDK initialization."));
        FMetaSDK::bIsInitSuccessful = false;
        return FMetaSDK::bIsInitSuccessful;
    }
    auto PlatformInitializeResult =
        ovr_PlatformInitializeAndroid(TCHAR_TO_ANSI(*MetaAppIdStr), FAndroidApplication::GetGameActivityThis(), Env);
    FMetaSDK::bIsInitSuccessful = (PlatformInitializeResult == ovrPlatformInitialize_Success);
    if (!FMetaSDK::bIsInitSuccessful)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Warning,
            TEXT("Oculus Platform SDK failed to initialize with result code '%s'."),
            ovrPlatformInitializeResultToString(PlatformInitializeResult));
    }
#else
    // On earlier versions, we try to get the Oculus online subsystem. If it returns a valid pointer, then Oculus got
    // initialized.
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    FMetaSDK::bIsInitSuccessful = IOnlineSubsystem::Get(OCULUS_SUBSYSTEM) != nullptr;
    PRAGMA_ENABLE_DEPRECATION_WARNINGS
    if (!FMetaSDK::bIsInitSuccessful)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Warning,
            TEXT("Oculus Platform SDK failed to initialize because the Oculus Online Subsystem is not available. "
                 "Please ensure the Oculus online subsystem is enabled. If you continue to experience issues, please "
                 "upgrade to Unreal Engine 5.5 for Meta Quest support, as we no longer support Meta Quest on earlier "
                 "engine versions."));
    }
#endif
#endif
    FMetaSDK::bIsInitAttempted = true;
    if (FMetaSDK::bIsInitSuccessful)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationMeta,
            Verbose,
            TEXT("Oculus Platform SDK was successfully initialized."));
    }

    return FMetaSDK::bIsInitSuccessful;
}

#if PLATFORM_WINDOWS

bool FMetaSDK::IsMetaQuestLinkOnDesktop()
{
    // If we initialize successfully on Windows, then we're running for Meta Quest Link.
    return InitIfNeeded();
}

#endif

FMetaSDK::FOnDeeplinkReceived FMetaSDK::OnDeeplinkReceivedDelegate;

FMetaSDK::FOnDeeplinkReceived &FMetaSDK::OnDeeplinkReceived()
{
    return FMetaSDK::OnDeeplinkReceivedDelegate;
}

void FMetaSDK::GetLoggedInUser(const FOnGetLoggedInUserComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<ovrUserHandle>());
        return;
    }

    auto Request = ovr_User_GetLoggedInUser();
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<ovrUserHandle>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<ovrUserHandle>());
                return;
            }

            auto UserHandle = ovr_Message_GetUser(Message.GetValue());
            OnComplete.ExecuteIfBound(Errors::Success(), UserHandle);
        }));
}

void FMetaSDK::GetUser(ovrID UserId, const FOnGetUserComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<ovrUserHandle>());
        return;
    }

    auto Request = ovr_User_Get(UserId);
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<ovrUserHandle>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<ovrUserHandle>());
                return;
            }

            auto UserHandle = ovr_Message_GetUser(Message.GetValue());
            OnComplete.ExecuteIfBound(Errors::Success(), UserHandle);
        }));
}

void FMetaSDK::GetIsViewerEntitled(const FOnGetIsViewerEntitledComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")));
        return;
    }

    auto Request = ovr_Entitlement_GetIsViewerEntitled();
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(Errors::TimedOut(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Timed out while waiting for the Meta SDK to respond.")));
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))));
                return;
            }

            // @note: This call has no payload; we just need to make sure it's successful.
            OnComplete.ExecuteIfBound(Errors::Success());
        }));
}

void FMetaSDK::GetUserProof(const FOnGetUserProofComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<FString>());
        return;
    }

    auto Request = ovr_User_GetUserProof();
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<FString>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<FString>());
                return;
            }

            auto ProofHandle = ovr_Message_GetUserProof(Message.GetValue());
            OnComplete.ExecuteIfBound(Errors::Success(), FString(ANSI_TO_TCHAR(ovr_UserProof_GetNonce(ProofHandle))));
        }));
}

void FMetaSDK::GetLoggedInUserFriends(const FOnGetLoggedInUserFriendsComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<TArray<ovrUserHandle>>());
        return;
    }

    auto Request = ovr_User_GetLoggedInUserFriends();
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<TArray<ovrUserHandle>>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<TArray<ovrUserHandle>>());
                return;
            }

            auto UserArrayHandle = ovr_Message_GetUserArray(Message.GetValue());
            FMetaSDK::GetFullUserArray(UserArrayHandle, TArray<ovrUserHandle>(), OnComplete);
        }));
}

void FMetaSDK::GetFullUserArray(
    ovrUserArrayHandle UserArray,
    TArray<ovrUserHandle> Accumulated,
    const FOnGetFullUserArrayComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<TArray<ovrUserHandle>>());
        return;
    }

    size_t Size = ovr_UserArray_GetSize(UserArray);
    for (size_t i = 0; i < Size; i++)
    {
        Accumulated.Add(ovr_UserArray_GetElement(UserArray, i));
    }

    if (!ovr_UserArray_HasNextPage(UserArray))
    {
        OnComplete.ExecuteIfBound(Errors::Success(), Accumulated);
        return;
    }

    auto Request = ovr_User_GetNextUserArrayPage(UserArray);
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([Accumulated, OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<TArray<ovrUserHandle>>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<TArray<ovrUserHandle>>());
                return;
            }

            auto UserArrayHandle = ovr_Message_GetUserArray(Message.GetValue());
            FMetaSDK::GetFullUserArray(UserArrayHandle, Accumulated, OnComplete);
        }));
}

#if REDPOINT_EOS_META_HAS_USER_GET_BLOCKED_USERS

void FMetaSDK::GetBlockedUsers(const FOnGetBlockedUsersComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<TArray<ovrBlockedUserHandle>>());
        return;
    }

    auto Request = ovr_User_GetBlockedUsers();
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<TArray<ovrBlockedUserHandle>>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<TArray<ovrBlockedUserHandle>>());
                return;
            }

            auto BlockedUserArrayHandle = ovr_Message_GetBlockedUserArray(Message.GetValue());
            FMetaSDK::GetFullBlockedUserArray(BlockedUserArrayHandle, TArray<ovrBlockedUserHandle>(), OnComplete);
        }));
}

void FMetaSDK::GetFullBlockedUserArray(
    ovrBlockedUserArrayHandle UserArray,
    TArray<ovrBlockedUserHandle> Accumulated,
    const FOnGetFullBlockedUserArrayComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")),
            TOptional<TArray<ovrBlockedUserHandle>>());
        return;
    }

    size_t Size = ovr_BlockedUserArray_GetSize(UserArray);
    for (size_t i = 0; i < Size; i++)
    {
        Accumulated.Add(ovr_BlockedUserArray_GetElement(UserArray, i));
    }

    if (!ovr_BlockedUserArray_HasNextPage(UserArray))
    {
        OnComplete.ExecuteIfBound(Errors::Success(), Accumulated);
        return;
    }

    auto Request = ovr_User_GetNextBlockedUserArrayPage(UserArray);
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([Accumulated, OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(
                    Errors::TimedOut(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Timed out while waiting for the Meta SDK to respond.")),
                    TOptional<TArray<ovrBlockedUserHandle>>());
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(
                    Errors::UnexpectedError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))),
                    TOptional<TArray<ovrBlockedUserHandle>>());
                return;
            }

            auto BlockedUserArrayHandle = ovr_Message_GetBlockedUserArray(Message.GetValue());
            FMetaSDK::GetFullBlockedUserArray(BlockedUserArrayHandle, Accumulated, OnComplete);
        }));
}

#endif

TOptional<FString> FMetaSDK::UserIdToString(ovrID UserId)
{
    if (!InitIfNeeded())
    {
        return TOptional<FString>();
    }

    const size_t BufferSize = 64;
    char *Buffer = static_cast<char *>(FMemory::MallocZeroed(sizeof(char) * BufferSize));
    auto Success = ovrID_ToString(Buffer, BufferSize, UserId);
    TOptional<FString> Result;
    if (Success)
    {
#if REDPOINT_EOS_UE_5_4_OR_LATER
        Result = FString::ConstructFromPtrSize(
            ANSI_TO_TCHAR(Buffer),
            static_cast<int32>(FMath::Min(BufferSize, strnlen(Buffer, BufferSize))));
#else
        Result =
            FString(static_cast<int32>(FMath::Min(BufferSize, strnlen(Buffer, BufferSize))), ANSI_TO_TCHAR(Buffer));
#endif
    }
    FMemory::Free(static_cast<void *>(Buffer));
    return Result;
}

TOptional<ovrID> FMetaSDK::UserIdFromString(const FString &UserId)
{
    if (!InitIfNeeded())
    {
        return TOptional<ovrID>();
    }

    ovrID Result;
    if (ovrID_FromString(&Result, TCHAR_TO_ANSI(*UserId)))
    {
        return Result;
    }

    return TOptional<ovrID>();
}

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SET

void FMetaSDK::SetGroupPresence(ovrGroupPresenceOptionsHandle Options, const FOnSetGroupPresenceComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")));
        return;
    }

    auto Request = ovr_GroupPresence_Set(Options);
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([Options, OnComplete](const TOptional<ovrMessageHandle> &Message) {
            ovr_GroupPresenceOptions_Destroy(Options);

            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(Errors::TimedOut(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Timed out while waiting for the Meta SDK to respond.")));
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))));
                return;
            }

            OnComplete.ExecuteIfBound(Errors::Success());
        }));
}

#endif

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_SEND_INVITES

void FMetaSDK::SendGroupPresenceInvite(ovrID TargetUserId, const FOnSendGroupPresenceInviteComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")));
        return;
    }

    ovrID TargetUserIds[1] = {TargetUserId};

    auto Request = ovr_GroupPresence_SendInvites(TargetUserIds, 1);
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([OnComplete](const TOptional<ovrMessageHandle> &Message) {
            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(Errors::TimedOut(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Timed out while waiting for the Meta SDK to respond.")));
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))));
                return;
            }

            OnComplete.ExecuteIfBound(Errors::Success());
        }));
}

#endif

#if REDPOINT_EOS_META_HAS_GROUP_PRESENCE_LAUNCH_INVITE_PANEL

void FMetaSDK::LaunchGroupPresenceInvitePanel(const FOnLaunchGroupPresenceInvitePanelComplete &OnComplete)
{
    if (!InitIfNeeded())
    {
        OnComplete.ExecuteIfBound(
            Errors::Disabled(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The Meta SDK is not currently available.")));
        return;
    }

    ovrInviteOptionsHandle InviteOptions = ovr_InviteOptions_Create();

    auto Request = ovr_GroupPresence_LaunchInvitePanel(InviteOptions);
    FMetaSDK::AwaitRequest(
        Request,
        FOnResponseOrTimeout::CreateLambda([InviteOptions, OnComplete](const TOptional<ovrMessageHandle> &Message) {
            ovr_InviteOptions_Destroy(InviteOptions);

            if (!Message.IsSet())
            {
                OnComplete.ExecuteIfBound(Errors::TimedOut(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Timed out while waiting for the Meta SDK to respond.")));
                return;
            }

            if (ovr_Message_IsError(Message.GetValue()))
            {
                auto ErrorHandle = ovr_Message_GetError(Message.GetValue());
                OnComplete.ExecuteIfBound(Errors::UnexpectedError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("%s"), ANSI_TO_TCHAR(ovr_Error_GetMessage(ErrorHandle)))));
                return;
            }

            OnComplete.ExecuteIfBound(Errors::Success());
        }));
}

#endif

}

#endif

REDPOINT_EOS_CODE_GUARD_END()