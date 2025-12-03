// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

class REDPOINTEOSAPI_API IReleasableBase
{
protected:
    virtual FPlatformHandle GetPlatformHandle() const = 0;
    static FPlatformHandle GetPlatformHandleFromBase(const IReleasableBase &InBase)
    {
        return InBase.GetPlatformHandle();
    }

    IReleasableBase() = default;

public:
    UE_NONCOPYABLE(IReleasableBase);
    virtual ~IReleasableBase() = default;
};

}

REDPOINT_EOS_CODE_GUARD_END()