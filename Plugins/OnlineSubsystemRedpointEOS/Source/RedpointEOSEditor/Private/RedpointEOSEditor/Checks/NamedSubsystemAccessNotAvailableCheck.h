// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_IS_FREE_EDITION
#include "Checks/IEOSCheck.h"
#include "ITargetDeviceServicesModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_IS_FREE_EDITION

namespace REDPOINT_EOS_FILE_NS_ID(847567554, Redpoint::EOS::Editor::Checks)
{

class FNamedSubsystemAccessNotAvailableCheck : public IEOSCheck
{
private:
    FDateTime NextAllowedDisplayTime;

public:
    FNamedSubsystemAccessNotAvailableCheck();
    UE_NONCOPYABLE(FNamedSubsystemAccessNotAvailableCheck);
    virtual ~FNamedSubsystemAccessNotAvailableCheck() override = default;

    virtual const TArray<FEOSCheckEntry> ProcessCustomSignal(const FString &Context, const FString &SignalId)
        const override;

    virtual void HandleAction(const FString &CheckId, const FString &ActionId) const override;
};

}

namespace Redpoint::EOS::Editor::Checks
{
REDPOINT_EOS_FILE_NS_EXPORT(847567554, Redpoint::EOS::Editor::Checks, FNamedSubsystemAccessNotAvailableCheck);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()