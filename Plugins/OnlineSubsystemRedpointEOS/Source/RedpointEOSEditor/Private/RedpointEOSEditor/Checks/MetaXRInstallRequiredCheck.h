// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Checks/IEOSCheck.h"
#include "ITargetDeviceServicesModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(300840337, Redpoint::EOS::Editor::Checks)
{

class FMetaXRInstallRequiredCheck : public IEOSCheck
{
private:
    float SecondsUntilNextCheck;
    bool bNeverCheckAgain;

public:
    FMetaXRInstallRequiredCheck();
    UE_NONCOPYABLE(FMetaXRInstallRequiredCheck);
    virtual ~FMetaXRInstallRequiredCheck() override = default;

    virtual bool ShouldTick() const override;

    virtual const TArray<FEOSCheckEntry> Tick(float DeltaSeconds) override;

    virtual void HandleAction(const FString &CheckId, const FString &ActionId) const override;
};

}

namespace Redpoint::EOS::Editor::Checks
{
REDPOINT_EOS_FILE_NS_EXPORT(300840337, Redpoint::EOS::Editor::Checks, FMetaXRInstallRequiredCheck);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()