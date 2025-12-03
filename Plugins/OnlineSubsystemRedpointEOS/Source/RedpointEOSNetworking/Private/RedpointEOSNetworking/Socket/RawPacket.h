// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Socket/PacketTiming.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3622946561, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Networking::Socket;

class FRawPacket : public TSharedFromThis<FRawPacket>
{
private:
    void *Data;
    int32 DataLen;
    TSharedPtr<FPacketTiming> Timing;

public:
    FRawPacket(void *InData, int32 InDataLen, const TSharedPtr<FPacketTiming> &InTiming);
    UE_NONCOPYABLE(FRawPacket);
    ~FRawPacket();

    const void *GetData() const;
    int32 GetDataLen() const;
    const TSharedPtr<FPacketTiming> &GetTiming() const;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3622946561, Redpoint::EOS::Networking::Socket, FRawPacket)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
