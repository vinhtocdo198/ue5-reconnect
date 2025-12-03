// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "Containers/Array.h"
#include <functional>

EOS_ENABLE_STRICT_WARNINGS

class ONLINESUBSYSTEMREDPOINTEOS_API FAsyncMutex
{
private:
    bool Available;
    TArray<TFunction<void(const TFunction<void()> &MutexRelease)>> Pending;

public:
    FAsyncMutex()
        : Available(true){};
    void Run(const TFunction<void(const TFunction<void()> &MutexRelease)> &Callback);
};

EOS_DISABLE_STRICT_WARNINGS
