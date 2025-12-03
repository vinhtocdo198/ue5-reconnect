// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2367786565, Redpoint::EOS::AntiCheat)
{

REDPOINTEOSANTICHEAT_API FString
EOS_EAntiCheatCommonClientActionReason_ToString(const EOS_EAntiCheatCommonClientActionReason &Reason);

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(2367786565, Redpoint::EOS::AntiCheat, EOS_EAntiCheatCommonClientActionReason_ToString)
}

REDPOINT_EOS_CODE_GUARD_END()