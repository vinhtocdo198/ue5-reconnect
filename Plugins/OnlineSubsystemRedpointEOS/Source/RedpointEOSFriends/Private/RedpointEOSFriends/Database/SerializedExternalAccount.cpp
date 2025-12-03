// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/SerializedExternalAccount.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1626598827, Redpoint::EOS::Friends::Database)
{

FArchive &operator<<(FArchive &Ar, FSerializedExternalAccount &Obj)
{
    int8 Version = -1;
    if (Ar.IsSaving())
    {
        Version = 1;
    }
    Ar << Version;
    Ar << Obj.AccountType;
    Ar << Obj.AccountId;
    Ar << Obj.AccountDisplayName;
    return Ar;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()