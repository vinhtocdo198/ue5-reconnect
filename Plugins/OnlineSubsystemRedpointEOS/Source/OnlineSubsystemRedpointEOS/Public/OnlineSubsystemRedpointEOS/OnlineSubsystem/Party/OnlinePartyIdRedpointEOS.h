// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1949368713, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartyIdRedpointEOS : public FOnlinePartyIdRedpointBase
{
public:
    FOnlinePartyIdRedpointEOS(const FRoomIdRef &InRoomId);
    FOnlinePartyIdRedpointEOS();
    UE_NONCOPYABLE(FOnlinePartyIdRedpointEOS);
    virtual ~FOnlinePartyIdRedpointEOS() override = default;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1949368713,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartyIdRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()