// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1949368713, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{

FOnlinePartyIdRedpointEOS::FOnlinePartyIdRedpointEOS(const FRoomIdRef &InRoomId)
    : FOnlinePartyIdRedpointBase(InRoomId)
{
}

FOnlinePartyIdRedpointEOS::FOnlinePartyIdRedpointEOS()
    : FOnlinePartyIdRedpointBase(nullptr)
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()