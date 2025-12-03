// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSStorage/StorageOperationImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(707357170, Redpoint::EOS::Storage)
{

FStorageOperationImpl::FStorageOperationImpl(EOS_HPlayerDataStorageFileTransferRequest InHandle)
    : PlayerDataStorageHandle(InHandle)
    , TitleStorageHandle()
    , bCancelled(false)
{
}

FStorageOperationImpl::FStorageOperationImpl(EOS_HTitleStorageFileTransferRequest InHandle)
    : PlayerDataStorageHandle()
    , TitleStorageHandle(InHandle)
    , bCancelled(false)
{
}

FStorageOperationImpl::FStorageOperationImpl()
    : PlayerDataStorageHandle()
    , TitleStorageHandle()
    , bCancelled(false)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()