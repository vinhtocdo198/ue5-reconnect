// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/SerializedPendingFriendRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2931148415, Redpoint::EOS::Friends::Database)
{

FArchive &operator<<(FArchive &Ar, FSerializedPendingFriendRequest &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 2;
    }
    Ar << Version;
    Ar << Obj.ProductUserId;
    Ar << *reinterpret_cast<uint8 *>(&Obj.Mode);
    if (Version >= 2)
    {
        Ar << Obj.NextAttempt;
    }
    return Ar;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()