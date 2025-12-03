// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Presence/CopyPresence.h"

#include "RedpointEOSAPI/Templates/Impl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

REDPOINT_EOSSDK_API_CALL_SYNC_COPY_IMPL(CopyPresence);

void FCopyPresence::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.TargetUserId = *Options.TargetUserId;
}

void FCopyPresence::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.Status = NativeResult.Status;
    Result.UserId = NativeResult.UserId;
    Result.ProductId = Converter.FromUtf8(NativeResult.ProductId);
    Result.ProductVersion = Converter.FromUtf8(NativeResult.ProductVersion);
    Result.Platform = Converter.FromUtf8(NativeResult.Platform);
    Result.RichText = Converter.FromUtf8(NativeResult.RichText);
    for (int32_t i = 0; i < NativeResult.RecordsCount; i++)
    {
        Result.DataRecords.Add(
            Converter.FromUtf8(NativeResult.Records[i].Key),
            Converter.FromUtf8(NativeResult.Records[i].Value));
    }
    Result.ProductName = Converter.FromUtf8(NativeResult.ProductName);
    Result.IntegratedPlatform = Converter.FromUtf8(NativeResult.IntegratedPlatform);
}

}

REDPOINT_EOS_CODE_GUARD_END()