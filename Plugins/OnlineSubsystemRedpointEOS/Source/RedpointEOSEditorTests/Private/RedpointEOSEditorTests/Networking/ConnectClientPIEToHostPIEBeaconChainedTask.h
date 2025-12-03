// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTestBeaconControlledClient.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1401072254, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FConnectClientPIEToHostPIEBeaconChainedTask : public FChainedTask,
                                                    public TSharedFromThis<FConnectClientPIEToHostPIEBeaconChainedTask>
{
private:
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClient;
    int HostInstanceIndex;
    int ClientInstanceIndex;
    uint16 Port;
    double TimeoutAt;

public:
    FConnectClientPIEToHostPIEBeaconChainedTask(
        TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> InBeaconClient,
        int InHostInstanceIndex,
        int InClientInstanceIndex,
        uint16 InPort);
    UE_NONCOPYABLE(FConnectClientPIEToHostPIEBeaconChainedTask);
    virtual ~FConnectClientPIEToHostPIEBeaconChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Editor::Tests::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1401072254,
    Redpoint::EOS::Editor::Tests::Networking,
    FConnectClientPIEToHostPIEBeaconChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()