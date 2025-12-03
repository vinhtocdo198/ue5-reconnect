// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTestBeaconControlledHost.h"
#include "RedpointEOSTestBeaconControlledHostObject.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1588596934, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FSpawnBeaconOnPIEChainedTask : public FChainedTask, public TSharedFromThis<FSpawnBeaconOnPIEChainedTask>
{
private:
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> BeaconHost;
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> BeaconHostObject;
    int InstanceIndex;
    uint16 Port;
    FString ExtraParams;

public:
    FSpawnBeaconOnPIEChainedTask(
        TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHost>> InBeaconHost,
        TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledHostObject>> InBeaconHostObject,
        int InInstanceIndex,
        uint16 InPort,
        const FString &InExtraParams);
    UE_NONCOPYABLE(FSpawnBeaconOnPIEChainedTask);
    virtual ~FSpawnBeaconOnPIEChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Editor::Tests::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(1588596934, Redpoint::EOS::Editor::Tests::Networking, FSpawnBeaconOnPIEChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()