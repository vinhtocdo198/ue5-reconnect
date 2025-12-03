// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskRegisteredEvent.h"

namespace REDPOINT_EOS_FILE_NS_ID(1359423209, Redpoint::EOS::Tests::TestUtilities::ChainedTask)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

template <typename FEventHandlerType> class FRegisterEventChainedTask : public FChainedTask
{
public:
    typedef TDelegate<FDelegateHandle(const FChainedTaskContextRef &Context, const FEventHandlerType &OnEventRaised)>
        FOnRegister;
    typedef TDelegate<void(const FChainedTaskContextRef &, FDelegateHandle)> FOnUnregister;
    typedef typename FChainedTaskRegisteredEventHelper<FEventHandlerType>::RegisteredEventType FRegisteredEventType;

private:
    FString EventName;
    FOnRegister OnRegister;
    FOnUnregister OnUnregister;

public:
    FRegisterEventChainedTask(
        const FString &InEventName,
        const FOnRegister &InOnRegister,
        const FOnUnregister &InOnUnregister)
        : EventName(InEventName)
        , OnRegister(InOnRegister)
        , OnUnregister(InOnUnregister)
    {
    }

    template <typename RegisterFunctorType, typename UnregisterFunctorType>
    FRegisterEventChainedTask(
        const FString &InEventName,
        RegisterFunctorType &&InOnRegister,
        UnregisterFunctorType &&InOnUnregister)
        : EventName(InEventName)
        , OnRegister(FRegisterEventChainedTask::FOnRegister::CreateLambda(InOnRegister))
        , OnUnregister(FRegisterEventChainedTask::FOnUnregister::CreateLambda(InOnUnregister))
    {
    }

    UE_NONCOPYABLE(FRegisterEventChainedTask);
    virtual ~FRegisterEventChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
    {
        auto RegisteredEvent = MakeShared<FRegisteredEventType>(this->OnUnregister);
        this->OnRegister.Execute(Context, RegisteredEvent->GetDelegateForRegistration());
        Context->RegisteredEvents.Add(this->EventName, RegisteredEvent);
        OnDone.Execute(true);
    }
};

}

namespace Redpoint::EOS::Tests::TestUtilities::ChainedTask
{
REDPOINT_EOS_FILE_NS_EXPORT(1359423209, Redpoint::EOS::Tests::TestUtilities::ChainedTask, FRegisterEventChainedTask)
}