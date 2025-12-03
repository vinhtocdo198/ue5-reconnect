// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/JoinRoomRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2858216805, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyJoinInfoRedpointEOS : public IOnlinePartyJoinInfo
{
protected:
    FOnlinePartyJoinInfoRedpointEOS() = default;

public:
    UE_NONCOPYABLE(FOnlinePartyJoinInfoRedpointEOS);
    virtual ~FOnlinePartyJoinInfoRedpointEOS() override = default;

    virtual Requests::FJoinRoomRequest GetJoinRoomRequest(const FUniqueNetId &LocalUserId) const = 0;
    virtual FRoomSearchResultPtr GetRoomSearchResult() const = 0;
    virtual bool IsKnownToBePrimaryParty() const
    {
        return false;
    }
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2858216805,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyJoinInfoRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()