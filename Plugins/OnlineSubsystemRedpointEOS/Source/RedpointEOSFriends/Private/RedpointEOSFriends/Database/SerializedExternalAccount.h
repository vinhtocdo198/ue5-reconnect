// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1626598827, Redpoint::EOS::Friends::Database)
{

struct FSerializedExternalAccount
{
public:
    FString AccountType;
    FString AccountId;
    FString AccountDisplayName;

    friend FArchive &operator<<(FArchive &Ar, FSerializedExternalAccount &Obj);
};

}

namespace Redpoint::EOS::Friends::Database
{
REDPOINT_EOS_FILE_NS_EXPORT(1626598827, Redpoint::EOS::Friends::Database, FSerializedExternalAccount);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()