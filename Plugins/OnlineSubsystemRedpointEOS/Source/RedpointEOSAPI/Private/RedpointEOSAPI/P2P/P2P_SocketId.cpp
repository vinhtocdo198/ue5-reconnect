// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/P2P/SocketId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

EOS_P2P_SocketId *AllocateSocketId(
    ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator &Allocator,
    const FSocketId &SocketId)
{
    checkf(SocketId.Len() >= 0 && SocketId.Len() < EOS_P2P_SOCKETID_SOCKETNAME_SIZE, TEXT("SocketId is too large!"));
    auto SocketIdStr = Allocator.AsAnsi(SocketId);
    auto &SocketIdPtr = Allocator.Allocate<EOS_P2P_SocketId>();
    SocketIdPtr.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
    FMemory::Memcpy(
        &SocketIdPtr.SocketName[0],
        SocketIdStr,
        static_cast<size_t>(FMath::Min(SocketId.Len(), EOS_P2P_SOCKETID_SOCKETNAME_SIZE)));
    SocketIdPtr.SocketName[EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1] = '\0';
    return &SocketIdPtr;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()