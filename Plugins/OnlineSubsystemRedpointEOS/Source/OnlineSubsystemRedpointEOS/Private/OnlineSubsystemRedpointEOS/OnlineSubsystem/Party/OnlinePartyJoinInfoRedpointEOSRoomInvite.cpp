// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyJoinInfoRedpointEOSRoomInvite.h"

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2739172158, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Core::Id;

FOnlinePartyJoinInfoRedpointEOSRoomInvite::FOnlinePartyJoinInfoRedpointEOSRoomInvite(
    const FPlatformHandle &InPlatformHandle,
    const FRoomInvitePtr &InRoomInvite)
    : PlatformHandle(InPlatformHandle)
    , RoomInvite(InRoomInvite)
    , EmptyString()
    , SourceDisplayName()
    , SenderUserInfo()
{
    auto UserCache = PlatformHandle->GetSystem<IUserCacheSystem>();
    this->SenderUserInfo = UserCache->GetUnresolvedUser(
        this->RoomInvite->GetRoomInviteRecipientUserId(),
        this->RoomInvite->GetRoomInviteSenderUserId());
}

Requests::FJoinRoomRequest FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetJoinRoomRequest(
    const FUniqueNetId &LocalUserId) const
{
    return Requests::FJoinRoomRequest(ERoomScope::User, this->RoomInvite.ToSharedRef());
}

FRoomSearchResultPtr FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetRoomSearchResult() const
{
    return this->RoomInvite.IsValid() ? this->RoomInvite->GetRoomInviteSearchResult() : FRoomSearchResultPtr();
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::IsValid() const
{
    return this->RoomInvite.IsValid();
}

TSharedRef<const FOnlinePartyId> FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetPartyId() const
{
    checkf(
        this->RoomInvite.IsValid(),
        TEXT("IsValid must return true before calling %s."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return MakeShared<FOnlinePartyIdRedpointEOS>(this->RoomInvite->GetRoomInviteSearchResult()->GetRoomId());
}

FOnlinePartyTypeId FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetPartyTypeId() const
{
    checkf(
        this->RoomInvite.IsValid(),
        TEXT("IsValid must return true before calling %s."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return FOnlinePartyTypeId(this->RoomInvite->GetRoomInviteSearchResult()
                                  ->GetRoomFeatureSearchStateChecked<Features::IPartyTypeRoomFeatureSearchState>()
                                  ->PartyTypeId());
}

FUniqueNetIdRef FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetSourceUserId() const
{
    checkf(
        this->RoomInvite.IsValid(),
        TEXT("IsValid must return true before calling %s."),
        ANSI_TO_TCHAR(__FUNCTION__));
    auto SourceUserId = GetUniqueNetId(this->RoomInvite->GetRoomInviteSenderUserId());
    if (!SourceUserId.IsValid())
    {
        return FUniqueNetIdEOS::InvalidId();
    }
    return SourceUserId.ToSharedRef();
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetSourceDisplayName() const
{
    const auto &Attributes = this->SenderUserInfo->GetAttributes();
    auto *DisplayName = Attributes.Find(TEXT("displayName"));
    const_cast<FOnlinePartyJoinInfoRedpointEOSRoomInvite *>(this)->SourceDisplayName =
        DisplayName != nullptr ? *DisplayName : this->EmptyString;
    return this->SourceDisplayName;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetSourcePlatform() const
{
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetPlatformData() const
{
    return this->EmptyString;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::HasKey() const
{
    // @note: The EOS SDK has no concept of "keys".
    return false;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::HasPassword() const
{
    checkf(
        this->RoomInvite.IsValid(),
        TEXT("IsValid must return true before calling %s."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->RoomInvite->GetRoomInviteSearchResult()
        ->GetRoomFeatureSearchStateChecked<Features::IPartyConfigurationRoomFeatureSearchState>()
        ->IsPasswordRequired();
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::IsAcceptingMembers() const
{
    // @note: This is always true because this join info represents an invite that we can use.
    return true;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::IsPartyOfOne() const
{
    // @todo: Should we enable the PublicMemberList feature and compare against the current member count?
    return this->RoomInvite->GetRoomInviteSearchResult()->GetRoomMaxMembers() == 1;
}

int32 FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetNotAcceptingReason() const
{
    // @note: The EOS SDK does not track the reason for a party not accepting members.
    return 0;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetAppId() const
{
    // @todo
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomInvite::GetBuildId() const
{
    // @todo
    return this->EmptyString;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::CanJoin() const
{
    // @note: This is always true because this join info represents an invite that we can use.
    return true;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::CanJoinWithPassword() const
{
    // If the party has a password, then we can join with a password.
    return this->HasPassword();
}

bool FOnlinePartyJoinInfoRedpointEOSRoomInvite::CanRequestAnInvite() const
{
    // @note: The EOS SDK does not support this functionality.
    return false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()