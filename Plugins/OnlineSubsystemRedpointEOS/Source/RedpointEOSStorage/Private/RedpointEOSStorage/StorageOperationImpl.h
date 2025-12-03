// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSStorage/StorageOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(707357170, Redpoint::EOS::Storage)
{
using namespace ::Redpoint::EOS::Storage;

class REDPOINTEOSSTORAGE_API FStorageOperationImpl : public IStorageOperation
{
public:
    FStorageOperationImpl(EOS_HPlayerDataStorageFileTransferRequest InHandle);
    FStorageOperationImpl(EOS_HTitleStorageFileTransferRequest InHandle);
    FStorageOperationImpl();
    UE_NONCOPYABLE(FStorageOperationImpl);
    virtual ~FStorageOperationImpl() override = default;

    TOptional<EOS_HPlayerDataStorageFileTransferRequest> PlayerDataStorageHandle;
    TOptional<EOS_HTitleStorageFileTransferRequest> TitleStorageHandle;
    bool bCancelled;
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(707357170, Redpoint::EOS::Storage, FStorageOperationImpl)
}

REDPOINT_EOS_CODE_GUARD_END()