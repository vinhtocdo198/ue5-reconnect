// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/RawPacket.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3622946561, Redpoint::EOS::Networking::Socket)
{

FRawPacket::FRawPacket(void *InData, int32 InDataLen, const TSharedPtr<FPacketTiming> &InTiming)
    : Data(InData)
    , DataLen(InDataLen)
    , Timing(InTiming)
{
}

FRawPacket::~FRawPacket()
{
    FMemory::Free(this->Data);
}

const void *FRawPacket::GetData() const
{
    return (const void *)this->Data;
}

int32 FRawPacket::GetDataLen() const
{
    return this->DataLen;
}

const TSharedPtr<FPacketTiming> &FRawPacket::GetTiming() const
{
    return this->Timing;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
