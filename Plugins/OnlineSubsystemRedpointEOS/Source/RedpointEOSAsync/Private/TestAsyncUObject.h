// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "TestAsyncUObject.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class UTestAsyncUObject : public UObject
{
    GENERATED_BODY()

public:
    ::Redpoint::EOS::Async::TTask<void> VoidTask();
    ::Redpoint::EOS::Async::TTask<int> IntTask();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()