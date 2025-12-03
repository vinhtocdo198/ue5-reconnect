// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3744226451, Redpoint::EOS::Friends::Database)
{

struct FSerializedCachedFriend
{
public:
    struct FSerializedCachedFriendLinkedAccount
    {
        EOS_EExternalAccountType ExternalAccountType;
        FString DisplayName;
        FString AccountId;
        TOptional<int64> LastLoginTime;

        friend FArchive &operator<<(FArchive &Ar, FSerializedCachedFriendLinkedAccount &Obj);
    };

    TMap<EOS_EExternalAccountType, FSerializedCachedFriendLinkedAccount> LinkedAccounts;
    TMap<FString, FString> Attributes;
    FString AccountAvatarUrl;
    FDateTime AccountAvatarUrlLastFetched;

    friend FArchive &operator<<(FArchive &Ar, FSerializedCachedFriend &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(3744226451, Redpoint::EOS::Friends::Database, FSerializedCachedFriend);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()