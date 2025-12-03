// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(973220594, Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat)
{

typedef TDelegate<void()> FEOSVoiceManagerLocalUserPostLoginComplete;

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat
{
REDPOINT_EOS_FILE_NS_EXPORT(
    973220594,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat,
    FEOSVoiceManagerLocalUserPostLoginComplete)
}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER

REDPOINT_EOS_CODE_GUARD_END()