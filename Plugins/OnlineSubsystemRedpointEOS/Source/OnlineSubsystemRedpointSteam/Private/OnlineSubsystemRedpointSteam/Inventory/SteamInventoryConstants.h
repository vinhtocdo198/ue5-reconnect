// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory
{

#if defined(REDPOINT_EOS_STEAM_ENABLED)

extern const TCHAR *SteamTypeDlc;
extern const TCHAR *SteamTypeItem;
extern const TCHAR *SteamTypeItemDefinition;

#endif

}