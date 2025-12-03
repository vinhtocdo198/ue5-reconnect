// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IEOSCheck.h"

class FEOSEditorRequiresRestartForEpicEOSSDK : public IEOSCheck
{
public:
    FEOSEditorRequiresRestartForEpicEOSSDK() = default;
    UE_NONCOPYABLE(FEOSEditorRequiresRestartForEpicEOSSDK);
    virtual ~FEOSEditorRequiresRestartForEpicEOSSDK(){};

    virtual const TArray<FEOSCheckEntry> ProcessCustomSignal(const FString &Context, const FString &SignalId)
        const override;

    virtual void HandleAction(const FString &CheckId, const FString &ActionId) const override;
};