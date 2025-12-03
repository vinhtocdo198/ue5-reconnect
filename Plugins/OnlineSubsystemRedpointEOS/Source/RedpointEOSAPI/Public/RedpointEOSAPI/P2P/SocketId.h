// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

using FSocketId = FString;

EOS_P2P_SocketId *AllocateSocketId(
    ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator &Allocator,
    const FSocketId &SocketId);

}

#endif

REDPOINT_EOS_CODE_GUARD_END()