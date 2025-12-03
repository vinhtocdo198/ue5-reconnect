// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/Optional.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1984048542, Redpoint::Matchmaking::Utility)
{

MATCHMAKING_API TOptional<TArray<int32>> ParseTeamCapacities(const FString &TeamCapacities);

}

namespace Redpoint::Matchmaking::Utility
{
REDPOINT_EOS_FILE_NS_EXPORT(1984048542, Redpoint::Matchmaking::Utility, ParseTeamCapacities);
}

REDPOINT_EOS_CODE_GUARD_END()