// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineStatsInterface.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMatchmaking_SubmitMatchResults,
    "Redpoint.Matchmaking.SubmitMatchResults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

class FOnlineStatsInterfaceSubmit : public IOnlineStats,
                                    public TSharedFromThis<FOnlineStatsInterfaceSubmit, ESPMode::ThreadSafe>
{
private:
    FMatchmaking_SubmitMatchResults *T;

public:
    FOnlineStatsInterfaceSubmit(FMatchmaking_SubmitMatchResults *InT)
        : T(InT)
        , bUpdateWasCalled(false) {};

    bool bUpdateWasCalled;

    virtual void QueryStats(
        const FUniqueNetIdRef LocalUserId,
        const FUniqueNetIdRef StatsUser,
        const FOnlineStatsQueryUserStatsComplete &Delegate) override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::QueryStats, return;);
    virtual void QueryStats(
        const FUniqueNetIdRef LocalUserId,
        const TArray<FUniqueNetIdRef> &StatUsers,
        const TArray<FString> &StatNames,
        const FOnlineStatsQueryUsersStatsComplete &Delegate) override;
    virtual TSharedPtr<const FOnlineStatsUserStats> GetStats(const FUniqueNetIdRef StatsUserId) const override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::GetStats, return nullptr;);
    virtual void UpdateStats(
        const FUniqueNetIdRef LocalUserId,
        const TArray<FOnlineStatsUserUpdatedStats> &UpdatedUserStats,
        const FOnlineStatsUpdateStatsComplete &Delegate) override;
#if !UE_BUILD_SHIPPING
    virtual void ResetStats(const FUniqueNetIdRef StatsUserId) override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::ResetStats, return;);
#endif // !UE_BUILD_SHIPPING
};

void FOnlineStatsInterfaceSubmit::QueryStats(
    const FUniqueNetIdRef LocalUserId,
    const TArray<FUniqueNetIdRef> &StatUsers,
    const TArray<FString> &StatNames,
    const FOnlineStatsQueryUsersStatsComplete &Delegate)
{
    TMap<FString, double> Values = {
        {"0_mu", 25.0},
        {"0_sigma", 25.0 / 3.0},
        {"1_mu", 25.0},
        {"1_sigma", 25.0 / 3.0},
        // 2_ missing to test when user has no existing stat.
        {"3_mu", 25.0},
        {"3_sigma", 25.0 / 3.0},
        {"4_mu", 25.0},
        {"4_sigma", 25.0 / 3.0},
        {"5_mu", 25.0},
        {"5_sigma", 25.0 / 3.0},
    };

    TArray<TSharedRef<const FOnlineStatsUserStats>> UsersStatsResult;
    for (const auto &UserId : StatUsers)
    {
        auto UserIdTest = StaticCastSharedRef<const FUniqueNetIdMMTest>(UserId);

        TMap<FString, FOnlineStatValue> Stats;
        for (const auto &Name : StatNames)
        {
            if (Name.EndsWith("_mu"))
            {
                FString Key = FString::Printf(TEXT("%d_mu"), UserIdTest->GetIntValue());
                if (Values.Contains(Key))
                {
                    Stats.Add(Name, Values[Key]);
                }
            }
            if (Name.EndsWith("_sigma"))
            {
                FString Key = FString::Printf(TEXT("%d_sigma"), UserIdTest->GetIntValue());
                if (Values.Contains(Key))
                {
                    Stats.Add(Name, Values[Key]);
                }
            }
        }

        UsersStatsResult.Add(MakeShared<FOnlineStatsUserStats>(UserId, Stats));
    }
    Delegate.ExecuteIfBound(FOnlineError::Success(), UsersStatsResult);
}

void FOnlineStatsInterfaceSubmit::UpdateStats(
    const FUniqueNetIdRef LocalUserId,
    const TArray<FOnlineStatsUserUpdatedStats> &UpdatedUserStats,
    const FOnlineStatsUpdateStatsComplete &Delegate)
{
    TMap<FString, double> Values = {
        {"0_mu", 27.21366020491262},
        {"0_sigma", 8.274321317985242},
        {"1_mu", 21.84646897330287},
        {"1_sigma", 8.213058173195341},
        {"2_mu", 21.84646897330287},
        {"2_sigma", 8.213058173195341},
        {"3_mu", 25.939870821784513},
        {"3_sigma", 8.247641552260456},
        {"4_mu", 25.939870821784513},
        {"4_sigma", 8.247641552260456},
        {"5_mu", 25.939870821784513},
        {"5_sigma", 8.247641552260456},
    };
    TArray<bool> UserSeen = {false, false, false, false, false, false};

    for (const auto &UpdatedStat : UpdatedUserStats)
    {
        const auto &UserId = StaticCastSharedRef<const FUniqueNetIdMMTest>(UpdatedStat.Account);

        double Mu, Sigma;
        UpdatedStat.Stats["mmr_mu"].GetValue().GetValue(Mu);
        UpdatedStat.Stats["mmr_sigma"].GetValue().GetValue(Sigma);

        T->TestEqual(
            FString::Printf(TEXT("User %d Mu"), UserId->GetIntValue()),
            Mu,
            Values[FString::Printf(TEXT("%d_mu"), UserId->GetIntValue())]);
        T->TestEqual(
            FString::Printf(TEXT("User %d Sigma"), UserId->GetIntValue()),
            Sigma,
            Values[FString::Printf(TEXT("%d_sigma"), UserId->GetIntValue())]);
        UserSeen[UserId->GetIntValue()] = true;
    }
    for (int32 i = 0; i < UserSeen.Num(); i++)
    {
        T->TestTrue(FString::Printf(TEXT("User %d Seen"), i), UserSeen[i]);
    }

    this->bUpdateWasCalled = true;
}

bool FMatchmaking_SubmitMatchResults::RunTest(const FString &Parameters)
{
    FMatchmakingEngineResponse OriginalResponse{TArray<FMatchmakingEngineResponseTeam>{
        // Team 0
        FMatchmakingEngineResponseTeam{TArray<FMatchmakingEngineResponseTeamSlot>{FMatchmakingEngineResponseTeamSlot{
            EMatchmakingEngineResponseTeamSlotType::User,
            MakeShared<FUniqueNetIdMMTest>(0)}}},
        // Team 1
        FMatchmakingEngineResponseTeam{TArray<FMatchmakingEngineResponseTeamSlot>{
            {EMatchmakingEngineResponseTeamSlotType::User, MakeShared<FUniqueNetIdMMTest>(1)},
            {EMatchmakingEngineResponseTeamSlotType::User, MakeShared<FUniqueNetIdMMTest>(2)}}},
        // Team 2
        FMatchmakingEngineResponseTeam{TArray<FMatchmakingEngineResponseTeamSlot>{
            {EMatchmakingEngineResponseTeamSlotType::User, MakeShared<FUniqueNetIdMMTest>(3)},
            {EMatchmakingEngineResponseTeamSlotType::User, MakeShared<FUniqueNetIdMMTest>(4)},
            {EMatchmakingEngineResponseTeamSlotType::User, MakeShared<FUniqueNetIdMMTest>(5)}}},
    }};

    auto Stats = MakeShared<FOnlineStatsInterfaceSubmit, ESPMode::ThreadSafe>(this);
    IMatchmakingEngine::ForciblyCreateInstanceForUnitTests()->SubmitMatchResults(
        MakeShared<FUniqueNetIdMMTest>(0),
        Stats,
        TEXT("mmr"),
        OriginalResponse,
        TArray<double>{50.0, 10.0, 100.0},
        FMatchmakingEngineSubmitComplete());
    TestTrue(TEXT("UpdateStats was called"), Stats->bUpdateWasCalled);

    return true;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()