// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointGameInstance.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1074738087, Redpoint::EOS::Framework)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;

void GetAutomaticPartySettings(
    const FPlatformHandle &PlatformHandle,
    bool &bOutEnsureAlwaysInParty,
    int32 &OutPartyMaximumSize,
    bool &bOutEnableVoiceChatInParty);

void CreateAutomaticParty(
    int32 PartySize,
    bool bEnableVoiceChatInParty,
    const TSharedRef<IRoomSystem> &RoomSystem,
    const FRoomUserId &LocalUserId,
    const IRoomSystem::FOnCreateRoomComplete &OnComplete);

}

namespace Redpoint::EOS::Framework
{
REDPOINT_EOS_FILE_NS_EXPORT(1074738087, Redpoint::EOS::Framework, GetAutomaticPartySettings)
REDPOINT_EOS_FILE_NS_EXPORT(1074738087, Redpoint::EOS::Framework, CreateAutomaticParty)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()