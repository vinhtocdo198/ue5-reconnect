// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyJoinInfoRedpointEOSRoomSearchResult.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(412438171, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::FOnlinePartyJoinInfoRedpointEOSRoomSearchResult(
    const FRoomSearchResultRef &InRoomSearchResult)
    : RoomSearchResult(InRoomSearchResult)
    , EmptyString()
{
}

Requests::FJoinRoomRequest FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetJoinRoomRequest(
    const FUniqueNetId &LocalUserId) const
{
    return Requests::FJoinRoomRequest(ERoomScope::User, GetAccountId(LocalUserId), this->RoomSearchResult);
}

FRoomSearchResultPtr FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetRoomSearchResult() const
{
    return this->RoomSearchResult;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::IsValid() const
{
    return true;
}

TSharedRef<const FOnlinePartyId> FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetPartyId() const
{
    return MakeShared<FOnlinePartyIdRedpointEOS>(this->RoomSearchResult->GetRoomId());
}

FOnlinePartyTypeId FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetPartyTypeId() const
{
    return FOnlinePartyTypeId(
        this->RoomSearchResult->GetRoomFeatureSearchStateChecked<Features::IPartyTypeRoomFeatureSearchState>()
            ->PartyTypeId());
}

FUniqueNetIdRef FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetSourceUserId() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room search result."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return FUniqueNetIdEOS::InvalidId();
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetSourceDisplayName() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room search result."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetSourcePlatform() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room search result."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetPlatformData() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room search result."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::HasKey() const
{
    // @note: The EOS SDK has no concept of "keys".
    return false;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::HasPassword() const
{
    return this->RoomSearchResult
        ->GetRoomFeatureSearchStateChecked<Features::IPartyConfigurationRoomFeatureSearchState>()
        ->IsPasswordRequired();
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::IsAcceptingMembers() const
{
    // @note: This is always true because this join info represents an invite that we can use.
    return true;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::IsPartyOfOne() const
{
    // @todo: Should we enable the PublicMemberList feature and compare against the current member count?
    return this->RoomSearchResult->GetRoomMaxMembers() == 1;
}

int32 FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetNotAcceptingReason() const
{
    // @note: The EOS SDK does not track the reason for a party not accepting members.
    return 0;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetAppId() const
{
    // @todo
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::GetBuildId() const
{
    // @todo
    return this->EmptyString;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::CanJoin() const
{
    // @note: This is always true because this join info represents an invite that we can use.
    return true;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::CanJoinWithPassword() const
{
    // If the party has a password, then we can join with a password.
    return this->HasPassword();
}

bool FOnlinePartyJoinInfoRedpointEOSRoomSearchResult::CanRequestAnInvite() const
{
    // @note: The EOS SDK does not support this functionality.
    return false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()