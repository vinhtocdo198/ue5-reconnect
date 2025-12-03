// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Session/CreateSessionChainedTask.h"
#include "RedpointEOSTests/Session/FindSessionByIdChainedTask.h"
#include "RedpointEOSTests/Session/JoinSessionChainedTask.h"
#include "RedpointEOSTests/Session/RegisterPlayerChainedTask.h"
#include "RedpointEOSTests/Session/SearchSessionsChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(910620073, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Session_FindPrivateById,
    "Redpoint.EOS.Session.FindPrivateById",
    // @note: This test is turned off until we get clarification from Epic as to whether this behaviour is expected to
    // work.
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter | EAutomationTestFlags::Disabled);

void FOnlineSubsystemEOS_Session_FindPrivateById::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Tests::Session;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));
    TSharedRef<FOnlineSessionSearchResult> FoundSearchResult = MakeShared<FOnlineSessionSearchResult>();

    // Define maximum number of players in a session.
    int MaxPlayers = 2;

    // Create our private session.
    Executor->Then<FCreateSessionChainedTask>(
        0,
        NAME_GameSession,
        FConfigureSessionSettings::CreateLambda([MaxPlayers](FOnlineSessionSettings &Settings) {
            Settings.NumPublicConnections = MaxPlayers;
            Settings.NumPrivateConnections = 0;
            Settings.bShouldAdvertise = false;
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
        }));
    Executor->Then<FRegisterPlayerChainedTask>(0, 0, NAME_GameSession);

    // Find the private session by it's ID.
    Executor->Then<FFindSessionByIdChainedTask>(1, 0, NAME_GameSession, FoundSearchResult);

    // Join the session by the search result we got.
    Executor->Then<FJoinSessionChainedTask>(1, NAME_GameSession, FoundSearchResult);

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()