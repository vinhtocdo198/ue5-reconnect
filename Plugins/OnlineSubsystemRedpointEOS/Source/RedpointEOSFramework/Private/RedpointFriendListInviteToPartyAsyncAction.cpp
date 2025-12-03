// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFriendListInviteToPartyAsyncAction.h"

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointFriendListInviteToPartyAsyncAction *URedpointFriendListInviteToPartyAsyncAction::InviteToParty(
    URedpointFriendListEntry *FriendListEntry)
{
    URedpointFriendListInviteToPartyAsyncAction *Node = NewObject<URedpointFriendListInviteToPartyAsyncAction>();
    Node->Entry = FriendListEntry;
    return Node;
}

void URedpointFriendListInviteToPartyAsyncAction::Activate()
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    using namespace ::Redpoint::EOS::Core::Id;

    if (!IsValid(this->Entry) || !this->Entry->UserId.IsValid() || !IsValid(this->Entry->BoundForLocalPlayer))
    {
        this->OnComplete.Broadcast();
        return;
    }

    auto *OSSv1 = Online::GetSubsystem(this->Entry->BoundForLocalPlayer->GetWorld());
    if (OSSv1 == nullptr)
    {
        this->OnComplete.Broadcast();
        return;
    }

    auto PartySystem = OSSv1->GetPartyInterface();
    if (!PartySystem.IsValid())
    {
        this->OnComplete.Broadcast();
        return;
    }

    auto PrimaryParty =
        PartySystem->GetParty(*GetUniqueNetId(Entry->BoundForLocalUserId), PartySystem->GetPrimaryPartyTypeId());
    if (!PrimaryParty.IsValid())
    {
        this->OnComplete.Broadcast();
        return;
    }

    this->Entry->bIsBeingActedUpon = true;

    if (!PartySystem->SendInvitation(
            *GetUniqueNetId(Entry->BoundForLocalUserId),
            *PrimaryParty->PartyId,
            FPartyInvitationRecipient(*Entry->UserId),
            FOnSendPartyInvitationComplete::CreateWeakLambda(
                this,
                [this](
                    const FUniqueNetId &,
                    const FOnlinePartyId &,
                    const FUniqueNetId &,
                    const ESendPartyInvitationCompletionResult) {
                    // @todo: Propagate errors in some standard way.
                    this->Entry->bIsBeingActedUpon = false;
                    this->OnComplete.Broadcast();
                    return;
                })))
    {
        this->Entry->bIsBeingActedUpon = false;
        this->OnComplete.Broadcast();
        return;
    }
#else
    this->OnComplete.Broadcast();
#endif
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()