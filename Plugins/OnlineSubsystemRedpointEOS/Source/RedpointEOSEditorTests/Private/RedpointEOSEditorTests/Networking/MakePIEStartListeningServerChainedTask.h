// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1931797628, Redpoint::EOS::Editor::Tests::Networking)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FMakePIEStartListeningServerChainedTask : public FChainedTask,
                                                public TSharedFromThis<FMakePIEStartListeningServerChainedTask>
{
private:
    int InstanceIndex;
    uint16 Port;
    FString ExtraParams;

public:
    FMakePIEStartListeningServerChainedTask(int InInstanceIndex, uint16 InPort, const FString &InExtraParams);
    UE_NONCOPYABLE(FMakePIEStartListeningServerChainedTask);
    virtual ~FMakePIEStartListeningServerChainedTask() override = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone) override;
};

}

namespace Redpoint::EOS::Editor::Tests::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1931797628,
    Redpoint::EOS::Editor::Tests::Networking,
    FMakePIEStartListeningServerChainedTask)
}

REDPOINT_EOS_CODE_GUARD_END()