// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_IS_FREE_EDITION
#include "Checks/IEOSCheck.h"
#include "ITargetDeviceServicesModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_IS_FREE_EDITION

namespace REDPOINT_EOS_FILE_NS_ID(2943692109, Redpoint::EOS::Editor::Checks)
{

class FMatchmakingNotAvailableCheck : public IEOSCheck
{
private:
    FDateTime NextAllowedDisplayTime;

public:
    FMatchmakingNotAvailableCheck();
    UE_NONCOPYABLE(FMatchmakingNotAvailableCheck);
    virtual ~FMatchmakingNotAvailableCheck() override = default;

    virtual const TArray<FEOSCheckEntry> ProcessCustomSignal(const FString &Context, const FString &SignalId)
        const override;

    virtual void HandleAction(const FString &CheckId, const FString &ActionId) const override;
};

}

namespace Redpoint::EOS::Editor::Checks
{
REDPOINT_EOS_FILE_NS_EXPORT(2943692109, Redpoint::EOS::Editor::Checks, FMatchmakingNotAvailableCheck);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()