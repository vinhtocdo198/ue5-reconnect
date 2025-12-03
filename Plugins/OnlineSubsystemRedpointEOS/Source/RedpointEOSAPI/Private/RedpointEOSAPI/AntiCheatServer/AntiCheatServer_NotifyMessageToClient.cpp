// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatServer/NotifyMessageToClient.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_SERVER_CODE

namespace Redpoint::EOS::API::AntiCheatServer
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyMessageToClient);

void FNotifyMessageToClient::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyMessageToClient::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ClientHandle = NativeResult.ClientHandle;
    Result.MessageData = NativeResult.MessageData;
    Result.MessageDataSizeInBytes = NativeResult.MessageDataSizeBytes;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()