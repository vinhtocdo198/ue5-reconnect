// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IEOSCheck.h"

#if !REDPOINT_EOS_UE_5_5_OR_LATER

class FEOSNetDriverSettingsCheck : public IEOSCheck
{
public:
    FEOSNetDriverSettingsCheck();
    UE_NONCOPYABLE(FEOSNetDriverSettingsCheck);
    virtual ~FEOSNetDriverSettingsCheck() {};

    virtual bool ShouldTick() const override
    {
        return true;
    }

    virtual const TArray<FEOSCheckEntry> ProcessCustomSignal(const FString &Context, const FString &SignalId)
        const override;

    virtual void HandleAction(const FString &CheckId, const FString &ActionId) const override;
};

#endif