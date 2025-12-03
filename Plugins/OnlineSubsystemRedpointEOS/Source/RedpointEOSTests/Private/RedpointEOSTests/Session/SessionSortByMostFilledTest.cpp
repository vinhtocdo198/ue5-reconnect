// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Session/CreateSessionChainedTask.h"
#include "RedpointEOSTests/Session/RegisterPlayerChainedTask.h"
#include "RedpointEOSTests/Session/SearchSessionsChainedTask.h"
#include "RedpointEOSTests/Session/StartSessionChainedTask.h"
#include "RedpointEOSTests/Session/UpdateSessionChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1028121421, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Session_SortByMostFilled,
    "Redpoint.EOS.Session.SortByMostFilled",
    // @note: This test is turned off until we get clarification from Epic as to whether this behaviour is expected to
    // work.
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter | EAutomationTestFlags::Disabled);

struct FSessionSpec
{
    FName SessionTestId;
    FString SearchGuid;
    int PlayersToRegister;
};

void FOnlineSubsystemEOS_Session_SortByMostFilled::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Tests::Session;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1, 1, 1}));

    // Define maximum number of players in a session.
    int MaxPlayers = 4;

    // Define our test global GUID, so we don't get sessions from concurrent tests.
    FString TestGlobalGuid = FGuid::NewGuid().ToString();
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("Running session test with attribute: TestGlobalGuid=%s"),
        *TestGlobalGuid);
    this->AddInfo(FString::Printf(TEXT("Running session test with attribute: TestGlobalGuid=%s"), *TestGlobalGuid));

    // Define our session specs.
    auto SessionSpecs = TArray<FSessionSpec>{
        FSessionSpec{FName(TEXT("Session1")), FGuid::NewGuid().ToString(), 4},
        FSessionSpec{FName(TEXT("Session2")), FGuid::NewGuid().ToString(), 3},
        FSessionSpec{FName(TEXT("Session3")), FGuid::NewGuid().ToString(), 2},
        FSessionSpec{FName(TEXT("Session4")), FGuid::NewGuid().ToString(), 1},
    };

    // Create our sessions.
    for (const auto &SessionSpec : SessionSpecs)
    {
        Executor->Then<FCreateSessionChainedTask>(
            0,
            SessionSpec.SessionTestId,
            FConfigureSessionSettings::CreateLambda(
                [SessionSpec, TestGlobalGuid, MaxPlayers](FOnlineSessionSettings &Settings) {
                    Settings.NumPublicConnections = MaxPlayers;
                    Settings.NumPrivateConnections = 0;
                    Settings.bShouldAdvertise = true;
                    Settings.bAllowJoinInProgress = false;
                    Settings.bIsLANMatch = false;
                    Settings.bIsDedicated = false;
                    Settings.bUsesStats = false;
                    Settings.bAllowInvites = false;
                    Settings.bUsesPresence = false;
                    Settings.bAllowJoinViaPresence = false;
                    Settings.bAllowJoinViaPresenceFriendsOnly = false;
                    Settings.bAntiCheatProtected = false;
                    Settings.bUseLobbiesIfAvailable = false;
                    Settings.bUseLobbiesVoiceChatIfAvailable = false;
                    Settings.SessionIdOverride = TEXT("");
                    Settings.BuildUniqueId = 0;
                    Settings.Settings.Empty();
                    Settings.MemberSettings.Empty();

                    Settings.Settings.Add(
                        FName(TEXT("TestGuid")),
                        FOnlineSessionSetting(SessionSpec.SearchGuid, EOnlineDataAdvertisementType::ViaOnlineService));
                    Settings.Settings.Add(
                        FName(TEXT("TestGlobalGuid")),
                        FOnlineSessionSetting(TestGlobalGuid, EOnlineDataAdvertisementType::ViaOnlineService));
                }));
    }

    // Register our players into the relevant sessions.
    for (const auto &SessionSpec : SessionSpecs)
    {
        for (int PlayerId = 0; PlayerId < SessionSpec.PlayersToRegister; PlayerId++)
        {
            Executor->Then<FRegisterPlayerChainedTask>(0, PlayerId, SessionSpec.SessionTestId);
        }
    }

    // Start our sessions.
    for (const auto &SessionSpec : SessionSpecs)
    {
        Executor->Then<FStartSessionChainedTask>(0, SessionSpec.SessionTestId);
    }

    // For sessions that aren't full, update them to allow join-in-progress again.
    for (const auto &SessionSpec : SessionSpecs)
    {
        if (SessionSpec.PlayersToRegister != MaxPlayers)
        {
            Executor->Then<FUpdateSessionChainedTask>(
                0,
                SessionSpec.SessionTestId,
                FConfigureSessionSettings::CreateLambda(
                    [SessionSpec, TestGlobalGuid](FOnlineSessionSettings &Settings) {
                        Settings.bAllowJoinInProgress = true;
                    }));
        }
    }

    // Search for our sessions in order of the most full.
    Executor->Then<FSearchSessionsChainedTask>(
        0,
        FConfigureSessionSearch::CreateLambda([TestGlobalGuid](FOnlineSessionSearch &Search) {
            // Get multiple search results.
            Search.MaxSearchResults = 50;

            // We want to sort with the "slots available" being as close to 0 as possible.
            Search.QuerySettings.SearchParams.Add(
                FName(TEXT("minslotsavailable")),
                FOnlineSessionSearchParam((int64)0, EOnlineComparisonOp::Near));

            // We only want sessions that are for this test.
            Search.QuerySettings.SearchParams.Add(
                FName(TEXT("TestGlobalGuid")),
                FOnlineSessionSearchParam(TestGlobalGuid, EOnlineComparisonOp::Equals));
        }),
        FEvaluateSessionSearch::CreateLambda(
            [SessionSpecs, TestGlobalGuid](
                const TestUtilities::FChainedTaskContextRef &Context,
                const FOnlineSessionSearch &Search) -> bool {
                Context->Assert().TestEqual(TEXT("Expected to get 3 search results."), Search.SearchResults.Num(), 3);

                // Generate a list of sessions by GUID, in order.
                TArray<FString> SessionGuidsInOrderOfSearchResults;
                for (const auto &SearchResult : Search.SearchResults)
                {
                    auto *TestGuidAttribute =
                        SearchResult.Session.SessionSettings.Settings.Find(FName(TEXT("TestGuid")));
                    if (Context->Assert().TestNotNull(
                            TEXT("Expected to have 'TestGuid' attribute on session."),
                            TestGuidAttribute))
                    {
                        FString TestGuidValue;
                        TestGuidAttribute->Data.GetValue(TestGuidValue);
                        Context->Assert().TestFalse(
                            TEXT("Expected 'TestGuid' attribute to not be empty."),
                            TestGuidValue.IsEmpty());
                        SessionGuidsInOrderOfSearchResults.Add(TestGuidValue);
                    }
                }

                // Make sure we got 3 search GUIDs.
                if (Context->Assert().TestEqual(
                        TEXT("Expected to get 3 search GUIDs out of the search results."),
                        SessionGuidsInOrderOfSearchResults.Num(),
                        3))
                {
                    // Make sure our results are in order and what we expect.
                    Context->Assert().TestFalse(
                        TEXT("Expected to not get full 'Session1' in search results (as it is full)."),
                        SessionGuidsInOrderOfSearchResults.Contains(SessionSpecs[0].SearchGuid));
                    Context->Assert().TestEqual(
                        TEXT("Expected 'Session2' to be in position 0 (as it is missing 1 player)."),
                        SessionGuidsInOrderOfSearchResults[0],
                        SessionSpecs[1].SearchGuid);
                    Context->Assert().TestEqual(
                        TEXT("Expected 'Session3' to be in position 1 (as it is missing 2 players)."),
                        SessionGuidsInOrderOfSearchResults[1],
                        SessionSpecs[2].SearchGuid);
                    Context->Assert().TestEqual(
                        TEXT("Expected 'Session4' to be in position 2 (as it is missing 3 players)."),
                        SessionGuidsInOrderOfSearchResults[2],
                        SessionSpecs[3].SearchGuid);
                }

                return true;
            }));

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()