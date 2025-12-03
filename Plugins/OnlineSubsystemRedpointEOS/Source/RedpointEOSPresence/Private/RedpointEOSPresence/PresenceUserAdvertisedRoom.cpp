// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3258393336, Redpoint::EOS::Presence)
{

FPresenceUserAdvertisedRoom::FPresenceUserAdvertisedRoom(const FRoomIdRef &InRoomId, const FName &InRoomNamespace)
    : RoomId(InRoomId)
    , RoomNamespace(InRoomNamespace)
    , CurrentMemberCount()
    , MaximumMemberCount()
    , bJoinable(false)
    , SecretPassword()
    , bRequiresSecretPassword()
    , bIsMatch()
    , bMatchStarted()
{
}

uint32 GetTypeHash(const FPresenceUserAdvertisedRoom &P)
{
    uint32 H = GetTypeHashHelper(*P.RoomId);
    H = HashCombineFast(H, GetTypeHashHelper(P.RoomNamespace));
    H = HashCombineFast(H, GetTypeHashHelper(P.CurrentMemberCount));
    H = HashCombineFast(H, GetTypeHashHelper(P.MaximumMemberCount));
    H = HashCombineFast(H, GetTypeHashHelper(P.bJoinable));
    H = HashCombineFast(H, GetTypeHashHelper(P.SecretPassword));
    H = HashCombineFast(H, GetTypeHashHelper(P.bRequiresSecretPassword));
    H = HashCombineFast(H, GetTypeHashHelper(P.bIsMatch));
    H = HashCombineFast(H, GetTypeHashHelper(P.bMatchStarted));
    return H;
}

#if !REDPOINT_EOS_UE_5_4_OR_LATER
uint32 GetTypeHash(const TOptional<FPresenceUserAdvertisedRoom> &P)
{
    return P.IsSet() ? GetTypeHashHelper(*P) : 0;
}
#endif

}

#endif

REDPOINT_EOS_CODE_GUARD_END()