// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/LocalPlayer.h"
#include "GameFramework/OnlineReplStructs.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointFriendListEntryInviteStatus.h"
#include "RedpointFriendListEntryPresenceStatus.h"
#include "RedpointFriendListSectionType.h"

#include "RedpointFriendListEntry.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
class REDPOINTEOSFRAMEWORK_API URedpointFriendListEntry : public UObject
{
    GENERATED_BODY()

public:
    /**
     * The local player that this friend list entry belongs to. This is used by async blueprint actions to perform
     * actions directly on friend list entries without the local player subsystem reference.
     */
    UE::Online::FAccountId BoundForLocalUserId;

    /**
     * The local player that this friend list entry belongs to. This is used by async blueprint actions to perform
     * actions directly on friend list entries without the local player subsystem reference.
     */
    UPROPERTY()
    TObjectPtr<ULocalPlayer> BoundForLocalPlayer;

    /**
     * If true, this user has a game profile and the UserId property will be filled with a valid user ID. The local user
     * may have friends from the local platform who have never played this game before, in which case the UserId
     * property will not have a valid user ID set.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bHasGameProfile = true;

    /**
     * The user ID.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    FUniqueNetIdRepl UserId;

    /**
     * The user's display name.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    FString DisplayName;

    /**
     * This user is in the local player's current party.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bInParty = false;

    /**
     * If true, this user can be invited to parties or games via EOS or the local platform.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bIsInvitable = false;

    /**
     * If true, this user represents the local user. There will only be an entry for the local player if friend lists
     * are configured to display the local player under the "In Party" section. This should not be used by game code.
     */
    bool bIsSelf = false;

    /**
     * If true, the local player has authority over this party member. This should not be used directly;
     * CanKickFromParty should be used instead.
     */
    bool bHasAuthorityOverPartyMember = false;

    /**
     * If true, this party member is the leader of the party that the local player is in. This should not be used
     * directly; CanLeaveParty should be used instead.
     */
    bool bIsCurrentPartyLeader = false;

    /**
     * The user's invite status, and how they are related to the local player.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    ERedpointFriendListEntryInviteStatus InviteStatus;

    /**
     * The user's presence status.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    ERedpointFriendListEntryPresenceStatus PresenceStatus;

    /**
     * If true, this user is currently playing this game.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bPlayingThisGame = false;

    /**
     * If true, this user is a friend only at the game level, and not from the platform.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bIsGameFriend = false;

    /**
     * If true, the local player is currently performing some kind of action on this friend (such as inviting them to
     * the party). This boolean value can be used to disable UI elements when the user is already doing something or to
     * show an operation as in-progress.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bIsBeingActedUpon = false;

    /**
     * If the user is currently in a party that the local player can join.
     */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool bInJoinableParty = false;

    /**
     * Returns the section type that this entry should be displayed under.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    ERedpointFriendListSectionType GetSectionType() const;

    /**
     * Returns true if this entry should be sorted earlier in a list than the entry passed in Other.
     */
    bool IsOrderedLessThan(const URedpointFriendListEntry *Other) const;

    /**
     * If true, the local player can invite this person to the party.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanInviteToParty() const;

    /**
     * If true, the local player can kick this person from the party.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanKickFromParty() const;

    /**
     * If true, the local player can promote this person to party leader.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanPromoteToPartyLeader() const;

    /**
     * If true, the local player can send a friend request to this person.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanSendFriendRequest() const;

    /**
     * If true, the local player can block this player.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanBlockPlayer() const;

    /**
     * If true, the local player can unblock this player.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanUnblockPlayer() const;

    /**
     * If true, the local player can remove this friend.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanRemoveFriend() const;

    /**
     * If true, the local player can accept this friend request.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanAcceptFriendRequest() const;

    /**
     * If true, the local player can reject this friend request.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanRejectFriendRequest() const;

    /**
     * If true, the local player can join this person's current party.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanJoinParty() const;

    /**
     * If true, the local player is a member of this person's party and this person is the leader.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Friend Lists for UMG")
    bool CanLeaveParty() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRedpointFriendListActionComplete);

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()