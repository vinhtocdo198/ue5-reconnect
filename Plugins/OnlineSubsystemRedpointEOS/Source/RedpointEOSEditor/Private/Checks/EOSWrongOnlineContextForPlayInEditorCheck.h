// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IEOSCheck.h"

class FEOSWrongOnlineContextForPlayInEditorCheck : public IEOSCheck
{
public:
    FEOSWrongOnlineContextForPlayInEditorCheck() = default;
    UE_NONCOPYABLE(FEOSWrongOnlineContextForPlayInEditorCheck);
    virtual ~FEOSWrongOnlineContextForPlayInEditorCheck() = default;

    virtual const TArray<FEOSCheckEntry> ProcessCustomSignal(const FString &Context, const FString &SignalId)
        const override;

    virtual void HandleAction(const FString &CheckId, const FString &ActionId) const override;
};