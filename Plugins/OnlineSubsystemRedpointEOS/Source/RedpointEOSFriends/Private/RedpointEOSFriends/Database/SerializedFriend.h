// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2296086848, Redpoint::EOS::Friends::Database)
{

struct FSerializedFriend
{
public:
    FString ProductUserId;
    TOptional<FString> AvatarUrl;

    friend FArchive &operator<<(FArchive &Ar, FSerializedFriend &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(2296086848, Redpoint::EOS::Friends::Database, FSerializedFriend);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()