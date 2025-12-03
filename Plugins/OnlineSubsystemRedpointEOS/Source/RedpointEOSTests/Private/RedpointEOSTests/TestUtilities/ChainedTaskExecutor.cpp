// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"
#include "RedpointEOSTests/TestUtilities/InstanceFactory.h"
#include "TestHelpers.h"

namespace REDPOINT_EOS_FILE_NS_ID(3518103627, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

void FChainedTaskExecutor::OnDoneRequested()
{
    this->OnDone();
}

void FChainedTaskExecutor::OnStepComplete(
    bool bContinueTesting,
    TSharedRef<FChainedTaskContext> InContext,
    TSharedRef<FChainedTaskExecutor> InSelf,
    TSharedRef<ChainedTask::FChainedTask> InExecutedStep,
    int32 InTotalSteps)
{
    if (this->Steps.Num() == 0 || !bContinueTesting)
    {
        // We're done testing.
        if (this->Steps.Num() == 0)
        {
            InContext->Assert().AddInfo(FString::Printf(
                TEXT("Finished executing all chained task steps."),
                InTotalSteps - this->Steps.Num(),
                InTotalSteps));
        }
        else
        {
            InContext->Assert().AddInfo(FString::Printf(
                TEXT("Task step %d has stopped chained task execution."),
                InTotalSteps - this->Steps.Num(),
                InTotalSteps));
        }
        this->OnDoneRequested();
        return;
    }

    auto Next = this->Steps[0];
    this->Steps.RemoveAt(0);
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("Executing chained task step %d of %d..."),
        InTotalSteps - this->Steps.Num(),
        InTotalSteps);
    InContext->Assert().AddInfo(FString::Printf(
        TEXT("Executing chained task step %d of %d..."),
        InTotalSteps - this->Steps.Num(),
        InTotalSteps));
    Next->ExecuteAsync(
        InContext,
        ChainedTask::FChainedTask::FOnComplete::CreateSP(
            this,
            &FChainedTaskExecutor::OnStepComplete,
            InContext,
            InSelf,
            Next,
            InTotalSteps));
}

FChainedTaskExecutor::FChainedTaskExecutor(
    FAsyncHotReloadableAutomationTestBase *InTest,
    TFunction<void()> InOnDone,
    const TArray<int32> &InInstanceLayout,
    EChainedTaskExecutorFlag InFlags)
    : Test(InTest)
    , OnDone(MoveTemp(InOnDone))
    , InstanceLayout(InInstanceLayout)
    , Steps()
    , bExecuting(false)
    , Flags(InFlags)
{
}

void FChainedTaskExecutor::Execute()
{
    checkf(!this->bExecuting, TEXT("Chained task already executing!"));
    TSharedRef<FChainedTaskExecutor> This = this->AsShared();
    if (this->Steps.Num() == 0)
    {
        this->Steps.Add(MakeShared<FLambdaChainedTask>(
            [](const FChainedTaskContextRef &, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
                TaskOnDone.ExecuteIfBound(true);
            }));
    }
    TSharedRef<FChainedTaskContext> Context = MakeShareable(new FChainedTaskContext(This));

    FInstanceFactory::EFlag FactoryFlags = FInstanceFactory::EFlag::None;
    if ((this->Flags & EChainedTaskExecutorFlag::DedicatedServer) != EChainedTaskExecutorFlag::None)
    {
        FactoryFlags |= FInstanceFactory::EFlag::DedicatedServer;
    }
    if ((this->Flags & EChainedTaskExecutorFlag::EpicGames) != EChainedTaskExecutorFlag::None)
    {
        FactoryFlags |= FInstanceFactory::EFlag::EpicGames;
    }

    FInstanceFactory::CreateInstances(
        this->Test,
        this->InstanceLayout,
        this->OnDone,
        FactoryFlags,
        [This, Context](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &_Unused) {
            auto TotalSteps = This->Steps.Num();
            auto First = This->Steps[0];
            This->Steps.RemoveAt(0);
            Context->AttachInstances(Instances);
            Context->Assert().AddInfo(FString::Printf(
                TEXT("Executing chained task step %d of %d..."),
                TotalSteps - This->Steps.Num(),
                TotalSteps));
            First->ExecuteAsync(
                Context,
                ChainedTask::FChainedTask::FOnComplete::CreateSP(
                    This,
                    &FChainedTaskExecutor::OnStepComplete,
                    Context,
                    This,
                    First,
                    TotalSteps));
        });
}

}