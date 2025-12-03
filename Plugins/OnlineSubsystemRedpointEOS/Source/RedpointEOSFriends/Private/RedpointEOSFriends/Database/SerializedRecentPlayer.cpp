// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/SerializedRecentPlayer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3253301326, Redpoint::EOS::Friends::Database)
{

FArchive &operator<<(FArchive &Ar, FSerializedRecentPlayer &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 1;
    }
    Ar << Version;
    Ar << Obj.ProductUserId;
    Ar << Obj.LastSeen;
    return Ar;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()