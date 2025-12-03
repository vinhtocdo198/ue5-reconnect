// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{

REDPOINT_EOS_DECLARE_CYCLE_STAT(
    VoiceChat,
    TEXT("Sync/PerformDeferredSynchronisation"),
    SyncPerformDeferredSynchronisation);
REDPOINT_EOS_DECLARE_CYCLE_STAT(VoiceChatUser, TEXT("Sync/ScheduleSynchronisation"), SyncScheduleSynchronisation);
REDPOINT_EOS_DECLARE_CYCLE_STAT(VoiceChatUser, TEXT("Sync/PerformSynchronisation"), SyncPerformSynchronisation);
REDPOINT_EOS_DECLARE_CYCLE_STAT(VoiceChatUser, TEXT("Api/SetAudioInputVolume"), ApiSetAudioInputVolume);

}

REDPOINT_EOS_CODE_GUARD_END()
