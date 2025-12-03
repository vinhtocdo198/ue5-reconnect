// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPresence/PresenceMessageConnection.h"
#include "RedpointEOSPresence/PresenceMessageInbound.h"
#include "RedpointEOSPresence/PresenceMessageOutbound.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(402271213, Redpoint::EOS::Presence, FPresenceMessageGlobalConnection);

namespace REDPOINT_EOS_FILE_NS_ID(3479525659, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Identity;

class FPresenceMessageInstancedConnection : public IPresenceMessageConnection,
                                            public TSharedFromThis<FPresenceMessageInstancedConnection>
{
public:
    const TSharedRef<FPresenceMessageGlobalConnection> GlobalConnection;
    const FName InterestedSystem;
    const int16 ConnectionId;

    TMap<int64, TSharedPtr<FPresenceMessageOutbound>> UnackedOutboundMessages;
    TMap<int64, TSharedPtr<FPresenceMessageInbound>> UnackedInboundMessages;
    int64 AckedInboundMessageId;
    uint32 LastFlushedHash;
    bool bOpen;

    FOnMessageReceivedOrCloseDelegate OnMessageReceivedOrCloseDelegate;

    FPresenceMessageInstancedConnection(
        TSharedRef<FPresenceMessageGlobalConnection> InGlobalConnection,
        FName InInterestedSystem,
        int16 InConnectionId);
    UE_NONCOPYABLE(FPresenceMessageInstancedConnection);
    virtual ~FPresenceMessageInstancedConnection() override = default;

    virtual FIdentityUserId GetLocalUserId() override;
    virtual FIdentityUserId GetTargetUserId() override;
    virtual FName GetInterestedSystem() override;
    virtual void SendMessage(
        const FName &InMessageType,
        const TSharedRef<FJsonValue> &InMessage,
        const FOnSendMessageComplete &InCompletionDelegate) override;
    virtual FOnMessageReceivedOrCloseDelegate &OnMessageReceivedOrClose() override;
    virtual void Close(const FOnCloseComplete &InCompletionDelegate) override;

    void SerializeAndHashOutbound(FString &OutUnackedMessages, uint32 &OutHash);
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(3479525659, Redpoint::EOS::Presence, FPresenceMessageInstancedConnection)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()