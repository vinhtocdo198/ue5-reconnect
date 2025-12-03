// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2386936411, Redpoint::EOS::Storage)
{

class REDPOINTEOSSTORAGE_API IStorageOperation
{
public:
    IStorageOperation() = default;
    UE_NONCOPYABLE(IStorageOperation);
    virtual ~IStorageOperation() = default;
};

typedef TSharedRef<IStorageOperation> FStorageOperationRef;

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(2386936411, Redpoint::EOS::Storage, IStorageOperation)
REDPOINT_EOS_FILE_NS_EXPORT(2386936411, Redpoint::EOS::Storage, FStorageOperationRef)
}

REDPOINT_EOS_CODE_GUARD_END()