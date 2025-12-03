// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskRegisteredEvent.h"

namespace REDPOINT_EOS_FILE_NS_ID(3904195470, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class REDPOINTEOSTESTS_API FUnregisterEventChainedTask : public FChainedTask
{
private:
    FString EventName;

public:
    FUnregisterEventChainedTask(const FString &InEventName)
        : EventName(InEventName)
    {
    }
    UE_NONCOPYABLE(FUnregisterEventChainedTask);
    virtual ~FUnregisterEventChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
    {
        Context->RegisteredEvents[this->EventName]->Unregister(Context);
        Context->RegisteredEvents.Remove(this->EventName);
        OnDone.Execute(true);
    }
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(3904195470, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FUnregisterEventChainedTask)
}