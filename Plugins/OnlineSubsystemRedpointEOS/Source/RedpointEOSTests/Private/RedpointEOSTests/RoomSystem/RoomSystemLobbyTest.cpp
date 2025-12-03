// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/SanctionEnforcementRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsurePublicMemberListChainedTask.h"
#include "RedpointEOSTests/RoomSystem/EnsureRoomMembersChainedTask.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"
#include "RedpointEOSTests/RoomSystem/KickRoomMemberChainedTask.h"
#include "RedpointEOSTests/RoomSystem/LeaveRoomChainedTask.h"
#include "RedpointEOSTests/RoomSystem/PromoteRoomMemberChainedTask.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"
#include "RedpointEOSTests/RoomSystem/UpdateRoomChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3898903581, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;

void RunRoomSystemLobbyTestWithFlags(
    class FAsyncHotReloadableAutomationTestBase *InTest,
    TFunction<void()> InOnDone,
    bool bInSingleSubsystem,
    bool bPublicMemberList)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Id;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(
        new FChainedTaskExecutor(InTest, InOnDone, bInSingleSubsystem ? TArray<int32>{2} : TArray<int32>{1, 1}));

    // Create a shared pointer to hold the room ID.
    TSharedRef<FRoomIdPtr> RoomId = MakeShared<FRoomIdPtr>();
    FGuid SearchGuid = FGuid::NewGuid();

    // Make sure creating a lobby with an unsupported feature fails.
    Executor->Then<FLambdaChainedTask>(
        [RoomId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetRoomSystem();

            auto CreateRoomRequest =
                FCreateRoomRequest(ERoomScope::User, GetAccountId(Context->User(0)), NAME_Lobby, NAME_AutomationTests);

            CreateRoomRequest.AddFeatureRequest(MakeShared<FSanctionEnforcementRoomFeatureRequest>());

            RoomSystem->CreateRoom(
                CreateRoomRequest,
                IRoomSystem::FOnCreateRoomComplete::CreateLambda(
                    [RoomId, Context, OnDone](FError ErrorCode, FRoomPtr Room) {
                        bool bContinue = Context->Assert().TestFalse(
                            FString::Printf(
                                TEXT("Expected CreateRoom for lobby provider to fail for unsupported feature."),
                                *ErrorCode.ToLogString()),
                            ErrorCode.WasSuccessful());
                        OnDone.ExecuteIfBound(bContinue);
                    }));
        });

    // Create a lobby with the room system.
    Executor->Then<FCreateRoomChainedTask>(
        0,
        SearchGuid,
        RoomId,
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([bPublicMemberList](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>("TestBucket"));
            CreateRoomRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(false));
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
            if (bPublicMemberList)
            {
                CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());
            }
            CreateRoomRequest.Attributes.Add(
                TEXT("Hello"),
                FRoomAttribute(FString(TEXT("World")), FRoomAttribute::EVisibility::Public));
            CreateRoomRequest.MemberAttributes.Add(
                TEXT("HelloMember"),
                FRoomAttribute(FString(TEXT("WorldMember")), FRoomAttribute::EVisibility::Public));
        }));

    // Make sure we can see the user in the room via GetCurrentRooms.
    Executor->Then<FLambdaChainedTask>(
        [RoomId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetRoomSystem();

            // Verify we're in exactly one room.
            auto CurrentRooms = RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(0))));
            if (!Context->Assert().TestTrue(TEXT("Expected to see user in 1 room."), CurrentRooms.Num() == 1))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }

            // Verify the room looks like we expect it to.
            auto Room = CurrentRooms[0];
            Context->Assert().TestEqual(
                TEXT("Room IDs to be the same"),
                Room->GetRoomId()->GetRoomId(),
                (*RoomId)->GetRoomId());
            Context->Assert().TestEqual(
                TEXT("Expected room to have namespace of AutomationTests."),
                Room->GetRoomNamespace(),
                NAME_AutomationTests);
            auto BucketFeature = Room->GetRoomFeature<IBucketRoomFeature>();
            if (Context->Assert().TestTrue(TEXT("Expected room to have BucketRoomFeature"), BucketFeature.IsValid()))
            {
                Context->Assert().TestEqual(
                    TEXT("Expected BucketRoomFeature to have BucketId of 'TestBucket'"),
                    BucketFeature->GetBucketId(),
                    TEXT("TestBucket"));
            }
            Context->Assert().TestTrue(
                TEXT("Expected room to have InvitesRoomFeature"),
                Room->HasRoomFeature<IInvitesRoomFeature>());
            Context->Assert().TestTrue(
                TEXT("Expected room to have PresenceRoomFeature"),
                Room->HasRoomFeature<IPresenceRoomFeature>());
            Context->Assert().TestTrue(
                TEXT("Expected room to have PublicAdvertisementRoomFeature"),
                Room->HasRoomFeature<IPublicAdvertisementRoomFeature>());
            auto Attributes = Room->GetRoomAttributes();
            if (Context->Assert().TestTrue(TEXT("Expected room to have Hello attribute"), Attributes.Contains("Hello")))
            {
                Context->Assert().TestEqual(
                    TEXT("Expected room attribute Hello to be equal to World"),
                    Attributes["Hello"].GetString(),
                    FString(TEXT("World")));
            }
            auto Members = Room->GetMembers();
            if (Context->Assert().TestEqual(TEXT("Expected room to have 1 member"), Members.Num(), 1))
            {
                auto MemberAttributes = Members[0]->GetAttributes();
                if (Context->Assert().TestTrue(
                        TEXT("Expected only room member to have HelloMember attribute"),
                        MemberAttributes.Contains("HelloMember")))
                {
                    Context->Assert().TestEqual(
                        TEXT("Expected only room member attribute HelloMember to be equal to WorldMember"),
                        MemberAttributes["HelloMember"].GetString(),
                        FString(TEXT("WorldMember")));
                }
                Context->Assert().TestTrue(
                    TEXT("Expected Members[0]->GetAttributes() to not surface prefixed attributes"),
                    MemberAttributes
                            .FilterByPredicate([](const auto &KV) {
                                return KV.Key.StartsWith("Redpoint:EOS:");
                            })
                            .Num() == 0);
            }
            Context->Assert().TestTrue(
                TEXT("Expected GetRoomAttributes to not surface prefixed attributes"),
                Attributes
                        .FilterByPredicate([](const auto &KV) {
                            return KV.Key.StartsWith("Redpoint:EOS:");
                        })
                        .Num() == 0);

            OnDone.ExecuteIfBound(true);
        });

    // Attempt to search for the lobby.
    Executor->Then<FLambdaChainedTask>(
        [SearchGuid, RoomId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(1)).GetRoomSystem();

            auto SearchRoomRequest = FSearchRoomsRequest(
                GetAccountId(Context->User(1)),
                NAME_Lobby,
                NAME_AutomationTests,
                TMap<FString, FRoomAttributeConstraint>{
                    {TEXT("TestSearchGuid"),
                     FRoomAttributeConstraint(
                         FRoomAttribute(SearchGuid.ToString()),
                         FRoomAttributeConstraint::EConstraintType::Equal)}});

            RoomSystem->SearchRooms(
                SearchRoomRequest,
                IRoomSystem::FOnSearchRoomsComplete::CreateLambda(
                    [RoomId, Context, OnDone](FError ErrorCode, TArray<FRoomSearchResultRef> RoomSearchResults) {
                        if (!Context->Assert().TestTrue(
                                FString::Printf(
                                    TEXT("Expected SearchRooms for lobby provider to succeed, but got error '%s'."),
                                    *ErrorCode.ToLogString()),
                                ErrorCode.WasSuccessful()))
                        {
                            OnDone.ExecuteIfBound(false);
                            return;
                        }

                        // Check that we got a result.
                        if (!Context->Assert().TestEqual(
                                TEXT("Expected to get exactly one search result"),
                                RoomSearchResults.Num(),
                                1))
                        {
                            OnDone.ExecuteIfBound(false);
                            return;
                        }

                        // Check that our search result matches what we expect.
                        auto SearchResult = RoomSearchResults[0];
                        Context->Assert().TestEqual(
                            TEXT("Room IDs to be the same"),
                            SearchResult->GetRoomId()->GetRoomId(),
                            (*RoomId)->GetRoomId());
                        Context->Assert().TestEqual(
                            TEXT("Expected room to have namespace of AutomationTests."),
                            SearchResult->GetRoomNamespace(),
                            NAME_AutomationTests);
                        auto BucketFeatureSearchState =
                            SearchResult->GetRoomFeatureSearchState<IBucketRoomFeatureSearchState>();
                        if (Context->Assert().TestTrue(
                                TEXT("Expected room to have BucketRoomFeature"),
                                BucketFeatureSearchState.IsValid()))
                        {
                            Context->Assert().TestEqual(
                                TEXT("Expected BucketRoomFeature to have BucketId of 'TestBucket'"),
                                BucketFeatureSearchState->Bucket(),
                                TEXT("TestBucket"));
                        }
                        Context->Assert().TestTrue(
                            TEXT("Expected room to have InvitesRoomFeature"),
                            SearchResult->HasRoomFeatureSearchState<IInvitesRoomFeatureSearchState>());
                        Context->Assert().TestTrue(
                            TEXT("Expected room to have PresenceRoomFeature"),
                            SearchResult->HasRoomFeatureSearchState<IPresenceRoomFeatureSearchState>());
                        Context->Assert().TestTrue(
                            TEXT("Expected room to have PublicAdvertisementRoomFeature"),
                            SearchResult->HasRoomFeatureSearchState<IPublicAdvertisementRoomFeatureSearchState>());
                        auto Attributes = SearchResult->GetRoomAttributes();
                        if (Context->Assert().TestTrue(
                                TEXT("Expected room to have Hello attribute"),
                                Attributes.Contains("Hello")))
                        {
                            Context->Assert().TestEqual(
                                TEXT("Expected room attribute Hello to be equal to World"),
                                Attributes["Hello"].GetString(),
                                FString(TEXT("World")));
                        }
                        Context->Assert().TestTrue(
                            TEXT("Expected GetRoomAttributes to not surface prefixed attributes"),
                            Attributes
                                    .FilterByPredicate([](const auto &KV) {
                                        return KV.Key.StartsWith("Redpoint:EOS:");
                                    })
                                    .Num() == 0);

                        OnDone.ExecuteIfBound(true);
                    }));
        });

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0}, TSet<int32>{1}, RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(0, TSet<int32>{0}, RoomId);
    }

    // Search and join the lobby from user 1. This also ensures appropriate events are fired.
    Executor->Then<FJoinRoomViaSearchChainedTask>(1, TSet<int32>{0}, SearchGuid, RoomId, true);

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0, 1}, TSet<int32>(), RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(0, TSet<int32>{0, 1}, RoomId);
    }

    // Make sure updating a lobby with a feature that can't be changed fails.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetRoomSystem();

            auto CurrentRoom = RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(0))))[0];

            auto UpdateRoomRequest = FUpdateRoomRequest();
            UpdateRoomRequest.DeleteFeature(IPresenceRoomFeature::GetFeatureName());

            RoomSystem->UpdateRoom(
                CurrentRoom,
                UpdateRoomRequest,
                IRoomSystem::FOnUpdateRoomComplete::CreateLambda([Context, OnDone](FError ErrorCode) {
                    bool bContinue = Context->Assert().TestFalse(
                        FString::Printf(
                            TEXT("Expected UpdateRoom for lobby provider to fail for deleting a feature that can't be "
                                 "deleted."),
                            *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    OnDone.ExecuteIfBound(bContinue);
                }));
        });

    // Update the lobby and change some features that can be modified.
    Executor->Then<FUpdateRoomChainedTask>(
        0,
        TArray<int32>{0, 1},
        RoomId,
        TDelegate<void(FUpdateRoomRequest &)>::CreateLambda([](FUpdateRoomRequest &UpdateRoomRequest) {
            UpdateRoomRequest.MaxMemberCount = 8;
            UpdateRoomRequest.DeleteFeature(IPublicAdvertisementRoomFeature::GetFeatureName());
            UpdateRoomRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>("OtherAutomationTests"));
            UpdateRoomRequest.UpsertMemberAttributes.Add(
                TEXT("HelloMember"),
                FRoomAttribute(FString(TEXT("WorldTest")), FRoomAttribute::EVisibility::Public));
        }),
        TDelegate<void(const FChainedTaskContextRef &, const FRoomRef &)>::CreateLambda(
            [](const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom) {
                Context->Assert().TestEqual(
                    TEXT("Expected room to have a maximum of 8 members."),
                    CurrentRoom->GetRoomMaxMembers(),
                    8);
                Context->Assert().TestEqual(
                    TEXT("Expected room to have namespace of AutomationTests."),
                    CurrentRoom->GetRoomNamespace(),
                    NAME_AutomationTests);
                Context->Assert().TestFalse(
                    TEXT("Expected room to no longer have PublicAdvertisementRoomFeature."),
                    CurrentRoom->HasRoomFeature<IPublicAdvertisementRoomFeature>());
                auto BucketRoomFeature = CurrentRoom->GetRoomFeature<IBucketRoomFeature>();
                if (Context->Assert().TestTrue(
                        TEXT("Expected room to have BucketRoomFeature."),
                        BucketRoomFeature.IsValid()))
                {
                    Context->Assert().TestEqual(
                        TEXT("Expected room to have a bucket ID of OtherAutomationTests."),
                        BucketRoomFeature->GetBucketId(),
                        FString("OtherAutomationTests"));
                }
                const auto &Members = CurrentRoom->GetMembers();
                if (Context->Assert().TestEqual(TEXT("Expected room to have 2 members"), Members.Num(), 2))
                {
                    auto OwnerMember = Members.FindByPredicate([CurrentRoom](const FRoomMemberRef &Member) {
                        return Member->GetUserId() == CurrentRoom->GetRoomOwner()->GetUserId();
                    });
                    auto NonOwnerMember = Members.FindByPredicate([CurrentRoom](const FRoomMemberRef &Member) {
                        return Member->GetUserId() != CurrentRoom->GetRoomOwner()->GetUserId();
                    });
                    if (Context->Assert().TestNotNull(TEXT("Expected to be able to get owner member"), OwnerMember))
                    {
                        auto MemberAttributes = (*OwnerMember)->GetAttributes();
                        if (Context->Assert().TestTrue(
                                TEXT("Expected only room member to have HelloMember attribute"),
                                MemberAttributes.Contains("HelloMember")))
                        {
                            Context->Assert().TestEqual(
                                TEXT("Expected only room member attribute HelloMember to be equal to WorldTest"),
                                MemberAttributes["HelloMember"].GetString(),
                                FString(TEXT("WorldTest")));
                        }
                    }
                    Context->Assert().TestNotNull(TEXT("Expected to be able to get non-owner member"), NonOwnerMember);
                }
            }));

    // Leave the lobby.
    Executor->Then<FLeaveRoomChainedTask>(
        0,
        TMap<int32, ERoomLeaveReason>{{0, ERoomLeaveReason::LeftExplicitly}, {1, ERoomLeaveReason::RoomDestroyed}},
        TSet<int32>(),
        RoomId);

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>(), TSet<int32>{0, 1}, RoomId);

    // Re-generate the search GUID for a new lobby.
    SearchGuid = FGuid::NewGuid();

    // Create a lobby with owner migration enabled.
    Executor->Then<FCreateRoomChainedTask>(
        0,
        SearchGuid,
        RoomId,
        TDelegate<void(FCreateRoomRequest &)>::CreateLambda([bPublicMemberList](FCreateRoomRequest &CreateRoomRequest) {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FOwnerMigrationRoomFeatureRequest>());
            CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
            if (bPublicMemberList)
            {
                CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());
            }
        }));

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0}, TSet<int32>{1}, RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(0, TSet<int32>{0}, RoomId);
    }

    // Search for the lobby and then join it from user 1.
    Executor->Then<FJoinRoomViaSearchChainedTask>(1, TSet<int32>{0}, SearchGuid, RoomId, false);

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0, 1}, TSet<int32>(), RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(0, TSet<int32>{0, 1}, RoomId);
    }

    // Leave the lobby.
    Executor->Then<FLeaveRoomChainedTask>(
        0,
        TMap<int32, ERoomLeaveReason>{{0, ERoomLeaveReason::LeftExplicitly}},
        TSet<int32>{1},
        RoomId);

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{1}, TSet<int32>{0}, RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(1, TSet<int32>{1}, RoomId);
    }

    // Search for the lobby and then join it from user 0.
    Executor->Then<FJoinRoomViaSearchChainedTask>(0, TSet<int32>{1}, SearchGuid, RoomId, false);

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0, 1}, TSet<int32>(), RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(1, TSet<int32>{0, 1}, RoomId);
    }

    // Promote user 0 to the owner of the lobby as user 1.
    Executor->Then<FPromoteRoomMemberChainedTask>(1, 0, TSet<int32>{0, 1}, RoomId);

    // Kick user 1 from the lobby.
    Executor->Then<FKickRoomMemberChainedTask>(0, 1, RoomId, true);

    // Ensure the instances are in the expected state.
    Executor->Then<FEnsureRoomMembersChainedTask>(TSet<int32>{0}, TSet<int32>{1}, RoomId);
    if (bPublicMemberList)
    {
        Executor->Then<FEnsurePublicMemberListChainedTask>(0, TSet<int32>{0}, RoomId);
    }

    // Run the test.
    Executor->Execute();
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_Lobby,
    "Redpoint.EOS.RoomSystem.Lobby",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_Lobby::RunAsyncTest(const TFunction<void()> &OnDone)
{
    RunRoomSystemLobbyTestWithFlags(this, OnDone, false, false);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyPublicMemberList,
    "Redpoint.EOS.RoomSystem.LobbyPublicMemberList",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyPublicMemberList::RunAsyncTest(const TFunction<void()> &OnDone)
{
    RunRoomSystemLobbyTestWithFlags(this, OnDone, false, true);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyMultipleLocalUsers,
    "Redpoint.EOS.RoomSystem.LobbyMultipleLocalUsers",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyMultipleLocalUsers::RunAsyncTest(const TFunction<void()> &OnDone)
{
    RunRoomSystemLobbyTestWithFlags(this, OnDone, true, false);
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_LobbyMultipleLocalUsersPublicMemberList,
    "Redpoint.EOS.RoomSystem.LobbyMultipleLocalUsersPublicMemberList",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_RoomSystem_LobbyMultipleLocalUsersPublicMemberList::RunAsyncTest(
    const TFunction<void()> &OnDone)
{
    RunRoomSystemLobbyTestWithFlags(this, OnDone, true, true);
}

}