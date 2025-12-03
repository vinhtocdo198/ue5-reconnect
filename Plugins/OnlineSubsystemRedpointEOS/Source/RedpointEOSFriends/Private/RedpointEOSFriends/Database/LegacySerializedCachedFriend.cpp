// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/LegacySerializedCachedFriend.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1592561027, Redpoint::EOS::Friends::Database)
{

FArchive &operator<<(FArchive &Ar, FLegacySerializedCachedFriend &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 3;
    }
    Ar << Version;
    Ar << Obj.AccountId;
    Ar << Obj.AccountIdBytes;
    Ar << Obj.AccountIdTypeHash;
    Ar << Obj.AccountDisplayName;
    Ar << Obj.AccountRealName;
    Ar << Obj.ExternalAccountId;
    Ar << Obj.ExternalAccountIdType;
    if (Version >= 2)
    {
        Ar << Obj.AccountAvatarUrl;
    }
    if (Version >= 3)
    {
        Ar << Obj.AccountAvatarUrlLastFetched;
    }
    return Ar;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()