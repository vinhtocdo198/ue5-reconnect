// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceMessageGlobalConnection.h"

#include "Containers/Ticker.h"
#include "Dom/JsonObject.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSPresence/Logging.h"
#include "RedpointEOSPresence/PresenceMessageInbound.h"
#include "RedpointEOSPresence/PresenceMessageInstancedConnection.h"
#include "RedpointEOSPresence/PresenceMessageOutbound.h"
#include "RedpointEOSPresence/PresenceSystemLocalUserState.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(402271213, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Core::Id;

FPresenceMessageGlobalConnection::FPresenceMessageGlobalConnection(
    const TSharedRef<IRoomSystem> &InRoomSystem,
    const FIdentityUserId &InLocalUserId,
    const FIdentityUserId &InTargetUserId,
    const FRoomRef &InRoom,
    const FName &InInitialInterestedSystem)
    : RoomSystem(InRoomSystem)
    , LocalUserId(InLocalUserId)
    , TargetUserId(InTargetUserId)
    , Room(InRoom)
    , InstancedConnections()
    , ClosedConnections()
    , NextOutboundMessageId(8000)
    , NextConnectionId(20)
    , LastFlushedHash()
    , bOpen(false)
    , OnRemoteConnectionReceivedDelegate()
    , OnRoomLeftDelegateHandle()
    , OnRoomMemberUpdatedDelegateHandle()
    , WorkQueue(MakeShared<FExclusiveAsyncOperationQueue>())
    , InitialInterestedSystem(InInitialInterestedSystem)
{
}

void FPresenceMessageGlobalConnection::RegisterEvents()
{
    // Make sure we're still in the room, otherwise we're immediately closed.
    auto CurrentRooms = RoomSystem->GetCurrentRooms(
        FGetCurrentRoomsRequest(ERoomScope::User, this->LocalUserId, CrossPlatformPresenceNamespace));
    bool bFoundRoom = false;
    for (const auto &CurrentRoom : CurrentRooms)
    {
        if (CurrentRoom == this->Room)
        {
            bFoundRoom = true;
        }
    }
    this->bOpen = bFoundRoom;
    if (!this->bOpen)
    {
        return;
    }

    // Register events.
    this->OnRoomLeftDelegateHandle = this->RoomSystem->OnRoomLeft(CrossPlatformPresenceNamespace)
                                         .AddSP(this, &FPresenceMessageGlobalConnection::OnRoomLeft);
    this->OnRoomMemberUpdatedDelegateHandle =
        this->Room->OnRoomMemberUpdated().AddSP(this, &FPresenceMessageGlobalConnection::OnRoomMemberUpdated);

    // If our desired initial connection isn't NAME_None (this gets used when we're receiving a connection but don't
    // know any connection yet), then create the first instanced connection.
    if (!this->InitialInterestedSystem.IsNone())
    {
        // Add the initial instanced connection.
        auto NewInstancedConnection = MakeShared<FPresenceMessageInstancedConnection>(
            this->AsShared(),
            this->InitialInterestedSystem,
            this->NextConnectionId++);
        this->InstancedConnections.Add(NewInstancedConnection->ConnectionId, NewInstancedConnection);

        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Presence message global connection from %s to %s configured initial connection for system %s with "
                 "connection ID %d."),
            *GetUserIdString(this->LocalUserId),
            *GetUserIdString(this->TargetUserId),
            *this->InitialInterestedSystem.ToString(),
            NewInstancedConnection->ConnectionId);

        // Do an initial flush so the remote immediately knows about this new connection (even with no messages sent
        // yet).
        this->ScheduleWork();
    }
}

FPresenceMessageGlobalConnection::~FPresenceMessageGlobalConnection()
{
    // Close connection in background.
    this->CloseInternal(true);
}

void FPresenceMessageGlobalConnection::CloseDueToRemoteUserLeaving()
{
    checkf(
        this->Room->GetRoomOwner()->GetUserId() == this->LocalUserId,
        TEXT("CloseDueToRemoteUserLeaving must only be called for hosts of presence rooms!"));
    this->CloseInternal(false);
}

void FPresenceMessageGlobalConnection::OnRoomLeft(
    const FRoomEventContext &EventContext,
    const FRoomRef &EventRoom,
    ERoomLeaveReason Reason)
{
    // We've left the room before the connection was expected to be closed.
    if (this->Room->GetRoomId()->IsEqual(EventRoom->GetRoomId()))
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Warning,
            TEXT("Presence message connection closed by remote host or EOS SDK unexpectedly!"));
        this->CloseInternal(false);
    }
}

void FPresenceMessageGlobalConnection::OnRoomMemberUpdated(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &,
    FRoomUserId TargetUser,
    const FRoomMemberUpdateInfo &RoomMemberUpdateInfo)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (TargetUser != this->TargetUserId)
    {
        // An update on a member that we're not interested in.
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Ignoring OnRoomMemberUpdated event fired for local user %s and target user %s, for global presence "
                 "connection that is monitoring local user %s and target user %s."),
            *GetUserIdString(EventContext.LocalUserId),
            *GetUserIdString(TargetUser),
            *GetUserIdString(this->LocalUserId),
            *GetUserIdString(this->TargetUserId));
        return;
    }

    const auto &Members = this->Room->GetMembers();
    FRoomMemberPtr Member;
    for (const auto &M : Members)
    {
        if (M->GetUserId() == TargetUser)
        {
            Member = M;
            break;
        }
    }
    checkf(Member.IsValid(), TEXT("Expected room to contain member after receiving OnRoomMemberUpdated event!"));

    auto IncomingSystemNameAttributePrefix = FString::Printf(TEXT("s.%s."), *GetUserIdString(this->LocalUserId));
    auto IncomingUnackedMessagesAttributePrefix = FString::Printf(TEXT("u.%s."), *GetUserIdString(this->LocalUserId));
    auto IncomingAckedMessageIdAttributePrefix = FString::Printf(TEXT("a.%s."), *GetUserIdString(this->LocalUserId));
    auto IncomingClosedAttributePrefix = FString::Printf(TEXT("c.%s."), *GetUserIdString(this->LocalUserId));

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Received OnRoomMemberUpdated event on local user %s for target user %s."),
        *GetUserIdString(this->LocalUserId),
        *GetUserIdString(this->TargetUserId));

    // Find all of the system name attributes, and derive all of our connections and related attributes from that.
    for (const auto &KV : Member->GetAttributes())
    {
        if (!KV.Key.StartsWith(IncomingSystemNameAttributePrefix) || KV.Value.Type() != FRoomAttribute::EType::String)
        {
            continue;
        }

        auto ConnectionIdString = KV.Key.Mid(IncomingSystemNameAttributePrefix.Len());
        auto ConnectionId = static_cast<int16>(FCString::Atoi(*ConnectionIdString));

        auto IncomingUnackedMessagesAttributeName =
            FString::Printf(TEXT("%s%d"), *IncomingUnackedMessagesAttributePrefix, ConnectionId);
        auto IncomingAckedMessageIdAttributeName =
            FString::Printf(TEXT("%s%d"), *IncomingAckedMessageIdAttributePrefix, ConnectionId);
        auto IncomingClosedAttributeName = FString::Printf(TEXT("%s%d"), *IncomingClosedAttributePrefix, ConnectionId);

        auto IncomingSystemNameAttribute = KV.Value;
        auto *IncomingUnackedMessagesAttribute = Member->GetAttributes().Find(IncomingUnackedMessagesAttributeName);
        auto *IncomingAckedMessageIdAttribute = Member->GetAttributes().Find(IncomingAckedMessageIdAttributeName);
        auto *IncomingClosedAttribute = Member->GetAttributes().Find(IncomingClosedAttributeName);

        // If the closed attribute exists, handle close.
        if (IncomingClosedAttribute != nullptr)
        {
            UE_LOG(
                LogRedpointEOSPresence,
                VeryVerbose,
                TEXT("Presence message global connection from %s to %s detected that the remote user requested close "
                     "of instanced connection with connection ID %d."),
                *GetUserIdString(this->LocalUserId),
                *GetUserIdString(this->TargetUserId),
                ConnectionId);

            if (this->InstancedConnections.Contains(ConnectionId))
            {
                this->CloseInstanced(
                    this->InstancedConnections[ConnectionId].ToSharedRef(),
                    IPresenceMessageConnection::FOnCloseComplete());
            }
            continue;
        }

        // If any of the values are invalid, ignore this entry.
        if (IncomingUnackedMessagesAttribute == nullptr || IncomingAckedMessageIdAttribute == nullptr ||
            IncomingUnackedMessagesAttribute->Type() != FRoomAttribute::EType::String ||
            IncomingAckedMessageIdAttribute->Type() != FRoomAttribute::EType::Int64)
        {
            continue;
        }

        // If none of the relevant values have changed, ignore this entry.
        if (!RoomMemberUpdateInfo.AttributesAdded.Contains(IncomingUnackedMessagesAttributeName) &&
            !RoomMemberUpdateInfo.AttributesAdded.Contains(IncomingAckedMessageIdAttributeName) &&
            !RoomMemberUpdateInfo.AttributesUpdated.Contains(IncomingUnackedMessagesAttributeName) &&
            !RoomMemberUpdateInfo.AttributesUpdated.Contains(IncomingAckedMessageIdAttributeName))
        {
            continue;
        }

        // If we don't have an instanced connection, then this is a new connection!
        if (!this->InstancedConnections.Contains(ConnectionId))
        {
            UE_LOG(
                LogRedpointEOSPresence,
                VeryVerbose,
                TEXT("Presence message global connection from %s to %s received a new instanced connection with "
                     "connection ID %d."),
                *GetUserIdString(this->LocalUserId),
                *GetUserIdString(this->TargetUserId),
                ConnectionId);

            auto NewInstancedConnection = MakeShared<FPresenceMessageInstancedConnection>(
                this->AsShared(),
                FName(*IncomingSystemNameAttribute.GetString()),
                ConnectionId);
            this->InstancedConnections.Add(ConnectionId, NewInstancedConnection);
            this->OnRemoteConnectionReceivedDelegate.Broadcast(NewInstancedConnection);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSPresence,
                VeryVerbose,
                TEXT("Presence message global connection from %s to %s received an updated inbound message and ack for "
                     "connection ID %d."),
                *GetUserIdString(this->LocalUserId),
                *GetUserIdString(this->TargetUserId),
                ConnectionId);
        }
        auto &InstancedConnection = this->InstancedConnections[ConnectionId];

        // Parse the unacked messages out.
        UE_LOG(
            LogRedpointEOSPresence,
            VeryVerbose,
            TEXT("Received incoming unacked messages attribute for local user '%s': %s"),
            *GetUserIdString(this->TargetUserId),
            *IncomingUnackedMessagesAttribute->GetString());
        TArray<TSharedPtr<FJsonValue>> Array;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(IncomingUnackedMessagesAttribute->GetString());
        FJsonSerializer::Deserialize(*Reader, Array);
        for (const auto &Entry : Array)
        {
            if (Entry->Type != EJson::Object)
            {
                UE_LOG(
                    LogRedpointEOSPresence,
                    Warning,
                    TEXT("JSON entry in presence message connection attributes ignored as it is invalid."));
                continue;
            }
            auto Object = Entry->AsObject();
            auto MessageIdField = Object->TryGetField(TEXT("Id"));
            auto MessageTypeField = Object->TryGetField(TEXT("Type"));
            auto MessageContentField = Object->TryGetField(TEXT("Content"));
            if (MessageIdField.IsValid() && MessageIdField->Type == EJson::String && MessageTypeField.IsValid() &&
                MessageTypeField->Type == EJson::String && MessageContentField.IsValid() &&
                FCString::Atoi64(*MessageIdField->AsString()) != 0)
            {
                auto MessageId = FCString::Atoi64(*MessageIdField->AsString());
                if (InstancedConnection->UnackedInboundMessages.Contains(MessageId))
                {
                    // We've already got this message.
                    continue;
                }
                auto MessageType = FName(*MessageTypeField->AsString());
                auto MessageContent = MessageContentField.ToSharedRef();
                InstancedConnection->UnackedInboundMessages.Add(
                    MessageId,
                    MakeShareable(new FPresenceMessageInbound{
                        MessageId,
                        MessageType,
                        MessageContent,
                        FDateTime::UtcNow(),
                        false}));
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSPresence,
                    Warning,
                    TEXT("JSON entry in presence message connection attributes ignored as it is invalid."));
            }
        }

        // For any outbound messages where the message ID is lower or equal to the acked message ID, remove them.
        auto IncomingAckedMessageId = IncomingAckedMessageIdAttribute->GetInt64();
        TSet<int64> MessageIdsToRemove;
        for (const auto &OutboundMessage : InstancedConnection->UnackedOutboundMessages)
        {
            if (OutboundMessage.Key <= IncomingAckedMessageId)
            {
                MessageIdsToRemove.Add(OutboundMessage.Key);
            }
        }
        for (const auto &Id : MessageIdsToRemove)
        {
            // Make a copy of the delegate so we can fire it after we remove the message from unacked outbound messages.
            auto SendMessageDelegate = InstancedConnection->UnackedOutboundMessages[Id]->OnSendMessageComplete;

            // Remove the now acked message.
            InstancedConnection->UnackedOutboundMessages.Remove(Id);

            // Broadcast the event (which may result in connections closing and UnackedOutboundMessages being iterated
            // on).
            SendMessageDelegate.ExecuteIfBound(Errors::Success());
        }
    }

    // Schedule processing of received messages.
    this->ScheduleWork();
}

void FPresenceMessageGlobalConnection::OnKickRoomMemberOnCloseComplete(
    const FError &ErrorCode,
    TSharedRef<FPresenceMessageGlobalConnection> KeepAlive)
{
    if (!ErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Warning,
            TEXT("Presence message connection failed to kick room member on close: %s"),
            *ErrorCode.ToLogString());
    }
}

void FPresenceMessageGlobalConnection::OnLeaveRoomOnCloseComplete(
    const FError &ErrorCode,
    TSharedRef<FPresenceMessageGlobalConnection> KeepAlive)
{
    if (!ErrorCode.WasSuccessful())
    {
        UE_LOG(
            LogRedpointEOSPresence,
            Warning,
            TEXT("Presence message connection failed to leave room on close: %s"),
            *ErrorCode.ToLogString());
    }
}

void FPresenceMessageGlobalConnection::SendInstancedMessage(
    const TSharedRef<FPresenceMessageInstancedConnection> &InInstancedConnection,
    const FName &InMessageType,
    const TSharedRef<FJsonValue> &InMessage,
    const IPresenceMessageConnection::FOnSendMessageComplete &InCompletionDelegate)
{
    // Check that the connection is still valid.
    checkf(
        this->InstancedConnections.Contains(InInstancedConnection->ConnectionId) &&
            this->InstancedConnections[InInstancedConnection->ConnectionId] == InInstancedConnection &&
            InInstancedConnection->bOpen,
        TEXT("Invalid instanced connection passed to SendInstancedMessage"));

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Presence message global connection from %s to %s is sending instanced message for system %s with "
             "connection ID %d."),
        *GetUserIdString(this->LocalUserId),
        *GetUserIdString(this->TargetUserId),
        *InInstancedConnection->InterestedSystem.ToString(),
        InInstancedConnection->ConnectionId);

    // Add the message to our unacked messages.
    auto MessageId = this->NextOutboundMessageId++;
    InInstancedConnection->UnackedOutboundMessages.Add(
        MessageId,
        MakeShareable(new FPresenceMessageOutbound{MessageId, InMessageType, InMessage, InCompletionDelegate}));

    // Request that the update be pushed to the room member data.
    this->ScheduleWork();
}

void FPresenceMessageGlobalConnection::CloseInstanced(
    const TSharedRef<FPresenceMessageInstancedConnection> &InInstancedConnection,
    const IPresenceMessageConnection::FOnCloseComplete &InCompletionDelegate)
{
    checkf(
        this->InstancedConnections.Contains(InInstancedConnection->ConnectionId) &&
            this->InstancedConnections[InInstancedConnection->ConnectionId] == InInstancedConnection &&
            InInstancedConnection->bOpen,
        TEXT("Invalid instanced connection passed to SendInstancedMessage"));

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Presence message global connection from %s to %s is closing instanced connection for system %s with "
             "connection ID %d."),
        *GetUserIdString(this->LocalUserId),
        *GetUserIdString(this->TargetUserId),
        *InInstancedConnection->InterestedSystem.ToString(),
        InInstancedConnection->ConnectionId);

    InInstancedConnection->bOpen = false;
    for (const auto &PendingSend : InInstancedConnection->UnackedOutboundMessages)
    {
        PendingSend.Value->OnSendMessageComplete.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The connection went away.")));
    }
    InInstancedConnection->UnackedOutboundMessages.Empty();
    InInstancedConnection->OnMessageReceivedOrCloseDelegate
        .Broadcast(true, NAME_None, nullptr, IPresenceMessageConnection::FAckMessage());
    this->InstancedConnections.Remove(InInstancedConnection->ConnectionId);
    this->ClosedConnections.Add(InInstancedConnection->ConnectionId);
    if (this->InstancedConnections.IsEmpty())
    {
        // We have no more instanced connections, close the global connection by leaving the room or kicking
        // the target member.
        this->CloseInternal(true);
    }

    // @note: Should this wait until CloseInternal completes?
    InCompletionDelegate.ExecuteIfBound(Errors::Success());
}

TSharedPtr<IPresenceMessageConnection> FPresenceMessageGlobalConnection::CreateInstanced(
    const FName &InInterestedSystem)
{
    if (!this->bOpen)
    {
        // Can't establish new connections if we're not open.
        return nullptr;
    }

    auto ConnectionId = this->NextConnectionId++;
    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Presence message global connection from %s to %s is creating instanced connection for system %s with "
             "connection ID %d."),
        *GetUserIdString(this->LocalUserId),
        *GetUserIdString(this->TargetUserId),
        *InInterestedSystem.ToString(),
        ConnectionId);

    // Add the initial instanced connection.
    auto NewInstancedConnection =
        MakeShared<FPresenceMessageInstancedConnection>(this->AsShared(), this->InitialInterestedSystem, ConnectionId);
    this->InstancedConnections.Add(NewInstancedConnection->ConnectionId, NewInstancedConnection);

    // Do an initial flush so the remote immediately knows about this new connection (even with no messages sent yet).
    this->ScheduleWork();

    return NewInstancedConnection;
}

void FPresenceMessageGlobalConnection::CloseInternal(bool bRequiresRoomLeaveOrKick)
{
    checkf(
        this->OnConnectionClosedDelegate.IsBound(),
        TEXT("Expected OnConnectionClosedDelegate to be bound when CloseInternal is called! This delegate must be set "
             "up immediately after the global connection is created."));

    // Skip if already closed.
    if (!this->bOpen)
    {
        return;
    }

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Closing presence message global connection from %s to %s (requires room leave or kick: %s)."),
        *GetUserIdString(this->LocalUserId),
        *GetUserIdString(this->TargetUserId),
        bRequiresRoomLeaveOrKick ? TEXT("yes") : TEXT("no"));

    // Mark as closed early since CloseInternal() is async.
    this->bOpen = false;

    // Notify that this connection is now fully closed.
    TSharedPtr<FPresenceMessageGlobalConnection> KeepAlive = this->AsShared();
    this->OnConnectionClosedDelegate.Broadcast();

    // Close all of the instanced connections.
    for (const auto &KV : TMap<int16, TSharedPtr<FPresenceMessageInstancedConnection>>(this->InstancedConnections))
    {
        this->CloseInstanced(KV.Value.ToSharedRef(), IPresenceMessageConnection::FOnCloseComplete());
    }

    // Unregister events.
    this->RoomSystem->OnRoomLeft(CrossPlatformPresenceNamespace).Remove(this->OnRoomLeftDelegateHandle);
    this->Room->OnRoomMemberUpdated().Remove(this->OnRoomMemberUpdatedDelegateHandle);

    // If we're not the owner, leave the room. If we are the owner, kick the target user.
    // (FPresenceMessageConnectionImpl is shared between both hosts and clients).
    if (bRequiresRoomLeaveOrKick)
    {
        if (this->Room->GetRoomOwner()->GetUserId() == this->LocalUserId)
        {
            this->RoomSystem->KickRoomMember(
                this->Room,
                FKickRoomMemberRequest{this->TargetUserId},
                IRoomSystem::FOnKickRoomMemberComplete::CreateSP(
                    this,
                    &FPresenceMessageGlobalConnection::OnKickRoomMemberOnCloseComplete,
                    KeepAlive.ToSharedRef()));
        }
        else
        {
            this->RoomSystem->LeaveRoom(
                this->Room,
                FLeaveRoomRequest(),
                IRoomSystem::FOnLeaveRoomComplete::CreateSP(
                    this,
                    &FPresenceMessageGlobalConnection::OnLeaveRoomOnCloseComplete,
                    KeepAlive.ToSharedRef()));
        }
    }
    else
    {
        KeepAlive.Reset();
    }
}

void FPresenceMessageGlobalConnection::ScheduleWork()
{
    if (!this->bOpen)
    {
        return;
    }

    // Check if our outbound content is up-to-date.
    uint32 Hash = 0;
    Hash = HashCombine(Hash, GetTypeHash(this->InstancedConnections.Num()));
    for (const auto &InstancedConnection : this->InstancedConnections)
    {
        FString LocalOutputString;
        uint32 LocalHash;
        InstancedConnection.Value->SerializeAndHashOutbound(LocalOutputString, LocalHash);
        Hash = HashCombine(Hash, GetTypeHash(InstancedConnection.Key));
        Hash = HashCombine(Hash, GetTypeHash(LocalHash));
    }
    Hash = HashCombine(Hash, GetTypeHash(this->ClosedConnections.Num()));
    for (const auto &ConnectionId : this->ClosedConnections)
    {
        Hash = HashCombine(Hash, GetTypeHash(ConnectionId));
    }
    bool bIsOutboundUpToDate = Hash == this->LastFlushedHash;

    // Schedule an update of the outbound room member attributes if we're not up-to-date.
    if (!bIsOutboundUpToDate)
    {
        this->WorkQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
            this,
            &FPresenceMessageGlobalConnection::ProcessOutboundWork));
    }

    // Check if our inbound content is up-to-date.
    bool bIsInboundUpToDate = true;
    for (const auto &InstancedConnection : this->InstancedConnections)
    {
        if (InstancedConnection.Value->UnackedInboundMessages.Num() > 0)
        {
            bIsInboundUpToDate = false;
            break;
        }
    }

    // Schedule processing of the next inbound message if there are inbound messages that are not yet acked.
    if (!bIsInboundUpToDate)
    {
        this->WorkQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
            this,
            &FPresenceMessageGlobalConnection::ProcessInboundWork));
    }
}

void FPresenceMessageGlobalConnection::ProcessOutboundWork(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone)
{
    if (!this->bOpen)
    {
        InOnDone.ExecuteIfBound();
        return;
    }

    // Check if our outbound content is up-to-date, since we might get multiple instances of this call queued up and
    // only the first one needs to run.
    uint32 Hash = 0;
    Hash = HashCombine(Hash, GetTypeHash(this->InstancedConnections.Num()));
    for (const auto &InstancedConnection : this->InstancedConnections)
    {
        FString LocalOutputString;
        uint32 LocalHash;
        InstancedConnection.Value->SerializeAndHashOutbound(LocalOutputString, LocalHash);
        Hash = HashCombine(Hash, GetTypeHash(InstancedConnection.Key));
        Hash = HashCombine(Hash, GetTypeHash(LocalHash));
    }
    Hash = HashCombine(Hash, GetTypeHash(this->ClosedConnections.Num()));
    for (const auto &ConnectionId : this->ClosedConnections)
    {
        Hash = HashCombine(Hash, GetTypeHash(ConnectionId));
    }
    bool bIsOutboundUpToDate = Hash == this->LastFlushedHash;
    if (bIsOutboundUpToDate)
    {
        InOnDone.ExecuteIfBound();
        return;
    }

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Processing outbound work for presence connection because hash %d does not match expected hash %d."),
        this->LastFlushedHash,
        Hash);

    // Update the room member data.
    auto Request = FUpdateRoomRequest();
    for (const auto &InstancedConnection : this->InstancedConnections)
    {
        FString LocalOutputString;
        uint32 LocalHash;
        InstancedConnection.Value->SerializeAndHashOutbound(LocalOutputString, LocalHash);

        Request.UpsertMemberAttributes.Add(
            FString::Printf(
                TEXT("s.%s.%d"),
                *GetUserIdString(this->TargetUserId),
                InstancedConnection.Value->ConnectionId),
            InstancedConnection.Value->InterestedSystem.ToString());
        Request.UpsertMemberAttributes.Add(
            FString::Printf(
                TEXT("u.%s.%d"),
                *GetUserIdString(this->TargetUserId),
                InstancedConnection.Value->ConnectionId),
            LocalOutputString);
        Request.UpsertMemberAttributes.Add(
            FString::Printf(
                TEXT("a.%s.%d"),
                *GetUserIdString(this->TargetUserId),
                InstancedConnection.Value->ConnectionId),
            InstancedConnection.Value->AckedInboundMessageId);
    }
    for (const auto &ClosedConnection : this->ClosedConnections)
    {
        Request.DeleteMemberAttributes.Add(
            FString::Printf(TEXT("s.%s.%d"), *GetUserIdString(this->TargetUserId), ClosedConnection));
        Request.DeleteMemberAttributes.Add(
            FString::Printf(TEXT("u.%s.%d"), *GetUserIdString(this->TargetUserId), ClosedConnection));
        Request.DeleteMemberAttributes.Add(
            FString::Printf(TEXT("a.%s.%d"), *GetUserIdString(this->TargetUserId), ClosedConnection));
        Request.UpsertMemberAttributes.Add(
            FString::Printf(TEXT("c.%s.%d"), *GetUserIdString(this->TargetUserId), ClosedConnection),
            true);
    }
    this->RoomSystem->UpdateRoom(
        this->Room,
        Request,
        IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(this, [this, InOnDone, Hash](const FError &ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSPresence,
                    Warning,
                    TEXT("Failed to update room for presence communication: %s"),
                    *ErrorCode.ToLogString());
            }
            else
            {
                this->LastFlushedHash = Hash;
            }
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSPLambda(this, [this](float) {
                this->ScheduleWork();
                return false;
            }));
            InOnDone.ExecuteIfBound();
        }));
}

void FPresenceMessageGlobalConnection::ProcessInboundWork(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone)
{
    if (!this->bOpen)
    {
        InOnDone.ExecuteIfBound();
        return;
    }

    // Pop the next incoming message off the queue.
    TSharedPtr<FPresenceMessageInstancedConnection> NextInstancedConnection;
    TSharedPtr<FPresenceMessageInbound> NextInboundMessage;
    for (const auto &InstancedConnection : this->InstancedConnections)
    {
        InstancedConnection.Value->UnackedInboundMessages.KeySort([](const int64 &A, const int64 &B) {
            return A < B;
        });
        for (const auto &KV : InstancedConnection.Value->UnackedInboundMessages)
        {
            NextInboundMessage = KV.Value;
            NextInstancedConnection = InstancedConnection.Value;
            break;
        }
        if (NextInboundMessage.IsValid())
        {
            break;
        }
    }
    if (!NextInboundMessage.IsValid())
    {
        InOnDone.ExecuteIfBound();
        return;
    }

    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Processing inbound work for presence connection because there's at least one unacked inbound message."));

    // Start a timer. This will close the connection if we are unable to handle the message within the
    // timeout.
    auto TimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSPLambda(
            this,
            [this, NextInboundMessage, NextInstancedConnection, InOnDone](float) {
                if (NextInboundMessage->bIsAcked)
                {
                    return false;
                }
                // Failed to ack! Close connection because we can't ack any more messages beyond this
                // point.
                UE_LOG(
                    LogRedpointEOSPresence,
                    Error,
                    TEXT("Receiver of presence communication failed to ack received message within "
                         "timeout - the connection will be closed!"));
                if (this->InstancedConnections.Contains(NextInstancedConnection->ConnectionId) &&
                    NextInstancedConnection->bOpen)
                {
                    this->CloseInstanced(
                        NextInstancedConnection.ToSharedRef(),
                        IPresenceMessageConnection::FOnCloseComplete());
                    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSPLambda(this, [this](float) {
                        this->ScheduleWork();
                        return false;
                    }));
                }
                InOnDone.ExecuteIfBound();
                return false;
            }),
        15.0f);

    // Ask one of our handlers to handle the message.
    UE_LOG(
        LogRedpointEOSPresence,
        VeryVerbose,
        TEXT("Recipient is dispatching message ID %d for handling."),
        NextInboundMessage->MessageId);
    NextInstancedConnection->OnMessageReceivedOrCloseDelegate.Broadcast(
        false,
        NextInboundMessage->MessageType,
        NextInboundMessage->MessageContent,
        IPresenceMessageConnection::FAckMessage::CreateLambda(
            [this, NextInboundMessage, NextInstancedConnection, TimeoutHandle, InOnDone]() {
                // Mark as acked.
                if (NextInboundMessage->bIsAcked)
                {
                    return;
                }
                UE_LOG(
                    LogRedpointEOSPresence,
                    VeryVerbose,
                    TEXT("Recipient will ack message ID %d."),
                    NextInboundMessage->MessageId);
                NextInboundMessage->bIsAcked = true;

                // Cancel our timeout timer.
                FTSTicker::GetCoreTicker().RemoveTicker(TimeoutHandle);

                // Increment our acked message ID.
                NextInstancedConnection->AckedInboundMessageId = NextInboundMessage->MessageId;

                // Remove our inbound message from the unacked inbound messages list.
                NextInstancedConnection->UnackedInboundMessages.Remove(NextInboundMessage->MessageId);

                // Schedule work to flush the new acked message ID. Do this on a fire-and-forget next frame ticker,
                // otherwise we'll overflow the stack.
                FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSPLambda(this, [this](float) {
                    this->ScheduleWork();
                    return false;
                }));

                // Allow more work to run.
                InOnDone.ExecuteIfBound();
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()