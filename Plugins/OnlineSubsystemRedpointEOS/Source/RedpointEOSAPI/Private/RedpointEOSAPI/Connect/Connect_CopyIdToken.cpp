// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/CopyIdToken.h"

#include "RedpointEOSAPI/Templates/Impl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_SYNC_COPY_IMPL(CopyIdToken);

void FCopyIdToken::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FCopyIdToken::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.JsonWebToken = Converter.FromUtf8(NativeResult.JsonWebToken);
}

}

REDPOINT_EOS_CODE_GUARD_END()