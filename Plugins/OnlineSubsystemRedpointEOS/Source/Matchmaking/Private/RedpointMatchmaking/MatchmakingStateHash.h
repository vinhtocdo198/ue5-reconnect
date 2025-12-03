// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingStateHash
{
public:
    /**
     * Computes a hash of the current matchmaking lobby's state for follow requests. This is used by recipients
     * to ensure that the requester is not acting on stale information.
     */
    static FString ComputeStateHashForLobby(
        IOnlineLobby &InOnlineLobby,
        const FUniqueNetId &InUserId,
        const FOnlineLobbyId &InLobbyId);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()