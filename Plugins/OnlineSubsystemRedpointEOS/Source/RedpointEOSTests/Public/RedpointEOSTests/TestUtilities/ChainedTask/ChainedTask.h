// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(4074429332, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

class REDPOINTEOSTESTS_API FChainedTask
{
public:
    FChainedTask() = default;
    UE_NONCOPYABLE(FChainedTask);
    virtual ~FChainedTask() = default;

    typedef TDelegate<void(bool bContinueTesting)> FOnComplete;

    virtual void ExecuteAsync(const FChainedTaskContextRef &, const FOnComplete &OnDone) = 0;
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(4074429332, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FChainedTask)
}