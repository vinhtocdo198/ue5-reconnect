// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Misc/AutomationTest.h"
#include "RedpointMatchmaking/Openskill/Openskill.h"
#include "RedpointMatchmaking/Openskill/OpenskillRating.h"
#include "RedpointMatchmaking/Openskill/OpenskillUtils.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PlackettLuce,
    "Redpoint.Matchmaking.Openskill.PlackettLuce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PlackettLuce::RunTest(const FString &Parameters)
{
    TArray<FOpenskillRating> Team1;
    Team1.Add(FOpenskillRating());

    TArray<FOpenskillRating> Team2;
    Team2.Add(FOpenskillRating());
    Team2.Add(FOpenskillRating());

    TArray<FOpenskillRating> Team3;
    Team3.Add(FOpenskillRating());
    Team3.Add(FOpenskillRating());
    Team3.Add(FOpenskillRating());

    auto TestMatch =
        [this](const FString &What, TArray<TArray<FOpenskillRating>> In, TArray<TArray<FOpenskillRating>> Out) {
            TArray<FOpenskillTeam> InTeam;
            for (const auto &I : In)
            {
                InTeam.Add(FOpenskillTeam(I));
            }
            TArray<FOpenskillTeam> OutTeam = FOpenskill::Rate(InTeam);
            if (this->TestEqual(*FString::Printf(TEXT("%s (Len)"), *What), OutTeam.Num(), Out.Num()))
            {
                for (int32 I = 0; I < OutTeam.Num(); I++)
                {
                    if (this->TestEqual(
                            *FString::Printf(TEXT("%s [Team %d] (Len)"), *What, I),
                            OutTeam[I].GetPlayerRatings().Num(),
                            Out[I].Num()))
                    {
                        for (int32 P = 0; P < Out[I].Num(); P++)
                        {
                            this->TestEqual(
                                *FString::Printf(TEXT("%s [Team %d] [Player %d] (Mu)"), *What, I, P),
                                OutTeam[I].GetPlayerRatings()[P].GetMu(),
                                Out[I][P].GetMu());
                            this->TestEqual(
                                *FString::Printf(TEXT("%s [Team %d] [Player %d] (Sigma)"), *What, I, P),
                                OutTeam[I].GetPlayerRatings()[P].GetSigma(),
                                Out[I][P].GetSigma());
                        }
                    }
                }
            }
        };

    TestMatch(TEXT("Single"), {Team1}, {Team1});

    TestMatch(
        TEXT("2P FFA"),
        {Team1, Team1},
        {{FOpenskillRating(27.63523138347365, 8.065506316323548)},
         {FOpenskillRating(22.36476861652635, 8.065506316323548)}});

    TestMatch(
        TEXT("3P FFA"),
        {Team1, Team1, Team1},
        {{FOpenskillRating(27.868876552746237, 8.204837030780652)},
         {FOpenskillRating(25.717219138186557, 8.057829747583874)},
         {FOpenskillRating(21.413904309067206, 8.057829747583874)}});

    TestMatch(
        TEXT("4P FFA"),
        {Team1, Team1, Team1, Team1},
        {{FOpenskillRating(27.795084971874736, 8.263160757613477)},
         {FOpenskillRating(26.552824984374855, 8.179213704945203)},
         {FOpenskillRating(24.68943500312503, 8.083731307186588)},
         {FOpenskillRating(20.96265504062538, 8.083731307186588)}});

    TestMatch(
        TEXT("5P FFA"),
        {Team1, Team1, Team1, Team1, Team1},
        {{FOpenskillRating(27.666666666666668, 8.290556877154474)},
         {FOpenskillRating(26.833333333333332, 8.240145629781066)},
         {FOpenskillRating(25.72222222222222, 8.179996679645559)},
         {FOpenskillRating(24.055555555555557, 8.111796013701358)},
         {FOpenskillRating(20.72222222222222, 8.111796013701358)}});

    TestMatch(
        TEXT("3 Different Sized Teams"),
        {Team3, Team1, Team2},
        {{FOpenskillRating(25.939870821784513, 8.247641552260456),
          FOpenskillRating(25.939870821784513, 8.247641552260456),
          FOpenskillRating(25.939870821784513, 8.247641552260456)},
         {FOpenskillRating(27.21366020491262, 8.274321317985242)},
         {FOpenskillRating(21.84646897330287, 8.213058173195341),
          FOpenskillRating(21.84646897330287, 8.213058173195341)}});

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PlackettLuce_Series,
    "Redpoint.Matchmaking.Openskill.PlackettLuce_Series",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PlackettLuce_Series::RunTest(const FString &Parameters)
{
    FOpenskillRating P00;
    FOpenskillRating P10;
    FOpenskillRating P20;
    FOpenskillRating P30;
    FOpenskillRating P40;

    TArray<FOpenskillTeam> OutTeam = FOpenskill::Rate(
        TArray<FOpenskillTeam>{
            FOpenskillTeam({P00}),
            FOpenskillTeam({P10}),
            FOpenskillTeam({P20}),
            FOpenskillTeam({P30}),
            FOpenskillTeam({P40})},
        TArray<double>{-9.0, -7.0, -7.0, -5.0, -5.0});

    FOpenskillRating P01 = OutTeam[0].GetPlayerRatings()[0];
    FOpenskillRating P11 = OutTeam[1].GetPlayerRatings()[0];
    FOpenskillRating P21 = OutTeam[2].GetPlayerRatings()[0];
    FOpenskillRating P31 = OutTeam[3].GetPlayerRatings()[0];
    FOpenskillRating P41 = OutTeam[4].GetPlayerRatings()[0];

    FOpenskillRating P02 = P01;
    FOpenskillRating P32 = P31;

    OutTeam = FOpenskill::Rate(
        TArray<FOpenskillTeam>{FOpenskillTeam({P41}), FOpenskillTeam({P21}), FOpenskillTeam({P11})},
        TArray<double>{-9.0, -5.0, -5.0});

    FOpenskillRating P42 = OutTeam[0].GetPlayerRatings()[0];
    FOpenskillRating P22 = OutTeam[1].GetPlayerRatings()[0];
    FOpenskillRating P12 = OutTeam[2].GetPlayerRatings()[0];

    FOpenskillRating P43 = P42;

    OutTeam = FOpenskill::Rate(
        TArray<FOpenskillTeam>{
            FOpenskillTeam({P32}),
            FOpenskillTeam({P12}),
            FOpenskillTeam({P22}),
            FOpenskillTeam({P02})},
        TArray<double>{-9.0, -9.0, -7.0, -7.0});

    FOpenskillRating P33 = OutTeam[0].GetPlayerRatings()[0];
    FOpenskillRating P13 = OutTeam[1].GetPlayerRatings()[0];
    FOpenskillRating P23 = OutTeam[2].GetPlayerRatings()[0];
    FOpenskillRating P03 = OutTeam[3].GetPlayerRatings()[0];

    auto TestRating = [this](const FString &What, FOpenskillRating Actual, double ExpectedMu, double ExpectedSigma) {
        this->TestEqual(*FString::Printf(TEXT("%s (Mu)"), *What), Actual.GetMu(), ExpectedMu);
        this->TestEqual(*FString::Printf(TEXT("%s (Sigma)"), *What), Actual.GetSigma(), ExpectedSigma);
    };

    TestRating(TEXT("P03"), P03, 26.353761103, 8.11102706);
    TestRating(TEXT("P13"), P13, 24.618479789, 7.90533551);
    TestRating(TEXT("P23"), P23, 23.065819512, 7.822005595);
    TestRating(TEXT("P33"), P33, 24.476332403, 8.106111471);
    TestRating(TEXT("P43"), P43, 26.385499685, 8.054090809);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_1v1,
    "Redpoint.Matchmaking.Openskill.PredictWinners.1v1",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_1v1::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;
    FOpenskillRating A2(32.444, 5.123);
    FOpenskillRating B1(73.381, 1.421);
    FOpenskillRating B2(25.188, 6.211);

    TArray<FOpenskillRating> Team1{A1, A2};
    TArray<FOpenskillRating> Team2{B1, B2};

    TArray<double> Predictions = FOpenskill::PredictWinners({Team1, Team2});
    TestEqual(TEXT("Team 1 prediction"), Predictions[0], 0.34641823958165474);
    TestEqual(TEXT("Team 2 prediction"), Predictions[1], 0.6535817604183453);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_Asymmetric,
    "Redpoint.Matchmaking.Openskill.PredictWinners.Asymmetric",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_Asymmetric::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;
    FOpenskillRating A2(32.444, 5.123);
    FOpenskillRating B1(73.381, 1.421);
    FOpenskillRating B2(25.188, 6.211);

    TArray<FOpenskillRating> Team1{A1, A2};
    TArray<FOpenskillRating> Team2{B1, B2};

    TArray<double> Predictions =
        FOpenskill::PredictWinners({Team1, Team2, TArray<FOpenskillRating>{A2}, TArray<FOpenskillRating>{B2}});
    TestEqual(TEXT("Team 1 prediction"), Predictions[0], 0.2613515941642222);
    TestEqual(TEXT("Team 2 prediction"), Predictions[1], 0.41117430943389155);
    TestEqual(TEXT("A2 prediction"), Predictions[2], 0.1750905983112395);
    TestEqual(TEXT("B2 prediction"), Predictions[3], 0.15238349809064686);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_3P_FFA,
    "Redpoint.Matchmaking.Openskill.PredictWinners.3P_FFA",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_3P_FFA::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;

    TArray<double> Predictions = FOpenskill::PredictWinners(
        {TArray<FOpenskillRating>{A1}, TArray<FOpenskillRating>{A1}, TArray<FOpenskillRating>{A1}});
    TestEqual(TEXT("P1"), Predictions[0], 0.333333333333);
    TestEqual(TEXT("P2"), Predictions[1], 0.333333333333);
    TestEqual(TEXT("P3"), Predictions[2], 0.333333333333);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_4P_FFA,
    "Redpoint.Matchmaking.Openskill.PredictWinners.4P_FFA",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_4P_FFA::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;

    TArray<double> Predictions = FOpenskill::PredictWinners(
        {TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1}});
    TestEqual(TEXT("P1"), Predictions[0], 0.25);
    TestEqual(TEXT("P2"), Predictions[1], 0.25);
    TestEqual(TEXT("P3"), Predictions[2], 0.25);
    TestEqual(TEXT("P4"), Predictions[3], 0.25);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_4P_VaryingSkill,
    "Redpoint.Matchmaking.Openskill.PredictWinners.4P_VaryingSkill",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_4P_VaryingSkill::RunTest(const FString &Parameters)
{
    FOpenskillRating R1(1, 0.1);
    FOpenskillRating R2(2, 0.1);
    FOpenskillRating R3(3, 0.1);
    FOpenskillRating R4(4, 0.1);

    TArray<double> Predictions = FOpenskill::PredictWinners(
        {TArray<FOpenskillRating>{R1},
         TArray<FOpenskillRating>{R2},
         TArray<FOpenskillRating>{R3},
         TArray<FOpenskillRating>{R4}});
    TestEqual(TEXT("R1"), Predictions[0], 0.2028051110543726);
    TestEqual(TEXT("R2"), Predictions[1], 0.23419421333676907);
    TestEqual(TEXT("R3"), Predictions[2], 0.2658057866632309);
    TestEqual(TEXT("R4"), Predictions[3], 0.29719488894562746);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_5P_FFA,
    "Redpoint.Matchmaking.Openskill.PredictWinners.5P_FFA",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_5P_FFA::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;

    TArray<double> Predictions = FOpenskill::PredictWinners(
        {TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1}});
    TestEqual(TEXT("P1"), Predictions[0], 0.2);
    TestEqual(TEXT("P2"), Predictions[1], 0.2);
    TestEqual(TEXT("P3"), Predictions[2], 0.2);
    TestEqual(TEXT("P4"), Predictions[3], 0.2);
    TestEqual(TEXT("P5"), Predictions[4], 0.2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictWinners_5P_Imposter,
    "Redpoint.Matchmaking.Openskill.PredictWinners.5P_Imposter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictWinners_5P_Imposter::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;
    FOpenskillRating A2(32.444, 5.123);

    TArray<double> Predictions = FOpenskill::PredictWinners(
        {TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A1},
         TArray<FOpenskillRating>{A2},
         TArray<FOpenskillRating>{A1}});
    TestEqual(TEXT("R1"), Predictions[0], 0.196037416522638);
    TestEqual(TEXT("R2"), Predictions[1], 0.196037416522638);
    TestEqual(TEXT("R3"), Predictions[2], 0.196037416522638);
    TestEqual(TEXT("R4"), Predictions[3], 0.21585034503812);
    TestEqual(TEXT("R5"), Predictions[4], 0.196037416522638);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_PredictDraw,
    "Redpoint.Matchmaking.Openskill.PredictDraw",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOpenskill_PredictDraw::RunTest(const FString &Parameters)
{
    FOpenskillRating A1;
    FOpenskillRating A2(32.444, 1.123);
    FOpenskillRating B1(35.881, 0.0001);
    FOpenskillRating B2(25.188, 1.421);

    TArray<FOpenskillRating> Team1{A1, A2};
    TArray<FOpenskillRating> Team2{B1, B2};

    TestEqual(TEXT("Draw chance 1"), FOpenskill::PredictDraw({Team1, Team2}), 0.1260703143635969);
    TestEqual(
        TEXT("Draw chance 2"),
        FOpenskill::PredictDraw(
            {Team1, Team2, TArray<FOpenskillRating>{A1}, TArray<FOpenskillRating>{A2}, TArray<FOpenskillRating>{B1}}),
        0.04322122887507519);
    TestEqual(
        TEXT("Draw chance 3"),
        FOpenskill::PredictDraw({TArray<FOpenskillRating>{B1}, TArray<FOpenskillRating>{B1}}),
        1.0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpenskill_Unwind,
    "Redpoint.Matchmaking.Openskill.Unwind",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

template <typename TType>
void TestListEqual(
    FAutomationTestBase *T,
    const FString &What,
    const TArray<TType> &Actual,
    const TArray<TType> &Expected)
{
    if (T->TestEqual(*FString::Printf(TEXT("%s length"), *What), Actual.Num(), Expected.Num()))
    {
        for (int32 i = 0; i < Actual.Num(); i++)
        {
            T->TestEqual(*FString::Printf(TEXT("%s #%d"), *What, i), Actual[i], Expected[i]);
        }
    }
}

bool FOpenskill_Unwind::RunTest(const FString &Parameters)
{
    TArray<FString> Elements = {TEXT("a"), TEXT("b"), TEXT("c"), TEXT("d"), TEXT("e")};
    TArray<int32> Ranks = {8, 10, 5, 10, 4};

    TArray<int32> OriginalIndexes;
    TArray<FString> SortedElements;
    TArray<int32> SortedRanks;
    Openskill::Unwind<FString>(Ranks, Elements, OriginalIndexes, SortedElements, SortedRanks);

    TestListEqual(
        this,
        TEXT("Sorted elements matches"),
        SortedElements,
        TArray<FString>{TEXT("e"), TEXT("c"), TEXT("a"), TEXT("d"), TEXT("b")});
    TestListEqual(this, TEXT("Tenet matches"), OriginalIndexes, TArray<int32>{4, 2, 0, 3, 1});
    TestListEqual(this, TEXT("Sorted ranks matches"), SortedRanks, TArray<int32>{4, 5, 8, 10, 10});

    TArray<int32> _;
    TArray<FString> OriginalElements;
    TArray<int32> __;
    Openskill::Unwind<FString>(OriginalIndexes, SortedElements, _, OriginalElements, __);

    TestListEqual(
        this,
        TEXT("Original matches"),
        OriginalElements,
        TArray<FString>{TEXT("a"), TEXT("b"), TEXT("c"), TEXT("d"), TEXT("e")});

    return true;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()