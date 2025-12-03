// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskRegisteredEvent.h"

namespace REDPOINT_EOS_FILE_NS_ID(1733755149, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

template <typename FEventHandlerType> class FWaitForEventChainedTask : public FChainedTask
{
public:
    typedef typename FChainedTaskRegisteredEventHelper<FEventHandlerType>::RegisteredEventType FRegisteredEventType;
    typedef typename FRegisteredEventType::FOnFilter FOnFilter;

private:
    FString EventName;
    float TimeoutSeconds;
    FOnFilter OnFilter;

public:
    FWaitForEventChainedTask(const FString &InEventName, float InTimeoutSeconds, const FOnFilter &InOnFilter)
        : EventName(InEventName)
        , TimeoutSeconds(InTimeoutSeconds)
        , OnFilter(InOnFilter)
    {
    }

    template <typename FunctorType, typename... VarTypes>
    FWaitForEventChainedTask(
        const FString &InEventName,
        float InTimeoutSeconds,
        FunctorType &&InFunctor,
        VarTypes &&...Vars)
        : EventName(InEventName)
        , TimeoutSeconds(InTimeoutSeconds)
        , OnFilter(FOnFilter::CreateLambda(InFunctor, Vars...))
    {
    }

    UE_NONCOPYABLE(FWaitForEventChainedTask);
    virtual ~FWaitForEventChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
    {
        auto RegisteredEvent = StaticCastSharedRef<FRegisteredEventType>(Context->RegisteredEvents[this->EventName]);
        checkf(OnDone.IsBound(), TEXT("OnDone should be bound!"));
        RegisteredEvent->WaitAndProcessEvents(Context, this->EventName, this->OnFilter, this->TimeoutSeconds, OnDone);
    }
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(1733755149, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FWaitForEventChainedTask)
}