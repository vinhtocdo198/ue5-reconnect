// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Utility/ParseTeamCapacities.h"

#include "RedpointMatchmaking/MatchmakingLog.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1984048542, Redpoint::Matchmaking::Utility)
{

TOptional<TArray<int32>> ParseTeamCapacities(const FString &TeamCapacities)
{
    TArray<int32> GeneratedTeamCapacities;
    if (TeamCapacities.Contains(TEXT("x")))
    {
        TArray<FString> MultiplyComponents;
        TeamCapacities.ParseIntoArray(MultiplyComponents, TEXT("x"));
        if (MultiplyComponents.Num() != 2)
        {
            UE_LOG(LogMatchmaking, Error, TEXT("Can't start matchmaking because the team configuration is invalid!"));
            return TOptional<TArray<int32>>();
        }
        int32 X = FCString::Atoi(*MultiplyComponents[0]);
        int32 Y = FCString::Atoi(*MultiplyComponents[1]);
        if (X <= 0 || Y <= 0)
        {
            UE_LOG(LogMatchmaking, Error, TEXT("Can't start matchmaking because the team configuration is invalid!"));
            return TOptional<TArray<int32>>();
        }
        for (int i = 0; i < Y; i++)
        {
            GeneratedTeamCapacities.Add(X);
        }
    }
    else
    {
        TArray<FString> TeamComponents;
        TeamCapacities.ParseIntoArray(TeamComponents, TEXT("v"));
        if (TeamComponents.Num() == 0)
        {
            UE_LOG(LogMatchmaking, Error, TEXT("Can't start matchmaking because the team configuration is invalid!"));
            return TOptional<TArray<int32>>();
        }
        for (const auto &CapacityString : TeamComponents)
        {
            int32 Capacity = FCString::Atoi(*CapacityString);
            if (Capacity == 0)
            {
                UE_LOG(LogMatchmaking, Warning, TEXT("Team capacity of 0 will be ignored!"));
                continue;
            }
            GeneratedTeamCapacities.Add(Capacity);
        }
    }
    return GeneratedTeamCapacities;
}

}

REDPOINT_EOS_CODE_GUARD_END()