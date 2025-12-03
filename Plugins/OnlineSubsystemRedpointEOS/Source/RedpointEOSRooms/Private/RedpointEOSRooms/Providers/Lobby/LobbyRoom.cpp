// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoom.h"

#include "RedpointEOSAPI/Lobby/CopyLobbyDetailsHandle.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProvider.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1188242319, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;

TReleasableRef<EOS_HLobbyDetails> FLobbyRoom::ResolveLobbyDetails(
    const FPlatformHandle &PlatformHandle,
    const FRoomUserId &LocalUser,
    const FString &LobbyId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    // @note: The lobby details can be stored for the lifetime of the lobby. We do not need
    // to get a new EOS_HLobbyDetails when the data associated with a lobby (e.g. info or
    // member count) changes.
    EOS_EResult UnusedResultCode;
    return FCopyLobbyDetailsHandle::Execute(
        PlatformHandle,
        FCopyLobbyDetailsHandle::Options{GetProductUserId(LocalUser), LobbyId},
        UnusedResultCode);
}

FLobbyRoom::FLobbyRoom(const FPlatformHandle &InPlatformHandle, const FRoomUserId &LocalUser, const FString &LobbyId)
    : FRoomImpl(LocalUser, MakeShared<FRoomId>(Redpoint::EOS::Rooms::Providers::Lobby::LobbyRoomProviderName, LobbyId))
    , PlatformHandle(InPlatformHandle)
    , LobbyDetails(ResolveLobbyDetails(InPlatformHandle, LocalUser, LobbyId))
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);
}

TReleasableRef<EOS_HLobbyDetails> FLobbyRoom::GetLobbyDetails() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbyDetails;
}

}

REDPOINT_EOS_CODE_GUARD_END()