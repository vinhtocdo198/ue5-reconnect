// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointEOSRooms/Room.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3350094983, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;

class FOnlineLobbyEOS : public FOnlineLobby
{
public:
    FOnlineLobbyEOS(const FRoomRef &InRoom);
    UE_NONCOPYABLE(FOnlineLobbyEOS);
    virtual ~FOnlineLobbyEOS() = default;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3350094983,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby,
    FOnlineLobbyEOS)
}

#endif