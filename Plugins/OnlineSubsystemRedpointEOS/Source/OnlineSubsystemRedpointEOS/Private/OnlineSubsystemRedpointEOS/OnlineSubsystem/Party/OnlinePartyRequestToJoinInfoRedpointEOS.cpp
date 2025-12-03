// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyRequestToJoinInfoRedpointEOS.h"

// #include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"
#include "RedpointEOSRooms/RoomEventContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3968060140, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;

FOnlinePartyRequestToJoinInfoRedpointEOS::FOnlinePartyRequestToJoinInfoRedpointEOS(
    const FRoomEventContext &InEventContext,
    const FRoomRequestedJoinRef &InRoomRequestedJoin)
    : UserId(GetUniqueNetIdRef(InEventContext.LocalUserId))
    , PartyId(MakeShared<FOnlinePartyIdRedpointEOS>(InRoomRequestedJoin->GetRoomSearchResult()->GetRoomId()))
    , EventContext(InEventContext)
    , RoomRequestedJoin(InRoomRequestedJoin)
    , EmptyString()
    , ExpirationDateTime(FDateTime::MaxValue())
{
}

const FUniqueNetIdPtr FOnlinePartyRequestToJoinInfoRedpointEOS::GetUserId() const
{
    return this->UserId;
}

const FString &FOnlinePartyRequestToJoinInfoRedpointEOS::GetDisplayName() const
{
    return this->EmptyString;
}

const FString &FOnlinePartyRequestToJoinInfoRedpointEOS::GetPlatformData() const
{
    return this->EmptyString;
}

const FDateTime &FOnlinePartyRequestToJoinInfoRedpointEOS::GetExpirationTime() const
{
    return this->ExpirationDateTime;
}

const FOnlinePartyTypeId FOnlinePartyRequestToJoinInfoRedpointEOS::GetPartyTypeId() const
{
    auto SearchState =
        this->RoomRequestedJoin->GetRoomSearchResult()->GetRoomFeatureSearchState<IPartyTypeRoomFeatureSearchState>();
    if (SearchState.IsValid())
    {
        return FOnlinePartyTypeId(SearchState->PartyTypeId());
    }

    // @note: We should never hit this point as the request to join will only be created if the party type room feature
    // existed on the search state originally.
    return FOnlinePartyTypeId(0);
}

const TSharedRef<FOnlinePartyId> &FOnlinePartyRequestToJoinInfoRedpointEOS::GetPartyId() const
{
    return this->PartyId;
}

const FRoomEventContext &FOnlinePartyRequestToJoinInfoRedpointEOS::GetRoomEventContext() const
{
    return this->EventContext;
}

const FRoomRequestedJoinRef &FOnlinePartyRequestToJoinInfoRedpointEOS::GetRoomRequestedJoin() const
{
    return this->RoomRequestedJoin;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()