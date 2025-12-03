// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1592561027, Redpoint::EOS::Friends::Database)
{

struct FLegacySerializedCachedFriend
{
public:
    FString AccountId;
    TArray<uint8> AccountIdBytes;
    uint32 AccountIdTypeHash;
    FString AccountDisplayName;
    FString AccountRealName;
    FString ExternalAccountId;
    int32 ExternalAccountIdType;
    FString AccountAvatarUrl;
    FDateTime AccountAvatarUrlLastFetched;

    friend FArchive &operator<<(FArchive &Ar, FLegacySerializedCachedFriend &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(1592561027, Redpoint::EOS::Friends::Database, FLegacySerializedCachedFriend);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()