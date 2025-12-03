// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Lobby/OnlineLobbyEOS.h"

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"
#include "RedpointEOSCore/Id/Id.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3350094983, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem;

FOnlineLobbyEOS::FOnlineLobbyEOS(const FRoomRef &InRoom)
{
    using namespace ::Redpoint::EOS::Core::Id;
    this->Id = Party::FOnlinePartyIdRedpointBase::Create(InRoom->GetRoomId());
    this->OwnerId = GetUniqueNetId(InRoom->GetRoomOwner()->GetUserId());
}

}

#endif