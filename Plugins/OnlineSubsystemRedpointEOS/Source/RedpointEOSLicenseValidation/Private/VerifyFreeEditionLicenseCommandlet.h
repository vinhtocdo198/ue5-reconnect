// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "VerifyFreeEditionLicenseCommandlet.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class UVerifyFreeEditionLicenseCommandlet : public UCommandlet
{
    GENERATED_UCLASS_BODY()

public:
    virtual int32 Main(const FString &Params) override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()