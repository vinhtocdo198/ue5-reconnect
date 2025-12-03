// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/VoiceChat/VoiceChatStats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{

REDPOINT_EOS_DEFINE_STAT(VoiceChat, SyncPerformDeferredSynchronisation);
REDPOINT_EOS_DEFINE_STAT(VoiceChatUser, SyncScheduleSynchronisation);
REDPOINT_EOS_DEFINE_STAT(VoiceChatUser, SyncPerformSynchronisation);
REDPOINT_EOS_DEFINE_STAT(VoiceChatUser, ApiSetAudioInputVolume);

}

REDPOINT_EOS_CODE_GUARD_END()
