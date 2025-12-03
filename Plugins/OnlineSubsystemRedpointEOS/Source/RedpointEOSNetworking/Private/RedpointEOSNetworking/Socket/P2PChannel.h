// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Networking::Socket
{

using FP2PChannelId = uint8;

constexpr auto P2PChannelIdModulo = (std::numeric_limits<FP2PChannelId>().max() + 1 - 1);
constexpr auto P2PChannelIdControl = (std::numeric_limits<FP2PChannelId>().max());

constexpr int8 P2PSocketNameMaxLength = (EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1);

extern const FString P2PDomainSuffix;

}

#endif

REDPOINT_EOS_CODE_GUARD_END()