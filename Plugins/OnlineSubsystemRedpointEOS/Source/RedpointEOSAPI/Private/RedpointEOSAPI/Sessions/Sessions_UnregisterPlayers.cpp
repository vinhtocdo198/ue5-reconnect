// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/UnregisterPlayers.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(UnregisterPlayers);

void FUnregisterPlayers::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
    NativeOptions.PlayersToUnregister = const_cast<EOS_ProductUserId *>(Allocator.AllocateArray(
        *Options.PlayersToUnregister,
        NativeOptions.PlayersToUnregisterCount,
        Templates::Util::EAllocationFlags::Default));
}

void FUnregisterPlayers::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    if (NativeResult.UnregisteredPlayers == nullptr || NativeResult.UnregisteredPlayersCount == 0)
    {
        Result.UnregisteredPlayers = TArray<EOS_ProductUserId>();
    }
    else
    {
        for (uint32 i = 0; i < NativeResult.UnregisteredPlayersCount; i++)
        {
            Result.UnregisteredPlayers.Add(NativeResult.UnregisteredPlayers[i]);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()