// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3589726765, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyPassword,
    "Redpoint.EOS.RoomSystem.LobbyPassword",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyPassword::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Id;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create a shared pointer to hold the room ID.
    TSharedRef<FRoomIdPtr> RoomId = MakeShared<FRoomIdPtr>();
    FGuid SearchGuid = FGuid::NewGuid();

    // Create a lobby with the room system.
    Executor->Then<FCreateRoomChainedTask>(
        0,
        SearchGuid,
        RoomId,
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
                EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
                EPartyConfigurationRoomFeaturePermissionType::Anyone,
                EPartyConfigurationRoomFeaturePermissionType::Anyone,
                TEXT("MyParty"),
                TEXT("Party description."),
                TEXT("password")));
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        }));

    // Attempt to join a lobby with no password.
    Executor->Then<FJoinRoomViaSearchChainedTask>(
        1,
        TSet<int32>(),
        SearchGuid,
        RoomId,
        true,
        TDelegate<void(FJoinRoomRequest &)>(),
        true);

    // Attempt to join a lobby with the wrong password.
    Executor->Then<FJoinRoomViaSearchChainedTask>(
        1,
        TSet<int32>(),
        SearchGuid,
        RoomId,
        true,
        TDelegate<void(FJoinRoomRequest &)>::CreateLambda([](FJoinRoomRequest &JoinRoomRequest) {
            JoinRoomRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(TEXT("WrongPassword")));
        }),
        true);

    // Attempt to join a lobby with the correct password.
    Executor->Then<FJoinRoomViaSearchChainedTask>(
        1,
        TSet<int32>{0},
        SearchGuid,
        RoomId,
        true,
        TDelegate<void(FJoinRoomRequest &)>::CreateLambda([](FJoinRoomRequest &JoinRoomRequest) {
            JoinRoomRequest.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(TEXT("password")));
        }),
        false);

    // Run the test.
    Executor->Execute();
}

}