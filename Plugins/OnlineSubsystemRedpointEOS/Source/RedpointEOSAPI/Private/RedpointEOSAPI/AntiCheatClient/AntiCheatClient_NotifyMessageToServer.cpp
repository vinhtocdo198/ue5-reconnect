// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/NotifyMessageToServer.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyMessageToServer);

void FNotifyMessageToServer::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator)
{
}

void FNotifyMessageToServer::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.MessageData = NativeResult.MessageData;
    Result.MessageDataSizeInBytes = NativeResult.MessageDataSizeBytes;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()