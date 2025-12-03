// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3253301326, Redpoint::EOS::Friends::Database)
{

struct FSerializedRecentPlayer
{
public:
    FString ProductUserId;
    FDateTime LastSeen;

    friend FArchive &operator<<(FArchive &Ar, FSerializedRecentPlayer &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(3253301326, Redpoint::EOS::Friends::Database, FSerializedRecentPlayer);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()