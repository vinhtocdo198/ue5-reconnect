// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Dom/JsonObject.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(627140365, Redpoint::EOS::Presence)
{

struct FPresenceMessageInbound
{
    int64 MessageId;
    FName MessageType;
    TSharedRef<FJsonValue> MessageContent;
    FDateTime ReceivedTime;
    bool bIsAcked;
};

}

namespace Redpoint::EOS::Presence
{
REDPOINT_EOS_FILE_NS_EXPORT(627140365, Redpoint::EOS::Presence, FPresenceMessageInbound)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()