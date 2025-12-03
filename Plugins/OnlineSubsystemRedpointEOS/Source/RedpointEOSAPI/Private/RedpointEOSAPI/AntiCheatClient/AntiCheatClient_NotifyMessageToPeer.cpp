// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/AntiCheatClient/NotifyMessageToPeer.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/Event.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::AntiCheatClient
{

REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(NotifyMessageToPeer);

void FNotifyMessageToPeer::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FNotifyMessageToPeer::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ClientHandle = NativeResult.ClientHandle;
    Result.MessageData = NativeResult.MessageData;
    Result.MessageDataSizeInBytes = NativeResult.MessageDataSizeBytes;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()