// Copyright June Rhodes. All Rights Reserved.

#include "./OpenskillStatistics.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "gcem.hpp"
#include "stats.hpp"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

namespace Openskill
{
namespace Statistics
{
double PhiMajor(const double &X)
{
    // cdf
    return stats::pnorm(X, 0.0, 1.0);
}

double PhiMajorInverse(const double &X)
{
    // ppf
    return stats::qnorm(X, 0.0, 1.0);
}

double PhiMinor(const double &X)
{
    // pdf
    return stats::dnorm(X, 0.0, 1.0);
}

double V(const double &X, const double &T)
{
    double XT = X - T;
    double Denom = PhiMajor(XT);
    if (Denom < DBL_EPSILON)
    {
        return -XT;
    }
    else
    {
        return PhiMinor(XT) / Denom;
    }
}

double W(const double &X, const double &T)
{
    double XT = X - T;
    double Denom = PhiMajor(XT);
    if (Denom < DBL_EPSILON)
    {
        return (X < 0) ? 1.0 : 0.0;
    }
    else
    {
        double VXT = V(X, T);
        return VXT * (VXT + XT);
    }
}

double VT(const double &X, const double &T)
{
    double XX = gcem::abs(X);
    double B = PhiMajor(T - XX) - PhiMajor(-T - XX);
    if (B < 1.0e-5)
    {
        if (X < 0)
        {
            return -X - T;
        }
        else
        {
            return -X + T;
        }
    }
    else
    {
        double A = PhiMinor(-T - XX) - PhiMinor(T - XX);
        if (X < 0)
        {
            return -A / B;
        }
        else
        {
            return A / B;
        }
    }
}

double WT(const double &X, const double &T)
{
    double XX = gcem::abs(X);
    double B = PhiMajor(T - XX) - PhiMajor(-T - XX);
    if (B < DBL_EPSILON)
    {
        return 1.0;
    }
    else
    {
        double VTXT = VT(X, T);
        return ((T - XX) * PhiMinor(T - XX) + (T + XX) * PhiMinor(-T - XX)) / B + VTXT * VTXT;
    }
}
}; // namespace Statistics
}; // namespace Openskill

#endif

REDPOINT_EOS_CODE_GUARD_END()