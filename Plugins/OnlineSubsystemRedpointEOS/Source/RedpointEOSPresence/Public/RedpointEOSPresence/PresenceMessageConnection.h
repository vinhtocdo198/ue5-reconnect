// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Dom/JsonValue.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityUserId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(639251057, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;

/**
 * Represents the whole presence state of a user.
 */
class REDPOINTEOSPRESENCE_API IPresenceMessageConnection
{
protected:
    IPresenceMessageConnection() = default;

public:
    UE_NONCOPYABLE(IPresenceMessageConnection);
    virtual ~IPresenceMessageConnection() = default;

    typedef TDelegate<void(const FError &Error)> FOnSendMessageComplete;
    typedef TDelegate<void()> FAckMessage;
    typedef TMulticastDelegate<void(
        bool bIsClosed,
        const FName &MessageType,
        const TSharedPtr<FJsonValue> &Message,
        const FAckMessage &AckMessage)>
        FOnMessageReceivedOrCloseDelegate;
    typedef TDelegate<void(const FError &Error)> FOnCloseComplete;

    /**
     * The local user sending messages.
     */
    virtual FIdentityUserId GetLocalUserId() = 0;

    /**
     * The target user that we're sending messages to.
     */
    virtual FIdentityUserId GetTargetUserId() = 0;

    /**
     * The interested system that this connection is for.
     */
    virtual FName GetInterestedSystem() = 0;

    /**
     * Send a message to the target user.
     */
    virtual void SendMessage(
        const FName &InMessageType,
        const TSharedRef<FJsonValue> &InMessage,
        const FOnSendMessageComplete &InCompletionDelegate) = 0;

    /**
     * This event is raised when a message is received from the target user, or when the connection is closed. For
     * messages, at least one handler must invoke the AckMessage parameter within the timeout, or the connection will be
     * closed.
     */
    virtual FOnMessageReceivedOrCloseDelegate &OnMessageReceivedOrClose() = 0;

    /**
     * Close the connection.
     */
    virtual void Close(const FOnCloseComplete &InCompletionDelegate) = 0;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(639251057, Redpoint::EOS::Presence, IPresenceMessageConnection)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()