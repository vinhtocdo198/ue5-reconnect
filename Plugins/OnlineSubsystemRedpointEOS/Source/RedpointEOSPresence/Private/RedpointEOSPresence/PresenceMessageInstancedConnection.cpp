// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceMessageInstancedConnection.h"

#include "Policies/CondensedJsonPrintPolicy.h"
#include "RedpointEOSPresence/PresenceMessageGlobalConnection.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3479525659, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::API;

FPresenceMessageInstancedConnection::FPresenceMessageInstancedConnection(
    TSharedRef<FPresenceMessageGlobalConnection> InGlobalConnection,
    FName InInterestedSystem,
    int16 InConnectionId)
    : GlobalConnection(InGlobalConnection)
    , InterestedSystem(InInterestedSystem)
    , ConnectionId(InConnectionId)
    , UnackedOutboundMessages()
    , UnackedInboundMessages()
    , AckedInboundMessageId()
    , LastFlushedHash()
    , bOpen(true)
    , OnMessageReceivedOrCloseDelegate()
{
}

FIdentityUserId FPresenceMessageInstancedConnection::GetLocalUserId()
{
    return this->GlobalConnection->LocalUserId;
}

FIdentityUserId FPresenceMessageInstancedConnection::GetTargetUserId()
{
    return this->GlobalConnection->TargetUserId;
}

FName FPresenceMessageInstancedConnection::GetInterestedSystem()
{
    return this->InterestedSystem;
}

void FPresenceMessageInstancedConnection::SendMessage(
    const FName &InMessageType,
    const TSharedRef<FJsonValue> &InMessage,
    const FOnSendMessageComplete &InCompletionDelegate)
{
    if (!this->bOpen)
    {
        InCompletionDelegate.ExecuteIfBound(
            Errors::NoConnection(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The connection has been closed.")));
        return;
    }

    return this->GlobalConnection
        ->SendInstancedMessage(this->AsShared(), InMessageType, InMessage, InCompletionDelegate);
}

IPresenceMessageConnection::FOnMessageReceivedOrCloseDelegate &FPresenceMessageInstancedConnection::
    OnMessageReceivedOrClose()
{
    return this->OnMessageReceivedOrCloseDelegate;
}

void FPresenceMessageInstancedConnection::Close(const FOnCloseComplete &InCompletionDelegate)
{
    if (!this->bOpen)
    {
        InCompletionDelegate.ExecuteIfBound(Errors::Success());
        return;
    }

    this->GlobalConnection->CloseInstanced(this->AsShared(), InCompletionDelegate);
}

void FPresenceMessageInstancedConnection::SerializeAndHashOutbound(FString &OutUnackedMessages, uint32 &OutHash)
{
    // Sort the unacked messages.
    this->UnackedOutboundMessages.KeySort([](const int64 &A, const int64 &B) {
        return A < B;
    });

    // Generate the JSON blob for our unacked messages property.
    TArray<TSharedPtr<FJsonValue>> Json;
    for (const auto &UnackedMessage : UnackedOutboundMessages)
    {
        TSharedRef<FJsonObject> Message = MakeShared<FJsonObject>();
        Message->SetStringField("Id", FString::Printf(TEXT("%lld"), UnackedMessage.Key));
        Message->SetStringField("Type", UnackedMessage.Value->MessageType.ToString());
        Message->SetField("Content", UnackedMessage.Value->MessageContent);
        Json.Add(MakeShared<FJsonValueObject>(Message));
    }
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
        TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutUnackedMessages);
    FJsonSerializer::Serialize(Json, Writer);

    // Check if we're already up-to-date.
    OutHash = GetTypeHash(OutUnackedMessages);
    OutHash = HashCombine(OutHash, GetTypeHash(this->AckedInboundMessageId));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()