// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Auth/LinkAccount.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(LinkAccount);

void FLinkAccount::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.ContinuanceToken = *Options.ContinuanceToken;
    NativeOptions.LinkAccountFlags = *Options.LinkAccountFlags;
    NativeOptions.LocalUserId = *Options.LocalUserId;
}

void FLinkAccount::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LocalUserId = NativeResult.LocalUserId;
    if (NativeResult.PinGrantInfo != nullptr)
    {
        FPinGrantInfo Info = {};
        Info.UserCode = Converter.FromUtf8(NativeResult.PinGrantInfo->UserCode);
        Info.VerificationURI = Converter.FromUtf8(NativeResult.PinGrantInfo->VerificationURI);
        Info.ExpiresIn = NativeResult.PinGrantInfo->ExpiresIn;
        Info.VerificationURIComplete = Converter.FromUtf8(NativeResult.PinGrantInfo->VerificationURIComplete);
        Result.PinGrantInfo = Info;
    }
    else
    {
        Result.PinGrantInfo.Reset();
    }
    Result.SelectedAccountId = NativeResult.SelectedAccountId;
}

}

REDPOINT_EOS_CODE_GUARD_END()