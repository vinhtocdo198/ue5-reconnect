// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

#include "Engine/Engine.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"

namespace REDPOINT_EOS_FILE_NS_ID(4034983407, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

FChainedTaskContext::FChainedTaskContext(const TSharedRef<FChainedTaskExecutor> &InExecutor)
    : Executor(InExecutor)
    , Instances()
{
}

void FChainedTaskContext::AttachInstances(const TArray<FMultiplayerScenarioInstance> &InInstances)
{
    this->Instances = InInstances;
}

FAutomationTestBase &FChainedTaskContext::Assert() const
{
    return *this->Executor->Test;
}

IOnlineSubsystem &FChainedTaskContext::Instance(int Index)
{
    IOnlineSubsystemPtr Ptr = this->Instances[Index].Subsystem.Pin();
    checkf(Ptr.IsValid(), TEXT("Expected online subsystem to be valid!"));
    return *Ptr.Get();
}

const FUniqueNetIdEOS &FChainedTaskContext::User(int Index)
{
    return *this->Instances[Index].UserId.Get();
}

FWorldContext *FChainedTaskContext::WorldContext(int Index)
{
    if (GEngine == nullptr)
    {
        return nullptr;
    }
    return GEngine->GetWorldContextFromHandle(this->Instances[Index].WorldContextHandle);
}

TSoftObjectPtr<UWorld> FChainedTaskContext::World(int Index)
{
    auto WorldContext = this->WorldContext(Index);
    if (WorldContext == nullptr)
    {
        return nullptr;
    }
    return WorldContext->World();
}

}