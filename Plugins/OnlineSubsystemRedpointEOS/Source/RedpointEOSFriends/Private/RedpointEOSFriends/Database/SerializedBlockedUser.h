// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(56419245, Redpoint::EOS::Friends::Database)
{

struct FSerializedBlockedUser
{
public:
    FString ProductUserId;

    friend FArchive &operator<<(FArchive &Ar, FSerializedBlockedUser &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(56419245, Redpoint::EOS::Friends::Database, FSerializedBlockedUser);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()