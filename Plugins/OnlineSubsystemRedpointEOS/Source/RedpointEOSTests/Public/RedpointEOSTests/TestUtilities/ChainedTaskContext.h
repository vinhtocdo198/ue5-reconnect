// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Misc/AutomationTest.h"
#include "OnlineSubsystem.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSTests/TestUtilities/MultiplayerScenarioInstance.h"
#include "TestHelpers.h"

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3518103627, Redpoint::EOS::Tests::TestUtilities, FChainedTaskExecutor)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    372572194,
    Redpoint::EOS::Tests::TestUtilities,
    FChainedTaskRegisteredEventBase)

namespace REDPOINT_EOS_FILE_NS_ID(4034983407, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

class REDPOINTEOSTESTS_API FChainedTaskContext
{
    friend FChainedTaskExecutor;

public:
    // @todo: Hide this and make it available only to the register/wait/unregister classes.
    TMap<FString, TSharedRef<FChainedTaskRegisteredEventBase>> RegisteredEvents;

private:
    TSharedPtr<FChainedTaskExecutor> Executor;
    TArray<FMultiplayerScenarioInstance> Instances;

public:
    FChainedTaskContext(const TSharedRef<FChainedTaskExecutor> &InExecutor);
    UE_NONCOPYABLE(FChainedTaskContext);
    ~FChainedTaskContext() = default;

    // @todo: Hide this only for FChainedTaskExecutor and it's descendants.
    void AttachInstances(const TArray<FMultiplayerScenarioInstance> &InInstances);

    FAutomationTestBase &Assert() const;
    IOnlineSubsystem &Instance(int Index);
    const FUniqueNetIdEOS &User(int Index);
    FWorldContext *WorldContext(int Index);
    TSoftObjectPtr<UWorld> World(int Index);
};

typedef TSharedRef<FChainedTaskContext> FChainedTaskContextRef;

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(4034983407, Redpoint::EOS::Tests::TestUtilities, FChainedTaskContext)
REDPOINT_EOS_FILE_NS_EXPORT(4034983407, Redpoint::EOS::Tests::TestUtilities, FChainedTaskContextRef)
}