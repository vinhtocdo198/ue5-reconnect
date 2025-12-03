// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./OpenskillConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

struct FOpenskillRating
{
private:
    double Mu;
    double Sigma;

public:
    FOpenskillRating()
        : Mu(Openskill::DefaultMu)
        , Sigma(Openskill::DefaultSigma)
    {
    }
    FOpenskillRating(double InMu, double InSigma)
        : Mu(InMu)
        , Sigma(InSigma)
    {
    }
    FOpenskillRating(const FOpenskillRating &) = default;
    FOpenskillRating(FOpenskillRating &&) = default;
    ~FOpenskillRating() = default;

    FOpenskillRating &operator=(const FOpenskillRating &) = default;
    FOpenskillRating &operator=(FOpenskillRating &&) = default;

    FORCEINLINE double GetMu() const
    {
        return this->Mu;
    }

    FORCEINLINE double GetSigma() const
    {
        return this->Sigma;
    }

    FORCEINLINE double GetOrdinal() const
    {
        return this->Mu - Openskill::DefaultZ * this->Sigma;
    }

    FORCEINLINE bool Equals(const FOpenskillRating &Other) const
    {
        return FMath::IsNearlyEqual(this->Mu, Other.Mu) && FMath::IsNearlyEqual(this->Sigma, Other.Sigma);
    }

    friend bool operator==(const FOpenskillRating &InA, const FOpenskillRating &InB)
    {
        return InA.Equals(InB);
    }

    friend bool operator!=(const FOpenskillRating &InA, const FOpenskillRating &InB)
    {
        return !InA.Equals(InB);
    }
};

#endif

REDPOINT_EOS_CODE_GUARD_END()