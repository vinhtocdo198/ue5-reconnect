// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/EditorAntiCheatSystemImpl.h"

#include "RedpointEOSAntiCheat/Logging.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2381734488, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;

bool FEditorAntiCheatSystemImpl::ShouldGameExitOnStartup() const
{
    return false;
}

EAntiCheatImplementationType FEditorAntiCheatSystemImpl::GetImplementationType() const
{
    return EAntiCheatImplementationType::Editor;
}

bool FEditorAntiCheatSystemImpl::ShouldRemoteSkipPeerRegistration() const
{
    return false;
}

TSharedPtr<FAntiCheatSession> FEditorAntiCheatSystemImpl::CreateSession(
    bool bIsServer,
    UE::Online::FAccountId HostUserId,
    bool bIsDedicatedServerSession,
    TOptional<UE::Online::FAccountId> ListenServerUserId,
    FString ServerConnectionUrlOnClient)
{
    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT("Anti-Cheat: Creating editor AC session."));

    auto Session = MakeShared<FEditorAntiCheatSession>(HostUserId);
    Session->bIsServer = bIsServer;
    Session->bIsDedicatedServerSession = bIsDedicatedServerSession;
    this->CurrentSessions.Add(Session);
    return Session;
}

bool FEditorAntiCheatSystemImpl::DestroySession(FAntiCheatSession &Session)
{
    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT("Anti-Cheat: Destroying editor AC session."));
    this->CurrentSessions.Remove(StaticCastSharedRef<FEditorAntiCheatSession>(Session.AsShared()));

    return true;
}

bool FEditorAntiCheatSystemImpl::RegisterPlayer(
    FAntiCheatSession &Session,
    UE::Online::FAccountId UserId,
    EOS_EAntiCheatCommonClientType ClientType,
    EOS_EAntiCheatCommonClientPlatform ClientPlatform)
{
    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT("Anti-Cheat: Registering player with editor AC session."));

    FEditorAntiCheatSession &EditorSession = (FEditorAntiCheatSession &)Session;

    TSharedRef<FEditorAntiCheatSession::FRegisteredPlayer> RegisteredPlayer =
        MakeShared<FEditorAntiCheatSession::FRegisteredPlayer>();
    RegisteredPlayer->ClientType = ClientType;
    RegisteredPlayer->ClientPlatform = ClientPlatform;
    EditorSession.RegisteredPlayers.Add(UserId, RegisteredPlayer);

    // Send a message to the target.
    uint8 Bytes[40];
    uint32 Len = static_cast<uint32>(StringToBytes(TEXT("CLIENT-OK"), Bytes, 40));
    checkf(
        this->OnSendNetworkMessage().IsBound(),
        TEXT("Expected OnSendNetworkMessage to be bound for editor Anti-Cheat."));
    this->OnSendNetworkMessage().Execute(Session.AsShared(), GetDedicatedServerAccountId(), UserId, &Bytes[0], Len);

    // Tell the server that the client is authenticated.
    this->OnPlayerAuthStatusChanged().Broadcast(
        UserId,
        EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete);

    return true;
}

bool FEditorAntiCheatSystemImpl::UnregisterPlayer(FAntiCheatSession &Session, UE::Online::FAccountId UserId)
{
    UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT("Anti-Cheat: Unregistering player with editor AC session."));

    FEditorAntiCheatSession &EditorSession = (FEditorAntiCheatSession &)Session;

    EditorSession.RegisteredPlayers.Remove(UserId);

    return true;
}

bool FEditorAntiCheatSystemImpl::ReceiveNetworkMessage(
    FAntiCheatSession &Session,
    UE::Online::FAccountId SourceUserId,
    UE::Online::FAccountId TargetUserId,
    const uint8 *Data,
    uint32_t Size)
{
    FString EditorMessage = BytesToString(Data, static_cast<int32>(Size));

    UE_LOG(
        LogRedpointEOSAntiCheat,
        Verbose,
        TEXT("Anti-Cheat: Receiving networking message for player with editor AC session: '%s'"),
        *EditorMessage);

    if (EditorMessage == TEXT("CLIENT-OK"))
    {
        // Respond to server.
        uint8 Bytes[40];
        uint32 Len = static_cast<uint32>(StringToBytes(TEXT("SERVER-OK"), Bytes, 40));
        checkf(
            this->OnSendNetworkMessage().IsBound(),
            TEXT("Expected OnSendNetworkMessage to be bound for editor Anti-Cheat."));
        this->OnSendNetworkMessage().Execute(Session.AsShared(), TargetUserId, SourceUserId, &Bytes[0], Len);
    }
    else if (EditorMessage == TEXT("SERVER-OK"))
    {
        // This client is "verified".
        this->OnPlayerAuthStatusChanged().Broadcast(
            SourceUserId,
            EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete);
    }

    return true;
}

void FEditorAntiCheatSystemImpl::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &Commands)
{
#if UE_ALLOW_EXEC_DEV
    FAutoCompleteCommand SimulateKick;
    SimulateKick.Command = TEXT("SimulateKick");
    SimulateKick.Desc = TEXT("Simulate an Anti-Cheat kick for the first user in the Anti-Cheat session.");
    Commands.Add(SimulateKick);
#endif
}

bool FEditorAntiCheatSystemImpl::Exec_Dev(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    if (FParse::Command(&Cmd, TEXT("SimulateKick"), false))
    {
        UE_LOG(LogRedpointEOSAntiCheat, Verbose, TEXT("Simulating Anti-Cheat kick for first user in session..."));
        for (const auto &Session : this->CurrentSessions)
        {
            if (Session->RegisteredPlayers.Num() > 0)
            {
                TArray<UE::Online::FAccountId> Keys;
                Session->RegisteredPlayers.GetKeys(Keys);
                this->OnPlayerActionRequired().Broadcast(
                    Keys[0],
                    EOS_EAntiCheatCommonClientAction::EOS_ACCCA_RemovePlayer,
                    EOS_EAntiCheatCommonClientActionReason::EOS_ACCCAR_InternalError,
                    TEXT("Simulated Anti-Cheat kick!"));
            }
        }
        return true;
    }

    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()