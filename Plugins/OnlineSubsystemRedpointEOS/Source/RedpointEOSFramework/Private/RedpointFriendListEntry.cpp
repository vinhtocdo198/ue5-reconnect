// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFriendListEntry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

ERedpointFriendListSectionType URedpointFriendListEntry::GetSectionType() const
{
    if (this->bInParty)
    {
        return ERedpointFriendListSectionType::InParty;
    }

    if (this->InviteStatus == ERedpointFriendListEntryInviteStatus::Blocked)
    {
        return ERedpointFriendListSectionType::Blocked;
    }

    if (this->InviteStatus == ERedpointFriendListEntryInviteStatus::Recent)
    {
        return ERedpointFriendListSectionType::Recent;
    }

    if (this->bPlayingThisGame && this->InviteStatus == ERedpointFriendListEntryInviteStatus::MutualFriends)
    {
        return ERedpointFriendListSectionType::InGame;
    }

    if (this->PresenceStatus != ERedpointFriendListEntryPresenceStatus::Offline)
    {
        return ERedpointFriendListSectionType::Online;
    }

    return ERedpointFriendListSectionType::Offline;
}

bool URedpointFriendListEntry::IsOrderedLessThan(const URedpointFriendListEntry *Other) const
{
    if (!IsValid(Other))
    {
        return false;
    }

    auto SectionType = this->GetSectionType();

    if (SectionType == ERedpointFriendListSectionType::InGame ||
        SectionType == ERedpointFriendListSectionType::Online || SectionType == ERedpointFriendListSectionType::Recent)
    {
        if (this->PresenceStatus == Other->PresenceStatus)
        {
            return this->DisplayName < Other->DisplayName;
        }
        else
        {
            return this->PresenceStatus > Other->PresenceStatus;
        }
    }
    else
    {
        return this->DisplayName < Other->DisplayName;
    }
}

bool URedpointFriendListEntry::CanInviteToParty() const
{
    return !this->bInParty && this->InviteStatus != ERedpointFriendListEntryInviteStatus::Blocked &&
           this->bIsInvitable && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanKickFromParty() const
{
    return this->bInParty && this->bHasAuthorityOverPartyMember && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanPromoteToPartyLeader() const
{
    return this->bInParty && this->bHasAuthorityOverPartyMember && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanSendFriendRequest() const
{
    return this->bHasGameProfile && this->InviteStatus != ERedpointFriendListEntryInviteStatus::Blocked &&
           this->InviteStatus != ERedpointFriendListEntryInviteStatus::MutualFriends &&
           this->InviteStatus != ERedpointFriendListEntryInviteStatus::PendingInbound &&
           this->InviteStatus != ERedpointFriendListEntryInviteStatus::PendingOutbound && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanBlockPlayer() const
{
    return this->bHasGameProfile && this->InviteStatus != ERedpointFriendListEntryInviteStatus::Blocked &&
           !this->bIsSelf;
}

bool URedpointFriendListEntry::CanUnblockPlayer() const
{
    return this->bHasGameProfile && this->InviteStatus == ERedpointFriendListEntryInviteStatus::Blocked &&
           !this->bIsSelf;
}

bool URedpointFriendListEntry::CanRemoveFriend() const
{
    return this->bHasGameProfile && this->bIsGameFriend &&
           this->InviteStatus == ERedpointFriendListEntryInviteStatus::MutualFriends && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanAcceptFriendRequest() const
{
    return this->InviteStatus == ERedpointFriendListEntryInviteStatus::PendingInbound && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanRejectFriendRequest() const
{
    return this->InviteStatus == ERedpointFriendListEntryInviteStatus::PendingInbound && !this->bIsSelf;
}

bool URedpointFriendListEntry::CanJoinParty() const
{
    return this->bInJoinableParty && !this->bInParty && !this->bIsSelf &&
           this->InviteStatus != ERedpointFriendListEntryInviteStatus::Blocked;
}

bool URedpointFriendListEntry::CanLeaveParty() const
{
    return this->bInParty && this->bIsSelf;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()