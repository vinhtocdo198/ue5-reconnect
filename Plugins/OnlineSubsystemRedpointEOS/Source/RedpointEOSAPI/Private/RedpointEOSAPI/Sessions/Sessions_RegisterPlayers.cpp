// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Sessions/RegisterPlayers.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(RegisterPlayers);

void FRegisterPlayers::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SessionName = Allocator.AsAnsi(*Options.SessionName);
    NativeOptions.PlayersToRegister = const_cast<EOS_ProductUserId *>(Allocator.AllocateArray(
        *Options.PlayersToRegister,
        NativeOptions.PlayersToRegisterCount,
        Templates::Util::EAllocationFlags::Default));
}

void FRegisterPlayers::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    if (NativeResult.RegisteredPlayers == nullptr || NativeResult.RegisteredPlayersCount == 0)
    {
        Result.RegisteredPlayers = TArray<EOS_ProductUserId>();
    }
    else
    {
        for (uint32 i = 0; i < NativeResult.RegisteredPlayersCount; i++)
        {
            Result.RegisteredPlayers.Add(NativeResult.RegisteredPlayers[i]);
        }
    }
    if (NativeResult.SanctionedPlayers == nullptr || NativeResult.SanctionedPlayersCount == 0)
    {
        Result.SanctionedPlayers = TArray<EOS_ProductUserId>();
    }
    else
    {
        for (uint32 i = 0; i < NativeResult.SanctionedPlayersCount; i++)
        {
            Result.SanctionedPlayers.Add(NativeResult.SanctionedPlayers[i]);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()