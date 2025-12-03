// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSFriends/Database/SerializedFriendDatabase.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(522457488, Redpoint::EOS::Friends::Database)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Friends::Database;

class FDatabaseIO
{
public:
    typedef TDelegate<void(const FError &ErrorCode, const TSharedPtr<FSerializedFriendDatabase> &FriendDatabase)>
        FOnFriendDatabaseLoaded;
    typedef TDelegate<void(const FError &ErrorCode)> FOnFriendDatabaseSaved;

    static void LoadFriendDatabase(
        const FPlatformHandle &PlatformHandle,
        const UE::Online::FAccountId &LocalUserId,
        const FOnFriendDatabaseLoaded &OnLoaded);

    static void SaveFriendDatabase(
        const FPlatformHandle &PlatformHandle,
        const UE::Online::FAccountId &LocalUserId,
        const TSharedRef<FSerializedFriendDatabase> &FriendDatabase,
        bool bRequireNoDatabaseFilePresent,
        const FOnFriendDatabaseSaved &OnSaved);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(522457488, Redpoint::EOS::Friends::Database, FDatabaseIO);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()