// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFrameworkLocalPlayerSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "LogRedpointEOSFramework.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/AccessPrivate.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSFriends/FriendSystem.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

struct FRedpointTreeViewOnGetItemChildrenAccessor
{
    using Member = TSlateDelegates<UObject *>::FOnGetChildren UTreeView::*;
    friend Member GetPrivate(FRedpointTreeViewOnGetItemChildrenAccessor);
};
template struct TRedpointAccessPrivate<FRedpointTreeViewOnGetItemChildrenAccessor, &UTreeView::OnGetItemChildren>;

URedpointFrameworkLocalPlayerSubsystem::URedpointFrameworkLocalPlayerSubsystem(
    const FObjectInitializer &ObjectInitializer)
    : ULocalPlayerSubsystem()
    , bInitialized(false)
    , FriendSections()
    , FriendEntries()
    , bDisplayCachedPlatformFriends(false)
    , BoundTreeViews()
{
}

void URedpointFrameworkLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    this->TryInitialize();
#endif
}

void URedpointFrameworkLocalPlayerSubsystem::PlayerControllerChanged(APlayerController *NewPlayerController)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    // Player controller changed. Assume that the user ID might have changed and clear friend sections.
    for (const auto &SectionKV : this->FriendSections)
    {
        SectionKV.Value->Entries.Reset();
        SectionKV.Value->bRequiresInit = true;
    }
    this->FriendEntries.Reset();
#endif
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void URedpointFrameworkLocalPlayerSubsystem::TryInitialize()
{
    if (this->bInitialized)
    {
        return;
    }

    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Friends;
    using namespace ::Redpoint::EOS::Identity;
    using namespace ::Redpoint::EOS::Platform::Types;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::UserCache;

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetLocalPlayerChecked()->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        return;
    }

    auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
    IdentitySystem->OnUserSlotChanged().AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnUserSlotChanged);

    auto FriendSystem = PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->OnFriendsChanged().AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnFriendsChanged);
    FriendSystem->OnFriendPresenceChanged().AddUObject(
        this,
        &URedpointFrameworkLocalPlayerSubsystem::OnFriendPresenceChanged);
    FriendSystem->OnNonUnifiedFriendPresenceChanged().AddUObject(
        this,
        &URedpointFrameworkLocalPlayerSubsystem::OnNonUnifiedFriendPresenceChanged);

    auto RoomSystem = PlatformHandle->GetSystem<IRoomSystem>();
    RoomSystem->OnRoomJoined(NAME_Game).AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnRoomJoined);
    RoomSystem->OnRoomUpdated(NAME_Game).AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnRoomUpdated);

    this->bInitialized = true;
}

#endif

void URedpointFrameworkLocalPlayerSubsystem::SetDisplayCachedPlatformFriends(bool bInDisplayCachedPlatformFriends)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    this->bDisplayCachedPlatformFriends = bInDisplayCachedPlatformFriends;
    this->SyncEverything();
#endif
}

void URedpointFrameworkLocalPlayerSubsystem::SetIncludeLocalPlayerInPartySection(
    bool bInIncludeLocalPlayerInPartySection)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    this->bIncludeLocalPlayerInPartySection = bInIncludeLocalPlayerInPartySection;
    this->SyncEverything();
#endif
}

void URedpointFrameworkLocalPlayerSubsystem::BindFriendListSections(
    UTreeView *TreeView,
    const TArray<ERedpointFriendListSectionType> &Sections)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    this->TryInitialize();
    if (!this->bInitialized)
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Error,
            TEXT("This game is not configured to use the Redpoint EOS Online Framework as the online subsystem."));
        return;
    }

    bool bShouldClearSelectedItem = false;
    if (!this->BoundTreeViews.Contains(TreeView))
    {
        ((*TreeView).*GetPrivate(FRedpointTreeViewOnGetItemChildrenAccessor()))
            .BindWeakLambda(this, [this](UObject *Parent, TArray<UObject *> &OutChildren) {
                auto *Section = Cast<URedpointFriendListSection>(Parent);
                if (!Section)
                {
                    return;
                }

                auto *OurSection = this->FriendSections.Find(Section->Type);
                if (OurSection == nullptr || *OurSection != Section)
                {
                    return;
                }

                if ((*OurSection)->bRequiresInit)
                {
                    this->SyncEverything();
                    (*OurSection)->bRequiresInit = false;
                }

                for (const auto &Entry : (*OurSection)->Entries)
                {
                    OutChildren.Add(Entry);
                }
            });

        bShouldClearSelectedItem = true;
    }

    for (const auto &Section : Sections)
    {
        auto &Ptr = this->FriendSections.FindOrAdd(Section);
        if (!Ptr)
        {
            Ptr = NewObject<URedpointFriendListSection>(this);
            Ptr->Type = Section;
            switch (Ptr->Type)
            {
            case ERedpointFriendListSectionType::InParty:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "InParty", "In Party");
                break;
            case ERedpointFriendListSectionType::InGame:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "InGame", "In Game");
                break;
            case ERedpointFriendListSectionType::Online:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "Online", "Online");
                break;
            case ERedpointFriendListSectionType::Offline:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "Offline", "Offline");
                break;
            case ERedpointFriendListSectionType::Recent:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "Recent", "Recent");
                break;
            case ERedpointFriendListSectionType::Blocked:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "Blocked", "Blocked");
                break;
            default:
                Ptr->Title = NSLOCTEXT("RedpointEOSFramework", "Unknown", "Unknown");
                break;
            }
            Ptr->bRequiresInit = true;
        }
    }

    this->BoundTreeViews.Add(TreeView, Sections);

    this->SyncEverything();

    if (bShouldClearSelectedItem)
    {
        TreeView->SetSelectedItem(nullptr);

        UE_LOG(LogRedpointEOSFramework, Verbose, TEXT("Bound new UMG tree view widget to friend list."));
    }
#endif
}

void URedpointFrameworkLocalPlayerSubsystem::UnbindFriendListSections(UTreeView *TreeView)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (this->BoundTreeViews.Contains(TreeView))
    {
        ((*TreeView).*GetPrivate(FRedpointTreeViewOnGetItemChildrenAccessor())).Unbind();
        TreeView->SetListItems(TArray<UObject *>());
        this->BoundTreeViews.Remove(TreeView);

        UE_LOG(LogRedpointEOSFramework, Verbose, TEXT("Unbound existing UMG tree view widget to friend list."));
    }
#endif
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void URedpointFrameworkLocalPlayerSubsystem::OnUserSlotChanged(
    ::Redpoint::EOS::Identity::FIdentityUserSlot UserSlot,
    ::Redpoint::EOS::Identity::EIdentityUserSlotState OldUserSlotState,
    ::Redpoint::EOS::Identity::FIdentityUserId OldUserId,
    ::Redpoint::EOS::Identity::EIdentityUserSlotState NewUserSlotState,
    ::Redpoint::EOS::Identity::FIdentityUserId NewUserId,
    ::Redpoint::EOS::Identity::EIdentityUserSlotChangeReason ChangeReason)
{
    ULocalPlayer *LocalPlayer =
        GEngine->GetLocalPlayerFromControllerId(this->GetLocalPlayerChecked()->GetWorld(), UserSlot);
    if (LocalPlayer == this->GetLocalPlayerChecked() && OldUserId != NewUserId)
    {
        // This event is for our local player and the user ID is changing. Reset our friend sections array.
        for (const auto &SectionKV : this->FriendSections)
        {
            SectionKV.Value->Entries.Reset();
            SectionKV.Value->bRequiresInit = true;
        }
        this->FriendEntries.Reset();
    }
}

void URedpointFrameworkLocalPlayerSubsystem::SyncEverything()
{
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Friends;
    using namespace ::Redpoint::EOS::Platform::Types;
    using namespace ::Redpoint::EOS::Presence;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::UserCache;

    if (this->FriendSections.Num() == 0)
    {
        // If the game isn't using the friend list blueprints, and hasn't retrieved any sections, skip doing any work at
        // all to avoid any performance penalty from an unused feature.
        return;
    }

    this->TryInitialize();
    if (!this->bInitialized)
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Error,
            TEXT("This game is not configured to use the Redpoint EOS Online Framework as the online subsystem."));
        return;
    }

    TArray<TSoftObjectPtr<UTreeView>> TreeViewsGC;
    for (const auto &KV : this->BoundTreeViews)
    {
        if (!KV.Key.IsValid())
        {
            TreeViewsGC.Remove(KV.Key);
        }
    }
    for (const auto &Key : TreeViewsGC)
    {
        this->BoundTreeViews.Remove(Key);
    }

    auto LocalUserId = GetAccountId(this->GetLocalPlayerChecked()->GetCachedUniqueNetId());

    auto PlatformHandle = FWorldResolution::GetPlatformHandle(this->GetLocalPlayerChecked()->GetWorld());

    TAccountIdMap<FUserInfoPtr> UnifiedUsers;
    TArray<FExternalUserInfoRef> NonUnifiedFriends;
    TSet<FString> SeenIds;

    // Get members of the current party, excluding the local user.
    TAccountIdMap<FUserInfoRef> PartyMembers;
    auto RoomSystem = PlatformHandle->GetSystem<IRoomSystem>();
    auto UserCacheSystem = PlatformHandle->GetSystem<IUserCacheSystem>();
    bool bIsLocalPlayerPartyLeader = false;
    FRoomUserId PartyLeaderUserId;
    auto Rooms = RoomSystem->GetCurrentRooms(FGetCurrentRoomsRequest(ERoomScope::User, LocalUserId, NAME_Game));
    for (const auto &Room : Rooms)
    {
        auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
        if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
        {
            PartyLeaderUserId = Room->GetRoomOwner()->GetUserId();
            bIsLocalPlayerPartyLeader = Room->GetRoomOwner()->GetUserId() == LocalUserId;
            for (const auto &Member : Room->GetMembers())
            {
                if (Member->GetUserId() != LocalUserId || this->bIncludeLocalPlayerInPartySection)
                {
                    auto PartyMember = UserCacheSystem->GetUnresolvedUser(LocalUserId, Member->GetUserId());
                    if (!PartyMember->IsReady())
                    {
                        UserCacheSystem->GetUser(
                            LocalUserId,
                            Member->GetUserId(),
                            IUserCacheSystem::FOnGetUserComplete::CreateWeakLambda(
                                this,
                                [this](const FError &ErrorCode, const FUserInfoPtr &) {
                                    if (ErrorCode.WasSuccessful())
                                    {
                                        this->SyncEverything();
                                    }
                                }));
                    }
                    PartyMembers.Add(Member->GetUserId(), PartyMember);
                    SeenIds.Add(GetFriendEntryKey(Member->GetUserId()));
                }
            }
        }
    }

    // Get friends.
    auto FriendSystem = PlatformHandle->GetSystem<IFriendSystem>();
    TAccountIdMap<FUserInfoRef> UnifiedFriends;
    TAccountIdMap<FUserInfoRef> InboundFriendRequests;
    TAccountIdMap<FUserInfoRef> OutboundFriendRequests;
    TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> RecentPlayers;
    TAccountIdMap<FUserInfoPtr> BlockedPlayers;
    if (FriendSystem->GetFriends(LocalUserId, UnifiedFriends, NonUnifiedFriends).WasSuccessful())
    {
        for (const auto &Friend : UnifiedFriends)
        {
            UnifiedUsers.Add(Friend.Key, Friend.Value);
        }
    }
    if (FriendSystem->GetInboundFriendRequests(LocalUserId, InboundFriendRequests).WasSuccessful())
    {
        for (const auto &Friend : InboundFriendRequests)
        {
            if (!UnifiedUsers.Contains(Friend.Key))
            {
                UnifiedUsers.Add(Friend.Key, Friend.Value);
            }
        }
    }
    if (FriendSystem->GetOutboundFriendRequests(LocalUserId, OutboundFriendRequests).WasSuccessful())
    {
        for (const auto &Friend : OutboundFriendRequests)
        {
            if (!UnifiedUsers.Contains(Friend.Key))
            {
                UnifiedUsers.Add(Friend.Key, Friend.Value);
            }
        }
    }
    if (FriendSystem->GetRecentPlayers(LocalUserId, RecentPlayers).WasSuccessful())
    {
        for (const auto &Friend : RecentPlayers)
        {
            if (!UnifiedUsers.Contains(Friend.Key))
            {
                UnifiedUsers.Add(Friend.Key, Friend.Value.Key);
            }
        }
    }
    if (FriendSystem->GetBlockedPlayers(LocalUserId, BlockedPlayers).WasSuccessful())
    {
        for (const auto &Friend : BlockedPlayers)
        {
            if (!UnifiedUsers.Contains(Friend.Key))
            {
                UnifiedUsers.Add(Friend.Key, Friend.Value);
            }
        }
    }

    // Add and sync any unified users and non-unified friends to the friend cache.
    for (const auto &UnifiedUser : UnifiedUsers)
    {
        auto Id = GetFriendEntryKey(UnifiedUser.Key);
        SeenIds.Add(Id);

        auto &Ptr = this->FriendEntries.FindOrAdd(Id);
        if (!Ptr)
        {
            Ptr = NewObject<URedpointFriendListEntry>(this);
            Ptr->bHasGameProfile = true;
            Ptr->BoundForLocalUserId = LocalUserId;
            Ptr->BoundForLocalPlayer = this->GetLocalPlayerChecked();
            Ptr->UserId = GetUniqueNetIdRepl(UnifiedUser.Key);
        }

        if (UnifiedUser.Value.IsValid())
        {
            Ptr->DisplayName = UnifiedUser.Value->GetAttributes().FindRef(TEXT("displayName"));
        }

        if (UnifiedUser.Key == LocalUserId)
        {
            // This is a reference to the local user, which will happen if the local user should be displayed in the
            // party.
            Ptr->bIsSelf = true;
            Ptr->bIsGameFriend = false;
            Ptr->bInParty = true;
            Ptr->bIsInvitable = false;
            Ptr->bHasAuthorityOverPartyMember = bIsLocalPlayerPartyLeader;
            Ptr->bIsCurrentPartyLeader = UnifiedUser.Key == PartyLeaderUserId;
            Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::MutualFriends;
            Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Online;
            Ptr->bPlayingThisGame = true;
            Ptr->bInJoinableParty = false;
            continue;
        }

        Ptr->bIsSelf = false;

        if (UnifiedUser.Value.IsValid())
        {
            Ptr->bIsGameFriend = FriendSystem->IsFriendDeletable(LocalUserId, UnifiedUser.Key);
        }

        Ptr->bInParty = PartyMembers.Contains(UnifiedUser.Key);
        Ptr->bIsInvitable = true;
        Ptr->bHasAuthorityOverPartyMember = bIsLocalPlayerPartyLeader;
        Ptr->bIsCurrentPartyLeader = UnifiedUser.Key == PartyLeaderUserId;

        Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::Unknown;
        Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Offline;

        auto Presence = FriendSystem->GetUnifiedFriendPresence(LocalUserId, UnifiedUser.Key);
        if (Presence.IsSet())
        {
            Ptr->bPlayingThisGame = Presence->bIsPlayingThisGame;
            Ptr->bInJoinableParty = Presence->AdvertisedParty.IsSet() && Presence->AdvertisedParty->bJoinable;
            // @note: There is no "bInJoinableGameSession" yet because game sessions are only advertised on runtime
            // platform integrations and not via cross-platform presence yet. This will be available after we finish
            // refactoring sessions to use the room system.

            switch (Presence->ActivityState)
            {
            case EPresenceUserActivityState::Online:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Online;
                break;
            case EPresenceUserActivityState::Idle:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Idle;
                break;
            case EPresenceUserActivityState::DoNotDisturb:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::DoNotDisturb;
                break;
            case EPresenceUserActivityState::Offline:
            default:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Offline;
                break;
            }
        }
        else
        {
            Ptr->bPlayingThisGame = false;
            Ptr->bInJoinableParty = false;

            if (UnifiedUser.Value.IsValid())
            {
                auto ExternalPresence =
                    FriendSystem->GetNonUnifiedFriendPresence(LocalUserId, UnifiedUser.Value.ToSharedRef());
                if (ExternalPresence.IsSet())
                {
                    switch (*ExternalPresence)
                    {
                    case EPresenceUserActivityState::Online:
                        Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Online;
                        break;
                    case EPresenceUserActivityState::Idle:
                        Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Idle;
                        break;
                    case EPresenceUserActivityState::DoNotDisturb:
                        Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::DoNotDisturb;
                        break;
                    case EPresenceUserActivityState::Offline:
                    default:
                        Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Offline;
                        break;
                    }
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSFramework,
                        Verbose,
                        TEXT("There is no unified presence and no non-unified presence for target friend with user ID "
                             "'%s'."),
                        *GetUserIdString(UnifiedUser.Key));
                }
            }
        }

        if (UnifiedFriends.Contains(UnifiedUser.Key))
        {
            Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::MutualFriends;
        }
        else if (InboundFriendRequests.Contains(UnifiedUser.Key))
        {
            Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::PendingInbound;
        }
        else if (OutboundFriendRequests.Contains(UnifiedUser.Key))
        {
            Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::PendingOutbound;
        }
        else if (RecentPlayers.Contains(UnifiedUser.Key))
        {
            Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::Recent;
        }
        else if (BlockedPlayers.Contains(UnifiedUser.Key))
        {
            Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::Blocked;
        }
    }
    for (const auto &NonUnifiedFriend : NonUnifiedFriends)
    {
        auto Id = GetFriendEntryKey(NonUnifiedFriend);
        SeenIds.Add(Id);

        if (!this->FriendEntries.Contains(Id))
        {
            // Wrangle a local subsystem user ID so that avatars can work.
            //
            // @todo: Figure out a better way to round trip a non-unified friend ID to the avatar UMG widget than
            // FUniqueNetIdRepl.
            FUniqueNetIdRepl NonUnifiedUniqueNetIdRepl;
            bool bIsInvitable = false;
            TOptional<TTuple<EOS_EExternalAccountType, FString>> PrimaryExternalUserId;
            for (const auto &KV : NonUnifiedFriend->GetLinkedExternalAccounts())
            {
                PrimaryExternalUserId = TTuple<EOS_EExternalAccountType, FString>(KV.Key, KV.Value.AccountId);
            }
            if (PrimaryExternalUserId.IsSet())
            {
                FName SyntheticSubsystem = NAME_None;
                if (PrimaryExternalUserId->Get<0>() == EOS_EExternalAccountType::EOS_EAT_STEAM)
                {
                    SyntheticSubsystem = STEAM_SUBSYSTEM;
                }
                else if (
                    PrimaryExternalUserId->Get<0>() == EOS_EExternalAccountType::EOS_EAT_XBL ||
                    PrimaryExternalUserId->Get<0>() == EOS_EExternalAccountType::EOS_EAT_PSN ||
                    PrimaryExternalUserId->Get<0>() == EOS_EExternalAccountType::EOS_EAT_NINTENDO)
                {
                    auto PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
                    if (PlatformSubsystem != nullptr)
                    {
                        SyntheticSubsystem = PlatformSubsystem->GetSubsystemName();
                    }
                }

                if (!SyntheticSubsystem.IsNone())
                {
                    bool bDidSetUserId = false;
                    auto *Subsystem = Online::GetSubsystem(this->GetWorld(), SyntheticSubsystem);
                    if (Subsystem != nullptr)
                    {
                        auto Friends = Subsystem->GetFriendsInterface();
                        auto Identity = Subsystem->GetIdentityInterface();
                        if (Identity.IsValid() && Friends.IsValid())
                        {
                            auto SyntheticFriendId = Identity->CreateUniquePlayerId(PrimaryExternalUserId->Get<1>());
                            if (SyntheticFriendId.IsValid())
                            {
                                NonUnifiedUniqueNetIdRepl = SyntheticFriendId;
                                bIsInvitable = true;
                                bDidSetUserId = true;
                            }
                        }
                    }

                    if (!bDidSetUserId)
                    {
                        if (!this->bDisplayCachedPlatformFriends)
                        {
                            // This cached friend should not be included, since the user can not act on them in any
                            // meaningful way.
                            continue;
                        }
                        else
                        {
                            // This is extremely sketchy, but required to make avatars work for external friends for
                            // whom the subsystem is not yet available, but we have an external avatar URL cached in the
                            // friends database.
                            FUniqueNetIdRef UserIdString =
                                FUniqueNetIdString::Create(PrimaryExternalUserId->Get<1>(), SyntheticSubsystem);
                            NonUnifiedUniqueNetIdRepl = UserIdString;
                            bIsInvitable = false;
                        }
                    }
                }
            }

            auto NewPtr = NewObject<URedpointFriendListEntry>(this);
            NewPtr->bHasGameProfile = false;
            NewPtr->bIsGameFriend = false;
            NewPtr->BoundForLocalUserId = LocalUserId;
            NewPtr->BoundForLocalPlayer = this->GetLocalPlayerChecked();
            NewPtr->UserId = NonUnifiedUniqueNetIdRepl;
            NewPtr->bIsInvitable = bIsInvitable;
            NewPtr->bIsSelf = false;
            this->FriendEntries.Add(Id, NewPtr);
        }

        auto &Ptr = this->FriendEntries.FindChecked(Id);

        for (const auto &ExternalLinkedAccount : NonUnifiedFriend->GetLinkedExternalAccounts())
        {
            Ptr->DisplayName = ExternalLinkedAccount.Value.DisplayName;
            break;
        }

        Ptr->bInParty = false;
        Ptr->bHasAuthorityOverPartyMember = false;
        Ptr->bIsCurrentPartyLeader = false;

        // @note: The friend system should only return non-unified friends that are mutual friends.
        Ptr->InviteStatus = ERedpointFriendListEntryInviteStatus::MutualFriends;
        Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Offline;

        Ptr->bPlayingThisGame = false;
        Ptr->bInJoinableParty = false;

        auto Presence = FriendSystem->GetNonUnifiedFriendPresence(LocalUserId, NonUnifiedFriend);
        if (Presence.IsSet())
        {
            switch (*Presence)
            {
            case EPresenceUserActivityState::Online:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Online;
                break;
            case EPresenceUserActivityState::Idle:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Idle;
                break;
            case EPresenceUserActivityState::DoNotDisturb:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::DoNotDisturb;
                break;
            case EPresenceUserActivityState::Offline:
            default:
                Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Offline;
                break;
            }
        }
        else
        {
            Ptr->PresenceStatus = ERedpointFriendListEntryPresenceStatus::Offline;
        }
    }

    // Remove any friend entries that no longer exist.
    TArray<FString> ExistingKeys;
    this->FriendEntries.GetKeys(ExistingKeys);
    for (const auto &Key : ExistingKeys)
    {
        if (!SeenIds.Contains(Key))
        {
            this->FriendEntries.Remove(Key);
        }
    }

    // Sync sections.
    TMap<ERedpointFriendListSectionType, TArray<TObjectPtr<URedpointFriendListEntry>>> PreviousSectionEntries;
    bool bRefreshRequired = false;
    for (const auto &SectionKV : this->FriendSections)
    {
        PreviousSectionEntries.Add(SectionKV.Key, SectionKV.Value->Entries);
        SectionKV.Value->Entries.Reset();
    }
    for (const auto &EntryKV : this->FriendEntries)
    {
        auto *Section = this->FriendSections.Find(EntryKV.Value->GetSectionType());
        if (Section && IsValid(*Section))
        {
            (*Section)->Entries.Add(EntryKV.Value);
        }
    }
    for (const auto &SectionKV : this->FriendSections)
    {
#if REDPOINT_EOS_UE_5_6_OR_LATER
        SectionKV.Value->Entries.Sort([](URedpointFriendListEntry &A, URedpointFriendListEntry &B) -> bool {
            return A.IsOrderedLessThan(&B);
        });
#else
        SectionKV.Value->Entries.Sort(
            [](const TObjectPtr<URedpointFriendListEntry> &A, const TObjectPtr<URedpointFriendListEntry> &B) -> bool {
                return A->IsOrderedLessThan(B);
            });
#endif
        SectionKV.Value->bRequiresInit = false;
        if (SectionKV.Value->Entries != PreviousSectionEntries[SectionKV.Key])
        {
            UE_LOG(
                LogRedpointEOSFramework,
                Verbose,
                TEXT("Detected that a friend section no longer has the same child entries; will request a general "
                     "refresh of "
                     "tree views..."));
            bRefreshRequired = true;
        }
    }

    // Sync tree views.
    for (const auto &KV : this->BoundTreeViews)
    {
        if (!KV.Key.IsValid())
        {
            continue;
        }

        TArray<UObject *> ExpectedRootItems;
        for (const auto &SectionType : KV.Value)
        {
            auto *Ptr = this->FriendSections.Find(SectionType);
            if (Ptr != nullptr && (*Ptr)->Entries.Num() > 0)
            {
                ExpectedRootItems.Add(*Ptr);
            }
        }

        if (KV.Key->GetListItems() != ExpectedRootItems)
        {
            // The visible sections have changed because at least one section became non-empty or empty.
            UE_LOG(
                LogRedpointEOSFramework,
                Verbose,
                TEXT("The root items (sections) in a tree view changed, setting the new root items and expanding "
                     "everything..."));
            bool bDidFirstItemChange = false;
            if ((KV.Key->GetListItems().Num() > 0) != (ExpectedRootItems.Num() > 0))
            {
                bDidFirstItemChange = true;
            }
            else if (KV.Key->GetListItems().Num() > 0 && KV.Key->GetListItems()[0] != ExpectedRootItems[0])
            {
                bDidFirstItemChange = true;
            }
            KV.Key->SetListItems<UObject *>(ExpectedRootItems);
            if ((KV.Key->GetVerticalEntrySpacing() > 0.0f || KV.Key->GetHorizontalEntrySpacing() > 0.0f) &&
                bDidFirstItemChange)
            {
                // If there is spacing and the current first item got moved down and a new one is put in it's place, the
                // item that got moved down will have incorrect spacing applied. We have to call RegenerateAllEntries to
                // force all rows to be recreated, so that the spacing is set up correctly again.
                KV.Key->RegenerateAllEntries();
            }
        }

        if (bRefreshRequired)
        {
            UE_LOG(
                LogRedpointEOSFramework,
                Verbose,
                TEXT("Requesting a general refresh of the tree view as one or more sections had child entries change "
                     "earlier."));
            KV.Key->RequestRefresh();
        }

        KV.Key->ExpandAll();
    }
}

void URedpointFrameworkLocalPlayerSubsystem::OnFriendsChanged(
    const ::Redpoint::EOS::Identity::FIdentityUserId &LocalUserId,
    ::Redpoint::EOS::Platform::Types::ERelationshipType ChangedRelationships)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (LocalUserId != GetAccountId(this->GetLocalPlayerChecked()->GetCachedUniqueNetId()))
    {
        return;
    }

    UE_LOG(LogRedpointEOSFramework, Verbose, TEXT("Notified that friends changed; syncing friend lists for UMG..."));
    this->SyncEverything();
}

void URedpointFrameworkLocalPlayerSubsystem::OnFriendPresenceChanged(
    const ::Redpoint::EOS::Identity::FIdentityUserId &LocalUserId,
    const ::Redpoint::EOS::Identity::FIdentityUserId &FriendUserId,
    const ::Redpoint::EOS::Presence::FPresenceUserState &NewFriendPresenceState)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (LocalUserId != GetAccountId(this->GetLocalPlayerChecked()->GetCachedUniqueNetId()))
    {
        return;
    }

    // @todo: Optimize this so we only update the specific friend.
    UE_LOG(
        LogRedpointEOSFramework,
        Verbose,
        TEXT("Notified that a unified friend's presence changed; syncing friend lists for UMG..."));
    this->SyncEverything();
}

void URedpointFrameworkLocalPlayerSubsystem::OnNonUnifiedFriendPresenceChanged(
    const ::Redpoint::EOS::Identity::FIdentityUserId &LocalUserId,
    const ::Redpoint::EOS::UserCache::FExternalUserInfoRef &NonUnifiedFriend,
    const ::Redpoint::EOS::Presence::EPresenceUserActivityState &NewFriendPresenceActivityState)
{
    using namespace ::Redpoint::EOS::Core::Id;

    if (LocalUserId != GetAccountId(this->GetLocalPlayerChecked()->GetCachedUniqueNetId()))
    {
        return;
    }

    // @todo: Optimize this so we only update the specific friend.
    UE_LOG(
        LogRedpointEOSFramework,
        Verbose,
        TEXT("Notified that a non-unified friend's presence changed; syncing friend lists for UMG..."));
    this->SyncEverything();
}

void URedpointFrameworkLocalPlayerSubsystem::OnRoomJoined(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const ::Redpoint::EOS::Rooms::FRoomRef &Room,
    ::Redpoint::EOS::Rooms::ERoomJoinReason Reason)
{
    using namespace ::Redpoint::EOS::Rooms::Features;

    auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // Sync now.
        // @todo: Optimize this so we only update the specific party members.
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("Notified that the user joined a new primary party; syncing friend lists for UMG..."));
        this->SyncEverything();

        Room->OnRoomMemberJoined().AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberJoined);
        Room->OnRoomMemberUpdated().AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberUpdated);
        Room->OnRoomMemberPromoted().AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberPromoted);
        Room->OnRoomMemberLeft().AddUObject(this, &URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberLeft);
    }
}

void URedpointFrameworkLocalPlayerSubsystem::OnRoomUpdated(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const ::Redpoint::EOS::Rooms::FRoomRef &Room,
    const ::Redpoint::EOS::Rooms::FRoomUpdateInfo &RoomUpdateInfo)
{
    using namespace ::Redpoint::EOS::Rooms::Features;

    auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // Sync now.
        // @todo: Optimize this so we only update the specific party members.
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("Notified that the user's primary party was updated; syncing friend lists for UMG..."));
        this->SyncEverything();
    }
}

void URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberJoined(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser)
{
    using namespace ::Redpoint::EOS::Rooms::Features;

    auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // Sync now.
        // @todo: Optimize this so we only update the specific party members.
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("Notified that a new member joined the user's primary party; syncing friend lists for UMG..."));
        this->SyncEverything();
    }
}

void URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberUpdated(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
    const ::Redpoint::EOS::Rooms::FRoomMemberUpdateInfo &RoomMemberUpdateInfo)
{
    using namespace ::Redpoint::EOS::Rooms::Features;

    auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // Sync now.
        // @todo: Optimize this so we only update the specific party members.
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("Notified that an existing member was updated in the user's primary party; syncing friend lists for "
                 "UMG..."));
        this->SyncEverything();
    }
}

void URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberPromoted(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser)
{
    using namespace ::Redpoint::EOS::Rooms::Features;

    auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // Sync now.
        // @todo: Optimize this so we only update the specific party members.
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("Notified that the party leader changed for user's primary party; syncing friend lists for UMG..."));
        this->SyncEverything();
    }
}

void URedpointFrameworkLocalPlayerSubsystem::OnRoomMemberLeft(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
    ::Redpoint::EOS::Rooms::ERoomLeaveReason Reason)
{
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Rooms::Features;

    if (EventContext.LocalUserId == GetAccountId(this->GetLocalPlayerChecked()->GetCachedUniqueNetId()))
    {
        return;
    }

    auto PartyTypeRoomFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (PartyTypeRoomFeature.IsValid() && PartyTypeRoomFeature->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // Sync now.
        // @todo: Optimize this so we only update the specific party members.
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("Notified that an existing party member left the user's primary party; syncing friend lists for "
                 "UMG..."));
        this->SyncEverything();
    }
}

FString URedpointFrameworkLocalPlayerSubsystem::GetFriendEntryKey(const UE::Online::FAccountId &UserId)
{
    using namespace ::Redpoint::EOS::Core::Id;

    return FString::Printf(TEXT("EOS:%s"), *GetUserIdString(UserId));
}

FString URedpointFrameworkLocalPlayerSubsystem::GetFriendEntryKey(
    const ::Redpoint::EOS::UserCache::FExternalUserInfoRef &NonUnifiedFriend)
{
    using namespace ::Redpoint::EOS::API::Common;

    for (const auto &KV : NonUnifiedFriend->GetLinkedExternalAccounts())
    {
        return FString::Printf(TEXT("%s:%s"), *ExternalAccountTypeMap.Get(KV.Key), *KV.Value.AccountId);
    }

    return TEXT("Unknown");
}

#endif

UParty *URedpointFrameworkLocalPlayerSubsystem::GetPrimaryParty() const
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto *OSSv1 = Online::GetSubsystem(this->GetLocalPlayerChecked()->GetWorld());
    if (OSSv1 == nullptr)
    {
        return nullptr;
    }

    auto PartySystem = OSSv1->GetPartyInterface();
    if (!PartySystem.IsValid())
    {
        return nullptr;
    }

    auto UserId = this->GetLocalPlayerChecked()->GetCachedUniqueNetId();
    if (!UserId.IsValid())
    {
        return nullptr;
    }

    auto PrimaryParty = PartySystem->GetParty(*UserId, PartySystem->GetPrimaryPartyTypeId());
    if (!PrimaryParty.IsValid())
    {
        return nullptr;
    }

    return UParty::FromNative(PrimaryParty);
#else
    return nullptr;
#endif
}

bool URedpointFrameworkLocalPlayerSubsystem::IsPrimaryPartyLeader() const
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto *OSSv1 = Online::GetSubsystem(this->GetLocalPlayerChecked()->GetWorld());
    if (OSSv1 == nullptr)
    {
        return false;
    }

    auto PartySystem = OSSv1->GetPartyInterface();
    if (!PartySystem.IsValid())
    {
        return false;
    }

    auto UserId = this->GetLocalPlayerChecked()->GetCachedUniqueNetId();
    if (!UserId.IsValid())
    {
        return false;
    }

    auto PrimaryParty = PartySystem->GetParty(*UserId, PartySystem->GetPrimaryPartyTypeId());
    if (!PrimaryParty.IsValid())
    {
        return false;
    }

    return PartySystem->IsMemberLeader(*UserId, *PrimaryParty->PartyId, *UserId);
#else
    return false;
#endif
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()