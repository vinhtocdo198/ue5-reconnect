// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingSerialization
{
public:
    static FString SerializeResult(const TArray<FMatchmakingEngineResponseTeam> &InTeams);
    static TArray<FMatchmakingEngineResponseTeam> DeserializeResult(
        const TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> &InOnlineIdentity,
        const FString &InValue);

    static void SerializeResultToTransaction(
        const TArray<FMatchmakingEngineResponseTeam> &InTeams,
        const FString &Prefix,
        TSharedPtr<FOnlineLobbyTransaction> &Txn);
    static TArray<FMatchmakingEngineResponseTeam> DeserializeResultFromLobbyData(
        const TSharedRef<IOnlineLobby, ESPMode::ThreadSafe> &InOnlineLobby,
        const TSharedRef<IOnlineIdentity, ESPMode::ThreadSafe> &InOnlineIdentity,
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FString &Prefix);

    static void DumpTeamArrayState(const TArray<FMatchmakingEngineResponseTeam> &Data, const FString &Context);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()