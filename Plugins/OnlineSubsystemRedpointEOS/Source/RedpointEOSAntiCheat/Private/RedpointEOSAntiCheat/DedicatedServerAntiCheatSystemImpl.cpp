// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/DedicatedServerAntiCheatSystemImpl.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAPI/AntiCheatServer/BeginSession.h"
#include "RedpointEOSAPI/AntiCheatServer/EndSession.h"
#include "RedpointEOSAPI/AntiCheatServer/ReceiveMessageFromClient.h"
#include "RedpointEOSAPI/AntiCheatServer/RegisterClient.h"
#include "RedpointEOSAPI/AntiCheatServer/UnregisterClient.h"
#include "RedpointEOSAntiCheat/DedicatedServerAntiCheatSession.h"
#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSCore/Id/Id.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace REDPOINT_EOS_FILE_NS_ID(3183064733, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::API::AntiCheatServer;
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;

void FDedicatedServerAntiCheatSystemImpl::EnsureSessionIsCurrent(FAntiCheatSession &Session, const TCHAR *FunctionName)
    const
{
    bool bIsCurrentSession = Session.GetInstanceGuid() == this->CurrentSession->GetInstanceGuid();
    checkf(
        bIsCurrentSession,
        TEXT("Anti-Cheat requested session %s in %s must match current session %s!"),
        *Session.GetInstanceGuid().ToString(),
        FunctionName,
        *this->CurrentSession->GetInstanceGuid().ToString());
    if (!bIsCurrentSession)
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Fatal,
            TEXT("Anti-Cheat requested session %s in %s must match current session %s!"),
            *Session.GetInstanceGuid().ToString(),
            FunctionName,
            *this->CurrentSession->GetInstanceGuid().ToString());
    }
}

FDedicatedServerAntiCheatSystemImpl::FDedicatedServerAntiCheatSystemImpl(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
{
}

void FDedicatedServerAntiCheatSystemImpl::RegisterEvents()
{
    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT("Dedicated server Anti-Cheat: %p: Init"), this);

    this->NotifyMessageToClient = FNotifyMessageToClient::Register(
        this->PlatformHandle,
        FNotifyMessageToClient::Options{},
        FNotifyMessageToClient::EventDelegate::CreateSP(
            this,
            &FDedicatedServerAntiCheatSystemImpl::OnNotifyMessageToClient));
    this->NotifyClientActionRequired = FNotifyClientActionRequired::Register(
        this->PlatformHandle,
        FNotifyClientActionRequired::Options{},
        FNotifyClientActionRequired::EventDelegate::CreateSP(
            this,
            &FDedicatedServerAntiCheatSystemImpl::OnNotifyClientActionRequired));
    this->NotifyClientAuthStatusChanged = FNotifyClientAuthStatusChanged::Register(
        this->PlatformHandle,
        FNotifyClientAuthStatusChanged::Options{},
        FNotifyClientAuthStatusChanged::EventDelegate::CreateSP(
            this,
            &FDedicatedServerAntiCheatSystemImpl::OnNotifyClientAuthStatusChanged));
}

void FDedicatedServerAntiCheatSystemImpl::OnNotifyMessageToClient(const FNotifyMessageToClient::Result &Result)
{
#define FUNCTION_CONTEXT_MACRO                                                                                         \
    "Dedicated server Anti-Cheat: %p: NotifyMessageToClient(ClientHandle: %d, MessageData: ..., "                      \
    "MessageDataSizeInBytes: %u)"
#define FUNCTION_CONTEXT_ARGS this, Result.ClientHandle, Result.MessageDataSizeInBytes
    if (this->CurrentSession.IsValid())
    {
        auto PlayerId = this->CurrentSession->GetPlayer(Result.ClientHandle);
        if (PlayerId.IsSet())
        {
            this->OnSendNetworkMessage().ExecuteIfBound(
                this->CurrentSession.ToSharedRef(),
                GetDedicatedServerAccountId(),
                PlayerId.GetValue(),
                reinterpret_cast<const uint8 *>(Result.MessageData),
                Result.MessageDataSizeInBytes);

            if (!this->OnSendNetworkMessage().IsBound())
            {
                UE_LOG(
                    LogRedpointEOSAntiCheat,
                    Warning,
                    TEXT(FUNCTION_CONTEXT_MACRO
                         ": Propagated event OnSendNetworkMessage() handler, but no handlers were bound."),
                    FUNCTION_CONTEXT_ARGS);
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSAntiCheat,
                    VeryVerbose,
                    TEXT(FUNCTION_CONTEXT_MACRO ": Propagated event to OnSendNetworkMessage() handler."),
                    FUNCTION_CONTEXT_ARGS);
            }
        }
        else
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Error,
                TEXT(FUNCTION_CONTEXT_MACRO ": Ignoring call as it refers to an invalid player ID."),
                FUNCTION_CONTEXT_ARGS);
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Error,
            TEXT(FUNCTION_CONTEXT_MACRO ": Ignoring call as there is no current session."),
            FUNCTION_CONTEXT_ARGS);
    }
#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

void FDedicatedServerAntiCheatSystemImpl::OnNotifyClientActionRequired(
    const FNotifyClientActionRequired::Result &Result)
{
#define FUNCTION_CONTEXT_MACRO                                                                                         \
    "Dedicated server Anti-Cheat: %p: NotifyClientActionRequired(ClientHandle: %d, ClientAction: %u, "                 \
    "ActionReasonCode: %u, ActionReasonDetailsString: %s)"
#define FUNCTION_CONTEXT_ARGS                                                                                          \
    this, Result.ClientHandle, Result.ClientAction, Result.ActionReasonCode, *Result.ActionReasonDetailsString
    if (this->CurrentSession.IsValid())
    {
        this->OnPlayerActionRequired().Broadcast(
            this->CurrentSession->GetPlayer(Result.ClientHandle).GetValue(),
            Result.ClientAction,
            Result.ActionReasonCode,
            Result.ActionReasonDetailsString);

        if (!this->OnPlayerActionRequired().IsBound())
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Warning,
                TEXT(FUNCTION_CONTEXT_MACRO
                     ": Propagated event OnPlayerActionRequired() handler, but no handlers were bound."),
                FUNCTION_CONTEXT_ARGS);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Verbose,
                TEXT(FUNCTION_CONTEXT_MACRO ": Propagated event to OnPlayerActionRequired() handler."),
                FUNCTION_CONTEXT_ARGS);
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Error,
            TEXT(FUNCTION_CONTEXT_MACRO ": Ignoring call as there is no current session."),
            FUNCTION_CONTEXT_ARGS);
    }
#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

void FDedicatedServerAntiCheatSystemImpl::OnNotifyClientAuthStatusChanged(
    const FNotifyClientAuthStatusChanged::Result &Result)
{
#define FUNCTION_CONTEXT_MACRO                                                                                         \
    "Dedicated server Anti-Cheat: %p: NotifyClientAuthStatusChanged(ClientHandle: %d, ClientAuthStatus: %u)"
#define FUNCTION_CONTEXT_ARGS this, Result.ClientHandle, Result.ClientAuthStatus
    if (this->CurrentSession.IsValid())
    {
        this->OnPlayerAuthStatusChanged().Broadcast(
            *this->CurrentSession->GetPlayer(Result.ClientHandle),
            Result.ClientAuthStatus);

        if (!this->OnPlayerAuthStatusChanged().IsBound())
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Warning,
                TEXT(FUNCTION_CONTEXT_MACRO
                     ": Propagated event OnPlayerAuthStatusChanged() handler, but no handlers were bound."),
                FUNCTION_CONTEXT_ARGS);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Verbose,
                TEXT(FUNCTION_CONTEXT_MACRO ": Propagated event to OnPlayerAuthStatusChanged() handler."),
                FUNCTION_CONTEXT_ARGS);
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Error,
            TEXT(FUNCTION_CONTEXT_MACRO ": Ignoring call as there is no current session."),
            FUNCTION_CONTEXT_ARGS);
    }
#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

bool FDedicatedServerAntiCheatSystemImpl::ShouldGameExitOnStartup() const
{
    return false;
}

EAntiCheatImplementationType FDedicatedServerAntiCheatSystemImpl::GetImplementationType() const
{
    return EAntiCheatImplementationType::EasyAntiCheat;
}

bool FDedicatedServerAntiCheatSystemImpl::ShouldRemoteSkipPeerRegistration() const
{
    return false;
}

TSharedPtr<FAntiCheatSession> FDedicatedServerAntiCheatSystemImpl::CreateSession(
    bool bIsServer,
    UE::Online::FAccountId HostUserId,
    bool bIsDedicatedServerSession,
    TOptional<UE::Online::FAccountId> ListenServerUserId,
    FString ServerConnectionUrlOnClient)
{
#define FUNCTION_CONTEXT_MACRO                                                                                         \
    "Dedicated server Anti-Cheat: %p: CreateSession(bIsServer: %s, HostUserId: %s, bIsDedicatedServerSession: %s, "    \
    "ListenServerUserId: %s, ServerConnectionUrlOnClient: %s)"
#define FUNCTION_CONTEXT_ARGS                                                                                          \
    this, bIsServer ? TEXT("true") : TEXT("false"), *GetUserIdDebugString(HostUserId),                                 \
        bIsDedicatedServerSession ? TEXT("true") : TEXT("false"),                                                      \
        ListenServerUserId.IsSet() ? *GetUserIdDebugString(ListenServerUserId.GetValue()) : TEXT("(none)"),            \
        *ServerConnectionUrlOnClient

    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT(FUNCTION_CONTEXT_MACRO ": Called"), FUNCTION_CONTEXT_ARGS);

    checkf(bIsServer, TEXT("Dedicated server Anti-Cheat expects bIsServer=true"));

    if (this->CurrentSession.IsValid())
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": Current session is already valid, "
                                        "incrementing stack count to %d."),
            FUNCTION_CONTEXT_ARGS,
            this->CurrentSession->StackCount + 1);

        this->CurrentSession->StackCount++;
        return this->CurrentSession;
    }

    auto Result = FBeginSession::Execute(
        this->PlatformHandle,
        FBeginSession::Options{
            // Recommended value for EOS
            60,
            // @todo: Use the session ID for NAME_GameSession
            TEXT(""),
            // @todo: Implement gameplay data.
            false,
            static_cast<EOS_ProductUserId>(nullptr),
        });
    if (Result != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Error,
            TEXT(FUNCTION_CONTEXT_MACRO ": Unable to begin dedicated server session (got result %s)."),
            FUNCTION_CONTEXT_ARGS,
            ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
        return nullptr;
    }

    TSharedRef<FDedicatedServerAntiCheatSession> Session = MakeShared<FDedicatedServerAntiCheatSession>();
    Session->StackCount = 1;
    this->CurrentSession = Session;

    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT(FUNCTION_CONTEXT_MACRO ": Started new session; current session is now %s."),
        FUNCTION_CONTEXT_ARGS,
        *this->CurrentSession->ToString());

    return Session;

#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

bool FDedicatedServerAntiCheatSystemImpl::DestroySession(FAntiCheatSession &Session)
{
#define FUNCTION_CONTEXT_MACRO "Dedicated server Anti-Cheat: %p: DestroySession(Session: %s)"
#define FUNCTION_CONTEXT_ARGS this, *Session.ToString()

    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT(FUNCTION_CONTEXT_MACRO ": Called"), FUNCTION_CONTEXT_ARGS);

    if (!this->CurrentSession.IsValid())
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": There is no current session, ignoring DestroySession call."),
            FUNCTION_CONTEXT_ARGS);

        return true;
    }

    this->EnsureSessionIsCurrent(Session, ANSI_TO_TCHAR(__FUNCTION__));
    this->CurrentSession->StackCount--;

    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT(FUNCTION_CONTEXT_MACRO ": Decremented stack count of session; stack count is now at %d."),
        FUNCTION_CONTEXT_ARGS,
        this->CurrentSession->StackCount);

    if (this->CurrentSession->StackCount == 0)
    {
        auto Result = FEndSession::Execute(this->PlatformHandle, FEndSession::Options{});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Error,
                TEXT(FUNCTION_CONTEXT_MACRO ": Unable to end dedicated server session (got result %s)."),
                FUNCTION_CONTEXT_ARGS,
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return false;
        }

        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": Successfully ended dedicated server session."),
            FUNCTION_CONTEXT_ARGS);

        this->CurrentSession.Reset();
    }

    return true;

#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

bool FDedicatedServerAntiCheatSystemImpl::RegisterPlayer(
    FAntiCheatSession &Session,
    UE::Online::FAccountId UserId,
    EOS_EAntiCheatCommonClientType ClientType,
    EOS_EAntiCheatCommonClientPlatform ClientPlatform)
{
#define FUNCTION_CONTEXT_MACRO                                                                                         \
    "Dedicated server Anti-Cheat: %p: RegisterPlayer(Session: %s, UserId: %s, ClientType: %u, ClientPlatform: "        \
    "%u)"
#define FUNCTION_CONTEXT_ARGS this, *Session.ToString(), *GetUserIdDebugString(UserId), ClientType, ClientPlatform

    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT(FUNCTION_CONTEXT_MACRO ": Called"), FUNCTION_CONTEXT_ARGS);

    this->EnsureSessionIsCurrent(Session, ANSI_TO_TCHAR(__FUNCTION__));

    auto &ServerSession = (FDedicatedServerAntiCheatSession &)Session;

    bool bShouldRegister = false;
    EOS_AntiCheatCommon_ClientHandle ClientHandle = ServerSession.AddPlayer(UserId, bShouldRegister);

    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT(FUNCTION_CONTEXT_MACRO ": Should we register the specified player? %s"),
        FUNCTION_CONTEXT_ARGS,
        bShouldRegister ? TEXT("true") : TEXT("false"));

    if (bShouldRegister)
    {
        auto Result = FRegisterClient::Execute(
            this->PlatformHandle,
            FRegisterClient::Options{ClientHandle, ClientType, ClientPlatform, TEXT(""), GetProductUserId(UserId)});
        if (Result != EOS_EResult::EOS_Success)
        {
            ServerSession.RemovePlayer(UserId);
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Error,
                TEXT(FUNCTION_CONTEXT_MACRO ": Unable to register client (got result %s)."),
                FUNCTION_CONTEXT_ARGS,
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return false;
        }

        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": Successfully registered player with Anti-Cheat interface."),
            FUNCTION_CONTEXT_ARGS);
    }

    return true;

#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

bool FDedicatedServerAntiCheatSystemImpl::UnregisterPlayer(FAntiCheatSession &Session, UE::Online::FAccountId UserId)
{
#define FUNCTION_CONTEXT_MACRO "Dedicated server Anti-Cheat: %p: UnregisterPlayer(Session: %s, UserId: %s)"
#define FUNCTION_CONTEXT_ARGS this, *Session.ToString(), *GetUserIdDebugString(UserId)

    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT(FUNCTION_CONTEXT_MACRO ": Called"), FUNCTION_CONTEXT_ARGS);

    this->EnsureSessionIsCurrent(Session, ANSI_TO_TCHAR(__FUNCTION__));

    auto &ServerSession = (FDedicatedServerAntiCheatSession &)Session;

    if (ServerSession.ShouldDeregisterPlayerBeforeRemove(UserId))
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": Player tracking indicated we should try to deregister player with "
                                        "Anti-Cheat interface before removing them from tracking."),
            FUNCTION_CONTEXT_ARGS);

        auto Result = FUnregisterClient::Execute(
            this->PlatformHandle,
            FUnregisterClient::Options{ServerSession.GetHandle(UserId)});
        if (Result != EOS_EResult::EOS_Success)
        {
            UE_LOG(
                LogRedpointEOSAntiCheat,
                Error,
                TEXT("Anti-Cheat: Unable to unregister client (got result %s)."),
                ANSI_TO_TCHAR(EOS_EResult_ToString(Result)));
            return false;
        }

        UE_LOG(
            LogRedpointEOSAntiCheat,
            Verbose,
            TEXT(FUNCTION_CONTEXT_MACRO ": Successfully unregistered player from Anti-Cheat interface."),
            FUNCTION_CONTEXT_ARGS);
    }

    ServerSession.RemovePlayer(UserId);

    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT(FUNCTION_CONTEXT_MACRO ": Removed player from player tracking."),
        FUNCTION_CONTEXT_ARGS);

    return true;

#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

bool FDedicatedServerAntiCheatSystemImpl::ReceiveNetworkMessage(
    FAntiCheatSession &Session,
    UE::Online::FAccountId SourceUserId,
    UE::Online::FAccountId TargetUserId,
    const uint8 *Data,
    uint32_t Size)
{
#define FUNCTION_CONTEXT_MACRO                                                                                         \
    "Dedicated server Anti-Cheat: %p: ReceiveNetworkMessage(Session: %s, SourceUserId: %s, TargetUserId: %s, Data: "   \
    "..., Size: %u)"
#define FUNCTION_CONTEXT_ARGS                                                                                          \
    this, *Session.ToString(), *GetUserIdDebugString(SourceUserId), *GetUserIdDebugString(TargetUserId), Size

    auto ReceiveResult = FReceiveMessageFromClient::Execute(
        this->PlatformHandle,
        FReceiveMessageFromClient::Options{this->CurrentSession->GetHandle(SourceUserId), Size, Data});
    if (ReceiveResult != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSAntiCheat,
            Error,
            TEXT(FUNCTION_CONTEXT_MACRO ": Failed to receive message from client: %s"),
            FUNCTION_CONTEXT_ARGS,
            ANSI_TO_TCHAR(EOS_EResult_ToString(ReceiveResult)));
        return false;
    }

    UE_LOG(
        LogRedpointEOSAntiCheat,
        VeryVerbose,
        TEXT(FUNCTION_CONTEXT_MACRO ": Received network message."),
        FUNCTION_CONTEXT_ARGS);

    return true;

#undef FUNCTION_CONTEXT_MACRO
#undef FUNCTION_CONTEXT_ARGS
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()