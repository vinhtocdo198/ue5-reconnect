// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "GauntletTestController.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSDeviceTestController.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class URedpointEOSDeviceTestController : public UGauntletTestController
{
    GENERATED_BODY()

private:
    TSharedPtr<::Redpoint::EOS::Async::TTask<int32>> CompletionTask;

public:
    virtual void OnInit() override;
    virtual void OnTick(float TimeDelta) override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
