// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsurePublicMemberListChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsureRoomMembersChainedTask.h"
#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"
#include "RedpointEOSTests/RoomSystem/LeaveRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/PromoteRoomMemberChainedTask.h"
#include "RedpointEOSTests/RoomSystem/SearchRoomsChainedTask.h"
#include "RedpointEOSTests/RoomSystem/UpdateRoomChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(2113273976, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbySearchArray,
    "Redpoint.EOS.RoomSystem.LobbySearchArray",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbySearchArray::RunAsyncTest(const TFunction<void()> &OnDone)
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
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([SearchGuid](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
            CreateRoomRequest.Attributes.Add(TEXT("Guid"), SearchGuid.ToString());
            CreateRoomRequest.Attributes.Add(TEXT("SingleValue"), TEXT("A"));
            CreateRoomRequest.Attributes.Add(TEXT("MultiValue"), TArray<FString>{TEXT("A"), TEXT("B"), TEXT("C")});
        }));

    // Make sure we can find the lobby by the GUID.
    Executor->Then<FSearchRoomsChainedTask>(
        1,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("Guid"),
             FRoomAttributeConstraint(SearchGuid.ToString(), FRoomAttributeConstraint::EConstraintType::Equal)}},
        TDelegate<void(const FChainedTaskContextRef &, const TArray<FRoomSearchResultRef> &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &Results) {
                Context->Assert().TestTrue(TEXT("Expected to find search result by GUID only!"), Results.Num() > 0);
            }));

    // Make sure we can find the lobby by matching single value in lobby system with any value of a list.
    Executor->Then<FSearchRoomsChainedTask>(
        1,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("Guid"),
             FRoomAttributeConstraint(SearchGuid.ToString(), FRoomAttributeConstraint::EConstraintType::Equal)},
            {TEXT("SingleValue"),
             FRoomAttributeConstraint(
                 TArray<FString>{TEXT("A"), TEXT("B"), TEXT("C")},
                 FRoomAttributeConstraint::EConstraintType::AnyOf)}},
        TDelegate<void(const FChainedTaskContextRef &, const TArray<FRoomSearchResultRef> &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &Results) {
                Context->Assert().TestTrue(TEXT("Expected to find search result by AnyOf!"), Results.Num() > 0);
            }));

    // Make sure we can't find the lobby if our local value doesn't contain any of the values on the room.
    Executor->Then<FSearchRoomsChainedTask>(
        1,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("Guid"),
             FRoomAttributeConstraint(SearchGuid.ToString(), FRoomAttributeConstraint::EConstraintType::Equal)},
            {TEXT("SingleValue"),
             FRoomAttributeConstraint(
                 TArray<FString>{TEXT("D"), TEXT("E"), TEXT("F")},
                 FRoomAttributeConstraint::EConstraintType::AnyOf)}},
        TDelegate<void(const FChainedTaskContextRef &, const TArray<FRoomSearchResultRef> &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &Results) {
                Context->Assert().TestTrue(TEXT("Expected to find no search results!"), Results.Num() == 0);
            }),
        false);

    // @note: OneOf comparisons are not supported for EOS lobbies (perhaps only EOS sessions, but I haven't had a chance
    // to test that yet). This code is left here under an #if guard so that if Epic adds support for OneOf comparisons
    // on EOS lobbies in the future, we can re-enable it.
#if 0
    // Make sure we can find the lobby by matching multiple value in lobby system with one value searched.
    Executor->Then<FSearchRoomsChainedTask>(
        1,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("Guid"),
             FRoomAttributeConstraint(SearchGuid.ToString(), FRoomAttributeConstraint::EConstraintType::Equal)},
            {TEXT("MultiValue"),
             FRoomAttributeConstraint(TEXT("B"), FRoomAttributeConstraint::EConstraintType::OneOf)}},
        TDelegate<void(const FChainedTaskContextRef &, const TArray<FRoomSearchResultRef> &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &Results) {
                Context->Assert().TestTrue(TEXT("Expected to find search result by OneOf!"), Results.Num() > 0);
            }));

    // Make sure we can't find the lobby if we pick a value that isn't in the lobby system.
    Executor->Then<FSearchRoomsChainedTask>(
        1,
        TMap<FString, FRoomAttributeConstraint>{
            {TEXT("Guid"),
             FRoomAttributeConstraint(SearchGuid.ToString(), FRoomAttributeConstraint::EConstraintType::Equal)},
            {TEXT("MultiValue"),
             FRoomAttributeConstraint(TEXT("D"), FRoomAttributeConstraint::EConstraintType::OneOf)}},
        TDelegate<void(const FChainedTaskContextRef &, const TArray<FRoomSearchResultRef> &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const TArray<FRoomSearchResultRef> &Results) {
                Context->Assert().TestTrue(TEXT("Expected to find no search results!"), Results.Num() == 0);
            }));
#endif

    // Leave the lobby.
    Executor->Then<FLeaveRoomChainedTask>(
        0,
        TMap<int32, ERoomLeaveReason>{{0, ERoomLeaveReason::LeftExplicitly}},
        TSet<int32>(),
        RoomId);

    // Run the test.
    Executor->Execute();
}

}