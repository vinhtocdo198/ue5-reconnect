// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

class REDPOINTEOSAPI_API INativeHeapStateCleanup
{
public:
    INativeHeapStateCleanup() = default;
    INativeHeapStateCleanup(const INativeHeapStateCleanup &) = default;
    INativeHeapStateCleanup(INativeHeapStateCleanup &&) = default;
    virtual ~INativeHeapStateCleanup() = default;
};

}

REDPOINT_EOS_CODE_GUARD_END()