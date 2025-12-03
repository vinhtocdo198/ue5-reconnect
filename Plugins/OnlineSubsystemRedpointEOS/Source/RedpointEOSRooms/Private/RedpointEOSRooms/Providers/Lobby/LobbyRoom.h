// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Providers/RoomImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4289788093, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProvider)

namespace REDPOINT_EOS_FILE_NS_ID(1188242319, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::API;

class FLobbyRoom : public FRoomImpl, public TSharedFromThis<FLobbyRoom>
{
public:
    FPlatformHandle PlatformHandle;

private:
    TReleasableRef<EOS_HLobbyDetails> LobbyDetails;

    static TReleasableRef<EOS_HLobbyDetails> ResolveLobbyDetails(
        const FPlatformHandle &PlatformHandle,
        const FRoomUserId &LocalUser,
        const FString &LobbyId);

public:
    FLobbyRoom(const FPlatformHandle &PlatformHandle, const FRoomUserId &LocalUser, const FString &LobbyId);
    UE_NONCOPYABLE(FLobbyRoom);
    virtual ~FLobbyRoom() override = default;

    TReleasableRef<EOS_HLobbyDetails> GetLobbyDetails() const;
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(1188242319, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoom)
}

REDPOINT_EOS_CODE_GUARD_END()