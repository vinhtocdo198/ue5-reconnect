// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/TreeView.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSPresence/PresenceUserActivityState.h"
#include "RedpointEOSPresence/PresenceUserState.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointFriendListSection.h"
#include "RedpointFriendListSectionType.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Types/OSBPartyTypes.h"

#include "RedpointFrameworkLocalPlayerSubsystem.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(
    Blueprintable,
    Category = "Redpoint EOS Online Framework",
    meta = (DisplayName = "Redpoint EOS Online Framework Local Player Subsystem"))
class REDPOINTEOSFRAMEWORK_API URedpointFrameworkLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    URedpointFrameworkLocalPlayerSubsystem(const FObjectInitializer &ObjectInitializer);

    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void PlayerControllerChanged(APlayerController *NewPlayerController) override;

    bool bInitialized;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    void TryInitialize();
#endif

    /**
     * The list of friend section objects that are cached for this local player.
     */
    UPROPERTY()
    TMap<ERedpointFriendListSectionType, TObjectPtr<URedpointFriendListSection>> FriendSections;

    /**
     * Shared cache of friend entry objects so that we can move friends between different sections without recreating
     * the objects.
     */
    UPROPERTY()
    TMap<FString, TObjectPtr<URedpointFriendListEntry>> FriendEntries;

    /**
     * If true, cached platform friends that do not currently have a game profile will be included in the friends list.
     * Local players are unable to do anything with these friends, as the platform implementation is not available
     * (they're coming from the cache), and EOS does not have a PUID to send over.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        BlueprintSetter = SetDisplayCachedPlatformFriends,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Display Cached Platform Friends"))
    bool bDisplayCachedPlatformFriends = false;

    /**
     * Change whether cached platform friends are displayed in friends lists. These friends can't be acted upon, so it's
     * recommended that you exclude them (which is also the default setting).
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Set Display Cached Platform Friends"))
    void SetDisplayCachedPlatformFriends(bool bEnabled);

    /**
     * If true, the local player will be included in the "In Party" section of friend lists.
     */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        BlueprintSetter = SetIncludeLocalPlayerInPartySection,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Include Local Player in Party Section"))
    bool bIncludeLocalPlayerInPartySection = false;

    /**
     * Change whether the local player is included in the "In Party" section of friend lists.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Set Include Local Player in Party Section"))
    void SetIncludeLocalPlayerInPartySection(bool bEnabled);

    /**
     * The tree views that are currently bound to friends lists and maintained by this subsystem.
     */
    TMap<TSoftObjectPtr<UTreeView>, TArray<ERedpointFriendListSectionType>> BoundTreeViews;

    /**
     * Binds the specified friend list sections to the UMG tree view. The subsystem will automatically keep the tree
     * view items up-to-date and bind the "On Get Item Children" event of the tree view to populate child items.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Bind Friend List Sections to Tree View Widget"))
    void BindFriendListSections(UTreeView *TreeView, const TArray<ERedpointFriendListSectionType> &Sections);

    /**
     * Unbinds all of the friend list sections from the UMG tree view. The subsystem will clear all items and unbind the
     * "On Get Item Children" event.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Redpoint EOS Online Framework|Friend Lists for UMG",
        meta = (DisplayName = "Unbind Friend List Sections from Tree View Widget"))
    void UnbindFriendListSections(UTreeView *TreeView);

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
private:
    void OnUserSlotChanged(
        ::Redpoint::EOS::Identity::FIdentityUserSlot UserSlot,
        ::Redpoint::EOS::Identity::EIdentityUserSlotState OldUserSlotState,
        ::Redpoint::EOS::Identity::FIdentityUserId OldUserId,
        ::Redpoint::EOS::Identity::EIdentityUserSlotState NewUserSlotState,
        ::Redpoint::EOS::Identity::FIdentityUserId NewUserId,
        ::Redpoint::EOS::Identity::EIdentityUserSlotChangeReason ChangeReason);

    void SyncEverything();

    void OnFriendsChanged(
        const ::Redpoint::EOS::Identity::FIdentityUserId &LocalUserId,
        ::Redpoint::EOS::Platform::Types::ERelationshipType ChangedRelationships);

    void OnFriendPresenceChanged(
        const ::Redpoint::EOS::Identity::FIdentityUserId &LocalUserId,
        const ::Redpoint::EOS::Identity::FIdentityUserId &FriendUserId,
        const ::Redpoint::EOS::Presence::FPresenceUserState &NewFriendPresenceState);

    void OnNonUnifiedFriendPresenceChanged(
        const ::Redpoint::EOS::Identity::FIdentityUserId &LocalUserId,
        const ::Redpoint::EOS::UserCache::FExternalUserInfoRef &NonUnifiedFriend,
        const ::Redpoint::EOS::Presence::EPresenceUserActivityState &NewFriendPresenceActivityState);

    void OnRoomJoined(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const ::Redpoint::EOS::Rooms::FRoomRef &Room,
        ::Redpoint::EOS::Rooms::ERoomJoinReason Reason);

    void OnRoomUpdated(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const ::Redpoint::EOS::Rooms::FRoomRef &Room,
        const ::Redpoint::EOS::Rooms::FRoomUpdateInfo &RoomUpdateInfo);

    void OnRoomMemberJoined(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser);

    void OnRoomMemberUpdated(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
        const ::Redpoint::EOS::Rooms::FRoomMemberUpdateInfo &RoomMemberUpdateInfo);

    void OnRoomMemberPromoted(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser);

    void OnRoomMemberLeft(
        const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
        const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
        ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
        ::Redpoint::EOS::Rooms::ERoomLeaveReason Reason);

    static FString GetFriendEntryKey(const UE::Online::FAccountId &UserId);
    static FString GetFriendEntryKey(const ::Redpoint::EOS::UserCache::FExternalUserInfoRef &NonUnifiedFriend);
#endif

public:
    /**
     * Returns the primary party that the local player is currently a member of.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Parties",
        meta = (DisplayName = "Get Primary Party"))
    UParty *GetPrimaryParty() const;

    /**
     * Returns true if the local player is in a primary party and the leader of that primary party.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Parties",
        meta = (DisplayName = "Is Leader of Primary Party?"))
    bool IsPrimaryPartyLeader() const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()