// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

namespace Openskill
{
constexpr double DefaultZ = 3.0;
constexpr double DefaultMu = 25.0;
constexpr double DefaultSigma = DefaultMu / DefaultZ;
constexpr double DefaultEpsilon = 0.0001;
constexpr double DefaultBeta = DefaultSigma / 2;
constexpr double DefaultBetaSquared = DefaultBeta * DefaultBeta;
constexpr double DefaultTwoBetaSquared = DefaultBetaSquared * 2;
}; // namespace Openskill

typedef uint16 FOpenskillRanking;

#endif

REDPOINT_EOS_CODE_GUARD_END()