// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Dom/JsonObject.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSPresence/PresenceMessageConnection.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(649210371, Redpoint::EOS::Presence)
{
using namespace ::Redpoint::EOS::Presence;

struct FPresenceMessageOutbound
{
    int64 MessageId;
    FName MessageType;
    TSharedRef<FJsonValue> MessageContent;
    IPresenceMessageConnection::FOnSendMessageComplete OnSendMessageComplete;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(649210371, Redpoint::EOS::Presence, FPresenceMessageOutbound)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()