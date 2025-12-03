// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(189412454, Redpoint::EOS::Tests::TestUtilities)
{

class REDPOINTEOSTESTS_API TestHelpers
{
public:
    static int16 WorkerNum;
    // Returns a port number offset by the worker num to avoid port collisions.
    static int Port(int BasePort);
};

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(189412454, Redpoint::EOS::Tests::TestUtilities, TestHelpers)
}

REDPOINT_EOS_CODE_GUARD_END()