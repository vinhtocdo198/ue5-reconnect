// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/RejectInvite.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(RejectInvite);

void FRejectInvite::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.InviteId = Allocator.AsAnsi(*Options.InviteId);
}

void FRejectInvite::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.InviteId = Converter.FromAnsi(NativeResult.InviteId);
}

}

REDPOINT_EOS_CODE_GUARD_END()