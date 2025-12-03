// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/JoinRoomRequest.h"
#include "RedpointEOSRooms/RoomEventContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3968060140, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyRequestToJoinInfoRedpointEOS : public IOnlinePartyRequestToJoinInfo
{
private:
    FUniqueNetIdRef UserId;
    TSharedRef<FOnlinePartyId> PartyId;
    FRoomEventContext EventContext;
    FRoomRequestedJoinRef RoomRequestedJoin;
    FString EmptyString;
    FDateTime ExpirationDateTime;

public:
    FOnlinePartyRequestToJoinInfoRedpointEOS(
        const FRoomEventContext &InEventContext,
        const FRoomRequestedJoinRef &InRoomRequestedJoin);
    UE_NONCOPYABLE(FOnlinePartyRequestToJoinInfoRedpointEOS);
    virtual ~FOnlinePartyRequestToJoinInfoRedpointEOS() override = default;

    virtual const FUniqueNetIdPtr GetUserId() const override;
    virtual const FString &GetDisplayName() const override;
    virtual const FString &GetPlatformData() const override;
    virtual const FDateTime &GetExpirationTime() const override;
    virtual const FOnlinePartyTypeId GetPartyTypeId() const override;

    const TSharedRef<FOnlinePartyId> &GetPartyId() const;
    const FRoomEventContext &GetRoomEventContext() const;
    const FRoomRequestedJoinRef &GetRoomRequestedJoin() const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3968060140,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyRequestToJoinInfoRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()