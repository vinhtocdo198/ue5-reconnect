// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingFollowRequest.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FVariantData FMatchmakingFollowRequest::SerializeFollowRequest(
    const FOnlineLobbyId &InLobbyId,
    int32 InRequestId,
    int64 InLobbyCreatedUnixTimestamp,
    const FString &InStateHash)
{
    check(InRequestId > 0);
    check(!InStateHash.IsEmpty());

    return FVariantData(FString::Printf(
        TEXT("%d|%lld|%s|%s"),
        InRequestId,
        InLobbyCreatedUnixTimestamp,
        *InStateHash,
        *InLobbyId.ToString()));
}

static TArray<FString> UnpackString(const FString &InStr, int32 InTargetElements)
{
    TArray<FString> Results;
    FString Current = InStr;
    for (int32 i = 0; i < InTargetElements - 1; i++)
    {
        int32 SeperatorIndex = Current.Find(TEXT("|"));
        if (SeperatorIndex == -1)
        {
            return TArray<FString>();
        }

        Results.Add(Current.Mid(0, SeperatorIndex));
        Current = Current.Mid(SeperatorIndex + 1);
    }
    Results.Add(Current);
    return Results;
}

bool FMatchmakingFollowRequest::TryDeserializeFollowRequest(
    const TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> &InOnlineLobby,
    const FVariantData &InFollowRequest,
    TSharedPtr<const FOnlineLobbyId> &OutLobbyId,
    int32 &OutRequestId,
    int64 &OutLobbyCreatedUnixTimestamp,
    FString &OutStateHash)
{
    if (InFollowRequest.GetType() != EOnlineKeyValuePairDataType::String)
    {
        return false;
    }

    FString FollowRequestStr;
    InFollowRequest.GetValue(FollowRequestStr);

    TArray<FString> Unpacked = UnpackString(FollowRequestStr, 4);
    if (Unpacked.Num() != 4)
    {
        return false;
    }

    FString SerializedRequestId = Unpacked[0];
    FString SerializedCreatedUnixTimestamp = Unpacked[1];
    FString SerializedStateHash = Unpacked[2];
    FString SerializedLobbyId = Unpacked[3];

    OutLobbyId = InOnlineLobby->ParseSerializedLobbyId(SerializedLobbyId);
    OutRequestId = FCString::Atoi(*SerializedRequestId);
    OutLobbyCreatedUnixTimestamp = FCString::Atoi64(*SerializedCreatedUnixTimestamp);
    OutStateHash = SerializedStateHash;

    return OutLobbyId.IsValid() && OutRequestId > 0 && OutLobbyCreatedUnixTimestamp > 0 && !OutStateHash.IsEmpty();
}

#endif

REDPOINT_EOS_CODE_GUARD_END()