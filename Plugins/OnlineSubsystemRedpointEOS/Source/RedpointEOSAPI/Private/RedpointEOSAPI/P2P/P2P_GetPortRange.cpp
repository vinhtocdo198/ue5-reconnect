// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/GetPortRange.h"

#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Impl/SyncGetTwo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

REDPOINT_EOSSDK_API_CALL_SYNC_GET_TWO_IMPL(GetPortRange);

void FGetPortRange::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FGetPortRange::MapResult1(Result1 &Result1, const NativeResult1 &NativeResult1, NativeConverter &Converter)
{
    Result1 = NativeResult1;
}

void FGetPortRange::MapResult2(Result2 &Result2, const NativeResult2 &NativeResult2, NativeConverter &Converter)
{
    Result2 = NativeResult2;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()