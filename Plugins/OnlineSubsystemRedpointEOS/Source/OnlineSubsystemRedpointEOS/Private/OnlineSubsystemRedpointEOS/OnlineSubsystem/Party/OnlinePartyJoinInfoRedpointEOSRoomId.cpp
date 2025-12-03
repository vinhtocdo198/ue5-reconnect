// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyJoinInfoRedpointEOSRoomId.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2321124487, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

FOnlinePartyJoinInfoRedpointEOSRoomId::FOnlinePartyJoinInfoRedpointEOSRoomId(
    const FRoomIdRef &InRoomId,
    bool bInIsPrimaryParty)
    : RoomId(InRoomId)
    , EmptyString()
    , bIsPrimaryParty(bInIsPrimaryParty)
{
}

Requests::FJoinRoomRequest FOnlinePartyJoinInfoRedpointEOSRoomId::GetJoinRoomRequest(
    const FUniqueNetId &LocalUserId) const
{
    return Requests::FJoinRoomRequest(ERoomScope::User, GetAccountId(LocalUserId), this->RoomId);
}

FRoomSearchResultPtr FOnlinePartyJoinInfoRedpointEOSRoomId::GetRoomSearchResult() const
{
    return nullptr;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::IsKnownToBePrimaryParty() const
{
    return this->bIsPrimaryParty;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::IsValid() const
{
    return true;
}

TSharedRef<const FOnlinePartyId> FOnlinePartyJoinInfoRedpointEOSRoomId::GetPartyId() const
{
    return MakeShared<FOnlinePartyIdRedpointEOS>(this->RoomId);
}

FOnlinePartyTypeId FOnlinePartyJoinInfoRedpointEOSRoomId::GetPartyTypeId() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return FOnlinePartyTypeId();
}

FUniqueNetIdRef FOnlinePartyJoinInfoRedpointEOSRoomId::GetSourceUserId() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return FUniqueNetIdEOS::InvalidId();
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomId::GetSourceDisplayName() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomId::GetSourcePlatform() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomId::GetPlatformData() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::HasKey() const
{
    // @note: The EOS SDK has no concept of "keys".
    return false;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::HasPassword() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return false;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::IsAcceptingMembers() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return true;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::IsPartyOfOne() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return false;
}

int32 FOnlinePartyJoinInfoRedpointEOSRoomId::GetNotAcceptingReason() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return 0;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomId::GetAppId() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

const FString &FOnlinePartyJoinInfoRedpointEOSRoomId::GetBuildId() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return this->EmptyString;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::CanJoin() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return true;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::CanJoinWithPassword() const
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Warning,
        TEXT("%s: This operation is not available on a JoinInfo based purely on a room ID."),
        ANSI_TO_TCHAR(__FUNCTION__));
    return false;
}

bool FOnlinePartyJoinInfoRedpointEOSRoomId::CanRequestAnInvite() const
{
    // @note: The EOS SDK does not support this functionality.
    return false;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()