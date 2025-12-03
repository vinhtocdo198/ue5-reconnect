// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/TestUtilities/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3328965025, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FCreatePartyChainedTask::FCreatePartyChainedTask(
    int32 InInstanceId,
    TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId,
    FOnlinePartyTypeId InPartyTypeId,
    bool bInEnableVoiceChat,
    EJoinRequestAction InJoinRequestAction)
    : InstanceId(InInstanceId)
    , StoredPartyId(InStoredPartyId)
    , PartyTypeId(InPartyTypeId)
    , OnPartyJoinedHandle()
    , OnPartyMemberJoinedHandle()
    , bSawPartyJoinedEvent(false)
    , bSawPartyMemberJoinedEvent(false)
    , PartyIdFromOnPartyJoined()
    , bEnableVoiceChat(bInEnableVoiceChat)
    , JoinRequestAction(InJoinRequestAction)
{
}

void FCreatePartyChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto PartyInterface = Context->Instance(this->InstanceId).GetPartyInterface();

    // Register events that we're interested in.
    this->OnPartyJoinedHandle = PartyInterface->AddOnPartyJoinedDelegate_Handle(
        FOnPartyJoinedDelegate::CreateSP(this, &FCreatePartyChainedTask::OnPartyJoinedEvent, Context));
    this->OnPartyMemberJoinedHandle = PartyInterface->AddOnPartyMemberJoinedDelegate_Handle(
        FOnPartyMemberJoinedDelegate::CreateSP(this, &FCreatePartyChainedTask::OnPartyMemberJoinedEvent, Context));

    // Log what party type ID we're about to create.
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("User '%s' is creating party with party type '%u'."),
        *Context->User(this->InstanceId).ToString(),
        this->PartyTypeId.GetValue());

    // Create the party.
    auto PartyConfiguration = MakeShared<FPartyConfiguration>();
    PartyConfiguration->JoinRequestAction = this->JoinRequestAction;
    PartyConfiguration->PresencePermissions = this->PartyTypeId == IOnlinePartySystem::GetPrimaryPartyTypeId()
                                                  ? PartySystemPermissions::EPermissionType::Anyone
                                                  : PartySystemPermissions::EPermissionType::Noone;
    PartyConfiguration->InvitePermissions = PartySystemPermissions::EPermissionType::Anyone;
    PartyConfiguration->bChatEnabled = bEnableVoiceChat;
    PartyConfiguration->bShouldRemoveOnDisconnection = false;
    PartyConfiguration->bIsAcceptingMembers = true;
    PartyConfiguration->NotAcceptingMembersReason = 0;
    PartyConfiguration->MaxMembers = 4;
    if (!Context->Assert().TestTrue(
            TEXT("Expected CreateParty operation to start successfully."),
            PartyInterface->CreateParty(
                Context->User(this->InstanceId),
                this->PartyTypeId,
                *PartyConfiguration,
                FOnCreatePartyComplete::CreateSP(this, &FCreatePartyChainedTask::OnPartyCreated, Context, OnDone))))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
}

void FCreatePartyChainedTask::OnPartyCreated(
    const FUniqueNetId &LocalUserId,
    const TSharedPtr<const FOnlinePartyId> &PartyId,
    const ECreatePartyCompletionResult Result,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    auto PartyInterface = Context->Instance(this->InstanceId).GetPartyInterface();

    if (!Context->Assert().TestEqual(
            TEXT("Expected CreateParty operation to succeed."),
            Result,
            ECreatePartyCompletionResult::Succeeded))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    UE_LOG(LogRedpointEOSTests, Verbose, TEXT("Party system state after creating party:"));
    PartyInterface->DumpPartyState();

    // @note: Make a new copy of the user ID to test that the PartyDatabase's index over FUniqueNetIdRef
    // is correctly implemented.
    auto RecreatedUserId = MakeShared<FUniqueNetIdEOS>(
        StaticCastSharedRef<const FUniqueNetIdEOS>(LocalUserId.AsShared())->GetProductUserId());

    // Make sure that the joined party appears in GetJoinedParties.
    TArray<TSharedRef<const FOnlinePartyId>> PartyIdArray;
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetJoinedParties to return true."),
            PartyInterface->GetJoinedParties(*RecreatedUserId, PartyIdArray)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    bool bFoundJoinedParty = false;
    for (const auto &PartyIdEntry : PartyIdArray)
    {
        if (*PartyIdEntry == *PartyId)
        {
            bFoundJoinedParty = true;
            break;
        }
    }
    if (!Context->Assert().TestTrue(
            TEXT("Expected array from GetJoinedParties to contain the new party ID."),
            bFoundJoinedParty))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure we can get the party object.
    auto Party = PartyInterface->GetParty(*RecreatedUserId, *PartyId);
    if (!Context->Assert().TestTrue(TEXT("Expected GetParty to return party object."), Party.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure the party object has a party type ID that matches what we expect.
    if (!Context->Assert().TestEqual(
            TEXT("Expected party type ID to match what was requested."),
            Party->PartyTypeId.GetValue(),
            this->PartyTypeId.GetValue()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure we can get the party object by type ID.
    auto PartyByType = PartyInterface->GetParty(*RecreatedUserId, this->PartyTypeId);
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetParty (by type ID) to return party object."),
            PartyByType.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure we can get the party data.
    auto PartyData = PartyInterface->GetPartyData(*RecreatedUserId, *PartyId, NAME_Default);
    if (!Context->Assert().TestTrue(TEXT("Expected GetPartyData to return party object."), PartyData.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure the party system reports that the user local user is a member of the party.
    TArray<FOnlinePartyMemberConstRef> PartyMembers;
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetPartyMembers to return true."),
            PartyInterface->GetPartyMembers(*RecreatedUserId, *PartyId, PartyMembers)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    bool bFoundLocalUser = false;
    for (const auto &PartyMemberEntry : PartyMembers)
    {
        if (*PartyMemberEntry->GetUserId() == *RecreatedUserId)
        {
            bFoundLocalUser = true;
            break;
        }
    }
    if (!Context->Assert().TestTrue(
            TEXT("Expected array from GetPartyMembers to contain the local user as a member."),
            bFoundLocalUser))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure we can get the local user's member information via GetPartyMember.
    auto PartyMember = PartyInterface->GetPartyMember(*RecreatedUserId, *PartyId, *RecreatedUserId);
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetPartyMember to return party member for local user."),
            PartyMember.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make sure we can get the local user's member data via GetPartyMemberData.
    auto PartyMemberData =
        PartyInterface->GetPartyMemberData(*RecreatedUserId, *PartyId, *RecreatedUserId, NAME_Default);
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetPartyMemberData to return party member for local user."),
            PartyMemberData.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Store the party ID.
    *this->StoredPartyId = PartyId;

    // Ensure that we saw OnPartyJoined trigger (it should happen before the CreateParty callback).
    if (Context->Assert().TestTrue(
            TEXT("Expected OnPartyJoined event to trigger in response to party creation."),
            this->bSawPartyJoinedEvent))
    {
        // Ensure that the party ID we saw from OnPartyJoined matches the stored party ID.
        Context->Assert().TestTrue(
            TEXT("Expected PartyId from OnPartyJoined to match the PartyId in the create callback."),
            this->PartyIdFromOnPartyJoined.IsValid() && *PartyIdFromOnPartyJoined == *PartyId);
    }

    // We have to wait a tick to see whether the party system fires events correctly.
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FCreatePartyChainedTask::OnNextTick, Context, OnDone));
}

bool FCreatePartyChainedTask::OnNextTick(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    // Ensure that we *did not* see an OnPartyMemberJoined event trigger, since the person who created the party will
    // implicitly be in the party.
    Context->Assert().TestFalse(
        TEXT("Expected to NOT see an OnPartyMemberJoined event triggered in response to party creation."),
        this->bSawPartyMemberJoinedEvent);

    // Unregister events.
    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->ClearOnPartyJoinedDelegate_Handle(this->OnPartyJoinedHandle);
    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->ClearOnPartyMemberJoinedDelegate_Handle(this->OnPartyMemberJoinedHandle);

    // Return success.
    OnDone.ExecuteIfBound(true);

    // Don't tick again.
    return false;
}

void FCreatePartyChainedTask::OnPartyJoinedEvent(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    FChainedTaskContextRef Context)
{
    // If this event isn't for our local user, ignore it.
    if (LocalUserId != Context->User(this->InstanceId))
    {
        return;
    }

    // Ensure we don't have StoredPartyId yet.
    if (!Context->Assert().TestFalse(
            TEXT("Expected StoredPartyId to not be set yet (the global OnPartyJoined event should fire before the "
                 "CreateParty callback)."),
            this->StoredPartyId->IsValid()))
    {
        return;
    }

    // Ensure that the event is fired for the party we expect.
    this->PartyIdFromOnPartyJoined = PartyId.AsShared();
    this->bSawPartyJoinedEvent = true;
}

void FCreatePartyChainedTask::OnPartyMemberJoinedEvent(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    FChainedTaskContextRef Context)
{
    // If this event isn't for our local user, ignore it.
    if (LocalUserId != Context->User(this->InstanceId))
    {
        return;
    }

    // We don't care about the other parameters here, because we expect this event to not fire at all.
    this->bSawPartyMemberJoinedEvent = true;
}

}

REDPOINT_EOS_CODE_GUARD_END()