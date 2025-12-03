// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTestBeaconControlledClient.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(550696605, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FCloseClientPIEBeaconChainedTask : public FChainedTask, public TSharedFromThis<FCloseClientPIEBeaconChainedTask>
{
private:
    TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> BeaconClient;

public:
    FCloseClientPIEBeaconChainedTask(TSharedRef<TSoftObjectPtr<ARedpointEOSTestBeaconControlledClient>> InBeaconClient);
    UE_NONCOPYABLE(FCloseClientPIEBeaconChainedTask);
    virtual ~FCloseClientPIEBeaconChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Editor::Tests::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(550696605, Redpoint::EOS::Editor::Tests::Networking, FCloseClientPIEBeaconChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()