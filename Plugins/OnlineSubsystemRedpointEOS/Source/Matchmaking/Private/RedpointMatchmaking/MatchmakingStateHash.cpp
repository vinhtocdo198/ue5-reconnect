// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingStateHash.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Dom/JsonObject.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FString FMatchmakingStateHash::ComputeStateHashForLobby(
    IOnlineLobby &InOnlineLobby,
    const FUniqueNetId &InUserId,
    const FOnlineLobbyId &InLobbyId)
{
    TArray<TSharedPtr<FJsonValue>> StateHashComputationDebugJson;

    uint32 HashCode = 0;

    if (UE_LOG_ACTIVE(LogMatchmaking, Verbose))
    {
        auto JsonEntry = MakeShared<FJsonObject>();
        JsonEntry->SetStringField("Message", "Initializing state hash");
        JsonEntry->SetStringField("UpdatedStateHash", FString::Printf(TEXT("%u"), HashCode));
        StateHashComputationDebugJson.Add(MakeShared<FJsonValueObject>(JsonEntry));
    }

    int32 MemberCount = 0;
    if (!InOnlineLobby.GetMemberCount(InUserId, InLobbyId, MemberCount))
    {
        UE_LOG(
            LogMatchmaking,
            Error,
            TEXT("%s: IOnlineLobby API failed to return GetMemberCount for lobby %s."),
            *InUserId.ToString(),
            *InLobbyId.ToString());
        return TEXT("");
    }
    HashCode ^= GetTypeHash(MemberCount);
    if (UE_LOG_ACTIVE(LogMatchmaking, Verbose))
    {
        auto JsonEntry = MakeShared<FJsonObject>();
        JsonEntry->SetStringField("Message", "Added member count to state hash");
        JsonEntry->SetNumberField("MemberCount", (double)MemberCount);
        JsonEntry->SetStringField("UpdatedStateHash", FString::Printf(TEXT("%u"), HashCode));
        StateHashComputationDebugJson.Add(MakeShared<FJsonValueObject>(JsonEntry));
    }

    TArray<TSharedRef<const FUniqueNetId>> MemberIds;
    for (int32 i = 0; i < MemberCount; i++)
    {
        TSharedPtr<const FUniqueNetId> MemberId;
        if (!InOnlineLobby.GetMemberUserId(InUserId, InLobbyId, i, MemberId))
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%s: IOnlineLobby API failed to return GetMemberUserId at index %d for lobby %s."),
                *InUserId.ToString(),
                i,
                *InLobbyId.ToString());
            return TEXT("");
        }
        if (!MemberId.IsValid())
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%s: IOnlineLobby API call to GetMemberUserId at index %d did not return a valid member ID for "
                     "lobby %s."),
                *InUserId.ToString(),
                i,
                *InLobbyId.ToString());
            return TEXT("");
        }
        MemberIds.Add(MemberId.ToSharedRef());
    }

    MemberIds.StableSort([](const TSharedRef<const FUniqueNetId> &LHS, const TSharedRef<const FUniqueNetId> &RHS) {
        return GetTypeHash(*LHS) < GetTypeHash(*RHS);
    });

    for (const auto &MemberId : MemberIds)
    {
        HashCode ^= GetTypeHash(*MemberId);
        if (UE_LOG_ACTIVE(LogMatchmaking, Verbose))
        {
            auto JsonEntry = MakeShared<FJsonObject>();
            JsonEntry->SetStringField("Message", "Added member ID to state hash");
            JsonEntry->SetStringField("MemberId", MemberId->ToString());
            JsonEntry->SetStringField("UpdatedStateHash", FString::Printf(TEXT("%u"), HashCode));
            StateHashComputationDebugJson.Add(MakeShared<FJsonValueObject>(JsonEntry));
        }

        FVariantData TeamValue;
        FString TeamKey = FString::Printf(TEXT("%s.%s"), *MemberId->ToString(), MatchmakingMemberMetadataTeam);
        if (!InOnlineLobby.GetLobbyMetadataValue(InUserId, InLobbyId, TeamKey, TeamValue) ||
            TeamValue.GetType() != EOnlineKeyValuePairDataType::Int64)
        {
            UE_LOG(
                LogMatchmaking,
                Error,
                TEXT("%s: IOnlineLobby API call to GetLobbyMetadataValue did not return a value for attribute '%s' for "
                     "lobby %s."),
                *InUserId.ToString(),
                *TeamKey,
                *InLobbyId.ToString());
            return TEXT("");
        }

        int64 Team;
        TeamValue.GetValue(Team);

        HashCode ^= GetTypeHash(Team);
        if (UE_LOG_ACTIVE(LogMatchmaking, Verbose))
        {
            auto JsonEntry = MakeShared<FJsonObject>();
            JsonEntry->SetStringField("Message", "Added member's team number to state hash");
            JsonEntry->SetStringField("MemberId", MemberId->ToString());
            JsonEntry->SetNumberField("TeamNumber", (double)Team);
            JsonEntry->SetStringField("UpdatedStateHash", FString::Printf(TEXT("%u"), HashCode));
            StateHashComputationDebugJson.Add(MakeShared<FJsonValueObject>(JsonEntry));
        }
    }

    if (UE_LOG_ACTIVE(LogMatchmaking, Verbose))
    {
        FString JsonString;
        auto JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonString);
        FJsonSerializer::Serialize(StateHashComputationDebugJson, JsonWriter);
        JsonWriter->Close();

        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%s: Computed state hash %u for lobby %s, steps were: %s"),
            *InUserId.ToString(),
            HashCode,
            *InLobbyId.ToString(),
            *JsonString);
    }
    return FString::Printf(TEXT("%u"), HashCode);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()