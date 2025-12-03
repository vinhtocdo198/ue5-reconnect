// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class ONLINESUBSYSTEMREDPOINTEOS_API IEOSUser
{
public:
    IEOSUser() = default;
    UE_NONCOPYABLE(IEOSUser);
    virtual ~IEOSUser() = default;
};

REDPOINT_EOS_CODE_GUARD_END()