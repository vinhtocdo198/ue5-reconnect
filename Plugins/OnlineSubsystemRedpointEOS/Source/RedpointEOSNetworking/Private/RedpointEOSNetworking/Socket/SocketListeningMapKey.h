// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/P2P/SocketId.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3549223374, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::API::P2P;

struct FSocketListeningMapKey
{
private:
    const UE::Online::FAccountId LocalUserId;
    const FSocketId SymmetricSocketId;

public:
    FSocketListeningMapKey(const UE::Online::FAccountId InLocalUserId, const FString InSymmetricSocketId);
    FSocketListeningMapKey(const FSocketListeningMapKey &Other);
    ~FSocketListeningMapKey() = default;

    friend uint32 GetTypeHash(const FSocketListeningMapKey &A);

    FString ToString() const;

    const UE::Online::FAccountId &GetLocalUserId() const;
    const FSocketId &GetSymmetricSocketId() const;
};

}

namespace Redpoint::EOS::Networking::Socket
{
REDPOINT_EOS_FILE_NS_EXPORT(3549223374, Redpoint::EOS::Networking::Socket, FSocketListeningMapKey)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()