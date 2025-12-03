// Copyright June Rhodes. All Rights Reserved.

// @note: This test is too flaky to enable, since it relies on the presence system querying lobbies after the EOS
// backend has replicated the new lobby state to search queries, and we don't know how long that will take. We could
// make this test retry periodically, but the refresh period for presence is around 10 seconds, so that will make this
// test take ages to complete if we have to wait for the backend to reconcile.
#if 0

#include "Internationalization/TextLocalizationManager.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentityUserId.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Requests/CreateRoomRequest.h"
#include "RedpointEOSTests/Presence/PresenceTestLocalizedTextSource.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(571474677, Redpoint::EOS::Tests::Presence)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::Tests::Presence;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Presence_UpdatePresenceInterestWithPresenceRoom,
    "Redpoint.EOS.Presence.UpdatePresenceInterestWithPresenceRoom",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Presence_UpdatePresenceInterestWithPresenceRoom::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create a presence advertised party on the first user.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IRoomSystem>();

        FCreateRoomRequest Request(ERoomScope::User, GetAccountId(Context->User(0)), NAME_Lobby, NAME_Game, 4);
        Request.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
        Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        Request.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(PartyTypePrimaryPartyId));
        Request.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
            EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            EPartyConfigurationRoomFeaturePermissionType::Anyone,
            TEXT("Nickname"),
            TEXT("Description"),
            TEXT("Password")));

        RoomSystem->CreateRoom(
            Request,
            IRoomSystem::FOnCreateRoomComplete::CreateLambda(
                [Context, OnDone](const FError &ErrorCode, const FRoomPtr &Room) {
                    OnDone.ExecuteIfBound(Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected CreateRoom operation to succeed."), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful()));
                }));
    });

    // Call SetPresence for the first user.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        // Register a text localization source so that presence can restore the StatusTextFormat from FText::FindText
        // (which usually only works if you prepare localization for the game).
        if (!FPresenceTestLocalizedTextSource::bRegistered)
        {
            FTextLocalizationManager::Get().RegisterTextSource(MakeShared<FPresenceTestLocalizedTextSource>());
            FPresenceTestLocalizedTextSource::bRegistered = true;
        }

        auto DesiredState = FPresenceUserState(GetAccountId(Context->User(0)));
        DesiredState.ActivityState = EPresenceUserActivityState::Online;
        DesiredState.StatusTextFormat = NSLOCTEXT("RedpointEOSTests", "StatusTextTest", "Status number is {Number}");
        DesiredState.StatusTextFormatArguments.Add(TEXT("Number"), FFormatArgumentValue(20));
        DesiredState.CustomAttributes.Add(TEXT("TestAttribute"), FRoomAttribute(TEXT("Hello World")));

        PresenceSystem->SetPresence(
            GetAccountId(Context->User(0)),
            DesiredState,
            IPresenceSystem::FOnSetPresenceComplete::CreateLambda(
                [PresenceSystem, Context, OnDone](const FError &ErrorCode) {
                    OnDone.ExecuteIfBound(Context->Assert().TestTrue(
                        FString::Printf(TEXT("Expected SetPresence operation to succeed."), *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful()));
                }));
    });

    // Check that we can read our own presence with GetPresence.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &OnDone) {
            auto PresenceSystem =
                ((FOnlineSubsystemEOS &)Context->Instance(0)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

            auto RetrievedState =
                PresenceSystem->GetPresence(GetAccountId(Context->User(0)), GetAccountId(Context->User(0)));
            if (!Context->Assert().TestTrue(
                    TEXT("Expected GetPresence to return a value for the local user."),
                    RetrievedState.IsSet()))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }
            if (!Context->Assert().TestTrue(
                    TEXT("Expected local presence to have advertised party"),
                    RetrievedState->AdvertisedParty.IsSet()))
            {
                OnDone.ExecuteIfBound(false);
                return;
            }

            Context->Assert().TestEqual(
                TEXT("Expected local presence advertised party current member count to be 1"),
                RetrievedState->AdvertisedParty->CurrentMemberCount,
                1);
            Context->Assert().TestEqual(
                TEXT("Expected local presence advertised party current member count to be 4"),
                RetrievedState->AdvertisedParty->MaximumMemberCount,
                4);
            Context->Assert().TestTrue(
                TEXT("Expected local presence advertised party to allow join by presence"),
                RetrievedState->AdvertisedParty->bJoinable);
            Context->Assert().TestEqual(
                TEXT("Expected local presence advertised party to have secret password"),
                RetrievedState->AdvertisedParty->SecretPassword,
                TEXT("Password"));
            Context->Assert().TestTrue(
                TEXT("Expected local presence advertised party to require secret password"),
                RetrievedState->AdvertisedParty->bRequiresSecretPassword);
            Context->Assert().TestFalse(
                TEXT("Expected local presence advertised party to not be a match"),
                RetrievedState->AdvertisedParty->bIsMatch);

            OnDone.ExecuteIfBound(true);
        });

    // Check that the second user can call UpdatePresenceInterest, see the presence data, and then call
    // GetPresence to also see it.
    Executor->Then<FLambdaChainedTask>([](const FChainedTaskContextRef &Context,
                                          const FLambdaChainedTask::FOnComplete &OnDone) {
        auto PresenceSystem =
            ((FOnlineSubsystemEOS &)Context->Instance(1)).GetPlatformHandle()->GetSystem<IPresenceSystem>();

        PresenceSystem->UpdatePresenceInterest(
            FName(TEXT("AutomationTests")),
            GetAccountId(Context->User(1)),
            TArray<FPresenceInterestChange>{FPresenceInterestChange(GetAccountId(Context->User(0)), true)},
            true,
            IPresenceSystem::FOnUpdatePresenceInterestComplete::CreateLambda(
                [PresenceSystem, Context, OnDone](
                    const FError &ErrorCode,
                    const TMap<FIdentityUserId, FPresenceUserState> &RemoteUserPresenceState) {
                    bool bContinue = Context->Assert().TestTrue(
                        FString::Printf(
                            TEXT("Expected UpdatePresenceInterest operation to succeed."),
                            *ErrorCode.ToLogString()),
                        ErrorCode.WasSuccessful());
                    if (!bContinue)
                    {
                        OnDone.ExecuteIfBound(bContinue);
                        return;
                    }

                    auto RunAssertions = [Context](const FPresenceUserState &State) {
                        Context->Assert().TestEqual(
                            TEXT("Expected ActivityState to be Online"),
                            State.ActivityState,
                            EPresenceUserActivityState::Online);
#if REDPOINT_EOS_UE_5_5_OR_LATER
                        Context->Assert().TestEqual(
                            TEXT("Expected StatusTextFormat to have text key StatusTextTest"),
                            FString(
                                FTextInspector::GetTextId(State.StatusTextFormat.GetSourceText()).GetKey().ToString()),
                            TEXT("StatusTextTest"));
#else
                        Context->Assert().TestEqual(
                            TEXT("Expected StatusTextFormat to have text key StatusTextTest"),
                            FString(
                                FTextInspector::GetTextId(State.StatusTextFormat.GetSourceText()).GetKey().GetChars()),
                            TEXT("StatusTextTest"));
#endif
                        Context->Assert().TestEqual(
                            TEXT("Expected StatusText to format correctly"),
                            State.GetStatusTextFormatted().ToString(),
                            TEXT("Status number is 20"));
                        Context->Assert().TestTrue(
                            TEXT("Expected to have TestAttribute custom attribute"),
                            State.CustomAttributes.Contains(TEXT("TestAttribute")));

                        if (Context->Assert().TestTrue(
                                TEXT("Expected presence to have advertised party"),
                                State.AdvertisedParty.IsSet()))
                        {
                            Context->Assert().TestEqual(
                                TEXT("Expected presence advertised party current member count to be 1"),
                                State.AdvertisedParty->CurrentMemberCount,
                                1);
                            Context->Assert().TestEqual(
                                TEXT("Expected presence advertised party current member count to be 4"),
                                State.AdvertisedParty->MaximumMemberCount,
                                4);
                            Context->Assert().TestTrue(
                                TEXT("Expected presence advertised party to allow join by presence"),
                                State.AdvertisedParty->bJoinable);
                            Context->Assert().TestEqual(
                                TEXT("Expected presence advertised party to not have secret password data (because "
                                     "it's intentionally hidden)"),
                                State.AdvertisedParty->SecretPassword,
                                TEXT(""));
                            Context->Assert().TestTrue(
                                TEXT("Expected presence advertised party to require secret password"),
                                State.AdvertisedParty->bRequiresSecretPassword);
                            Context->Assert().TestFalse(
                                TEXT("Expected presence advertised party to not be a match"),
                                State.AdvertisedParty->bIsMatch);
                        }
                    };

                    auto *RemoteUser = RemoteUserPresenceState.Find(GetAccountId(Context->User(0)));
                    if (!Context->Assert().TestNotNull(
                            TEXT("Expected remote user presence state to have an entry for the desired user"),
                            RemoteUser))
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }
                    RunAssertions(*RemoteUser);

                    auto RetrievedState =
                        PresenceSystem->GetPresence(GetAccountId(Context->User(1)), GetAccountId(Context->User(0)));
                    if (!Context->Assert().TestTrue(
                            TEXT("Expected GetPresence to return a value for the remote user that we have just "
                                 "fetched presence for."),
                            RetrievedState.IsSet()))
                    {
                        OnDone.ExecuteIfBound(false);
                        return;
                    }
                    RunAssertions(RetrievedState.GetValue());

                    OnDone.ExecuteIfBound(true);
                }));
    });

    // Run the test.
    Executor->Execute();
}

}

#endif