// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTestBeaconControlledClient.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3432554009, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FConnectClientPIEToHostPIEChainedTask : public FChainedTask,
                                              public TSharedFromThis<FConnectClientPIEToHostPIEChainedTask>
{
private:
    int HostInstanceIndex;
    int ClientInstanceIndex;
    uint16 Port;
    FString ExtraParams;
    double TimeoutAt;
    double TimeoutSeconds;
    TArray<TSoftObjectPtr<APlayerController>> ExistingControllersOnHost;
    FSimpleDelegate BeforeBrowse;

public:
    FConnectClientPIEToHostPIEChainedTask(
        int InHostInstanceIndex,
        int InClientInstanceIndex,
        uint16 InPort,
        FSimpleDelegate InBeforeBrowse = FSimpleDelegate(),
        double InTimeoutSeconds = 10.0);
    UE_NONCOPYABLE(FConnectClientPIEToHostPIEChainedTask);
    virtual ~FConnectClientPIEToHostPIEChainedTask() override = default;

private:
    bool ConnectClientPIEToHostPIE(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);
    bool WaitForClientPIEToConnectToHostPIE(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone);

public:
    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Editor::Tests::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(3432554009, Redpoint::EOS::Editor::Tests::Networking, FConnectClientPIEToHostPIEChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()