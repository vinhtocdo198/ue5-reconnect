// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/P2P/SocketId.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(800251675, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;

struct FSocketConnectionMapKey
{
private:
    const UE::Online::FAccountId LocalUserId;
    const UE::Online::FAccountId RemoteUserId;
    const FSocketId SymmetricSocketId;
    const uint8 SymmetricChannel;

public:
    FSocketConnectionMapKey(
        UE::Online::FAccountId InLocalUserId,
        UE::Online::FAccountId InRemoteUserId,
        FSocketId InSymmetricSocketId,
        uint8 InSymmetricChannel);
    FSocketConnectionMapKey(const FSocketConnectionMapKey &Other);
    ~FSocketConnectionMapKey() = default;

    friend uint32 GetTypeHash(const FSocketConnectionMapKey &A);

    FString ToString(bool bExcludeChannel = false) const;
    bool IsIdenticalExcludingChannel(const FSocketConnectionMapKey &Other) const;

    const UE::Online::FAccountId &GetLocalUserId() const;
    const UE::Online::FAccountId &GetRemoteUserId() const;
    const FSocketId &GetSymmetricSocketId() const;
    const uint8 &GetSymmetricChannel() const;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(800251675, Redpoint::EOS::Networking::Socket, FSocketConnectionMapKey)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()