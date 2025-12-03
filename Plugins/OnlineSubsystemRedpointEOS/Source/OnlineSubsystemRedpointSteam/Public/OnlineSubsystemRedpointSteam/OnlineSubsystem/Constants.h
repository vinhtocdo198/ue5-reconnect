// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{

extern ONLINESUBSYSTEMREDPOINTSTEAM_API const FName SubsystemName;

}

#define REDPOINT_STEAM_SUBSYSTEM                                                                                       \
    EMIT_CUSTOM_WARNING("The REDPOINT_STEAM_SUBSYSTEM macro has been deprecated. Please use "                          \
                        "Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem::SubsystemName instead.")        \
    FName(TEXT("RedpointSteam"))