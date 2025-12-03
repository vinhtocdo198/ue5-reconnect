// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Connect/VerifyIdToken.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(VerifyIdToken);

void FVerifyIdToken::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    auto &IdToken = Allocator.Allocate<EOS_Connect_IdToken>();
    IdToken.ApiVersion = EOS_CONNECT_IDTOKEN_API_LATEST;
    IdToken.JsonWebToken = Allocator.AsUtf8(*Options.JsonWebToken);
    IdToken.ProductUserId = *Options.ProductUserId;

    NativeOptions.IdToken = &IdToken;
}

void FVerifyIdToken::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

}

REDPOINT_EOS_CODE_GUARD_END()