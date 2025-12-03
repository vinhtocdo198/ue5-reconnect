// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/FlagEnum.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskRegisteredEvent.h"

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(4034983407, Redpoint::EOS::Tests::TestUtilities, FChainedTaskContext)

namespace REDPOINT_EOS_FILE_NS_ID(3518103627, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

enum class EChainedTaskExecutorFlag : uint8
{
    None,

    DedicatedServer = 0x2,

    EpicGames = 0x4,
};

REDPOINT_EOS_DECLARE_FLAG_ENUMERATION_OPERATORS(EChainedTaskExecutorFlag);

class REDPOINTEOSTESTS_API FChainedTaskExecutor : public TSharedFromThis<FChainedTaskExecutor>
{
    friend FChainedTaskContext;

protected:
    FAsyncHotReloadableAutomationTestBase *Test;
    TFunction<void()> OnDone;
    TArray<int32> InstanceLayout;
    TArray<TSharedRef<ChainedTask::FChainedTask>> Steps;
    bool bExecuting;
    EChainedTaskExecutorFlag Flags;

    virtual void OnDoneRequested();

    void OnStepComplete(
        bool bInContinueTesting,
        TSharedRef<FChainedTaskContext> InContext,
        TSharedRef<FChainedTaskExecutor> InSelf,
        // @note: This is only passed to hold the FChainedTask alive while it's executing.
        TSharedRef<ChainedTask::FChainedTask> InExecutedStep,
        int32 InTotalSteps);

public:
    FChainedTaskExecutor(
        FAsyncHotReloadableAutomationTestBase *InTest,
        TFunction<void()> InOnDone,
        const TArray<int32> &InInstanceLayout,
        EChainedTaskExecutorFlag InFlags = EChainedTaskExecutorFlag::None);
    UE_NONCOPYABLE(FChainedTaskExecutor);
    virtual ~FChainedTaskExecutor() = default;

    template <typename InObjectType, typename... InArgTypes> TSharedRef<FChainedTaskExecutor> Then(InArgTypes &&...Args)
    {
        static_assert(
            std::is_base_of_v<ChainedTask::FChainedTask, InObjectType>,
            "InObjectType must be an FChainedTask!");
        TSharedRef<InObjectType> Next = MakeShared<InObjectType>(Forward<InArgTypes>(Args)...);
        Steps.Add(Next);
        return this->AsShared();
    }

    virtual void Execute();
};

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(3518103627, Redpoint::EOS::Tests::TestUtilities, EChainedTaskExecutorFlag)
REDPOINT_EOS_FILE_NS_EXPORT(3518103627, Redpoint::EOS::Tests::TestUtilities, FChainedTaskExecutor)
}