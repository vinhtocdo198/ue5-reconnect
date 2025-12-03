// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTCAudio/QueryOutputDevicesInformation.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_IMPL(QueryOutputDevicesInformation);

void FQueryOutputDevicesInformation::MapQueryOptions(
    NativeQueryOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FQueryOutputDevicesInformation::MapCountOptions(
    NativeCountOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FQueryOutputDevicesInformation::MapCopyByIndexOptions(
    NativeCopyByIndexOptions &NativeOptions,
    const Options &Options,
    uint32_t Index,
    NativeAllocator &Allocator)
{
    NativeOptions.DeviceIndex = Index;
}

void FQueryOutputDevicesInformation::MapCopyByIndexResult(
    ResultEntry &Result,
    const NativeCopyByIndexResult &NativeResult,
    NativeConverter &Converter)
{
    Result.Id = Converter.FromAnsi(NativeResult.DeviceId);
    Result.DisplayName = Converter.FromUtf8(NativeResult.DeviceName);
    Result.bIsDefaultDevice = NativeResult.bDefaultDevice == EOS_TRUE;
    Result.bIsInputDevice = false;
}

}

REDPOINT_EOS_CODE_GUARD_END()