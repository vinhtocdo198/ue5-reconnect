// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

namespace Openskill
{
namespace Statistics
{
double PhiMajor(const double &X);
double PhiMajorInverse(const double &X);
double PhiMinor(const double &X);
double V(const double &X, const double &T);
double W(const double &X, const double &T);
double VT(const double &X, const double &T);
double WT(const double &X, const double &T);
}; // namespace Statistics
}; // namespace Openskill

#endif

REDPOINT_EOS_CODE_GUARD_END()