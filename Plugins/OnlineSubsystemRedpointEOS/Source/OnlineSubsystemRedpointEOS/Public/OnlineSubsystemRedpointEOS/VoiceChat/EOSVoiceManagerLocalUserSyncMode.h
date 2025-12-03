// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3226005201, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{

enum EEOSVoiceManagerLocalUserSyncMode : uint8
{
    None = 0x0,

    InputSettings = 0x1,

    OutputSettings = 0x2,

    TransmitMode = 0x4,

    JoinedRoom = 0x8,

    TransmitModeAndJoinedRoom = TransmitMode | JoinedRoom,

    All = 0xFF,
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3226005201,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    EEOSVoiceManagerLocalUserSyncMode)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()