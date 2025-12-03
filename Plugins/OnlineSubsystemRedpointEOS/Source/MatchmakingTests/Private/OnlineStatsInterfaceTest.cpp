// Copyright June Rhodes. All Rights Reserved.

#include "OnlineStatsInterfaceTest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

bool bAlternateStatsArrangement = false;

void FOnlineStatsInterfaceTest::QueryStats(
    const FUniqueNetIdRef LocalUserId,
    const TArray<FUniqueNetIdRef> &StatUsers,
    const TArray<FString> &StatNames,
    const FOnlineStatsQueryUsersStatsComplete &Delegate)
{
    TArray<TSharedRef<const FOnlineStatsUserStats>> UsersStatsResult;
    for (const auto &UserId : StatUsers)
    {
        auto UserIdTest = StaticCastSharedRef<const FUniqueNetIdMMTest>(UserId);

        TMap<FString, FOnlineStatValue> Stats;
        for (const auto &Name : StatNames)
        {
            if (Name.EndsWith("_mu"))
            {
                if (bAlternateStatsArrangement)
                {
                    Stats.Add(Name, 50.0 * ((UserIdTest->GetIntValue() / 2) % 2));
                }
                else
                {
                    Stats.Add(Name, 50.0 * (UserIdTest->GetIntValue() % 2));
                }
            }
            if (Name.EndsWith("_sigma"))
            {
                Stats.Add(Name, 0.1);
            }
        }

        UsersStatsResult.Add(MakeShared<FOnlineStatsUserStats>(UserId, Stats));
    }
    Delegate.ExecuteIfBound(FOnlineError::Success(), UsersStatsResult);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()