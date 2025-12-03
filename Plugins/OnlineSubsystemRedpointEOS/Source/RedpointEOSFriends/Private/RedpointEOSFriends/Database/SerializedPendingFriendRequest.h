// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSFriends/Database/SerializedPendingFriendRequestMode.h"
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2931148415, Redpoint::EOS::Friends::Database)
{
using namespace ::Redpoint::EOS::Friends::Database;

struct FSerializedPendingFriendRequest
{
public:
    FString ProductUserId;
    ESerializedPendingFriendRequestMode Mode;
    FDateTime NextAttempt;

    friend FArchive &operator<<(FArchive &Ar, FSerializedPendingFriendRequest &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(2931148415, Redpoint::EOS::Friends::Database, FSerializedPendingFriendRequest);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()