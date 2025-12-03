// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "Settings/LevelEditorPlaySettings.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2606800861, Redpoint::EOS::Editor::Tests)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

class FEditorChainedTaskExecutor : public FChainedTaskExecutor
{
private:
    EPlayNetMode PlayMode;
    FString AdditionalLaunchParameters;
    double TimeoutAt;

    [[nodiscard]] bool CreateMapIfNeeded(const FString &MapName);

    virtual void OnDoneRequested() override;

    void OnStepComplete(
        bool bContinueTesting,
        TSharedRef<FChainedTaskContext> InContext,
        TSharedRef<FChainedTaskExecutor> InSelf,
        TSharedRef<ChainedTask::FChainedTask> InExecutedStep,
        int32 InTotalSteps);

public:
    FEditorChainedTaskExecutor(
        FAsyncHotReloadableAutomationTestBase *InTest,
        TFunction<void()> InOnDone,
        const TArray<int32> &InInstanceLayout,
        EPlayNetMode InPlayMode,
        const FString &InAdditionalLaunchParameters = TEXT(""),
        EChainedTaskExecutorFlag InFlags = EChainedTaskExecutorFlag::None);
    UE_NONCOPYABLE(FEditorChainedTaskExecutor);
    virtual ~FEditorChainedTaskExecutor() override = default;

    virtual void Execute() override;

private:
    bool HasStandalonePlayerControllers(TArray<FInstanceFactory::FInstanceDescriptor> InstanceDescriptors);
    bool HasListenServerPlayerControllers();
    bool HasDedicatedServerPlayerControllers();

    bool OnCheckIfReadyToTest(
        float DeltaSeconds,
        TSharedRef<FChainedTaskContext> Context,
        TArray<FInstanceFactory::FInstanceDescriptor> InstanceDescriptors);
};

}

namespace Redpoint::EOS::Editor::Tests
{
REDPOINT_EOS_FILE_NS_EXPORT(2606800861, Redpoint::EOS::Editor::Tests, FEditorChainedTaskExecutor)
}

REDPOINT_EOS_CODE_GUARD_END()