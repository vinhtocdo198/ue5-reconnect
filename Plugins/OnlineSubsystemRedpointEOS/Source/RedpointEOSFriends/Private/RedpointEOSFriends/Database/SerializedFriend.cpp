// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/SerializedFriend.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2296086848, Redpoint::EOS::Friends::Database)
{

FArchive &operator<<(FArchive &Ar, FSerializedFriend &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 2;
    }
    Ar << Version;
    Ar << Obj.ProductUserId;
    if (Version >= 2)
    {
        Ar << Obj.AvatarUrl;
    }
    return Ar;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()